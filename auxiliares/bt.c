#include "bt.h"
#include "auxiliar.h"
#include <stdio.h>

// Funções Auxiliares da Árvore B (invisíveis ao usuário)

// Inicializa um nó da árvore B
void bt_no_inicializar(NO *no)
{
    no->removido = '0';
    no->proximo = NULO;
    no->tipoNo = 1;
    no->nroChaves = 0;

    for (int i = 0; i < CHAVES_MAX; i++)
    {
        no->chaves[i] = NULO;
        no->rrns[i] = NULO;
    }

    for (int i = 0; i < ORDEM; i++)
        no->filhos[i] = NULO;
}

// Calcula o byte offset para um dado RRN
long bt_offset_no(int rrn)
{
    return TAMANHO_CABECALHO + (long)rrn * TAM_NO;
}

// Lê um nó do arquivo de índice com árvore B
bool bt_ler_no(FILE *arquivo_indice, int rrn, NO *no)
{
    if (arquivo_indice == NULL || no == NULL)
        return false;

    if (fseek(arquivo_indice, bt_offset_no(rrn), SEEK_SET) != 0)
        return false;

    if (fread(&no->removido, sizeof(char), 1, arquivo_indice) != 1)
        return false;
    if (fread(&no->proximo, sizeof(int), 1, arquivo_indice) != 1)
        return false;
    if (fread(&no->tipoNo, sizeof(int), 1, arquivo_indice) != 1)
        return false;
    if (fread(&no->nroChaves, sizeof(int), 1, arquivo_indice) != 1)
        return false;

    for (int i = 0; i < CHAVES_MAX; i++)
    {
        if (fread(&no->chaves[i], sizeof(int), 1, arquivo_indice) != 1)
            return false;
        if (fread(&no->rrns[i], sizeof(int), 1, arquivo_indice) != 1)
            return false;
    }

    for (int i = 0; i < ORDEM; i++)
        if (fread(&no->filhos[i], sizeof(int), 1, arquivo_indice) != 1)
            return false;

    return true;
}

// Escreve um nó no arquivo de índice
bool bt_escrever_no(FILE *arquivo_indice, int rrn, NO *no)
{
    if (arquivo_indice == NULL || no == NULL)
        return false;

    if (fseek(arquivo_indice, bt_offset_no(rrn), SEEK_SET) != 0)
        return false;

    if (fwrite(&no->removido, sizeof(char), 1, arquivo_indice) != 1)
        return false;
    if (fwrite(&no->proximo, sizeof(int), 1, arquivo_indice) != 1)
        return false;
    if (fwrite(&no->tipoNo, sizeof(int), 1, arquivo_indice) != 1)
        return false;
    if (fwrite(&no->nroChaves, sizeof(int), 1, arquivo_indice) != 1)
        return false;

    for (int i = 0; i < CHAVES_MAX; i++)
    {
        if (fwrite(&no->chaves[i], sizeof(int), 1, arquivo_indice) != 1)
            return false;
        if (fwrite(&no->rrns[i], sizeof(int), 1, arquivo_indice) != 1)
            return false;
    }

    for (int i = 0; i < ORDEM; i++)
        if (fwrite(&no->filhos[i], sizeof(int), 1, arquivo_indice) != 1)
            return false;

    return true;
}

// Reserva/escolhe o próximo RRN disponível para uma inserção
int bt_reservar_rrn(CabecalhoBT *cabecalho_bt)
{
    int rrn = cabecalho_bt->proxRRN;
    cabecalho_bt->proxRRN++;
    cabecalho_bt->nroNos++;
    return rrn;
}

// Calcula a posição que uma chave deve ocupar dentro do nó da árvore B
int bt_obter_posicao(NO *no, int chave)
{
    int pos = 0;
    while (pos < no->nroChaves && chave > no->chaves[pos])
        pos++;
    return pos;
}

// Insere uma chave, seu RRN no arquivo de dados e seu filho direito em um nó
void bt_inserir_em_no(NO *no, int pos, int chave, int rrn_registro, int filho_dir)
{
    for (int i = no->nroChaves; i > pos; i--)
    {
        no->chaves[i] = no->chaves[i - 1];
        no->rrns[i] = no->rrns[i - 1];
    }

    for (int i = no->nroChaves + 1; i > pos + 1; i--)
        no->filhos[i] = no->filhos[i - 1];

    no->chaves[pos] = chave;
    no->rrns[pos] = rrn_registro;
    no->filhos[pos + 1] = filho_dir;
    no->nroChaves++;
    no->tipoNo = (no->filhos[0] == NULO) ? 1 : 0;
}

// Operação de split de um nó
int bt_dividir_no(FILE *arquivo_indice, CabecalhoBT *cabecalho_bt, int rrn_no, NO *no, int chave, int rrn_registro, int filho_dir, int *promo_chave, int *promo_rrn, int *promo_filho_dir)
{
    int pos = bt_obter_posicao(no, chave);
    int chaves_temp[CHAVES_MAX + 1];
    int rrns_temp[CHAVES_MAX + 1];
    int filhos_temp[ORDEM + 1];

    for (int i = 0; i < CHAVES_MAX + 1; i++)
    {
        chaves_temp[i] = NULO;
        rrns_temp[i] = NULO;
    }
    for (int i = 0; i < ORDEM + 1; i++)
        filhos_temp[i] = NULO;

    int indice_chave = 0;
    for (int i = 0; i < no->nroChaves + 1; i++)
    {
        if (i == pos)
        {
            chaves_temp[i] = chave;
            rrns_temp[i] = rrn_registro;
        }
        else
        {
            chaves_temp[i] = no->chaves[indice_chave];
            rrns_temp[i] = no->rrns[indice_chave];
            indice_chave++;
        }
    }

    int indice_filho = 0;
    for (int i = 0; i < no->nroChaves + 2; i++)
    {
        if (i == pos + 1)
            filhos_temp[i] = filho_dir;
        else
        {
            filhos_temp[i] = no->filhos[indice_filho];
            indice_filho++;
        }
    }

    int total_chaves = no->nroChaves + 1;
    int meio = (total_chaves + 1) / 2; 

    NO no_esquerdo;
    NO no_direito;
    bt_no_inicializar(&no_esquerdo);
    bt_no_inicializar(&no_direito);

    // Nó esquerdo (original modificado) fica com as primeiras chaves
    no_esquerdo.nroChaves = meio;
    for (int i = 0; i < meio; i++)
    {
        no_esquerdo.chaves[i] = chaves_temp[i];
        no_esquerdo.rrns[i] = rrns_temp[i];
    }
    for (int i = 0; i <= meio; i++)
        no_esquerdo.filhos[i] = filhos_temp[i];
    no_esquerdo.tipoNo = (no_esquerdo.filhos[0] == NULO) ? 1 : 0;

    *promo_chave = chaves_temp[meio];
    *promo_rrn = rrns_temp[meio];
    *promo_filho_dir = bt_reservar_rrn(cabecalho_bt);

    // Nó direito (Nova página criada SEMPRE à direita)
    no_direito.nroChaves = total_chaves - meio - 1;
    for (int i = 0; i < no_direito.nroChaves; i++)
    {
        no_direito.chaves[i] = chaves_temp[meio + 1 + i];
        no_direito.rrns[i] = rrns_temp[meio + 1 + i];
    }
    for (int i = 0; i <= no_direito.nroChaves; i++)
        no_direito.filhos[i] = filhos_temp[meio + 1 + i];
    no_direito.tipoNo = (no_direito.filhos[0] == NULO) ? 1 : 0;

    if (!bt_escrever_no(arquivo_indice, rrn_no, &no_esquerdo))
        return -1;
    if (!bt_escrever_no(arquivo_indice, *promo_filho_dir, &no_direito))
        return -1;

    return 1;
}

// Desce recursivamente até um nó folha da árvore B para realizar a inserção
int bt_inserir_recursivo(FILE *arquivo_indice, CabecalhoBT *cabecalho_bt, int rrn_no, int chave, int rrn_registro, int *promo_chave, int *promo_rrn, int *promo_filho_dir)
{
    NO no;
    if (!bt_ler_no(arquivo_indice, rrn_no, &no))
        return -1;

    int pos = bt_obter_posicao(&no, chave);
    if (pos < no.nroChaves && no.chaves[pos] == chave)
    {
        no.rrns[pos] = rrn_registro;
        return bt_escrever_no(arquivo_indice, rrn_no, &no) ? 0 : -1;
    }

    if (no.filhos[0] == NULO) // Nó Folha
    {
        if (no.nroChaves < CHAVES_MAX)
        {
            bt_inserir_em_no(&no, pos, chave, rrn_registro, NULO);
            return bt_escrever_no(arquivo_indice, rrn_no, &no) ? 0 : -1;
        }

        return bt_dividir_no(arquivo_indice, cabecalho_bt, rrn_no, &no, chave, rrn_registro, NULO, promo_chave, promo_rrn, promo_filho_dir);
    }

    int promo_chave_filho;
    int promo_rrn_filho;
    int promo_filho_dir_filho;
    
    int retorno = bt_inserir_recursivo(arquivo_indice, cabecalho_bt, no.filhos[pos], chave, rrn_registro, &promo_chave_filho, &promo_rrn_filho, &promo_filho_dir_filho);
    if (retorno != 1)
        return retorno;

    if (no.nroChaves < CHAVES_MAX)
    {
        bt_inserir_em_no(&no, pos, promo_chave_filho, promo_rrn_filho, promo_filho_dir_filho);
        return bt_escrever_no(arquivo_indice, rrn_no, &no) ? 0 : -1;
    }

    return bt_dividir_no(arquivo_indice, cabecalho_bt, rrn_no, &no, promo_chave_filho, promo_rrn_filho, promo_filho_dir_filho, promo_chave, promo_rrn, promo_filho_dir);
}

int bt_recuperar_registro(FILE *arquivo_indice, int rrn_raiz, int chave_busca) {
    if(rrn_raiz == -1) return -1;

    NO no_raiz;
    bt_ler_no(arquivo_indice, rrn_raiz, &no_raiz);
    for(int i = 0; i < no_raiz.nroChaves; i++) {
        if(no_raiz.chaves[i] == chave_busca)
            return no_raiz.rrns[i];
        else if(no_raiz.chaves[i] > chave_busca)
            return bt_recuperar_registro(arquivo_indice, no_raiz.filhos[i], chave_busca);
    }
    return bt_recuperar_registro(arquivo_indice, no_raiz.filhos[no_raiz.nroChaves], chave_busca);
}

// Funções Principais da Árvore B

// Realiza a inserção de nós/registros no arquivo de índice
bool inserir_registro_indice(FILE *arquivo_indice, CabecalhoBT *cabecalho_bt, int chave, int rrn_registro)
{
    if (arquivo_indice == NULL || cabecalho_bt == NULL)
        return false;

    if (cabecalho_bt->noRaiz == NULO)
    {
        int rrn_raiz = bt_reservar_rrn(cabecalho_bt);
        NO raiz;
        bt_no_inicializar(&raiz);
        raiz.nroChaves = 1;
        raiz.chaves[0] = chave;
        raiz.rrns[0] = rrn_registro;
        raiz.filhos[0] = NULO;
        raiz.filhos[1] = NULO;
        raiz.tipoNo = 1;
        cabecalho_bt->noRaiz = rrn_raiz;
        return bt_escrever_no(arquivo_indice, rrn_raiz, &raiz);
    }

    int promo_chave;
    int promo_rrn;
    int promo_filho_dir;
    int retorno = bt_inserir_recursivo(arquivo_indice, cabecalho_bt, cabecalho_bt->noRaiz, chave, rrn_registro, &promo_chave, &promo_rrn, &promo_filho_dir);
    if (retorno < 0)
        return false;

    if (retorno == 1)
    {
        int rrn_nova_raiz = bt_reservar_rrn(cabecalho_bt);
        NO raiz;
        bt_no_inicializar(&raiz);
        raiz.nroChaves = 1;
        raiz.chaves[0] = promo_chave;
        raiz.rrns[0] = promo_rrn;
        raiz.filhos[0] = cabecalho_bt->noRaiz;
        raiz.filhos[1] = promo_filho_dir;
        raiz.tipoNo = 0;
        cabecalho_bt->noRaiz = rrn_nova_raiz;

        if (!bt_escrever_no(arquivo_indice, rrn_nova_raiz, &raiz))
            return false;
    }

    return true;
}

int recuperar_registro_indice(FILE *arquivo_indice, CabecalhoBT *bt_cabecalho, int chave_busca) {
    if(arquivo_indice == NULL || bt_cabecalho == NULL) return -1;

    bt_recuperar_registro(arquivo_indice, bt_cabecalho->noRaiz, chave_busca);
}