#include "../auxiliares/auxiliar.h"

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
        if (scanf("%d", &quantidade_criterios) != 1 || quantidade_criterios <= 0 || quantidade_criterios > MAX_CRITERIOS)
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
            
            if (registro_atende_criterios(&registro, criterios, quantidade_criterios)) {
                imprimir_registro(&registro);
                encontrado = 1;
            }

        }

        if (!encontrado) {
            printf("%s\n", "Arquivo inexistente");
        }
    }

    fclose(arquivo_bin);
}