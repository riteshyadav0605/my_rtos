#include <stdint.h>

#define UART0_DR    (*(volatile uint32_t *)0x4000C000)
#define SYST_CSR    (*(volatile uint32_t *)0xE000E010)
#define SYST_RVR    (*(volatile uint32_t *)0xE000E014)
#define SYST_CVR    (*(volatile uint32_t *)0xE000E018)
#define SCB_ICSR    (*(volatile uint32_t *)0xE000ED04)

#define NUM_TASKS   3

/* Task Control Block (TCB) */
typedef struct {
    uint32_t *sp;          // Offset 0: Stack Pointer (Assembly ke liye first hona zaroori hai)
    uint32_t priority;     // Smaller number = Higher priority (1 = High, 2 = Low, 3 = Idle)
    uint32_t sleep_ticks;  // 0 = Ready to run, >0 = Sleeping
} TCB_t;

TCB_t tasks[NUM_TASKS];
TCB_t *current_task_ptr = 0;
TCB_t *next_task_ptr = 0;

/* Har task ka alag stack memory buffer */
uint32_t task1_stack[128];
uint32_t task2_stack[128];
uint32_t idle_stack[128];

void uart_print(const char *str) {
    while (*str) {
        UART0_DR = (uint32_t)(*str++);
    }
}

void busy_delay(volatile uint32_t count) {
    while (count--) {
        __asm volatile ("nop");
    }
}

/* Priority Scheduler: Sabse high priority ready task select karta hai */
void schedule(void) {
    TCB_t *best_task = &tasks[2]; // Default: Idle task

    for (int i = 0; i < NUM_TASKS; i++) {
        if (tasks[i].sleep_ticks == 0) {
            if (tasks[i].priority < best_task->priority) {
                best_task = &tasks[i];
            }
        }
    }

    if (best_task != current_task_ptr) {
        next_task_ptr = best_task;
        SCB_ICSR = (1 << 28); // Trigger PendSV context switch
    }
}

/* Non-blocking OS Delay: Task ko sleep state me bhejta hai */
void os_delay(uint32_t ticks) {
    __asm volatile ("cpsid i");
    current_task_ptr->sleep_ticks = ticks;
    schedule();
    __asm volatile ("cpsie i");
    __asm volatile ("isb");
}

/* Task 1: HIGH Priority (Priority = 1) */
void task1_high(void) {
    while (1) {
        uart_print("\n[Task 1 - HIGH] Urgent Sensor Check! Going to sleep for 3 ticks...\n");
        os_delay(3); // 3 SysTick intervals ke liye CPU chhod dega
    }
}

/* Task 2: LOW Priority (Priority = 2) */
void task2_low(void) {
    while (1) {
        uart_print("    [Task 2 - LOW] Running background work...\n");
        busy_delay(4000000);
    }
}

/* Task 3: IDLE Task (Priority = 3) */
void task_idle(void) {
    while (1) {
        __asm volatile ("nop");
    }
}

/* Task initial stack frame aur priority setup */
void task_create(TCB_t *tcb, void (*task_func)(void), uint32_t *stack_top, uint32_t priority) {
    stack_top--;
    *stack_top = 0x01000000;              // xPSR (Thumb mode)
    stack_top--;
    *stack_top = (uint32_t)task_func;      // PC (Task entry point)
    stack_top--;
    *stack_top = 0xFFFFFFFD;              // LR (Return to Thread mode with PSP)
    stack_top--;
    *stack_top = 0;                       // R12
    stack_top--;
    *stack_top = 0;                       // R3
    stack_top--;
    *stack_top = 0;                       // R2
    stack_top--;
    *stack_top = 0;                       // R1
    stack_top--;
    *stack_top = 0;                       // R0

    // Software saved registers (R11 to R4)
    for (int i = 0; i < 8; i++) {
        stack_top--;
        *stack_top = 0;
    }

    tcb->sp = stack_top;
    tcb->priority = priority;
    tcb->sleep_ticks = 0;
}

/* SysTick Interrupt: Sleep timers ghatao aur highest priority task chuno */
void SysTick_Handler(void) {
    for (int i = 0; i < NUM_TASKS; i++) {
        if (tasks[i].sleep_ticks > 0) {
            tasks[i].sleep_ticks--;
        }
    }
    schedule();
}

void systick_init(uint32_t ticks) {
    SYST_RVR = ticks - 1;
    SYST_CVR = 0;
    SYST_CSR = 0x07;
}

void main(void) {
    uart_print("\n=== Custom Priority-Based RTOS Started ===\n");

    task_create(&tasks[0], task1_high, &task1_stack[128], 1); // Priority 1 (Highest)
    task_create(&tasks[1], task2_low,  &task2_stack[128], 2); // Priority 2 (Low)
    task_create(&tasks[2], task_idle,  &idle_stack[128],  3); // Priority 3 (Idle)

    next_task_ptr = &tasks[0];

    systick_init(1000000);

    SCB_ICSR = (1 << 28);       // Pehla switch trigger karein
    __asm volatile ("cpsie i"); // Interrupts on karein

    while (1) {
        // Main idle
    }
}