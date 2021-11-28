#if defined(_WIN32) || (!defined(__unix__) && !defined(__unix) && (!defined(__APPLE__) || !defined(__MACH__)))
#warning Este código foi planejado para ambientes UNIX (LInux, *BSD, MacOS). A compilação e execução em outros ambientes é responsabilidade do usuário.
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "queue.h"

void queue_append (queue_t **queue, queue_t *elem) {
  queue_t *aux;
  aux = *queue;

  if(elem->next != NULL || elem->prev != NULL) { return; }

  if(aux == NULL) {
    elem->next = elem;
    elem->prev = elem;
    *queue = elem;
    return;
  }
  while(aux->next != *queue) {
    aux = aux->next;
  }
  (*queue)->prev = elem;
  aux->next = elem;
  elem->prev = aux;
  elem->next = *queue;
}

int queue_size (queue_t *queue) {
    int tam = 1;
    queue_t *aux = queue;

    if(aux == NULL) {
      return 0;
    }

    while(aux->next != queue) {
      tam++;
      aux = aux->next;
    }

    return tam;
}

queue_t *queue_remove (queue_t **queue, queue_t *elem) {
    if(elem->prev == NULL && elem->next == NULL) { return NULL; }

    queue_t *aux;
    aux = *queue;
    int ver = 0;
    if(*queue != NULL) {
      while(aux->next != *queue) {
        if(aux == elem) {
          ver = 1;
          break;
        }
        aux = aux->next;
      }
      if(aux == elem) {
        ver = 1;
      }
      if(ver == 0) {
        return NULL;
      }
    }

    if(*queue == elem){
      if(elem->next != elem) {
        (elem->next)->prev = (*queue)->prev;
        (elem->prev)->next = (*queue)->next;
        *queue = elem->next;
      }
      else {
        *queue = NULL;
      }
    }
    else {
      (elem->next)->prev = elem->prev;
      (elem->prev)->next = elem->next;
    }
    elem->prev = NULL;
    elem->next = NULL;
    return elem;
}

void queue_print (char *name, queue_t *queue, void print_elem (void*) ) {
  queue_t *aux;
  aux = queue;
  printf("%s: [", name);
  if(aux == NULL) {
    printf("]\n");
    return;
  }
  while(aux->next != queue) {
    print_elem(aux);
    printf(" ");
    aux = aux->next;
  }
  print_elem(aux);
  printf("]\n");
}
