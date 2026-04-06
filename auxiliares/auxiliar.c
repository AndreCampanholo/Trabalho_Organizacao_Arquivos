#include "auxiliar.h"

// A partir de um RRN, calcula o byte offset correspondente
long rrn_para_offset(int rrn)
{
    return TAMANHO_CABECALHO + (long)rrn * TAMANHO_REGISTRO;
}

// Abre o arquivo binário no modo informado pelo usuário, fechando-o caso esteja inconsistente
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

    // Caso estivesse consistente, mas o modo de abertura é para escrita, define como inconsistente
    if (eh_escrita)
    {
        cabecalho->status = '0';
        escrever_cabecalho(*arquivo, cabecalho);
    }
    return 1;
}

// Chama a função acima para escrita
int abrir_binario_escrita(FILE **arquivo, char *nome_arquivo, Cabecalho *cabecalho)
{
    return abrir_binario(arquivo, nome_arquivo, "r+b", cabecalho, 1);
}

// Fecha o arquivo binário aberto para escrita (seta status para '0' -> consistente)
void fechar_binario_escrita(FILE *arquivo, Cabecalho *cabecalho)
{
    cabecalho->status = '1';
    escrever_cabecalho(arquivo, cabecalho);
    fclose(arquivo);
}

// Preenche o espaço restante do registro com lixo ('$')
bool preencher_campos_variaveis_lixo(FILE *arquivo, Registro *registro) {
    // 37 bytes fixos + tamanhos variáveis
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
    // O cabeçalho sempre começa no byte 0 do arquivo.
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
        // O chamador deve reposicionar para o próximo RRN.
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

    // Os 43 bytes variáveis são divididos entre nomeEstacao e nomeLinha.
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

int preparar_csv_e_contar_registros(FILE *arquivo_csv)
{
    char linha[512];
    if (fgets(linha, sizeof(linha), arquivo_csv) == NULL)
        return 0;

    int quantidade = 0;
    while (fgets(linha, sizeof(linha), arquivo_csv) != NULL)
        quantidade++;

    // Volta o ponteiro para começo do csv
    if (fseek(arquivo_csv, 0, SEEK_SET) != 0)
        return -1;
    // Consome a primeira linha do csv (não é um registro) e posiciona ponteiro no primeiro byte de registro válido
    if (fgets(linha, sizeof(linha), arquivo_csv) == NULL)
        return -1;

    return quantidade;
}

// Lê os registros do arquivo .csv e escreve-os no .bin
// Retorno: 1 = registro processado, 0 = fim do csv, -1 = erro
int ler_escrever_registros(FILE *csv, FILE *bin, Cabecalho *cabecalho, Registro *registro_lido)
{
    // Variáveis auxiliares
    char linha[512];
    char *campos[8] = {0};
    int qtd_campos = 0;

    registro_lido->removido = '0';
    registro_lido->proximo = -1;

    // Se não houverem mais linhas de registros no csv, retorna false
    if (fgets(linha, sizeof(linha), csv) == NULL)
        return 0;

    // Remove quebra de linha do final da linha lida e substitui por '\0'
    linha[strcspn(linha, "\r\n")] = '\0';

    // Separa os 8 campos do csv preservando campos vazios entre vírgulas
    char *inicio = linha;
    for (char *p = linha;; p++)
    {
        if (*p == ',' || *p == '\0')
        {
            if (qtd_campos < 8)
                campos[qtd_campos++] = inicio;

            if (*p == '\0')
                break;

            *p = '\0'; // Troca ',' por '\0'
            inicio = p + 1;
        }
    }

    if (qtd_campos != 8)
        return -1;

    // Atribuição dos valores do csv às variáveis
    registro_lido->codEstacao = atoi(campos[0]);

    strncpy(registro_lido->nomeEstacao, campos[1], TAMANHO_CAMPO_VARIAVEL - 1);
    registro_lido->nomeEstacao[TAMANHO_CAMPO_VARIAVEL - 1] = '\0';

    registro_lido->codLinha = (campos[2][0] != '\0') ? atoi(campos[2]) : -1;

    strncpy(registro_lido->nomeLinha, campos[3], TAMANHO_CAMPO_VARIAVEL - 1);
    registro_lido->nomeLinha[TAMANHO_CAMPO_VARIAVEL - 1] = '\0';

    registro_lido->codProxEstacao = (campos[4][0] != '\0') ? atoi(campos[4]) : -1;
    registro_lido->distProxEstacao = (campos[5][0] != '\0') ? atoi(campos[5]) : -1;
    registro_lido->codLinhaIntegra = (campos[6][0] != '\0') ? atoi(campos[6]) : -1;
    registro_lido->codEstIntegra = (campos[7][0] != '\0') ? atoi(campos[7]) : -1;

    // Valores dos indicadores de tamanho dos campos de tamanho variável
    registro_lido->tamNomeEstacao = (int)strlen(registro_lido->nomeEstacao);
    registro_lido->tamNomeLinha = (int)strlen(registro_lido->nomeLinha);

    // Escreve registro no arquivo binário
    if (!escrever_registro(bin, registro_lido))
        return -1;

    // proxRRN aponta para o primeiro RRN livre no final da área de dados.
    cabecalho->proxRRN++;

    return 1;
}

// Imprime NULO caso o valor do campo seja -1 ou o valor caso contrário
void int_ou_nulo(int valor)
{
    if (valor == -1)
        printf("NULO ");
    else
        printf("%d ", valor);
}

// Verifica se uma string 'valor' == "NULO" ou não
int inteiro_ou_nulo(char *valor)
{
    if (strcmp(valor, "NULO") == 0)
        return FLAG_CAMPO_NULO;
    return atoi(valor);
}

// Retorno: 1 = inseriu par novo, 0 = par ja existia, -1 = erro de alocacao
int adicionar_par_unico(int codEstacao, int codProxEstacao, ParEstacao **pares, int *quantidade, int *capacidade)
{
    if (codProxEstacao == FLAG_CAMPO_NULO)
        return 0;

    // Verifica se o par atual já existia no vetor de pares de estações vistas
    for (int i = 0; i < *quantidade; i++)
    {
        if ((*pares)[i].codEstacao == codEstacao && (*pares)[i].codProxEstacao == codProxEstacao)
            return 0;
    }

    if (*quantidade >= *capacidade)
        return -1;

    (*pares)[*quantidade].codEstacao = codEstacao;
    (*pares)[*quantidade].codProxEstacao = codProxEstacao;
    (*quantidade)++;
    return 1;
}

// Inicialização dos campos das struct 'estacoes'
void inicializar_estacoes_vistas(EstacoesVistas *estacoes)
{
    estacoes->nomes = NULL;
    estacoes->quantidade = 0;
    estacoes->capacidade = 0;
}

// Liberação da memória do vetor de nomes de estações
void liberar_estacoes_vistas(EstacoesVistas *estacoes)
{
    for (int i = 0; i < estacoes->quantidade; i++)
        free(estacoes->nomes[i]);
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

    if (estacoes->quantidade >= estacoes->capacidade)
        return false;

    // Adiciona um novo nome ao vetor de nomes de estações vistas, incrementando a quantidade
    estacoes->nomes[estacoes->quantidade] = (char *)malloc(strlen(novo_nome) + 1);
    strcpy(estacoes->nomes[estacoes->quantidade], novo_nome);
    estacoes->quantidade++;
    return true;
}

// Calcula 'nroEstacoes' e 'nroParesEstacoes'
int calcular_nroEstacoes_nroParesEstacoes(FILE *arquivo, Cabecalho *cabecalho)
{
    if (arquivo == NULL || cabecalho == NULL)
        return 0;

    // Guarda pares (origem, destino) já contabilizados.
    typedef struct
    {
        int codEstacao;
        int codProxEstacao;
    } ParEstacaoLocal;

    EstacoesVistas estacoes;
    inicializar_estacoes_vistas(&estacoes);
    estacoes.capacidade = cabecalho->proxRRN > 0 ? cabecalho->proxRRN : 1;
    estacoes.nomes = (char **)malloc((size_t)estacoes.capacidade * sizeof(char *));
    if (estacoes.nomes == NULL)
        return 0;

    int capacidade_pares = cabecalho->proxRRN > 0 ? cabecalho->proxRRN : 1;
    ParEstacaoLocal *pares = (ParEstacaoLocal *)malloc((size_t)capacidade_pares * sizeof(ParEstacaoLocal));
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
            // Registro removido: consumir o restante do RRN para manter alinhamento.
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
        if (registro.codProxEstacao != FLAG_CAMPO_NULO)
        {
            for (int i = 0; i < qtd_pares; i++)
            {
                if (pares[i].codEstacao == registro.codEstacao && pares[i].codProxEstacao == registro.codProxEstacao)
                {
                    par_existe = 1;
                    break;
                }
            }
        }

        if (registro.codProxEstacao != FLAG_CAMPO_NULO && !par_existe)
        {
            // Conta apenas pares distintos no arquivo inteiro.
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