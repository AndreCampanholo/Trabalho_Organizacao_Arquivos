#ifndef FUNCIONALIDADES_H
#define FUNCIONALIDADES_H

#include "../auxiliares/auxiliar.h"

// Protótipos das funções de cada funcionalidade

void criar_tabela(char *nome_csv, char *nome_bin);
void recuperar_registros(char *nome_arquivo_bin);
void recuperar_registros_condicional(char *nome_arquivo_bin);
void deletar_registros(char *nome_arquivo, int qtd_remocoes);
void inserir_registros(char *nome_arquivo, int qtd_insercoes);
void atualizar_registros(char *nome_arquivo, int qtd_atualizacoes);

#endif