#include "funcionalidades.h"

#include "arquivo_binario.h"
#include "criterios.h"
#include "gerenciamento.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Estruturas auxiliares para contagem de chaves distintas. */
typedef struct {
    int *dados;
    int quantidade;
    int capacidade;
} VetorInt;

typedef struct {
    int origem;
    int destino;
} Par;

typedef struct {
    Par *dados;
    int quantidade;
    int capacidade;
} VetorPar;

static void vetor_int_init(VetorInt *vetor) {
    vetor->dados = NULL;
    vetor->quantidade = 0;
    vetor->capacidade = 0;
}

static int vetor_int_contem(const VetorInt *vetor, int valor) {
    for (int i = 0; i < vetor->quantidade; i++) {
        if (vetor->dados[i] == valor) return 1;
    }
    return 0;
}

static void vetor_int_adiciona(VetorInt *vetor, int valor) {
    if (vetor->quantidade == vetor->capacidade) {
        vetor->capacidade = vetor->capacidade == 0 ? 16 : vetor->capacidade * 2;
        vetor->dados = (int *)realloc(vetor->dados, (size_t)vetor->capacidade * sizeof(int));
    }
    vetor->dados[vetor->quantidade++] = valor;
}

static void vetor_par_init(VetorPar *vetor) {
    vetor->dados = NULL;
    vetor->quantidade = 0;
    vetor->capacidade = 0;
}

static int vetor_par_contem(const VetorPar *vetor, int origem, int destino) {
    for (int i = 0; i < vetor->quantidade; i++) {
        if (vetor->dados[i].origem == origem && vetor->dados[i].destino == destino) return 1;
    }
    return 0;
}

static void vetor_par_adiciona(VetorPar *vetor, int origem, int destino) {
    if (vetor->quantidade == vetor->capacidade) {
        vetor->capacidade = vetor->capacidade == 0 ? 16 : vetor->capacidade * 2;
        vetor->dados = (Par *)realloc(vetor->dados, (size_t)vetor->capacidade * sizeof(Par));
    }
    vetor->dados[vetor->quantidade].origem = origem;
    vetor->dados[vetor->quantidade].destino = destino;
    vetor->quantidade++;
}

static void remover_quebra_linha(char *texto) {
    size_t tamanho = strlen(texto);
    while (tamanho > 0 && (texto[tamanho - 1] == '\n' || texto[tamanho - 1] == '\r')) {
        texto[tamanho - 1] = '\0';
        tamanho--;
    }
}

static int parse_int_ou_nulo(const char *token) {
    if (token == NULL || token[0] == '\0') return VALOR_NULO_INTEIRO;
    return atoi(token);
}

/*
 * Parser CSV simples por virgula.
 * O dataset do trabalho nao exige tratamento de virgula escapada em aspas.
 */
static void parse_linha_csv(char *linha, char colunas[][TAMANHO_TEXTO], int *quantidade_colunas) {
    *quantidade_colunas = 0;
    remover_quebra_linha(linha);

    char *cursor = linha;
    while (*cursor && *quantidade_colunas < 16) {
        int i = 0;
        while (*cursor && *cursor != ',') {
            if (i < TAMANHO_TEXTO - 1) {
                colunas[*quantidade_colunas][i++] = *cursor;
            }
            cursor++;
        }
        colunas[*quantidade_colunas][i] = '\0';
        (*quantidade_colunas)++;
        if (*cursor == ',') cursor++;
    }

    if (linha[0] == '\0') *quantidade_colunas = 0;
}

/* Converte colunas CSV para representacao interna de registro. */
static void preencher_registro_csv(Registro *registro, char colunas[][TAMANHO_TEXTO]) {
    registro->removido = '0';
    registro->proximo = -1;

    registro->codEstacao = parse_int_ou_nulo(colunas[0]);
    registro->codLinha = parse_int_ou_nulo(colunas[2]);
    registro->codProxEstacao = parse_int_ou_nulo(colunas[4]);
    registro->distProxEstacao = parse_int_ou_nulo(colunas[5]);
    registro->codLinhaIntegra = parse_int_ou_nulo(colunas[6]);
    registro->codEstIntegra = parse_int_ou_nulo(colunas[7]);

    if (colunas[1][0] == '\0') {
        registro->tamNomeEstacao = 0;
        registro->nomeEstacao[0] = '\0';
    } else {
        strncpy(registro->nomeEstacao, colunas[1], TAMANHO_TEXTO - 1);
        registro->nomeEstacao[TAMANHO_TEXTO - 1] = '\0';
        registro->tamNomeEstacao = (int)strlen(registro->nomeEstacao);
    }

    if (colunas[3][0] == '\0') {
        registro->tamNomeLinha = 0;
        registro->nomeLinha[0] = '\0';
    } else {
        strncpy(registro->nomeLinha, colunas[3], TAMANHO_TEXTO - 1);
        registro->nomeLinha[TAMANHO_TEXTO - 1] = '\0';
        registro->tamNomeLinha = (int)strlen(registro->nomeLinha);
    }
}

void funcionalidade_1_criar_tabela(const char *nome_csv, const char *nome_binario) {
    /* Fluxo: abre arquivos, inicializa cabecalho, importa CSV e fecha consistente. */
    FILE *arquivo_csv = fopen(nome_csv, "r");
    FILE *arquivo_bin = fopen(nome_binario, "wb+");

    if (arquivo_csv == NULL || arquivo_bin == NULL) {
        if (arquivo_csv) fclose(arquivo_csv);
        if (arquivo_bin) fclose(arquivo_bin);
        printf("%s\n", MSG_FALHA);
        return;
    }

    Cabecalho cabecalho = {'0', -1, 0, 0, 0};
    escrever_cabecalho(arquivo_bin, &cabecalho);

    char linha[1024];
    if (!fgets(linha, sizeof(linha), arquivo_csv)) {
        cabecalho.status = '1';
        escrever_cabecalho(arquivo_bin, &cabecalho);
        fclose(arquivo_csv);
        fclose(arquivo_bin);
        binarioNaTela(nome_binario);
        return;
    }

    VetorInt estacoes;
    VetorPar pares;
    vetor_int_init(&estacoes);
    vetor_par_init(&pares);
    while (fgets(linha, sizeof(linha), arquivo_csv)) {
        char colunas[16][TAMANHO_TEXTO];
        int quantidade_colunas = 0;
        parse_linha_csv(linha, colunas, &quantidade_colunas);
        if (quantidade_colunas < 8) continue;

        Registro registro;
        preencher_registro_csv(&registro, colunas);

        if (!vetor_int_contem(&estacoes, registro.codEstacao)) {
            vetor_int_adiciona(&estacoes, registro.codEstacao);
        }

        if (!vetor_par_contem(&pares, registro.codEstacao, registro.codProxEstacao)) {
            vetor_par_adiciona(&pares, registro.codEstacao, registro.codProxEstacao);
        }

        escrever_registro_rrn(arquivo_bin, cabecalho.proxRRN, &registro);
        cabecalho.proxRRN++;
    }

    cabecalho.nroEstacoes = estacoes.quantidade;
    cabecalho.nroParesEstacao = pares.quantidade;
    cabecalho.status = '1';
    escrever_cabecalho(arquivo_bin, &cabecalho);

    free(estacoes.dados);
    free(pares.dados);
    fclose(arquivo_csv);
    fclose(arquivo_bin);

    binarioNaTela(nome_binario);
}

void funcionalidade_2_listar_todos(const char *nome_binario) {
    /* Leitura sequencial completa, ignorando registros removidos logicamente. */
    FILE *arquivo_bin;
    Cabecalho cabecalho;

    if (!abrir_binario_consistente(&arquivo_bin, nome_binario, "rb", &cabecalho, 0)) {
        printf("%s\n", MSG_FALHA);
        return;
    }

    int quantidade_mostrada = 0;
    for (int rrn = 0; rrn < cabecalho.proxRRN; rrn++) {
        Registro registro;
        if (!ler_registro_rrn(arquivo_bin, rrn, &registro)) {
            fclose(arquivo_bin);
            printf("%s\n", MSG_FALHA);
            return;
        }

        if (registro.removido == '1') continue;

        imprimir_registro(&registro);
        quantidade_mostrada++;
    }

    if (quantidade_mostrada == 0) {
        printf("%s\n", MSG_INEXISTENTE);
    }

    fclose(arquivo_bin);
}

void funcionalidade_3_busca_condicional(const char *nome_binario) {
    /* Para cada consulta, aplica AND entre todos os criterios informados. */
    FILE *arquivo_bin;
    Cabecalho cabecalho;

    if (!abrir_binario_consistente(&arquivo_bin, nome_binario, "rb", &cabecalho, 0)) {
        printf("%s\n", MSG_FALHA);
        return;
    }

    int quantidade_buscas;
    if (scanf("%d", &quantidade_buscas) != 1) {
        fclose(arquivo_bin);
        printf("%s\n", MSG_FALHA);
        return;
    }

    for (int b = 0; b < quantidade_buscas; b++) {
        int quantidade_criterios;
        if (scanf("%d", &quantidade_criterios) != 1 || quantidade_criterios < 0 || quantidade_criterios > MAX_CRITERIOS) {
            fclose(arquivo_bin);
            printf("%s\n", MSG_FALHA);
            return;
        }

        Criterio criterios[MAX_CRITERIOS];
        for (int i = 0; i < quantidade_criterios; i++) {
            if (!ler_criterio(&criterios[i])) {
                fclose(arquivo_bin);
                printf("%s\n", MSG_FALHA);
                return;
            }
        }

        int encontrado = 0;
        for (int rrn = 0; rrn < cabecalho.proxRRN; rrn++) {
            Registro registro;
            if (!ler_registro_rrn(arquivo_bin, rrn, &registro)) {
                fclose(arquivo_bin);
                printf("%s\n", MSG_FALHA);
                return;
            }

            if (registro.removido == '1') continue;

            if (registro_atende_criterios(&registro, criterios, quantidade_criterios)) {
                imprimir_registro(&registro);
                encontrado = 1;
            }
        }

        if (!encontrado) {
            printf("%s\n", MSG_INEXISTENTE);
        }
    }

    fclose(arquivo_bin);
}

void funcionalidade_4_remocao_logica(const char *nome_binario) {
    /* Remove logicamente e empilha para reaproveitamento futuro. */
    FILE *arquivo_bin;
    Cabecalho cabecalho;

    if (!abrir_binario_consistente(&arquivo_bin, nome_binario, "rb+", &cabecalho, 1)) {
        printf("%s\n", MSG_FALHA);
        return;
    }

    int quantidade_remocoes;
    if (scanf("%d", &quantidade_remocoes) != 1) {
        fclose(arquivo_bin);
        printf("%s\n", MSG_FALHA);
        return;
    }

    for (int r = 0; r < quantidade_remocoes; r++) {
        int quantidade_criterios;
        if (scanf("%d", &quantidade_criterios) != 1 || quantidade_criterios < 0 || quantidade_criterios > MAX_CRITERIOS) {
            fclose(arquivo_bin);
            printf("%s\n", MSG_FALHA);
            return;
        }

        Criterio criterios[MAX_CRITERIOS];
        for (int i = 0; i < quantidade_criterios; i++) {
            if (!ler_criterio(&criterios[i])) {
                fclose(arquivo_bin);
                printf("%s\n", MSG_FALHA);
                return;
            }
        }

        for (int rrn = 0; rrn < cabecalho.proxRRN; rrn++) {
            Registro registro;
            if (!ler_registro_rrn(arquivo_bin, rrn, &registro)) {
                fclose(arquivo_bin);
                printf("%s\n", MSG_FALHA);
                return;
            }

            if (registro.removido == '1') continue;

            if (registro_atende_criterios(&registro, criterios, quantidade_criterios)) {
                empilhar_removido(arquivo_bin, &cabecalho, rrn);
            }
        }
    }

    if (!recalcular_contadores(arquivo_bin, &cabecalho)) {
        fclose(arquivo_bin);
        printf("%s\n", MSG_FALHA);
        return;
    }

    cabecalho.status = '1';
    escrever_cabecalho(arquivo_bin, &cabecalho);
    fclose(arquivo_bin);

    binarioNaTela(nome_binario);
}

static void ler_registro_insercao(Registro *registro) {
    /* Leitura no formato da especificacao: 8 campos na ordem definida. */
    char token[TAMANHO_TEXTO];

    scan_quote_string(token, sizeof(token));
    registro->codEstacao = token[0] == '\0' ? VALOR_NULO_INTEIRO : atoi(token);

    scan_quote_string(token, sizeof(token));
    if (token[0] == '\0') {
        registro->tamNomeEstacao = 0;
        registro->nomeEstacao[0] = '\0';
    } else {
        strncpy(registro->nomeEstacao, token, TAMANHO_TEXTO - 1);
        registro->nomeEstacao[TAMANHO_TEXTO - 1] = '\0';
        registro->tamNomeEstacao = (int)strlen(registro->nomeEstacao);
    }

    scan_quote_string(token, sizeof(token));
    registro->codLinha = token[0] == '\0' ? VALOR_NULO_INTEIRO : atoi(token);

    scan_quote_string(token, sizeof(token));
    if (token[0] == '\0') {
        registro->tamNomeLinha = 0;
        registro->nomeLinha[0] = '\0';
    } else {
        strncpy(registro->nomeLinha, token, TAMANHO_TEXTO - 1);
        registro->nomeLinha[TAMANHO_TEXTO - 1] = '\0';
        registro->tamNomeLinha = (int)strlen(registro->nomeLinha);
    }

    scan_quote_string(token, sizeof(token));
    registro->codProxEstacao = token[0] == '\0' ? VALOR_NULO_INTEIRO : atoi(token);

    scan_quote_string(token, sizeof(token));
    registro->distProxEstacao = token[0] == '\0' ? VALOR_NULO_INTEIRO : atoi(token);

    scan_quote_string(token, sizeof(token));
    registro->codLinhaIntegra = token[0] == '\0' ? VALOR_NULO_INTEIRO : atoi(token);

    scan_quote_string(token, sizeof(token));
    registro->codEstIntegra = token[0] == '\0' ? VALOR_NULO_INTEIRO : atoi(token);

    registro->removido = '0';
    registro->proximo = -1;
}

void funcionalidade_5_insercao(const char *nome_binario) {
    /*
     * Estrategia de alocacao:
     * 1) tenta reutilizar topo da pilha de removidos;
     * 2) se vazia, usa proxRRN.
     */
    FILE *arquivo_bin;
    Cabecalho cabecalho;

    if (!abrir_binario_consistente(&arquivo_bin, nome_binario, "rb+", &cabecalho, 1)) {
        printf("%s\n", MSG_FALHA);
        return;
    }

    int quantidade_insercoes;
    if (scanf("%d", &quantidade_insercoes) != 1) {
        fclose(arquivo_bin);
        printf("%s\n", MSG_FALHA);
        return;
    }

    for (int i = 0; i < quantidade_insercoes; i++) {
        Registro registro;
        ler_registro_insercao(&registro);

        int rrn_destino = desempilhar_removido(arquivo_bin, &cabecalho);
        if (rrn_destino == -1) {
            rrn_destino = cabecalho.proxRRN;
            cabecalho.proxRRN++;
        }

        escrever_registro_rrn(arquivo_bin, rrn_destino, &registro);
    }

    if (!recalcular_contadores(arquivo_bin, &cabecalho)) {
        fclose(arquivo_bin);
        printf("%s\n", MSG_FALHA);
        return;
    }

    cabecalho.status = '1';
    escrever_cabecalho(arquivo_bin, &cabecalho);
    fclose(arquivo_bin);

    binarioNaTela(nome_binario);
}

void funcionalidade_6_atualizacao(const char *nome_binario) {
    /* Atualizacao in-place, pois o tamanho fisico do registro e fixo (80 bytes). */
    FILE *arquivo_bin;
    Cabecalho cabecalho;

    if (!abrir_binario_consistente(&arquivo_bin, nome_binario, "rb+", &cabecalho, 1)) {
        printf("%s\n", MSG_FALHA);
        return;
    }

    int quantidade_atualizacoes;
    if (scanf("%d", &quantidade_atualizacoes) != 1) {
        fclose(arquivo_bin);
        printf("%s\n", MSG_FALHA);
        return;
    }

    for (int a = 0; a < quantidade_atualizacoes; a++) {
        int quantidade_criterios_busca;
        if (scanf("%d", &quantidade_criterios_busca) != 1 || quantidade_criterios_busca < 0 || quantidade_criterios_busca > MAX_CRITERIOS) {
            fclose(arquivo_bin);
            printf("%s\n", MSG_FALHA);
            return;
        }

        Criterio criterios_busca[MAX_CRITERIOS];
        for (int i = 0; i < quantidade_criterios_busca; i++) {
            if (!ler_criterio(&criterios_busca[i])) {
                fclose(arquivo_bin);
                printf("%s\n", MSG_FALHA);
                return;
            }
        }

        int quantidade_campos_atualizar;
        if (scanf("%d", &quantidade_campos_atualizar) != 1 || quantidade_campos_atualizar < 0 || quantidade_campos_atualizar > MAX_CRITERIOS) {
            fclose(arquivo_bin);
            printf("%s\n", MSG_FALHA);
            return;
        }

        Criterio campos_atualizacao[MAX_CRITERIOS];
        for (int i = 0; i < quantidade_campos_atualizar; i++) {
            if (!ler_criterio(&campos_atualizacao[i])) {
                fclose(arquivo_bin);
                printf("%s\n", MSG_FALHA);
                return;
            }
        }

        for (int rrn = 0; rrn < cabecalho.proxRRN; rrn++) {
            Registro registro;
            if (!ler_registro_rrn(arquivo_bin, rrn, &registro)) {
                fclose(arquivo_bin);
                printf("%s\n", MSG_FALHA);
                return;
            }

            if (registro.removido == '1') continue;
            if (!registro_atende_criterios(&registro, criterios_busca, quantidade_criterios_busca)) continue;

            for (int i = 0; i < quantidade_campos_atualizar; i++) {
                aplicar_criterio_no_registro(&registro, &campos_atualizacao[i]);
            }

            escrever_registro_rrn(arquivo_bin, rrn, &registro);
        }
    }

    if (!recalcular_contadores(arquivo_bin, &cabecalho)) {
        fclose(arquivo_bin);
        printf("%s\n", MSG_FALHA);
        return;
    }

    cabecalho.status = '1';
    escrever_cabecalho(arquivo_bin, &cabecalho);
    fclose(arquivo_bin);

    binarioNaTela(nome_binario);
}
