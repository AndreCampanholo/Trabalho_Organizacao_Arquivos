#include "../auxiliares/auxiliar.h"

// Funcionalidade [5]: Insere os registros informados pelo usuário.
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
            fechar_binario_escrita(arquivo_bin, &cabecalho);
            return;
        }

        ScanQuoteString(novo_registro.nomeEstacao);
        if (scanf("%4s", novo_codLinha) != 1)
        {
            printf("%s\n", MSG_FALHA);
            fechar_binario_escrita(arquivo_bin, &cabecalho);
            return;
        }

        ScanQuoteString(novo_registro.nomeLinha);

        if (scanf("%4s %4s %4s %4s", novo_codProxEstacao, novo_distProxEstacao, novo_codLinhaIntegra, novo_codEstIntegra) != 4)
        {
            printf("%s\n", MSG_FALHA);
            fechar_binario_escrita(arquivo_bin, &cabecalho);
            return;
        }

        // Os campos lidos como texto permitem que o caso de um valor nulo seja tratado corretamente.
        novo_registro.codLinha = inteiro_ou_nulo(novo_codLinha);
        novo_registro.codProxEstacao = inteiro_ou_nulo(novo_codProxEstacao);
        novo_registro.distProxEstacao = inteiro_ou_nulo(novo_distProxEstacao);
        novo_registro.codLinhaIntegra = inteiro_ou_nulo(novo_codLinhaIntegra);
        novo_registro.codEstIntegra = inteiro_ou_nulo(novo_codEstIntegra);

        novo_registro.tamNomeEstacao = (int)strlen(novo_registro.nomeEstacao);
        novo_registro.tamNomeLinha = (int)strlen(novo_registro.nomeLinha);
        novo_registro.removido = '0';
        novo_registro.proximo = -1;

        long rrn_destino;
        if (cabecalho.topo != -1)
        {
            // Se houver um espaço livre na pilha de removidos, a inserção reaproveita esse espaço primeiro.
            int proximo_topo = -1;
            rrn_destino = cabecalho.topo;
            if (fseek(arquivo_bin, rrn_para_offset(rrn_destino) + sizeof(char), SEEK_SET) != 0 ||
                fread(&proximo_topo, sizeof(int), 1, arquivo_bin) != 1)
            {
                printf("%s\n", MSG_FALHA);
                fechar_binario_escrita(arquivo_bin, &cabecalho);
                return;
            }
            cabecalho.topo = proximo_topo;
        }
        else
        {
            // Quando não há espaço livre, o novo registro vai para o final da área de dados do arquivo.
            rrn_destino = cabecalho.proxRRN;
            cabecalho.proxRRN++;
        }

        if (fseek(arquivo_bin, rrn_para_offset(rrn_destino), SEEK_SET) != 0 || !escrever_registro(arquivo_bin, &novo_registro))
        {
            printf("%s\n", MSG_FALHA);
            fechar_binario_escrita(arquivo_bin, &cabecalho);
            return;
        }
    }

    // Depois de todas as inserções, os totais agregados do cabeçalho são recalculados para manter a consistência.
    if (!calcular_nroEstacoes_nroParesEstacoes(arquivo_bin, &cabecalho))
    {
        printf("%s\n", MSG_FALHA);
        fechar_binario_escrita(arquivo_bin, &cabecalho);
        return;
    }

    fechar_binario_escrita(arquivo_bin, &cabecalho);

    BinarioNaTela(nome_arquivo);
}
