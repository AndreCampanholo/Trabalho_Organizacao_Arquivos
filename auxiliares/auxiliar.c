#include "auxiliar.h"
#include "bt.h"

// Converte um RRN para o offset em bytes no arquivo binário, levando em conta o tamanho fixo do cabeçalho e de cada registro.
long rrn_para_offset(int rrn)
{
    return TAMANHO_CABECALHO + (long)rrn * TAMANHO_REGISTRO;
}

// Função principal de abertura do arquivo binário: abre o arquivo no modo indicado, lê o cabeçalho e verifica se o arquivo está consistente (status == '1'). Se for abertura para escrita, o status é imediatamente marcado como inconsistente ('0') e regravado, protegendo contra interrupções inesperadas.
int abrir_binario(FILE **arquivo, char *nome_arquivo, char *modo, Cabecalho *cabecalho, int eh_escrita)
{
    *arquivo = fopen(nome_arquivo, modo);
    if (*arquivo == NULL)
        return 0;

    // Lê o cabeçalho e verifica se o arquivo está em estado consistente.
    if (!ler_cabecalho(*arquivo, cabecalho))
    {
        fclose(*arquivo);
        return 0;
    }

    // Um status diferente de '1' indica que o arquivo foi encerrado de forma irregular.
    if (cabecalho->status != '1')
    {
        fclose(*arquivo);
        return 0;
    }

    // Se a abertura for para escrita, define o status como inconsistente antes de qualquer alteração, garantindo que uma eventual falha não deixe o arquivo em estado corrompido e silencioso.
    if (eh_escrita)
    {
        cabecalho->status = '0';
        escrever_cabecalho(*arquivo, cabecalho);
    }
    return 1;
}

// Abre o arquivo binário de dados em modo leitura/escrita ("r+b"), marcando-o como inconsistente. É um atalho conveniente para a rotina genérica 'abrir_binario'.
int abrir_binario_escrita(FILE **arquivo, char *nome_arquivo, Cabecalho *cabecalho)
{
    return abrir_binario(arquivo, nome_arquivo, "r+b", cabecalho, 1);
}

// Restaura o status do arquivo para consistente ('1'), escreve o cabeçalho atualizado e fecha o ponteiro. Deve ser chamada ao término de qualquer operação de escrita bem-sucedida.
void fechar_binario_escrita(FILE *arquivo, Cabecalho *cabecalho)
{
    cabecalho->status = '1';
    escrever_cabecalho(arquivo, cabecalho);
    fclose(arquivo);
}

// Preenche com o caractere de lixo '$' os bytes que sobram após a parte útil do registro, mantendo o tamanho fixo de TAMANHO_REGISTRO bytes. Os 37 bytes fixos mais os tamanhos dos dois campos variáveis determinam quantos bytes já foram usados.
bool preencher_campos_variaveis_lixo(FILE *arquivo, Registro *registro)
{
    int bytes_usados = 37 + registro->tamNomeEstacao + registro->tamNomeLinha;
    int bytes_restantes = TAMANHO_REGISTRO - bytes_usados;

    char lixo = '$';
    for (int i = 0; i < bytes_restantes; i++)
    {
        if (fwrite(&lixo, sizeof(char), 1, arquivo) != 1)
        {
            return false;
        }
    }
    return true;
}

// Insere o terminador '\0' ao final dos campos de texto de tamanho variável do registro, viabilizando o uso direto de funções de string (strcmp, printf etc.) sobre esses campos. Os índices são validados antes da escrita para evitar acesso fora dos limites do buffer.
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

// Lê os campos do cabeçalho sequencialmente a partir do byte 0 do arquivo e os armazena na estrutura apontada por 'cabecalho'. Retorna 0 se qualquer leitura falhar.
int ler_cabecalho(FILE *arquivo, Cabecalho *cabecalho)
{
    // O cabeçalho sempre começa no byte 0 do arquivo.
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

// Escreve todos os campos do cabeçalho no início do arquivo, sobrescrevendo os valores anteriores. Deve ser chamada sempre que qualquer campo do cabeçalho for alterado em memória.
void escrever_cabecalho(FILE *arquivo, Cabecalho *cabecalho)
{
    fseek(arquivo, 0, SEEK_SET);
    fwrite(&cabecalho->status, sizeof(char), 1, arquivo);
    fwrite(&cabecalho->topo, sizeof(int), 1, arquivo);
    fwrite(&cabecalho->proxRRN, sizeof(int), 1, arquivo);
    fwrite(&cabecalho->nroEstacoes, sizeof(int), 1, arquivo);
    fwrite(&cabecalho->nroParesEstacoes, sizeof(int), 1, arquivo);
}

// Escreve o cabeçalho do arquivo de índice (árvore-B) no início do arquivo, atualizando todos os metadados da estrutura de índice em disco.
void escrever_cabecalho_bt(FILE *arquivo, CabecalhoBT *cabecalho_bt)
{
    fseek(arquivo, 0, SEEK_SET);
    fwrite(&cabecalho_bt->status, sizeof(char), 1, arquivo);
    fwrite(&cabecalho_bt->noRaiz, sizeof(int), 1, arquivo);
    fwrite(&cabecalho_bt->topo, sizeof(int), 1, arquivo);
    fwrite(&cabecalho_bt->proxRRN, sizeof(int), 1, arquivo);
    fwrite(&cabecalho_bt->nroNos, sizeof(int), 1, arquivo);
}

// Lê um registro completo a partir da posição atual do ponteiro do arquivo. Retorna 0 em caso de falha de leitura, -1 se o registro estiver marcado como removido (avançando o ponteiro os bytes restantes para manter o alinhamento) e 1 em caso de sucesso. Os bytes de lixo que completam o tamanho fixo são consumidos antes de retornar.
int ler_registro(FILE *arquivo, Registro *registro)
{
    if (fread(&registro->removido, sizeof(char), 1, arquivo) != 1)
        return 0;

    // Se o registro está marcado como removido, avança os 79 bytes restantes para manter o ponteiro alinhado no início do próximo registro e retorna -1.
    if (registro->removido == '1' || registro->removido == '*')
    {
        if (fseek(arquivo, TAMANHO_REGISTRO - 1, SEEK_CUR) != 0)
            return 0;
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

    // Descarta os bytes de lixo que preenchem o espaço restante até completar TAMANHO_REGISTRO bytes.
    int bytes_usados = 37 + registro->tamNomeEstacao + registro->tamNomeLinha;
    if (bytes_usados < TAMANHO_REGISTRO)
    {
        if (fseek(arquivo, TAMANHO_REGISTRO - bytes_usados, SEEK_CUR) != 0)
            return 0;
    }

    return 1;
}

// Escreve um registro completo na posição atual do ponteiro do arquivo. Valida os ponteiros e os tamanhos dos campos variáveis antes de escrever, retornando 0 em qualquer falha. Os bytes restantes são preenchidos com lixo para manter o tamanho fixo.
int escrever_registro(FILE *arquivo, Registro *registro)
{
    if (arquivo == NULL || registro == NULL)
        return 0;

    // Rejeita tamanhos inválidos que causariam escrita corrompida ou estouro de buffer.
    if (registro->tamNomeEstacao < 0 || registro->tamNomeLinha < 0 ||
        registro->tamNomeEstacao + registro->tamNomeLinha >= TAMANHO_CAMPO_VARIAVEL)
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

    // Os 43 bytes variáveis são divididos entre nomeEstacao e nomeLinha, precedidos cada um pelo seu respectivo indicador de tamanho.
    if (fwrite(&registro->tamNomeEstacao, sizeof(int), 1, arquivo) != 1)
        return 0;

    if (registro->tamNomeEstacao > 0)
    {
        if (fwrite(registro->nomeEstacao, sizeof(char), (size_t)registro->tamNomeEstacao, arquivo) !=
            (size_t)registro->tamNomeEstacao)
            return 0;
    }

    // Calcula quantos bytes ainda restam para o nomeLinha dentro da área variável.
    int bytes_disponiveis = 43 - registro->tamNomeEstacao;

    if (fwrite(&registro->tamNomeLinha, sizeof(int), 1, arquivo) != 1)
        return 0;
    if (registro->tamNomeLinha > 0 && registro->tamNomeLinha <= bytes_disponiveis)
    {
        if (fwrite(registro->nomeLinha, sizeof(char), (size_t)registro->tamNomeLinha, arquivo) !=
            (size_t)registro->tamNomeLinha)
            return 0;
    }

    // Completa o espaço restante do registro com o caractere de lixo '$'.
    if (!preencher_campos_variaveis_lixo(arquivo, registro))
        return 0;

    return 1;
}

// Insere uma entrada no índice (árvore-B), associando a chave ao RRN do registro no arquivo de dados. Delega integralmente a operação para a rotina 'inserir_indice' do módulo de árvore-B.
bool escrever_registro_bt(FILE *arquivo_indice, CabecalhoBT *cabecalho_bt, int rrn_no_arquivo_dados, int chave)
{
    return inserir_indice(arquivo_indice, cabecalho_bt, chave, rrn_no_arquivo_dados);
}

// Lê o cabeçalho do CSV para descartar a linha de títulos, conta as linhas de dados restantes e reposiciona o ponteiro logo após o cabeçalho para que a leitura dos registros possa começar. Retorna a quantidade de registros encontrados, 0 se o CSV estiver vazio ou -1 em caso de erro.
int preparar_csv_e_contar_registros(FILE *arquivo_csv)
{
    char linha[512];
    if (fgets(linha, sizeof(linha), arquivo_csv) == NULL)
        return 0;

    int quantidade = 0;
    while (fgets(linha, sizeof(linha), arquivo_csv) != NULL)
        quantidade++;

    // Retorna ao início do arquivo para reler o cabeçalho e posicionar no primeiro registro válido.
    if (fseek(arquivo_csv, 0, SEEK_SET) != 0)
        return -1;
    // Consome novamente a linha de cabeçalho, posicionando o ponteiro no primeiro byte de dado real.
    if (fgets(linha, sizeof(linha), arquivo_csv) == NULL)
        return -1;

    return quantidade;
}

// Lê uma linha do CSV, extrai os 8 campos delimitados por vírgula, preenche a estrutura 'registro_lido' com os valores convertidos e escreve o registro no arquivo binário. Retorna 1 em caso de sucesso, 0 se não houver mais linhas e -1 em caso de erro.
int ler_escrever_registros(FILE *csv, FILE *bin, Cabecalho *cabecalho, Registro *registro_lido)
{
    // Variáveis auxiliares para a leitura e separação dos campos da linha CSV.
    char linha[512];
    char *campos[8] = {0};
    int qtd_campos = 0;

    registro_lido->removido = '0';
    registro_lido->proximo = -1;

    // Se não houver mais linhas de registros no CSV, sinaliza o fim da leitura retornando 0.
    if (fgets(linha, sizeof(linha), csv) == NULL)
        return 0;

    // Remove a quebra de linha do final e substitui por '\0' para facilitar o parsing.
    linha[strcspn(linha, "\r\n")] = '\0';

    // Percorre a linha caractere a caractere, separando os 8 campos e preservando campos vazios entre vírgulas.
    char *inicio = linha;
    for (char *p = linha;; p++)
    {
        if (*p == ',' || *p == '\0')
        {
            if (qtd_campos < 8)
                campos[qtd_campos++] = inicio;

            if (*p == '\0')
                break;

            *p = '\0'; // Substitui a vírgula por '\0', isolando cada campo como uma string independente.
            inicio = p + 1;
        }
    }

    if (qtd_campos != 8)
        return -1;

    // Atribui os valores dos campos do CSV às variáveis correspondentes da estrutura do registro.
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

    // Calcula os tamanhos dos campos variáveis com base no conteúdo efetivamente lido.
    registro_lido->tamNomeEstacao = (int)strlen(registro_lido->nomeEstacao);
    registro_lido->tamNomeLinha = (int)strlen(registro_lido->nomeLinha);

    // Escreve o registro populado no arquivo binário.
    if (!escrever_registro(bin, registro_lido))
        return -1;

    // Avança o proxRRN para apontar para o próximo slot livre ao final da área de dados.
    cabecalho->proxRRN++;

    return 1;
}

// Imprime o valor inteiro ou a string "NULO" (com espaço) caso o valor seja -1, padronizando a exibição de campos opcionais nas saídas do programa.
void int_ou_nulo(int valor)
{
    if (valor == -1)
        printf("NULO ");
    else
        printf("%d ", valor);
}

// Converte a string de um campo CSV para inteiro, retornando FLAG_CAMPO_NULO se o valor for a string "NULO", mantendo a semântica de campo ausente na estrutura do registro.
int inteiro_ou_nulo(char *valor)
{
    if (strcmp(valor, "NULO") == 0)
        return FLAG_CAMPO_NULO;
    return atoi(valor);
}

// Tenta adicionar o par (codEstacao, codProxEstacao) ao vetor de pares já contabilizados. Ignora pares cujo destino seja FLAG_CAMPO_NULO, pares duplicados ou quando o vetor está cheio. Retorna 1 se o par foi adicionado, 0 se foi ignorado e -1 se o vetor não tem mais capacidade.
int adicionar_par_unico(int codEstacao, int codProxEstacao, ParEstacao **pares, int *quantidade, int *capacidade)
{
    if (codProxEstacao == FLAG_CAMPO_NULO)
        return 0;

    // Verifica se o par (origem, destino) já existe no vetor antes de adicioná-lo.
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

// Inicializa a estrutura de controle de estações vistas com valores nulos, deixando-a pronta para ser populada pelas funções de contagem de estações únicas.
void inicializar_estacoes_vistas(EstacoesVistas *estacoes)
{
    estacoes->nomes = NULL;
    estacoes->quantidade = 0;
    estacoes->capacidade = 0;
}

// Libera a memória de cada nome de estação armazenado e depois libera o vetor de ponteiros, evitando vazamentos de memória ao encerrar o uso da estrutura de estações vistas.
void liberar_estacoes_vistas(EstacoesVistas *estacoes)
{
    for (int i = 0; i < estacoes->quantidade; i++)
        free(estacoes->nomes[i]);
    free(estacoes->nomes);
}

// Verifica se o nome da estação ainda não foi registrado e, caso seja novo, aloca uma cópia e a adiciona ao vetor de nomes. Retorna false se o nome for NULL, duplicado ou se o vetor já estiver na capacidade máxima, e true se a inserção foi realizada com sucesso.
bool nova_estacao(char *novo_nome, EstacoesVistas *estacoes)
{
    if (novo_nome == NULL)
        return false;

    // Percorre os nomes já registrados para verificar se esta estação já foi contabilizada.
    for (int i = 0; i < estacoes->quantidade; i++)
    {
        if (strcmp(estacoes->nomes[i], novo_nome) == 0)
        {
            return false;
        }
    }

    if (estacoes->quantidade >= estacoes->capacidade)
        return false;

    // Adiciona uma cópia do nome ao vetor de estações únicas vistas e incrementa o contador.
    estacoes->nomes[estacoes->quantidade] = (char *)malloc(strlen(novo_nome) + 1);
    strcpy(estacoes->nomes[estacoes->quantidade], novo_nome);
    estacoes->quantidade++;
    return true;
}

// Percorre todos os registros ativos do arquivo, contabilizando estações únicas (por nome) e pares distintos de (codEstacao, codProxEstacao), e atualiza os campos 'nroEstacoes' e 'nroParesEstacoes' do cabeçalho. Retorna 0 em qualquer falha de leitura ou alocação.
int calcular_nroEstacoes_nroParesEstacoes(FILE *arquivo, Cabecalho *cabecalho)
{
    if (arquivo == NULL || cabecalho == NULL)
        return 0;

    // Estrutura local para armazenar os pares (origem, destino) já contabilizados.
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
    // Posiciona o ponteiro no início da área de dados para a varredura sequencial.
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
            // O 'ler_registro' já avançou os 80 bytes do slot removido, basta continuar para o próximo.
            continue;
        }

        // Valida os tamanhos dos campos variáveis antes de acessar os buffers de texto.
        if (registro.tamNomeEstacao < 0 || registro.tamNomeEstacao >= TAMANHO_CAMPO_VARIAVEL ||
            registro.tamNomeLinha < 0 || registro.tamNomeLinha >= TAMANHO_CAMPO_VARIAVEL)
        {
            free(pares);
            liberar_estacoes_vistas(&estacoes);
            return 0;
        }

        // Insere o terminador para usar a string como nome da estação na verificação de duplicatas.
        if (registro.tamNomeEstacao > 0)
        {
            registro.nomeEstacao[registro.tamNomeEstacao] = '\0';
            nova_estacao(registro.nomeEstacao, &estacoes);
        }

        // Verifica se o par (codEstacao, codProxEstacao) deste registro já foi contabilizado.
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

        // Adiciona o par ao vetor somente se ele ainda não tiver sido contabilizado.
        if (registro.codProxEstacao != FLAG_CAMPO_NULO && !par_existe)
        {
            pares[qtd_pares].codEstacao = registro.codEstacao;
            pares[qtd_pares].codProxEstacao = registro.codProxEstacao;
            qtd_pares++;
        }
    }

    // Atualiza os campos do cabeçalho com os totais recalculados.
    cabecalho->nroEstacoes = estacoes.quantidade;
    cabecalho->nroParesEstacoes = qtd_pares;

    free(pares);
    liberar_estacoes_vistas(&estacoes);
    return 1;
}

// Lê os campos do cabeçalho do arquivo de índice (árvore-B) a partir do byte 0 e os armazena na estrutura apontada por 'cabecalho_bt'. Retorna 0 se qualquer leitura falhar.
int ler_cabecalho_bt(FILE *arquivo, CabecalhoBT *cabecalho_bt)
{
    fseek(arquivo, 0, SEEK_SET);
    if (fread(&cabecalho_bt->status, sizeof(char), 1, arquivo) != 1)
        return 0;
    if (fread(&cabecalho_bt->noRaiz, sizeof(int), 1, arquivo) != 1)
        return 0;
    if (fread(&cabecalho_bt->topo, sizeof(int), 1, arquivo) != 1)
        return 0;
    if (fread(&cabecalho_bt->proxRRN, sizeof(int), 1, arquivo) != 1)
        return 0;
    if (fread(&cabecalho_bt->nroNos, sizeof(int), 1, arquivo) != 1)
        return 0;
    return 1;
}

// Rotina genérica de abertura do arquivo de índice (árvore-B): abre no modo indicado, lê o cabeçalho, verifica a consistência e, se for abertura para escrita, marca como inconsistente antes de qualquer alteração para proteger contra falhas durante a operação.
int abrir_binario_bt(FILE **arquivo, char *nome_arquivo, char *modo, CabecalhoBT *cabecalho_bt, int eh_escrita)
{
    *arquivo = fopen(nome_arquivo, modo);
    if (*arquivo == NULL)
        return 0;

    if (!ler_cabecalho_bt(*arquivo, cabecalho_bt))
    {
        fclose(*arquivo);
        return 0;
    }

    // Um status diferente de '1' indica inconsistência no arquivo de índice.
    if (cabecalho_bt->status != '1')
    {
        fclose(*arquivo);
        return 0;
    }

    // Marca o arquivo como inconsistente imediatamente ao abri-lo para escrita.
    if (eh_escrita)
    {
        cabecalho_bt->status = '0';
        escrever_cabecalho_bt(*arquivo, cabecalho_bt);
    }
    return 1;
}

// Abre o arquivo de índice (árvore-B) em modo leitura/escrita ("r+b"), marcando-o como inconsistente. É um atalho conveniente para a rotina genérica 'abrir_binario_bt'.
int abrir_binario_escrita_bt(FILE **arquivo, char *nome_arquivo, CabecalhoBT *cabecalho_bt)
{
    return abrir_binario_bt(arquivo, nome_arquivo, "r+b", cabecalho_bt, 1);
}

// Restaura o status do arquivo de índice para consistente ('1'), escreve o cabeçalho e fecha o ponteiro. Deve ser chamada ao término de qualquer operação de escrita bem-sucedida no índice.
void fechar_binario_escrita_bt(FILE *arquivo, CabecalhoBT *cabecalho_bt)
{
    cabecalho_bt->status = '1';
    escrever_cabecalho_bt(arquivo, cabecalho_bt);
    fclose(arquivo);
}

// Realiza a remoção lógica do registro localizado no 'offset' informado: escreve o marcador '1' no campo 'removido' e escreve o antigo topo da pilha no campo 'proximo', encadeando o registro removido à lista de espaços livres. O RRN deste registro passa a ser o novo topo da pilha.
int remover_registro_logico(FILE *arquivo, Cabecalho *cabecalho, long offset)
{
    char removido = '1';
    int antigo_topo = cabecalho->topo;
    int rrn = (int)((offset - TAMANHO_CABECALHO) / TAMANHO_REGISTRO);

    if (fseek(arquivo, offset, SEEK_SET) != 0)
        return 0;
    if (fwrite(&removido, sizeof(char), 1, arquivo) != 1)
        return 0;
    // Encadeia o registro removido na pilha de espaços livres, gravando o antigo topo em 'proximo'.
    if (fwrite(&antigo_topo, sizeof(int), 1, arquivo) != 1)
        return 0;

    // O registro recém-removido se torna o novo topo da pilha de espaços livres.
    cabecalho->topo = rrn;
    return 1;
}