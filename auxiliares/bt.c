#include "auxiliar.h"
#include "bt.h"

// Inicializa um nó da árvore B
void bt_no_inicializar(NO *no)
{
    no->removido = '0';
    no->proximo = NULO;
    no->tipoNo = -1;
    no->nroChaves = 0;

    for (int i = 0; i < CHAVES_MAX; i++)
    {
        no->chaves[i] = NULO;
        no->offsets[i] = NULO;
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
        if (fread(&no->offsets[i], sizeof(int), 1, arquivo_indice) != 1)
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
        if (fwrite(&no->offsets[i], sizeof(int), 1, arquivo_indice) != 1)
            return false;
    }

    for (int i = 0; i < ORDEM; i++)
        if (fwrite(&no->filhos[i], sizeof(int), 1, arquivo_indice) != 1)
            return false;

    return true;
}

// Reserva/escolhe o próximo RRN disponível para uma inserção.
int bt_alocar_no(FILE *arquivo_indice, CabecalhoBT *cabecalho_bt)
{
    int rrn;
    if (cabecalho_bt->topo != NULO)
    {
        // Reaproveita o RRN do topo da pilha de páginas removidas
        rrn = cabecalho_bt->topo;
        NO no_removido;
        bt_ler_no(arquivo_indice, rrn, &no_removido);
        cabecalho_bt->topo = no_removido.proximo; // desempilha
    }
    else
    {
        rrn = cabecalho_bt->proxRRN;
        cabecalho_bt->proxRRN++;
    }
    cabecalho_bt->nroNos++;
    return rrn;
}

// Libera um nó: marca como removido e o empilha na pilha de páginas livres.
// Mantém todos os demais campos do nó inalterados.
void bt_liberar_no(FILE *arquivo_indice, CabecalhoBT *cabecalho_bt, int rrn)
{
    NO no;
    bt_ler_no(arquivo_indice, rrn, &no);
    no.removido = '1';
    no.proximo = cabecalho_bt->topo;
    cabecalho_bt->topo = rrn;
    cabecalho_bt->nroNos--;
    bt_escrever_no(arquivo_indice, rrn, &no);
}

// Calcula a posição que uma chave deve ocupar dentro do nó da árvore B
int bt_obter_posicao(NO *no, int chave)
{
    int pos = 0;
    while (pos < no->nroChaves && chave > no->chaves[pos])
        pos++;
    return pos;
}

// Insere uma chave, seu offset no arquivo de dados e seu filho direito em um nó
void bt_inserir_em_no(NO *no, int pos, int chave, int offset_registro, int filho_dir)
{
    for (int i = no->nroChaves; i > pos; i--)
    {
        no->chaves[i] = no->chaves[i - 1];
        no->offsets[i] = no->offsets[i - 1];
    }

    for (int i = no->nroChaves + 1; i > pos + 1; i--)
        no->filhos[i] = no->filhos[i - 1];

    no->chaves[pos] = chave;
    no->offsets[pos] = offset_registro;
    no->filhos[pos + 1] = filho_dir;
    no->nroChaves++;
    no->tipoNo = (no->filhos[0] == NULO) ? -1 : 1;
}

// Operação de split de um nó
int bt_dividir_no(FILE *arquivo_indice, CabecalhoBT *cabecalho_bt, int rrn_no, NO *no, int chave, int offset_registro, int filho_dir, int *promo_chave, int *promo_offset, int *promo_filho_dir)
{
    int pos = bt_obter_posicao(no, chave);
    int chaves_temp[CHAVES_MAX + 1];
    int offsets_temp[CHAVES_MAX + 1];
    int filhos_temp[ORDEM + 1];

    for (int i = 0; i < CHAVES_MAX + 1; i++)
    {
        chaves_temp[i] = NULO;
        offsets_temp[i] = NULO;
    }
    for (int i = 0; i < ORDEM + 1; i++)
        filhos_temp[i] = NULO;

    int indice_chave = 0;
    for (int i = 0; i < no->nroChaves + 1; i++)
    {
        if (i == pos)
        {
            chaves_temp[i] = chave;
            offsets_temp[i] = offset_registro;
        }
        else
        {
            chaves_temp[i] = no->chaves[indice_chave];
            offsets_temp[i] = no->offsets[indice_chave];
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
        no_esquerdo.offsets[i] = offsets_temp[i];
    }
    for (int i = 0; i <= meio; i++)
        no_esquerdo.filhos[i] = filhos_temp[i];
    no_esquerdo.tipoNo = (no_esquerdo.filhos[0] == NULO) ? -1 : 1;

    *promo_chave = chaves_temp[meio];
    *promo_offset = offsets_temp[meio];
    *promo_filho_dir = bt_alocar_no(arquivo_indice, cabecalho_bt);

    // Nó direito (Nova página criada SEMPRE à direita)
    no_direito.nroChaves = total_chaves - meio - 1;
    for (int i = 0; i < no_direito.nroChaves; i++)
    {
        no_direito.chaves[i] = chaves_temp[meio + 1 + i];
        no_direito.offsets[i] = offsets_temp[meio + 1 + i];
    }
    for (int i = 0; i <= no_direito.nroChaves; i++)
        no_direito.filhos[i] = filhos_temp[meio + 1 + i];
    no_direito.tipoNo = (no_direito.filhos[0] == NULO) ? -1 : 1;

    if (!bt_escrever_no(arquivo_indice, rrn_no, &no_esquerdo))
        return -1;
    if (!bt_escrever_no(arquivo_indice, *promo_filho_dir, &no_direito))
        return -1;

    return 1;
}

// Desce recursivamente até um nó folha da árvore B para realizar a inserção
int bt_inserir_recursivo(FILE *arquivo_indice, CabecalhoBT *cabecalho_bt, int rrn_no, int chave, int offset_registro, int *promo_chave, int *promo_offset, int *promo_filho_dir)
{
    NO no;
    if (!bt_ler_no(arquivo_indice, rrn_no, &no))
        return -1;

    int pos = bt_obter_posicao(&no, chave);
    if (pos < no.nroChaves && no.chaves[pos] == chave)
    {
        no.offsets[pos] = offset_registro;
        return bt_escrever_no(arquivo_indice, rrn_no, &no) ? 0 : -1;
    }

    if (no.filhos[0] == NULO) // Nó Folha
    {
        if (no.nroChaves < CHAVES_MAX)
        {
            bt_inserir_em_no(&no, pos, chave, offset_registro, NULO);
            return bt_escrever_no(arquivo_indice, rrn_no, &no) ? 0 : -1;
        }

        return bt_dividir_no(arquivo_indice, cabecalho_bt, rrn_no, &no, chave, offset_registro, NULO, promo_chave, promo_offset, promo_filho_dir);
    }

    int promo_chave_filho;
    int promo_offset_filho;
    int promo_filho_dir_filho;

    int retorno = bt_inserir_recursivo(arquivo_indice, cabecalho_bt, no.filhos[pos], chave, offset_registro, &promo_chave_filho, &promo_offset_filho, &promo_filho_dir_filho);
    if (retorno != 1)
        return retorno;

    if (no.nroChaves < CHAVES_MAX)
    {
        bt_inserir_em_no(&no, pos, promo_chave_filho, promo_offset_filho, promo_filho_dir_filho);
        return bt_escrever_no(arquivo_indice, rrn_no, &no) ? 0 : -1;
    }

    return bt_dividir_no(arquivo_indice, cabecalho_bt, rrn_no, &no, promo_chave_filho, promo_offset_filho, promo_filho_dir_filho, promo_chave, promo_offset, promo_filho_dir);
}

int bt_recuperar_registro(FILE *arquivo_indice, int rrn_raiz, int chave_busca)
{
    if (rrn_raiz == -1)
        return -1;

    NO no_raiz;
    bt_ler_no(arquivo_indice, rrn_raiz, &no_raiz);
    for (int i = 0; i < no_raiz.nroChaves; i++)
    {
        if (no_raiz.chaves[i] == chave_busca)
            return no_raiz.offsets[i];
        else if (no_raiz.chaves[i] > chave_busca)
            return bt_recuperar_registro(arquivo_indice, no_raiz.filhos[i], chave_busca);
    }
    return bt_recuperar_registro(arquivo_indice, no_raiz.filhos[no_raiz.nroChaves], chave_busca);
}

// Funções Principais da Árvore B

// Realiza a inserção de nós/registros no arquivo de índice
bool inserir_indice(FILE *arquivo_indice, CabecalhoBT *cabecalho_bt, int chave, int offset_registro)
{
    if (arquivo_indice == NULL || cabecalho_bt == NULL)
        return false;

    if (cabecalho_bt->noRaiz == NULO)
    {
        int rrn_raiz = bt_alocar_no(arquivo_indice, cabecalho_bt);
        NO raiz;
        bt_no_inicializar(&raiz);
        raiz.nroChaves = 1;
        raiz.chaves[0] = chave;
        raiz.offsets[0] = offset_registro;
        raiz.filhos[0] = NULO;
        raiz.filhos[1] = NULO;
        raiz.tipoNo = -1;
        cabecalho_bt->noRaiz = rrn_raiz;
        return bt_escrever_no(arquivo_indice, rrn_raiz, &raiz);
    }

    int promo_chave;
    int promo_offset;
    int promo_filho_dir;
    int retorno = bt_inserir_recursivo(arquivo_indice, cabecalho_bt, cabecalho_bt->noRaiz, chave, offset_registro, &promo_chave, &promo_offset, &promo_filho_dir);
    if (retorno < 0)
        return false;

    if (retorno == 1)
    {
        int rrn_raiz_antiga = cabecalho_bt->noRaiz;
        int rrn_nova_raiz = bt_alocar_no(arquivo_indice, cabecalho_bt);

        // A antiga raiz deixa de ser raiz: vira folha ou nó intermediário
        NO raiz_antiga;
        bt_ler_no(arquivo_indice, rrn_raiz_antiga, &raiz_antiga);
        raiz_antiga.tipoNo = (raiz_antiga.filhos[0] == NULO) ? -1 : 1;
        bt_escrever_no(arquivo_indice, rrn_raiz_antiga, &raiz_antiga);

        NO raiz;
        bt_no_inicializar(&raiz);
        raiz.nroChaves = 1;
        raiz.chaves[0] = promo_chave;
        raiz.offsets[0] = promo_offset;
        raiz.filhos[0] = rrn_raiz_antiga;
        raiz.filhos[1] = promo_filho_dir;
        raiz.tipoNo = 0;
        cabecalho_bt->noRaiz = rrn_nova_raiz;

        if (!bt_escrever_no(arquivo_indice, rrn_nova_raiz, &raiz))
            return false;
    }

    return true;
}

// Remove a chave na posição 'pos' de um nó, deslocando os elementos à esquerda
void bt_remover_de_no(NO *no, int pos)
{
    for (int i = pos; i < no->nroChaves - 1; i++)
    {
        no->chaves[i] = no->chaves[i + 1];
        no->offsets[i] = no->offsets[i + 1];
    }
    no->chaves[no->nroChaves - 1] = NULO;
    no->offsets[no->nroChaves - 1] = NULO;
    no->nroChaves--;
}

// Encontra a chave sucessora imediata: a menor chave da subárvore (descendo sempre
// pelo filho mais à esquerda até atingir uma folha)
void bt_obter_sucessor(FILE *arquivo_indice, int rrn_no, int *chave, int *rrn)
{
    NO no;
    bt_ler_no(arquivo_indice, rrn_no, &no);
    while (no.filhos[0] != NULO)
        bt_ler_no(arquivo_indice, no.filhos[0], &no);

    *chave = no.chaves[0];
    *rrn = no.offsets[0];
}

// Redistribuição tomando uma chave do irmão à direita ('dir').
// A primeira chave de 'dir' é promovida para o pai; o antigo separador do pai
// desce para o final de 'esq'. Se não for folha, o filho mais à esquerda de
// 'dir' passa a ser o último filho de 'esq'.
void bt_redistribuir_direita(NO *pai, NO *esq, NO *dir, int idx, bool folha)
{
    int k = esq->nroChaves;

    esq->chaves[k] = pai->chaves[idx];
    esq->offsets[k] = pai->offsets[idx];
    if (!folha)
        esq->filhos[k + 1] = dir->filhos[0];
    esq->nroChaves++;

    pai->chaves[idx] = dir->chaves[0];
    pai->offsets[idx] = dir->offsets[0];

    for (int i = 0; i < dir->nroChaves - 1; i++)
    {
        dir->chaves[i] = dir->chaves[i + 1];
        dir->offsets[i] = dir->offsets[i + 1];
    }
    dir->chaves[dir->nroChaves - 1] = NULO;
    dir->offsets[dir->nroChaves - 1] = NULO;

    if (!folha)
    {
        for (int i = 0; i < ORDEM - 1; i++)
            dir->filhos[i] = dir->filhos[i + 1];
        dir->filhos[ORDEM - 1] = NULO;
    }
    dir->nroChaves--;
}

// Redistribuição tomando uma chave do irmão à esquerda ('esq').
// A última chave de 'esq' é promovida para o pai; o antigo separador do pai
// desce para o início de 'dir'. Se não for folha, o último filho de 'esq'
// passa a ser o primeiro filho de 'dir'.
void bt_redistribuir_esquerda(NO *pai, NO *esq, NO *dir, int idx, bool folha)
{
    for (int i = dir->nroChaves; i > 0; i--)
    {
        dir->chaves[i] = dir->chaves[i - 1];
        dir->offsets[i] = dir->offsets[i - 1];
    }
    if (!folha)
        for (int i = ORDEM - 1; i > 0; i--)
            dir->filhos[i] = dir->filhos[i - 1];

    dir->chaves[0] = pai->chaves[idx];
    dir->offsets[0] = pai->offsets[idx];
    if (!folha)
        dir->filhos[0] = esq->filhos[esq->nroChaves];
    dir->nroChaves++;

    int ultimo = esq->nroChaves - 1;
    pai->chaves[idx] = esq->chaves[ultimo];
    pai->offsets[idx] = esq->offsets[ultimo];

    esq->chaves[ultimo] = NULO;
    esq->offsets[ultimo] = NULO;
    if (!folha)
        esq->filhos[ultimo + 1] = NULO;
    esq->nroChaves--;
}

// Concatenação: o separador do pai (índice 'idx') e todas as chaves/filhos de
// 'dir' são incorporados ao final de 'esq'. 'dir' é a página destruída.
void bt_concatenar(NO *pai, NO *esq, NO *dir, int idx, bool folha)
{
    int k = esq->nroChaves;

    esq->chaves[k] = pai->chaves[idx];
    esq->offsets[k] = pai->offsets[idx];
    k++;

    for (int i = 0; i < dir->nroChaves; i++)
    {
        esq->chaves[k + i] = dir->chaves[i];
        esq->offsets[k + i] = dir->offsets[i];
    }

    if (!folha)
        for (int i = 0; i <= dir->nroChaves; i++)
            esq->filhos[k + i] = dir->filhos[i];

    esq->nroChaves = k + dir->nroChaves;

    // Remove o separador (idx) e o ponteiro para 'dir' (idx+1) do pai
    for (int i = idx; i < pai->nroChaves - 1; i++)
    {
        pai->chaves[i] = pai->chaves[i + 1];
        pai->offsets[i] = pai->offsets[i + 1];
    }
    for (int i = idx + 1; i < pai->nroChaves; i++)
        pai->filhos[i] = pai->filhos[i + 1];

    pai->chaves[pai->nroChaves - 1] = NULO;
    pai->offsets[pai->nroChaves - 1] = NULO;
    pai->filhos[pai->nroChaves] = NULO;
    pai->nroChaves--;
}

// Verifica se o filho 'idx_filho' de 'rrn_pai' está em underflow (nroChaves < CHAVES_MIN)
// e, caso esteja, aplica: 1) redistribuição com irmão direito, 2) redistribuição com
// irmão esquerdo, 3) concatenação com irmão esquerdo, 4) concatenação com irmão direito
// — exatamente nessa ordem, conforme a especificação.
void bt_corrigir_underflow(FILE *arquivo_indice, CabecalhoBT *cabecalho_bt, int rrn_pai, int idx_filho)
{
    NO pai, filho;
    bt_ler_no(arquivo_indice, rrn_pai, &pai);

    int rrn_filho = pai.filhos[idx_filho];
    bt_ler_no(arquivo_indice, rrn_filho, &filho);

    if (filho.nroChaves >= CHAVES_MIN)
        return; // sem underflow, nada a fazer

    bool folha = (filho.filhos[0] == NULO);

    // 1. Redistribuição com o irmão à direita
    if (idx_filho < pai.nroChaves)
    {
        int rrn_dir = pai.filhos[idx_filho + 1];
        NO direito;
        bt_ler_no(arquivo_indice, rrn_dir, &direito);
        if (direito.nroChaves > CHAVES_MIN)
        {
            bt_redistribuir_direita(&pai, &filho, &direito, idx_filho, folha);
            bt_escrever_no(arquivo_indice, rrn_pai, &pai);
            bt_escrever_no(arquivo_indice, rrn_filho, &filho);
            bt_escrever_no(arquivo_indice, rrn_dir, &direito);
            return;
        }
    }

    // 2. Redistribuição com o irmão à esquerda
    if (idx_filho > 0)
    {
        int rrn_esq = pai.filhos[idx_filho - 1];
        NO esquerdo;
        bt_ler_no(arquivo_indice, rrn_esq, &esquerdo);
        if (esquerdo.nroChaves > CHAVES_MIN)
        {
            bt_redistribuir_esquerda(&pai, &esquerdo, &filho, idx_filho - 1, folha);
            bt_escrever_no(arquivo_indice, rrn_pai, &pai);
            bt_escrever_no(arquivo_indice, rrn_esq, &esquerdo);
            bt_escrever_no(arquivo_indice, rrn_filho, &filho);
            return;
        }
    }

    // 3. Concatenação com o irmão à esquerda (página direita é destruída)
    if (idx_filho > 0)
    {
        int rrn_esq = pai.filhos[idx_filho - 1];
        NO esquerdo;
        bt_ler_no(arquivo_indice, rrn_esq, &esquerdo);

        bt_concatenar(&pai, &esquerdo, &filho, idx_filho - 1, folha);

        bt_escrever_no(arquivo_indice, rrn_esq, &esquerdo);
        bt_escrever_no(arquivo_indice, rrn_pai, &pai);
        bt_liberar_no(arquivo_indice, cabecalho_bt, rrn_filho);
        return;
    }

    // 4. Concatenação com o irmão à direita (página direita é destruída)
    {
        int rrn_dir = pai.filhos[idx_filho + 1];
        NO direito;
        bt_ler_no(arquivo_indice, rrn_dir, &direito);

        bt_concatenar(&pai, &filho, &direito, idx_filho, folha);

        bt_escrever_no(arquivo_indice, rrn_filho, &filho);
        bt_escrever_no(arquivo_indice, rrn_pai, &pai);
        bt_liberar_no(arquivo_indice, cabecalho_bt, rrn_dir);
    }
}

// Remove recursivamente 'chave' da subárvore enraizada em 'rrn_no'.
// Retorna true se a chave existia e foi removida.
bool bt_remover_rec(FILE *arquivo_indice, CabecalhoBT *cabecalho_bt, int rrn_no, int chave)
{
    NO no;
    if (!bt_ler_no(arquivo_indice, rrn_no, &no))
        return false;

    int pos = bt_obter_posicao(&no, chave);
    bool eh_folha = (no.filhos[0] == NULO);
    bool achou = (pos < no.nroChaves && no.chaves[pos] == chave);

    if (achou)
    {
        if (eh_folha)
        {
            // Remoção direta em folha
            bt_remover_de_no(&no, pos);
            return bt_escrever_no(arquivo_indice, rrn_no, &no);
        }

        // Chave está em nó interno: troca pela sucessora imediata (folha)
        int chave_suc, rrn_suc;
        bt_obter_sucessor(arquivo_indice, no.filhos[pos + 1], &chave_suc, &rrn_suc);

        no.chaves[pos] = chave_suc;
        no.offsets[pos] = rrn_suc;
        if (!bt_escrever_no(arquivo_indice, rrn_no, &no))
            return false;

        // Remove a chave sucessora da folha onde ela estava
        if (!bt_remover_rec(arquivo_indice, cabecalho_bt, no.filhos[pos + 1], chave_suc))
            return false;

        bt_corrigir_underflow(arquivo_indice, cabecalho_bt, rrn_no, pos + 1);
        return true;
    }

    if (eh_folha)
        return false; // chave não encontrada na árvore

    // Desce recursivamente para o filho apropriado
    if (!bt_remover_rec(arquivo_indice, cabecalho_bt, no.filhos[pos], chave))
        return false;

    bt_corrigir_underflow(arquivo_indice, cabecalho_bt, rrn_no, pos);
    return true;
}

// Remove a chave do índice árvore-B, ajustando a raiz caso a árvore encolha.
// Retorna true se a chave existia e foi removida com sucesso.
bool remover_registro_indice(FILE *arquivo_indice, CabecalhoBT *cabecalho_bt, int chave)
{
    if (arquivo_indice == NULL || cabecalho_bt == NULL)
        return false;

    if (cabecalho_bt->noRaiz == NULO)
        return false; // árvore vazia

    if (!bt_remover_rec(arquivo_indice, cabecalho_bt, cabecalho_bt->noRaiz, chave))
        return false;

    // Verifica se a raiz ficou sem chaves (encolhimento da árvore)
    NO raiz;
    bt_ler_no(arquivo_indice, cabecalho_bt->noRaiz, &raiz);

    if (raiz.nroChaves == 0)
    {
        int rrn_raiz_antiga = cabecalho_bt->noRaiz;

        if (raiz.filhos[0] == NULO)
        {
            // A raiz era folha e ficou vazia: a árvore inteira ficou vazia
            cabecalho_bt->noRaiz = NULO;
        }
        else
        {
            // O único filho restante da raiz passa a ser a nova raiz
            cabecalho_bt->noRaiz = raiz.filhos[0];

            NO nova_raiz;
            bt_ler_no(arquivo_indice, cabecalho_bt->noRaiz, &nova_raiz);
            nova_raiz.tipoNo = (nova_raiz.filhos[0] == NULO) ? -1 : 0;
            bt_escrever_no(arquivo_indice, cabecalho_bt->noRaiz, &nova_raiz);
        }

        bt_liberar_no(arquivo_indice, cabecalho_bt, rrn_raiz_antiga);
    }

    return true;
}

// Implementação efetiva de recuperar_registro_indice (busca na árvore-B)
int recuperar_registro_indice(FILE *arquivo_indice, CabecalhoBT *bt_cabecalho, int chave_busca)
{
    if (arquivo_indice == NULL || bt_cabecalho == NULL)
        return NULO;

    int rrn_atual = bt_cabecalho->noRaiz;
    while (rrn_atual != NULO)
    {
        NO no;
        if (!bt_ler_no(arquivo_indice, rrn_atual, &no))
            return NULO;

        int pos = bt_obter_posicao(&no, chave_busca);
        if (pos < no.nroChaves && no.chaves[pos] == chave_busca)
            return no.offsets[pos]; // RRN do registro no arquivo de dados

        if (no.filhos[0] == NULO)
            return NULO; // folha: chave não existe

        rrn_atual = no.filhos[pos];
    }

    return NULO;
}