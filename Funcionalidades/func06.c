#include "../auxiliares/auxiliar.h"

void atualizar_registros(char *nome_arquivo, int qtd_atualizacoes)
{
	if (nome_arquivo == NULL || qtd_atualizacoes <= 0)
	{
		printf("%s\n", MSG_FALHA);
		return;
	}

	FILE *arquivo_bin;
	Cabecalho cabecalho;
	if (!abrir_binario_escrita(&arquivo_bin, nome_arquivo, &cabecalho))
	{
		printf("%s\n", MSG_FALHA);
		return;
	}

	for (int a = 0; a < qtd_atualizacoes; a++)
	{
		int qtd_criterios_busca = 0;
		Criterio criterios_busca[MAX_CRITERIOS];
		if (!ler_lista_criterios(criterios_busca, &qtd_criterios_busca, 1))
		{
			printf("%s\n", MSG_FALHA);
			fclose(arquivo_bin);
			return;
		}

		int qtd_campos_atualizar = 0;
		Criterio campos_atualizacao[MAX_CRITERIOS];
		if (!ler_lista_criterios(campos_atualizacao, &qtd_campos_atualizar, 1))
		{
			printf("%s\n", MSG_FALHA);
			fclose(arquivo_bin);
			return;
		}

		for (int rrn = 0; rrn < cabecalho.proxRRN; rrn++)
		{
			Registro registro;
			long offset = rrn_para_offset(rrn);

			if (fseek(arquivo_bin, offset, SEEK_SET) != 0)
			{
				printf("%s\n", MSG_FALHA);
				fclose(arquivo_bin);
				return;
			}

			int leitura = ler_registro(arquivo_bin, &registro);
			if (leitura == 0)
			{
				printf("%s\n", MSG_FALHA);
				fclose(arquivo_bin);
				return;
			}
			if (leitura == -1)
			{
				continue;
			}

			normalizar_campos_texto_registro(&registro);

			if (!registro_atende_criterios(&registro, criterios_busca, qtd_criterios_busca))
			{
				continue;
			}

			for (int i = 0; i < qtd_campos_atualizar; i++)
			{
				aplicar_criterio_no_registro(&arquivo_bin, &registro, &campos_atualizacao[i]);
			}

			if (registro.tamNomeEstacao < 0 || registro.tamNomeLinha < 0 ||
				registro.tamNomeEstacao + registro.tamNomeLinha > 43)
			{
				printf("%s\n", MSG_FALHA);
				fclose(arquivo_bin);
				return;
			}

			if (fseek(arquivo_bin, offset, SEEK_SET) != 0 || !escrever_registro(arquivo_bin, &registro))
			{
				printf("%s\n", MSG_FALHA);
				fclose(arquivo_bin);
				return;
			}
		}
	}

	if (!calcular_nroEstacoes_nroParesEstacoes(arquivo_bin, &cabecalho))
	{
		printf("%s\n", MSG_FALHA);
		fclose(arquivo_bin);
		return;
	}

	fechar_binario_escrita(arquivo_bin, &cabecalho);

	BinarioNaTela(nome_arquivo);
}