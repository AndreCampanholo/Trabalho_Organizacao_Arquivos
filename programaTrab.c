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
    char nome_bin[TAMANHO_TEXTO];
    char indice_bin[TAMANHO_TEXTO];

    // Variáveis de quantidade declaradas no escopo do switch para evitar redeclaração
    int qtd_remocoes;
    int qtd_insercoes;
    int qtd_atualizacoes;

    switch (funcionalidade)
    {
    case 1: // func01: conversão dos dados de registro do formato .csv para .bin
        if (scanf("%255s %255s", nome_csv, nome_bin) != 2)
        {
            printf("%s\n", MSG_FALHA);
            return 0;
        }
        criar_tabela(nome_csv, nome_bin);
        break;

    case 2: // func02: impressão de registros do .bin
        if (scanf("%255s", nome_bin) != 1)
        {
            printf("%s\n", MSG_FALHA);
            return 0;
        }
        recuperar_registros(nome_bin);
        break;

    case 3: // func03: busca condicional
        if (scanf("%255s", nome_bin) != 1)
        {
            printf("%s\n", MSG_FALHA);
            return 0;
        }
        recuperar_registros_condicional(nome_bin);
        break;

    case 4: // func04: remoção de registros
        if (scanf("%255s", nome_bin) != 1)
        {
            printf("%s\n", MSG_FALHA);
            return 0;
        }
        getchar();
        scanf("%d", &qtd_remocoes);
        deletar_registros(nome_bin, qtd_remocoes);
        break;

    case 5: // func05: inserção de novos registros
        if (scanf("%255s", nome_bin) != 1)
        {
            printf("%s\n", MSG_FALHA);
            return 0;
        }
        getchar();
        scanf("%d", &qtd_insercoes);
        inserir_registros(nome_bin, qtd_insercoes);
        break;

    case 6: // func06: atualização de registros
        if (scanf("%255s", nome_bin) != 1)
        {
            printf("%s\n", MSG_FALHA);
            return 0;
        }
        getchar();
        scanf("%d", &qtd_atualizacoes);
        atualizar_registros(nome_bin, qtd_atualizacoes);
        break;

    case 7: // func07.c: criação do arquivo de índice com árvore B a partir de um .bin
        if (scanf("%255s %255s", nome_bin, indice_bin) != 2)
        {
            printf("%s\n", MSG_FALHA);
            return 0;
        }
        criar_indice(nome_bin, indice_bin);
        break;

    case 8: // func08: seleção de registros via árvore B caso a busca seja pelo codEstacao
        if (scanf("%255s %255s", nome_bin, indice_bin) != 2)
        {
            printf("%s\n", MSG_FALHA);
            return 0;
        }
        recuperar_registros_indice(nome_bin, indice_bin);
        break;

    case 9: // func09.c: insere registros no arquivo de dados e no arquivo de índice
        if (scanf("%255s %255s", nome_bin, indice_bin) != 2)
        {
            printf("%s\n", MSG_FALHA);
            return 0;
        }
        getchar();
        scanf("%d", &qtd_insercoes);
        inserir_registros_indice(nome_bin, indice_bin, qtd_insercoes);
        break;

    case 10: // func10.c: deleta logicamente registros do arquivo de dados e do arquivo de índice por meio do critério de busca da func08
        if (scanf("%255s %255s", nome_bin, indice_bin) != 2)
        {
            printf("%s\n", MSG_FALHA);
            return 0;
        }
        getchar();
        scanf("%d", &qtd_remocoes);
        deletar_registros_indice(nome_bin, indice_bin, qtd_remocoes);
        break;

    default: // Funcionalidade inválida
        printf("%s\n", MSG_FALHA);
    }
    return 0;
}