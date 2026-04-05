#ifndef FUNCIONALIDADES_H
    #define FUNCIONALIDADE_H

    #define TAMANHO_CABECALHO 17
    #define TAMANHO_REGISTRO 80
    #define TAMANHO_CAMPO_VARIAVEL 44
    #define TAMANHO_TEXTO 128

    // Limite máximo de pares (nomeCampo, valorCampo) que podem ser informados como critérios por vez
    #define MAX_CRITERIOS 8

    #define FLAG_CAMPO_NULO -1

    #define MSG_FALHA "Falha no processamento do arquivo."
    #define MSG_INEXISTENTE "Registro inexistente."

    typedef struct cabecalho Cabecalho;
    typedef struct registro Registro;
    typedef struct estacoesVistas EstacoesVistas;

    void criar_tabela(char *nome_csv, char *nome_bin);
    void recuperar_registros(char *nome_arquivo_bin);
    void recuperar_registros_condicional(char *nome_arquivo_bin);
    void deletar_registros(char *nome_arquivo, int qtd_remocoes);
    void inserir_registros(char *nome_arquivo, int qtd_insercoes);

#endif