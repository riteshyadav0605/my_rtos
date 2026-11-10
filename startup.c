#include <stdint.h>

extern uint32_t _estack;
extern void main(void);
extern void PendSV_Handler(void);
extern void SysTick_Handler(void);

void Reset_Handler(void) {
    main();
    while (1);
}

void Default_Handler(void) {
    while (1);
}

__attribute__ ((section(".isr_vector")))
void (* const vector_table[])(void) = {
    (void (*)(void))((uint32_t)&_estack), // 0: Initial Stack Pointer
    Reset_Handler,                         // 1: Reset Handler
    Default_Handler,                       // 2: NMI
    Default_Handler,                       // 3: HardFault
    Default_Handler,                       // 4: MemManage
    Default_Handler,                       // 5: BusFault
    Default_Handler,                       // 6: UsageFault
    0, 0, 0, 0,                            // 7-10: Reserved
    Default_Handler,                       // 11: SVCall
    Default_Handler,                       // 12: Debug Monitor
    0,                                     // 13: Reserved
    PendSV_Handler,                        // 14: PendSV Handler
    SysTick_Handler                        // 15: SysTick Timer Handler
};