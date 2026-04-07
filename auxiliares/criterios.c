#include "auxiliar.h"

// O campo de texto é considerado nulo quando o seu tamanho é igual a 0 (ou quando a string for vazia).
bool campo_nulo(char *valor, int tamanho)
{
    return tamanho == 0 || valor[0] == '\0';
}

// Identifica se o campo fornecido corresponde a uma string de texto do registro
bool campo_eh_texto(char *nome_campo)
{
    return strcmp(nome_campo, "nomeEstacao") == 0 || strcmp(nome_campo, "nomeLinha") == 0;
}

// Busca o valor de um campo inteiro específico dentro do registro
int obter_campos_inteiros(Registro *registro, char *nome_campo, int *eh_valido)
{
    *eh_valido = 1;
    if (strcmp(nome_campo, "codEstacao") == 0)
        return registro->codEstacao;
    if (strcmp(nome_campo, "codLinha") == 0)
        return registro->codLinha;
    if (strcmp(nome_campo, "codProxEstacao") == 0)
        return registro->codProxEstacao;
    if (strcmp(nome_campo, "distProxEstacao") == 0)
        return registro->distProxEstacao;
    if (strcmp(nome_campo, "codLinhaIntegra") == 0)
        return registro->codLinhaIntegra;
    if (strcmp(nome_campo, "codEstIntegra") == 0)
        return registro->codEstIntegra;
    *eh_valido = 0;
    return 0;
}

// Busca o valor de um campo de texto específico dentro do registro.
// Retorna o ponteiro para a string e o seu devido tamanho. Se o campo não for válido, ele retorna uma string vazia e tamanho 0
char *obter_campos_textos(Registro *registro, char *nome_campo, int *tamanho, int *eh_valido)
{
    *eh_valido = 1;
    if (strcmp(nome_campo, "nomeEstacao") == 0)
    {
        *tamanho = registro->tamNomeEstacao;
        return registro->nomeEstacao;
    }
    if (strcmp(nome_campo, "nomeLinha") == 0)
    {
        *tamanho = registro->tamNomeLinha;
        return registro->nomeLinha;
    }

    *eh_valido = 0;
    *tamanho = 0;
    return "";
}

// Lê os pares (nomeCampo e valorCampo) de critérios de busca que foram informados pelo usuário na entrada
int ler_criterio(Criterio *criterio)
{
    // Leitura do primeiro valor (nomeCampo)
    if (scanf("%31s", criterio->nome) != 1)
        return 0;

    // Leitura do respectivo valor acoplado (valorCampo)
    char str_campo_texto[TAMANHO_CAMPO_VARIAVEL] = {0};
    if (campo_eh_texto(criterio->nome))
        ScanQuoteString(str_campo_texto);
    else
    {
        if (scanf(" %43s", str_campo_texto) != 1)
            return 0;
    }

    // Caso o valor lido do campo seja vazio (ou escrito explicitamente como "NULO" em campos inteiros), o sistema o trata como um elemento nulo
    if (str_campo_texto[0] == '\0' || (!campo_eh_texto(criterio->nome) && strcmp(str_campo_texto, "NULO") == 0))
    {
        criterio->ehNulo = 1;
        criterio->valorInteiro = FLAG_CAMPO_NULO; // O valor predefinido para indicar um registro interno nulo em campos numéricos inteiros
        criterio->valorTexto[0] = '\0';
    }
    else
    {   // Verifica se o campo previamente informado (nomeCampo) é de fato textual (de tamanho variável) ou não
        criterio->ehNulo = 0;
        if (campo_eh_texto(criterio->nome))
        {
            // Se ele for um campo textual autêntico, atribui ao parâmetro valorTexto utilizando a função strncpy segura
            strncpy(criterio->valorTexto, str_campo_texto, TAMANHO_TEXTO - 1);
            criterio->valorTexto[TAMANHO_TEXTO - 1] = '\0';
        }
        else
        {
            // Se ele for um campo do tipo inteiro, o transcreve para valorInteiro utilizando a conversão com a função atoi
            criterio->valorInteiro = atoi(str_campo_texto);
        }
    }
    return 1;
}

// Lê a lista sequencial de critérios definida para as funcionalidades de 3, 4 e 6
int ler_lista_criterios(Criterio *criterios, int *quantidade, int minimo)
{
    // Retorna um indicador de erro caso a quantidade absoluta de critérios informados não esteja dentro dos devidos limites estabelecidos previamente
    if (scanf("%d", quantidade) != 1 || *quantidade < minimo || *quantidade > MAX_CRITERIOS)
        return 0;

    for (int i = 0; i < *quantidade; i++)
    {
        if (!ler_criterio(&criterios[i]))
            return 0;
    }
    return 1;
}

// Verifica se um determinado registro já preenchido atende a todos os critérios listados na busca
int registro_atende_criterios(Registro *registro, Criterio *criterios, int quantidade)
{
    // A busca condicional aplica uma operação lógica AND entre todos os pares nomeCampo e valorCampo informados em sequência
    for (int i = 0; i < quantidade; i++)
    {
        if (campo_eh_texto(criterios[i].nome))
        {
            int tamanho = 0;
            int eh_valido = 0;
            char *texto = obter_campos_textos(registro, criterios[i].nome, &tamanho, &eh_valido);

            if (!eh_valido)
                return 0;

            if (criterios[i].ehNulo)
            {
                if (tamanho != 0)
                    return 0;
            }
            else
            {
                if (tamanho == 0)
                    return 0;
                if (strcmp(texto, criterios[i].valorTexto) != 0)
                    return 0;
            }
        }
        else
        {
            int eh_valido = 0;
            int valor = obter_campos_inteiros(registro, criterios[i].nome, &eh_valido);
            if (!eh_valido)
                return 0;

            if (criterios[i].ehNulo)
            {
                if (valor != FLAG_CAMPO_NULO)
                    return 0;
            }
            else
            {
                if (valor != criterios[i].valorInteiro)
                    return 0;
            }
        }
    }

    return 1;
}

// Função auxiliar da funcionalidade 6 para aplicar os critérios de atualização lidos cima do próprio registro já recuperado
void aplicar_criterio_no_registro(Registro *registro, Criterio *criterio)
{
    // Atualiza especificamente e somente os campos da estrutura listados pelo usuário na entrada. Os demais permanecem inalterados por questão de segurança de registro original
    if (strcmp(criterio->nome, "codEstacao") == 0)
    {
        registro->codEstacao = criterio->ehNulo ? FLAG_CAMPO_NULO : criterio->valorInteiro;
    }
    else if (strcmp(criterio->nome, "codLinha") == 0)
    {
        registro->codLinha = criterio->ehNulo ? FLAG_CAMPO_NULO : criterio->valorInteiro;
    }
    else if (strcmp(criterio->nome, "codProxEstacao") == 0)
    {
        registro->codProxEstacao = criterio->ehNulo ? FLAG_CAMPO_NULO : criterio->valorInteiro;
    }
    else if (strcmp(criterio->nome, "distProxEstacao") == 0)
    {
        registro->distProxEstacao = criterio->ehNulo ? FLAG_CAMPO_NULO : criterio->valorInteiro;
    }
    else if (strcmp(criterio->nome, "codLinhaIntegra") == 0)
    {
        registro->codLinhaIntegra = criterio->ehNulo ? FLAG_CAMPO_NULO : criterio->valorInteiro;
    }
    else if (strcmp(criterio->nome, "codEstIntegra") == 0)
    {
        registro->codEstIntegra = criterio->ehNulo ? FLAG_CAMPO_NULO : criterio->valorInteiro;
    }
    else if (strcmp(criterio->nome, "nomeEstacao") == 0)
    {
        if (criterio->ehNulo)
        {
            registro->tamNomeEstacao = 0;
            registro->nomeEstacao[0] = '\0';
        }
        else
        {
            strncpy(registro->nomeEstacao, criterio->valorTexto, TAMANHO_CAMPO_VARIAVEL - 1);
            registro->tamNomeEstacao = (int)strlen(registro->nomeEstacao);
        }
    }
    else if (strcmp(criterio->nome, "nomeLinha") == 0)
    {
        if (criterio->ehNulo)
        {
            registro->tamNomeLinha = 0;
            registro->nomeLinha[0] = '\0';
        }
        else
        {
            strncpy(registro->nomeLinha, criterio->valorTexto, TAMANHO_CAMPO_VARIAVEL - 1);
            registro->tamNomeLinha = (int)strlen(registro->nomeLinha);
        }
    }
}

void imprimir_registro(Registro *registro)
{
    // Impressão no layout exigido: inteiros com NULO para -1 e textos vazios como NULO.
    printf("%d ", registro->codEstacao);

    if (campo_nulo(registro->nomeEstacao, registro->tamNomeEstacao))
        printf("NULO ");
    else
        printf("%s ", registro->nomeEstacao);

    if (registro->codLinha == FLAG_CAMPO_NULO)
        printf("NULO ");
    else
        printf("%d ", registro->codLinha);

    if (campo_nulo(registro->nomeLinha, registro->tamNomeLinha))
        printf("NULO ");
    else
        printf("%s ", registro->nomeLinha);

    if (registro->codProxEstacao == FLAG_CAMPO_NULO)
        printf("NULO ");
    else
        printf("%d ", registro->codProxEstacao);

    if (registro->distProxEstacao == FLAG_CAMPO_NULO)
        printf("NULO ");
    else
        printf("%d ", registro->distProxEstacao);

    if (registro->codLinhaIntegra == FLAG_CAMPO_NULO)
        printf("NULO ");
    else
        printf("%d ", registro->codLinhaIntegra);

    if (registro->codEstIntegra == FLAG_CAMPO_NULO)
        printf("NULO\n");
    else
        printf("%d\n", registro->codEstIntegra);
}