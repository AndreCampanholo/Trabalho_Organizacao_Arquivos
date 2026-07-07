#include "../headers/funcionalidades.h"
#include "../headers/auxiliares.h"

void ordenar_arquivo(char *nome_arquivo_entrada, char *campo_ordenacao, char *nome_arquivo_ordenado, bool imprimirBinarioNaTela, bool imprimir_erro, bool *sucesso)
{
    *sucesso = false;

    if (nome_arquivo_entrada == NULL || campo_ordenacao == NULL || nome_arquivo_ordenado == NULL)
    {
        if (imprimir_erro)
            printf("%s\n", MSG_FALHA);
        return;
    }

    if (strcmp(campo_ordenacao, "codProxEstacao") != 0 && strcmp(campo_ordenacao, "codEstacao") != 0)
    {
        if (imprimir_erro)
            printf("%s\n", MSG_FALHA);
        return;
    }

    Cabecalho cabecalho_entrada;
    Cabecalho cabecalho_ordenado;

    FILE *f1, *f2;
    if (!abrir_binario(&f1, nome_arquivo_entrada, "rb", &cabecalho_entrada, 0))
    {
        if (imprimir_erro)
            printf("%s\n", MSG_FALHA);
        return;
    }
    // f1 só foi aberto para o cabecalho_entrada e já é fechado, pois carregar_registros reabre o arquivo por conta própria para ler os registros
    fclose(f1);

    if ((f2 = fopen(nome_arquivo_ordenado, "wb")) == NULL)
    {
        if (imprimir_erro)
            printf("%s\n", MSG_FALHA);
        return;
    }

    // Escreve um cabeçalho provisório (status '0' = inconsistente) só para reservar o espaço no início do arquivo. Ressalta-se que os valores reais de proxRRN/nroEstacoes são gravados mais abaixo, antes de fechar o arquivo definitivamente
    cabecalho_ordenado = cabecalho_entrada;
    cabecalho_ordenado.status = '0';
    cabecalho_ordenado.topo = -1;
    cabecalho_ordenado.proxRRN = 0;
    escrever_cabecalho(f2, &cabecalho_ordenado);

    // Lê o arquivo inteiro (registros ativos) para a RAM
    int qtd_lida;
    Registro *registros = carregar_registros(nome_arquivo_entrada, &qtd_lida);
    if (qtd_lida == -1)
    {
        if (imprimir_erro)
            printf("%s\n", MSG_FALHA);
        fclose(f2);
        remove(nome_arquivo_ordenado);
        return;
    }

    // Ordena a partir do campo pedido, usando o heap sort
    heap_sort(registros, campo_ordenacao, qtd_lida);

    // Posiciona o ponteiro no início da seção de dados
    fseek(f2, TAMANHO_CABECALHO, SEEK_SET);

    // Escreve os registros já ordenados de volta no arquivo de saída
    int escrita;
    for (int j = 0; j < qtd_lida; j++)
    {
        escrita = escrever_registro(f2, &registros[j]);
        if (!escrita)
        {
            if (imprimir_erro)
                printf("%s\n", MSG_FALHA);
            free(registros);
            fclose(f2);
            remove(nome_arquivo_ordenado);
            return;
        }
    }

    cabecalho_ordenado.proxRRN = qtd_lida;
    cabecalho_ordenado.topo = -1;

    free(registros);
    fechar_binario_escrita(f2, &cabecalho_ordenado); // fecha o arquivo redefinindo o cabecalho para impressão do BinarioNaTela

    // A impressão só ocorre quando chamada diretamente pela funcionalidade [13]
    if (imprimirBinarioNaTela)
        BinarioNaTela(nome_arquivo_ordenado);

    *sucesso = true;
}