#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../auxiliares/auxiliar.h"

static int abrir_binario(FILE **arquivo, char *nome_arquivo, char *modo, Cabecalho *cabecalho, int eh_escrita) {
    *arquivo = fopen(nome_arquivo, modo);
    if (*arquivo == NULL) return 0;

    if (!ler_cabecalho(*arquivo, cabecalho)) {
        fclose(*arquivo);
        return 0;
    }

    if (cabecalho->status != '1') {
        fclose(*arquivo);
        return 0;
    }

    if (eh_escrita) {
        cabecalho->status = '0';
        escrever_cabecalho(*arquivo, cabecalho);
    }

    return 1;
}

void recuperar_registros_condicional(char *nome_arquivo_bin) {
    FILE *arquivo_bin;
    Cabecalho cabecalho;

    if (!abrir_binario(&arquivo_bin, nome_arquivo_bin, "rb", &cabecalho, 0)) {
        printf("%s\n", MSG_FALHA);
        return;
    }

    int quantidade_buscas;
    if (scanf("%d", &quantidade_buscas) != 1) {
        fclose(arquivo_bin);
        printf("%s\n", MSG_FALHA);
        return;
    }

    for (int b = 0; b < quantidade_buscas; b++) {
        int quantidade_criterios;
        if (scanf("%d", &quantidade_criterios) != 1 || quantidade_criterios < 0 || quantidade_criterios > MAX_CRITERIOS) {
            fclose(arquivo_bin);
            printf("%s\n", MSG_FALHA);
            return;
        }

        Criterio criterios[MAX_CRITERIOS];
        for (int i = 0; i < quantidade_criterios; i++) {
            if (!ler_criterio(&criterios[i])) {
                fclose(arquivo_bin);
                printf("%s\n", MSG_FALHA);
                return;
            }
        }

        int encontrado = 0;
        for (int rrn = 0; rrn < cabecalho.proxRRN; rrn++) {
            Registro registro;
            long offset = rrn_para_offset(rrn);

            if (fseek(arquivo_bin, offset, SEEK_SET) != 0) {
                fclose(arquivo_bin);
                printf("%s\n", MSG_FALHA);
                return;
            }

            int leitura = ler_registro(arquivo_bin, &registro);
            if (leitura == 0) {
                fclose(arquivo_bin);
                printf("%s\n", MSG_FALHA);
                return;
            }
            if (leitura == -1) continue;

            if (registro.tamNomeEstacao >= 0 && registro.tamNomeEstacao < TAMANHO_CAMPO_VARIAVEL) {
                registro.nomeEstacao[registro.tamNomeEstacao] = '\0';
            }
            if (registro.tamNomeLinha >= 0 && registro.tamNomeLinha < TAMANHO_CAMPO_VARIAVEL) {
                registro.nomeLinha[registro.tamNomeLinha] = '\0';
            }

            if (registro_atende_criterios(&registro, criterios, quantidade_criterios)) {
                imprimir_registro(&registro);
                encontrado = 1;
            }
        }

        if (!encontrado) {
            printf("%s\n", MSG_INEXISTENTE);
        }
    }

    fclose(arquivo_bin);
}
