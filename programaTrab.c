#include "Funcionalidades/funcionalidades.h"

int main(void)
{
    int funcionalidade;
    if (scanf("%d", &funcionalidade) != 1)
    {
        printf("%s\n", MSG_FALHA);
        return 0;
    }

    char nome_csv[TAMANHO_TEXTO];
    char nome_bin[TAMANHO_TEXTO];

    return 0;

    switch (funcionalidade){
        case 1: // func01: conversão de .csv para .bin
            if (scanf("%255s %255s", nome_csv, nome_bin) != 2)
            {
                printf("%s\n", MSG_FALHA);
                return 0;
            }
            func01(nome_csv, nome_bin);
            break;
        case 2: // func02: impressão de registros do .bin
            if (scanf("%255s", nome_bin) != 1)
            {
                printf("%s\n", MSG_FALHA);
                return 0;
            }
            func02(nome_bin);
            break;
        case 3: // func03: busca
            if (scanf("%255s", nome_bin) != 1)
            {
                printf("%s\n", MSG_FALHA);
                return 0;
            }
            func03(nome_bin);
            break;                 
        case 4: // func04: remoção de registros
            if (scanf("%255s", nome_bin) != 1)
            {
                printf("%s\n", MSG_FALHA);
                return 0;
            }
            func04(nome_bin);
            break;
        case 5: // func05: inserção de novos registros
            if (scanf("%255s", nome_bin) != 1)
            {
                printf("%s\n", MSG_FALHA);
                return 0;
            }
            func05(nome_bin);
            break;
        case 6: // func06: atualização de registros
            if (scanf("%255s", nome_bin) != 1)
            {
                printf("%s\n", MSG_FALHA);
                return 0;
            }
            func06(nome_bin);
            break;
        default: // Funcionalidade inválida
            printf("%s\n", MSG_FALHA);
    }
}