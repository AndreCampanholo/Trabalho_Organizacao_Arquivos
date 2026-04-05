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
}