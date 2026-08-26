/*
 * scheduler.c
 *
 *  Created on: Aug 26, 2026
 *      Author: dell
 */

#include"main.h"


/*Global data*/
static uint8_t 	current_task = 1; //task_1 is running

uint32_t g_tick_count = 0;

TCB_t user_tasks[MAX_TASK];



/*
 * Scheduler functions definitions
 */
void init_systick_timer(uint32_t tick_hz)
{

	uint32_t *pSCSR = (uint32_t*)0xE000E010;
	uint32_t *pSRVR = (uint32_t*)0xE000E014;
	uint32_t count_value = (SYSTICK_TIM_CLK / tick_hz) - 1; //N-1 according to datasheet

	//clear the value of SVR register
	*pSRVR &= ~(0x00FFFFFFFF);

	//load the value into SVR
	*pSRVR |= count_value;

	//do some settings
	*pSCSR |= (1 << 1);	//enable SysTick exception request
	*pSCSR |= (1 << 2);	//setting the clock source as the processor clock

	//enable the systick
	*pSCSR |= (1 << 0);
}


__attribute__((naked)) void init_scheduler_stack(uint32_t sched_top_of_stack)
{
	__asm volatile("MSR MSP, %0": : "r" (sched_top_of_stack) : );
	__asm volatile("BX LR");
	/*
	 * We use naked function because is standard C function with changed SP(MSP)
	 * processor will not be able to read prologue and epilogue of C function
	 * in stack to return back in Main and will fall in fault.
	 */
}


void init_task_stack(void)
{
	/*Initializing tasks status*/
	for(uint8_t i = 0 ; i < MAX_TASK ; i++)
	{
		user_tasks[i].current_state = TASK_READY_STATE;
	}

	user_tasks[0].psp_value = IDLE_STACK_START;
	user_tasks[1].psp_value = T1_STACK_START;
	user_tasks[2].psp_value = T2_STACK_START;
	user_tasks[3].psp_value = T3_STACK_START;
	user_tasks[4].psp_value = T4_STACK_START;

	user_tasks[0].task_handler = idle_task;
	user_tasks[1].task_handler = task1_handler;
	user_tasks[2].task_handler = task2_handler;
	user_tasks[3].task_handler = task3_handler;
	user_tasks[4].task_handler = task4_handler;


	/*Initializing tasks stack*/
	uint32_t *pPSP;

	for(int i = 0 ; i < MAX_TASK ; i++)
	{
		pPSP = (uint32_t*)user_tasks[i].psp_value;

		pPSP--; //Full-Descending stack model(SP points to full memory address)
		*pPSP = DUMMY_XPSR; //0x01000000

		pPSP--;
		*pPSP = (uint32_t)user_tasks[i].task_handler; //PC(return address)

		pPSP--;
		*pPSP = EXC_RETURN; //LR 0xFFFFFFFD return to thread mode with PSP

		for(int j = 0 ; j < GENERAL_REG_NUM ; j++) 	//R0-R11 = 0
		{
			pPSP--;
			*pPSP = 0;
		}

		user_tasks[i].psp_value = (uint32_t) pPSP;
	}
}


void enable_processor_faults(void)
{
	uint32_t *pSHCSR = (uint32_t*)0xE000ED24;

	*pSHCSR |= (1 << 16); //mem manage enable
	*pSHCSR |= (1 << 17); //bus fault enable
	*pSHCSR |= (1 << 18); //usage fault enable
}


uint32_t get_psp_value(void)
{
	return user_tasks[current_task].psp_value;
}


void save_psp_value(uint32_t current_psp_value)
{
	user_tasks[current_task].psp_value = current_psp_value;
}


void update_next_task(void)
{
	int state = TASK_BLOCKED_STATE;

	for(int i = 0 ; i < MAX_TASK ; i++)
	{
		current_task++;
		current_task = current_task % MAX_TASK;
		state = user_tasks[current_task].current_state;
		if((state == TASK_READY_STATE) && (current_task != 0))
			break;

	}

	if(state != TASK_READY_STATE)
		current_task = 0;
}


void update_global_tick_count(void)
{
	g_tick_count++;
}


__attribute__((naked)) void switch_sp_to_psp(void)
{
	//1. Initialize the PSP with TASK_1 stack start

	//get the value of psp of current task
	__asm volatile("PUSH {LR}");		//preserve LR which connects back to main
	__asm volatile("BL get_psp_value");
	__asm volatile("MSR PSP, R0");		//initialize PSP
	__asm volatile("POP {LR}");		//pops back LR value

	//2. Change SP to PSP using CONTROL reg
	__asm volatile("MOV R0, #0x02");
	__asm volatile("MSR CONTROL, R0");
	__asm volatile("BX LR"); //go back to main(LR value will be copied into PC)
}


void schedule(void)
{
	uint32_t *pICSR = (uint32_t*)0xE000ED04;
	//pend the pendSV exception
	*pICSR |= (1 << 28);
}




void task_delay(uint32_t tick_count)
{
	//disable interrupt
	INTERRUPT_DISABLE();

	if(current_task)
	{
		user_tasks[current_task].block_count = g_tick_count + tick_count;
		user_tasks[current_task].current_state = TASK_BLOCKED_STATE;
		schedule();
	}

	//enable interrupt
	INTERRUPT_ENABLE();
}


__attribute((naked)) void PendSV_Handler(void)
{
	/*Save the context of current task*/
	//1. Get the current running task`s PSP value
	__asm volatile("MRS R0, PSP");

	//2. Using that PSP value to store SF2(R4 to R11)
	__asm volatile("STMDB R0!, {R4-R11}");    //store multiple registers, decrement before

	__asm volatile("PUSH {LR}");	//save the context of LR

	//3. Save the current value of PSP
	__asm volatile("BL save_psp_value");


	/*Retrieve the context of next task*/
	//1. Decide next task to run
	__asm volatile("BL update_next_task");

	//2. Get its past PSP value
	__asm volatile("BL get_psp_value");

	//3. Using that PSP value retrieve SF2(R4 to R11)
	__asm volatile("LDMIA R0!, {R4-R11}");		//Load Multiple registers, increment after

	//4. Update PSP and exit
	__asm volatile("MSR PSP, R0");

	/* Manually exiting the exception*/
	__asm volatile("POP {LR}");		//retrieve the context of LR
	__asm volatile("BX LR");
}


void unblock_tasks(void)
{
	for(int i = 1 ; i < MAX_TASK ; i++)		//(i = 1), because idle task state is always equal to zero
	{
		if(user_tasks[i].current_state != TASK_READY_STATE)
		{
			if(user_tasks[i].block_count == g_tick_count)
			{
				user_tasks[i].current_state = TASK_READY_STATE;
			}
		}
	}
}


void SysTick_Handler(void)
{
	update_global_tick_count();
	unblock_tasks();
	schedule();
}


void HardFault_Handler(void)
{
	printf("Exception: HardFault\n");
	while(1);
}


void MemManage_Handler(void)
{
	printf("Exception: MemManage\n");
	while(1);
}


void BusFault_Handler(void)
{
	printf("Exception: BusFault\n");
	while(1);
}


void UsageFault_Handler(void)
{
	printf("Exception: UsageFault\n");
	while(1);
}
