// Grupo 49
// Larissa Oliveira - 59830
// Nickolas Ficker - 60810
// Tiago Lopes - 60745

#include <zookeeper/zookeeper.h>

/* ZooKeeper Znode Data Length (1MB, the max supported) */
#define ZDATALEN 1024 * 1024

char* get_node_data(zhandle_t *zh, char* node){ 
    char *data = malloc(ZDATALEN * sizeof(char));
    int data_len = ZDATALEN;

    char* nodePath = malloc(ZDATALEN * sizeof(char));
    snprintf(nodePath, ZDATALEN, "/chain/%s", node);   

    if (ZOK != zoo_get(zh, nodePath, 0, data, &data_len, NULL)){
        fprintf(stderr, "Erro ao obter data de %s!\n", nodePath);
        return NULL;
    }
    free(nodePath);
    return data;
}

zhandle_t* connect_to_zookeeper(const char* zookeeper_address){
    /* Connect to ZooKeeper server */
	zhandle_t* zh = zookeeper_init(zookeeper_address, NULL, 2000, 0, NULL, 0); 
	if (zh == NULL)	{
		fprintf(stderr, "Error connecting to ZooKeeper server!\n");
	    exit(EXIT_FAILURE);
	}
    return zh;
}