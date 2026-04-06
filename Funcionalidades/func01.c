#include "../auxiliares/auxiliar.h"

typedef struct
{
    int codEstacao;
    int codProxEstacao;
} ParEstacao;

int preparar_csv_e_contar_registros(FILE *arquivo_csv)
{
    char linha[512];
    if (fgets(linha, sizeof(linha), arquivo_csv) == NULL)
        return 0;

    int quantidade = 0;
    while (fgets(linha, sizeof(linha), arquivo_csv) != NULL)
        quantidade++;

    if (fseek(arquivo_csv, 0, SEEK_SET) != 0)
        return -1;
    if (fgets(linha, sizeof(linha), arquivo_csv) == NULL)
        return -1;

    return quantidade;
}

// Retorno: 1 = inseriu par novo, 0 = par ja existia, -1 = erro de alocacao.
int adicionar_par_unico(int codEstacao, int codProxEstacao, ParEstacao **pares, int *quantidade, int *capacidade)
{
    for (int i = 0; i < *quantidade; i++)
    {
        if ((*pares)[i].codEstacao == codEstacao && (*pares)[i].codProxEstacao == codProxEstacao)
            return 0;
    }

    if (*quantidade >= *capacidade)
        return -1;

    (*pares)[*quantidade].codEstacao = codEstacao;
    (*pares)[*quantidade].codProxEstacao = codProxEstacao;
    (*quantidade)++;
    return 1;
}

// Funcionalidade [1]: cria arquivo binário a partir de registro de arquivo csv
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
        printf("%s\n", MSG_FALHA);
        return;
    }

    // Inicializa o cabeçalho do arquivo binário
    Cabecalho cabecalho = {'0', -1, 0, 0, 0};
    escrever_cabecalho(arquivo_bin, &cabecalho);

    int qtd_registros_csv = preparar_csv_e_contar_registros(arquivo_csv);
    if (qtd_registros_csv < 0)
    {
        printf("%s\n", MSG_FALHA);
        fclose(arquivo_csv);
        fclose(arquivo_bin);
        return;
    }

    // Inicializa estruturas de apoio para contagem incremental
    EstacoesVistas estacoes;
    inicializar_estacoes_vistas(&estacoes);
    estacoes.capacidade = qtd_registros_csv;
    if (estacoes.capacidade > 0)
    {
        estacoes.nomes = (char **)malloc((size_t)estacoes.capacidade * sizeof(char *));
        if (estacoes.nomes == NULL)
        {
            printf("%s\n", MSG_FALHA);
            fclose(arquivo_csv);
            fclose(arquivo_bin);
            return;
        }
    }

    ParEstacao *pares = NULL;
    int qtd_pares = 0;
    int capacidade_pares = qtd_registros_csv;
    if (capacidade_pares > 0)
    {
        pares = (ParEstacao *)malloc((size_t)capacidade_pares * sizeof(ParEstacao));
        if (pares == NULL)
        {
            printf("%s\n", MSG_FALHA);
            liberar_estacoes_vistas(&estacoes);
            fclose(arquivo_csv);
            fclose(arquivo_bin);
            return;
        }
    }

    // Laço de leitura dos registros do .csv e escrita no .bin, com atualização incremental do cabeçalho
    Registro registro_lido;
    while (ler_escrever_registros(arquivo_csv, arquivo_bin, &cabecalho, &registro_lido))
    {
        if (nova_estacao(registro_lido.nomeEstacao, &estacoes))
            cabecalho.nroEstacoes++;

        int resultado_par = adicionar_par_unico(registro_lido.codEstacao, registro_lido.codProxEstacao, &pares, &qtd_pares, &capacidade_pares);
        if (resultado_par == -1)
        {
            printf("%s\n", MSG_FALHA);
            free(pares);
            liberar_estacoes_vistas(&estacoes);
            fclose(arquivo_csv);
            fclose(arquivo_bin);
            return;
        }

        if (resultado_par == 1)
            cabecalho.nroParesEstacoes++;
    }

    free(pares);

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