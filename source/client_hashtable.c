// Grupo 49
// Larissa Oliveira - 59830
// Nickolas Ficker - 60810
// Tiago Lopes - 60745

#include "client_stub.h"
#include "client_stub-private.h"
#include "stats.h"
#include "zoo_helper.h"
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <zookeeper/zookeeper.h>

#define MAX_MSG 2048 

/* ZooKeeper Znode Data Length (1MB, the max supported) */
#define ZDATALEN 1024 * 1024

static zhandle_t *zh;
static char *root_path = "/chain";
typedef struct String_vector zoo_string; 
static char *watcher_ctx = "ZooKeeper Data Watcher";

static struct rtable_t* head = NULL;
static struct rtable_t* tail = NULL;

/**
* Conecta o cliente ao servidor node
*/
struct rtable_t* connect_node(char* node){
    char *data = get_node_data(zh, node);
    struct rtable_t* table = rtable_connect(data);
    if(table == NULL){
        fprintf(stderr, "Erro ao criar rtable de next_server!\n");
        return NULL;
    }
    free(data);
    return table;
}

/**
* Checa se o servidor guardado em rtable e o servidor de node sao diferentes (saber se head ou tail mudaram)
*/
int alt_check(struct rtable_t* rtable, char* node){
    if(rtable != NULL){
        char porto[8];
        sprintf(porto, "%d", rtable->server_port);
        char* old_data = strdup(rtable->server_address);
        strcat(old_data, porto);

        char *novo_data = get_node_data(zh, node);

        if(strcmp(novo_data, old_data) == 0){
            free(old_data);
            free(novo_data);
            return 0;
        }
        free(old_data);
        free(novo_data);
        rtable_disconnect(rtable);
        return 1;
    }
    return 1;
}

/**
* Define os servidores head e tail aos quais o cliente vai se conectar
*/
int set_head_tail(zoo_string* children_list){
    char* max = children_list->data[0];
    char* min = children_list->data[0];
    for (int i = 1; i < children_list->count; i++)  {
        if(strcmp(children_list->data[i], max) > 0){
            max = children_list->data[i];
        }
        if(strcmp(children_list->data[i], min) < 0){
            min = children_list->data[i];
        }
    }
    if(alt_check(head, max) == 1){
        char* minPath = strdup(min);
        head = connect_node(minPath);
        free(minPath);
    }
    if(alt_check(tail, min) == 1){
        char* maxPath = strdup(max);
        tail = connect_node(maxPath);
        free(maxPath);
    }
    if(head == NULL || tail == NULL)
        return -1;

    return 0;
}

/**
* Data Watcher function for /chain node
*/
static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx) {
	zoo_string* children_list =	(zoo_string *) malloc(sizeof(zoo_string));
	if (state == ZOO_CONNECTED_STATE){
		if (type == ZOO_CHILD_EVENT){
	 	   /* Get the updated children and reset the watch */ 
 			if (ZOK != zoo_wget_children(wzh, zpath, child_watcher, watcher_ctx, children_list)) {
 				fprintf(stderr, "Error setting watch at %s!\n", zpath);
 			}
			set_head_tail(children_list);
		} 
	}
	free(children_list);
}

void print_usage() {
    printf("Comando inválido.\n");
    printf("Utilização: p[ut] <key> <value> | g[et] <key> | d[el] <key> | s[ize] | [get]k[eys] | [get]t[able] | st[ats] | q[uit]\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) return -1;

    zh = connect_to_zookeeper(argv[1]);

    zoo_string* children_list =	(zoo_string *) malloc(sizeof(zoo_string));
    
    if (ZOK != zoo_wget_children(zh, root_path, &child_watcher, watcher_ctx, children_list)) {
        fprintf(stderr, "Error setting watch at %s!\n", root_path);
        exit(EXIT_FAILURE);
    }
    set_head_tail(children_list);
    free(children_list);
    char input[MAX_MSG];

    signal(SIGPIPE, SIG_IGN);

    while(1){

        printf("Comando: ");
        if(fgets(input, MAX_MSG, stdin) == NULL){
            print_usage(); 
            continue;
        }
        input[strcspn(input, "\n")] = '\0';

        char* op = strtok(input, " ");
        if(op == NULL){
            print_usage();
            continue;
        }
        
        if(strcmp(op, "p") == 0 || strcmp(op, "put") == 0){
            char* key = strtok(NULL, " ");
            char* value = strtok(NULL, "");
            if(key == NULL || value == NULL){
                print_usage();
                continue;
            }
            struct block_t* block = block_create(strlen(value), value);
            struct entry_t* entry = entry_create(key, block);
            if(rtable_put(head, entry) < 0){
                printf("Erro em rtable_put.\n");
            } else {
                printf("Entrada inserida com sucesso.\n");
            }
            printf("\n");
            free(block);
            free(entry);

        } else if(strcmp(op, "g") == 0 || strcmp(op, "get") == 0){
            char* key = strtok(NULL, " ");

            if(key == NULL){
                print_usage();
                continue;
            }
            struct block_t* block = rtable_get(tail, key);
            if(block == NULL){
                printf("Erro em rtable_get ou chave não encontrada.\n");
            } else {
                fwrite(block->data, 1, block->datasize, stdout);
                printf("\n");
                block_destroy(block);
            }
            printf("\n");

        } else if(strcmp(op, "d") == 0 || strcmp(op, "del") == 0){
            char* key = strtok(NULL, " ");

            if(key == NULL){
                print_usage();
                continue;
            }
            if(rtable_del(head, key) < 0){
                printf("Erro em rtable_del ou chave não encontrada.\n");
            } else {
                printf("Entrada deletada com sucesso.\n");
            }
            printf("\n");
            
        } else if(strcmp(op, "s") == 0 || strcmp(op, "size") == 0){
            int size = rtable_size(tail);
            if(size < 0) {
                printf("Erro em rtable_size.\n");
            } else {
                printf("Tamanho da tabela: %d\n", size);
            }
            printf("\n");
            
        } else if(strcmp(op, "k") == 0 || strcmp(op, "getkeys") == 0){
            char** keys = rtable_get_keys(tail);
            if(keys == NULL)
                printf("Erro em rtable_get_keys.\n");
            else {
                int idx = 0;
                while(keys[idx] != NULL){
                    printf("%s\n", keys[idx]);
                    idx++;
                }
                rtable_free_keys(keys);
            }
            printf("\n");
            
        } else if(strcmp(op, "t") == 0 || strcmp(op, "gettable") == 0){
            struct entry_t** entries = rtable_get_table(tail);
            if(entries == NULL)
                printf("Erro em rtable_get_table.\n");
            else {
                int idx = 0;
                while(entries[idx] != NULL){
                    printf("%s :: ", entries[idx]->key);
                    fwrite(entries[idx]->value->data, 1, entries[idx]->value->datasize, stdout);
                    printf("\n");
                    idx++;
                }
                rtable_free_entries(entries);
            }
            printf("\n");
            
        } else if(strcmp(op, "st") == 0 || strcmp(op, "stats") == 0){
            struct statistics_t* stats = rtable_stats(tail);
            if(stats == NULL){
                printf("Erro em rtable_stats.\n");
            } else {
                printf("Número total de operações: %d\n", stats->n_operations);
                printf("Tempo total gasto nas operações: %ld microssegundos\n", stats->total_time);
                printf("Número de clientes ligados: %d\n", stats->n_clients);
                printf("\n");
                stats_destroy(stats);
            }

        } else if(strcmp(op, "q") == 0 || strcmp(op, "quit") == 0){
            printf("Tchau!\n");
            rtable_disconnect(head);
            rtable_disconnect(tail);
            zookeeper_close(zh);
            return 0;

        } else {
            print_usage();
        }
    }
}
