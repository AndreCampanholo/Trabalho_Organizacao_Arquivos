#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../auxiliares/auxiliar.h"

void inserir_registros(char *nome_arquivo, int qtd_insercoes) {
    if(nome_arquivo == NULL || qtd_insercoes == 0) {
        printf("%d\n", MSG_FALHA);
        return;
    }

    FILE *arquivo_bin = fopen(nome_arquivo, "r+b");
    if(arquivo_bin == NULL) {
        printf("%d\n", MSG_FALHA);
        return;
    }

    Cabecalho cabecalho;
    ler_cabecalho(arquivo_bin, &cabecalho);
    if (!ler_cabecalho(arquivo_bin, &cabecalho)) {
        printf("%s\n", MSG_FALHA);
        fclose(arquivo_bin);
        return;
    }

    if(cabecalho.status != '1') {
        printf("%s\n", MSG_FALHA);
        fclose(arquivo_bin);
        return;
    }
    cabecalho.status = '0';
    escrever_cabecalho(arquivo_bin, &cabecalho);

    Registro novo_registro;
    char novo_codLinha[5], novo_codProxEstacao[5], novo_distProxEstacao[5], novo_codLinhaIntegra[5], novo_codEstIntegra[5];
    for(int i = 0; i < qtd_insercoes; i++) {
        scanf("%d", &novo_registro.codEstacao);
        ScanQuoteString(&novo_registro.nomeEstacao);
        scanf("%s", novo_codLinha);
        ScanQuoteString(&novo_registro.nomeLinha);
        scanf("%s %s %s %s", novo_codProxEstacao, novo_distProxEstacao, novo_codLinhaIntegra, novo_codEstIntegra);

        if(strcmp(novo_codLinha, "NULO") == 0) novo_registro.codLinha = -1;
        else atoi(novo_codLinha);
        if(strcmp(novo_codProxEstacao, "NULO") == 0) novo_registro.codProxEstacao = -1;
        else atoi(novo_codProxEstacao);
        if(strcmp(novo_distProxEstacao, "NULO") == 0) novo_registro.distProxEstacao = -1;
        else atoi(novo_distProxEstacao);
        if(strcmp(novo_codLinhaIntegra, "NULO") == 0) novo_registro.codLinhaIntegra = -1;
        else atoi(novo_codLinhaIntegra);
        if(strcmp(novo_codEstIntegra, "NULO") == 0) novo_registro.codEstIntegra = -1;
        else atoi(novo_codEstIntegra);

        if(cabecalho.topo != -1) {
            if (fseek(arquivo_bin, cabecalho.topo, SEEK_SET) != 0) {
                printf("%s\n", MSG_FALHA);
                fclose(arquivo_bin);
                return;
            }
        } else {
            if (fseek(arquivo_bin, cabecalho.proxRRN, SEEK_SET) != 0) {
                printf("%s\n", MSG_FALHA);
                fclose(arquivo_bin);
                return;
            }
        }

        novo_registro.removido = '0';
        novo_registro.proximo = 

        if(!escrever_registro(arquivo_bin, &novo_registro)) {
            printf("%s\n", MSG_FALHA);
            fclose(arquivo_bin);
            return;
        }
    }
}