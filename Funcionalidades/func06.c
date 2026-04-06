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

	// Cada iteracao trata uma operacao completa de atualizacao: filtro de busca + campos para alterar.
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

		// O arquivo e percorrido por completo para localizar os registros que atendem aos criterios.
		for (int rrn = 0; rrn < cabecalho.proxRRN; rrn++)
		{
			Registro registro;
			// A atualizacao e in-place: o registro e lido, alterado em memoria e gravado no mesmo offset.
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

			// Adiciona '\0' ao final de campos de tamanho variável para comparar strings
			normalizar_campos_texto_registro(&registro);

			// O registro so entra na etapa de alteracao se ele passar em todos os criterios de busca.
			if (!registro_atende_criterios(&registro, criterios_busca, qtd_criterios_busca))
			{
				continue;
			}

			// Aqui a funcao aplica apenas os campos pedidos pelo usuario, sem mexer no restante do registro.
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

			// Retorna o ponteiro ao início do registro e escreve as atualizações no arquivo binário
			if (fseek(arquivo_bin, offset, SEEK_SET) != 0 || !escrever_registro(arquivo_bin, &registro))
			{
				printf("%s\n", MSG_FALHA);
				fclose(arquivo_bin);
				return;
			}
		}
	}

	// Ao final, as contagens agregadas do cabecalho sao recalculadas para manter a consistencia.
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