#include "../headers/funcionalidades.h"
#include "../headers/auxiliares.h"

void deletar_registros(char *nome_arquivo, int qtd_remocoes)
{
    // Verifica se os parâmetros informados são válidos.
    if (nome_arquivo == NULL || qtd_remocoes <= 0)
    {
        printf("%s\n", MSG_FALHA);
        return;
    }

    // Abre o arquivo para leitura e escrita binária.
    FILE *arquivo_bin;
    Cabecalho cabecalho;
    if (!abrir_binario_escrita(&arquivo_bin, nome_arquivo, &cabecalho))
    {
        printf("%s\n", MSG_FALHA);
        return;
    }

    int qtd_criterios;
    for (int i = 0; i < qtd_remocoes; i++)
    {
        Criterio criterios[MAX_CRITERIOS];

        if (!ler_lista_criterios(criterios, &qtd_criterios, 1))
        {
            printf("%s\n", MSG_FALHA);
            fechar_binario_escrita(arquivo_bin, &cabecalho);
            return;
        }

        if (fseek(arquivo_bin, TAMANHO_CABECALHO, SEEK_SET) != 0)
        {
            printf("%s\n", MSG_FALHA);
            fechar_binario_escrita(arquivo_bin, &cabecalho);
            return;
        }

        for (int rrn = 0; rrn < cabecalho.proxRRN; rrn++)
        {
            long offset = rrn_para_offset(rrn);

            Registro registro;
            int leitura = ler_registro(arquivo_bin, &registro);
            if (leitura == 0)
            {
                printf("%s\n", MSG_FALHA);
                fechar_binario_escrita(arquivo_bin, &cabecalho);
                return;
            }
            if (leitura == -1)
                continue;

            normalizar_campos_texto_registro(&registro);

            if (!registro_atende_criterios(&registro, criterios, qtd_criterios))
                continue;

            if (!remover_registro_logico(arquivo_bin, &cabecalho, offset))
            {
                printf("%s\n", MSG_FALHA);
                fechar_binario_escrita(arquivo_bin, &cabecalho);
                return;
            }

            // Reposiciona o ponteiro para o início do próximo registro, pois remover_registro_logico
            // deixa o ponteiro após os 5 bytes escritos (removido + proximo).
            if (fseek(arquivo_bin, offset + TAMANHO_REGISTRO, SEEK_SET) != 0)
            {
                printf("%s\n", MSG_FALHA);
                fechar_binario_escrita(arquivo_bin, &cabecalho);
                return;
            }
        }
    }

    // Recalcula as estatísticas do cabeçalho após todas as remoções.
    if (!calcular_nroEstacoes_nroParesEstacoes(arquivo_bin, &cabecalho))
    {
        printf("%s\n", MSG_FALHA);
        fechar_binario_escrita(arquivo_bin, &cabecalho);
        return;
    }

    fechar_binario_escrita(arquivo_bin, &cabecalho);
    BinarioNaTela(nome_arquivo);
}