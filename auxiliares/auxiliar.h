#ifndef AUXILIAR_H
#define AUXILIAR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define TAMANHO_CABECALHO 17
#define TAMANHO_REGISTRO 80
#define TAMANHO_CAMPO_VARIAVEL 44
#define TAMANHO_TEXTO 128

// Limite máximo de pares (nomeCampo, valorCampo) que podem ser informados como critérios por vez
#define MAX_CRITERIOS 8

#define FLAG_CAMPO_NULO -1

#define MSG_FALHA "Falha no processamento do arquivo."
#define MSG_INEXISTENTE "Registro inexistente."

typedef struct cabecalho Cabecalho;
typedef struct registro Registro;
typedef struct estacoesVistas EstacoesVistas;

typedef struct {
    char status;
    int topo;
    int proxRRN;
    int nroEstacoes;
    int nroParesEstacoes;
} Cabecalho;

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
    char nomeEstacao[44];
    int tamNomeLinha;
    char nomeLinha[44];
} Registro;

typedef struct {
    char **nomes;
    int quantidade;
    int capacidade;
} EstacoesVistas;

typedef struct {
    char nome[32];
    int ehNulo;
    int valorInteiro;
    char valorTexto[TAMANHO_TEXTO];
} Criterio;

int ler_cabecalho(FILE *arquivo, Cabecalho *cabecalho);
void escrever_cabecalho(FILE *arquivo, Cabecalho *cabecalho);

long rrn_para_offset(int rrn);

int ler_registro(FILE *arquivo, Registro *registro);
int escrever_registro(FILE *arquivo, Registro *registro);
int calcular_nroEstacoes_nroParesEstacoes(FILE *arquivo, Cabecalho *cabecalho);
void inicializar_estacoes_vistas(EstacoesVistas *estacoes);
void liberar_estacoes_vistas(EstacoesVistas *estacoes);
bool nova_estacao(char *novo_nome, EstacoesVistas *estacoes);

bool ler_escrever_registros(FILE *csv, FILE *bin, Cabecalho *cabecalho, EstacoesVistas *estacoes);

void int_ou_nulo(int valor);

bool campo_nulo (char *valor, int tamanho);

bool campo_eh_texto(char *nome_campo);

int ler_criterio(Criterio *criterio);

int registro_atende_criterios(Registro *registro, Criterio *criterios, int quantidade);

void aplicar_criterio_no_registro(Registro *registro, Criterio *criterio);

void imprimir_registro(Registro *registro);

int obter_campos_inteiros(Registro *registro, char *nome_campo, int *eh_valido);

char *obter_campos_textos(Registro *registro, char *nome_campo, int *tamanho, int *eh_valido);

void aplicar_criterio_no_registro(Registro *registro, Criterio *criterio);

void imprimir_registro(Registro *registro);

void BinarioNaTela(char *arquivo);

void ScanQuoteString(char *str);

int abrir_binario(FILE **arquivo, char *nome_arquivo, char *modo, Cabecalho *cabecalho, int eh_escrita);
#endif