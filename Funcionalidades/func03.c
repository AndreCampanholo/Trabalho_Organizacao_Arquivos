#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../pilha/pilha.h"
#include "../auxiliares/auxiliar.h"

typedef struct
{
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

typedef struct{
    char nome[32];
    int ehNulo;
    int valorInteiro;
    char valorTexto[44];
} Criterio;

int abrir_binario(FILE **arquivo, char *nome_arquivo, char *modo, Cabecalho *cabecalho, int eh_escrita) {
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

void recuperar_registros_condicional(char *nome_arquivo_bin)
{
    /* Para cada consulta, o programa aplica uma operação AND entre todos os criterios informados. */
    FILE *arquivo_bin;
    Cabecalho cabecalho;

    if (!abrir_binario(&arquivo_bin, nome_arquivo_bin, "rb", &cabecalho, 0))
    {
        printf("%s\n", MSG_FALHA);
        return;
    }

    int quantidade_buscas;
    if (scanf("%d", &quantidade_buscas) != 1)
    {
        fclose(arquivo_bin);
        printf("%s\n", MSG_FALHA);
        return;
    }

    for (int b = 0; b < quantidade_buscas; b++)
    {
        int quantidade_criterios;
        if (scanf("%d", &quantidade_criterios) != 1 || quantidade_criterios < 0 || quantidade_criterios > MAX_CRITERIOS)
        {
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
            if (!ler_registro_rrn(arquivo_bin, &registro)) {
                fclose(arquivo_bin);
                printf("%s\n", MSG_FALHA);
                return;
            }

            if (registro.removido == '1') continue;

        }

        if (!encontrado) {
            printf("%s\n", "Arquivo inexistente");
        }
    }

    fclose(arquivo_bin);
}