#include "../auxiliares/auxiliar.h"

// Funcionalidade [5]: Insere registros informados pelo usuário
void inserir_registros(char *nome_arquivo, int qtd_insercoes)
{
    if (nome_arquivo == NULL || qtd_insercoes <= 0)
    {
        printf("%s\n", MSG_FALHA);
        return;
    }

    FILE *arquivo_bin;
    Cabecalho cabecalho;
    if (!abrir_binario_escrita(&arquivo_bin, nome_arquivo, &cabecalho))
    {
        printf("%s\n", MSG_FALHA);
        return;
    }

    for (int i = 0; i < qtd_insercoes; i++)
    {
        Registro novo_registro;

        char novo_codLinha[5], novo_codProxEstacao[5], novo_distProxEstacao[5], novo_codLinhaIntegra[5], novo_codEstIntegra[5];

        if (scanf("%d", &novo_registro.codEstacao) != 1)
        {
            printf("%s\n", MSG_FALHA);
            fclose(arquivo_bin);
            return;
        }

        ScanQuoteString(novo_registro.nomeEstacao);
        if (scanf("%4s", novo_codLinha) != 1)
        {
            printf("%s\n", MSG_FALHA);
            fclose(arquivo_bin);
            return;
        }

        ScanQuoteString(novo_registro.nomeLinha);

        if (scanf("%4s %4s %4s %4s", novo_codProxEstacao, novo_distProxEstacao, novo_codLinhaIntegra, novo_codEstIntegra) != 4)
        {
            printf("%s\n", MSG_FALHA);
            fclose(arquivo_bin);
            return;
        }

        novo_registro.codLinha = inteiro_ou_nulo(novo_codLinha);
        novo_registro.codProxEstacao = inteiro_ou_nulo(novo_codProxEstacao);
        novo_registro.distProxEstacao = inteiro_ou_nulo(novo_distProxEstacao);
        novo_registro.codLinhaIntegra = inteiro_ou_nulo(novo_codLinhaIntegra);
        novo_registro.codEstIntegra = inteiro_ou_nulo(novo_codEstIntegra);

        novo_registro.tamNomeEstacao = (int)strlen(novo_registro.nomeEstacao);
        novo_registro.tamNomeLinha = (int)strlen(novo_registro.nomeLinha);
        novo_registro.removido = '0';
        novo_registro.proximo = -1;

        long offset_destino;
        if (cabecalho.topo != -1)
        {
            // Reaproveita espaço de registro removido (pilha de removidos).
            int proximo_topo = -1;
            if (fseek(arquivo_bin, cabecalho.topo + sizeof(char), SEEK_SET) != 0 ||
                fread(&proximo_topo, sizeof(int), 1, arquivo_bin) != 1)
            {
                printf("%s\n", MSG_FALHA);
                fclose(arquivo_bin);
                return;
            }
            offset_destino = cabecalho.topo;
            cabecalho.topo = proximo_topo;
        }
        else
        {
            // Sem removidos disponíveis: escreve no final da área de dados.
            offset_destino = rrn_para_offset(cabecalho.proxRRN);
            cabecalho.proxRRN++;
        }

        if (fseek(arquivo_bin, offset_destino, SEEK_SET) != 0 || !escrever_registro(arquivo_bin, &novo_registro))
        {
            printf("%s\n", MSG_FALHA);
            fclose(arquivo_bin);
            return;
        }
    }

    if (!calcular_nroEstacoes_nroParesEstacoes(arquivo_bin, &cabecalho))
    {
        printf("%s\n", MSG_FALHA);
        fclose(arquivo_bin);
        return;
    }

    fechar_binario_escrita(arquivo_bin, &cabecalho);

    BinarioNaTela(nome_arquivo);
}
