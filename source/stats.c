// Grupo 49
// Larissa Oliveira - 59830
// Nickolas Ficker - 60810
// Tiago Lopes - 60745

#include "stats.h"
#include <stdlib.h>
#include <stdio.h>

/* Função para criar e inicializar uma nova estrutura de estatísticas.
 * Retorna a struct ou NULL em caso de erro. */
struct statistics_t* stats_create(){
    struct statistics_t* stats = (struct statistics_t*) malloc(sizeof(struct statistics_t));
    if(stats == NULL){
        perror("Erro ao alocar stats.\n");
        return NULL;
    }
    stats->n_operations = 0;
    stats->total_time = 0;
    stats->n_clients = 0;
    pthread_mutex_init(&stats->m_stats, NULL);
    return stats;
}

/* Função que elimina a estrutura de estatísticas dada, 
 * libertando a memória utilizada. */
void stats_destroy(struct statistics_t* stats){
    if(stats == NULL){
        perror("Erro ao destruir stats.\n");
        return;
    }
    pthread_mutex_destroy(&stats->m_stats);
    free(stats);
}

/* Função que atualiza os campos n_operations e total_time 
 * da estrutura de estatísticas dada. Nomeadamente, 
 * incrementa o número de operações, e soma o tempo dado 
 * ao tempo total das operações. */
void update_operations(struct statistics_t* stats, int time){
    if(stats == NULL){
        perror("Erro ao atualizar stats operations.\n");
        return;
    }
    pthread_mutex_lock(&stats->m_stats);
    stats->n_operations++;
    stats->total_time += time;
    pthread_mutex_unlock(&stats->m_stats);
}

/* Função que incrementa o campo n_clients 
 * da estrutura de estatísticas dada. */
void update_clients_connect(struct statistics_t* stats){
    if(stats == NULL){
        perror("Erro ao atualizar stats client.\n");
        return;
    }
    pthread_mutex_lock(&stats->m_stats);
    stats->n_clients++;
    pthread_mutex_unlock(&stats->m_stats);
}

/* Função que decrementa o campo n_clients 
 * da estrutura de estatísticas dada. */
void update_clients_disconnect(struct statistics_t* stats){
    if(stats == NULL){
        perror("Erro ao atualizar stats operations.\n");
        return;
    }
    pthread_mutex_lock(&stats->m_stats);
    stats->n_clients--;
    pthread_mutex_unlock(&stats->m_stats);
}

