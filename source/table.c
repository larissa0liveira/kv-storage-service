// Grupo 49
// Larissa Oliveira - 59830
// Nickolas Ficker - 60810
// Tiago Lopes - 60745

#include "../include/table-private.h"
#include "../include/table.h"
#include <stdlib.h>
#include <string.h>

/* Função para criar e inicializar uma nova tabela hash, com n
 * linhas (n = módulo da função hash).
 * Retorna a tabela ou NULL em caso de erro.
 */
struct table_t *table_create(int n){
    if(n <= 0)
        return NULL;

    struct table_t* table = (struct table_t *) malloc(sizeof(struct table_t));
    if(table == NULL)
        return NULL;

    //aloca memoria para o vetor de listas
    table -> arrayLista = (struct list_t **) malloc(n * sizeof(struct list_t*));
    if(table -> arrayLista == NULL)
        return NULL;
    
    //inicializa cada lista a NULL
    for(int i = 0; i < n; i++){
        table -> arrayLista[i] = NULL;
    }

    table -> size = n;
    return table;
}

/* Função hash. Transforma a chave (string) dada em um índice do
 * vetor de tamanho size.
*/
int hashValue(char *key, int size) {
    int soma = 0;
    for (int i = 0; key[i] != '\0'; i++) {
        soma += key[i];  
    }
    return soma % size;
}

/* Função para adicionar um par chave-valor à tabela. Os dados de entrada
 * desta função deverão ser copiados, ou seja, a função vai criar uma nova
 * entry com *CÓPIAS* da key (string) e dos dados. Se a key já existir na
 * tabela, a função tem de substituir a entry existente na tabela pela
 * nova, fazendo a necessária gestão da memória.
 * Retorna 0 (ok) ou -1 em caso de erro.
 */
int table_put(struct table_t *t, char *key, struct block_t *value){
    if(t == NULL || key == NULL || value == NULL)
        return -1;
    
    //aloca memoria para a cópia da chave, tamanho da chave anterior + 1 para caracter nulo
    char* copiaKey = (char *) malloc(strlen(key) + 1);
    if(copiaKey == NULL)
        return -1;

    strcpy(copiaKey, key);

    //cópia profunda do bloco de dados
    struct block_t *copiaValue = block_duplicate(value);
    if(copiaValue == NULL)
        return -1;

    struct entry_t *entrada = entry_create(copiaKey, copiaValue);
    if(entrada == NULL)
        return -1;

    int hash = hashValue(key, t -> size);

    //caso a posição da tabela dada pelo valor hash esteja vazia, cria nova lista
    if(t->arrayLista[hash] == NULL){
        struct list_t* lista = list_create();
        if(lista == NULL)
            return -1;
        list_add(lista, entrada);
        t->arrayLista[hash] = lista;
    }
    else{
        list_add(t->arrayLista[hash], entrada); 
    }
    return 0;
}

/* Função que procura na tabela uma entry com a chave key. 
 * Retorna uma *CÓPIA* dos dados (estrutura block_t) nessa entry ou 
 * NULL se não encontrar a entry ou em caso de erro.
 */
struct block_t *table_get(struct table_t *t, char *key){
    if(t == NULL || key == NULL)
        return NULL;
    
    int hash = hashValue(key, t -> size);

    if(t->arrayLista[hash] == NULL)
        return NULL;

    struct entry_t *entrada = list_get(t->arrayLista[hash], key);
    if(entrada == NULL)
        return NULL;
    
    struct block_t *copiaBloco = block_duplicate(entrada -> value);
    if(copiaBloco == NULL)
        return NULL;

    return copiaBloco;
}

/* Função que conta o número de entries na tabela passada como argumento.
 * Retorna o tamanho da tabela ou -1 em caso de erro.
 */
int table_size(struct table_t *t){
    if(t == NULL)
        return -1;
    
    int totalEntradas = 0;
    for(int i = 0; i < t->size; i++){
        if(t->arrayLista[i] != NULL)
            totalEntradas += list_size(t->arrayLista[i]);
    }
    return totalEntradas;
}

/* Função auxiliar que constrói um array de char* com a cópia de todas as keys na 
 * tabela, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 * Retorna o array de strings ou NULL em caso de erro.
 */
char **table_get_keys(struct table_t *t){
    if(t == NULL)
        return NULL;
    
    //aloca memoria para o vetor de chaves, tamanho de entradas na tabela + 1 para valor NULL
    char **keys = (char **) malloc(sizeof(char*) * (table_size(t) + 1));
    if(keys == NULL)
        return NULL;
    
    int index = 0;
    char **keysToBeCopied = NULL;   //para guardar lista de chaves de cada lista na tabela

    //para cada posição da tabela
    for(int i = 0; i < t->size; i++){
        struct list_t* listaAtual = t->arrayLista[i];

        //se a posição não está vazia, faz cópia das chaves dessa lista para o vetor
        if(listaAtual != NULL){
            keysToBeCopied = list_get_keys(listaAtual);

            for(int j = 0; j < list_size(listaAtual); j++){
                keys[index] = strdup(keysToBeCopied[j]);   //aloca memória e faz cópia profunda da chave
                index++;
            }
            list_free_keys(keysToBeCopied);
        }   
    }
    keys[index] = NULL;
    return keys;
}

/* Função auxiliar que liberta a memória ocupada pelo array de keys obtido pela 
 * função table_get_keys.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int table_free_keys(char **keys){
    return list_free_keys(keys);
}

/* Função que remove da lista a entry com a chave key, libertando a
 * memória ocupada pela entry.
 * Retorna 0 se encontrou e removeu a entry, 1 se não encontrou a entry,
 * ou -1 em caso de erro.
 */
int table_remove(struct table_t *t, char *key){
    if(t == NULL || key == NULL)
        return -1;
    
    int hash = hashValue(key, t->size);
    if(t->arrayLista[hash] == NULL)
        return -1;

    return list_remove(t -> arrayLista[hash], key);
}

/* Função que elimina uma tabela, libertando *toda* a memória utilizada
 * pela tabela.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int table_destroy(struct table_t *t){
    if(t == NULL)
        return -1;
    
    for(int i = 0; i < t -> size; i++){
        if(t -> arrayLista[i] != NULL)
            list_destroy(t -> arrayLista[i]);
    }
    free(t -> arrayLista);
    free(t);

    return 0;
}
