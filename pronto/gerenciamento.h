#ifndef GERENCIAMENTO_H
#define GERENCIAMENTO_H

#include "arquivo_binario.h"

/* Revarre o arquivo para atualizar campos agregados do cabecalho. */
int recalcular_contadores(FILE *arquivo, Cabecalho *cabecalho);

/* Insere RRN removido logicamente no topo da pilha de removidos. */
void empilhar_removido(FILE *arquivo, Cabecalho *cabecalho, int rrn);

/* Remove e retorna o topo da pilha; retorna -1 se pilha vazia. */
int desempilhar_removido(FILE *arquivo, Cabecalho *cabecalho);

#endif
