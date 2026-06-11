#include "../auxiliares/auxiliar.h"
#include "../auxiliares/bt.h"

// Funcionalidade [7]: Cria um arquivo de índice com árvore B a partir de um arquivo de dados binário
void criar_indice(char *nome_arquivo_dados, char *nome_arquivo_indice)
{
    FILE *arquivo_dados = fopen(nome_arquivo_dados, "rb");
    if (arquivo_dados == NULL)
    {
        printf("%s\n", MSG_FALHA);
        return;
    }
    FILE *arquivo_indice = fopen(nome_arquivo_indice, "wb");
    if (arquivo_indice == NULL)
    {
        fclose(arquivo_dados);
        printf("%s\n", MSG_FALHA);
        return;
    }

    CabecalhoBT bt_cabecalho;
    bt_cabecalho.status = '0';
    bt_cabecalho.noRaiz = NULO;
    bt_cabecalho.topo = NULO;
    bt_cabecalho.proxRRN = 0;
    bt_cabecalho.nroNos = 0;
    escrever_cabecalho_bt(arquivo_indice, &bt_cabecalho);

    Registro registro_lido;
    int rrn_no_arquivo_dados = 0;
    while (1)
    {
        int resultado_leitura = ler_registro(arquivo_dados, &registro_lido);
        if (resultado_leitura == 0)
            break;
        if (resultado_leitura == 1)
        {
            if (!escrever_registro_bt(arquivo_indice, &bt_cabecalho, rrn_no_arquivo_dados, registro_lido.codEstacao))
            {
                fclose(arquivo_dados);
                fclose(arquivo_indice);
                printf("%s\n", MSG_FALHA);
                return;
            }
        }
        rrn_no_arquivo_dados++;
    }

    bt_cabecalho.status = '1';
    escrever_cabecalho_bt(arquivo_indice, &bt_cabecalho);
    fclose(arquivo_dados);
    fclose(arquivo_indice);
    BinarioNaTela(nome_arquivo_indice);
    return;
}