#include "funcionalidades.h"
#include "../auxiliares/auxiliar.h"

// Funcionalidade [14]: Une dois arquivos via junção ordenação-intercalação: ordena ambos os arquivos de dados informados de acordo com os campos de ordenação e intercala os registros
void juncao_ordenacao_intercalacao(char *nome_bin1, char *campo1, char *nome_bin2, char *campo2) {
    if(nome_bin1 == NULL || nome_bin2 == NULL || campo1 == NULL || campo2 == NULL) {
        printf("%s", MSG_FALHA);
        return;
    }

    if(strcmp(campo1, "codProxEstacao") != 0 || strcmp(campo2, "codEstacao") != 0) {
        printf("%s", MSG_FALHA);
        return;
    }

    Cabecalho cabecalho1, cabecalho2;

    FILE *f1, *f2;
    if(!abrir_binario(&f1, nome_bin1, "rb", &cabecalho1, 0)) {
        printf("%s", MSG_FALHA);
        return;
    } 
    if(!abrir_binario(&f2, nome_bin2, "rb", &cabecalho2, 0)) {
        fclose(f1);
        printf("%s", MSG_FALHA);
        return;
    }

    
}