#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

/* Mensagens padrao exigidas pelo enunciado. */
#define MSG_FALHA "Falha no processamento do arquivo."
#define MSG_INEXISTENTE "Registro inexistente."

/*
 * Calcula e imprime o checksum do arquivo binario no formato usado pelo avaliador.
 * Deve ser chamada apenas apos fechar o arquivo manipulado pela funcionalidade.
 */
void binarioNaTela(const char *nomeArquivoBinario);

/*
 * Leitura de token com suporte a:
 * - strings entre aspas duplas;
 * - marcador NULO (retorna string vazia);
 * - token simples separado por espacos.
 */
void scan_quote_string(char *str, size_t capacidade);

#endif
