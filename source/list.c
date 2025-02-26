// Grupo 49
// Larissa Oliveira - 59830
// Nickolas Ficker - 60810
// Tiago Lopes - 60745

#include "../include/list.h"
#include "../include/list-private.h"
#include <stdlib.h>
#include <string.h>

/* Função que cria e inicializa uma nova lista (estrutura list_t a
 * ser definida pelo grupo no ficheiro list-private.h).
 * Retorna a lista ou NULL em caso de erro.
 */
struct list_t *list_create() {
	struct list_t *l = (struct list_t *) malloc(sizeof(struct list_t));
	if (l == NULL)
		return NULL;
	
    l->head = NULL;
    l->size = 0;

	return l;
}

/* Função que adiciona à lista a entry passada como argumento.
 * A entry é inserida de forma ordenada, tendo por base a comparação
 * de entries feita pela função entry_compare do módulo entry e
 * considerando que a entry menor deve ficar na cabeça da lista.
 * Se já existir uma entry igual (com a mesma chave), a entry
 * já existente na lista será substituída pela nova entry,
 * sendo libertada a memória ocupada pela entry antiga.
 * Retorna 0 se a entry ainda não existia, 1 se já existia e foi
 * substituída, ou -1 em caso de erro.
 */
int list_add(struct list_t *l, struct entry_t *entry) {
    if(l == NULL || entry == NULL)
        return -1;

    struct node_t *cur = l->head; //nó atual
    struct node_t *prev = NULL; //nó anterior

    //enquanto a nova chave não é igual ou maior que a do nó atual, continua a procurar
    while(cur != NULL && entry_compare(cur->entry, entry) < 0) {
        prev = cur;
        cur = prev->next;
    }
    //caso sejam iguais, substitui entrada
    if(cur != NULL && entry_compare(cur->entry, entry) == 0) {
        entry_destroy(cur->entry);
        cur->entry = entry;
        return 1;
    }
    //caso contrario, aloca memoria para o novo nó
    struct node_t *new = (struct node_t *) malloc(sizeof(struct node_t));
    if (new == NULL)
        return -1;

    new->entry = entry;
    new->next = cur;

    if(prev == NULL)
        l->head = new;
    else
        prev->next = new;

    l->size++;
    return 0;
}

/* Função que conta o número de entries na lista passada como argumento.
 * Retorna o tamanho da lista ou -1 em caso de erro.
 */
int list_size(struct list_t *l) {
    return l == NULL? -1 : l->size;
}

/* Função que obtém da lista a entry com a chave key.
 * Retorna a referência da entry na lista ou NULL se não encontrar a
 * entry ou em caso de erro.
*/
struct entry_t *list_get(struct list_t *l, char *key) {
    if(l == NULL || key == NULL)
        return NULL;

    struct node_t *cur = l->head;

    while(cur != NULL) {
        int cmp = strcmp(cur->entry->key, key);
        if(cmp == 0)
            return cur->entry;
        if(cmp > 0)     //lista ordenada, se já estiver em uma key maior, nao tem pq continuar
            break;
        cur = cur->next;
    }
    return NULL;
}

/* Função auxiliar que constrói um array de char* com a cópia de todas as keys na 
 * lista, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 * Retorna o array de strings ou NULL em caso de erro.
 */
char **list_get_keys(struct list_t *l) {
    if(l == NULL || l->head == NULL)
        return NULL;

    //aloca memoria para o vetor, tamanho da lista + 1 para valor NULL
    char **keys = (char **) malloc((l->size + 1) * sizeof(char*));
    if (keys == NULL)
        return NULL;

    struct node_t *cur = l->head;
    int idx = 0;

    while(cur != NULL) {
        keys[idx] = strdup(cur->entry->key); //aloca memoria e faz uma cópia profunda de cada chave
        idx++;
        cur = cur->next;
    }
    keys[idx] = NULL;
    return keys;
}

/* Função auxiliar que liberta a memória ocupada pelo array de keys obtido pela 
 * função list_get_keys.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int list_free_keys(char **keys) {
    if(keys == NULL)
        return -1;

    int idx = 0;
    while(keys[idx] != NULL) {
        free(keys[idx]);
        idx++;
    }
    free(keys);
    return 0;
}

/* Função que elimina da lista a entry com a chave key, libertando a
 * memória ocupada pela entry.
 * Retorna 0 se encontrou e removeu a entry, 1 se não encontrou a entry,
 * ou -1 em caso de erro.
 */
int list_remove(struct list_t *l, char *key) {
    if(l == NULL || key == NULL)
        return -1;

    struct node_t *cur = l->head;
    struct node_t *prev = NULL;

    while(cur != NULL) {
        int cmp = strcmp(cur->entry->key, key);

        if(cmp == 0) {
            entry_destroy(cur->entry);

            if (prev == NULL) 
                l->head = cur->next;
            else 
                prev->next = cur->next;

            free(cur);
            l->size--;
            return 0;
        }
        if(cmp > 0)
            break;  //para de buscar quando encontra uma chave maior que a procurada
        prev = cur;
        cur = cur->next;
    }
    return 1;
}

/* Função que elimina uma lista, libertando *toda* a memória utilizada
 * pela lista (incluindo todas as suas entradas).
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int list_destroy(struct list_t *l) {
    if(l == NULL)
        return -1;

    struct node_t *cur = l->head; //nó usado para percorrer a lista
    struct node_t *temp = NULL; //nó usado para libertar a memória a cada iteração

    while(cur != NULL){
        temp = cur;
        cur = cur->next;
        entry_destroy(temp->entry);
        free(temp);
    }

    free(l);

    return 0;
}