#include "funcionalidades.h"
#include "../auxiliares/auxiliar.h"

// Funcionalidade [11]: A partir de dois arquivos de dados de entrada (podem ser iguais), verifica a igualdade de dois registro a partir dos campos registro1.CodProxEstacao e registro2.CodEstacao e, caso positivo, imprime ambos
void juncao_loop_aninhado(char *nome_bin1, char *campo1, char *nome_bin2, char *campo2)
{
    Cabecalho *cabecalho1;
    Cabecalho *cabecalho2;

    // Caso seja informado o mesmo arquivo, abrir apenas uma vez, caso contrário abrir ambos
    FILE *f1, *f2;
    if (strcmp(nome_bin1, nome_bin2) == 0)
    {
        abrir_binario(&f1, nome_bin1, "rb", cabecalho1, 0);
        f2 = f1;
    }
    else
    {
        abrir_binario(&f1, nome_bin1, "rb", cabecalho1, 0);
        abrir_binario(&f2, nome_bin2, "rb", cabecalho2, 0);
    }
    
    Registro registro_arquivo1, registro_arquivo2;
    bool encontrou = false;
    
    while(f1 != EOF) {
        ler_registro(f1, &registro_arquivo1);
        fseek(f2, TAMANHO_CABECALHO, SEEK_SET);
        while(f2 != EOF) {
            ler_registro(f2, &registro_arquivo2);
            if(registro_arquivo1.codProxEstacao == registro_arquivo2.codEstacao) {
                encontrou = true;
                normalizar_campos_texto_registro(&registro_arquivo1);
                normalizar_campos_texto_registro(&registro_arquivo2);
                printf("%d %s %s %d %s", registro_arquivo1.codEstacao, registro_arquivo1.nomeEstacao, registro_arquivo1.nomeLinha, registro_arquivo1.codProxEstacao, registro_arquivo2.nomeEstacao);
            }
        }
    }

    if(!encontrou) printf("%s", MSG_INEXISTENTE);

    fclose(f1);
    if (strcmp(nome_bin1, nome_bin2) != 0)
    {
        fclose(f2);
    }
    
}