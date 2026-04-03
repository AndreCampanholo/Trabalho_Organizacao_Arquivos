#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../pilha/pilha.h"

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
    char *nomeEstacao;
    int tamNomeLinha;
    char *nomeLinha;
} Registro;

// Escrita do cabeçalho campo a campo (evita padding interno)
void escrever_cabecalho(FILE *arquivo, Cabecalho *cabecalho) {
    fseek(arquivo, 0, SEEK_SET);
    fwrite(&cabecalho->status, sizeof(char), 1, arquivo);
    fwrite(&cabecalho->topo, sizeof(int), 1, arquivo);
    fwrite(&cabecalho->proxRRN, sizeof(int), 1, arquivo);
    fwrite(&cabecalho->nroEstacoes, sizeof(int), 1, arquivo);
    fwrite(&cabecalho->nroParesEstacoes, sizeof(int), 1, arquivo);
}
// Lê registros do arquivo .csv e escreve-os no .bin
bool ler_escrever_registros(FILE *csv, FILE *bin, Cabecalho *cabecalho) {
    char linha[512];
    char *campo;
    int lixoQtd;

    char removido = '0';
    int proximo = -1;
    int codEstacao;
    int codLinha;
    int codProxEstacao;
    int distProxEstacao;
    int codLinhaIntegra;
    int codEstIntegra;
    int tamNomeEstacao;
    char *nomeEstacao;
    int tamNomeLinha;
    char *nomeLinha;

    if (fgets(linha, sizeof(linha), csv) == NULL) return false;

    campo = strtok(linha, ",");
    codEstacao = atoi(campo);    
    campo = strtok(NULL, ",");
    nomeEstacao = (campo != NULL) ? campo : "";
    campo = strtok(NULL, ",");
    codLinha = atoi(campo);
    campo = strtok(NULL, ",");
    nomeLinha = (campo != NULL) ? campo : "";
    campo = strtok(NULL, ",");
    codProxEstacao = atoi(campo);
    campo = strtok(NULL, ",");
    distProxEstacao = atoi(campo);
    campo = strtok(NULL, ",");
    codLinhaIntegra = atoi(campo);
    campo = strtok(NULL, ",");
    codEstIntegra = atoi(campo);

    tamNomeEstacao = strlen(nomeEstacao);
    tamNomeLinha = strlen(nomeLinha);
    
    fwrite(&removido, sizeof(char), 1, bin);
    fwrite(&proximo, sizeof(int), 1, bin);
    fwrite(&codEstacao, sizeof(int), 1, bin);
    fwrite(&codLinha, sizeof(int), 1, bin);
    fwrite(&codProxEstacao, sizeof(int), 1, bin);
    fwrite(&distProxEstacao, sizeof(int), 1, bin);
    fwrite(&codLinhaIntegra, sizeof(int), 1, bin);
    fwrite(&codEstIntegra, sizeof(int), 1, bin);
    fwrite(&tamNomeEstacao, sizeof(int), 1, bin);
    fwrite(nomeEstacao, sizeof(char), tamNomeEstacao, bin);
    fwrite(&tamNomeLinha, sizeof(int), 1, bin);
    fwrite(nomeLinha, sizeof(char), tamNomeLinha, bin);

    lixoQtd = 43 - tamNomeEstacao - tamNomeLinha;
    for (int i = 0; i < lixoQtd; i++) {
        char lixo = '$';
        fwrite(&lixo, sizeof(char), 1, bin);
    }

    cabecalho->proxRRN++;
    cabecalho->nroEstacoes++;
    //REVER!!!
    if (codLinhaIntegra != -1 && codEstIntegra != -1) {
        cabecalho->nroParesEstacoes++;
    }
    
    return true;
}

void criar_tabela(char *nome_csv, char *nome_bin){   
    FILE *arquivo_csv = fopen(nome_csv, "r");
    FILE *arquivo_bin = fopen(nome_bin, "wb+");

    if(arquivo_csv == NULL || arquivo_bin == NULL){
        if(arquivo_csv) fclose(arquivo_csv);
        if(arquivo_bin) fclose(arquivo_bin);
        printf("%s\n", "Falha no processamento do arquivo.");
        return;
    }

    // Inicializa o cabeçalho do arquivo binário
    Cabecalho cabecalho = {'0', -1, 0, 0, 0};
    escrever_cabecalho(arquivo_bin, &cabecalho);

    // Descarte da primeira linha do arquivo .csv
    char linha[512];
    fgets(linha, sizeof(linha), arquivo_csv);

    while(ler_escrever_registros(arquivo_csv, arquivo_bin, &cabecalho));

    cabecalho.status = 1;
    escrever_cabecalho(arquivo_bin, &cabecalho);

    fclose(arquivo_csv);
    fclose(arquivo_bin);
}