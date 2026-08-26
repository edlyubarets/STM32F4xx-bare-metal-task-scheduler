# STM32F4xx-bare-metal-task-scheduler
### Overview 
---
Introducing Bare Metal Task Scheduler

This project created completely from scratch, without using any libraries such as HAL, LL or CMSIS headers

- **hardware used:** `STM32F407G Discovery Board`

This scheduler manages 4 user tasks which are used for blinking onboard LEDs & 1 idle task which is used for task delay

### Work principles description
---
- **Stack initialization:**
  
  Each task has it's own stack 1KB wide.
  For task stack manipulation we use only Processor Stack Pointer(PSP).
  For scheduler manipulation we use Main Stack pointer(MSP).
  Each task has its own stack frame 1 & stack frame 2.
  In stack initialization we have to create dummy stackframes so that task has its own stackframe before first context switch.

- **Context switch:**
 
  Context switching happens in PendSV handler. PendSV interrupt happens after pending PendSV in special function.
  Task scheduling is triggered every 1ms by a SysTick timer interrupt.

- **Stack frame:**
 
  According to stacking procedure StackFrame 1 `(xPSR, PC, LR, R12, R0-R3 registers)` is saved and restore automatically by hardware when exception is triggered, so
  StackFrame 2 `(R4-R11 registers)` have to be saved and restored by software(scheduler).

- **Task delay:**

  Task delay is implemented by blocking state for task, when a task has nothing to do it should simply call a delay function which should put the task into blocked state from running state until     the specified delay is elapsed. Scheduler schedules only those tasks which are in running state.

  ### Demonstration
---
<img width="586" height="330" alt="PXL_20260826_211333393" src="https://github.com/user-attachments/assets/c7711bc9-bbd9-4a34-be24-e33876f69272" />
