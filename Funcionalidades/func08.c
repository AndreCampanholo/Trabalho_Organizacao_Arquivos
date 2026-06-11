#include "../auxiliares/auxiliar.h"
#include "../auxiliares/bt.h"

// Funcionalidade [8]: Imprime todos os registros que atendem aos critérios informados pelo usuário. Caso o critério de busca seja codEstacao utiliza o arquivo de índice.
void recuperar_registros_indice(char *nome_arquivo_bin, char *nome_arquivo_indice)
{
    // Em cada consulta, o registro só é aceito quando ele atende a todos os critérios ao mesmo tempo
    FILE *arquivo_bin;
    Cabecalho cabecalho;

    FILE *arquivo_indice;
    CabecalhoBT bt_cabecalho;

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
        // Cada busca vem com a sua própria lista de critérios, que é independente das anteriores.
        int quantidade_criterios = 0;
        Criterio criterios[MAX_CRITERIOS];

        if (!ler_lista_criterios(criterios, &quantidade_criterios, 1))
        {
            fclose(arquivo_bin);
            printf("%s\n", MSG_FALHA);
            return;
        }

        int encontrado = 0;

        int busca_por_cod_estacao = 0;
        for (int i = 0; i < quantidade_criterios; i++)
        {
            if (strcmp(criterios[i].nome, "codEstacao") == 0 && !criterios[i].ehNulo)
            {
                busca_por_cod_estacao = 1;
                break;
            }
        }

        if(busca_por_cod_estacao) {
            if (!abrir_binario(&arquivo_indice, nome_arquivo_indice, "rb", &bt_cabecalho, 0))
            {
                printf("%s\n", MSG_FALHA);
                return;
            }
            buscar_registro_arquivo_indice();
        }

        // Pula o cabeçalho apenas uma vez a cada busca
        if (fseek(arquivo_bin, TAMANHO_CABECALHO, SEEK_SET) != 0)
        {
            fclose(arquivo_bin);
            printf("%s\n", MSG_FALHA);
            return;
        }

        for (int rrn = 0; rrn < cabecalho.proxRRN; rrn++)
        {
            Registro registro;
            // Realiza uma busca linear sequencial, deixando o fread avançar de forma natural (sem fseek)
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

            // Quando um registro passa pelo filtro, ele é impresso imediatamente na tela.
            if (registro_atende_criterios(&registro, criterios, quantidade_criterios))
            {
                imprimir_registro(&registro);
                encontrado = 1;
            }
        }

        if (!encontrado)
            printf("%s\n", MSG_INEXISTENTE);
        // Uma linha em branco é inserida para separar visualmente o resultado de cada consulta.
        printf("\n");
    }

    fclose(arquivo_bin);
    fclose(arquivo_indice);
}