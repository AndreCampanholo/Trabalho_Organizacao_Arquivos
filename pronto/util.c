#include "util.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Reproducao da rotina de checksum usada nas disciplinas SCC0215/SCC0220.
 * A saida permite comparacao automatica do arquivo binario gerado.
 */
void binarioNaTela(const char *nomeArquivoBinario) {
    unsigned long i, soma_bytes;
    unsigned char *memoria_buffer;
    size_t tamanho_arquivo;

    FILE *arquivo_stream = fopen(nomeArquivoBinario, "rb");
    if (arquivo_stream == NULL) {
        printf("%s\n", MSG_FALHA);
        return;
    }

    fseek(arquivo_stream, 0, SEEK_END);
    tamanho_arquivo = (size_t)ftell(arquivo_stream);
    fseek(arquivo_stream, 0, SEEK_SET);

    memoria_buffer = (unsigned char *)malloc(tamanho_arquivo == 0 ? 1 : tamanho_arquivo);
    fread(memoria_buffer, 1, tamanho_arquivo, arquivo_stream);

    soma_bytes = 0;
    for (i = 0; i < tamanho_arquivo; i++) {
        soma_bytes += (unsigned long)memoria_buffer[i];
    }

    printf("%lf\n", soma_bytes / (double)100);

    free(memoria_buffer);
    fclose(arquivo_stream);
}

/*
 * Parser de entrada textual no padrao dos trabalhos:
 * - ignora espacos iniciais;
 * - converte NULO para string vazia;
 * - suporta aspas duplas para campos com espaco interno.
 */
void scan_quote_string(char *str, size_t capacidade) {
    char c;
    size_t i = 0;

    while ((c = (char)getchar()) != EOF && isspace((unsigned char)c)) {
    }

    if (c == EOF) {
        str[0] = '\0';
        return;
    }

    if (c == 'N' || c == 'n') {
        char resto[4] = {0};
        resto[0] = c;
        for (int j = 1; j < 4; j++) {
            int lido = getchar();
            resto[j] = (char)(lido == EOF ? '\0' : lido);
        }

        if ((resto[0] == 'N' || resto[0] == 'n') &&
            (resto[1] == 'U' || resto[1] == 'u') &&
            (resto[2] == 'L' || resto[2] == 'l') &&
            (resto[3] == 'O' || resto[3] == 'o')) {
            str[0] = '\0';
            return;
        }

        for (i = 0; i < 4 && i + 1 < capacidade; i++) {
            str[i] = resto[i];
        }

        while ((c = (char)getchar()) != EOF && !isspace((unsigned char)c)) {
            if (i + 1 < capacidade) {
                str[i++] = c;
            }
        }

        str[i] = '\0';
        return;
    }

    if (c == '"') {
        while ((c = (char)getchar()) != EOF && c != '"') {
            if (i + 1 < capacidade) {
                str[i++] = c;
            }
        }
        str[i] = '\0';
        return;
    }

    str[i++] = c;
    while ((c = (char)getchar()) != EOF && !isspace((unsigned char)c)) {
        if (i + 1 < capacidade) {
            str[i++] = c;
        }
    }

    str[i] = '\0';
}
