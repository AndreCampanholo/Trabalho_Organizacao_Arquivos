#ifndef BT_H
    #define BT_H

    #include "auxiliar.h"

    #define ORDEM 4

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
        int C[ORDEM - 1];
        int PR[ORDEM - 1];
        int P[ORDEM];    
    } NO;

    typedef struct bt {

    } BT;

    void BT_criar();
    void BT_inserir();
    void BT_remover();
    void BT_buscar();

#endif