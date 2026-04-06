#ifndef AUXILIAR_H
#define AUXILIAR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Constantes auxiliares 

#define TAMANHO_CABECALHO 17
#define TAMANHO_REGISTRO 80
#define TAMANHO_CAMPO_VARIAVEL 44
#define TAMANHO_TEXTO 128

// Limite máximo de pares (nomeCampo, valorCampo) que podem ser informados como critérios por vez
#define MAX_CRITERIOS 8

#define FLAG_CAMPO_NULO -1

// Strings de erro 

#define MSG_FALHA "Falha no processamento do arquivo."
#define MSG_INEXISTENTE "Registro inexistente."

// Structs gerais para armazenamento de campos

typedef struct cabecalho
{
    char status;
    int topo;
    int proxRRN;
    int nroEstacoes;
    int nroParesEstacoes;
} Cabecalho;

typedef struct registro
{
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

typedef struct estacoesVistas
{
    char **nomes;
    int quantidade;
    int capacidade;
} EstacoesVistas;

typedef struct parEstacao
{
    int codEstacao;
    int codProxEstacao;
} ParEstacao;

typedef struct criterio
{
    char nome[32];
    int ehNulo;
    int valorInteiro;
    char valorTexto[TAMANHO_TEXTO];
} Criterio;

// Protótipos de funções de manipulação geral de arquivos binários e RRN

int ler_cabecalho(FILE *arquivo, Cabecalho *cabecalho);
void escrever_cabecalho(FILE *arquivo, Cabecalho *cabecalho);
long rrn_para_offset(int rrn);
int ler_registro(FILE *arquivo, Registro *registro);
int escrever_registro(FILE *arquivo, Registro *registro);

int abrir_binario(FILE **arquivo, char *nome_arquivo, char *modo, Cabecalho *cabecalho, int eh_escrita);
int abrir_binario_escrita(FILE **arquivo, char *nome_arquivo, Cabecalho *cabecalho);
void fechar_binario_escrita(FILE *arquivo, Cabecalho *cabecalho);

// Protótipos de funções de leitura do .csv e processamento de cabeçalhos / variáveis

int preparar_csv_e_contar_registros(FILE *arquivo_csv);
int adicionar_par_unico(int codEstacao, int codProxEstacao, ParEstacao **pares, int *quantidade, int *capacidade);
int calcular_nroEstacoes_nroParesEstacoes(FILE *arquivo, Cabecalho *cabecalho);
void inicializar_estacoes_vistas(EstacoesVistas *estacoes);
void liberar_estacoes_vistas(EstacoesVistas *estacoes);
bool preencher_campos_variaveis_lixo(FILE *arquivo, Registro *registro);
bool nova_estacao(char *novo_nome, EstacoesVistas *estacoes);
int ler_escrever_registros(FILE *csv, FILE *bin, Cabecalho *cabecalho, Registro *registro_lido);

// Protótipos de funções de critérios de busca / aplicação (funcionalidades 3, 4, 6)

int ler_criterio(Criterio *criterio);
int ler_lista_criterios(Criterio *criterios, int *quantidade, int minimo);
int registro_atende_criterios(Registro *registro, Criterio *criterios, int quantidade);
void aplicar_criterio_no_registro(Registro *registro, Criterio *criterio);

// Protótipos de funções de tratamento de strings, campos nulos e impressão

void int_ou_nulo(int valor);
int inteiro_ou_nulo(char *valor);
bool campo_nulo(char *valor, int tamanho);
bool campo_eh_texto(char *nome_campo);

void imprimir_registro(Registro *registro);
int obter_campos_inteiros(Registro *registro, char *nome_campo, int *eh_valido);
char *obter_campos_textos(Registro *registro, char *nome_campo, int *tamanho, int *eh_valido);
void normalizar_campos_texto_registro(Registro *registro);

// Protótipos de funções auxiliares fornecidas

void BinarioNaTela(char *arquivo);
void ScanQuoteString(char *str);

#endif