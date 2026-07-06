#include "../auxiliares/auxiliar.h"
#include "../auxiliares/bt.h"

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

    if (!abrir_binario_bt(&arquivo_indice, nome_arquivo_indice, "rb", &bt_cabecalho, 0))
    {
        printf("%s\n", MSG_FALHA);
        fclose(arquivo_bin);
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

        // Verifica se algum critério usa codEstacao como chave de busca (acessa o índice).
        int chave_codEstacao;
        int busca_por_cod_estacao = criterio_obter_codEstacao(criterios, quantidade_criterios, &chave_codEstacao);

        if (busca_por_cod_estacao)
        {
            // recuperar_registro_indice retorna o byte offset do registro no arquivo de dados
            long offset = (long)recuperar_registro_indice(arquivo_indice, &bt_cabecalho, chave_codEstacao);
            if (offset == (long)NULO)
            {
                printf("%s\n", MSG_INEXISTENTE);
                // Uma linha em branco é inserida para separar visualmente o resultado de cada consulta.
                printf("\n");

                continue;
            }

            if (fseek(arquivo_bin, offset, SEEK_SET) != 0)
            {
                fclose(arquivo_bin);
                fclose(arquivo_indice);
                printf("%s\n", MSG_FALHA);
                return;
            }

            Registro registro;

            int leitura = ler_registro(arquivo_bin, &registro);
            if (leitura == 0)
            {
                fclose(arquivo_bin);
                fclose(arquivo_indice);
                printf("%s\n", MSG_FALHA);
                return;
            }
            if (leitura != -1 && registro.removido != '1')
            {
                normalizar_campos_texto_registro(&registro);

                if (registro_atende_criterios(&registro, criterios, quantidade_criterios))
                {
                    imprimir_registro(&registro);
                    encontrado = 1;
                }
            }

            if (!encontrado)
                printf("%s\n", MSG_INEXISTENTE);

            printf("\n");
            continue;
        }

        // Pula o cabeçalho apenas uma vez a cada busca
        if (fseek(arquivo_bin, TAMANHO_CABECALHO, SEEK_SET) != 0)
        {
            fclose(arquivo_bin);
            fclose(arquivo_indice);
            printf("%s\n", MSG_FALHA);
            return;
        }

        // Realiza uma busca linear sequencial, deixando o fread avançar de forma natural (sem fseek)
        for (int rrn = 0; rrn < cabecalho.proxRRN; rrn++)
        {
            Registro registro;

            int leitura = ler_registro(arquivo_bin, &registro);
            if (leitura == 0)
            {
                fclose(arquivo_bin);
                fclose(arquivo_indice);
                printf("%s\n", MSG_FALHA);
                return;
            }
            if (leitura == -1 || registro.removido == '1')
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