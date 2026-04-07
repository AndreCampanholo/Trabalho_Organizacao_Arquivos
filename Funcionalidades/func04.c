#include "../auxiliares/auxiliar.h"

// Funcionalidade [4]: Apaga os registros do arquivo binário de acordo com os critérios informados pelo usuário.
void deletar_registros(char *nome_arquivo, int qtd_remocoes)
{
    // Verifica se os parâmetros informados são válidos.
    if (nome_arquivo == NULL || qtd_remocoes <= 0)
    {
        printf("%s\n", MSG_FALHA);
        return;
    }

    // Abre o arquivo para leitura e escrita binária.
    FILE *arquivo_bin;
    Cabecalho cabecalho;
    if (!abrir_binario_escrita(&arquivo_bin, nome_arquivo, &cabecalho))
    {
        printf("%s\n", MSG_FALHA);
        return;
    }

    // Laço que processa cada um dos comandos de remoção que foram informados na entrada.
    int qtd_criterios;
    for (int i = 0; i < qtd_remocoes; i++)
    {
        Criterio criterios[MAX_CRITERIOS];

        // Lê os 'm' pares (nomeCampo, valorCampo) utilizando a rotina modularizada.
        if (!ler_lista_criterios(criterios, &qtd_criterios, 1))
        {
            printf("%s\n", MSG_FALHA);
            fechar_binario_escrita(arquivo_bin, &cabecalho);
            return;
        }

        // Para cada comando, posiciona o ponteiro no início dos registros apenas uma vez
        if (fseek(arquivo_bin, TAMANHO_CABECALHO, SEEK_SET) != 0)
        {
            printf("%s\n", MSG_FALHA);
            fechar_binario_escrita(arquivo_bin, &cabecalho);
            return;
        }

        for (int rrn = 0; rrn < cabecalho.proxRRN; rrn++)
        {
            long offset = rrn_para_offset(rrn); // Guardar o offset caso precisemos gravar a remoção

            // Leitura sequencial do registro.
            Registro registro;
            int leitura = ler_registro(arquivo_bin, &registro);
            if (leitura == 0)
            { // leitura == 0 indica uma falha de leitura
                printf("%s\n", MSG_FALHA);
                fechar_binario_escrita(arquivo_bin, &cabecalho);
                return;
            }
            if (leitura == -1)
            { // leitura == -1 indica que o registro já foi removido logicamente
                continue;
            }

            // Adiciona o caractere nulo '\0' ao final dos campos de tamanho variável para realizar as comparações.
            normalizar_campos_texto_registro(&registro);

            // Se o registro não atende aos critérios, ele permanece da mesma forma que está.
            if (!registro_atende_criterios(&registro, criterios, qtd_criterios))
            {
                continue; // Se o registro não deve ser removido, o resto do laço não é executado.
            }

            char removido = '1';
            int antigo_topo = cabecalho.topo;

            // Posiciona o ponteiro de volta no início do registro (o ponteiro foi para o final devido ao 'ler_registro()').
            if (fseek(arquivo_bin, offset, SEEK_SET) != 0)
            {
                printf("%s\n", MSG_FALHA);
                fechar_binario_escrita(arquivo_bin, &cabecalho);
                return;
            }

            // Realiza a remoção lógica do registro.
            if (fwrite(&removido, sizeof(char), 1, arquivo_bin) != 1)
            {
                printf("%s\n", MSG_FALHA);
                fechar_binario_escrita(arquivo_bin, &cabecalho);
                return;
            }

            // O campo 'próximo' recebe o antigo topo, mantendo a lista de removidos em um formato de pilha.
            if (fwrite(&antigo_topo, sizeof(int), 1, arquivo_bin) != 1)
            {
                printf("%s\n", MSG_FALHA);
                fechar_binario_escrita(arquivo_bin, &cabecalho);
                return;
            }

            // O registro removido agora se torna o novo topo da pilha de espaços livres.
            cabecalho.topo = rrn;

            // Retorna o ponteiro para o início do próximo registro a ser lido na sequência (offset + 80 bytes).
            if (fseek(arquivo_bin, offset + TAMANHO_REGISTRO, SEEK_SET) != 0)
            {
                printf("%s\n", MSG_FALHA);
                fechar_binario_escrita(arquivo_bin, &cabecalho);
                return;
            }
        }
    }

    // No fim, as estatísticas do cabeçalho são recalculadas para refletir o estado atualizado do arquivo.
    if (!calcular_nroEstacoes_nroParesEstacoes(arquivo_bin, &cabecalho))
    {
        printf("%s\n", MSG_FALHA);
        fechar_binario_escrita(arquivo_bin, &cabecalho);
        return;
    }

    // Define o status do arquivo como consistente novamente e o fecha.
    fechar_binario_escrita(arquivo_bin, &cabecalho);

    // Imprime o arquivo binário na tela.
    BinarioNaTela(nome_arquivo);
}