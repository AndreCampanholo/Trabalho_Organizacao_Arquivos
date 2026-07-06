// André Campanholo Paschoalini - 14558061
// Eduardo Poltroniere da Silva - 16862892

#include "funcionalidades/funcionalidades.h"

int main(void)
{
    // Leitura da funcionalidade desejada
    int funcionalidade;
    if (scanf("%d", &funcionalidade) != 1)
    {
        printf("%s\n", MSG_FALHA);
        return 0;
    }
    getchar();

    char nome_csv[TAMANHO_TEXTO];

    char nome_bin1[TAMANHO_TEXTO];
    char nome_bin2[TAMANHO_TEXTO];

    char indice_bin[TAMANHO_TEXTO];

    char campo1[TAMANHO_TEXTO], campo2[TAMANHO_TEXTO];

    int qtd_remocoes;
    int qtd_insercoes;
    int qtd_atualizacoes;

    switch (funcionalidade)
    {
    case 1: // func01: conversão dos dados de registro do formato .csv para .bin
        if (scanf("%127s %127s", nome_csv, nome_bin1) != 2)
        {
            printf("%s\n", MSG_FALHA);
            return 0;
        }
        criar_tabela(nome_csv, nome_bin1);
        break;

    case 2: // func02: impressão dos registros do .bin
        if (scanf("%127s", nome_bin1) != 1)
        {
            printf("%s\n", MSG_FALHA);
            return 0;
        }
        recuperar_registros(nome_bin1);
        break;

    case 3: // func03: busca condicional
        if (scanf("%127s", nome_bin1) != 1)
        {
            printf("%s\n", MSG_FALHA);
            return 0;
        }
        recuperar_registros_condicional(nome_bin1);
        break;

    case 4: // func04: remoção de registros
        if (scanf("%127s", nome_bin1) != 1)
        {
            printf("%s\n", MSG_FALHA);
            return 0;
        }
        getchar();
        scanf("%d", &qtd_remocoes);
        deletar_registros(nome_bin1, qtd_remocoes);
        break;

    case 5: // func05: inserção de novos registros
        if (scanf("%127s", nome_bin1) != 1)
        {
            printf("%s\n", MSG_FALHA);
            return 0;
        }
        getchar();
        scanf("%d", &qtd_insercoes);
        inserir_registros(nome_bin1, qtd_insercoes);
        break;

    case 6: // func06: atualização de registros
        if (scanf("%127s", nome_bin1) != 1)
        {
            printf("%s\n", MSG_FALHA);
            return 0;
        }
        getchar();
        scanf("%d", &qtd_atualizacoes);
        atualizar_registros(nome_bin1, qtd_atualizacoes);
        break;

    case 7: // func07.c: criação do arquivo de índice com árvore B a partir de um .bin
        if (scanf("%127s %127s", nome_bin1, indice_bin) != 2)
        {
            printf("%s\n", MSG_FALHA);
            return 0;
        }
        criar_indice(nome_bin1, indice_bin);
        break;

    case 8: // func08: seleção de registros via árvore B caso a busca seja pelo codEstacao
        if (scanf("%127s %127s", nome_bin1, indice_bin) != 2)
        {
            printf("%s\n", MSG_FALHA);
            return 0;
        }
        recuperar_registros_indice(nome_bin1, indice_bin);
        break;

    case 9: // func09.c: insere registros no arquivo de dados e no arquivo de índice
        if (scanf("%127s %127s", nome_bin1, indice_bin) != 2)
        {
            printf("%s\n", MSG_FALHA);
            return 0;
        }
        getchar();
        scanf("%d", &qtd_insercoes);
        inserir_registros_indice(nome_bin1, indice_bin, qtd_insercoes);
        break;

    case 10: // func10.c: deleta logicamente registros do arquivo de dados e do arquivo de índice por meio do critério de busca da func08
        if (scanf("%127s %127s", nome_bin1, indice_bin) != 2)
        {
            printf("%s\n", MSG_FALHA);
            return 0;
        }
        getchar();
        scanf("%d", &qtd_remocoes);
        deletar_registros_indice(nome_bin1, indice_bin, qtd_remocoes);
        break;

    case 11: // func11.c: junção de loop aninhado (força bruta) entre dois arquivos de dados
        if (scanf("%127s %127s %127s %127s", nome_bin1, campo1, nome_bin2, campo2) != 4)
        {
            printf("%s\n", MSG_FALHA);
            return 0;
        }
        if (strcmp(campo1, "codProxEstacao") != 0 || strcmp(campo2, "codEstacao") != 0)
        {
            printf("%s", MSG_FALHA);
            return 0;
        }

        juncao_loop_aninhado(nome_bin1, campo1, nome_bin2, campo2);
        break;

    case 12: // func12.c: junção de loop único, usando o índice árvore-B de arquivo2
        if (scanf("%127s %127s %127s %127s %127s", nome_bin1, campo1, nome_bin2, campo2, indice_bin) != 5)
        {
            printf("%s\n", MSG_FALHA);
            return 0;
        }

        if (strcmp(campo1, "codProxEstacao") != 0 || strcmp(campo2, "codEstacao") != 0)
        {
            printf("%s", MSG_FALHA);
            return 0;
        }

        juncao_loop_unico(nome_bin1, campo1, nome_bin2, campo2, indice_bin);
        break;

    case 13: // func13.c: ordenação de um arquivo de dados na RAM e gravação do resultado no disco
    {
        char arq_entrada[TAMANHO_TEXTO];
        char campo_ordenacao[TAMANHO_TEXTO];
        char arq_ordenado[TAMANHO_TEXTO];

        if (scanf("%127s %127s %127s", arq_entrada, campo_ordenacao, arq_ordenado) != 3)
        {
            printf("%s\n", MSG_FALHA);
            return 0;
        }

        if (strcmp(campo_ordenacao, "codProxEstacao") != 0 && strcmp(campo_ordenacao, "codEstacao") != 0)
        {
            printf("%s", MSG_FALHA);
            return 0;
        }

        ordenarArquivo(arq_entrada, campo_ordenacao, arq_ordenado, 1);
        break;
    }

    case 14: // func14.c: junção ordenação-intercalação (merge-join) entre dois arquivos de dados
        if (scanf("%127s %127s %127s %127s", nome_bin1, campo1, nome_bin2, campo2) != 4)
        {
            printf("%s\n", MSG_FALHA);
            return 0;
        }

        if (strcmp(campo1, "codProxEstacao") != 0 || strcmp(campo2, "codEstacao") != 0)
        {
            printf("%s", MSG_FALHA);
            return 0;
        }

        juncao_ordenacao_intercalacao(nome_bin1, campo1, nome_bin2, campo2);
        break;

    default: // Funcionalidade inválida
        printf("%s\n", MSG_FALHA);
    }
    return 0;
}