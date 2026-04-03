#include "gerenciamento.h"

#include <stdlib.h>

/* Vetor dinamico simples para contagem de valores distintos. */
typedef struct {
    int *dados;
    int quantidade;
    int capacidade;
} VetorInteiros;

typedef struct {
    int origem;
    int destino;
} ParEstacao;

typedef struct {
    ParEstacao *dados;
    int quantidade;
    int capacidade;
} VetorPares;

static void vetor_int_iniciar(VetorInteiros *vetor) {
    vetor->dados = NULL;
    vetor->quantidade = 0;
    vetor->capacidade = 0;
}

static int vetor_int_contem(const VetorInteiros *vetor, int valor) {
    for (int i = 0; i < vetor->quantidade; i++) {
        if (vetor->dados[i] == valor) return 1;
    }
    return 0;
}

static void vetor_int_adicionar(VetorInteiros *vetor, int valor) {
    if (vetor->quantidade == vetor->capacidade) {
        vetor->capacidade = vetor->capacidade == 0 ? 16 : vetor->capacidade * 2;
        vetor->dados = (int *)realloc(vetor->dados, (size_t)vetor->capacidade * sizeof(int));
    }
    vetor->dados[vetor->quantidade++] = valor;
}

static void vetor_pares_iniciar(VetorPares *vetor) {
    vetor->dados = NULL;
    vetor->quantidade = 0;
    vetor->capacidade = 0;
}

static int vetor_pares_contem(const VetorPares *vetor, int origem, int destino) {
    for (int i = 0; i < vetor->quantidade; i++) {
        if (vetor->dados[i].origem == origem && vetor->dados[i].destino == destino) return 1;
    }
    return 0;
}

static void vetor_pares_adicionar(VetorPares *vetor, int origem, int destino) {
    if (vetor->quantidade == vetor->capacidade) {
        vetor->capacidade = vetor->capacidade == 0 ? 16 : vetor->capacidade * 2;
        vetor->dados = (ParEstacao *)realloc(vetor->dados, (size_t)vetor->capacidade * sizeof(ParEstacao));
    }
    vetor->dados[vetor->quantidade].origem = origem;
    vetor->dados[vetor->quantidade].destino = destino;
    vetor->quantidade++;
}

int recalcular_contadores(FILE *arquivo, Cabecalho *cabecalho) {
    /*
     * Recontagem completa apos operacoes de escrita para manter
     * consistencia do cabecalho, mesmo em alteracoes multiplo-registro.
     */
    VetorInteiros estacoes;
    VetorPares pares;
    vetor_int_iniciar(&estacoes);
    vetor_pares_iniciar(&pares);

    for (int rrn = 0; rrn < cabecalho->proxRRN; rrn++) {
        Registro registro;
        if (!ler_registro_rrn(arquivo, rrn, &registro)) {
            free(estacoes.dados);
            free(pares.dados);
            return 0;
        }

        if (registro.removido == '1') continue;

        if (!vetor_int_contem(&estacoes, registro.codEstacao)) {
            vetor_int_adicionar(&estacoes, registro.codEstacao);
        }

        if (!vetor_pares_contem(&pares, registro.codEstacao, registro.codProxEstacao)) {
            vetor_pares_adicionar(&pares, registro.codEstacao, registro.codProxEstacao);
        }
    }

    cabecalho->nroEstacoes = estacoes.quantidade;
    cabecalho->nroParesEstacao = pares.quantidade;

    free(estacoes.dados);
    free(pares.dados);
    return 1;
}

void empilhar_removido(FILE *arquivo, Cabecalho *cabecalho, int rrn) {
    /*
     * Remove logicamente e encadeia no topo da pilha via campo proximo.
     * Dados antigos do registro sao preservados, conforme especificacao.
     */
    Registro registro;
    if (!ler_registro_rrn(arquivo, rrn, &registro)) return;

    registro.removido = '1';
    registro.proximo = cabecalho->topo;
    escrever_registro_rrn(arquivo, rrn, &registro);

    cabecalho->topo = rrn;
}

int desempilhar_removido(FILE *arquivo, Cabecalho *cabecalho) {
    /* Retira o topo e reposiciona o ponteiro para o proximo removido. */
    if (cabecalho->topo == -1) return -1;

    int rrn_topo = cabecalho->topo;
    Registro registro;
    if (!ler_registro_rrn(arquivo, rrn_topo, &registro)) return -1;

    cabecalho->topo = registro.proximo;
    return rrn_topo;
}
