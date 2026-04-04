#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "criterios.h"
#include "auxiliar.h"

/* Campo de texto é considerado nulo quando o tamanho == 0 (ou string vazia). */
bool campo_nulo(char *valor, int tamanho){
    return tamanho == 0 || valor[0] == '\0';
}

/* Identifica se o campo corresponde a uma string do registro. */
bool campo_eh_texto(char *nome_campo){
    return strcmp(nome_campo, "nomeEstacao") == 0 || strcmp(nome_campo, "nomeLinha") == 0;
}

// Busca o valor de um campo inteiro no registro
int obter_campos_inteiros(Registro *registro, char *nome_campo, int *eh_valido){
    *eh_valido = 1;
    if (strcmp(nome_campo, "codEstacao") == 0) return registro->codEstacao;
    if (strcmp(nome_campo, "codLinha") == 0) return registro->codLinha;
    if (strcmp(nome_campo, "codProxEstacao") == 0) return registro->codProxEstacao;
    if (strcmp(nome_campo, "distProxEstacao") == 0) return registro->distProxEstacao;
    if (strcmp(nome_campo, "codLinhaIntegra") == 0) return registro->codLinhaIntegra;
    if (strcmp(nome_campo, "codEstIntegra") == 0) return registro->codEstIntegra;
    *eh_valido = 0;
    return 0;
}

// Busca o valor de um campo de texto no registro
// Retorna o ponteiro para a string e seu tamanho. Se o campo não for válido, retorna uma string vazia e tamanho 0.
char *obter_campos_textos(Registro *registro, char *nome_campo, int *tamanho, int *eh_valido){
    *eh_valido = 1;
    if(strcmp(nome_campo, "nomeEstacao") == 0) {
        *tamanho = registro->tamNomeEstacao;
        return registro->nomeEstacao;}
    if(strcmp(nome_campo, "nomeLinha") == 0){
        *tamanho = registro->tamNomeLinha;
        return registro->nomeLinha;
    }

    *eh_valido = 0;
    *tamanho = 0;
    return "";
}

int ler_criterio(Criterio *criterio) {
    if (scanf("%31s", criterio->nome) != 1) return 0;

    char str_campo_texto[TAMANHO_TEXTO];
    scan_quote_string(str_campo_texto, sizeof(str_campo_texto));

    if (str_campo_texto[0] == '\0') {
        criterio->ehNulo = 1;
        criterio->valorInteiro = -1; // Valor arbitrário para indicar nulo em campos inteiros;
        criterio->valorTexto[0] = '\0';
    } else {
        criterio->ehNulo = 0;
        if (campo_eh_texto(criterio->nome)) {
            strncpy(criterio->valorTexto, str_campo_texto, TAMANHO_TEXTO - 1);
            criterio->valorTexto[TAMANHO_TEXTO - 1] = '\0';
        } else {
            criterio->valorInteiro = atoi(str_campo_texto);
        }
    }
    return 1;
}

