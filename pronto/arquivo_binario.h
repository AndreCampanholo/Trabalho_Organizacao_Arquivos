#ifndef ARQUIVO_BINARIO_H
#define ARQUIVO_BINARIO_H

#include <stdio.h>

/* Tamanhos definidos pelo enunciado. */
#define TAMANHO_CABECALHO 17
#define TAMANHO_REGISTRO 80
#define VALOR_NULO_INTEIRO (-1)
#define TAMANHO_TEXTO 256

/*
 * Registro de cabecalho:
 * status, topo, proxRRN, nroEstacoes e nroParesEstacao.
 */
typedef struct {
    char status;
    int topo;
    int proxRRN;
    int nroEstacoes;
    int nroParesEstacao;
} Cabecalho;

/*
 * Estrutura logica do registro de dados. A serializacao em disco e feita
 * campo a campo (sem fwrite de struct completa) para evitar padding.
 */
typedef struct {
    char removido;
    int proximo;
    int codEstacao;
    int codLinha;
    int codProxEstacao;
    int distProxEstacao;
    int codLinhaIntegra;
    int codEstIntegra;
    int tamNomeEstacao;
    char nomeEstacao[TAMANHO_TEXTO];
    int tamNomeLinha;
    char nomeLinha[TAMANHO_TEXTO];
} Registro;

/* Conversao de endereco logico (RRN) para offset absoluto no arquivo. */
long rrn_para_offset(int rrn);

/* Quantidade real de bytes ocupados antes do preenchimento com '$'. */
int tamanho_registro_usado(const Registro *registro);

/* Operacoes de IO do cabecalho. */
void escrever_cabecalho(FILE *arquivo, const Cabecalho *cabecalho);
int ler_cabecalho(FILE *arquivo, Cabecalho *cabecalho);

/* Operacoes de IO dos registros de dados em um RRN especifico. */
void escrever_registro_rrn(FILE *arquivo, int rrn, const Registro *registro);
int ler_registro_rrn(FILE *arquivo, int rrn, Registro *registro);

/*
 * Abre arquivo binario e valida consistencia (status == '1').
 * Se eh_escrita == 1, ja marca status como inconsistente ('0').
 */
int abrir_binario_consistente(FILE **arquivo, const char *nome_arquivo, const char *modo, Cabecalho *cabecalho, int eh_escrita);

#endif
