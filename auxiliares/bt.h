#ifndef BT_H
    #define BT_H

    #include <stdio.h>
    #include <stdbool.h>

    #define ORDEM 4
    #define CHAVES_MAX (ORDEM - 1)
    #define CHAVES_MIN (CHAVES_MAX / 2)
    #define TAM_NO 53
    #define NULO -1

    typedef struct cabecalho_bt {
        char status;
        int noRaiz;
        int topo;
        int proxRRN;
        int nroNos;
    } CabecalhoBT;

    typedef struct no {
        char removido;
        int proximo;
        int tipoNo;
        int nroChaves;
        int chaves[CHAVES_MAX];
        int rrns[CHAVES_MAX];
        int filhos[ORDEM];    
    } NO;

    bool bt_inserir_registro_indice(FILE *arquivo_indice, CabecalhoBT *cabecalho_bt, int chave, int rrn_registro);

#endif