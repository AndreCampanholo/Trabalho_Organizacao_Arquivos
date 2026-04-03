#ifndef CRITERIOS_H
#define CRITERIOS_H

#include "arquivo_binario.h"

/* Limite maximo de pares (campo, valor) por consulta/atualizacao. */
#define MAX_CRITERIOS 8

/* Representa um filtro informado pelo usuario. */
typedef struct {
    char nome[32];
    int ehNulo;
    int valorInteiro;
    char valorTexto[TAMANHO_TEXTO];
} Criterio;

/* Classificacao de tipo para interpretar comparacao corretamente. */
int campo_eh_texto(const char *nome_campo);

/* Le um par (nomeCampo, valorCampo) da entrada padrao. */
int ler_criterio(Criterio *criterio);

/* Avalia se um registro satisfaz todos os criterios (AND logico). */
int registro_atende_criterios(const Registro *registro, const Criterio *criterios, int quantidade);

/* Aplica um criterio como atribuicao de campo em operacoes de update. */
void aplicar_criterio_no_registro(Registro *registro, const Criterio *criterio);

/* Imprime no formato exigido (NULO para campos ausentes). */
void imprimir_registro(const Registro *registro);

#endif
