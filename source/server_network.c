// Grupo 49
// Larissa Oliveira - 59830
// Nickolas Ficker - 60810
// Tiago Lopes - 60745

#include "table.h"
#include "htmessages.pb-c.h"
#include "message-private.h"
#include "server_network.h"
#include "server_skeleton.h"
#include "server_skeleton-private.h"
#include "stats.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

/* Função para preparar um socket de receção de pedidos de ligação
* num determinado porto.
* Retorna o descritor do socket ou -1 em caso de erro.
*/
int server_network_init(short port){
    int sockfd;
    // Cria socket TCP
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        perror("Erro ao criar socket");
        return -1;
    }

    int val = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val))) {
        perror("Erro ao configurar SO_REUSEADDR");
        close(sockfd);
        return -1;
    }

    struct sockaddr_in server;
    // Preenche estrutura server com endereço(s) para associar (bind) à socket 
    server.sin_family = AF_INET;
    server.sin_port = htons(port); // Porta TCP
    server.sin_addr.s_addr = htonl(INADDR_ANY); // Todos os endereços na máquina

     // Faz bind
    if (bind(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0){
        perror("Erro ao fazer bind");
        close(sockfd);
        return -1;
    }
    
    // Esta chamada diz ao SO que esta é uma socket para receber pedidos
    if (listen(sockfd, 10) < 0){                   
        perror("Erro ao executar listen");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

struct thread_parameters {
    int connsockfd;
    struct table_t *table;
    int thread_number;
};

/* Função executada pelas threads secundárias */
void* thread_client(void* params){
    struct thread_parameters *tp = (struct thread_parameters *) params;
    int connsockfd = tp->connsockfd;
    struct table_t *table = tp->table;
    int t_number = tp->thread_number;

    printf("Conexão com o cliente estabelecida pela thread %d.\n", t_number);

    struct statistics_t* stats = get_stats();
    update_clients_connect(stats);

    MessageT *message;
    while ((message = network_receive(connsockfd)) != NULL) { // cliente não fechou conexão
        if(invoke(message, table) == -1){   // Executa pedido contido em message
            break;
        }
        if(network_send(connsockfd, message) == -1){ // Envia resposta contida em message
            break;
        }
        message_t__free_unpacked(message, NULL);
    }
    
    update_clients_disconnect(stats);
    printf("Conexão com o cliente fechada pela thread %d.\n", t_number);
    close(connsockfd); // cliente fechou conexão ou ocorreu um erro
    free(tp);
    return 0;
}

/* A função network_main_loop() deve:
* - Aceitar uma conexão de um cliente;
* - Receber uma mensagem usando a função network_receive;
* - Entregar a mensagem de-serializada ao skeleton para ser processada
na tabela table;
* - Esperar a resposta do skeleton;
* - Enviar a resposta ao cliente usando a função network_send.
* A função não deve retornar, a menos que ocorra algum erro. Nesse
* caso retorna -1.
*/
int network_main_loop(int listening_socket, struct table_t *table){
    if(table == NULL){
        return -1;
    }

    int connsockfd;
    struct sockaddr_in client;
    socklen_t size_client = sizeof(client);
    int n_threads = 1;

    printf("Servidor pronto, a esperar por conexões.\n");
    while ((connsockfd = accept(listening_socket, (struct sockaddr *) &client, &size_client)) != -1) {

        struct thread_parameters *thread_p = malloc(sizeof(struct thread_parameters));
        if (thread_p == NULL) {
            perror("Erro ao alocar memória para thread_parameters.");
            close(connsockfd);
            continue;
        }

        pthread_t thread;
        thread_p->connsockfd = connsockfd;
        thread_p->table = table;
        thread_p->thread_number = n_threads;
        n_threads++;

        if (pthread_create(&thread, NULL, &thread_client, (void *) thread_p) != 0){
            perror("\nThread não criada.\n");
            free(thread_p);
            close(connsockfd);
        }
        pthread_detach(thread); 
    }
    return 0;
}

/* A função network_receive() deve:
* - Ler os bytes da rede, a partir do client_socket indicado;
* - De-serializar estes bytes e construir a mensagem com o pedido,
* reservando a memória necessária para a estrutura MessageT.
* Retorna a mensagem com o pedido ou NULL em caso de erro.
*/
MessageT *network_receive(int client_socket){
    short tamanhoBuffer;
    int nbytes, len;

    // Recebe tamanho da mensagem
	if((nbytes = read_all(client_socket, &tamanhoBuffer, sizeof(tamanhoBuffer))) != sizeof(tamanhoBuffer)){
        if(nbytes != 0){
            perror("Erro ao receber dados do cliente");
		    close(client_socket);
        }
		return NULL;
	}

    len = ntohs(tamanhoBuffer);
    void * buf = malloc(len);
    if(buf == NULL){
        return NULL;
    }

    // Recebe mensagem serializada
    if((nbytes = read_all(client_socket, buf, len)) != len){
        if(nbytes != 0){
            perror("Erro ao receber dados do cliente");
		    close(client_socket);
        }
		return NULL;
    };

    MessageT *msg;
    msg = message_t__unpack(NULL, len, buf);
    if (msg == NULL) {
        return NULL;
    }

    free(buf);
    return msg;
}

/* A função network_send() deve:
* - Serializar a mensagem de resposta contida em msg;
* - Enviar a mensagem serializada, através do client_socket.
* Retorna 0 (OK) ou -1 em caso de erro.
*/
int network_send(int client_socket, MessageT *msg){
    if(msg == NULL){
        return -1;
    }
    
    unsigned int len = message_t__get_packed_size(msg);
    void * buf = malloc(len);
    if(buf == NULL){
        return -1;
    }

    message_t__pack(msg,buf);
    int nbytes;
    short tamanhoBuffer = htons(len);

    // Envia tamanho buffer
    if((nbytes = write_all(client_socket, &tamanhoBuffer, sizeof(tamanhoBuffer))) != sizeof(tamanhoBuffer)){
        perror("Erro ao enviar dados ao servidor");
        close(client_socket);
        return -1;
    }

    // Envia mensagem serializada
    if((nbytes = write_all(client_socket, buf, len)) != (int) len){
        perror("Erro ao enviar dados ao servidor");
        close(client_socket);
        return -1;
    }

    free(buf);
    return 0;
}

/* Liberta os recursos alocados por server_network_init(), nomeadamente
* fechando o socket passado como argumento.
* Retorna 0 (OK) ou -1 em caso de erro.
*/
int server_network_close(int socket){
    return close(socket);
}