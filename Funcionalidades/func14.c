#include "../headers/funcionalidades.h"
#include "../headers/auxiliares.h"

void juncao_ordenacao_intercalacao(char *nome_bin1, char *campo1, char *nome_bin2, char *campo2)
{
    if (nome_bin1 == NULL || nome_bin2 == NULL || campo1 == NULL || campo2 == NULL)
    {
        printf("%s\n", MSG_FALHA);
        return;
    }

    if (strcmp(campo1, "codProxEstacao") != 0 || strcmp(campo2, "codEstacao") != 0)
    {
        printf("%s\n", MSG_FALHA);
        return;
    }

    bool ok;

    // Gera os arquivos temporários ordenados. A impressão do BinarioNaTela e das mensagens de erro é desabilitada, deixando a responsabilidade pelo tratamento de falhas para a funcionalidade [14] e evitando mensagens duplicadas
    ordenar_arquivo(nome_bin1, campo1, "arq1_ord.bin", false, false, &ok);

    if (!ok)
    {
        printf("%s\n", MSG_FALHA);
        return;
    }
    ordenar_arquivo(nome_bin2, campo2, "arq2_ord.bin", false, false, &ok);

    if (!ok)
    {
        printf("%s\n", MSG_FALHA);
        return;
    }

    // A funcionalidade [13] gera arquivos binários ordenados e libera a memória utilizada. Assim, para executar o merge-join, os arquivos ordenados são carregados novamente para a RAM. Essa decisão de projeto mantém a funcionalidade [13] independente, ao custo de uma leitura adicional em disco
    int n1, n2;
    Registro *regs1 = carregar_registros("arq1_ord.bin", &n1);
    if (n1 == -1)
    {
        printf("%s\n", MSG_FALHA);
        return;
    }
    Registro *regs2 = carregar_registros("arq2_ord.bin", &n2);
    if (n2 == -1)
    {
        free(regs1);
        printf("%s\n", MSG_FALHA);
        return;
    }

    // Merge-join: percorre os dois vetores ordenados simultaneamente com dois ponteiros (i para regs1, j para regs2), avançando sempre o lado de menor chave
    // Quando as chaves empatam, delimita o grupo inteiro de ocorrências consecutivas daquela chave em cada lado ([inicioI,fimI]) e ([inicioJ,fimJ]) e gera as combinações entre os dois grupos
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
        printf("%s\n", MSG_INEXISTENTE);

    free(regs1);
    free(regs2);

    remove("arq1_ord.bin");
    remove("arq2_ord.bin");
}