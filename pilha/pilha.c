#include "../pilha/pilha.h"
#include <stdlib.h>

struct no {
    int rrn;
    NO* anterior;
};

struct pilha {
    NO *topo;
    int tamanho;
};

PILHA *pilha_criar(void) {
    PILHA *pilha = (PILHA *)malloc(sizeof(PILHA));
    if(pilha != NULL) {
        pilha->topo = NULL;
    pilha->tamanho = 0;
    }
    return pilha;
};

void pilha_apagar(PILHA **pilha){
    NO *aux;
    if(*pilha != NULL) {
        while((*pilha)->topo != NULL) {
            aux = (*pilha)->topo;
            (*pilha)->topo = (*pilha)->topo->anterior;
            free(aux);
            aux = NULL;
        }
        (*pilha)->topo = NULL;
        free(*pilha);
        *pilha = NULL;
    }
};

bool pilha_vazia(PILHA *pilha) {
    if(pilha->topo == NULL) {
        return true;
    } else {
        return false;
    }
};

bool pilha_cheia(PILHA *pilha) {
    if(pilha->tamanho >= TAM) {
        return true;
    } else {
        return false;
    }
};

int pilha_tamanho(PILHA *pilha) {
    if(pilha == NULL) {
        return 0;
    }
    return pilha->tamanho;
};

int pilha_topo(PILHA *pilha) {
    if(pilha == NULL || pilha->topo == NULL) {
        return NULL;
    }
    return pilha->topo->rrn;
};

bool pilha_empilhar(PILHA *pilha, int rrn) {
    if(pilha == NULL || pilha_cheia(pilha)) {
        return false;
    } else {
        NO *novo = (NO *)malloc(sizeof(NO));
        NO *aux;
        aux = pilha->topo;
        pilha->topo = novo;
        novo->rrn = rrn;
        pilha->topo->anterior = aux;
        aux = NULL;
        pilha->tamanho++;
        return true;
    }
};

int pilha_desempilhar(PILHA *pilha) {
    if(pilha == NULL || pilha->topo == NULL) {
        return NULL;
    } else {
        NO *aux;
        aux = pilha->topo;
        int rrn = aux->rrn;
        pilha->topo = pilha->topo->anterior;
        aux->anterior = NULL;
        pilha->tamanho--;
        free(aux);
        aux = NULL;
        return rrn;
    }
};