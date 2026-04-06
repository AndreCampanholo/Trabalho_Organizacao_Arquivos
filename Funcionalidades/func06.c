#include "../auxiliares/auxiliar.h"

// Funcionalidade [6]: Atualiza registros para versão informada pelo usuário
void atualizar_registros(char *nome_arquivo, int qtd_atualizacoes)
{
	// Validação dos parâmetros da função
	if (nome_arquivo == NULL || qtd_atualizacoes <= 0)
	{
		printf("%s\n", MSG_FALHA);
		return;
	}

	// Abertura do arquivo binário para escrita
	FILE *arquivo_bin;
	Cabecalho cabecalho;
	if (!abrir_binario_escrita(&arquivo_bin, nome_arquivo, &cabecalho))
	{
		printf("%s\n", MSG_FALHA);
		return;
	}

	// Laço de leitura dos critérios e execução das atualizações
	for (int a = 0; a < qtd_atualizacoes; a++)
	{
		// Leitura de critérios de busca dos campos que devem ser alterados
		int qtd_criterios_busca = 0;
		Criterio criterios_busca[MAX_CRITERIOS];
		if (!ler_lista_criterios(criterios_busca, &qtd_criterios_busca, 1))
		{
			printf("%s\n", MSG_FALHA);
			fclose(arquivo_bin);
			return;
		}

		// Leitura dos campos que devem ser atualizados e seus novos valores (informados pelo usuário)
		int qtd_campos_atualizar = 0;
		Criterio campos_atualizacao[MAX_CRITERIOS];
		if (!ler_lista_criterios(campos_atualizacao, &qtd_campos_atualizar, 1))
		{
			printf("%s\n", MSG_FALHA);
			fclose(arquivo_bin);
			return;
		}

		// Laço que verifica se registro do RRN atual atenda a critérios
		for (int rrn = 0; rrn < cabecalho.proxRRN; rrn++)
		{
			Registro registro;
			long offset = rrn_para_offset(rrn);

			// Posiciona ponteiro no byte offset do registro do RRN atual
			if (fseek(arquivo_bin, offset, SEEK_SET) != 0)
			{
				printf("%s\n", MSG_FALHA);
				fclose(arquivo_bin);
				return;
			}

			// Lê registro
			int leitura = ler_registro(arquivo_bin, &registro);
			if (leitura == 0)
			{
				printf("%s\n", MSG_FALHA);
				fclose(arquivo_bin);
				return;
			}
			if (leitura == -1) // Entra aqui caso registro esteja removido
			{
				continue;
			}

			// Adiciona '\0' ao final de campos de tamanho variável para realização de comparações de string
			normalizar_campos_texto_registro(&registro);

			// Verifica se registro atende aos critérios, pulando laço de atualização caso não
			if (!registro_atende_criterios(&registro, criterios_busca, qtd_criterios_busca))
			{
				// printf("%s\n", MSG_FALHA);
				continue;
			}

			// Laço de atualização dos campos com seus novos valores (apenas muda campos da struct 'registro')
			for (int i = 0; i < qtd_campos_atualizar; i++)
			{
				aplicar_criterio_no_registro(&registro, &campos_atualizacao[i]);
			}

			// Validação dos campos de tamanho variável
			if (registro.tamNomeEstacao < 0 || registro.tamNomeLinha < 0 ||
				registro.tamNomeEstacao + registro.tamNomeLinha > 43)
			{
				printf("%s\n", MSG_FALHA);
				fclose(arquivo_bin);
				return;
			}

			// Retorna ponteiro ao início do registro e escreve atualizações no arquivo binário
			if (fseek(arquivo_bin, offset, SEEK_SET) != 0 || !escrever_registro(arquivo_bin, &registro))
			{
				printf("%s\n", MSG_FALHA);
				fclose(arquivo_bin);
				return;
			}
		}
	}

	// Atualiza 'nroEstacoes' e 'nroParesEstacoes'
	if (!calcular_nroEstacoes_nroParesEstacoes(arquivo_bin, &cabecalho))
	{
		printf("%s\n", MSG_FALHA);
		fclose(arquivo_bin);
		return;
	}

	// Fechar o arquivo
	fechar_binario_escrita(arquivo_bin, &cabecalho);

	// Imprime
	BinarioNaTela(nome_arquivo);
}