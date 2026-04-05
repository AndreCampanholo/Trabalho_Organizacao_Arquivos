#include "../auxiliares/auxiliar.h"

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

        Criterio criterios[MAX_CRITERIOS];

<<<<<<< HEAD
        // Lê os m critérios de remoção com a modularização do criterios.c
=======
        // Le os m pares (nomeCampo, valorCampo) usando a rotina modularizada.
>>>>>>> 2beda9e7d54c264e0442befbd00a51a6f1e3a960
        for (int j = 0; j < qtd_criterios; j++) {
            if (!ler_criterio(&criterios[j])) {
                printf("%s\n", MSG_FALHA);
                fclose(arquivo_bin);
                return;
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
            if (!registro_atende_criterios(&registro, criterios, qtd_criterios)) {
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