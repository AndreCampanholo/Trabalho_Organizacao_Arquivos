#include "../auxiliares/auxiliar.h"

// Funcionalidade [1]: cria arquivo binário a partir de registro de arquivo csv
void criar_tabela(char *nome_csv, char *nome_bin)
{
    // A funcao abre o CSV de entrada e o BIN de saida que sera preenchido.
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

    // O cabecalho nasce inconsistente para proteger o arquivo caso ocorra falha no meio do processo.
    Cabecalho cabecalho = {'0', -1, 0, 0, 0};
    escrever_cabecalho(arquivo_bin, &cabecalho);

    // Conta a quantidade de registros do csv para aloca memória para vetor de estacoesVistas e ParesEstacao
    int qtd_registros_csv = preparar_csv_e_contar_registros(arquivo_csv);
    if (qtd_registros_csv < 0)
    {
        printf("%s\n", MSG_FALHA);
        fclose(arquivo_csv);
        fclose(arquivo_bin);
        return;
    }

    // Esta estrutura guarda os nomes de estacao ja vistos para contar estacoes unicas.
    EstacoesVistas estacoes;
    inicializar_estacoes_vistas(&estacoes);
    estacoes.capacidade = qtd_registros_csv;
    if (estacoes.capacidade > 0)
    {
        // Aloca memória equivalente à quantidade de registros do arquivo .csv para vetor de estacoes vistas
        estacoes.nomes = (char **)malloc((size_t)estacoes.capacidade * sizeof(char *));
        if (estacoes.nomes == NULL)
        {
            printf("%s\n", MSG_FALHA);
            fclose(arquivo_csv);
            fclose(arquivo_bin);
            return;
        }
    }

    // Este vetor guarda pares (estacao, proxima estacao) sem repeticao para contar os pares unicos.
    ParEstacao *pares = NULL;
    int qtd_pares = 0;
    int capacidade_pares = qtd_registros_csv;
    if (capacidade_pares > 0)
    {
        // Aloca memória equivalente à quantidade de registros do arquivo .csv para vetor de pares de estacoes
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

    // Aqui acontece o fluxo principal: le do CSV, escreve no BIN e atualiza as estatisticas do cabecalho.
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

        // Verifica se estação lida do arquivo .csv e escrita no .bin é nova ou não, incrementando contador
        if (nova_estacao(registro_lido.nomeEstacao, &estacoes))
            cabecalho.nroEstacoes++;

        // Verifica se encontrou um par de estações unico/novo
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

        // Se encontrou um novo par, incrementa contador
        if (resultado_par == 1)
            cabecalho.nroParesEstacoes++;
    }

    // Libera vetor de pares de estações alocado
    free(pares);

    // Se chegou ate aqui, o arquivo ficou consistente e o status pode ser marcado como '1'.
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