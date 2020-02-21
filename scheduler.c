#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED

#include "scheduler.h"

#include <assert.h>
#include <curses.h>
#include <ucontext.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "util.h"

//#define ERR -1
// This is an upper limit on the number of tasks we can create.
#define MAX_TASKS 128

// This is the size of each task's stack memory
#define STACK_SIZE 65536

#define RUNNING 0
#define READY 1
#define WAIT 2
#define SLEEP 3
#define INPUT 4
#define EXIT 5
// This struct will hold the all the necessary information for each task
typedef struct task_info {
  // This field stores all the state required to switch back to this task
  ucontext_t context;
  
  // This field stores another context. This one is only used when the task
  // is exiting.
  ucontext_t exit_context;
  
  // TODO: Add fields here so you can:
  //   a. Keep track of this task's state.
  //   b. If the task is sleeping, when should it wake up?
  //   c. If the task is waiting for another task, which task is it waiting for?
  //   d. Was the task blocked waiting for user input? Once you successfully
  //      read input, you will need to save it here so it can be returned.
  int state;
  struct timeval wakeup_time;
  task_t task_wait_for;
  int input;

} task_info_t;

int current_task = 0; //< The handle of the currently-executing task
int num_tasks = 1;    //< The number of tasks created so far
task_info_t tasks[MAX_TASKS]; //< Information for every task

/**
 * Initialize the scheduler. Programs should call this before calling any other
 * functiosn in this file.
 */
void scheduler_init() {
  // TODO: Initialize the state of the scheduler 
  current_task=0;
  num_tasks=1;

  getcontext(&tasks[0].exit_context);
  tasks[0].exit_context.uc_stack.ss_sp = malloc(STACK_SIZE);
  tasks[0].exit_context.uc_stack.ss_size = STACK_SIZE;
  //makecontext(&tasks[0].exit_context, task_exit, 0);
  getcontext(&tasks[0].context);
  tasks[0].context.uc_stack.ss_sp = malloc(STACK_SIZE);
  tasks[0].context.uc_stack.ss_size = STACK_SIZE;
  //tasks[0].context.uc_link = &tasks[0].exit_context;

  tasks[0].state=RUNNING;
  tasks[0].task_wait_for=-1;
  tasks[0].input=-1;
  if(gettimeofday(&tasks[0].wakeup_time, NULL) == -1) {
    perror("gettimeofday");
    exit(2);
  }
}

int next_context(int current){
  int next=current;
  while(true){
    next=(next+1)%num_tasks;
    if(tasks[next].state==WAIT){
      if(tasks[tasks[next].task_wait_for].state==EXIT){
        tasks[next].state=READY;
        tasks[next].task_wait_for=-1;
        return next;
      }
    }
    else if(tasks[next].state==SLEEP){
      struct timeval now;
      if(gettimeofday(&now, NULL) == -1) {
        perror("gettimeofday");
        exit(2);
      }
      if((now.tv_sec*1000 + now.tv_usec/1000)>(tasks[next].wakeup_time.tv_sec*1000 + tasks[next].wakeup_time.tv_usec/1000)){
        tasks[next].state=READY;
        return next;
      }
    }
    else if(tasks[next].state==INPUT){
      int input=getch();
      if(input!=ERR){
        tasks[next].state=READY;
        tasks[next].input=input;
        return next;
      }
    }
    else if(tasks[next].state==READY){
      return next;
    }
  }
}
/**
 * This function will execute when a task's function returns. This allows you
 * to update scheduler states and start another task. This function is run
 * because of how the contexts are set up in the task_create function.
 */
void task_exit() {
  tasks[current_task].state=EXIT;
  if(current_task==0){
    return;
  }
  int prev=current_task;
  current_task=next_context(current_task);
  swapcontext(&tasks[prev].context,&tasks[current_task].context);
  // TODO: Handle the end of a task's execution here
}

/**
 * Create a new task and add it to the scheduler.
 *
 * \param handle  The handle for this task will be written to this location.
 * \param fn      The new task will run this function.
 */
void task_create(task_t* handle, task_fn_t fn) {
  // Claim an index for the new task
  int index = num_tasks;
  num_tasks++;
  
  // Set the task handle to this index, since task_t is just an int
  *handle = index;
 
  //initialize the info block
  tasks[index].state=READY;
  //tasks[index].task_wait_for=-1;
  //tasks[index].input=ERR;
  //if(gettimeofday(&tasks[index].wakeup_time, NULL) == -1) {
    //perror("gettimeofday");
    //exit(2);
  //}


  // We're going to make two contexts: one to run the task, and one that runs at the end of the task so we can clean up. Start with the second
  
  // First, duplicate the current context as a starting point
  getcontext(&tasks[index].exit_context);
  
  // Set up a stack for the exit context
  tasks[index].exit_context.uc_stack.ss_sp = malloc(STACK_SIZE);
  tasks[index].exit_context.uc_stack.ss_size = STACK_SIZE;
  
  // Set up a context to run when the task function returns. This should call task_exit.
  makecontext(&tasks[index].exit_context, task_exit, 0);
  
  // Now we start with the task's actual running context
  getcontext(&tasks[index].context);
  
  // Allocate a stack for the new task and add it to the context
  tasks[index].context.uc_stack.ss_sp = malloc(STACK_SIZE);
  tasks[index].context.uc_stack.ss_size = STACK_SIZE;
  
  // Now set the uc_link field, which sets things up so our task will go to the exit context when the task function finishes
  tasks[index].context.uc_link = &tasks[index].exit_context;
  
  // And finally, set up the context to execute the task function
  makecontext(&tasks[index].context, fn, 0);
}

/**
 * Wait for a task to finish. If the task has not yet finished, the scheduler should
 * suspend this task and wake it up later when the task specified by handle has exited.
 *
 * \param handle  This is the handle produced by task_create
 */
void task_wait(task_t handle) {
  // TODO: Block this task until the specified task has exited.

  if(tasks[handle].state==EXIT){
    return;
  }
  tasks[current_task].state=WAIT;
  tasks[current_task].task_wait_for=handle;

  int prev=current_task;
  current_task=next_context(current_task);
  tasks[current_task].state=RUNNING;
  swapcontext(&tasks[prev].context,&tasks[current_task].context);

}

/**
 * The currently-executing task should sleep for a specified time. If that time is larger
 * than zero, the scheduler should suspend this task and run a different task until at least
 * ms milliseconds have elapsed.
 * 
 * \param ms  The number of milliseconds the task should sleep.
 */
void task_sleep(size_t ms) {
  // TODO: Block this task until the requested time has elapsed.
  // Hint: Record the time the task should wake up instead of the time left for it to sleep. The bookkeeping is easier this way.
  struct timeval now;
  gettimeofday(&now,NULL);
  now.tv_usec=now.tv_usec+ms*1000;
  tasks[current_task].state=SLEEP;
  tasks[current_task].wakeup_time=now;

  int prev=current_task;
  current_task=next_context(current_task);
  tasks[current_task].state=RUNNING;
  swapcontext(&tasks[prev].context,&tasks[current_task].context);

}

/**
 * Read a character from user input. If no input is available, the task should
 * block until input becomes available. The scheduler should run a different
 * task while this task is blocked.
 *
 * \returns The read character code
 */
int task_readchar() {
  // TODO: Block this task until there is input available.
  // To check for input, call getch(). If it returns ERR, no input was available.
  // Otherwise, getch() will returns the character code that was read.
  int input=getch();
  if(input!=ERR){
    return input;
  }
  tasks[current_task].state=INPUT;
  int prev=current_task;
  current_task=next_context(current_task);
  tasks[current_task].state=RUNNING;
  swapcontext(&tasks[prev].context,&tasks[current_task].context);
  //printw("%d",tasks[current_task].input);
  return tasks[current_task].input;
}
