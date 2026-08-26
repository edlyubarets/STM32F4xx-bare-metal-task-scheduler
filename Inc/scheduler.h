/*
 * scheduler.h
 *
 *  Created on: Aug 26, 2026
 *      Author: dell
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

/*TASK CONTROL BLOCK STRUCTURE*/
typedef struct
{
	uint32_t psp_value				;
	uint32_t block_count			;
	uint32_t current_state			;
	void (*task_handler)(void)		;
}TCB_t;



/*Scheduler functions prototypes*/
void init_systick_timer(uint32_t tick_hz);	//systick timer initialization function

void init_scheduler_stack(uint32_t sched_top_of_stack); //scheduler stack initaiolization function

void init_task_stack(void);		//tasks stack initialization function

void enable_processor_faults(void);		//enabling faults function

__attribute__((naked)) void switch_sp_to_psp(void);

uint32_t get_psp_value(void);

void save_psp_value(uint32_t current_psp_value);

void task_delay(uint32_t tick_count);

void update_global_tick_count(void);

void unblock_tasks(void);



#endif /* SCHEDULER_H_ */
