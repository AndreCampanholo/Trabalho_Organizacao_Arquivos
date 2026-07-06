#include "funcionalidades.h"
#include "../auxiliares/auxiliar.h"

void juncao_ordenacao_intercalacao(char *nome_bin1, char *campo1, char *nome_bin2, char *campo2)
{
    if (nome_bin1 == NULL || nome_bin2 == NULL || campo1 == NULL || campo2 == NULL)
    {
        printf("%s", MSG_FALHA);
        return;
    }

    Cabecalho cabecalho1, cabecalho2;
    FILE *f1, *f2;
    if (!abrir_binario(&f1, nome_bin1, "rb", &cabecalho1, 0))
    {
        printf("%s", MSG_FALHA);
        return;
    }
    if (!abrir_binario(&f2, nome_bin2, "rb", &cabecalho2, 0))
    {
        fclose(f1);
        printf("%s", MSG_FALHA);
        return;
    }
    fclose(f1);
    fclose(f2);

    // Gera as versões ordenadas em arquivos temporários, sem imprimir (imprimirBinarioNaTela = false)
    ordenarArquivo(nome_bin1, campo1, "arquivo1_ordenado", false);
    ordenarArquivo(nome_bin2, campo2, "arquivo2_ordenado", false);

    // Consequência de decisão de projeto: ordenarArquivo já leu e ordenou esses registros na memória internamente, mas os libera antes de retornar. Por isso, é preciso relê-los do disco para ter o vetor ordenado disponível na memória para o merge-join
    int n1, n2;
    Registro *regs1 = carregar_registros("arquivo1_ordenado", &n1);
    if (n1 == -1)
    {
        printf("%s", MSG_FALHA);
        return;
    }
    Registro *regs2 = carregar_registros("arquivo2_ordenado", &n2);
    if (n2 == -1)
    {
        free(regs1);
        printf("%s", MSG_FALHA);
        return;
    }

    // Merge-join: percorre os dois vetores ordenados simultaneamente com dois ponteiros (i para regs1, j para regs2), avançando sempre o lado de menor chave
    // Quando as chaves empatam, delimita o grupo inteiro de ocorrências consecutivas daquela chave em cada lado ([inicioI,fimI) e [inicioJ,fimJ)) e gera as combinações entre os dois grupos
    bool encontrou = false;
    int i = 0, j = 0;
    while (i < n1 && j < n2)
    {
        if (regs1[i].codProxEstacao < regs2[j].codEstacao)
        {
            i++; // avança só o lado A
        }
        else if (regs1[i].codProxEstacao > regs2[j].codEstacao)
        {
            j++; // avança só o lado B
        }
        else
        {
            int chave = regs1[i].codProxEstacao;

            int inicioI = i;
            while (i < n1 && regs1[i].codProxEstacao == chave)
                i++;
            int fimI = i;

            int inicioJ = j;
            while (j < n2 && regs2[j].codEstacao == chave)
                j++;
            int fimJ = j;

            for (int a = inicioI; a < fimI; a++)
            {
                for (int b = inicioJ; b < fimJ; b++)
                {
                    encontrou = true;
                    Registro registro1 = regs1[a];
                    Registro registro2 = regs2[b];
                    normalizar_campos_texto_registro(&registro1);
                    normalizar_campos_texto_registro(&registro2);
                    printf("%d %s %s %d %s\n", registro1.codEstacao, registro1.nomeEstacao,
                           registro1.nomeLinha, registro1.codProxEstacao, registro2.nomeEstacao);
                }
            }
        }
    }

    if (!encontrou)
        printf("%s", MSG_INEXISTENTE);

    free(regs1);
    free(regs2);

    if (remove("arquivo1_ordenado") != 0 || remove("arquivo2_ordenado") != 0)
    {
        printf("%s", MSG_FALHA);
        return;
    }
}