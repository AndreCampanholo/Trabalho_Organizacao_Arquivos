#include <stdio.h>
#include <stdlib.h>
#include "../pilha/pilha.h"

int main() {
    FILE *arquivo_csv = fopen("estacoes.csv", "r");
    FILE *arquivo_bin = fopen("estacoes.bin", "wb+");

    if(arquivo_csv == NULL || arquivo_bin == NULL){
        if(arquivo_csv) fclose(arquivo_csv);
        if(arquivo_bin) fclose(arquivo_bin);
        printf("%s\n", "Falha no processamento do arquivo.");
        return;
    }

    
    
}