#include "funcionalidades.h"
#include "../auxiliares/auxiliar.h"
#include "../auxiliares/bt.h"

// Funcionalidade [12]: A partir de dois arquivos de dados de entrada (podem ser iguais), verifica a igualdade de dois registro a partir dos campos registro1.CodProxEstacao e registro2.CodEstacao e, caso positivo, imprime ambos
void juncao_loop_unico(char *nome_bin1, char *campo1, char *nome_bin2, char *campo2, char *nome_indice)
{
    Cabecalho cabecalho_arquivo1;
    Cabecalho cabecalho_arquivo2;
    CabecalhoBT cabecalho_indice;

    // Caso seja informado o mesmo arquivo, abrir duas vezes para manter os ponteiros de leitura independentes.
    FILE *f1, *f2, *f3;
    abrir_binario(&f1, nome_bin1, "rb", &cabecalho_arquivo1, 0);
    abrir_binario(&f2, nome_bin2, "rb", &cabecalho_arquivo2, 0);
    abrir_binario_bt(&f3, nome_indice, "rb", &cabecalho_indice, 0);


    Registro registro_arquivo1;
    Registro registro_proxEstacao;
    bool encontrou = false;

    while (true)
    {
        int leitura1 = ler_registro(f1, &registro_arquivo1);
        if (leitura1 == 0)
            break;
        if (leitura1 == -1)
            continue;

        int offset_encontrado_indice = recuperar_registro_indice(f3, &cabecalho_indice, registro_arquivo1.codProxEstacao);
        if (offset_encontrado_indice != NULO)
        {
            encontrou = true;
            fseek(f2, offset_encontrado_indice, SEEK_SET);
            ler_registro(f2, &registro_proxEstacao);
            normalizar_campos_texto_registro(&registro_arquivo1);
            normalizar_campos_texto_registro(&registro_proxEstacao);
            printf("%d %s %s %d %s", registro_arquivo1.codEstacao, registro_arquivo1.nomeEstacao, registro_arquivo1.nomeLinha, registro_arquivo1.codProxEstacao, registro_proxEstacao.nomeEstacao);
        }
    }

    if (!encontrou)
        printf("%s", MSG_INEXISTENTE);

    fclose(f1);
    fclose(f2);
    fclose(f3);
}