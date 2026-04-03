#ifndef _PILHA_H
#define _PILHA_H

  #include <stdbool.h>

  #define TAM 1005

  typedef struct pilha PILHA;
  typedef struct no NO;

  PILHA* pilha_criar(void);
  void pilha_apagar(PILHA** pilha);
  bool pilha_vazia(PILHA* pilha);
  bool pilha_cheia(PILHA* pilha);
  int pilha_tamanho(PILHA* pilha);
  int pilha_topo(PILHA* pilha);
  bool pilha_empilhar(PILHA* pilha, int rrn);
  int pilha_desempilhar(PILHA* pilha);

#endif 