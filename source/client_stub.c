// Grupo 49
// Larissa Oliveira - 59830
// Nickolas Ficker - 60810
// Tiago Lopes - 60745

#include "client_stub.h"
#include "client_stub-private.h"
#include "htmessages.pb-c.h"
#include "client_network.h"
#include "stats.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

/* Função para estabelecer uma associação entre o cliente e o servidor,
* em que address_port é uma string no formato <hostname>:<port>.
* Retorna a estrutura rtable preenchida, ou NULL em caso de erro.
*/
struct rtable_t *rtable_connect(char *address_port){
    if (address_port == NULL){
        return NULL;
    }

    char *hostName = strtok(address_port, ":");
    char *portoString = strtok(NULL, ":");
    if (hostName == NULL || portoString == NULL) {
        return NULL; 
    }
    int porto = atoi(portoString);
    if (porto <= 0){
        return NULL;
    }

    struct rtable_t *tabela = (struct rtable_t*) malloc(sizeof(struct rtable_t));
    if (tabela == NULL){
        return NULL;
    }

    tabela -> server_address = strdup(hostName);
    tabela -> server_port = porto;

    if (network_connect(tabela) == -1){
        free(tabela->server_address);
        free(tabela);
        return NULL;
    }

    return tabela;
}

/* Termina a associação entre o cliente e o servidor, fechando a
* ligação com o servidor e libertando toda a memória local. * Retorna 0 se tudo correr bem, ou -1 em caso de erro. */ 
int rtable_disconnect(struct rtable_t *rtable){
    if(rtable == NULL){
        return -1;
    }

    if(network_close(rtable) == -1){
        return -1;
    }

    free(rtable -> server_address);
    free(rtable);

    return 0;
}

/* Função para adicionar uma entrada na tabela.
* Se a key já existe, vai substituir essa entrada pelos novos dados.
* Retorna 0 (OK, em adição/substituição), ou -1 (erro).
*/
int rtable_put(struct rtable_t *rtable, struct entry_t *entry){
    if(rtable == NULL || entry == NULL || entry -> key == NULL){
        return -1;
    }

    EntryT entrada = ENTRY_T__INIT;
    entrada.key = entry->key;
    entrada.value.data = entry -> value -> data;
    entrada.value.len = entry -> value -> datasize;

    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_PUT;
    msg.c_type = MESSAGE_T__C_TYPE__CT_ENTRY;
    msg.entry = &entrada;

    MessageT *receivedMsg = network_send_receive(rtable, &msg);
    if(receivedMsg == NULL){
        return -1;
    }

    if(receivedMsg -> opcode != MESSAGE_T__OPCODE__OP_PUT+1){
        message_t__free_unpacked(receivedMsg, NULL);
        return -1;
    }
    message_t__free_unpacked(receivedMsg, NULL);
    return 0;
}

/* Retorna a entrada da tabela com chave key, ou NULL caso não exista
* ou se ocorrer algum erro.
*/
struct block_t *rtable_get(struct rtable_t *rtable, char *key){
    
    if(rtable == NULL || key == NULL){
        return NULL;
    }

    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_GET;
    msg.c_type = MESSAGE_T__C_TYPE__CT_KEY;
    msg.key = key;

    MessageT *receivedMsg = network_send_receive(rtable, &msg);
    if(receivedMsg == NULL){
        return NULL;
    }

    if(receivedMsg -> opcode != MESSAGE_T__OPCODE__OP_GET+1){
        message_t__free_unpacked(receivedMsg, NULL);
        return NULL;
    }

    void* buf = malloc(receivedMsg->value.len);
    memcpy(buf, receivedMsg->value.data, receivedMsg->value.len);

    struct block_t* value = block_create(receivedMsg->value.len, buf);
    message_t__free_unpacked(receivedMsg, NULL); 
    return value;
}

/* Função para remover um elemento da tabela. Vai libertar
* toda a memoria alocada na respetiva operação rtable_put().
* Retorna 0 (OK), ou -1 (chave não encontrada ou erro).
*/
int rtable_del(struct rtable_t *rtable, char *key){
    if(rtable == NULL || key == NULL){
        return -1;
    }

    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_DEL;
    msg.c_type = MESSAGE_T__C_TYPE__CT_KEY;
    msg.key = key;

    MessageT *receivedMsg = network_send_receive(rtable, &msg);
    if(receivedMsg == NULL){
        return -1;
    }

    if(receivedMsg -> opcode != MESSAGE_T__OPCODE__OP_DEL+1){
        message_t__free_unpacked(receivedMsg, NULL);
        return -1;
    }

    message_t__free_unpacked(receivedMsg, NULL); 
    return 0;
}

/* Retorna o número de elementos contidos na tabela ou -1 em caso de erro.
*/
int rtable_size(struct rtable_t *rtable){
    if(rtable == NULL){
        return -1;
    }

    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_SIZE;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *receivedMsg = network_send_receive(rtable, &msg);
    if(receivedMsg == NULL){
        return -1;
    }

    if(receivedMsg -> opcode != MESSAGE_T__OPCODE__OP_SIZE+1){
        message_t__free_unpacked(receivedMsg, NULL);
        return -1;
    }

    int size = receivedMsg->result;
    message_t__free_unpacked(receivedMsg, NULL); 
    return size;
}

/* Retorna um array de char* com a cópia de todas as keys da tabela,
* colocando um último elemento do array a NULL.
* Retorna NULL em caso de erro.
*/
char **rtable_get_keys(struct rtable_t *rtable){
    if(rtable == NULL){
        return NULL;
    }

    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *receivedMsg = network_send_receive(rtable, &msg);
    if(receivedMsg == NULL){
        return NULL;
    }

    if(receivedMsg -> opcode != MESSAGE_T__OPCODE__OP_GETKEYS+1){
        message_t__free_unpacked(receivedMsg, NULL);
        return NULL;
    }

    char** keys = (char **) malloc(sizeof(char *) * ((int) receivedMsg->n_keys + 1));
    if(keys == NULL){
        message_t__free_unpacked(receivedMsg, NULL); 
        return NULL;
    }

    int idx;
    for(idx = 0; idx < (int) receivedMsg->n_keys; idx++) {
        keys[idx] = strdup(receivedMsg->keys[idx]);
    }
    keys[idx] = NULL;

    message_t__free_unpacked(receivedMsg, NULL); //libertar receivedMsg->keys ??????
    return keys;
}

/* Liberta a memória alocada por rtable_get_keys().
*/
void rtable_free_keys(char **keys){
    if(keys == NULL)
        return;

    int idx = 0;
    while(keys[idx] != NULL) {
        free(keys[idx]);
        idx++;
    }
    free(keys);
}

/* Retorna um array de entry_t* com todo o conteúdo da tabela, colocando
* um último elemento do array a NULL. Retorna NULL em caso de erro.
*/
struct entry_t **rtable_get_table(struct rtable_t *rtable){
    if(rtable == NULL){
        return NULL;
    }

    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_GETTABLE;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *receivedMsg = network_send_receive(rtable, &msg);
    if(receivedMsg == NULL){
        return NULL;
    }

    if(receivedMsg -> opcode != MESSAGE_T__OPCODE__OP_GETTABLE+1){
        message_t__free_unpacked(receivedMsg, NULL);
        return NULL;
    }
    
    struct entry_t** table = (struct entry_t **) malloc(sizeof(struct entry_t *) * (receivedMsg->n_entries+1));
    if(table == NULL){
        message_t__free_unpacked(receivedMsg, NULL); 
        return NULL;
    }

    int i;
    for(i = 0; i < (int) receivedMsg->n_entries; i++) {
        struct block_t* block = block_create(receivedMsg->entries[i]->value.len, receivedMsg->entries[i]->value.data);
        struct entry_t* entry = entry_create(receivedMsg->entries[i]->key, block);
        table[i] = entry_duplicate(entry);
        free (block);
        free (entry);
    }
    table[i] = NULL;

    message_t__free_unpacked(receivedMsg, NULL); 
    return table;
}

/* Liberta a memória alocada por rtable_get_table().
*/
void rtable_free_entries(struct entry_t **entries){
    if(entries == NULL)
        return;

    int idx = 0;
    while(entries[idx] != NULL) {
        entry_destroy(entries[idx]);
        idx++;
    }
    free(entries);
}

/* Obtém as estatísticas do servidor. */
struct statistics_t *rtable_stats(struct rtable_t *rtable){
    if(rtable == NULL){
        return NULL;
    }
    
    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_STATS;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *receivedMsg = network_send_receive(rtable, &msg);
    if(receivedMsg == NULL){
        return NULL;
    }

    if(receivedMsg -> opcode != MESSAGE_T__OPCODE__OP_STATS+1){
        printf("erro no opcode");
        message_t__free_unpacked(receivedMsg, NULL);
        return NULL;
    }

    struct statistics_t* stats = stats_create();
    stats->n_operations = receivedMsg->stats->n_operations;
    stats->total_time = receivedMsg->stats->total_time;
    stats->n_clients = receivedMsg->stats->n_clients;

    message_t__free_unpacked(receivedMsg, NULL);

    return stats;
}
