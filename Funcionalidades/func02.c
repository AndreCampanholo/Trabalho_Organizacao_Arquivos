#include "../headers/funcionalidades.h"
#include "../headers/auxiliares.h"

void recuperar_registros(char *nome_arquivo_bin)
{
    // Abertura do arquivo binário no modo de leitura.
    FILE *arquivo_bin = fopen(nome_arquivo_bin, "rb");

    // Caso a abertura falhe, imprime uma mensagem de erro.
    if (arquivo_bin == NULL)
    {
        printf("%s\n", MSG_FALHA);
        return;
    }

    // O cabeçalho é lido no início para validar o arquivo e descobrir até qual RRN ele deve ser percorrido.
    Cabecalho curr_cabecalho;
    if (!ler_cabecalho(arquivo_bin, &curr_cabecalho))
    {
        printf("%s\n", MSG_FALHA);
        fclose(arquivo_bin);
        return;
    }

    // Caso o status do arquivo esteja inconsistente (0), fecha-o e imprime uma mensagem de erro.
    if (curr_cabecalho.status != '1')
    {
        fclose(arquivo_bin);
        printf("%s\n", MSG_FALHA);
        return;
    }

    bool existe_registro = false;
    Registro curr_registro;

    // Posiciona o ponteiro no início do primeiro registro (pulando o cabeçalho)
    if (fseek(arquivo_bin, TAMANHO_CABECALHO, SEEK_SET) != 0)
    {
        printf("%s\n", MSG_FALHA);
        fclose(arquivo_bin);
        return;
    }

    // O laço percorre todos os RRNs já ocupados e tenta reconstruir cada um dos registros na memória.
    for (int i = 0; i < curr_cabecalho.proxRRN; i++)
    {
        int ler_registros = ler_registro(arquivo_bin, &curr_registro);
        if (ler_registros == 0)
        {
            printf("%s\n", MSG_FALHA);
            fclose(arquivo_bin);
            return;
        }
        else if (ler_registros == -1)
        {
            // O registro removido logicamente é ignorado para não aparecer na saída final.
            continue;
        }

        normalizar_campos_texto_registro(&curr_registro);

        // Impressão de todos os campos do registro
        printf("%d %s ", curr_registro.codEstacao, curr_registro.nomeEstacao);
        int_ou_nulo(curr_registro.codLinha);
        if (curr_registro.tamNomeLinha == 0)
            printf("%s ", "NULO");
        else
            printf("%s ", curr_registro.nomeLinha);
        int_ou_nulo(curr_registro.codProxEstacao);
        int_ou_nulo(curr_registro.distProxEstacao);
        int_ou_nulo(curr_registro.codLinhaIntegra);

        if (curr_registro.codEstIntegra == -1)
            printf("NULO\n");
        else
            printf("%d\n", curr_registro.codEstIntegra);

        printf("\n");
        existe_registro = true;
    }

    if (!existe_registro)
        printf("%s\n", MSG_INEXISTENTE);

    // Fechamento do arquivo binário.
    fclose(arquivo_bin);
}