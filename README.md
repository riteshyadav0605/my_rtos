# Custom Bare-Metal Preemptive RTOS 🚀

A lightweight, custom-built Preemptive Real-Time Operating System (RTOS) kernel developed from scratch for the ARM Cortex-M3 architecture. Instead of relying on existing libraries like FreeRTOS, this project demonstrates a deep understanding of microcontroller architecture, core registers, and hardware-level multitasking.

## 🛠️ Tech Stack & Architecture
* **Target Architecture:** ARM Cortex-M3 (Simulated on QEMU `lm3s6965evb`)
* **Languages:** Bare-metal C, ARM Assembly
* **Toolchain:** GNU ARM Embedded Toolchain (`arm-none-eabi-gcc`)
* **Build System & Debugging:** Custom Linker Script (`.ld`), GDB, QEMU

## ⚙️ Core Features Implemented
* **Hardware-Level Multitasking:** Independent infinite loops running seemingly in parallel.
* **Custom Context Switching:** Implemented via `PendSV` handler in ARM Assembly to securely push/pop CPU registers (R0-R15) to/from thread-specific stack memory.
* **Time-Slicing Scheduling:** Hardware `SysTick` timer configured to trigger context switches at exact millisecond intervals (Round-Robin scheduling).
* **Task Control Blocks (TCB):** Custom data structures managing individual stack pointers for active threads.

## 🚀 How to Run (Linux / WSL)

1. **Install Dependencies:**
   ```bash
   sudo apt update
   sudo apt install gcc-arm-none-eabi qemu-system-arm
