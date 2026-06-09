# Resumo do Projeto: Trabalho Prático 1 - Organização de Arquivos

## 1. Objetivo Geral
O objetivo central do trabalho é implementar um **índice Árvore-B** para indexar arquivos de dados binários. Esse índice será utilizado para realizar operações de recuperação, inserção e remoção lógica de dados, com reaproveitamento dinâmico de espaço (uso de pilha). O projeto deve ser desenvolvido em linguagem **C** por grupos de até **2 alunos**.

## 2. Estrutura e Regras da Árvore-B
* **Ordem:** $m = 4$ (cada nó comporta no máximo 3 chaves e 4 descendentes).
* **Chave de busca indexada:** `codEstacao`.
* **Simplificações em relação à teoria:** O tamanho do nó não precisa corresponder obrigatoriamente a uma página de disco real. 
* **Algoritmo de Inserção:** Deve implementar a operação de particionamento (*split*) promovendo a chave mediana (ou a primeira chave do novo nó, em divisões pares) de forma a balancear o nó. A página nova é criada sempre à direita. *Não* se aplica redistribuição na inserção.
* **Algoritmo de Remoção:** Implementa substituição pela sucessora imediata se for nó interno. Em caso de *underflow*, tenta-se primeiro a redistribuição (com a direita, depois esquerda). Se falhar, faz-se a concatenação (juntando no nó da esquerda).

## 3. Estruturação dos Arquivos Binários
### Registro de Cabeçalho (17 bytes fixos)
Guarda as meta-informações do índice da Árvore-B.
* `status` (1 byte): '0' (inconsistente) ou '1' (consistente).
* `noRaiz` (4 bytes): RRN do nó raiz (ou -1 se vazio).
* `topo` (4 bytes): RRN do topo da pilha de registros removidos (ou -1).
* `proxRRN` (4 bytes): Próximo RRN disponível para criação de novo nó.
* `nroNos` (4 bytes): Contagem total de nós na árvore.

### Registros de Dados / Nós da Árvore (53 bytes fixos)
A estrutura interna dos nós da árvore que guardam as chaves e ponteiros.
* `removido` (1 byte): '1' (removido) ou '0' (ativo).
* `próximo` (4 bytes): RRN do próximo nó na pilha de removidos.
* `tipoNo` (4 bytes): -1 (folha), 0 (raiz) ou 1 (intermediário).
* `nroChaves` (4 bytes): Quantidade de chaves presentes no nó.
* **Campos de Chaves e Ponteiros:** Intercalam-se ponteiros de subárvore ($P_x$), valores da chave ($C_x$) e ponteiros do arquivo de dados ($P_{Rx}$). Valores não utilizados devem ser preenchidos com `-1`.

## 4. Funcionalidades a Serem Implementadas
O programa deve ler um identificador da entrada padrão e executar uma das seguintes operações:

* **Funcionalidade [7] - Criação de Índice (CREATE INDEX):**
  Lê um arquivo de dados existente, ignora os registros logicamente removidos e cria um arquivo de índice Árvore-B inserindo as chaves (`codEstacao`) uma a uma. Retorna o conteúdo binário na tela usando a função `binarioNaTela()`.

* **Funcionalidade [8] - Recuperação de Dados (SELECT ... WHERE):**
  Busca registros no arquivo de dados baseado em critérios específicos. **Regra de ouro:** Se o campo de busca for a chave primária (`codEstacao`), a busca *obrigatoriamente* deve ser feita pelo índice Árvore-B.

* **Funcionalidade [9] - Inserção de Registros (INSERT INTO):**
  Insere novos registros no arquivo de dados. Exige a utilização de reaproveitamento dinâmico de espaço, alocando os novos dados no topo da pilha de removidos se houver espaço livre. Lixo excedente no espaço reaproveitado deve ser preenchido com `$`. A chave também deve ser adicionada à Árvore-B.

* **Funcionalidade [10] - Remoção Lógica (DELETE FROM):**
  Busca registros por um critério e os remove logicamente. O registro de dados e o nó do índice devem ser inseridos em suas respectivas pilhas de removidos (atualizando os campos `topo` e `próximo`) para reaproveitamento futuro.

## 5. Regras, Restrições e Entregáveis
* **Modo de Arquivo:** Leitura/Escrita obrigatoriamente no modo binário; a escrita deve ser feita campo a campo (não registro a registro).
* **Modularização e Código limpo:** O projeto exige modularização (separação em funções/procedimentos) e documentação interna rigorosa do código-fonte.
* **Integridade Acadêmica:** É expressamente proibido o uso de ferramentas de Inteligência Artificial para a implementação. Plágio zera as notas dos envolvidos.
* **Material a ser entregue:**
  1. **[run.codes]:** Arquivo `.zip` com código fonte em C e um arquivo `Makefile` com as diretivas `all` (que gera `programaTrab`) e `run`.
  2. **[e-disciplinas]:** Um vídeo com participação de todos os membros, de no máximo 7 minutos (com webcam ativada e tempo de fala balanceado), explicando o funcionamento e a implementação do código.
