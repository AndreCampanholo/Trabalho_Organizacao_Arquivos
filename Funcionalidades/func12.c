#include "../headers/auxiliares.h"
#include "../headers/funcionalidades.h"

void juncao_loop_unico(char *nome_bin1, char *campo1, char *nome_bin2, char *campo2, char *nome_indice)
{
    if (nome_bin1 == NULL || campo1 == NULL || nome_bin2 == NULL || campo2 == NULL || nome_indice == NULL)
    {
        printf("%s\n", MSG_FALHA);
        return;
    }

    if (strcmp(campo1, "codProxEstacao") != 0 || strcmp(campo2, "codEstacao") != 0)
    {
        printf("%s\n", MSG_FALHA);
        return;
    }

    Cabecalho cabecalho_arquivo1;
    Cabecalho cabecalho_arquivo2;
    CabecalhoBT cabecalho_indice;

    // Caso seja informado o mesmo arquivo (autojunção), abre-se duas vezes para manter os ponteiros de leitura de cada lado da junção independentes entre si
    FILE *f1, *f2, *f3;
    if (!abrir_binario(&f1, nome_bin1, "rb", &cabecalho_arquivo1, 0))
    {
        printf("%s\n", MSG_FALHA);
        return;
    }
    if (!abrir_binario(&f2, nome_bin2, "rb", &cabecalho_arquivo2, 0))
    {
        fclose(f1);
        printf("%s\n", MSG_FALHA);
        return;
    }
    if (!abrir_binario_bt(&f3, nome_indice, "rb", &cabecalho_indice, 0))
    {
        fclose(f1);
        fclose(f2);
        printf("%s\n", MSG_FALHA);
        return;
    }

    Registro registro_arquivo1;
    Registro registro_proxEstacao;
    bool encontrou = false;

    while (true)
    {
        int leitura1 = ler_registro(f1, &registro_arquivo1);
        if (leitura1 == 0) // fim do arquivo
            break;
        if (leitura1 == -1) // registro logicamente removido (pula para o próximo)
            continue;

        // Busca no arquivo de índice pelo codProxEstacao do registro_arquivo1
        // Como codEstacao é uma chave primária única em arquivo2, há, no máximo, um resultado.
        int offset_encontrado_indice = recuperar_registro_indice(f3, &cabecalho_indice, registro_arquivo1.codProxEstacao);
        if (offset_encontrado_indice != NULO)
        {
            encontrou = true;
            fseek(f2, offset_encontrado_indice, SEEK_SET);
            ler_registro(f2, &registro_proxEstacao);
            normalizar_campos_texto_registro(&registro_arquivo1); // Adiciona '/0' ao final dos campos textuais (Strings)
            normalizar_campos_texto_registro(&registro_proxEstacao);
            printf("%d %s %s %d %s\n", registro_arquivo1.codEstacao, registro_arquivo1.nomeEstacao, registro_arquivo1.nomeLinha, registro_arquivo1.codProxEstacao, registro_proxEstacao.nomeEstacao);
        }
    }

    // Se nenhum registro obedece à condição de junção, imprime mensagem de erro
    if (!encontrou)
        printf("%s\n", MSG_INEXISTENTE);

    fclose(f1);
    fclose(f2);
    fclose(f3);
}