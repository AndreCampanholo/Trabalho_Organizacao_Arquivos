#include "auxiliar.h"

/* Campo de texto é considerado nulo quando o tamanho == 0 (ou string vazia). */
bool campo_nulo(char *valor, int tamanho)
{
    return tamanho == 0 || valor[0] == '\0';
}

/* Identifica se o campo corresponde a uma string do registro. */
bool campo_eh_texto(char *nome_campo)
{
    return strcmp(nome_campo, "nomeEstacao") == 0 || strcmp(nome_campo, "nomeLinha") == 0;
}

// Busca o valor de um campo inteiro no registro
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

// Busca o valor de um campo de texto no registro
// Retorna o ponteiro para a string e seu tamanho. Se o campo não for válido, retorna uma string vazia e tamanho 0.
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

// Lê pares (nomeCampo valorCampo) de critérios informados pelo usuário
int ler_criterio(Criterio *criterio)
{
    // Leitura do nomeCampo
    if (scanf("%31s", criterio->nome) != 1)
        return 0;

    // Leitura do valorCampo
    char str_campo_texto[TAMANHO_TEXTO] = {0};
    if (campo_eh_texto(criterio->nome))
    {
        // Campo textual pode conter espaços e vir entre aspas.
        ScanQuoteString(str_campo_texto);
    }
    else
    {
        if (scanf(" %127s", str_campo_texto) != 1)
            return 0;
    }

    // Caso valor do campo seja vazio (ou NULO em campos inteiros), trata como nulo.
    if (str_campo_texto[0] == '\0' || (!campo_eh_texto(criterio->nome) && strcmp(str_campo_texto, "NULO") == 0))
    {
        // Convenção do trabalho: vazio representa NULO.
        criterio->ehNulo = 1;
        criterio->valorInteiro = FLAG_CAMPO_NULO; // Valor para indicar nulo em campos inteiros;
        criterio->valorTexto[0] = '\0';
    }
    else
    {   // Verifica se o campo informado (nomeCampo) é textual (tamanho variável) ou não
        criterio->ehNulo = 0;
        if (campo_eh_texto(criterio->nome))
        {
            strncpy(criterio->valorTexto, str_campo_texto, TAMANHO_TEXTO - 1);
            criterio->valorTexto[TAMANHO_TEXTO - 1] = '\0';
        }
        else
        {
            criterio->valorInteiro = atoi(str_campo_texto);
        }
    }
    return 1;
}

// Verifica se registro atende aos critérios
int registro_atende_criterios(Registro *registro, Criterio *criterios, int quantidade)
{
    // A busca condicional aplica AND entre todos os pares nomeCampo/valorCampo.
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

// Auxiliar da funcionalidade 6 (atualização de registros) para aplicar os critérios de atualização no registro.
void aplicar_criterio_no_registro(Registro *registro, Criterio *criterio)
{
    // Atualiza somente os campos listados; os demais permanecem inalterados.
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