#ifndef FUNCIONALIDADES_H
#define FUNCIONALIDADES_H

#include "../auxiliares/auxiliar.h"
#include "../auxiliares/bt.h"

//* Protótipos das funcionalidades sem índice */

// Funcionalidade [1]: Cria um arquivo binário de dados a partir de um arquivo CSV.
void criar_tabela(char *nome_csv, char *nome_bin);

// Funcionalidade [2]: Exibe na tela todos os registros ativos do arquivo binário.
void recuperar_registros(char *nome_arquivo_bin);

// Funcionalidade [3]: Busca e exibe registros que correspondam aos critérios informados via busca sequencial.
void recuperar_registros_condicional(char *nome_arquivo_bin);

// Funcionalidade [4]: Remove logicamente registros por critérios, gerenciando o espaço via pilha de removidos.
void deletar_registros(char *nome_arquivo, int qtd_remocoes);

// Funcionalidade [5]: Insere novos registros reaproveitando os espaços logicamente removidos ou gravando no fim do arquivo.
void inserir_registros(char *nome_arquivo, int qtd_insercoes);

// Funcionalidade [6]: Atualiza campos específicos dos registros que correspondam aos critérios informados.
void atualizar_registros(char *nome_arquivo, int qtd_atualizacoes);

//* Protótipos das funcionalidades com índice Árvore-B

// Funcionalidade [7]: Cria um arquivo de índice Árvore-B a partir de um arquivo de dados existente.
void criar_indice(char *nome_arquivo_dados, char *nome_arquivo_indice);

// Funcionalidade [8]: Realiza a busca condicional utilizando o índice Árvore-B quando o critério envolve a chave primária.
void recuperar_registros_indice(char *nome_arquivo, char *arquivo_indice);

// Funcionalidade [9]: Insere registros no arquivo de dados (com reuso de espaço) e adiciona a nova chave na Árvore-B.
void inserir_registros_indice(char *nome_arquivo, char *arquivo_indice, int qtd_insercoes);

// Funcionalidade [10]: Remove logicamente registros do arquivo de dados e de índice de acordo com os critérios de busca informados pelo usuário    
void deletar_registros_indice(char *nome_bin, char *indice_bin, int qtd_remocoes);

// Funcionalidade [11]: A partir de dois arquivos de dados de entrada (podem ser iguais), utiliza loop aninhado para verificar a igualdade de dois registro a partir dos campos registro1.CodProxEstacao e registro2.CodEstacao e, caso positivo, imprime ambos
void juncao_loop_aninhado(char *nome_bin1, char *campo1, char *nome_bin2, char *campo2);

// Funcionalidade [12]: A partir de dois arquivos de dados de entrada (podem ser iguais), utiliza um loop único e o arquivo de índice para verificar a igualdade de dois registro a partir dos campos registro1.CodProxEstacao e registro2.CodEstacao e, caso positivo, imprime ambos
void juncao_loop_unico(char *nome_bin1, char *campo1, char *nome_bin2, char *campo2, char *nome_indice);

// Funcionalidade [13]: a partir de um arquivo de dados desordenado retornar um arquivo ordenado pelo campo desejado (codEstacao ou codProxEstacao)
void ordenarArquivo(char *nome_arquivo_entrada, char *campo_ordenacao, char *nome_arquivo_ordenado, bool imprimirBinarioNaTela);

#endif