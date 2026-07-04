#include "funcionalidades.h"
#include "../auxiliares/auxiliar.h"
#include "../auxiliares/bt.h"

// Funcionalidade [12]: A partir de dois arquivos de dados de entrada (podem ser iguais), verifica a igualdade de dois registro a partir dos campos registro1.CodProxEstacao e registro2.CodEstacao e, caso positivo, imprime ambos
void juncao_loop_unico(char *nome_bin1, char *campo1, char *indice_bin, char *campo2)
{
    Cabecalho *cabecalho_dados;
    CabecalhoBT *cabecalho_indice;
    int offset_atual = 0;

    // Caso seja informado o mesmo arquivo, abrir apenas uma vez, caso contrário abrir ambos
    FILE *f1, *f2;
    abrir_binario(&f1, nome_bin1, "rb", cabecalho_dados, 0);
    offset_atual = 17;
    abrir_binario(&f2, indice_bin, "rb", cabecalho_indice, 0);

    Registro registro_arquivo1;
    Registro registro_proxEstacao;
    bool encontrou = false;
    
    while(f1 != EOF) {
        ler_registro(f1, &registro_arquivo1);
        offset_atual += 80;
        int offset_encontrado_indice = recuperar_registro_indice(f2, cabecalho_indice, registro_arquivo1.codProxEstacao);
        if(offset_encontrado_indice != NULO) {
            encontrou = true;
            fseek(f1, offset_encontrado_indice, SEEK_SET);
            ler_registro(f1, &registro_proxEstacao);
            normalizar_campos_texto_registro(&registro_arquivo1);
            normalizar_campos_texto_registro(&registro_proxEstacao);
            printf("%d %s %s %d %s", registro_arquivo1.codEstacao, registro_arquivo1.nomeEstacao, registro_arquivo1.nomeLinha, registro_arquivo1.codProxEstacao, registro_proxEstacao.nomeEstacao);
        }
        fseek(f1, offset_atual, SEEK_SET);
    }

    if(!encontrou) printf("%s", MSG_INEXISTENTE);

    fclose(f1);
    fclose(f2);
    
}