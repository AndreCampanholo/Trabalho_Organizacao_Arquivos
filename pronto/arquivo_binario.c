#include "arquivo_binario.h"

#include <stdio.h>

/* Primeiro byte de registro RRN i apos o cabecalho fixo. */
long rrn_para_offset(int rrn) {
    return TAMANHO_CABECALHO + (long)rrn * TAMANHO_REGISTRO;
}

/*
 * Campos fixos + campos variaveis efetivamente presentes.
 * O restante do registro de 80 bytes e preenchido com '$'.
 */
int tamanho_registro_usado(const Registro *registro) {
    return 37 + registro->tamNomeEstacao + registro->tamNomeLinha;
}

/* Serializacao fixa do cabecalho: ordem deve respeitar a especificacao. */
void escrever_cabecalho(FILE *arquivo, const Cabecalho *cabecalho) {
    fseek(arquivo, 0, SEEK_SET);
    fwrite(&cabecalho->status, sizeof(char), 1, arquivo);
    fwrite(&cabecalho->topo, sizeof(int), 1, arquivo);
    fwrite(&cabecalho->proxRRN, sizeof(int), 1, arquivo);
    fwrite(&cabecalho->nroEstacoes, sizeof(int), 1, arquivo);
    fwrite(&cabecalho->nroParesEstacao, sizeof(int), 1, arquivo);
}

/* Desserializacao do cabecalho; falha se qualquer campo nao puder ser lido. */
int ler_cabecalho(FILE *arquivo, Cabecalho *cabecalho) {
    fseek(arquivo, 0, SEEK_SET);
    if (fread(&cabecalho->status, sizeof(char), 1, arquivo) != 1) return 0;
    if (fread(&cabecalho->topo, sizeof(int), 1, arquivo) != 1) return 0;
    if (fread(&cabecalho->proxRRN, sizeof(int), 1, arquivo) != 1) return 0;
    if (fread(&cabecalho->nroEstacoes, sizeof(int), 1, arquivo) != 1) return 0;
    if (fread(&cabecalho->nroParesEstacao, sizeof(int), 1, arquivo) != 1) return 0;
    return 1;
}

/*
 * Escrita campo a campo para garantir layout exato em disco.
 * Ao final, completa bytes restantes com '$' para evitar lixo indefinido.
 */
void escrever_registro_rrn(FILE *arquivo, int rrn, const Registro *registro) {
    fseek(arquivo, rrn_para_offset(rrn), SEEK_SET);

    fwrite(&registro->removido, sizeof(char), 1, arquivo);
    fwrite(&registro->proximo, sizeof(int), 1, arquivo);
    fwrite(&registro->codEstacao, sizeof(int), 1, arquivo);
    fwrite(&registro->codLinha, sizeof(int), 1, arquivo);
    fwrite(&registro->codProxEstacao, sizeof(int), 1, arquivo);
    fwrite(&registro->distProxEstacao, sizeof(int), 1, arquivo);
    fwrite(&registro->codLinhaIntegra, sizeof(int), 1, arquivo);
    fwrite(&registro->codEstIntegra, sizeof(int), 1, arquivo);

    fwrite(&registro->tamNomeEstacao, sizeof(int), 1, arquivo);
    if (registro->tamNomeEstacao > 0) {
        fwrite(registro->nomeEstacao, sizeof(char), (size_t)registro->tamNomeEstacao, arquivo);
    }

    fwrite(&registro->tamNomeLinha, sizeof(int), 1, arquivo);
    if (registro->tamNomeLinha > 0) {
        fwrite(registro->nomeLinha, sizeof(char), (size_t)registro->tamNomeLinha, arquivo);
    }

    int bytes_usados = tamanho_registro_usado(registro);
    char lixo = '$';
    for (int i = bytes_usados; i < TAMANHO_REGISTRO; i++) 
        fwrite(&lixo, sizeof(char), 1, arquivo);
}

/*
 * Leitura campo a campo com validacao de tamanhos variaveis.
 * O fseek final avanca ate o inicio do proximo registro fisico (80 bytes).
 */
int ler_registro_rrn(FILE *arquivo, int rrn, Registro *registro) {
    fseek(arquivo, rrn_para_offset(rrn), SEEK_SET);

    if (fread(&registro->removido, sizeof(char), 1, arquivo) != 1) return 0;
    if (fread(&registro->proximo, sizeof(int), 1, arquivo) != 1) return 0;
    if (fread(&registro->codEstacao, sizeof(int), 1, arquivo) != 1) return 0;
    if (fread(&registro->codLinha, sizeof(int), 1, arquivo) != 1) return 0;
    if (fread(&registro->codProxEstacao, sizeof(int), 1, arquivo) != 1) return 0;
    if (fread(&registro->distProxEstacao, sizeof(int), 1, arquivo) != 1) return 0;
    if (fread(&registro->codLinhaIntegra, sizeof(int), 1, arquivo) != 1) return 0;
    if (fread(&registro->codEstIntegra, sizeof(int), 1, arquivo) != 1) return 0;
    if (fread(&registro->tamNomeEstacao, sizeof(int), 1, arquivo) != 1) return 0;

    if (registro->tamNomeEstacao < 0 || registro->tamNomeEstacao >= TAMANHO_TEXTO) return 0;
    if (registro->tamNomeEstacao > 0) {
        if (fread(registro->nomeEstacao, sizeof(char), (size_t)registro->tamNomeEstacao, arquivo) !=
            (size_t)registro->tamNomeEstacao)
            return 0;
    }
    registro->nomeEstacao[registro->tamNomeEstacao] = '\0';

    if (fread(&registro->tamNomeLinha, sizeof(int), 1, arquivo) != 1) return 0;
    if (registro->tamNomeLinha < 0 || registro->tamNomeLinha >= TAMANHO_TEXTO) return 0;
    if (registro->tamNomeLinha > 0) {
        if (fread(registro->nomeLinha, sizeof(char), (size_t)registro->tamNomeLinha, arquivo) !=
            (size_t)registro->tamNomeLinha)
            return 0;
    }
    registro->nomeLinha[registro->tamNomeLinha] = '\0';

    int bytes_usados = tamanho_registro_usado(registro);
    if (bytes_usados > TAMANHO_REGISTRO) return 0;
    fseek(arquivo, TAMANHO_REGISTRO - bytes_usados, SEEK_CUR);
    return 1;
}

/*
 * Guarda de consistencia global do arquivo.
 * Operacoes de escrita devem iniciar com status '0' e finalizar com '1'.
 */
int abrir_binario_consistente(FILE **arquivo, const char *nome_arquivo, const char *modo, Cabecalho *cabecalho, int eh_escrita) {
    *arquivo = fopen(nome_arquivo, modo);
    if (*arquivo == NULL) return 0;

    if (!ler_cabecalho(*arquivo, cabecalho)) {
        fclose(*arquivo);
        return 0;
    }

    if (cabecalho->status != '1') {
        fclose(*arquivo);
        return 0;
    }

    if (eh_escrita) {
        cabecalho->status = '0';
        escrever_cabecalho(*arquivo, cabecalho);
    }

    return 1;
}