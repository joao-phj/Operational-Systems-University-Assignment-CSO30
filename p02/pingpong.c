#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "queue.h"
#include "datatypes.h"

#define DEBUG

#if defined(_WIN32) || (!defined(__unix__) && !defined(__unix) && (!defined(__APPLE__) || !defined(__MACH__)))
#warning Este codigo foi planejado para ambientes UNIX (LInux, *BSD, MacOS). A compilacao e execucao em outros ambientes e responsabilidade do usuario.
#endif

#define STACKSIZE 32768		/* tamanho de pilha das threads */
#define _XOPEN_SOURCE 600	/* para compilar no MacOS */

task_t *current, main_context;
int counter;


void pingpong_init () {
  /* desativa o buffer da saida padrao (stdout), usado pela função printf */
  setvbuf (stdout, 0, _IONBF, 0) ;
  main_context.next = NULL;
  main_context.prev = NULL;

  main_context.tid = 0;

  counter = 1;

  current = &main_context;

  #ifdef DEBUG
  printf ("init: inicializada a tarefa %d\n", current->tid) ;
  #endif
}

int task_create (task_t *task, void (*start_func)(void *), void *arg) {
  getcontext(&(task->context));

  task->tid = counter;
  counter++;
  task->next = NULL;
  task->prev = NULL;
  char *stack ;

  stack = malloc (STACKSIZE) ;
  if (stack)
  {
     task->context.uc_stack.ss_sp = stack ;
     task->context.uc_stack.ss_size = STACKSIZE;
     task->context.uc_stack.ss_flags = 0;
     task->context.uc_link = 0;
  }
  else
  {
     perror ("Erro na criacao da pilha: ");
     exit (1);
  }

  makecontext (&(task->context), (void (*)(void))start_func, 1, arg);
  #ifdef DEBUG
  printf ("task_create: criou tarefa %d\n", task->tid) ;
  #endif

  return task->tid;
}

void task_exit (int exitCode) {
  #ifdef DEBUG
  printf ("task_exit: saiu tarefa %d\n", current->tid) ;
  #endif
  swapcontext(&(current->context), &(main_context.context));
}

int task_switch (task_t *task) {
  task_t *prev = current;
  current = task;
  #ifdef DEBUG
  printf ("task_switch: trocou tarefa %d, pela tarefa %d\n", prev->tid, task->tid) ;
  #endif
  swapcontext(&(prev->context), &(task->context));

  return 0;
}

int task_id () {
  return current->tid;
}
