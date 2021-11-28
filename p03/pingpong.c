#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "queue.h"
#include "datatypes.h"

//#define DEBUG

#if defined(_WIN32) || (!defined(__unix__) && !defined(__unix) && (!defined(__APPLE__) || !defined(__MACH__)))
#warning Este codigo foi planejado para ambientes UNIX (LInux, *BSD, MacOS). A compilacao e execucao em outros ambientes e responsabilidade do usuario.
#endif

#define STACKSIZE 32768		/* tamanho de pilha das threads */
#define _XOPEN_SOURCE 600	/* para compilar no MacOS */

void pingpong_init ();
void dispatcher_body ();
task_t* scheduler();
int task_create (task_t *task, void (*start_func)(void *), void *arg);
void task_yield ();
void task_exit (int exitCode);
void task_suspend (task_t *task, task_t **queue);
int task_switch (task_t *task);
int task_id ();
void task_suspend (task_t *task, task_t **queue);
void task_resume (task_t *task);

task_t *current, *prev, *prontas, *suspensas, *next, dispatcher, tarefaMain;
int counter, userTasks;


void pingpong_init () {
  /* desativa o buffer da saida padrao (stdout), usado pela função printf */
  setvbuf (stdout, 0, _IONBF, 0) ;
  tarefaMain.next = NULL;
  tarefaMain.prev = NULL;

  tarefaMain.tid = 0;

  userTasks = 0;
  counter = 1;

  current = &tarefaMain;

  dispatcher.next = NULL;
  dispatcher.prev = NULL;

  dispatcher.tid = 1;

  task_create(&dispatcher, dispatcher_body, NULL);

  #ifdef DEBUG
  printf ("init: inicializada a tarefa %d\n", current->tid) ;
  #endif
}

task_t* scheduler() {
  #ifdef DEBUG
  printf ("Scheduler: escolhida a tarefa %d\n", prontas->tid) ;
  #endif
  return prontas;
}

void dispatcher_body () {
  while (userTasks > 0) {
    next = scheduler() ; // scheduler é uma função
    if (next) {
      #ifdef DEBUG
      printf ("Dispatcher: trocando para tarefa %d\n", next->tid) ;
      #endif
      task_switch (next) ; // transfere controle para a tarefa "next"
      #ifdef DEBUG
      printf ("Dispatcher: tarefa %d retirada da fila\n", prev->tid) ;
      #endif
      current = &dispatcher;
      queue_remove(&prontas, prev);
      if(!prev->done) {
        #ifdef DEBUG
        printf ("Dispatcher: tarefa %d adicionada ao final da fila\n", prev->tid) ;
        #endif
        queue_append(&prontas, prev);
      }
    }
  }
  task_exit(0) ; // encerra a tarefa dispatcher
}

int task_create (task_t *task, void (*start_func)(void *), void *arg) {
  getcontext(&(task->context));

  task->tid = counter;
  counter++;
  task->next = NULL;
  task->prev = NULL;
  task->done = false;
  task->pronta = true;
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

  if(task->tid != 1) {
    userTasks++;
    queue_append(&prontas, task);
    printf("userTasks: %d\n", userTasks);
  }

  return task->tid;
}

void task_yield () {
  #ifdef DEBUG
  printf ("task_yield: yield tarefa %d\n", current->tid) ;
  #endif
  prev = current;
  task_switch(&dispatcher);
}

void task_exit (int exitCode) {
  #ifdef DEBUG
  printf ("task_exit: saiu tarefa %d\n", current->tid) ;
  #endif
  if(current->tid != 1) {
    userTasks--;
    current->done = true;
    current->pronta = false;
    prev = current;
    swapcontext(&(current->context), &(dispatcher.context));
  }
  else{
    swapcontext(&(current->context), &(tarefaMain.context));
  }
}

void task_suspend (task_t *task, task_t **queue) {
  task->pronta = false;

  queue_remove(queue, task);
  queue_append(&suspensas, task);
}

void task_resume (task_t *task) {
  if(task->next != NULL && task->prev != NULL) {
    task->pronta = true;
    queue_append(&prontas, task);
  }
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
