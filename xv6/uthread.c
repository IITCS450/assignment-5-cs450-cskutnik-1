#include "types.h"
#include "stat.h"
#include "user.h"
#include "uthread.h"

#define MAX_THREADS 8
#define STACK_SIZE   4096

#define FREE 	  0
#define RUNNABLE  1
#define RUNNING   2
#define ZOMBIE    3

struct context{
	uint edi;
	uint esi;
	uint ebx;
	uint ebp;
};

struct thread{
	tid_t 		tid;
	int 		state;
	struct context	*ctx;
	char		stack[STACK_SIZE];
};

static struct thread threads[MAX_THREADS];
static int current_tid;

static void __attribute__((noinline))
thread_stub(void){
	uint ebp_val;
	asm volatile("movl %%ebp, %0" : "=r"(ebp_val));
	void (*fn)(void*) = *(void(**)(void*))(ebp_val + 8);
	void *arg = *(void**)(ebp_val + 12);
	
	fn(arg);
	threads[current_tid].state = ZOMBIE;
	thread_yield();
	exit();
}

void thread_init(void){
	int i;
	for(i = 0;i < MAX_THREADS;i++){
		threads[i].tid = i;
		threads[i].state = FREE;
		threads[i].ctx = 0;
	}  
	threads[0].state = RUNNING;
	current_tid = 0;
}

tid_t thread_create(void (*fn)(void*), void *arg){ 
	int i;
	struct thread *t = 0;
	for(i = 1;i < MAX_THREADS;i++){
		if(threads[i].state == FREE){
			t = &threads[i];
			break;
		}
	}
	if(t == 0) return -1;

	uint *sp = (uint*)(t->stack + STACK_SIZE);
	*--sp = (uint)arg;
	*--sp = (uint)fn;
	*--sp = 0xdeadbeef;
	*--sp = (uint)thread_stub;
	*--sp = 0;
	*--sp = 0;
	*--sp = 0;
	*--sp = 0;

	t->ctx = (struct context *)sp;
	t->state = RUNNABLE;
	return t->tid;
}

static int pick_next(void){
	int i;
	for(i = 1;i <= MAX_THREADS;i++){
		int idx = (current_tid + i) % MAX_THREADS;
		if(threads[idx].state == RUNNABLE) return idx;
	}
	return -1;
}
	
void thread_yield(void){
	int old = current_tid;
	int next = pick_next();
	if(next == -1) return;
	if(threads[old].state == RUNNING) threads[old].state = RUNNABLE;

	threads[next].state = RUNNING;
	current_tid = next;
	uswtch(&threads[old].ctx, threads[next].ctx);	
}
int thread_join(tid_t tid){
	if(tid <= 0 || tid >= MAX_THREADS) return -1;

	while(threads[tid].state != ZOMBIE) thread_yield();

	threads[tid].state = FREE;
	threads[tid].ctx = 0;
	return 0;
}
