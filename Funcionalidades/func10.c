#include "../auxiliares/auxiliar.h"
#include "../auxiliares/bt.h"

// Funcionalidade [10]: Remove logicamente registros do arquivo de dados de acordo com os critérios de busca informados pelo usuário
void deletar_registros_indice(char *nome_arquivo, char *nome_arquivo_indice, int qtd_remocoes)
{
    // Validação dos parâmetros recebidos pela função
    if (nome_arquivo == NULL || nome_arquivo_indice == NULL || qtd_remocoes <= 0)
    {
        printf("%s\n", MSG_FALHA);
        return;
    }

    // Abre o arquivo de dados em modo de escrita (status passa a '0')
    FILE *arquivo_bin;
    Cabecalho cabecalho;
    if (!abrir_binario_escrita(&arquivo_bin, nome_arquivo, &cabecalho))
    {
        printf("%s\n", MSG_FALHA);
        return;
    }

    // Abre o arquivo de índice árvore-B em modo de escrita (status passa a '0')
    FILE *arquivo_indice;
    CabecalhoBT cabecalho_bt;
    if (!abrir_binario_escrita_bt(&arquivo_indice, nome_arquivo_indice, &cabecalho_bt))
    {
        printf("%s\n", MSG_FALHA);
        fechar_binario_escrita(arquivo_bin, &cabecalho);
        return;
    }

    // Cada uma das n remoções possui seu próprio conjunto independente de critérios
    for (int r = 0; r < qtd_remocoes; r++)
    {
        int qtd_criterios = 0;
        Criterio criterios[MAX_CRITERIOS];

        if (!ler_lista_criterios(criterios, &qtd_criterios, 1))
        {
            printf("%s\n", MSG_FALHA);
            fechar_binario_escrita(arquivo_bin, &cabecalho);
            fechar_binario_escrita_bt(arquivo_indice, &cabecalho_bt);
            return;
        }

        // Verifica se a busca utiliza codEstacao (chave indexada pela árvore-B)
        int idx_codEstacao = -1;
        for (int i = 0; i < qtd_criterios; i++)
        {
            if (strcmp(criterios[i].nome, "codEstacao") == 0 && !criterios[i].ehNulo)
            {
                idx_codEstacao = i;
                break;
            }
        }

        if (idx_codEstacao != -1)
        {
            // --- Busca direta via índice árvore-B ---
            // Como codEstacao é a chave de busca/primária, existe no máximo
            // um registro correspondente a ela.
            int chave_busca = criterios[idx_codEstacao].valorInteiro;
            int rrn_dados = recuperar_registro_indice(arquivo_indice, &cabecalho_bt, chave_busca);

            if (rrn_dados != NULO)
            {
                long offset = rrn_para_offset(rrn_dados);

                if (fseek(arquivo_bin, offset, SEEK_SET) != 0)
                {
                    printf("%s\n", MSG_FALHA);
                    fechar_binario_escrita(arquivo_bin, &cabecalho);
                    fechar_binario_escrita_bt(arquivo_indice, &cabecalho_bt);
                    return;
                }

                Registro registro;
                int leitura = ler_registro(arquivo_bin, &registro);

                if (leitura == 0)
                {
                    printf("%s\n", MSG_FALHA);
                    fechar_binario_escrita(arquivo_bin, &cabecalho);
                    fechar_binario_escrita_bt(arquivo_indice, &cabecalho_bt);
                    return;
                }

                // leitura == -1: registro já removido (índice desatualizado) — ignora
                if (leitura == 1)
                {
                    normalizar_campos_texto_registro(&registro);

                    // Só remove se TODOS os critérios (incluindo codEstacao)
                    // forem satisfeitos simultaneamente
                    if (registro_atende_criterios(&registro, criterios, qtd_criterios))
                    {
                        if (!remover_registro_logico(arquivo_bin, &cabecalho, offset))
                        {
                            printf("%s\n", MSG_FALHA);
                            fechar_binario_escrita(arquivo_bin, &cabecalho);
                            fechar_binario_escrita_bt(arquivo_indice, &cabecalho_bt);
                            return;
                        }

                        remover_registro_indice(arquivo_indice, &cabecalho_bt, chave_busca);
                    }
                }
            }
            // rrn_dados == NULO: chave não existe no índice, nada a remover nesta busca
        }
        else
        {
            // Todos os registros que satisfizerem os critérios são removidos.
            if (fseek(arquivo_bin, TAMANHO_CABECALHO, SEEK_SET) != 0)
            {
                printf("%s\n", MSG_FALHA);
                fechar_binario_escrita(arquivo_bin, &cabecalho);
                fechar_binario_escrita_bt(arquivo_indice, &cabecalho_bt);
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
                    fechar_binario_escrita_bt(arquivo_indice, &cabecalho_bt);
                    return;
                }
                if (leitura == -1)
                    continue; // registro já removido: ignora

                normalizar_campos_texto_registro(&registro);

                if (registro_atende_criterios(&registro, criterios, qtd_criterios))
                {
                    int chave = registro.codEstacao;

                    if (!remover_registro_logico(arquivo_bin, &cabecalho, offset))
                    {
                        printf("%s\n", MSG_FALHA);
                        fechar_binario_escrita(arquivo_bin, &cabecalho);
                        fechar_binario_escrita_bt(arquivo_indice, &cabecalho_bt);
                        return;
                    }

                    remover_registro_indice(arquivo_indice, &cabecalho_bt, chave);

                    // remover_registro_logico deixa o ponteiro no meio do registro
                    // (após escrever 'removido' e 'proximo'); reposiciona para o
                    // início do próximo registro antes de continuar a leitura sequencial.
                    if (fseek(arquivo_bin, offset + TAMANHO_REGISTRO, SEEK_SET) != 0)
                    {
                        printf("%s\n", MSG_FALHA);
                        fechar_binario_escrita(arquivo_bin, &cabecalho);
                        fechar_binario_escrita_bt(arquivo_indice, &cabecalho_bt);
                        return;
                    }
                }
            }
        }
    }

    // Recalcula as estatísticas agregadas do cabeçalho de dados (nroEstacoes,
    // nroParesEstacoes) após todas as remoções desta execução
    if (!calcular_nroEstacoes_nroParesEstacoes(arquivo_bin, &cabecalho))
    {
        printf("%s\n", MSG_FALHA);
        fechar_binario_escrita(arquivo_bin, &cabecalho);
        fechar_binario_escrita_bt(arquivo_indice, &cabecalho_bt);
        return;
    }

    // Marca ambos os arquivos como consistentes (status = '1') e os fecha
    fechar_binario_escrita(arquivo_bin, &cabecalho);
    fechar_binario_escrita_bt(arquivo_indice, &cabecalho_bt);

    // Exibe a saída dos arquivos de dados e de índice já atualizados
    BinarioNaTela(nome_arquivo);
    BinarioNaTela(nome_arquivo_indice);
}