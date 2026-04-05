#include "../auxiliares/auxiliar.h"

static int inteiro_ou_nulo(const char *valor) {
    if (strcmp(valor, "NULO") == 0) return FLAG_CAMPO_NULO;
    return atoi(valor);
}

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
<<<<<<< HEAD
        char novo_codLinha[TAMANHO_TEXTO];
        char novo_codProxEstacao[TAMANHO_TEXTO];
        char novo_distProxEstacao[TAMANHO_TEXTO];
        char novo_codLinhaIntegra[TAMANHO_TEXTO];
        char novo_codEstIntegra[TAMANHO_TEXTO];
=======
        char novo_codLinha[5], novo_codProxEstacao[5], novo_distProxEstacao[5], novo_codLinhaIntegra[5], novo_codEstIntegra[5];
>>>>>>> 2beda9e7d54c264e0442befbd00a51a6f1e3a960

        if (scanf("%d", &novo_registro.codEstacao) != 1) {
            printf("%s\n", MSG_FALHA);
            fclose(arquivo_bin);
            return;
<<<<<<< HEAD
        }

        ScanQuoteString(novo_registro.nomeEstacao);
        if (scanf("%127s", novo_codLinha) != 1) {
            printf("%s\n", MSG_FALHA);
            fclose(arquivo_bin);
            return;
        }

        ScanQuoteString(novo_registro.nomeLinha);

        if (scanf("%127s %127s %127s %127s", novo_codProxEstacao, novo_distProxEstacao, novo_codLinhaIntegra, novo_codEstIntegra) != 4) {
            printf("%s\n", MSG_FALHA);
            fclose(arquivo_bin);
            return;
        }

        novo_registro.codLinha = inteiro_ou_nulo(novo_codLinha);
        novo_registro.codProxEstacao = inteiro_ou_nulo(novo_codProxEstacao);
        novo_registro.distProxEstacao = inteiro_ou_nulo(novo_distProxEstacao);
        novo_registro.codLinhaIntegra = inteiro_ou_nulo(novo_codLinhaIntegra);
        novo_registro.codEstIntegra = inteiro_ou_nulo(novo_codEstIntegra);

        novo_registro.tamNomeEstacao = (int)strlen(novo_registro.nomeEstacao);
        novo_registro.tamNomeLinha = (int)strlen(novo_registro.nomeLinha);
        novo_registro.removido = '0';
        novo_registro.proximo = -1;

        long offset_destino;
        if (cabecalho.topo != -1) {
            int proximo_topo = -1;
            if (fseek(arquivo_bin, cabecalho.topo + sizeof(char), SEEK_SET) != 0 ||
                fread(&proximo_topo, sizeof(int), 1, arquivo_bin) != 1) {
                printf("%s\n", MSG_FALHA);
                fclose(arquivo_bin);
                return;
            }
            offset_destino = cabecalho.topo;
            cabecalho.topo = proximo_topo;
        } else {
            offset_destino = rrn_para_offset(cabecalho.proxRRN);
            cabecalho.proxRRN++;
        }

        if (fseek(arquivo_bin, offset_destino, SEEK_SET) != 0 || !escrever_registro(arquivo_bin, &novo_registro)) {
=======
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
>>>>>>> 2beda9e7d54c264e0442befbd00a51a6f1e3a960
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
<<<<<<< HEAD
    fclose(arquivo_bin);

    BinarioNaTela(nome_arquivo);
}
=======

    fclose(arquivo_bin);

    BinarioNaTela(nome_arquivo);
}
>>>>>>> 2beda9e7d54c264e0442befbd00a51a6f1e3a960
