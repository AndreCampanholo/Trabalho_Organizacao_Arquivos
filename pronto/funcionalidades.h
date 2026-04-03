#ifndef FUNCIONALIDADES_H
#define FUNCIONALIDADES_H

/* [1] CREATE TABLE: CSV -> binario de dados. */
void funcionalidade_1_criar_tabela(const char *nome_csv, const char *nome_binario);

/* [2] SELECT *: listagem completa de registros nao removidos. */
void funcionalidade_2_listar_todos(const char *nome_binario);

/* [3] SELECT ... WHERE: busca por um ou mais criterios. */
void funcionalidade_3_busca_condicional(const char *nome_binario);

/* [4] DELETE logico com reaproveitamento dinamico por pilha. */
void funcionalidade_4_remocao_logica(const char *nome_binario);

/* [5] INSERT com uso preferencial de espaco removido. */
void funcionalidade_5_insercao(const char *nome_binario);

/* [6] UPDATE por criterio de busca e campos de atualizacao. */
void funcionalidade_6_atualizacao(const char *nome_binario);

#endif
