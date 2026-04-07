#include "../auxiliares/auxiliar.h"

// Funcionalidade [6]: Atualiza os registros para a versão informada pelo usuário.
void atualizar_registros(char *nome_arquivo, int qtd_atualizacoes)
{
	// Validação dos parâmetros embutidos da função.
	if (nome_arquivo == NULL || qtd_atualizacoes <= 0)
	{
		printf("%s\n", MSG_FALHA);
		return;
	}

	// Abertura do arquivo binário no modo de escrita.
	FILE *arquivo_bin;
	Cabecalho cabecalho;
	if (!abrir_binario_escrita(&arquivo_bin, nome_arquivo, &cabecalho))
	{
		printf("%s\n", MSG_FALHA);
		return;
	}

	// Cada iteração trata uma operação completa de atualização: um filtro de busca acompanhado dos campos que se deseja alterar.
	for (int a = 0; a < qtd_atualizacoes; a++)
	{
		// Leitura dos critérios de busca dos campos que devem ser alterados.
		int qtd_criterios_busca = 0;
		Criterio criterios_busca[MAX_CRITERIOS];
		if (!ler_lista_criterios(criterios_busca, &qtd_criterios_busca, 1))
		{
			printf("%s\n", MSG_FALHA);
			fclose(arquivo_bin);
			return;
		}

		// Leitura dos campos específicos que devem ser atualizados, junto dos seus novos valores informados na entrada padrão.
		int qtd_campos_atualizar = 0;
		Criterio campos_atualizacao[MAX_CRITERIOS];
		if (!ler_lista_criterios(campos_atualizacao, &qtd_campos_atualizar, 1))
		{
			printf("%s\n", MSG_FALHA);
			fclose(arquivo_bin);
			return;
		}

		// Garante a leitura sequencial iniciando logo após o cabeçalho
		if (fseek(arquivo_bin, TAMANHO_CABECALHO, SEEK_SET) != 0)
		{
			printf("%s\n", MSG_FALHA);
			fclose(arquivo_bin);
			return;
		}

		// O arquivo é percorrido por completo para localizar os registros que atendem aos critérios especificados de forma algoritmica fluída e linear.
		for (int rrn = 0; rrn < cabecalho.proxRRN; rrn++)
		{
			Registro registro;
			// Guarda a localização base para aplicar uma possível substituição interna futuramente (caso modificado).
			long offset = rrn_para_offset(rrn);

			// Lê o registro atual do ponteiro de leitura mantido de modo sequencialmente nativo pelo rrn.
			int leitura = ler_registro(arquivo_bin, &registro);
			if (leitura == 0)
			{
				printf("%s\n", MSG_FALHA);
				fclose(arquivo_bin);
				return;
			}
			if (leitura == -1) // Entra aqui caso o registro analisado já esteja marcado como removido.
			{
				continue;
			}

			// Adiciona o caractere nulo '\0' ao final dos campos de tamanho variável para que se possa comparar adequadamente as strings lidas com os critérios informados
			normalizar_campos_texto_registro(&registro);

			// O registro só entra na etapa final de alteração de seus dados caso ele passe por todos os critérios de busca
			if (!registro_atende_criterios(&registro, criterios_busca, qtd_criterios_busca))
			{
				continue;
			}

			// Aqui a função aplica as mudanças estritamente sobre os campos que foram solicitados pelo usuário, de modo que não mexe no restante do registro preservado na memória ram auxiliar temporal que fará logo então a posterior transação segura para o disco
			for (int i = 0; i < qtd_campos_atualizar; i++)
			{
				aplicar_criterio_no_registro(&registro, &campos_atualizacao[i]);
			}

			// Procede então a uma validação de precaução com os escopos variáveis das strings
			if (registro.tamNomeEstacao < 0 || registro.tamNomeLinha < 0 ||
				registro.tamNomeEstacao + registro.tamNomeLinha > 43)
			{
				printf("%s\n", MSG_FALHA);
				fclose(arquivo_bin);
				return;
			}

			// Retorna o ponteiro ao início do bloco do registro contido internamente logo em seguida reescrevendo por completo a série com as devidas atualizações embutidas
			if (fseek(arquivo_bin, offset, SEEK_SET) != 0 || !escrever_registro(arquivo_bin, &registro))
			{
				printf("%s\n", MSG_FALHA);
				fclose(arquivo_bin);
				return;
			}
		}
	}

	fechar_binario_escrita(arquivo_bin, &cabecalho);

	BinarioNaTela(nome_arquivo);
}