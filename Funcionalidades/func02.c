#include "../auxiliares/auxiliar.h"

// Imprime todos os registros do arquivo binário
void recuperar_registros(char *nome_arquivo_bin)
{
    // Abertura do arquivo binário no modo de leitura
    FILE *arquivo_bin = fopen(nome_arquivo_bin, "rb");

    // Caso o fopen falhe, imprime mensagem de erro
    if (arquivo_bin == NULL)
    {
        printf("%s\n", MSG_FALHA);
        return;
    }

    // Leitura campo a campo do cabeçalho (consumo dos 17 bytes do cabeçalho sem fseek)
    Cabecalho curr_cabecalho;
    if (!ler_cabecalho(arquivo_bin, &curr_cabecalho))
    {
        printf("%s\n", MSG_FALHA);
        fclose(arquivo_bin);
        return;
    }

    // Caso o status do arquivo seja '1', fecha-o e imprime mensagem de erro
    if (curr_cabecalho.status != '1')
    {
        fclose(arquivo_bin);
        printf("%s\n", MSG_FALHA);
        return;
    }

    // Loop de leitura de cada registro campo a campo até proxRRN - 1 (último RRN ocupado)
    bool existe_registro = false;
    Registro curr_registro;
    for (int i = 0; i < curr_cabecalho.proxRRN; i++)
    {
        long offset = rrn_para_offset(i);
        if (fseek(arquivo_bin, offset, SEEK_SET) != 0)
        {
            printf("%s\n", MSG_FALHA);
            fclose(arquivo_bin);
            return;
        }

        int ler_registros = ler_registro(arquivo_bin, &curr_registro);
        if (ler_registros == 0)
        {
            printf("%s\n", MSG_FALHA);
            fclose(arquivo_bin);
            return;
        }
        else if (ler_registros == -1)
        {
            continue;
        }

        normalizar_campos_texto_registro(&curr_registro);

        // Impressão do registro no formato exigido
        printf("%d %s ", curr_registro.codEstacao, curr_registro.nomeEstacao);
        int_ou_nulo(curr_registro.codLinha);
        if (curr_registro.tamNomeLinha == 0)
            printf("%s ", "NULO");
        else
            printf("%s ", curr_registro.nomeLinha);
        int_ou_nulo(curr_registro.codProxEstacao);
        int_ou_nulo(curr_registro.distProxEstacao);
        int_ou_nulo(curr_registro.codLinhaIntegra);
        int_ou_nulo(curr_registro.codEstIntegra);
        printf("\n");
        existe_registro = true;
    }

    if (!existe_registro)
        printf("%s\n", MSG_INEXISTENTE);

    // Fechamento do arquivo binário
    fclose(arquivo_bin);
}