#include "auxiliar.h"

// A partir de um RRN, calcula byte offset
long rrn_para_offset(int rrn)
{
    return TAMANHO_CABECALHO + (long)rrn * TAMANHO_REGISTRO;
}

// Abre arquivo binário no modo informado pelo usuário, fechando-o caso esteja inconsistente
int abrir_binario(FILE **arquivo, char *nome_arquivo, char *modo, Cabecalho *cabecalho, int eh_escrita)
{
    *arquivo = fopen(nome_arquivo, modo);
    if (*arquivo == NULL)
        return 0;

    // Leitura do cabeçalho
    if (!ler_cabecalho(*arquivo, cabecalho))
    {
        fclose(*arquivo);
        return 0;
    }

    // Verificação de consistência
    if (cabecalho->status != '1')
    {
        fclose(*arquivo);
        return 0;
    }

    // Caso estivesse consistente e modo de abertura é para escrita, define como inconsistente
    if (eh_escrita)
    {
        cabecalho->status = '0';
        escrever_cabecalho(*arquivo, cabecalho);
    }

    return 1;
}

// Chama função acima para escrita
int abrir_binario_escrita(FILE **arquivo, char *nome_arquivo, Cabecalho *cabecalho)
{
    return abrir_binario(arquivo, nome_arquivo, "r+b", cabecalho, 1);
}

// Fecha arquivo binário aberto para escrita (seta status para '0')
void fechar_binario_escrita(FILE *arquivo, Cabecalho *cabecalho)
{
    cabecalho->status = '1';
    escrever_cabecalho(arquivo, cabecalho);
    fclose(arquivo);
}

// Preenche espaço disponível de registro com lixo ('$')
bool preencher_campos_variaveis_lixo(FILE *arquivo, Registro *registro) {
    int bytes_usados = 37 + registro->tamNomeEstacao + registro->tamNomeLinha;
    int bytes_restantes = TAMANHO_REGISTRO - bytes_usados;
    
    char lixo = '$';
    for(int i = 0; i < bytes_restantes; i++) {
        if(fwrite(&lixo, sizeof(char), 1, arquivo) != 1) {
            return false;
        }
    }
    return true;
}

// Lê lista de critérios para funcionalidade 3, 4 e 6
int ler_lista_criterios(Criterio *criterios, int *quantidade, int minimo)
{
    if (scanf("%d", quantidade) != 1 || *quantidade < minimo || *quantidade > MAX_CRITERIOS)
    {
        return 0;
    }

    for (int i = 0; i < *quantidade; i++)
    {
        if (!ler_criterio(&criterios[i])) // Função no critérios.c
        {
            return 0;
        }
    }

    return 1;
}

// Adiciona '\0' ao final de campos de tamanho variável para comparação de strings
void normalizar_campos_texto_registro(Registro *registro)
{
    if (registro->tamNomeEstacao >= 0 && registro->tamNomeEstacao < TAMANHO_CAMPO_VARIAVEL)
    {
        registro->nomeEstacao[registro->tamNomeEstacao] = '\0';
    }

    if (registro->tamNomeLinha >= 0 && registro->tamNomeLinha < TAMANHO_CAMPO_VARIAVEL)
    {
        registro->nomeLinha[registro->tamNomeLinha] = '\0';
    }
}

// Leitura do registro de cabeçalho do arquivo
int ler_cabecalho(FILE *arquivo, Cabecalho *cabecalho)
{
    fseek(arquivo, 0, SEEK_SET); // Posiciona ponteiro no byte offset zero
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

// Escrita do cabeçalho campo a campo (não escreve cabeçalho inteiro de uma vez devido ao padding interno)
void escrever_cabecalho(FILE *arquivo, Cabecalho *cabecalho)
{
    fseek(arquivo, 0, SEEK_SET);
    fwrite(&cabecalho->status, sizeof(char), 1, arquivo);
    fwrite(&cabecalho->topo, sizeof(int), 1, arquivo);
    fwrite(&cabecalho->proxRRN, sizeof(int), 1, arquivo);
    fwrite(&cabecalho->nroEstacoes, sizeof(int), 1, arquivo);
    fwrite(&cabecalho->nroParesEstacoes, sizeof(int), 1, arquivo);
}

// Lê um registro inteiro campo a campo
int ler_registro(FILE *arquivo, Registro *registro)
{
    if (fread(&registro->removido, sizeof(char), 1, arquivo) != 1)
        return 0;

    // Caso este registro esteja marcado como removido, retorna -1 (pula para próximo RRN)
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
    if (fread(&registro->nomeEstacao, sizeof(char), (size_t)registro->tamNomeEstacao, arquivo) != (size_t)registro->tamNomeEstacao)
        return 0;
    if (fread(&registro->tamNomeLinha, sizeof(int), 1, arquivo) != 1)
        return 0;
    if (fread(&registro->nomeLinha, sizeof(char), (size_t)registro->tamNomeLinha, arquivo) != (size_t)registro->tamNomeLinha)
        return 0;

    return 1;
}

// Escreve um registro campo a campo
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
        
    if (registro->tamNomeEstacao > 0)
    {
        if (fwrite(registro->nomeEstacao, sizeof(char), (size_t)registro->tamNomeEstacao, arquivo) !=
            (size_t)registro->tamNomeEstacao)
            return 0;
    }

    int bytes_disponiveis = 43 - registro->tamNomeEstacao;

    if (fwrite(&registro->tamNomeLinha, sizeof(int), 1, arquivo) != 1)
        return 0;
    if (registro->tamNomeLinha > 0 && registro->tamNomeLinha <= bytes_disponiveis)
    {
        if (fwrite(registro->nomeLinha, sizeof(char), (size_t)registro->tamNomeLinha, arquivo) !=
            (size_t)registro->tamNomeLinha)
            return 0;
    }

    if(!preencher_campos_variaveis_lixo(arquivo, registro))
        return 0;

    return 1;
}

// Lê registros do arquivo .csv e escreve-os no .bin
bool ler_escrever_registros(FILE *csv, FILE *bin, Cabecalho *cabecalho, Registro *registro_lido)
{
    // Variáveis auxiliares
    char linha[512];
    char *campo;

    Registro registro;
    registro.removido = '0';
    registro.proximo = -1;

    // Se não houverem mais linhas de registros no csv, retorna false
    if (fgets(linha, sizeof(linha), csv) == NULL)
        return false;

    // Atribuição dos valores do csv às variáveis por meio da tokenização de 'linha' nas virgulas (",")
    campo = strtok(linha, ",");
    registro.codEstacao = atoi(campo);
    campo = strtok(NULL, ",");
    strcpy(registro.nomeEstacao, campo);
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

    // Valores dos indicadores de tamanho dos campos de tamanho variável
    registro.tamNomeEstacao = strlen(registro.nomeEstacao);
    registro.tamNomeLinha = strlen(registro.nomeLinha);

    // Escreve registro no arquivo binário
    if (!escrever_registro(bin, &registro))
        return false;

    cabecalho->proxRRN++;

    if (registro_lido != NULL)
    {
        *registro_lido = registro;
    }

    return true;
}

// Imprime NULO caso o valor do campo seja -1 ou o valor caso contrário
void int_ou_nulo(int valor)
{
    if (valor == -1)
        printf("NULO ");
    else
        printf("%d ", valor);
}

// Verifica se string 'valor' == "NULO" ou não
int inteiro_ou_nulo(char *valor)
{
    if (strcmp(valor, "NULO") == 0)
        return FLAG_CAMPO_NULO;
    return atoi(valor);
}

// Inicialização dos campos das struct 'estacoes'
void inicializar_estacoes_vistas(EstacoesVistas *estacoes)
{
    estacoes->nomes = NULL;
    estacoes->quantidade = 0;
    estacoes->capacidade = 0;
}

// Liberação de memória do vetor de nomes de estações
void liberar_estacoes_vistas(EstacoesVistas *estacoes)
{
    for (int i = 0; i < estacoes->quantidade; i++)
    {
        free(estacoes->nomes[i]);
    }
    free(estacoes->nomes);
}

// Verifica se uma estação já foi vista (mesmo nome)
// Entrada: nome da estação a ser verificada e struct de estações vistas
// Saída: true se a estação for nova (não repetida), false caso contrário
bool nova_estacao(char *novo_nome, EstacoesVistas *estacoes)
{
    if (novo_nome == NULL)
        return false;

    for (int i = 0; i < estacoes->quantidade; i++)
    {
        if (strcmp(estacoes->nomes[i], novo_nome) == 0)
        {
            return false;
        }
    }

    // Caso quantidade de estações estoure capacidade atual, aloca mas memória
    if (estacoes->quantidade == estacoes->capacidade)
    {
        estacoes->capacidade = estacoes->capacidade == 0 ? 16 : estacoes->capacidade * 2;
        estacoes->nomes = (char **)realloc(estacoes->nomes, (size_t)estacoes->capacidade * sizeof(char *));
    }

    // Adiciona um novo nome ao vetor de nomes de estações vistas, incrementando a quantidade
    estacoes->nomes[estacoes->quantidade] = (char *)malloc(strlen(novo_nome) + 1);
    strcpy(estacoes->nomes[estacoes->quantidade], novo_nome);
    estacoes->quantidade++;
    return true;
}

// Calcula 'nroEstacoes' and 'nroParesEstacoes'
int calcular_nroEstacoes_nroParesEstacoes(FILE *arquivo, Cabecalho *cabecalho)
{
    if (arquivo == NULL || cabecalho == NULL)
        return 0;

    // Struct para verificação de pares de estações
    typedef struct
    {
        int codEstacao;
        int codProxEstacao;
    } ParEstacao;

    EstacoesVistas estacoes;
    inicializar_estacoes_vistas(&estacoes);

    
    int capacidade_pares = cabecalho->proxRRN > 0 ? cabecalho->proxRRN : 1;
    ParEstacao *pares = (ParEstacao *)malloc((size_t)capacidade_pares * sizeof(ParEstacao));
    if (pares == NULL)
    {
        return 0;
    }
 
    int qtd_pares = 0;
    if (fseek(arquivo, rrn_para_offset(0), SEEK_SET) != 0)
    {
        free(pares);
        liberar_estacoes_vistas(&estacoes);
        return 0;
    }

    for (int rrn = 0; rrn < cabecalho->proxRRN; rrn++)
    {

        Registro registro;
        int leitura = ler_registro(arquivo, &registro);
        if (leitura == 0)
        {
            free(pares);
            liberar_estacoes_vistas(&estacoes);
            return 0;
        }
        if (leitura == -1)
        {
            if (fseek(arquivo, TAMANHO_REGISTRO - 1, SEEK_CUR) != 0)
            {
                free(pares);
                liberar_estacoes_vistas(&estacoes);
                return 0;
            }
            continue;
        }

        if (registro.tamNomeEstacao < 0 || registro.tamNomeEstacao >= TAMANHO_CAMPO_VARIAVEL ||
            registro.tamNomeLinha < 0 || registro.tamNomeLinha >= TAMANHO_CAMPO_VARIAVEL)
        {
            free(pares);
            liberar_estacoes_vistas(&estacoes);
            return 0;
        }

        registro.nomeEstacao[registro.tamNomeEstacao] = '\0';
        nova_estacao(registro.nomeEstacao, &estacoes);

        int bytes_usados = 37 + registro.tamNomeEstacao + registro.tamNomeLinha;
        int bytes_restantes = TAMANHO_REGISTRO - bytes_usados;
        if (bytes_restantes > 0)
        {
            if (fseek(arquivo, bytes_restantes, SEEK_CUR) != 0)
            {
                free(pares);
                liberar_estacoes_vistas(&estacoes);
                return 0;
            }
        }

        int par_existe = 0;
        for (int i = 0; i < qtd_pares; i++)
        {
            if (pares[i].codEstacao == registro.codEstacao && pares[i].codProxEstacao == registro.codProxEstacao)
            {
                par_existe = 1;
                break;
            }
        }

        if (!par_existe)
        {
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