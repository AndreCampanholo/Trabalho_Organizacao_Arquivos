#include "../headers/funcionalidades.h"
#include "../headers/auxiliares.h"

void criar_indice(char *nome_arquivo_dados, char *nome_arquivo_indice)
{
    FILE *arquivo_dados;
    Cabecalho cabecalho_dados;
    if (!abrir_binario(&arquivo_dados, nome_arquivo_dados, "rb", &cabecalho_dados, 0))
    {
        printf("%s\n", MSG_FALHA);
        return;
    }

    FILE *arquivo_indice = fopen(nome_arquivo_indice, "wb+");
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
    int rrn_atual = 0;
    while (1)
    {
        int resultado_leitura = ler_registro(arquivo_dados, &registro_lido);
        if (resultado_leitura == 0)
            break;
        if (resultado_leitura == 1)
        {
            if (!escrever_registro_bt(arquivo_indice, &bt_cabecalho, rrn_para_offset(rrn_atual), registro_lido.codEstacao))
            {
                fclose(arquivo_dados);
                fclose(arquivo_indice);
                printf("%s\n", MSG_FALHA);
                return;
            }
        }
        rrn_atual++;
    }

    bt_cabecalho.status = '1';
    escrever_cabecalho_bt(arquivo_indice, &bt_cabecalho);
    fclose(arquivo_dados);
    fclose(arquivo_indice);
    BinarioNaTela(nome_arquivo_indice);
    return;
}