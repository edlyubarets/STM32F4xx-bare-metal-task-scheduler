/*
 * main.h
 *
 *  Created on: Jun 17, 2026
 *      Author: dell
 */

#ifndef MAIN_H_
#define MAIN_H_

#include<stdint.h>
#include<stdio.h>

#define MAX_TASK 5

/* stack memory calculations START */
#define SIZE_TASK_STACK 	1024U
#define SIZE_SCHED_STACK 	1024U

#define SRAM_START 			0x20000000U
#define SIZE_SRAM			((128) * (1024))
#define SRAM_END			((SRAM_START) + (SIZE_SRAM))

#define T1_STACK_START		SRAM_END
#define T2_STACK_START		((SRAM_END) - (SIZE_TASK_STACK))
#define T3_STACK_START		((SRAM_END) - (2 * SIZE_TASK_STACK))
#define T4_STACK_START		((SRAM_END) - (3 * SIZE_TASK_STACK))
#define IDLE_STACK_START	((SRAM_END) - (4 * SIZE_TASK_STACK))
#define SCHED_STACK_START	((SRAM_END) - (5 * SIZE_TASK_STACK))
/* stack memory calculations END */

/*SysTick Timer macros*/
#define TICK_HZ 		1000U //frequency of SysTick timer
#define HSI_CLK 		16000000U
#define SYSTICK_TIM_CLK	HSI_CLK

/*Tasks dummy context initialization macros*/
#define DUMMY_XPSR 0x01000000U
#define EXC_RETURN 0xFFFFFFFDU
#define GENERAL_REG_NUM 13U

#define TASK_READY_STATE 0x00
#define TASK_BLOCKED_STATE 0xFF

#define INTERRUPT_DISABLE()		do{__asm volatile("MOV r0, #0x1"); __asm volatile("MSR PRIMASK, R0");} while(0)
#define INTERRUPT_ENABLE()		do{__asm volatile("MOV r0, #0x0"); __asm volatile("MSR PRIMASK, R0");} while(0)


/*tasks prototypes*/
void task1_handler(void);	//this is task 1 of the application
void task2_handler(void);	//this is task 2 of the application
void task3_handler(void);	//this is task 3 of the application
void task4_handler(void);	//this is task 4 of the application
void idle_task(void);		//this task is running while every other task is blocked


#include"scheduler.h"
#include"led.h"


#endif /* MAIN_H_ */
