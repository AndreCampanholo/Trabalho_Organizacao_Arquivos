#include "funcionalidades.h"
#include "../auxiliares/auxiliar.h"

void ordenarArquivo(char *nome_arquivo_entrada, char *campo_ordenacao, char *nome_arquivo_ordenado, bool imprimirBinarioNaTela)
{
    if (nome_arquivo_entrada == NULL || campo_ordenacao == NULL || nome_arquivo_ordenado == NULL)
    {
        printf("%s", MSG_FALHA);
        return;
    }

    if (strcmp(campo_ordenacao, "codProxEstacao") != 0 && strcmp(campo_ordenacao, "codEstacao") != 0)
    {
        printf("%s", MSG_FALHA);
        return;
    }

    Cabecalho cabecalho_entrada;
    Cabecalho cabecalho_ordenado;

    FILE *f1, *f2;
    if (!abrir_binario(&f1, nome_arquivo_entrada, "rb", &cabecalho_entrada, 0))
    {
        printf("%s", MSG_FALHA);
        return;
    }

    if ((f2 = fopen(nome_arquivo_ordenado, "wb")) == NULL)
    {
        fclose(f1);
        printf("%s", MSG_FALHA);
        return;
    }

    cabecalho_ordenado.status = '0';
    cabecalho_ordenado.proxRRN = 0;
    cabecalho_ordenado.topo = -1;
    cabecalho_ordenado.nroEstacoes = 0;
    cabecalho_ordenado.nroParesEstacoes = cabecalho_entrada.nroParesEstacoes;
    escrever_cabecalho(f2, &cabecalho_ordenado);

    int qtd_estacoes = cabecalho_entrada.nroEstacoes;
    Registro *registros = (Registro *)calloc(qtd_estacoes, sizeof(Registro));

    int i = 0;
    while (true)
    {
        int leitura = ler_registro(f1, &registros[i]);
        if (leitura == 0)
            break;
        if (leitura == -1)
            continue;

        i++;
    }

    heap_sort(registros, campo_ordenacao, qtd_estacoes);

    fseek(f2, TAMANHO_CABECALHO, SEEK_SET);

    int escrita;
    for (int j = 0; j < i; j++)
    {
        escrita = escrever_registro(f2, &registros[j]);
        if (!escrita)
        {
            printf("%s", MSG_FALHA);
            return;
        }
    }

    if (imprimirBinarioNaTela)
        BinarioNaTela(nome_arquivo_ordenado);

    cabecalho_ordenado.proxRRN = i;
    cabecalho_ordenado.topo = -1;
    cabecalho_ordenado.nroEstacoes = i;

    free(registros);
    fclose(f1);
    fechar_binario_escrita(f2, &cabecalho_ordenado);
}