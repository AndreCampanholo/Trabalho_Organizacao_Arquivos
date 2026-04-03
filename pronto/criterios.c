#include "criterios.h"

#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Texto vazio no arquivo representa valor nulo para campo variavel. */
static int texto_nulo(const char *valor, int tamanho) {
    return tamanho == 0 || valor[0] == '\0';
}

int campo_eh_texto(const char *nome_campo) {
    return strcmp(nome_campo, "nomeEstacao") == 0 || strcmp(nome_campo, "nomeLinha") == 0;
}

/* Mapeia nome de campo inteiro para o valor atual no registro. */
static int obter_inteiro_campo(const Registro *registro, const char *nome_campo, int *ok) {
    *ok = 1;
    if (strcmp(nome_campo, "codEstacao") == 0) return registro->codEstacao;
    if (strcmp(nome_campo, "codLinha") == 0) return registro->codLinha;
    if (strcmp(nome_campo, "codProxEstacao") == 0) return registro->codProxEstacao;
    if (strcmp(nome_campo, "distProxEstacao") == 0) return registro->distProxEstacao;
    if (strcmp(nome_campo, "codLinhaIntegra") == 0) return registro->codLinhaIntegra;
    if (strcmp(nome_campo, "codEstIntegra") == 0) return registro->codEstIntegra;
    *ok = 0;
    return 0;
}

/* Mapeia nome de campo texto para ponteiro/tamanho atual no registro. */
static const char *obter_texto_campo(const Registro *registro, const char *nome_campo, int *tamanho, int *ok) {
    *ok = 1;
    if (strcmp(nome_campo, "nomeEstacao") == 0) {
        *tamanho = registro->tamNomeEstacao;
        return registro->nomeEstacao;
    }
    if (strcmp(nome_campo, "nomeLinha") == 0) {
        *tamanho = registro->tamNomeLinha;
        return registro->nomeLinha;
    }
    *ok = 0;
    *tamanho = 0;
    return "";
}

/*
 * Converte um par textual da entrada para representacao tipada.
 * NULO eh convertido para flag ehNulo = 1.
 */
int ler_criterio(Criterio *criterio) {
    if (scanf("%31s", criterio->nome) != 1) return 0;

    char texto_lido[TAMANHO_TEXTO];
    scan_quote_string(texto_lido, sizeof(texto_lido));

    if (texto_lido[0] == '\0') {
        criterio->ehNulo = 1;
        criterio->valorInteiro = VALOR_NULO_INTEIRO;
        criterio->valorTexto[0] = '\0';
    } else {
        criterio->ehNulo = 0;
        if (campo_eh_texto(criterio->nome)) {
            strncpy(criterio->valorTexto, texto_lido, TAMANHO_TEXTO - 1);
            criterio->valorTexto[TAMANHO_TEXTO - 1] = '\0';
        } else {
            criterio->valorInteiro = atoi(texto_lido);
        }
    }

    return 1;
}

/*
 * Comparacao de todos os criterios com curto-circuito:
 * qualquer divergencia encerra com falso.
 */
int registro_atende_criterios(const Registro *registro, const Criterio *criterios, int quantidade) {
    for (int i = 0; i < quantidade; i++) {
        if (campo_eh_texto(criterios[i].nome)) {
            int tamanho = 0;
            int ok = 0;
            const char *texto = obter_texto_campo(registro, criterios[i].nome, &tamanho, &ok);
            if (!ok) return 0;

            if (criterios[i].ehNulo) {
                if (tamanho != 0) return 0;
            } else {
                if (tamanho == 0) return 0;
                if (strcmp(texto, criterios[i].valorTexto) != 0) return 0;
            }
        } else {
            int ok = 0;
            int valor = obter_inteiro_campo(registro, criterios[i].nome, &ok);
            if (!ok) return 0;

            if (criterios[i].ehNulo) {
                if (valor != VALOR_NULO_INTEIRO) return 0;
            } else {
                if (valor != criterios[i].valorInteiro) return 0;
            }
        }
    }

    return 1;
}

/*
 * Atualizacao de registro por nome de campo.
 * Para texto, atualiza tambem o campo de tamanho correspondente.
 */
void aplicar_criterio_no_registro(Registro *registro, const Criterio *criterio) {
    if (strcmp(criterio->nome, "codEstacao") == 0) {
        registro->codEstacao = criterio->ehNulo ? VALOR_NULO_INTEIRO : criterio->valorInteiro;
    } else if (strcmp(criterio->nome, "codLinha") == 0) {
        registro->codLinha = criterio->ehNulo ? VALOR_NULO_INTEIRO : criterio->valorInteiro;
    } else if (strcmp(criterio->nome, "codProxEstacao") == 0) {
        registro->codProxEstacao = criterio->ehNulo ? VALOR_NULO_INTEIRO : criterio->valorInteiro;
    } else if (strcmp(criterio->nome, "distProxEstacao") == 0) {
        registro->distProxEstacao = criterio->ehNulo ? VALOR_NULO_INTEIRO : criterio->valorInteiro;
    } else if (strcmp(criterio->nome, "codLinhaIntegra") == 0) {
        registro->codLinhaIntegra = criterio->ehNulo ? VALOR_NULO_INTEIRO : criterio->valorInteiro;
    } else if (strcmp(criterio->nome, "codEstIntegra") == 0) {
        registro->codEstIntegra = criterio->ehNulo ? VALOR_NULO_INTEIRO : criterio->valorInteiro;
    } else if (strcmp(criterio->nome, "nomeEstacao") == 0) {
        if (criterio->ehNulo) {
            registro->tamNomeEstacao = 0;
            registro->nomeEstacao[0] = '\0';
        } else {
            strncpy(registro->nomeEstacao, criterio->valorTexto, TAMANHO_TEXTO - 1);
            registro->nomeEstacao[TAMANHO_TEXTO - 1] = '\0';
            registro->tamNomeEstacao = (int)strlen(registro->nomeEstacao);
        }
    } else if (strcmp(criterio->nome, "nomeLinha") == 0) {
        if (criterio->ehNulo) {
            registro->tamNomeLinha = 0;
            registro->nomeLinha[0] = '\0';
        } else {
            strncpy(registro->nomeLinha, criterio->valorTexto, TAMANHO_TEXTO - 1);
            registro->nomeLinha[TAMANHO_TEXTO - 1] = '\0';
            registro->tamNomeLinha = (int)strlen(registro->nomeLinha);
        }
    }
}

/*
 * Formata a saida de SELECT/WHERE conforme especificacao:
 * campos nulos exibidos como NULO.
 */
void imprimir_registro(const Registro *registro) {
    printf("%d ", registro->codEstacao);

    if (texto_nulo(registro->nomeEstacao, registro->tamNomeEstacao))
        printf("NULO ");
    else
        printf("%s ", registro->nomeEstacao);

    if (registro->codLinha == VALOR_NULO_INTEIRO)
        printf("NULO ");
    else
        printf("%d ", registro->codLinha);

    if (texto_nulo(registro->nomeLinha, registro->tamNomeLinha))
        printf("NULO ");
    else
        printf("%s ", registro->nomeLinha);

    if (registro->codProxEstacao == VALOR_NULO_INTEIRO)
        printf("NULO ");
    else
        printf("%d ", registro->codProxEstacao);

    if (registro->distProxEstacao == VALOR_NULO_INTEIRO)
        printf("NULO ");
    else
        printf("%d ", registro->distProxEstacao);

    if (registro->codLinhaIntegra == VALOR_NULO_INTEIRO)
        printf("NULO ");
    else
        printf("%d ", registro->codLinhaIntegra);

    if (registro->codEstIntegra == VALOR_NULO_INTEIRO)
        printf("NULO\n");
    else
        printf("%d\n", registro->codEstIntegra);
}
