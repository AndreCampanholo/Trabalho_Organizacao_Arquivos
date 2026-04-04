#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../pilha/pilha.h"

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

    switch (funcionalidade)
    {
    case 1:
        criar_tabela(nome_csv, nome_bin);
        break;
        
    }
}