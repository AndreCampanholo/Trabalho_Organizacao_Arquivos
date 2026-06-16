#include "auxiliar.h"
#include "bt.h"

// Inicializa todos os campos de um nó com valores padrão/nulos.
void bt_no_inicializar(NO *no)
{
    no->removido = '0';
    no->proximo = NULO;
    no->tipoNo = -1; // folha por padrão
    no->nroChaves = 0;

    for (int i = 0; i < CHAVES_MAX; i++)
    {
        no->chaves[i] = NULO;
        no->offsets[i] = NULO;
    }

    for (int i = 0; i < ORDEM; i++)
        no->filhos[i] = NULO;
}

// Retorna o byte offset do nó de RRN 'rrn' no arquivo de índice. Esquema cabeçalho de TAMANHO_CABECALHO bytes seguido de blocos de TAM_NO bytes.
long bt_offset_no(int rrn)
{
    return TAMANHO_CABECALHO + (long)rrn * TAM_NO;
}

// Lê o nó de RRN 'rrn' do arquivo de índice para a struct apontada por 'no'. Os pares (chave, offset) são lidos intercalados conforme o esquema de disco citado acima. Retorna true em sucesso, false em falha de seek ou read.
bool bt_ler_no(FILE *arq_indice, int rrn, NO *no)
{
    if (arq_indice == NULL || no == NULL)
        return false;

    if (fseek(arq_indice, bt_offset_no(rrn), SEEK_SET) != 0)
        return false;

    if (fread(&no->removido, sizeof(char), 1, arq_indice) != 1)
        return false;
    if (fread(&no->proximo, sizeof(int), 1, arq_indice) != 1)
        return false;
    if (fread(&no->tipoNo, sizeof(int), 1, arq_indice) != 1)
        return false;
    if (fread(&no->nroChaves, sizeof(int), 1, arq_indice) != 1)
        return false;

    // Pares (chave, offset) intercalados C1 PR1 C2 PR2 C3 PR3
    for (int i = 0; i < CHAVES_MAX; i++)
    {
        if (fread(&no->chaves[i], sizeof(int), 1, arq_indice) != 1)
            return false;
        if (fread(&no->offsets[i], sizeof(int), 1, arq_indice) != 1)
            return false;
    }

    for (int i = 0; i < ORDEM; i++)
        if (fread(&no->filhos[i], sizeof(int), 1, arq_indice) != 1)
            return false;

    return true;
}

// Escreve a struct 'no' no arquivo de índice na posição do RRN 'rrn'. Os campos são escritos campo a campo para evitar o padding interno da struct. Retorna true em sucesso, false em falha de seek ou write.
bool bt_escrever_no(FILE *arq_indice, int rrn, NO *no)
{
    if (arq_indice == NULL || no == NULL)
        return false;

    if (fseek(arq_indice, bt_offset_no(rrn), SEEK_SET) != 0)
        return false;

    if (fwrite(&no->removido, sizeof(char), 1, arq_indice) != 1)
        return false;
    if (fwrite(&no->proximo, sizeof(int), 1, arq_indice) != 1)
        return false;
    if (fwrite(&no->tipoNo, sizeof(int), 1, arq_indice) != 1)
        return false;
    if (fwrite(&no->nroChaves, sizeof(int), 1, arq_indice) != 1)
        return false;

    for (int i = 0; i < CHAVES_MAX; i++)
    {
        if (fwrite(&no->chaves[i], sizeof(int), 1, arq_indice) != 1)
            return false;
        if (fwrite(&no->offsets[i], sizeof(int), 1, arq_indice) != 1)
            return false;
    }

    for (int i = 0; i < ORDEM; i++)
        if (fwrite(&no->filhos[i], sizeof(int), 1, arq_indice) != 1)
            return false;

    return true;
}

// Reserva um RRN para um novo nó. Se a pilha de removidos não estiver vazia (topo != NULO), o espaço é reaproveitado o nó do topo e avançamos o ponteiro da pilha. Caso contrário, usamos proxRRN.
int bt_alocar_no(FILE *arq_indice, CabecalhoBT *cab)
{
    int rrn;
    if (cab->topo != NULO)
    {
        // Reaproveitamento lê o campo 'proximo' do nó removido para desempilhar
        rrn = cab->topo;
        NO no_removido;
        bt_ler_no(arq_indice, rrn, &no_removido);
        cab->topo = no_removido.proximo;
    }
    else
    {
        // Sem espaço reutilizável expande o arquivo alocando um RRN novo ao final
        rrn = cab->proxRRN;
        cab->proxRRN++;
    }
    cab->nroNos++;
    return rrn;
}

// Libera o nó de RRN 'rrn' logicamente marca removido='1', encadeia na pilha atualizando o campo 'proximo' com o antigo topo e promovendo 'rrn' ao novo topo. Os demais campos do nó permanecem intactos no disco, conforme a especificação.
void bt_liberar_no(FILE *arq_indice, CabecalhoBT *cab, int rrn)
{
    NO no;
    bt_ler_no(arq_indice, rrn, &no);

    no.removido = '1';
    no.proximo = cab->topo; // encadeia no antigo topo da pilha

    cab->topo = rrn; // este nó passa a ser o novo topo
    cab->nroNos--;

    bt_escrever_no(arq_indice, rrn, &no);
}

// Retorna o índice da primeira posição em que 'chave' pode ser inserida no nó, ou seja, a primeira posição i tal que chave <= no->chaves[i]. Se chave for maior que todas as chaves presentes, retorna no->nroChaves.
int bt_obter_posicao(NO *no, int chave)
{
    int pos = 0;
    while (pos < no->nroChaves && chave > no->chaves[pos])
        pos++;
    return pos;
}

// Insere 'chave', 'offset_registro' e o filho direito 'filho_dir' na posição 'pos' do nó. Desloca os elementos existentes à direita para abrir espaço. Atualiza tipoNo com base na presença de filhos (-1) folha, (1) intermediário.
void bt_inserir_em_no(NO *no, int pos, int chave, int offset_registro, int filho_dir)
{
    // Abre espaço para a nova chave deslocando os elementos à direita
    for (int i = no->nroChaves; i > pos; i--)
    {
        no->chaves[i] = no->chaves[i - 1];
        no->offsets[i] = no->offsets[i - 1];
    }

    // Abre espaço para o novo filho direito (fica à direita da chave inserida)
    for (int i = no->nroChaves + 1; i > pos + 1; i--)
        no->filhos[i] = no->filhos[i - 1];

    no->chaves[pos] = chave;
    no->offsets[pos] = offset_registro;
    no->filhos[pos + 1] = filho_dir;
    no->nroChaves++;

    // Preserva o tipoNo == 0 se este nó é a raiz, já que essa nunca vira uma folha (-1) ou um nó intermediario (1)
    if (no->tipoNo != 0)
        no->tipoNo = (no->filhos[0] == NULO) ? -1 : 1;
}

// Remove a chave na posição 'pos' do nó deslocando os elementos restantes à esquerda. Limpa com NULO as posições que ficam descobertas após o deslocamento.
void bt_remover_de_no(NO *no, int pos)
{
    // Desloca chaves e offsets à esquerda para preencher a lacuna em 'pos'
    for (int i = pos; i < no->nroChaves - 1; i++)
    {
        no->chaves[i] = no->chaves[i + 1];
        no->offsets[i] = no->offsets[i + 1];
    }

    // Limpa a última posição que ficou duplicada após o deslocamento
    no->chaves[no->nroChaves - 1] = NULO;
    no->offsets[no->nroChaves - 1] = NULO;
    no->nroChaves--;
}

// Desce sempre pelo filho mais à esquerda a partir do nó de RRN 'rrn_inicio' até atingir uma folha. Preenche '*chave_suc' e '*offset_suc' com os valores da menor chave encontrada (sucessora imediata da chave sendo removida).
void bt_obter_sucessor(FILE *arq_indice, int rrn_inicio, int *chave_suc, int *offset_suc)
{
    NO no;
    bt_ler_no(arq_indice, rrn_inicio, &no);

    // Desce pelo filho mais à esquerda até atingir uma folha
    while (no.filhos[0] != NULO)
        bt_ler_no(arq_indice, no.filhos[0], &no);

    // A menor chave da folha é a sucessora imediata
    *chave_suc = no.chaves[0];
    *offset_suc = no.offsets[0];
}

// Divide o nó 'no' (RRN 'rrn_no') que está cheio ao receber 'chave'/'offset_registro'/'filho_dir'. Monta arrays temporários com CHAVES_MAX+1 chaves intercalando a nova, determina a mediana e divide em nó esquerdo (original reaproveitado) e nó direito (nova página alocada). A mediana sobe para o pai via os ponteiros 'promo_*'. A nova página é SEMPRE criada à direita, conforme a especificação. Retorna 1 (promoção necessária) ou -1 (falha de escrita).
int bt_dividir_no(FILE *arq_indice, CabecalhoBT *cab,
                  int rrn_no, NO *no,
                  int chave, int offset_registro, int filho_dir,
                  int *promo_chave, int *promo_offset, int *promo_rrn_dir)
{
    int pos = bt_obter_posicao(no, chave);

    // Arrays temporários para acomodar as CHAVES_MAX+1 chaves e ORDEM+1 filhos resultantes da inserção da nova chave num nó que já estava cheio
    int chaves_tmp[CHAVES_MAX + 1];
    int offsets_tmp[CHAVES_MAX + 1];
    int filhos_tmp[ORDEM + 1];

    for (int i = 0; i < CHAVES_MAX + 1; i++)
    {
        chaves_tmp[i] = NULO;
        offsets_tmp[i] = NULO;
    }
    for (int i = 0; i < ORDEM + 1; i++)
        filhos_tmp[i] = NULO;

    // Intercala a nova chave na posição 'pos', copiando as existentes ao redor
    int i_chave = 0; // índice sobre as chaves originais do nó durante a intercalação
    for (int i = 0; i < no->nroChaves + 1; i++)
    {
        if (i == pos)
        {
            chaves_tmp[i] = chave;
            offsets_tmp[i] = offset_registro;
        }
        else
        {
            chaves_tmp[i] = no->chaves[i_chave];
            offsets_tmp[i] = no->offsets[i_chave];
            i_chave++;
        }
    }

    // Intercala o filho direito da nova chave na posição 'pos+1'
    int i_filho = 0; // índice sobre os filhos originais do nó durante a intercalação
    for (int i = 0; i < no->nroChaves + 2; i++)
    {
        if (i == pos + 1)
            filhos_tmp[i] = filho_dir;
        else
        {
            filhos_tmp[i] = no->filhos[i_filho];
            i_filho++;
        }
    }

    // Calcula o índice da mediana. (total+1)/2 garante que o nó esquerdo receba uma chave a mais quando ímpar, conforme exigido pela especificação.
    int total = no->nroChaves + 1;
    int meio = (total + 1) / 2; // índice da chave que será promovida ao pai

    NO no_esq, no_dir;
    bt_no_inicializar(&no_esq);
    bt_no_inicializar(&no_dir);

    // Nó esquerdo reaproveitado recebe as 'meio' primeiras chaves
    no_esq.nroChaves = meio;
    for (int i = 0; i < meio; i++)
    {
        no_esq.chaves[i] = chaves_tmp[i];
        no_esq.offsets[i] = offsets_tmp[i];
    }
    for (int i = 0; i <= meio; i++)
        no_esq.filhos[i] = filhos_tmp[i];
    no_esq.tipoNo = (no_esq.filhos[0] == NULO) ? -1 : 1;

    // A chave na posição 'meio' sobe para o pai
    *promo_chave = chaves_tmp[meio];
    *promo_offset = offsets_tmp[meio];

    // Nova página à direita recebe as chaves após a mediana
    *promo_rrn_dir = bt_alocar_no(arq_indice, cab);
    no_dir.nroChaves = total - meio - 1;
    for (int i = 0; i < no_dir.nroChaves; i++)
    {
        no_dir.chaves[i] = chaves_tmp[meio + 1 + i];
        no_dir.offsets[i] = offsets_tmp[meio + 1 + i];
    }
    for (int i = 0; i <= no_dir.nroChaves; i++)
        no_dir.filhos[i] = filhos_tmp[meio + 1 + i];
    no_dir.tipoNo = (no_dir.filhos[0] == NULO) ? -1 : 1;

    if (!bt_escrever_no(arq_indice, rrn_no, &no_esq))
        return -1;
    if (!bt_escrever_no(arq_indice, *promo_rrn_dir, &no_dir))
        return -1;

    return 1; // sinaliza ao chamador que há promoção pendente
}

// Desce recursivamente até a folha correta para inserir 'chave'/'offset_registro'. Retorna 0 = inserido sem promoção, 1 = promoção necessária (promo_* preenchidos), -1 = erro.
int bt_inserir_rec(FILE *arq_indice, CabecalhoBT *cab, int rrn_no, int chave, int offset_registro, int *promo_chave, int *promo_offset, int *promo_rrn_dir)
{
    NO no;
    if (!bt_ler_no(arq_indice, rrn_no, &no))
        return -1;

    int pos = bt_obter_posicao(&no, chave);

    // Se a chave for duplicada, retorna 0 imediatamente
    if (pos < no.nroChaves && no.chaves[pos] == chave)
        return 0;

    if (no.filhos[0] == NULO) // nó folha: inserção ocorre aqui
    {
        if (no.nroChaves < CHAVES_MAX)
        {
            // Há espaço insere diretamente sem necessidade de split
            bt_inserir_em_no(&no, pos, chave, offset_registro, NULO);
            return bt_escrever_no(arq_indice, rrn_no, &no) ? 0 : -1;
        }

        // Nó cheio realiza split e sinaliza promoção para o pai
        return bt_dividir_no(arq_indice, cab, rrn_no, &no,
                             chave, offset_registro, NULO,
                             promo_chave, promo_offset, promo_rrn_dir);
    }

    // Nó interno desce para o filho correspondente à posição da chave
    int pc, po, pd; // chave, offset e RRN direito promovidos pelo filho
    int ret = bt_inserir_rec(arq_indice, cab, no.filhos[pos],
                             chave, offset_registro, &pc, &po, &pd);
    if (ret != 1)
        return ret; // 0 = sem promoção, -1 = erro

    // Filho promoveu uma chave tenta incorporá-la neste nó
    if (no.nroChaves < CHAVES_MAX)
    {
        bt_inserir_em_no(&no, pos, pc, po, pd);
        return bt_escrever_no(arq_indice, rrn_no, &no) ? 0 : -1;
    }

    // Este nó também está cheio propaga o split para cima
    return bt_dividir_no(arq_indice, cab, rrn_no, &no,
                         pc, po, pd,
                         promo_chave, promo_offset, promo_rrn_dir);
}

// Pega uma chave emprestada do irmão à direita ('dir') para resolver o underflow de 'esq'. O separador do pai (posição 'idx_sep') desce para o fim de 'esq' A primeira chave de 'dir' sobe para o pai. Se não for uma folha, o primeiro filho de 'dir' muda para o último filho de 'esq'.
void bt_redistribuir_da_direita(NO *pai, NO *esq, NO *dir, int idx_sep, bool folha)
{
    int k = esq->nroChaves;

    // Separador do pai desce para o fim do nó com underflow
    esq->chaves[k] = pai->chaves[idx_sep];
    esq->offsets[k] = pai->offsets[idx_sep];
    if (!folha)
        esq->filhos[k + 1] = dir->filhos[0]; // primeiro filho de 'dir' migra para 'esq'
    esq->nroChaves++;

    // Primeira chave de 'dir' sobe para o pai (substitui o separador que desceu)
    pai->chaves[idx_sep] = dir->chaves[0];
    pai->offsets[idx_sep] = dir->offsets[0];

    // Desloca as chaves de 'dir' à esquerda para preencher a lacuna
    for (int i = 0; i < dir->nroChaves - 1; i++)
    {
        dir->chaves[i] = dir->chaves[i + 1];
        dir->offsets[i] = dir->offsets[i + 1];
    }
    dir->chaves[dir->nroChaves - 1] = NULO;
    dir->offsets[dir->nroChaves - 1] = NULO;

    if (!folha)
    {
        // Desloca os filhos de 'dir' à esquerda, pois o primeiro filho migrou para 'esq'
        for (int i = 0; i < ORDEM - 1; i++)
            dir->filhos[i] = dir->filhos[i + 1];
        dir->filhos[ORDEM - 1] = NULO;
    }
    dir->nroChaves--;
}

// Pega uma chave emprestada do irmão à esquerda ('esq') para resolver o underflow de 'dir'. O separador do pai (posição 'idx_sep') desce para o início de 'dir' A última chave de 'esq' sobe para o pai. Se não for uma folha, o último filho de 'esq' muda para o primeiro filho de 'dir'.
void bt_redistribuir_da_esquerda(NO *pai, NO *esq, NO *dir, int idx_sep, bool folha)
{
    // Abre espaço no início de 'dir' deslocando seus elementos à direita
    for (int i = dir->nroChaves; i > 0; i--)
    {
        dir->chaves[i] = dir->chaves[i - 1];
        dir->offsets[i] = dir->offsets[i - 1];
    }
    if (!folha)
        for (int i = ORDEM - 1; i > 0; i--)
            dir->filhos[i] = dir->filhos[i - 1];

    // Separador do pai desce para o início de 'dir'
    dir->chaves[0] = pai->chaves[idx_sep];
    dir->offsets[0] = pai->offsets[idx_sep];
    if (!folha)
        dir->filhos[0] = esq->filhos[esq->nroChaves]; // último filho de 'esq' migra para 'dir'
    dir->nroChaves++;

    // Última chave de 'esq' sobe para o pai (substitui o separador que desceu)
    int ultimo = esq->nroChaves - 1;
    pai->chaves[idx_sep] = esq->chaves[ultimo];
    pai->offsets[idx_sep] = esq->offsets[ultimo];

    // Limpa a última posição de 'esq' que agora está vaga
    esq->chaves[ultimo] = NULO;
    esq->offsets[ultimo] = NULO;
    if (!folha)
        esq->filhos[ultimo + 1] = NULO;
    esq->nroChaves--;
}

// Concatena 'dir' no final de 'esq', intercalando o separador do pai (posição 'idx_sep'). Remove o separador e o ponteiro para 'dir' do pai após a fusão. 'dir' é sempre a página destruída (liberada pelo chamador após esta função).
void bt_concatenar(NO *pai, NO *esq, NO *dir, int idx_sep, bool folha)
{
    int k = esq->nroChaves;

    // Separador do pai desce para separar as duas metades dentro de 'esq'
    esq->chaves[k] = pai->chaves[idx_sep];
    esq->offsets[k] = pai->offsets[idx_sep];
    k++;

    // Copia todas as chaves e offsets de 'dir' para o final de 'esq'
    for (int i = 0; i < dir->nroChaves; i++)
    {
        esq->chaves[k + i] = dir->chaves[i];
        esq->offsets[k + i] = dir->offsets[i];
    }

    // Copia os filhos de 'dir' para 'esq' (apenas nós internos possuem filhos)
    if (!folha)
        for (int i = 0; i <= dir->nroChaves; i++)
            esq->filhos[k + i] = dir->filhos[i];

    esq->nroChaves = k + dir->nroChaves;

    // Remove o separador (idx_sep) do pai deslocando as chaves restantes à esquerda
    for (int i = idx_sep; i < pai->nroChaves - 1; i++)
    {
        pai->chaves[i] = pai->chaves[i + 1];
        pai->offsets[i] = pai->offsets[i + 1];
    }

    // Remove o ponteiro para 'dir' (idx_sep+1) do pai deslocando os filhos à esquerda
    for (int i = idx_sep + 1; i < pai->nroChaves; i++)
        pai->filhos[i] = pai->filhos[i + 1];

    // Limpa a última posição do pai que ficou duplicada após os deslocamentos
    pai->chaves[pai->nroChaves - 1] = NULO;
    pai->offsets[pai->nroChaves - 1] = NULO;
    pai->filhos[pai->nroChaves] = NULO;
    pai->nroChaves--;
}

// Verifica se o filho na posição 'idx_filho' de 'rrn_pai' está em underflow e o corrige. Ordem de tentativas conforme a especificação 1. Redistribuição com irmão à direita 2. Redistribuição com irmão à esquerda 3. Concatenação com irmão à esquerda (página direita   'filho'   é destruída) 4. Concatenação com irmão à direita  (página direita   'dir'    é destruída)
void bt_corrigir_underflow(FILE *arq_indice, CabecalhoBT *cab, int rrn_pai, int idx_filho)
{
    NO pai, filho;
    bt_ler_no(arq_indice, rrn_pai, &pai);

    int rrn_filho = pai.filhos[idx_filho];
    bt_ler_no(arq_indice, rrn_filho, &filho);

    if (filho.nroChaves >= CHAVES_MIN)
        return; // sem underflow: nada a fazer

    bool folha = (filho.filhos[0] == NULO);

    // 1. Redistribuição com o irmão à direita (tem chaves sobrando)
    if (idx_filho < pai.nroChaves)
    {
        int rrn_dir = pai.filhos[idx_filho + 1];
        NO dir;
        bt_ler_no(arq_indice, rrn_dir, &dir);
        if (dir.nroChaves > CHAVES_MIN)
        {
            bt_redistribuir_da_direita(&pai, &filho, &dir, idx_filho, folha);
            bt_escrever_no(arq_indice, rrn_pai, &pai);
            bt_escrever_no(arq_indice, rrn_filho, &filho);
            bt_escrever_no(arq_indice, rrn_dir, &dir);
            return;
        }
    }

    // 2. Redistribuição com o irmão à esquerda (tem chaves sobrando)
    if (idx_filho > 0)
    {
        int rrn_esq = pai.filhos[idx_filho - 1];
        NO esq;
        bt_ler_no(arq_indice, rrn_esq, &esq);
        if (esq.nroChaves > CHAVES_MIN)
        {
            bt_redistribuir_da_esquerda(&pai, &esq, &filho, idx_filho - 1, folha);
            bt_escrever_no(arq_indice, rrn_pai, &pai);
            bt_escrever_no(arq_indice, rrn_esq, &esq);
            bt_escrever_no(arq_indice, rrn_filho, &filho);
            return;
        }
    }

    // 3. Concatenação com o irmão à esquerda 'filho' (direito) é a página destruída
    if (idx_filho > 0)
    {
        int rrn_esq = pai.filhos[idx_filho - 1];
        NO esq;
        bt_ler_no(arq_indice, rrn_esq, &esq);

        bt_concatenar(&pai, &esq, &filho, idx_filho - 1, folha);

        bt_escrever_no(arq_indice, rrn_esq, &esq);
        bt_escrever_no(arq_indice, rrn_pai, &pai);
        bt_liberar_no(arq_indice, cab, rrn_filho); // 'filho' (direito) é destruído
        return;
    }

    // 4. Concatenação com o irmão à direita 'dir' (direito) é a página destruída
    {
        int rrn_dir = pai.filhos[idx_filho + 1];
        NO dir;
        bt_ler_no(arq_indice, rrn_dir, &dir);

        bt_concatenar(&pai, &filho, &dir, idx_filho, folha);

        bt_escrever_no(arq_indice, rrn_filho, &filho);
        bt_escrever_no(arq_indice, rrn_pai, &pai);
        bt_liberar_no(arq_indice, cab, rrn_dir); // 'dir' (direito) é destruído
    }
}

// Remove 'chave' recursivamente da subárvore enraizada em 'rrn_no'. Após remover em um filho, verifica e corrige underflow naquele filho. Retorna true se a chave existia e foi removida, false caso contrário.
bool bt_remover_rec(FILE *arq_indice, CabecalhoBT *cab, int rrn_no, int chave)
{
    NO no;
    if (!bt_ler_no(arq_indice, rrn_no, &no))
        return false;

    int pos = bt_obter_posicao(&no, chave);
    bool folha = (no.filhos[0] == NULO);
    bool achou = (pos < no.nroChaves && no.chaves[pos] == chave);

    if (achou)
    {
        if (folha)
        {
            // Caso 1 chave está em uma folha   remoção direta
            bt_remover_de_no(&no, pos);
            return bt_escrever_no(arq_indice, rrn_no, &no);
        }

        // Caso 2 chave está em um nó interno   substitui pela sucessora. A sucessora é a menor chave da subárvore do filho à direita da chave removida.
        int chave_suc, offset_suc;
        bt_obter_sucessor(arq_indice, no.filhos[pos + 1], &chave_suc, &offset_suc);

        no.chaves[pos] = chave_suc;
        no.offsets[pos] = offset_suc;
        if (!bt_escrever_no(arq_indice, rrn_no, &no))
            return false;

        // Remove a sucessora da folha onde ela estava originalmente
        if (!bt_remover_rec(arq_indice, cab, no.filhos[pos + 1], chave_suc))
            return false;

        // Corrige o possível underflow no filho de onde a sucessora foi retirada
        bt_corrigir_underflow(arq_indice, cab, rrn_no, pos + 1);
        return true;
    }

    if (folha)
        return false; // chegou na folha sem encontrar a chave: não existe na árvore

    // Caso 3 chave não está neste nó   desce para o filho correto
    if (!bt_remover_rec(arq_indice, cab, no.filhos[pos], chave))
        return false;

    // Corrige o possível underflow no filho em que ocorreu a remoção
    bt_corrigir_underflow(arq_indice, cab, rrn_no, pos);
    return true;
}

// Insere 'chave' e 'offset_registro' no índice árvore-B. Se a árvore estiver vazia, cria o primeiro nó (folha-raiz). Se a inserção recursiva gerar promoção, cria uma nova raiz com a chave promovida.
bool inserir_indice(FILE *arq_indice, CabecalhoBT *cab, int chave, int offset_registro)
{
    if (arq_indice == NULL || cab == NULL)
        return false;

    if (cab->noRaiz == NULO)
    {
        // Árvore vazia cria o primeiro nó, que é simultaneamente raiz e folha (tipoNo = -1)
        int rrn_raiz = bt_alocar_no(arq_indice, cab);
        NO raiz;
        bt_no_inicializar(&raiz);
        raiz.nroChaves = 1;
        raiz.chaves[0] = chave;
        raiz.offsets[0] = offset_registro;
        raiz.tipoNo = -1; // folha-raiz
        cab->noRaiz = rrn_raiz;
        return bt_escrever_no(arq_indice, rrn_raiz, &raiz);
    }

    int pc, po, pd; // chave, offset e RRN direito promovidos se a raiz sofrer split
    int ret = bt_inserir_rec(arq_indice, cab, cab->noRaiz,
                             chave, offset_registro, &pc, &po, &pd);
    if (ret < 0)
        return false;

    if (ret == 1)
    {
        // A raiz sofreu split cria nova raiz com a chave promovida e dois filhos
        int rrn_raiz_antiga = cab->noRaiz;
        int rrn_nova_raiz = bt_alocar_no(arq_indice, cab);

        // A raiz antiga perde o tipoNo=0 e vira folha ou intermediário
        NO raiz_antiga;
        bt_ler_no(arq_indice, rrn_raiz_antiga, &raiz_antiga);
        raiz_antiga.tipoNo = (raiz_antiga.filhos[0] == NULO) ? -1 : 1;
        bt_escrever_no(arq_indice, rrn_raiz_antiga, &raiz_antiga);

        // Nova raiz aponta para a raiz antiga (esquerda) e a nova página do split (direita)
        NO nova_raiz;
        bt_no_inicializar(&nova_raiz);
        nova_raiz.nroChaves = 1;
        nova_raiz.chaves[0] = pc;
        nova_raiz.offsets[0] = po;
        nova_raiz.filhos[0] = rrn_raiz_antiga;
        nova_raiz.filhos[1] = pd;
        nova_raiz.tipoNo = 0; // raiz
        cab->noRaiz = rrn_nova_raiz;

        if (!bt_escrever_no(arq_indice, rrn_nova_raiz, &nova_raiz))
            return false;
    }

    return true;
}

// Busca 'chave_busca' no índice percorrendo iterativamente da raiz até uma folha. Retorna o offset em bytes do registro no arquivo de dados, ou NULO se não encontrado.
int recuperar_registro_indice(FILE *arq_indice, CabecalhoBT *cab, int chave_busca)
{
    if (arq_indice == NULL || cab == NULL)
        return NULO;

    int rrn = cab->noRaiz;
    while (rrn != NULO)
    {
        NO no;
        if (!bt_ler_no(arq_indice, rrn, &no))
            return NULO;

        int pos = bt_obter_posicao(&no, chave_busca);

        if (pos < no.nroChaves && no.chaves[pos] == chave_busca)
            return no.offsets[pos]; // encontrou: retorna o offset no arquivo de dados

        if (no.filhos[0] == NULO)
            return NULO; // chegou na folha sem encontrar: chave não existe

        rrn = no.filhos[pos]; // desce para o filho adequado
    }

    return NULO;
}

// Remove 'chave' do índice, encolhendo a árvore se necessário. Após a remoção recursiva, verifica se a raiz ficou vazia Se era folha e ficou vazia a árvore inteira ficou vazia (noRaiz = NULO). Se era interna e ficou sem chaves o único filho restante vira a nova raiz. Retorna true se a chave foi encontrada e removida, false caso contrário.
bool remover_registro_indice(FILE *arq_indice, CabecalhoBT *cab, int chave)
{
    if (arq_indice == NULL || cab == NULL)
        return false;

    if (cab->noRaiz == NULO)
        return false; // árvore vazia: nada a remover

    if (!bt_remover_rec(arq_indice, cab, cab->noRaiz, chave))
        return false;

    NO raiz;
    bt_ler_no(arq_indice, cab->noRaiz, &raiz);

    // Verifica se a raiz ficou sem chaves após a remoção (diminuição da altura da árvore)
    if (raiz.nroChaves == 0)
    {
        int rrn_raiz_antiga = cab->noRaiz;

        if (raiz.filhos[0] == NULO)
        {
            // Se a raiz era folha e ficou vazia, logo a árvore inteira está vazia
            cab->noRaiz = NULO;
        }
        else
        {
            // Se a raiz interna ficou sem chaves, seu único filho restante vira a nova raiz
            cab->noRaiz = raiz.filhos[0];

            NO nova_raiz;
            bt_ler_no(arq_indice, cab->noRaiz, &nova_raiz);
            nova_raiz.tipoNo = (nova_raiz.filhos[0] == NULO) ? -1 : 0;
            bt_escrever_no(arq_indice, cab->noRaiz, &nova_raiz);
        }

        bt_liberar_no(arq_indice, cab, rrn_raiz_antiga);
    }

    return true;
}