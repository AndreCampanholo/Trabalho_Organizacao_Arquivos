#include "auxiliar.h"

long rrn_para_offset(int rrn)
{
    return TAMANHO_CABECALHO + (long)rrn * TAMANHO_REGISTRO;
}

int abrir_binario(FILE **arquivo, char *nome_arquivo, char *modo, Cabecalho *cabecalho, int eh_escrita) {
    *arquivo = fopen(nome_arquivo, modo);
    if (*arquivo == NULL) return 0;

    if (!ler_cabecalho(*arquivo, cabecalho)) {
        fclose(*arquivo);
        return 0;
    }

    if (cabecalho->status != '1') {
        fclose(*arquivo);
        return 0;
    }

    if (eh_escrita) {
        cabecalho->status = '0';
        escrever_cabecalho(*arquivo, cabecalho);
    }

    return 1;
}

int ler_cabecalho(FILE *arquivo, Cabecalho *cabecalho)
{
    fseek(arquivo, 0, SEEK_SET);
    if (fread(&cabecalho->status, sizeof(char), 1, arquivo) != 1)
        return 0;
    if (fread(&cabecalho->topo, sizeof(int), 1, arquivo) != 1)
        return 0;
    if (fread(&cabecalho->proxRRN, sizeof(int), 1, arquivo) != 1)
        return 0;
    if (fread(&cabecalho->nroEstacoes, sizeof(int), 1, arquivo) != 1)
        return 0;
    if (fread(&cabecalho->nroParesEstacoes, sizeof(int), 1, arquivo) != 1)
        return 0;
    return 1;
}

// Escrita do cabeçalho campo a campo (padding interno)
void escrever_cabecalho(FILE *arquivo, Cabecalho *cabecalho)
{
    fseek(arquivo, 0, SEEK_SET);
    fwrite(&cabecalho->status, sizeof(char), 1, arquivo);
    fwrite(&cabecalho->topo, sizeof(int), 1, arquivo);
    fwrite(&cabecalho->proxRRN, sizeof(int), 1, arquivo);
    fwrite(&cabecalho->nroEstacoes, sizeof(int), 1, arquivo);
    fwrite(&cabecalho->nroParesEstacoes, sizeof(int), 1, arquivo);
}

int ler_registro(FILE *arquivo, Registro *registro)
{
    if (fread(&registro->removido, sizeof(char), 1, arquivo) != 1)
        return 0;

    // Caso este registro esteja marcado como removido, pula para próximo RRN
    if (registro->removido == '1')
    {
        return -1;
    }

    if (fread(&registro->proximo, sizeof(int), 1, arquivo) != 1)
        return 0;
    if (fread(&registro->codEstacao, sizeof(int), 1, arquivo) != 1)
        return 0;
    if (fread(&registro->codLinha, sizeof(int), 1, arquivo) != 1)
        return 0;
    if (fread(&registro->codProxEstacao, sizeof(int), 1, arquivo) != 1)
        return 0;
    if (fread(&registro->distProxEstacao, sizeof(int), 1, arquivo) != 1)
        return 0;
    if (fread(&registro->codLinhaIntegra, sizeof(int), 1, arquivo) != 1)
        return 0;
    if (fread(&registro->codEstIntegra, sizeof(int), 1, arquivo) != 1)
        return 0;
    if (fread(&registro->tamNomeEstacao, sizeof(int), 1, arquivo) != 1)
        return 0;
    if (fread(&registro->nomeEstacao, sizeof(char), registro->tamNomeEstacao, arquivo) != registro->tamNomeEstacao)
        return 0;
    if (fread(&registro->tamNomeLinha, sizeof(int), 1, arquivo) != 1)
        return 0;
    if (fread(&registro->nomeLinha, sizeof(char), registro->tamNomeLinha, arquivo) != registro->tamNomeLinha)
        return 0;

    return 1;
}

int escrever_registro(FILE *arquivo, Registro *registro)
{
    if (arquivo == NULL || registro == NULL)
        return 0;

    if (registro->tamNomeEstacao < 0 || registro->tamNomeLinha < 0 ||
        registro->tamNomeEstacao + registro->tamNomeLinha > 43)
        return 0;

    if (fwrite(&registro->removido, sizeof(char), 1, arquivo) != 1)
        return 0;
    if (fwrite(&registro->proximo, sizeof(int), 1, arquivo) != 1)
        return 0;
    if (fwrite(&registro->codEstacao, sizeof(int), 1, arquivo) != 1)
        return 0;
    if (fwrite(&registro->codLinha, sizeof(int), 1, arquivo) != 1)
        return 0;
    if (fwrite(&registro->codProxEstacao, sizeof(int), 1, arquivo) != 1)
        return 0;
    if (fwrite(&registro->distProxEstacao, sizeof(int), 1, arquivo) != 1)
        return 0;
    if (fwrite(&registro->codLinhaIntegra, sizeof(int), 1, arquivo) != 1)
        return 0;
    if (fwrite(&registro->codEstIntegra, sizeof(int), 1, arquivo) != 1)
        return 0;

    if (fwrite(&registro->tamNomeEstacao, sizeof(int), 1, arquivo) != 1)
        return 0;
    if (registro->tamNomeEstacao > 0) {
        if (fwrite(registro->nomeEstacao, sizeof(char), (size_t)registro->tamNomeEstacao, arquivo) !=
            (size_t)registro->tamNomeEstacao)
            return 0;
    }

    int bytes_disponiveis = 43 - registro->tamNomeEstacao;

    if (fwrite(&registro->tamNomeLinha, sizeof(int), 1, arquivo) != 1)
        return 0;
    if (registro->tamNomeLinha > 0 && registro->tamNomeLinha <= bytes_disponiveis) {
        if (fwrite(registro->nomeLinha, sizeof(char), (size_t)registro->tamNomeLinha, arquivo) !=
            (size_t)registro->tamNomeLinha)
            return 0;
    }

    int bytes_usados = 37 + registro->tamNomeEstacao + registro->tamNomeLinha;
    char lixo = '$';
    for (int i = bytes_usados; i < TAMANHO_REGISTRO; i++) {
        if (fwrite(&lixo, sizeof(char), 1, arquivo) != 1)
            return 0;
    }

    return 1;
}

// Lê registros do arquivo .csv e escreve-os no .bin
bool ler_escrever_registros(FILE *csv, FILE *bin, Cabecalho *cabecalho, EstacoesVistas *estacoes) {
    // Variáveis auxiliares
    char linha[512];
    char *campo;
    int lixoQtd;

    Registro registro;
    registro.removido = '0';
    registro.proximo = -1;

    // Se não houverem mais linhas de registros no csv, retorna false
    if (fgets(linha, sizeof(linha), csv) == NULL) return false;

    // Atribuição dos valores do csv às variáveis por meio da tokenização de 'linha' nas virgulas (",")
    campo = strtok(linha, ",");
    registro.codEstacao = atoi(campo);    
    campo = strtok(NULL, ",");
    strcpy(registro.nomeEstacao,campo);
    campo = strtok(NULL, ",");
    registro.codLinha = (campo != NULL && campo[0] != '\0') ? atoi(campo) : -1;
    campo = strtok(NULL, ",");
    strcpy(registro.nomeLinha, (campo != NULL) ? campo : "");
    campo = strtok(NULL, ",");
    registro.codProxEstacao = (campo != NULL && campo[0] != '\0') ? atoi(campo) : -1;
    campo = strtok(NULL, ",");
    registro.distProxEstacao = (campo != NULL && campo[0] != '\0') ? atoi(campo) : -1;
    campo = strtok(NULL, ",");
    registro.codLinhaIntegra = (campo != NULL && campo[0] != '\0') ? atoi(campo) : -1;
    campo = strtok(NULL, ",");
    registro.codEstIntegra = (campo != NULL && campo[0] != '\0') ? atoi(campo) : -1;

    // Só incrementa 'nroEstacoes' se 'nomeEstacao' não estiver no vetor de nomes da struct 'estacoes'
    if (nova_estacao(registro.nomeEstacao, estacoes))
        cabecalho->nroEstacoes++;

    // Valores dos indicadores de tamanho dos campos de tamanho variável
    registro.tamNomeEstacao = strlen(registro.nomeEstacao);
    registro.tamNomeLinha = strlen(registro.nomeLinha);
    
    escrever_registro(bin, &registro);
    
    lixoQtd = 43 - registro.tamNomeEstacao - registro.tamNomeLinha;
    for (int i = 0; i < lixoQtd; i++) {
        char lixo = '$';
        fwrite(&lixo, sizeof(char), 1, bin);
    }

    cabecalho->proxRRN++;

    if (registro.codLinhaIntegra != -1 && registro.codEstIntegra != -1) {
        cabecalho->nroParesEstacoes++;
    }

    return true;
}

// Imprime NULO caso o valor do campo seja -1 ou o valor caso contrário
void int_ou_nulo(int valor) {
    if (valor == -1)
        printf("NULO ");
    else
        printf("%d ", valor);
}

// Inicialização dos campos das struct
void inicializar_estacoes_vistas(EstacoesVistas *estacoes) {
    estacoes->nomes = NULL;
    estacoes->quantidade = 0;
    estacoes->capacidade = 0;
}

// Liberação de memória do vetor de nomes de estações
void liberar_estacoes_vistas(EstacoesVistas *estacoes) {
    for (int i = 0; i < estacoes->quantidade; i++) {
        free(estacoes->nomes[i]);
    }
    free(estacoes->nomes);
}

// Verifica se uma estação já foi vista (mesmo nome)
// Entrada: nome da estação a ser verificada e struct de estações vistas
// Saída: true se a estação for nova (não repetida), false caso contrário
bool nova_estacao(char *novo_nome, EstacoesVistas *estacoes) {
    if (novo_nome == NULL) return false;

    for (int i = 0; i < estacoes->quantidade; i++) {
        if (strcmp(estacoes->nomes[i], novo_nome) == 0) {
            return false;
        }
    }

    if (estacoes->quantidade == estacoes->capacidade) {
        estacoes->capacidade = estacoes->capacidade == 0 ? 16 : estacoes->capacidade * 2;
        estacoes->nomes = (char **)realloc(estacoes->nomes, (size_t)estacoes->capacidade * sizeof(char *));
    }

    estacoes->nomes[estacoes->quantidade] = (char *)malloc(strlen(novo_nome) + 1);
    strcpy(estacoes->nomes[estacoes->quantidade], novo_nome);
    estacoes->quantidade++;
    return true;
}

int calcular_nroEstacoes_nroParesEstacoes(FILE *arquivo, Cabecalho *cabecalho) {
    if (arquivo == NULL || cabecalho == NULL) return 0;

    typedef struct {
        int codEstacao;
        int codProxEstacao;
    } ParEstacao;

    EstacoesVistas estacoes;
    inicializar_estacoes_vistas(&estacoes);

    int capacidade_pares = cabecalho->proxRRN > 0 ? cabecalho->proxRRN : 1;
    ParEstacao *pares = (ParEstacao *)malloc((size_t)capacidade_pares * sizeof(ParEstacao));
    if (pares == NULL) {
        return 0;
    }

    int qtd_pares = 0;
    if (fseek(arquivo, rrn_para_offset(0), SEEK_SET) != 0) {
        free(pares);
        liberar_estacoes_vistas(&estacoes);
        return 0;
    }

    for (int rrn = 0; rrn < cabecalho->proxRRN; rrn++) {

        Registro registro;
        int leitura = ler_registro(arquivo, &registro);
        if (leitura == 0) {
            free(pares);
            liberar_estacoes_vistas(&estacoes);
            return 0;
        }
        if (leitura == -1) {
            if (fseek(arquivo, TAMANHO_REGISTRO - 1, SEEK_CUR) != 0) {
                free(pares);
                liberar_estacoes_vistas(&estacoes);
                return 0;
            }
            continue;
        }

        if (registro.tamNomeEstacao < 0 || registro.tamNomeEstacao >= TAMANHO_CAMPO_VARIAVEL ||
            registro.tamNomeLinha < 0 || registro.tamNomeLinha >= TAMANHO_CAMPO_VARIAVEL) {
            free(pares);
            liberar_estacoes_vistas(&estacoes);
            return 0;
        }

        registro.nomeEstacao[registro.tamNomeEstacao] = '\0';
        nova_estacao(registro.nomeEstacao, &estacoes);

        int bytes_usados = 37 + registro.tamNomeEstacao + registro.tamNomeLinha;
        int bytes_restantes = TAMANHO_REGISTRO - bytes_usados;
        if (bytes_restantes > 0) {
            if (fseek(arquivo, bytes_restantes, SEEK_CUR) != 0) {
                free(pares);
                liberar_estacoes_vistas(&estacoes);
                return 0;
            }
        }

        int par_existe = 0;
        for (int i = 0; i < qtd_pares; i++) {
            if (pares[i].codEstacao == registro.codEstacao && pares[i].codProxEstacao == registro.codProxEstacao) {
                par_existe = 1;
                break;
            }
        }

        if (!par_existe) {
            pares[qtd_pares].codEstacao = registro.codEstacao;
            pares[qtd_pares].codProxEstacao = registro.codProxEstacao;
            qtd_pares++;
        }
    }

    cabecalho->nroEstacoes = estacoes.quantidade;
    cabecalho->nroParesEstacoes = qtd_pares;

    free(pares);
    liberar_estacoes_vistas(&estacoes);
    return 1;
}