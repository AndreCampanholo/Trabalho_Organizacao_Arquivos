#include "../auxiliares/auxiliar.h"

// Funcionalidade [4]: apaga registros do arquivo binário de acordo com critérios informados pelo usuário
void deletar_registros(char *nome_arquivo, int qtd_remocoes)
{
    // Verifica se parâmetros são válidos
    if (nome_arquivo == NULL || qtd_remocoes <= 0)
    {
        printf("%s\n", MSG_FALHA);
        return;
    }

    // Abre o arquivo para leitura e escrita binária
    FILE *arquivo_bin;
    Cabecalho cabecalho;
    if (!abrir_binario_escrita(&arquivo_bin, nome_arquivo, &cabecalho))
    {
        printf("%s\n", MSG_FALHA);
        return;
    }

    // Laço que realiza as deleções para as n remoções (qtd_remoções)
    int qtd_criterios;
    for (int i = 0; i < qtd_remocoes; i++)
    {
        Criterio criterios[MAX_CRITERIOS];

        // Le os m pares (nomeCampo, valorCampo) usando a rotina modularizada.
        if (!ler_lista_criterios(criterios, &qtd_criterios, 1))
        {
            printf("%s\n", MSG_FALHA);
            fclose(arquivo_bin);
            return;
        }

        // Laço de verificação dos critérios para cada registro e remoção lógica a partir do rrn = 0
        for (int rrn = 0; rrn < cabecalho.proxRRN; rrn++)
        {
            // Cálculo do offset do registro atual
            long offset = rrn_para_offset(rrn);

            // Posiciona ponteiro no byte offset do registro atualmente sendo avaliado
            if (fseek(arquivo_bin, offset, SEEK_SET) != 0)
            {
                printf("%s\n", MSG_FALHA);
                fclose(arquivo_bin);
                return;
            }

            // Leitura do registro
            Registro registro;
            int leitura = ler_registro(arquivo_bin, &registro);
            if (leitura == 0)
            { // leitura == 0 indica falha de leitura
                printf("%s\n", MSG_FALHA);
                fclose(arquivo_bin);
                return;
            }
            if (leitura == -1)
            { // leitura == -1 indica que o registro já está removido
                continue;
            }

            // Adiciona '\0' ao final de campos de tam. variável para comparações
            normalizar_campos_texto_registro(&registro);

            // Compara registros com os critérios de deleção
            if (!registro_atende_criterios(&registro, criterios, qtd_criterios))
            {
                continue; // Se o registro não deve ser removido, não executa resto do loop
            }

            char removido = '1';
            int antigo_topo = cabecalho.topo;

            // Posiciona o ponteiro de volta no início do registro (foi para o final devido ao 'ler_registro()')
            if (fseek(arquivo_bin, offset, SEEK_SET) != 0)
            {
                printf("%s\n", MSG_FALHA);
                fclose(arquivo_bin);
                return;
            }

            // Remoção lógica do registro
            if (fwrite(&removido, sizeof(char), 1, arquivo_bin) != 1)
            {
                printf("%s\n", MSG_FALHA);
                fclose(arquivo_bin);
                return;
            }

            // Escreve byte offset do próximo registro removido em 'proximo' (garante funcionamento de pilha)
            if (fwrite(&antigo_topo, sizeof(int), 1, arquivo_bin) != 1)
            {
                printf("%s\n", MSG_FALHA);
                fclose(arquivo_bin);
                return;
            }

            // Atualiza o valor de topo
            cabecalho.topo = (int)offset;
        }
    }

    // Calcula novo 'nroEstacoes' e 'nroParesEstacoes' atualizando seus valores na struct cabeçalho
    if (!calcular_nroEstacoes_nroParesEstacoes(arquivo_bin, &cabecalho))
    {
        printf("%s\n", MSG_FALHA);
        fclose(arquivo_bin);
        return;
    }

    // Define status como consistente e fecha arquivo
    fechar_binario_escrita(arquivo_bin, &cabecalho);

    // Imprime
    BinarioNaTela(nome_arquivo);
}