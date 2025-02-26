// Grupo 49
// Larissa Oliveira - 59830
// Nickolas Ficker - 60810
// Tiago Lopes - 60745

#include "../include/entry.h"
#include <stdlib.h>
#include <string.h>

struct entry_t *entry_create(char *key, struct block_t *value){
    if(key == NULL || value == NULL)
        return NULL;

    struct entry_t* entrada = (struct entry_t*) malloc(sizeof(struct entry_t));
    if(entrada == NULL)
        return NULL;

    entrada -> key = key;
    entrada -> value = value;
    return entrada;
}

/* Função que compara duas entries e retorna a ordem das mesmas, sendo esta
 * ordem definida pela ordem das suas chaves.
 * Retorna 0 se as chaves forem iguais, -1 se e1 < e2,
 * 1 se e1 > e2 ou -2 em caso de erro.
 */
int entry_compare(struct entry_t *e1, struct entry_t *e2){
    if (e1 == NULL || e2 == NULL)
        return -2; 
    
    return strcmp(e1->key, e2->key);
}

/* Função que duplica uma entry, reservando a memória necessária para a
 * nova estrutura.
 * Retorna a nova entry ou NULL em caso de erro.
 */
struct entry_t *entry_duplicate(struct entry_t *e){
    if(e == NULL || e -> key == NULL || e -> value == NULL)
        return NULL;
    
    struct entry_t* novaEntrada = (struct entry_t*) malloc(sizeof(struct entry_t));
    if(novaEntrada == NULL)
        return NULL;

    novaEntrada -> key = (char *) malloc(strlen(e -> key) + 1); 
    if(novaEntrada -> key == NULL)
        return NULL;

    strcpy(novaEntrada -> key, e -> key);

    novaEntrada -> value = block_duplicate(e -> value);
    if (novaEntrada -> value == NULL)
        return NULL; 
    
    return novaEntrada;
}

/* Função que substitui o conteúdo de uma entry, usando a nova chave e
 * o novo valor passados como argumentos, e eliminando a memória ocupada
 * pelos conteúdos antigos da mesma.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int entry_replace(struct entry_t *e, char *new_key, struct block_t *new_value){
    if(e == NULL || new_key == NULL || new_value == NULL)
        return -1;

    free(e -> key);
    block_destroy(e -> value);
    
    e -> key = new_key;
    e -> value = new_value;
    return 0;
}

/* Função que elimina uma entry, libertando a memória por ela ocupada.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int entry_destroy(struct entry_t *e){
    if (e == NULL || e -> key == NULL || e -> value == NULL)
        return -1;

    free(e -> key);
    block_destroy(e -> value);
    free(e);
    return 0;
}