// Grupo 49
// Larissa Oliveira - 59830
// Nickolas Ficker - 60810
// Tiago Lopes - 60745

#include "server_skeleton.h"
#include "server_network.h"
#include "client_stub-private.h"
#include "client_stub.h"
#include "server_hashtable.h"
#include "zoo_helper.h"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zookeeper/zookeeper.h>

/* ZooKeeper Znode Data Length (1MB, the max supported) */
#define ZDATALEN 1024 * 1024

static struct table_t *table = NULL;
static int socket_n = -1;

static zhandle_t *zh;
static char* node_id;
static struct rtable_t* next_server = NULL;
static char *root_path = "/chain";

typedef struct String_vector zoo_string; 
static char *watcher_ctx = "ZooKeeper Data Watcher";

// Manipulador de sinais para SIGINT
void handle_sigint() {
    printf("\nRecebido SIGINT (Ctrl+C), finalizando servidor...\n");

    if (table != NULL) {
        server_skeleton_destroy(table);
    }

    if (socket_n != -1) {
        server_network_close(socket_n);
    }
    free(node_id);
    zookeeper_close(zh);
    exit(0); 
}

/**
* Define e se conecta ao servidor sucessor, levando em conta se mudou ou nao
*/
int set_next_server(zoo_string* children_list){
    char* novo_sucessor = NULL;

    for (int i = 0; i < children_list->count; i++)  {
        if(strcmp(children_list->data[i], node_id) > 0){
            if(novo_sucessor == NULL)
                novo_sucessor = children_list->data[i];
            else if(strcmp(children_list->data[i], novo_sucessor) < 0)
                novo_sucessor = children_list->data[i];
        }
    }
    if(novo_sucessor == NULL){
        if(next_server != NULL){
            rtable_disconnect(next_server);
            next_server = NULL;
        }
        return 0;
    }
    char *sucessor_data = get_node_data(zh, novo_sucessor);

    if(next_server != NULL){
        char porto[8]; 
        sprintf(porto, "%d", next_server->server_port);
        char* antigo_sucessor_data = strdup(next_server->server_address);
        strcat(antigo_sucessor_data, porto);

        if(strcmp(sucessor_data, antigo_sucessor_data) == 0){
            free(antigo_sucessor_data);
            return 0;
        }
        rtable_disconnect(next_server);
        free(antigo_sucessor_data);
    }

    next_server = rtable_connect(sucessor_data);
    if(next_server == NULL){
        fprintf(stderr, "Erro ao criar rtable de next_server!\n");
        return -1;
    }
    free(sucessor_data);
    return 0;
}

/**
* Se conecta ao servidor antecessor, caso exista, para obter as entradas da tabela.
*/
int set_antecessor(zoo_string* children_list){
    char* antecessor = NULL;
    for (int i = 0; i < children_list->count; i++)  {       
        if(strcmp(children_list->data[i], node_id) < 0){
            if(antecessor == NULL)
                antecessor = children_list->data[i];
            else if(strcmp(children_list->data[i], antecessor) > 0){
                antecessor = children_list->data[i];
            }
        }
    }

    if(antecessor != NULL){

        struct rtable_t *previous_server;
        char *antecessor_data = get_node_data(zh, antecessor);

        previous_server = rtable_connect(antecessor_data);
        if(previous_server == NULL){
            fprintf(stderr, "Erro ao criar rtable de next_server!\n");
            return -1;
        }
        struct entry_t** entries = rtable_get_table(previous_server);
        if(entries == NULL)
            printf("Erro em rtable_get_table.\n");
        else {
            int idx = 0;
            while(entries[idx] != NULL){
                table_put(table, entries[idx]->key, entries[idx]->value);
                idx++;
            }
            rtable_free_entries(entries);
        }
        rtable_disconnect(previous_server);
        free(antecessor_data);
    }
    return 0;
}

/**
* Data Watcher function for /chain noded 3.
*/
static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx) {
	zoo_string* children_list =	(zoo_string *) malloc(sizeof(zoo_string));
	if (state == ZOO_CONNECTED_STATE){
		if (type == ZOO_CHILD_EVENT){
	 	   /* Get the updated children and reset the watch */ 
 			if (ZOK != zoo_wget_children(wzh, zpath, child_watcher, watcher_ctx, children_list)) {
 				fprintf(stderr, "Error setting watch at %s!\n", zpath);
 			}
			set_next_server(children_list);
		} 
	}
	free(children_list);
}

int main(int argc, char *argv[]) {

    if (argc != 4) return -1;

    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, handle_sigint);

    short porto = (short) atoi(argv[1]);
    int n_lists = atoi(argv[2]);
    socket_n = server_network_init(porto);
    table = server_skeleton_init(n_lists);

    // Parte do zookeeper
    char data[128] = ""; 
    strcat(data, "127.0.0.1:");  // localhost
    strcat(data, argv[1]); 
    zh =connect_to_zookeeper(argv[3]);
    char* node_path = "/chain/node";
    int new_path_len = 1024;
	char* full_node_id = malloc (new_path_len);
    zoo_string* children_list =	(zoo_string *) malloc(sizeof(zoo_string));

        if (ZNONODE == zoo_exists(zh, root_path, 0, NULL)) {
            if (ZOK == zoo_create( zh, root_path, NULL, -1, & ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0)) {
                fprintf(stderr, "%s created!\n", root_path);
            } else {
                fprintf(stderr,"Error Creating %s!\n", root_path);
                exit(EXIT_FAILURE);
            } 
        }
        
        if (ZOK != zoo_create(zh, node_path, data, 128, & ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL | ZOO_SEQUENCE, full_node_id, new_path_len)) {
            fprintf(stderr, "Error creating znode from path %s!\n", node_path);
            exit(EXIT_FAILURE);
        }

        node_id = strdup(full_node_id + 7);

        if (ZOK != zoo_wget_children(zh, root_path, &child_watcher, watcher_ctx, children_list)) {
            fprintf(stderr, "Error setting watch at %s!\n", root_path);
            exit(EXIT_FAILURE);
        }

        set_next_server(children_list);
        set_antecessor(children_list);
        
    
    free(children_list);
    // Acabou parte do zookeeper
    
    printf("Servidor inicializado. Pressione Ctrl+C para encerrar.\n");
    network_main_loop(socket_n, table);
    free(node_id);
    zookeeper_close(zh);
    server_skeleton_destroy(table);
    server_network_close(socket_n);
    return 0;
}

/**
* Devolve o servidor sucessor
*/
struct rtable_t* get_next_server(){
    return next_server;
}
