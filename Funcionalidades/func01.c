#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../auxiliares/auxiliar.h"

// Lê registros do arquivo .csv e escreve-os no .bin
bool ler_escrever_registros(FILE *csv, FILE *bin, Cabecalho *cabecalho, EstacoesVistas *estacoes) {
    // Variáveis auxiliares
    char linha[512];
    char *campo;
    int lixoQtd;

    Registro registro;
    registro.removido = '0';
    registro.proximo = -1;

    // Se não houverem mais linhas de registros no csv, retorna false
    if (fgets(linha, sizeof(linha), csv) == NULL) return false;

    // Atribuição dos valores do csv às variáveis por meio da tokenização de 'linha' nas virgulas (",")
    campo = strtok(linha, ",");
    registro.codEstacao = atoi(campo);    
    campo = strtok(NULL, ",");
    strcpy(registro.nomeEstacao,campo);
    campo = strtok(NULL, ",");
    registro.codLinha = (campo != NULL && campo[0] != '\0') ? atoi(campo) : -1;
    campo = strtok(NULL, ",");
    strcpy(registro.nomeLinha, (campo != NULL) ? campo : "");
    campo = strtok(NULL, ",");
    registro.codProxEstacao = (campo != NULL && campo[0] != '\0') ? atoi(campo) : -1;
    campo = strtok(NULL, ",");
    registro.distProxEstacao = (campo != NULL && campo[0] != '\0') ? atoi(campo) : -1;
    campo = strtok(NULL, ",");
    registro.codLinhaIntegra = (campo != NULL && campo[0] != '\0') ? atoi(campo) : -1;
    campo = strtok(NULL, ",");
    registro.codEstIntegra = (campo != NULL && campo[0] != '\0') ? atoi(campo) : -1;

    // Só incrementa 'nroEstacoes' se 'nomeEstacao' não estiver no vetor de nomes da struct 'estacoes'
    if (nova_estacao(registro.nomeEstacao, estacoes))
        cabecalho->nroEstacoes++;

    // Valores dos indicadores de tamanho dos campos de tamanho variável
    registro.tamNomeEstacao = strlen(registro.nomeEstacao);
    registro.tamNomeLinha = strlen(registro.nomeLinha);
    
    escrever_registro(bin, &registro);
    
    lixoQtd = 43 - registro.tamNomeEstacao - registro.tamNomeLinha;
    for (int i = 0; i < lixoQtd; i++) {
        char lixo = '$';
        fwrite(&lixo, sizeof(char), 1, bin);
    }

    cabecalho->proxRRN++;

    if (registro.codLinhaIntegra != -1 && registro.codEstIntegra != -1) {
        cabecalho->nroParesEstacoes++;
    }

    return true;
}

void criar_tabela(char *nome_csv, char *nome_bin){   
    // Abertura dos arquivos csv e bin
    FILE *arquivo_csv = fopen(nome_csv, "r");
    FILE *arquivo_bin = fopen(nome_bin, "wb+");

    // Verifica se houve falha na abertura dos arquivos, imprimindo mensagem de erro caso necessário
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

    // Inicializa struct de estacoes vistas
    EstacoesVistas estacoes;
    inicializar_estacoes_vistas(&estacoes);

    // Laço de leitura dos registros do .csv e escrita no .bin, além da atualização do cabeçalho
    while (ler_escrever_registros(arquivo_csv, arquivo_bin, &cabecalho, &estacoes));

    // Seta o status no cabeçalho para consistente
    cabecalho.status = '1';
    escrever_cabecalho(arquivo_bin, &cabecalho);

    // Libera o vetor de nomes de estações vistas
    liberar_estacoes_vistas(&estacoes);

    //Fechamento dos arquivos abertos
    fclose(arquivo_csv);
    fclose(arquivo_bin);

    // Impressão do .bin na tela
    BinarioNaTela(nome_bin);
}