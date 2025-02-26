// Grupo 49
// Larissa Oliveira - 59830
// Nickolas Ficker - 60810
// Tiago Lopes - 60745

#include "table.h"
#include "table-private.h"
#include "list.h"
#include "list-private.h"
#include "entry.h"
#include "stats.h"
#include "htmessages.pb-c.h"
#include "server_skeleton.h"
#include "server_hashtable.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

struct statistics_t* stats;

int n_readers = 0;
int n_writers = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// Função de controlo de entrada para leitura
void start_read(){
    pthread_mutex_lock(&mutex);
    while(n_writers > 0){
        pthread_cond_wait(&cond, &mutex);
    }
    n_readers++;
    pthread_mutex_unlock(&mutex);
}
// Função de controlo de saída para leitura
void end_read(){
    pthread_mutex_lock(&mutex);
    n_readers--;
    if(n_readers == 0)
        pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);
}

// Função de controlo de entrada para escrita
void start_write(){
    pthread_mutex_lock(&mutex);
    while(n_readers > 0 || n_writers > 0){
        pthread_cond_wait(&cond, &mutex);
    }
    n_writers++;
    pthread_mutex_unlock(&mutex);
}
// Função de controlo de saída para escrita
void end_write(){
    pthread_mutex_lock(&mutex);
    n_writers--;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);
}


/* Inicia o skeleton da tabela. * O main() do servidor deve chamar esta função antes de poder usar a * função invoke(). O parâmetro n_lists define o número de listas a
* serem usadas pela tabela mantida no servidor.
* Retorna a tabela criada ou NULL em caso de erro. */ 
struct table_t *server_skeleton_init(int n_lists) {
    stats = stats_create();
    return table_create(n_lists);
}

/* Liberta toda a memória ocupada pela tabela e todos os recursos
* e outros recursos usados pelo skeleton.
* Retorna 0 (OK) ou -1 em caso de erro. */
int server_skeleton_destroy(struct table_t *table) {
    stats_destroy(stats);
    return table_destroy(table);
}

/* Executa na tabela table a operação indicada pelo opcode contido em msg
* e utiliza a mesma estrutura MessageT para devolver o resultado. * Retorna 0 (OK) ou -1 em caso de erro. */ 
int invoke(MessageT *msg, struct table_t *table) {
    if(msg == NULL || table == NULL)
        return -1;

    struct timeval start, end;
    long interval;

    struct rtable_t* next_server = get_next_server();

    switch (msg->opcode) {
        case MESSAGE_T__OPCODE__OP_PUT:
            gettimeofday(&start, NULL);

            start_write();
            struct block_t* dados = block_create(msg->entry->value.len, msg->entry->value.data);
            int res = table_put(table, msg->entry->key, dados);

            if(next_server != NULL)
                rtable_put(next_server, entry_create(msg->entry->key, dados));
            end_write();

            free(dados);

            if(res == -1){
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }
            msg->opcode = MESSAGE_T__OPCODE__OP_PUT+1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE; 

            gettimeofday(&end, NULL);
            interval = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
            update_operations(stats, interval);

            return 0;
        break;

        case MESSAGE_T__OPCODE__OP_GET:
            gettimeofday(&start, NULL);

            start_read();
            struct block_t* block = table_get(table, msg->key);
            end_read();

            if(block == NULL){
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                block_destroy(block);
                return 0;
            }
            msg->opcode = MESSAGE_T__OPCODE__OP_GET+1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;

            msg->value.data = malloc(block->datasize); 
            if(msg->value.data == NULL){
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                block_destroy(block);
                return -1;
            }
            memcpy(msg->value.data, block->data, block->datasize); 
            msg->value.len = block->datasize;

            block_destroy(block);

            gettimeofday(&end, NULL);
            interval = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
            update_operations(stats, interval);

            return 0;
        break;

        case MESSAGE_T__OPCODE__OP_DEL:
            gettimeofday(&start, NULL);

            start_write();
            int result = table_remove(table, msg->key);
            if(next_server != NULL)
                rtable_del(next_server, msg->key);
            end_write();

            if(result != 0){
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return 0;
            }
            msg->opcode = MESSAGE_T__OPCODE__OP_DEL+1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

            gettimeofday(&end, NULL);
            interval = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
            update_operations(stats, interval);

            return 0;
        break;

        case MESSAGE_T__OPCODE__OP_SIZE:
            gettimeofday(&start, NULL);

            start_read();
            int size = table_size(table);
            end_read();
            
            if(size == -1){
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }
            msg->opcode = MESSAGE_T__OPCODE__OP_SIZE+1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            msg->result = size;

            gettimeofday(&end, NULL);
            interval = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
            update_operations(stats, interval);

            return 0;
        break;

        case MESSAGE_T__OPCODE__OP_GETKEYS:
            gettimeofday(&start, NULL);

            start_read();
            msg->keys = table_get_keys(table);

            if(msg->keys == NULL){
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                end_read();
                return -1;
            }
            msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS+1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;
            
            msg->n_keys = table_size(table);
            end_read();

            gettimeofday(&end, NULL);
            interval = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
            update_operations(stats, interval);

            return 0;
        break;

        case MESSAGE_T__OPCODE__OP_GETTABLE:
            gettimeofday(&start, NULL);

            start_read();
            msg->entries = (EntryT**) malloc(sizeof(EntryT*) * (table_size(table)+1));
            if(msg->entries == NULL){
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                end_read();
                return -1;
            }

            int idx = 0;
            for(int i = 0; i < table->size; i++){
                struct list_t* listaAtual = table->lists[i];

                //se a posição não está vazia, faz cópia das chaves dessa lista para o vetor
                if(listaAtual != NULL){
                    struct node_t *cur = listaAtual->head;
                    while(cur != NULL) {
                        
                        EntryT *entrada = malloc(sizeof(EntryT));  // Alocar dinamicamente
                        if (entrada == NULL) {
                            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                            return -1;
                        }
                        entry_t__init(entrada);

                        entrada->key = strdup(cur->entry->key);

                        int datasize = cur->entry->value->datasize;
                        entrada->value.data = malloc(datasize);

                        memcpy(entrada->value.data, cur->entry->value->data, datasize);
                        entrada->value.len = datasize;

                        msg->entries[idx] = entrada; 
                        idx++;
                        cur = cur->next;
                    }
                }
            }
            msg->entries[idx] = NULL;

            msg->opcode = MESSAGE_T__OPCODE__OP_GETTABLE+1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_TABLE;
            msg->n_entries = table_size(table);
            end_read();

            gettimeofday(&end, NULL);
            interval = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
            update_operations(stats, interval);

            return 0;
        break;

        case MESSAGE_T__OPCODE__OP_STATS:
            StatisticsT *statistics = malloc(sizeof(StatisticsT));  // Alocar dinamicamente
            if (statistics == NULL) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }
            statistics_t__init(statistics);

            statistics->n_operations = stats->n_operations;
            statistics->total_time = stats->total_time;
            statistics->n_clients = stats->n_clients;

            msg->stats = statistics;

            msg->opcode = MESSAGE_T__OPCODE__OP_STATS+1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_STATS;

            return 0;
        break;

        default:
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return -1;
    }
}

/* Devolve o ponteiro para a estrutura de estatísticas */
struct statistics_t* get_stats(){
    return stats;
}



