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

    switch (funcionalidade){
        case 1: // func01: conversão de .csv para .bin
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
        case 3: // func03: busca
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
            int qtd_remocoes;
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
            int qtd_insercoes;
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
            int qtd_atualizacoes;
            scanf("%d", &qtd_atualizacoes);
            atualizar_registros(nome_bin, qtd_atualizacoes);
            break;
        default: // Funcionalidade inválida
            printf("%s\n", MSG_FALHA);
    }

    return 0;
}