#include "../auxiliares/auxiliar.h"

void criar_tabela(char *nome_csv, char *nome_bin)
{
    // Abertura dos arquivos csv e bin
    FILE *arquivo_csv = fopen(nome_csv, "r");
    FILE *arquivo_bin = fopen(nome_bin, "wb+");

    // Verifica se houve falha na abertura dos arquivos, imprimindo mensagem de erro caso necessário
    if (arquivo_csv == NULL || arquivo_bin == NULL)
    {
        if (arquivo_csv)
            fclose(arquivo_csv);
        if (arquivo_bin)
            fclose(arquivo_bin);
        printf("%s\n", "Falha no processamento do arquivo.");
        return;
    }

    // Inicializa o cabeçalho do arquivo binário
    Cabecalho cabecalho = {'0', -1, 0, 0, 0};
    escrever_cabecalho(arquivo_bin, &cabecalho);

    // Descarte da primeira linha do arquivo .csv
    char linha[512];
    fgets(linha, sizeof(linha), arquivo_csv);

    // Inicializa struct de estacoes vistas
    EstacoesVistas estacoes;
    inicializar_estacoes_vistas(&estacoes);

    // Laço de leitura dos registros do .csv e escrita no .bin, além da atualização do cabeçalho
    while (ler_escrever_registros(arquivo_csv, arquivo_bin, &cabecalho, &estacoes))
        ;

    if (!calcular_nroEstacoes_nroParesEstacoes(arquivo_bin, &cabecalho))
    {
        printf("%s\n", "Falha no processamento do arquivo.");
        liberar_estacoes_vistas(&estacoes);
        fclose(arquivo_csv);
        fclose(arquivo_bin);
        return;
    }

    // Seta o status no cabeçalho para consistente
    cabecalho.status = '1';
    escrever_cabecalho(arquivo_bin, &cabecalho);

    // Libera o vetor de nomes de estações vistas
    liberar_estacoes_vistas(&estacoes);

    // Fechamento dos arquivos abertos
    fclose(arquivo_csv);
    fclose(arquivo_bin);

    // Impressão do .bin na tela
    BinarioNaTela(nome_bin);
}