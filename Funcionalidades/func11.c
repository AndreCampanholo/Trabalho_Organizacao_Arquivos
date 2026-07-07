#include "../headers/funcionalidades.h"
#include "../headers/auxiliares.h"

void juncao_loop_aninhado(char *nome_bin1, char *campo1, char *nome_bin2, char *campo2)
{
    if (nome_bin1 == NULL || campo1 == NULL || nome_bin2 == NULL || campo2 == NULL)
    {
        printf("%s", MSG_FALHA);
        return;
    }

    if (strcmp(campo1, "codProxEstacao") != 0 || strcmp(campo2, "codEstacao") != 0)
    {
        printf("%s", MSG_FALHA);
        return;
    }

    Cabecalho cabecalho1;
    Cabecalho cabecalho2;

    // Caso seja informado o mesmo arquivo (autojunção), abre-se duas vezes para manter os ponteiros de leitura de cada lado da junção independentes entre si
    FILE *f1, *f2;
    if (!abrir_binario(&f1, nome_bin1, "rb", &cabecalho1, 0))
    {
        printf("%s\n", MSG_FALHA);
        return;
    }
    if (!abrir_binario(&f2, nome_bin2, "rb", &cabecalho2, 0))
    {
        fclose(f1);
        printf("%s\n", MSG_FALHA);
        return;
    }

    Registro registro_arquivo1, registro_arquivo2;
    bool encontrou = false;

    while (true)
    {
        int leitura1 = ler_registro(f1, &registro_arquivo1);
        if (leitura1 == 0) // fim do arquivo
            break;
        if (leitura1 == -1) // registro logicamente removido (pula para o próximo)
            continue;

        // Reinicia o loop interno do início da seção de dados a cada registro externo
        fseek(f2, TAMANHO_CABECALHO, SEEK_SET);

        while (true)
        {
            int leitura2 = ler_registro(f2, &registro_arquivo2);
            if (leitura2 == 0)
                break; // idem acima
            if (leitura2 == -1)
                continue;

            if (registro_arquivo1.codProxEstacao == registro_arquivo2.codEstacao)
            {
                encontrou = true;
                normalizar_campos_texto_registro(&registro_arquivo1); // Adiciona '/0' ao final dos campos textuais (Strings)
                normalizar_campos_texto_registro(&registro_arquivo2);
                printf("%d %s %s %d %s\n", registro_arquivo1.codEstacao, registro_arquivo1.nomeEstacao, registro_arquivo1.nomeLinha, registro_arquivo1.codProxEstacao, registro_arquivo2.nomeEstacao);
            }
        }
    }

    // Se nenhum registro obedece à condição de junção, imprime mensagem de erro
    if (!encontrou)
        printf("%s", MSG_INEXISTENTE);

    fclose(f1);
    fclose(f2);
}