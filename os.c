// os.c
// Runs on LM4F120/TM4C123/MSP432
// Lab 3 starter file.
// Daniel Valvano
// March 24, 2016

#include <stdint.h>
#include "os.h"
#include "CortexM.h"
#include "BSP.h"

// function definitions in osasm.s
void StartOS(void);

#define NUMTHREADS  6        // maximum number of threads
#define NUMPERIODIC 2        // maximum number of periodic threads
#define STACKSIZE   100      // number of 32-bit words in stack per thread
struct tcb{
  int32_t *sp;       // pointer to stack (valid for threads not running
  struct tcb *next;  // linked-list pointer
  
	int32_t *block; // nonzero if blocked on this semaphore. Address of the semaphore is used
  int32_t sleep; // nonzero if this thread is sleeping. In ms. Decrement counter on PTTT call
	int32_t ID;
//*FILL THIS IN**** DONE
};
typedef struct tcb tcbType;

tcbType tcbs[NUMTHREADS];
tcbType *RunPt;
int32_t Stacks[NUMTHREADS][STACKSIZE];


void static runperiodicevents(void);

// ******** OS_Init ************
// Initialize operating system, disable interrupts
// Initialize OS controlled I/O: periodic interrupt, bus clock as fast as possible
// Initialize OS global variables
// Inputs:  none
// Outputs: none
void OS_Init(void){
  DisableInterrupts();
  BSP_Clock_InitFastest();// set processor clock to fastest speed
  // perform any initializations needed
	
}

void SetInitialStack(int i){

  // **Same as Lab 2****
	tcbs[i].sp = &Stacks[i][STACKSIZE-16]; // thread stack pointer
  Stacks[i][STACKSIZE-1] = 0x01000000;   // thumb bit
  Stacks[i][STACKSIZE-3] = 0x14141414;   // R14
  Stacks[i][STACKSIZE-4] = 0x12121212;   // R12
  Stacks[i][STACKSIZE-5] = 0x03030303;   // R3
  Stacks[i][STACKSIZE-6] = 0x02020202;   // R2
  Stacks[i][STACKSIZE-7] = 0x01010101;   // R1
  Stacks[i][STACKSIZE-8] = 0x00000000;   // R0
  Stacks[i][STACKSIZE-9] = 0x11111111;   // R11
  Stacks[i][STACKSIZE-10] = 0x10101010;  // R10
  Stacks[i][STACKSIZE-11] = 0x09090909;  // R9
  Stacks[i][STACKSIZE-12] = 0x08080808;  // R8
  Stacks[i][STACKSIZE-13] = 0x07070707;  // R7
  Stacks[i][STACKSIZE-14] = 0x06060606;  // R6
  Stacks[i][STACKSIZE-15] = 0x05050505;  // R5
  Stacks[i][STACKSIZE-16] = 0x04040404;  // R4
}

//******** OS_AddThreads ***************
// Add six main threads to the scheduler
// Inputs: function pointers to six void/void main threads
// Outputs: 1 if successful, 0 if this thread can not be added
// This function will only be called once, after OS_Init and before OS_Launch
int OS_AddThreads(void(*thread0)(void),
                  void(*thread1)(void),
                  void(*thread2)(void),
                  void(*thread3)(void),
                  void(*thread4)(void),
                  void(*thread5)(void)){
  // **similar to Lab 2. initialize as not blocked, not sleeping****
	int32_t status;
	status = StartCritical(); // Critical code, not to be interrupted starts here ... 									
	tcbs[0].next = &tcbs[1]; // list element 0 pointer to next points to 1
	tcbs[1].next = &tcbs[2]; // list element 1 pointer to next points to 2
	tcbs[2].next = &tcbs[3]; // list element 2 pointer to next points to 3
	tcbs[3].next = &tcbs[4]; // list element 3 pointer to next points to 4
	tcbs[4].next = &tcbs[5]; // list element 4 pointer to next points to 5
	tcbs[5].next = &tcbs[0]; // list element 5 pointer to next points to 0	
										
	SetInitialStack(0); Stacks[0][STACKSIZE-2] = (int32_t)(thread0); // init. PC to start addr.
	SetInitialStack(1); Stacks[1][STACKSIZE-2] = (int32_t)(thread1); // init. PC to start addr.
	SetInitialStack(2); Stacks[2][STACKSIZE-2] = (int32_t)(thread2); // init. PC to start addr.
	SetInitialStack(3); Stacks[3][STACKSIZE-2] = (int32_t)(thread3); // init. PC to start addr.
	SetInitialStack(4); Stacks[4][STACKSIZE-2] = (int32_t)(thread4); // init. PC to start addr.
	SetInitialStack(5); Stacks[5][STACKSIZE-2] = (int32_t)(thread5); // init. PC to start addr.
													
	//init as non blocked, not sleeping
	
	tcbs[0].block = 0;	tcbs[0].sleep = 0; tcbs[0].ID = 0;
	tcbs[1].block = 0;	tcbs[1].sleep = 0; tcbs[1].ID = 1;
	tcbs[2].block = 0;	tcbs[2].sleep = 0; tcbs[2].ID = 2;
	tcbs[3].block = 0;	tcbs[3].sleep = 0; tcbs[3].ID = 3;
	tcbs[4].block = 0;	tcbs[4].sleep = 0; tcbs[4].ID = 4;
	tcbs[5].block = 0;	tcbs[5].sleep = 0; tcbs[5].ID = 5;
	RunPt = &tcbs[0];									
	
	EndCritical(status); // Critical code, not to be interrupted ends here ... 

  return 1;               // successful
}

//******** OS_AddPeriodicEventThread ***************
// Add one background periodic event thread
// Typically this function receives the highest priority
// Inputs: pointer to a void/void event thread function
//         period given in units of OS_Launch (Lab 3 this will be msec)
// Outputs: 1 if successful, 0 if this thread cannot be added
// It is assumed that the event threads will run to completion and return
// It is assumed the time to run these event threads is short compared to 1 msec
// These threads cannot spin, block, loop, sleep, or kill
// These threads can call OS_Signal
// In Lab 3 this will be called exactly twice
void (*pttt0Pt)(void);
void (*pttt1Pt)(void);
uint32_t pttt0_period = 0;
uint32_t pttt1_period = 0;

int OS_AddPeriodicEventThread(void(*thread)(void), uint32_t period){
// ****IMPLEMENT THIS**** TODO. Similar to Lab2
	if(pttt0_period == 0)
	{
		pttt0_period = period;
		pttt0Pt = thread;
	}
	else 
	{
		pttt1_period = period;
		pttt1Pt = thread;
	}
	
  return 1;
}

unsigned long millis = 0;
void static runperiodicevents(void){
// ****IMPLEMENT THIS**** TODO. Call T0, T1 with certain frequency (similar to scheduler in Lab2)
// **RUN PERIODIC THREADS, DECREMENT SLEEP COUNTERS
	DisableInterrupts();
	
	millis = (millis + 1)%100;
	if(millis%pttt0_period == 0) pttt0Pt();
	if(millis%pttt1_period == 0) pttt1Pt();	

	tcbType *pt;
	pt = RunPt; // search for a
	do
	{
		if(pt->sleep > 0) pt->sleep--;
		pt = pt->next;
	}while(pt != RunPt);
	// marker - thread
	// ready for sched.
	EnableInterrupts();
}

//******** OS_Launch ***************
// Start the scheduler, enable interrupts
// Inputs: number of clock cycles for each time slice
// Outputs: none (does not return)
// Errors: theTimeSlice must be less than 16,777,216
void OS_Launch(uint32_t theTimeSlice){
  STCTRL = 0;                  // disable SysTick during setup
	
  STCURRENT = 0;               // any write to current clears it
  SYSPRI3 =(SYSPRI3&0x00FFFFFF)|0xE0000000; // priority 7
  STRELOAD = theTimeSlice - 1; // reload value
	
	BSP_PeriodicTask_Init(&runperiodicevents, 1000, 0); //initialize hardware time and start it
	
  STCTRL = 0x00000007;         // enable, core clock and interrupt arm
	
  StartOS();                   // start on the first task
}


void Scheduler(void){ // every time slice
// ROUND ROBIN, skip blocked and sleeping threads
	do
	{
		RunPt = RunPt->next;
	}while(!(RunPt->block == 0 && RunPt->sleep == 0));
	//repeat until both block = 0 or sleep = 0
}

//******** OS_Suspend ***************
// Called by main thread to cooperatively suspend operation
// Inputs: none
// Outputs: none
// Will be run again depending on sleep/block status
void OS_Suspend(void){ //DONE
  STCURRENT = 0;        // any write to current clears it
  INTCTRL = 0x04000000; // trigger SysTick
	EnableInterrupts();
	//
// next thread gets a full time slice
}

// ******** OS_Sleep ************
// place this thread into a dormant state
// input:  number of msec to sleep
// output: none
// OS_Sleep(0) implements cooperative multitasking
void OS_Sleep(uint32_t sleepTime){
// set sleep parameter in TCB
// suspend, stops running
	RunPt->sleep = sleepTime;
	OS_Suspend();
}

// ******** OS_InitSemaphore ************
// Initialize counting semaphore
// Inputs:  pointer to a semaphore
//          initial value of semaphore
// Outputs: none
void OS_InitSemaphore(int32_t *semaPt, int32_t value){ //DONE
//***IMPLEMENT THIS***
	int32_t status;
	status = StartCritical(); // Critical code, not to be interrupted starts here ...
	*semaPt = value;
	EndCritical(status); // Critical code, not to be interrupted ends here ... 

}

// ******** OS_Wait ************
// Decrement semaphore and block if less than zero
// Lab2 spinlock (does not suspend while spinning)
// Lab3 block if less than zero
// Inputs:  pointer to a counting semaphore
// Outputs: none
void OS_Wait(int32_t *semaPt){ //DONE
//***IMPLEMENT THIS***
	DisableInterrupts();
	
	(*semaPt)--;
	
	if((*semaPt) < 0) //not available anymore
	{
		RunPt->block = semaPt;
		//Enable interrupts after the trigger in OS_Suspend
		OS_Suspend();
	}
	
	EnableInterrupts();
}

// ******** OS_Signal ************
// Increment semaphore
// Lab2 spinlock
// Lab3 wakeup blocked thread if appropriate
// Inputs:  pointer to a counting semaphore
// Outputs: none
void OS_Signal(int32_t *semaPt){ //DONE
//***IMPLEMENT THIS***
	tcbType *pt;
	DisableInterrupts();
	(*semaPt)++; //released
	
	if((*semaPt) <= 0) //there are some blocked threads
	{
		pt = RunPt->next; // search for a
		// thread blocked on
		// this semaphore
		while(pt->block != semaPt)
		{
			pt = pt->next;
		}
		pt->block = 0; // remove blocked
		// marker - thread
		// ready for sched.
	}
	EnableInterrupts();
}

#define FSIZE 10    // can be any size
uint32_t PutI;      // index of where to put next
uint32_t GetI;      // index of where to get next
uint32_t Fifo[FSIZE];
int32_t CurrentSize;// 0 means FIFO empty, FSIZE means full
uint32_t LostData;  // number of lost pieces of data

// ******** OS_FIFO_Init ************
// Initialize FIFO.  
// One event thread producer, one main thread consumer
// Inputs:  none
// Outputs: none
void OS_FIFO_Init(void){ //DONE
//***IMPLEMENT THIS***
	PutI = 0;
	GetI = 0;
	OS_InitSemaphore(&CurrentSize, 0);
	LostData = 0;
}

// ******** OS_FIFO_Put ************
// Put an entry in the FIFO.  
// Exactly one event thread puts,
// do not block or spin if full
// Inputs:  data to be stored
// Outputs: 0 if successful, -1 if the FIFO is full
int OS_FIFO_Put(uint32_t data){ //DONE
//***IMPLEMENT THIS*** TODO
	if(CurrentSize == FSIZE)
	{
		LostData++; // error: full
		return -1; // exit function here
	} 
	else
	{ // without writing to FIFO
		Fifo[PutI] = data; // Put
		PutI = PutI == FSIZE-1 ? 0 : PutI + 1;  //if PutI reached max, then reset, otherwise increment //(PutI+1)%FIFOSIZE;
		OS_Signal(&CurrentSize);
		return 0; // success
	}
}

// ******** OS_FIFO_Get ************
// Get an entry from the FIFO.   
// Exactly one main thread get,
// do block if empty
// Inputs:  none
// Outputs: data retrieved
uint32_t OS_FIFO_Get(void){uint32_t data; //DONE
//***IMPLEMENT THIS*** TODO
	OS_Wait(&CurrentSize); // block if empty
	data = Fifo[GetI]; // get
	GetI = GetI == FSIZE-1 ? 0 : GetI + 1;//GetI = (GetI+1)%FIFOSIZE; // place to get next
	return data;
}
