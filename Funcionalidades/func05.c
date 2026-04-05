#include "../auxiliares/auxiliar.h"

void inserir_registros(char *nome_arquivo, int qtd_insercoes) {
    if (nome_arquivo == NULL || qtd_insercoes <= 0) {
        printf("%s\n", MSG_FALHA);
        return;
    }

    FILE *arquivo_bin = fopen(nome_arquivo, "r+b");
    if (arquivo_bin == NULL) {
        printf("%s\n", MSG_FALHA);
        return;
    }

    Cabecalho cabecalho;
    if (!ler_cabecalho(arquivo_bin, &cabecalho)) {
        printf("%s\n", MSG_FALHA);
        fclose(arquivo_bin);
        return;
    }

    if (cabecalho.status != '1') {
        printf("%s\n", MSG_FALHA);
        fclose(arquivo_bin);
        return;
    }

    cabecalho.status = '0';
    escrever_cabecalho(arquivo_bin, &cabecalho);

    for (int i = 0; i < qtd_insercoes; i++) {
        Registro novo_registro;
        char novo_codLinha[5], novo_codProxEstacao[5], novo_distProxEstacao[5], novo_codLinhaIntegra[5], novo_codEstIntegra[5];

        if (scanf("%d", &novo_registro.codEstacao) != 1) {
            printf("%s\n", MSG_FALHA);
            fclose(arquivo_bin);
            return;
        }

        ScanQuoteString(novo_registro.nomeEstacao);
        novo_registro.tamNomeEstacao = (int)strlen(novo_registro.nomeEstacao);

        if (scanf("%4s", novo_codLinha) != 1) {
            printf("%s\n", MSG_FALHA);
            fclose(arquivo_bin);
            return;
        }

        ScanQuoteString(novo_registro.nomeLinha);
        novo_registro.tamNomeLinha = (int)strlen(novo_registro.nomeLinha);

        if (scanf("%4s %4s %4s %4s", novo_codProxEstacao, novo_distProxEstacao, novo_codLinhaIntegra, novo_codEstIntegra) != 4) {
            printf("%s\n", MSG_FALHA);
            fclose(arquivo_bin);
            return;
        }

        novo_registro.codLinha = (strcmp(novo_codLinha, "NULO") == 0) ? FLAG_CAMPO_NULO : atoi(novo_codLinha);
        novo_registro.codProxEstacao = (strcmp(novo_codProxEstacao, "NULO") == 0) ? FLAG_CAMPO_NULO : atoi(novo_codProxEstacao);
        novo_registro.distProxEstacao = (strcmp(novo_distProxEstacao, "NULO") == 0) ? FLAG_CAMPO_NULO : atoi(novo_distProxEstacao);
        novo_registro.codLinhaIntegra = (strcmp(novo_codLinhaIntegra, "NULO") == 0) ? FLAG_CAMPO_NULO : atoi(novo_codLinhaIntegra);
        novo_registro.codEstIntegra = (strcmp(novo_codEstIntegra, "NULO") == 0) ? FLAG_CAMPO_NULO : atoi(novo_codEstIntegra);

        long posicao_escrita;

        if (cabecalho.topo != -1) {
            posicao_escrita = cabecalho.topo;

            if (fseek(arquivo_bin, posicao_escrita + 1, SEEK_SET) != 0) {
                printf("%s\n", MSG_FALHA);
                fclose(arquivo_bin);
                return;
            }

            if (fread(&novo_registro.proximo, sizeof(int), 1, arquivo_bin) != 1) {
                printf("%s\n", MSG_FALHA);
                fclose(arquivo_bin);
                return;
            }

            cabecalho.topo = (novo_registro.proximo == -1) ? -1 : rrn_para_offset(novo_registro.proximo);
        } else {
            posicao_escrita = rrn_para_offset(cabecalho.proxRRN);
            novo_registro.proximo = -1;
            cabecalho.proxRRN++;
        }

        if (fseek(arquivo_bin, posicao_escrita, SEEK_SET) != 0) {
            printf("%s\n", MSG_FALHA);
            fclose(arquivo_bin);
            return;
        }

        novo_registro.removido = '0';

        if (!escrever_registro(arquivo_bin, &novo_registro)) {
            printf("%s\n", MSG_FALHA);
            fclose(arquivo_bin);
            return;
        }
    }

    if (!calcular_nroEstacoes_nroParesEstacoes(arquivo_bin, &cabecalho)) {
        printf("%s\n", MSG_FALHA);
        fclose(arquivo_bin);
        return;
    }

    cabecalho.status = '1';
    escrever_cabecalho(arquivo_bin, &cabecalho);

    fclose(arquivo_bin);

    BinarioNaTela(nome_arquivo);
}