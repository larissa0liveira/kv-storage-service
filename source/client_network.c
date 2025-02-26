// Grupo 49
// Larissa Oliveira - 59830
// Nickolas Ficker - 60810
// Tiago Lopes - 60745

#include "client_network.h"
#include "client_stub-private.h"
#include "htmessages.pb-c.h"
#include "message-private.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

/* Esta função deve:
* - Obter o endereço do servidor (struct sockaddr_in) com base na
* informação guardada na estrutura rtable;
* - Estabelecer a ligação com o servidor;
* - Guardar toda a informação necessária (e.g., descritor do socket)
* na estrutura rtable;
* - Retornar 0 (OK) ou -1 (erro).
*/
int network_connect(struct rtable_t *rtable){
    if(rtable == NULL){
        return -1;
    }

    int sockfd;
    struct sockaddr_in server;

    // Cria socket TCP
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erro ao criar socket TCP\n");
        return -1;
    }

    // Preenche estrutura server com endereço do servidor para estabelecer
    // conexão
    server.sin_family = AF_INET; // família de endereços
    server.sin_port = htons(rtable -> server_port); // Porta TCP
    if (inet_pton(AF_INET, rtable -> server_address, &server.sin_addr) < 1) { // Endereço IP
        printf("Erro ao converter IP\n");
        close(sockfd);
        return -1;
    }

    // Estabelece conexão com o servidor definido na estrutura server
    if (connect(sockfd,(struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Erro ao conectar-se ao servidor");
        close(sockfd);
        return -1;
    }

    rtable -> sockfd = sockfd;
    return 0;
}

/* Esta função deve:
* - Obter o descritor da ligação (socket) da estrutura rtable_t;
* - Serializar a mensagem contida em msg;
* - Enviar a mensagem serializada para o servidor;
* - Esperar a resposta do servidor;
* - De-serializar a mensagem de resposta;
* - Tratar de forma apropriada erros de comunicação;
* - Retornar a mensagem de-serializada ou NULL em caso de erro.
*/
MessageT *network_send_receive(struct rtable_t *rtable, MessageT *msg){
    if(msg == NULL){
        return NULL;
    }

    unsigned int len = message_t__get_packed_size(msg);
    void * buf = malloc(len);
    if(buf == NULL){
        return NULL;
    }
    message_t__pack(msg,buf);
    int nbytes;
    short tamanhoBuffer = htons(len);

    // Envia tamanho buffer
    if((nbytes = write_all(rtable -> sockfd, &tamanhoBuffer, sizeof(tamanhoBuffer))) != sizeof(tamanhoBuffer)){
        perror("Erro ao enviar dados ao servidor");
        close(rtable -> sockfd);
        return NULL;
    }

    // Envia mensagem serializada
    if((nbytes = write_all(rtable -> sockfd, buf, len)) != (int) len){
        perror("Erro ao enviar dados ao servidor");
        close(rtable -> sockfd);
        return NULL;
    }
    
    // Recebe tamanho da resposta
    if((nbytes = read_all(rtable -> sockfd, &tamanhoBuffer, sizeof(tamanhoBuffer))) != sizeof(tamanhoBuffer)){
        perror("Erro ao receber dados do servidor");
        close(rtable -> sockfd);
        return NULL;
    };

    free(buf);
    len = ntohs(tamanhoBuffer);
    void * readBuf = malloc(len);
    if(readBuf == NULL){
        return NULL;
    }

    // Recebe mensagem serializada
    if((nbytes = read_all(rtable -> sockfd, readBuf, len)) != (int) len){
        perror("Erro ao receber dados do servidor");
        close(rtable -> sockfd);
        return NULL;
    };
   
    MessageT *receivedMsg;
    receivedMsg = message_t__unpack(NULL, len, readBuf);
    if (receivedMsg == NULL) {
        return NULL;
    }

    free(readBuf);
    return receivedMsg;
}

/* Fecha a ligação estabelecida por network_connect().
* Retorna 0 (OK) ou -1 (erro).
*/
int network_close(struct rtable_t *rtable){
    if(rtable == NULL){
        return -1;
    }
    return close(rtable -> sockfd);
}

