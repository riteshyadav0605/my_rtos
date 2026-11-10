#include <stdint.h>

#define UART0_DR    (*(volatile uint32_t *)0x4000C000)
#define SYST_CSR    (*(volatile uint32_t *)0xE000E010)
#define SYST_RVR    (*(volatile uint32_t *)0xE000E014)
#define SYST_CVR    (*(volatile uint32_t *)0xE000E018)
#define SCB_ICSR    (*(volatile uint32_t *)0xE000ED04)


typedef struct {
    uint32_t *sp;
} TCB_t;

TCB_t tasks[2];
TCB_t *current_task_ptr = 0;
TCB_t *next_task_ptr = 0;

uint32_t task1_stack[128];
uint32_t task2_stack[128];

void uart_print(const char *str) {
    while (*str) {
        UART0_DR = (uint32_t)(*str++);
    }
}

void delay(volatile uint32_t count) {
    while (count--) {
        __asm volatile ("nop");
    }
}

/* Task 1 */
void task1(void) {
    while (1) {
        uart_print("[Task 1] Running...\n");
        delay(1200000);
    }
}

/* Task 2 */
void task2(void) {
    while (1) {
        uart_print("    [Task 2] Running...\n");
        delay(1200000);
    }
}

/* Task initial stack frame setup */
void task_create(TCB_t *tcb, void (*task_func)(void), uint32_t *stack_top) {
    // Hardware auto-stacked registers
    stack_top--;
    *stack_top = 0x01000000;              // xPSR (Thumb mode flag set)
    stack_top--;
    *stack_top = (uint32_t)task_func;      // PC (Task entry function)
    stack_top--;
    *stack_top = 0xFFFFFFFD;              // LR (Return to Thread mode)
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
}

/* SysTick Interrupt: Har tick par task switch trigger karta hai */
void SysTick_Handler(void) {
    if (current_task_ptr == &tasks[0]) {
        next_task_ptr = &tasks[1];
    } else {
        next_task_ptr = &tasks[0];
    }

    // PendSV interrupt set karein (bit 28)
    SCB_ICSR = (1 << 28);
}

void systick_init(uint32_t ticks) {
    SYST_RVR = ticks - 1;
    SYST_CVR = 0;
    SYST_CSR = 0x07; // Enable timer, interrupt, processor clock
}

void main(void) {
    uart_print("\n=== Custom Preemptive RTOS Started ===\n");

    task_create(&tasks[0], task1, &task1_stack[128]);
    task_create(&tasks[1], task2, &task2_stack[128]);

    next_task_ptr = &tasks[0];

    systick_init(1000000); // Periodic tick interval

    SCB_ICSR = (1 << 28);  // Trigger initial PendSV
    __asm volatile ("cpsie i"); // Enable interrupts

    while (1) {
        // Idle loop
    }
}