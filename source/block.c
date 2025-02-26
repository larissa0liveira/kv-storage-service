// Grupo 49
// Larissa Oliveira - 59830
// Nickolas Ficker - 60810
// Tiago Lopes - 60745

#include "../include/block.h"
#include <stdlib.h>
#include <string.h>

/* Função que cria um novo bloco de dados block_t e que inicializa 
 * os dados de acordo com os argumentos recebidos, sem necessidade de
 * reservar memória para os dados.	
 * Retorna a nova estrutura ou NULL em caso de erro.
 */
struct block_t *block_create(int size, void *data){

    if (size <= 0 || data == NULL)
        return NULL;  

    struct block_t *bloco = (struct block_t*) malloc(sizeof(struct block_t));
    if (bloco == NULL)
        return NULL; 

    bloco -> datasize = size;
    bloco -> data = data;
    return bloco;
}

/* Função que duplica uma estrutura block_t, reservando a memória
 * necessária para a nova estrutura.
 * Retorna a nova estrutura ou NULL em caso de erro.
 */
struct block_t *block_duplicate(struct block_t *b){

    if (b == NULL)
        return NULL;

    if (b -> datasize <= 0 || b -> data == NULL)
        return NULL;  

    struct block_t *novoBloco = (struct block_t*) malloc(sizeof(struct block_t));
    if (novoBloco == NULL)
        return NULL; 

    int size = b -> datasize;
    novoBloco -> datasize = size;
    novoBloco -> data = malloc(size); 

    if (novoBloco -> data == NULL) {
        free(novoBloco);
        return NULL;
    }

    memcpy(novoBloco->data, b->data, size);

    return novoBloco;
}

/* Função que substitui o conteúdo de um bloco de dados block_t.
 * Deve assegurar que liberta o espaço ocupado pelo conteúdo antigo.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int block_replace(struct block_t *b, int new_size, void *new_data){
    if (b == NULL || new_size <= 0 || new_data == NULL)
        return -1;

    free(b -> data);

    b -> datasize = new_size;
    b -> data = new_data;
    return 0;
}

/* Função que elimina um bloco de dados, apontado pelo parâmetro b,
 * libertando toda a memória por ele ocupada.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int block_destroy(struct block_t *b){
    if (b == NULL)
        return -1;
    
    free(b -> data);
    free(b);
    return 0;
}

