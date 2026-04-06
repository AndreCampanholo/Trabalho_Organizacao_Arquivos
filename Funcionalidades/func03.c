#include "../auxiliares/auxiliar.h"

// Funcionalidade [3]: Imprime todos os registros que atendem os critérios informados pelo usuário
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
        // Cada busca possui sua própria lista de critérios.
        int quantidade_criterios = 0;
        Criterio criterios[MAX_CRITERIOS];

        if (!ler_lista_criterios(criterios, &quantidade_criterios, 1))
        {
            fclose(arquivo_bin);
            printf("%s\n", MSG_FALHA);
            return;
        }

        int encontrado = 0;
        for (int rrn = 0; rrn < cabecalho.proxRRN; rrn++)
        {
            Registro registro;
            // Busca linear: avalia todos os registros ativos do arquivo.
            long offset = rrn_para_offset(rrn);

            if (fseek(arquivo_bin, offset, SEEK_SET) != 0)
            {
                fclose(arquivo_bin);
                printf("%s\n", MSG_FALHA);
                return;
            }

            int leitura = ler_registro(arquivo_bin, &registro);
            if (leitura == 0)
            {
                fclose(arquivo_bin);
                printf("%s\n", MSG_FALHA);
                return;
            }
            if (leitura == -1)
                continue;

            if (registro.removido == '1')
                continue;

            normalizar_campos_texto_registro(&registro);

            // Verifica se registro atende aos critérios de busca e caso positivo, imprime-o.
            if (registro_atende_criterios(&registro, criterios, quantidade_criterios))
            {
                imprimir_registro(&registro);
                encontrado = 1;
            }
        }
        printf("\n");
        if (!encontrado)
            printf("%s\n", MSG_INEXISTENTE);
    }

    fclose(arquivo_bin);
}