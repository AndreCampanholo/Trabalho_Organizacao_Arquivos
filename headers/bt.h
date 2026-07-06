#ifndef BT_H
#define BT_H

#include <stdio.h>
#include <stdbool.h>

#define ORDEM 4                     // ordem da árvore-B: cada nó tem no máximo ORDEM filhos e ORDEM-1 chaves
#define CHAVES_MAX (ORDEM - 1)      // número máximo de chaves por nó (3)
#define CHAVES_MIN (CHAVES_MAX / 2) // número mínimo de chaves por nó não-raiz (1); define a condição de underflow
#define TAM_NO 53                   // tamanho fixo em bytes de cada nó (página) no arquivo de índice
#define NULO -1                     // valor que representa um ponteiro(RRN) nulo na árvore-B

// Cabeçalho do arquivo de índice árvore-B (17 bytes)
// OBS: Possui os mesmos campos do arquivo de dados
typedef struct btCabecalho
{
    char status; // '1' = consistente, '0' = inconsistente (qunado é aberto para escrita ou é interrompido)
    int noRaiz;  // RRN do nó raiz; NULO (-1) quando a árvore está vazia
    int topo;    // RRN do topo da pilha de páginas logicamente removidas; NULO se vazia
    int proxRRN; // próximo RRN disponível para alocação de um novo nó
    int nroNos;  // quantidade de nós ativos (não removidos) na árvore
} CabecalhoBT;

// Nó (página) do arquivo de índice (53 bytes)
typedef struct no
{
    char removido;           // '0' = ativo, '1' = logicamente removido (disponível para reuso)
    int proximo;             // RRN do próximo nó na pilha de removidos; NULO quando não está removido
    int tipoNo;              // tipo do nó: (-1) folha, (0) raiz, (1) intermediário; folha-raiz também usa (-1)
    int nroChaves;           // quantidade de chaves válidas presentes neste nó
    int chaves[CHAVES_MAX];  // chaves de busca (codEstacao) em ordem crescente; NULO em posições vazias
    int offsets[CHAVES_MAX]; // offset em bytes do registro correspondente no arquivo de dados
    int filhos[ORDEM];       // RRNs dos filhos no arquivo de índice; NULO em posições não utilizadas
} NO;

// Insere 'chave' e seu 'offset_registro' no índice árvore-B.
// Realiza o split quando o nó destino estiver cheio, propagando a promoção até a raiz se necessário.
// Retorna true em sucesso, false em falha de leitura ou escrita.
bool inserir_indice(FILE *arq_indice, CabecalhoBT *cab, int chave, int offset_registro);

// Busca 'chave_busca' no índice percorrendo a árvore da raiz até uma folha.
// Retorna o offset em bytes do registro no arquivo de dados, ou NULO se não for encontrado.
int recuperar_registro_indice(FILE *arq_indice, CabecalhoBT *cab, int chave_busca);

// Reserva um RRN para um novo nó: reaproveitando o topo da pilha de removidos se disponível, ou incrementando o proxRRN caso contrário. Também incrementa o "nroNos" e retorna o RRN escolhido.
int bt_alocar_no(FILE *arq_indice, CabecalhoBT *cab);

// Libera o nó de 'rrn', marcando removido='1', encadeia na pilha de removidos e decrementa nroNos.
// Os demais campos do nó permanecem inalterados no disco, conforme a especificação.
void bt_liberar_no(FILE *arq_indice, CabecalhoBT *cab, int rrn);

// Remove 'chave' do índice, corrigindo o underflow via redistribuição ou concatenação.
// Encolhe a árvore se a raiz ficar sem chaves após a remoção.
// Retorna true se a chave existia e foi removida, false caso contrário.
bool remover_registro_indice(FILE *arq_indice, CabecalhoBT *cab, int chave);

#endif