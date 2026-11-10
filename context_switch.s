.syntax unified
.cpu cortex-m3
.thumb

.global PendSV_Handler
.extern current_task_ptr
.extern next_task_ptr

.type PendSV_Handler, %function
PendSV_Handler:
    cpsid i                 /* Critical section: interrupts temporarily disable karein */

    /* Check karein: Kya first task launch ho raha hai? (PSP == 0) */
    mrs r0, psp
    cbz r0, restore_context /* Agar PSP 0 hai, to context save karna skip karein */

    /* Active task ke registers R4-R11 stack par push karein */
    stmdb r0!, {r4-r11}
    ldr r1, =current_task_ptr
    ldr r1, [r1]
    str r0, [r1]            /* New stack pointer ko current TCB me store karein */

restore_context:
    /* current_task_ptr = next_task_ptr */
    ldr r1, =next_task_ptr
    ldr r1, [r1]
    ldr r2, =current_task_ptr
    str r1, [r2]

    /* Next task ka stack pointer TCB se retrieve karein */
    ldr r0, [r1]
    ldmia r0!, {r4-r11}     /* R4-R11 registers pop karein */
    msr psp, r0             /* Hardware Process Stack Pointer update karein */

    cpsie i                 /* Interrupts re-enable karein */
    ldr lr, =0xFFFFFFFD     /* Thread mode with PSP use karne ka signal */
    bx lr