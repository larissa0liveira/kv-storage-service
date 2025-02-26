#ifndef _ZOO_HELPER_H
#define _ZOO_HELPER_H

#include <zookeeper/zookeeper.h>

/**
* Devolve o conteudo de node (ip:porto do servidor)
*/
char* get_node_data(zhandle_t *zh, char* node);

/**
* Se conecta ao zookeeper
*/
zhandle_t* connect_to_zookeeper(const char* zookeeper_address);
#endif