#include "../auxiliares/auxiliar.h"

// Funcionalidade [1]: Cria um arquivo binário a partir dos registros de um arquivo CSV.
void criar_tabela(char *nome_csv, char *nome_bin)
{
    // Abre o arquivo CSV de entrada e o arquivo binário de saída que será preenchido.
    FILE *arquivo_csv = fopen(nome_csv, "r");
    FILE *arquivo_bin = fopen(nome_bin, "wb+");

    // Verifica se houve uma falha na abertura dos arquivos, imprimindo uma mensagem de erro caso necessário.
    if (arquivo_csv == NULL || arquivo_bin == NULL)
    {
        if (arquivo_csv)
            fclose(arquivo_csv);
        if (arquivo_bin)
            fclose(arquivo_bin);
        printf("%s\n", MSG_FALHA);
        return;
    }

    // O cabeçalho é inicializado como inconsistente para proteger o arquivo caso ocorra uma falha no processo.
    Cabecalho cabecalho = {'0', -1, 0, 0, 0};
    escrever_cabecalho(arquivo_bin, &cabecalho);

    // Conta a quantidade de registros do CSV para alocar memória para os vetores de estações vistas e pares de estações.
    int qtd_registros_csv = preparar_csv_e_contar_registros(arquivo_csv);
    if (qtd_registros_csv < 0)
    {
        printf("%s\n", MSG_FALHA);
        fclose(arquivo_csv);
        fclose(arquivo_bin);
        return;
    }

    // Guarda os nomes das estações já vistas para contar as estações únicas.
    EstacoesVistas estacoes;
    inicializar_estacoes_vistas(&estacoes);
    estacoes.capacidade = qtd_registros_csv;
    if (estacoes.capacidade > 0)
    {
        // Aloca uma memória equivalente à quantidade de registros do arquivo CSV para o vetor de estações vistas.
        estacoes.nomes = (char **)malloc((size_t)estacoes.capacidade * sizeof(char *));
        if (estacoes.nomes == NULL)
        {
            printf("%s\n", MSG_FALHA);
            fclose(arquivo_csv);
            fclose(arquivo_bin);
            return;
        }
    }

    // Guarda os pares (estação, próxima estação) sem repetição para contar os pares únicos.
    ParEstacao *pares = NULL;
    int qtd_pares = 0;
    int capacidade_pares = qtd_registros_csv;
    if (capacidade_pares > 0)
    {
        // Aloca uma memória equivalente à quantidade de registros do arquivo CSV para o vetor de pares de estações.
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

    // Fluxo principal: lê os dados do CSV, escreve no binário e atualiza as estatísticas do cabeçalho.
    Registro registro_lido;
    while (1)
    {
        int resultado_leitura = ler_escrever_registros(arquivo_csv, arquivo_bin, &cabecalho, &registro_lido);
        if (resultado_leitura == 0)
            break;
        if (resultado_leitura == -1)
        {
            printf("%s\n", MSG_FALHA);
            free(pares);
            liberar_estacoes_vistas(&estacoes);
            fclose(arquivo_csv);
            fclose(arquivo_bin);
            return;
        }

        // Verifica se a estação lida do arquivo CSV e escrita no binário é nova ou não, incrementando o seu contador.
        if (nova_estacao(registro_lido.nomeEstacao, &estacoes))
            cabecalho.nroEstacoes++;

        // Verifica se encontrou um par de estações único e novo.
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

        // Se encontrou um novo par, incrementa o seu contador.
        if (resultado_par == 1)
            cabecalho.nroParesEstacoes++;
    }

    // Libera o vetor de pares de estações alocado.
    free(pares);

    // Se chegou até aqui, o arquivo ficou consistente e o seu status pode ser marcado como '1'.
    cabecalho.status = '1';
    escrever_cabecalho(arquivo_bin, &cabecalho);

    // Libera o vetor de nomes de estações vistas.
    liberar_estacoes_vistas(&estacoes);

    // Fechamento dos arquivos abertos.
    fclose(arquivo_csv);
    fclose(arquivo_bin);

    // Impressão do arquivo binário na tela.
    BinarioNaTela(nome_bin);
}