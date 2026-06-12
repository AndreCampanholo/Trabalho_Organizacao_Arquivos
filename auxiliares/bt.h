#ifndef BT_H
#define BT_H

#include <stdio.h>
#include <stdbool.h>

#define ORDEM 4
#define CHAVES_MAX (ORDEM - 1)
#define CHAVES_MIN (CHAVES_MAX / 2)
#define TAM_NO 53
#define NULO -1

// Cabeçalho do arquivo de índice
typedef struct btCabecalho
{
    char status; // 1 byte  — '0' inconsistente, '1' consistente
    int noRaiz;  // 4 bytes — RRN da raiz, -1 se vazio
    int topo;    // 4 bytes — RRN do topo da pilha de removidos, -1 se vazia
    int proxRRN; // 4 bytes — próximo RRN livre
    int nroNos;  // 4 bytes — quantidade de nós ativos
} CabecalhoBT;

// Nó (página) do índice (53 bytes)
typedef struct no
{
    char removido;          // 1 byte  — '0' ativo, '1' removido
    int proximo;            // 4 bytes — próximo da pilha de removidos (-1 se não removido)
    int tipoNo;             // 4 bytes — (-1) folha, (0) raiz, (1) intermediário
    int nroChaves;          // 4 bytes — quantas chaves estão ocupadas no momento
    int chaves[CHAVES_MAX]; // 4 bytes cada — chaves de busca (codEstacao), -1 se vazio
    int offsets[CHAVES_MAX];   // 4 bytes cada — offset do registro correspondente no arquivo de DADOS
    int filhos[ORDEM];      // 4 bytes cada — RRNs dos filhos no arquivo de ÍNDICE, -1 se nulo
} NO;

bool inserir_indice(FILE *arquivo_indice, CabecalhoBT *cabecalho_bt, int chave, int rrn_registro);
// Usa o arquivo de índice para recuperar o rrn de um registro no arquivo de dados
int recuperar_registro_indice(FILE *arquivo_indice, CabecalhoBT *cabecalho_bt, int chave_busca);

#endif