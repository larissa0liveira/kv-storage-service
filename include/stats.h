#ifndef _STATS_H
#define _STATS_H

#include <pthread.h>

struct statistics_t {
    int n_operations;
    long total_time;
    int n_clients;
    pthread_mutex_t m_stats;
};

/* Função para criar e inicializar uma nova estrutura de estatísticas.
 * Retorna a struct ou NULL em caso de erro. */
struct statistics_t* stats_create();

/* Função que elimina a estrutura de estatísticas dada, 
 * libertando a memória utilizada. */
void stats_destroy(struct statistics_t* stats);

/* Função que atualiza os campos n_operations e total_time 
 * da estrutura de estatísticas dada. Nomeadamente, 
 * incrementa o número de operações, e soma o tempo dado 
 * ao tempo total das operações. */
void update_operations(struct statistics_t* stats, int time);

/* Função que incrementa o campo n_clients 
 * da estrutura de estatísticas dada. */
void update_clients_connect(struct statistics_t* stats);

/* Função que decrementa o campo n_clients 
 * da estrutura de estatísticas dada. */
void update_clients_disconnect(struct statistics_t* stats);

#endif