#ifndef AUXILIAR_H
#define AUXILIAR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "bt.h"

//* Constantes auxiliares

#define TAMANHO_CABECALHO 17      // 1 byte (char) + 4 ints = 17 bytes; leitura campo a campo evita padding
#define TAMANHO_REGISTRO 80       // tamanho fixo de cada registro na área de dados
#define TAMANHO_CAMPO_VARIAVEL 44 // limite máximo combinado para nomeEstacao e nomeLinha
#define TAMANHO_TEXTO 128         // Máximo de caracteres que um valorTexto dentro de um Criterio pode ter
#define MAX_CRITERIOS 8           // limite de pares (nomeCampo, valorCampo) por operação
#define FLAG_CAMPO_NULO -1        // flag para campos inteiros ausentes no registro

//* Strings de erro

#define MSG_FALHA "Falha no processamento do arquivo."
#define MSG_INEXISTENTE "Registro inexistente."

//* Structs gerais para armazenamento de campos

typedef struct cabecalho
{
    char status;          // '1' = consistente, '0' = inconsistente (arquivo aberto para escrita)
    int topo;             // RRN do topo da pilha de registros removidos; -1 se vazia
    int proxRRN;          // próximo slot disponível na área de dados
    int nroEstacoes;      // quantidade de estações únicas (nome distinto)
    int nroParesEstacoes; // quantidade de pares (codEstacao, codProxEstacao) únicos
} Cabecalho;

typedef struct registro
{
    char removido;  // '0' = ativo, '1' = removido logicamente
    int proximo;    // Quando removido, aponta para o próximo da pilha; -1 se for o último
    int codEstacao; // Chave primária usada pelo índice árvore-B
    int codLinha;
    int codProxEstacao;
    int distProxEstacao;
    int codLinhaIntegra;
    int codEstIntegra;
    int tamNomeEstacao;   // Comprimento do nome da estação em bytes (sem o terminador '\0')
    char nomeEstacao[44]; // Campo variável em que só os primeiros "tamNomeEstacao" bytes são dados válidos
    int tamNomeLinha;     // Comprimento do nome da linha em bytes (sem o terminador '\0')
    char nomeLinha[44];   // Campo variável em que só os primeiros "tamNomeLinha" bytes são dados válidos
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
    char nome[32];        // nome do campo desejado (ex: "codEstacao", "nomeEstacao")
    int ehNulo;           // 1 quando o critério busca por um valor ausente (NULO)
    int valorInteiro;     // valor esperado para campos inteiros
    char valorTexto[128]; // valor esperado para campos de texto
} Criterio;

//* Protótipos de funções de manipulação geral de arquivos binários e RRN

// Lê o cabeçalho do arquivo binário a partir do byte 0. Retorna 1 em sucesso, 0 em falha.
int ler_cabecalho(FILE *arquivo, Cabecalho *cabecalho);

// Escreve o cabeçalho no arquivo binário a partir do byte 0 campo a campo (evitando o padding interno).
void escrever_cabecalho(FILE *arquivo, Cabecalho *cabecalho);

// Converte um RRN para o byte offset correspondente no arquivo.
long rrn_para_offset(int rrn);

// Lê um registro do arquivo. Retorna 1 (lido), -1 (removido, ponteiro avançado) ou 0 (erro/EOF).
int ler_registro(FILE *arquivo, Registro *registro);

// Escreve um registro no arquivo na posição atual do ponteiro. Retorna 1 em sucesso, 0 em falha.
int escrever_registro(FILE *arquivo, Registro *registro);

// Escreve um registro no arquivo de índice. Retorna 1 em sucesso, 0 em falha.
bool escrever_registro_bt(FILE *arquivo_indice, CabecalhoBT *cabecalho_bt, int rrn_no_arquivo_dados, int chave);

// Abre o arquivo binário no modo indicado, lê o cabeçalho e verifica a consistência.
// Se eh_escrita != 0, marca o arquivo como inconsistente (status '0') antes de retornar.
// Retorna 1 em sucesso, 0 em falha.
int abrir_binario(FILE **arquivo, char *nome_arquivo, char *modo, Cabecalho *cabecalho, int eh_escrita);

// Atalho para abrir_binario no modo "r+b" com eh_escrita = 1.
int abrir_binario_escrita(FILE **arquivo, char *nome_arquivo, Cabecalho *cabecalho);

// Marca o arquivo como consistente (status '1'), reescreve o cabeçalho e fecha o arquivo.
void fechar_binario_escrita(FILE *arquivo, Cabecalho *cabecalho);

// Escreve o cabeçalho no arquivo de índice árvore-B a partir do byte 0.
void escrever_cabecalho_bt(FILE *arquivo, CabecalhoBT *cabecalho_bt);

// Lê o cabeçalho do arquivo de índice árvore-B a partir do byte 0.
int ler_cabecalho_bt(FILE *arquivo, CabecalhoBT *cabecalho_bt);

// Abre o arquivo de índice árvore-B, lê o cabeçalho e verifica a consistência.
int abrir_binario_bt(FILE **arquivo, char *nome_arquivo, char *modo, CabecalhoBT *cabecalho_bt, int eh_escrita);

// Atalho para abrir_binario_bt no modo "r+b" com eh_escrita = 1.
int abrir_binario_escrita_bt(FILE **arquivo, char *nome_arquivo, CabecalhoBT *cabecalho_bt);

// Marca o arquivo de índice como consistente, reescreve o cabeçalho e fecha o arquivo.
void fechar_binario_escrita_bt(FILE *arquivo, CabecalhoBT *cabecalho_bt);

// Remove logicamente o registro em 'offset': escreve removido='1' e proximo=cabecalho->topo no disco, depois atualiza cabecalho->topo = RRN do registro.
// Obs: o ponteiro do arquivo fica posicionado após os 5 bytes escritos.
// Retorna 1 em sucesso, 0 se o seek/write falhar.
int remover_registro_logico(FILE *arquivo, Cabecalho *cabecalho, long offset);

//* Protótipos de funções de leitura do .csv e processamento de cabeçalhos / variáveis

// Posiciona o ponteiro após a linha de cabeçalho do CSV e retorna a quantidade de registros.
// Retorna -1 em caso de erro.
int preparar_csv_e_contar_registros(FILE *arquivo_csv);

// Insere o par (codEstacao, codProxEstacao) no vetor se ainda não existir.
// Realoca o vetor se necessário. Retorna 1 (novo par), 0 (já existia) ou -1 (erro de alocação).
int adicionar_par_unico(int codEstacao, int codProxEstacao, ParEstacao **pares, int *quantidade, int *capacidade);

// Recalcula nroEstacoes e nroParesEstacoes percorrendo todos os registros do arquivo.
// Retorna 1 em sucesso, 0 em falha.
int calcular_nroEstacoes_nroParesEstacoes(FILE *arquivo, Cabecalho *cabecalho);

// Inicializa a struct EstacoesVistas com valores nulos/zero.
void inicializar_estacoes_vistas(EstacoesVistas *estacoes);

// Libera a memória alocada para os nomes de estações vistas.
void liberar_estacoes_vistas(EstacoesVistas *estacoes);

// Preenche com '$' os bytes restantes do registro até completar TAMANHO_REGISTRO bytes.
// Retorna true em sucesso, false em falha de escrita.
bool preencher_campos_variaveis_lixo(FILE *arquivo, Registro *registro);

// Verifica se o nome da estação ainda não foi visto; se for novo, armazena-o.
// Retorna true se a estação é nova, false caso contrário.
bool nova_estacao(char *novo_nome, EstacoesVistas *estacoes);

// Lê um registro do CSV e o escreve no arquivo binário, atualizando o cabeçalho.
// Retorna 1 (sucesso), 0 (fim do CSV) ou -1 (erro).
int ler_escrever_registros(FILE *csv, FILE *bin, Cabecalho *cabecalho, Registro *registro_lido);

//* Protótipos de funções de critérios de busca / aplicação (funcionalidades 3, 4, 6)

// Percorre o vetor de criterios e, se encontrar um criterio nao-nulo sobre codEstacao, armazena a chave em *chave e retorna 1. Retorna 0 se não houver criterio de codEstacao.
int criterio_obter_codEstacao(Criterio *criterios, int quantidade, int *chave);

// Le um par (nomeCampo, valorCampo) da entrada padrao e preenche a struct Criterio.
// Retorna 1 em sucesso, 0 em falha.
int ler_criterio(Criterio *criterio);

// Lê "minimo" ou mais critérios da entrada padrão e os armazena no vetor criterios.
// Atualiza *quantidade. Retorna 1 em sucesso, 0 em falha.
int ler_lista_criterios(Criterio *criterios, int *quantidade, int minimo);

// Verifica se o registro atende a todos os critérios fornecidos simultaneamente.
// Retorna 1 se atende, 0 caso contrário.
int registro_atende_criterios(Registro *registro, Criterio *criterios, int quantidade);

// Aplica um critério de atualização ao registro, sobrescrevendo o campo indicado.
void aplicar_criterio_no_registro(Registro *registro, Criterio *criterio);

//* Protótipos de funções de tratamento de strings, campos nulos e impressão

// Imprime o valor inteiro ou "NULO" se for FLAG_CAMPO_NULO, seguido de espaço.
void int_ou_nulo(int valor);

// Converte a string para inteiro. Retorna FLAG_CAMPO_NULO se a string representar nulo.
int inteiro_ou_nulo(char *valor);

// Retorna true se os 'tamanho' bytes a partir de 'valor' indicam campo nulo.
bool campo_nulo(char *valor, int tamanho);

// Retorna true se o nome do campo corresponde a um campo de texto (nomeEstacao ou nomeLinha).
bool campo_eh_texto(char *nome_campo);

// Imprime todos os campos não removidos do registro no formato padrão.
void imprimir_registro(Registro *registro);

// Retorna o valor inteiro do campo identificado por nome_campo.
// Define *eh_valido como 0 se o nome não corresponder a nenhum campo inteiro.
int obter_campos_inteiros(Registro *registro, char *nome_campo, int *eh_valido);

// Retorna o ponteiro para o campo de texto identificado por nome_campo,
// preenchendo *tamanho com seu comprimento. Define *eh_valido como 0 se inválido.
char *obter_campos_textos(Registro *registro, char *nome_campo, int *tamanho, int *eh_valido);

// Adiciona '\0' ao final dos campos de texto variáveis para permitir uso como strings C.
void normalizar_campos_texto_registro(Registro *registro);

//* Protótipos de funções auxiliares fornecidas

// Imprime o conteúdo do arquivo binário na tela (função previamente fornecida).
void BinarioNaTela(char *arquivo);

// Lê uma string delimitada por aspas da entrada padrão (função previamente fornecida).
void ScanQuoteString(char *str);

#endif