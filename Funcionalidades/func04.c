#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../auxiliares/auxiliar.h"

// Verifica se um registro deve ser removido ou não
bool registro_atende_criterios(Registro *registro, int qtd_criterios, char nome_campo[][32], char valor_campo[][45], int valor_campo_int[]) {
    for (int k = 0; k < qtd_criterios; k++) {
        // Se 'nome_campo' for de um campo de tamanho variável, entra
        if (campo_eh_texto(nome_campo[k])) {
            int tamanho = 0;
            int eh_valido = 0; 
            // Retorna o valor textual do registro de tamanho variável
            char *valor_registro = obter_campos_textos(registro, nome_campo[k], &tamanho, &eh_valido);

            // Se 'nome_campo' é inválido ou o valor do registro é diferente do valor informado, retorna false (não remove)
            if (!eh_valido || strcmp(valor_registro, valor_campo[k]) != 0) {
                return false;
            }
        //Se 'nome_campo' for de um campo de tamanho fixo, entra
        } else {
            int eh_valido = 0;
            // Retorna valor inteiro do registro de tamanho fixo
            int inteiro_registro = obter_campos_inteiros(registro, nome_campo[k], &eh_valido);

            // Se 'nome_campo' é inválido ou o valor do registro é diferente do valor informado, retorna false (não remove)
            if (!eh_valido || inteiro_registro != valor_campo_int[k]) {
                return false;
            }
        }
    }

    // Caso o registro atenda aos critérios, retorna true (será removido)
    return true;
}

void deletar_registros(char *nome_arquivo, int qtd_remocoes) {
    // Verifica se parâmetros são válidos
    if (nome_arquivo == NULL || qtd_remocoes <= 0) {
        printf("%s\n", MSG_FALHA);
        return;
    }

    // Abre o arquivo para leitura e escrita binária
    FILE *arquivo_bin = fopen(nome_arquivo, "r+b");
    if (arquivo_bin == NULL) {
        printf("%s\n", MSG_FALHA);
        return;
    }

    // Lê o cabeçalho do arquivo binário
    Cabecalho cabecalho;
    if (!ler_cabecalho(arquivo_bin, &cabecalho)) {
        printf("%s\n", MSG_FALHA);
        fclose(arquivo_bin);
        return;
    }

    // Verifica se o arquivo estava inconsistente, fechando-o em caso afirmativo
    if (cabecalho.status != '1') {
        printf("%s\n", MSG_FALHA);
        fclose(arquivo_bin);
        return;
    }

    // Atualiza o status do cabeçalho ('0' pois haverá escrita);
    cabecalho.status = '0';
    escrever_cabecalho(arquivo_bin, &cabecalho);

    // Laço que realiza as deleções para as n remoções (qtd_remoções)
    int qtd_criterios;
    for (int i = 0; i < qtd_remocoes; i++) {
        // Lê m para as n remoções
        if (scanf("%d", &qtd_criterios) != 1 || qtd_criterios < 0 || qtd_criterios > MAX_CRITERIOS) {
            printf("%s\n", MSG_FALHA);
            fclose(arquivo_bin);
            return;
        }

        // Variáveis auxiliares
        char nome_campo[MAX_CRITERIOS][32];
        char valor_campo[MAX_CRITERIOS][45];
        int valor_campo_int[MAX_CRITERIOS];

        // Lê o os m nomes e valores de campos dos critérios de remoção
        for (int j = 0; j < qtd_criterios; j++) {
            if (scanf("%31s", nome_campo[j]) != 1) {
                printf("%s\n", MSG_FALHA);
                fclose(arquivo_bin);
                return;
            }
            if (campo_eh_texto(nome_campo[j])) {
                ScanQuoteString(valor_campo[j]); // Caso campo seja textual, lê valor com ""
            } else {
                if (scanf("%d", &valor_campo_int[j]) != 1) {
                    printf("%s\n", MSG_FALHA);
                    fclose(arquivo_bin);
                    return;
                }
            }
        }

        // Laço de verificação dos critérios para cada registro e remoção lógica a partir do rrn = 0
        for (int rrn = 0; rrn < cabecalho.proxRRN; rrn++) {
            // Cálculo do offset do registro atual
            long offset = rrn_para_offset(rrn);

            // Posiciona ponteiro no byte offset do registro atualmente sendo avaliado
            if (fseek(arquivo_bin, offset, SEEK_SET) != 0) {
                printf("%s\n", MSG_FALHA);
                fclose(arquivo_bin);
                return;
            }

            // Leitura do registro
            Registro registro;
            int leitura = ler_registro(arquivo_bin, &registro);
            if (leitura == 0) { // leitura == 0 indica falha de leitura
                printf("%s\n", MSG_FALHA);
                fclose(arquivo_bin);
                return;
            } 
            if (leitura == -1) { // leitura == -1 indica que o registro já está removido
                continue;
            }

            // Adiciona '\0' ao final de campos de tam. variável para comparações
            if (registro.tamNomeEstacao >= 0 && registro.tamNomeEstacao < TAMANHO_CAMPO_VARIAVEL) {
                registro.nomeEstacao[registro.tamNomeEstacao] = '\0';
            }
            if (registro.tamNomeLinha >= 0 && registro.tamNomeLinha < TAMANHO_CAMPO_VARIAVEL) {
                registro.nomeLinha[registro.tamNomeLinha] = '\0';
            }

            // Compara registros com os critérios de deleção
            if (!registro_atende_criterios(&registro, qtd_criterios, nome_campo, valor_campo, valor_campo_int)) {
                continue; // Se o registro não deve ser removido, não executa resto do loop
            }

            char removido = '1';
            int antigo_topo = cabecalho.topo;

            // Posiciona o ponteiro de volta no início do registro (foi para o final devido ao 'ler_registro()')
            if (fseek(arquivo_bin, offset, SEEK_SET) != 0) {
                printf("%s\n", MSG_FALHA);
                fclose(arquivo_bin);
                return;
            }

            // Remoção lógica do registro
            if (fwrite(&removido, sizeof(char), 1, arquivo_bin) != 1) {
                printf("%s\n", MSG_FALHA);
                fclose(arquivo_bin);
                return;
            }

            // Escreve byte offset do próximo registro removido em 'proximo' (garante funcionamento de pilha)
            if (fwrite(&antigo_topo, sizeof(int), 1, arquivo_bin) != 1) {
                printf("%s\n", MSG_FALHA);
                fclose(arquivo_bin);
                return;
            }

            // Atualiza o valor de topo
            cabecalho.topo = (int)offset;
        }
    }

    // Calcula novo 'nroEstacoes' e 'nroParesEstacoes' atualizando seus valores na struct cabeçalho
    if (!calcular_nroEstacoes_nroParesEstacoes(arquivo_bin, &cabecalho)) {
        printf("%s\n", MSG_FALHA);
        fclose(arquivo_bin);
        return;
    }

    // Define status como consistente e escreve no cabeçalho (topo, nroEstacoes e nroParesEstacoes já foram ajustados)
    cabecalho.status = '1';
    escrever_cabecalho(arquivo_bin, &cabecalho);

    // Fecha arquivo aberto
    fclose(arquivo_bin);
    
    // Imprime
    BinarioNaTela(nome_arquivo);
}