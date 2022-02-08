/**
  * @file: fault_handler.c
  * @brief: This file contains basic fault handlers with a kind of core dump..
  * 
  *****************************************************************************
  * @Copyright 2019, GRDF, Inc.  All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted (subject to the limitations in the disclaimer
  * below) provided that the following conditions are met:
  *    - Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *    - Redistributions in binary form must reproduce the above copyright 
  *      notice, this list of conditions and the following disclaimer in the 
  *      documentation and/or other materials provided with the distribution.
  *    - Neither the name of GRDF, Inc. nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  *****************************************************************************
  *
  * Revision history
  * ----------------
  * 1.0.0 : 2020/10/01[GBI]
  * Initial version
  *
  *
  */

#ifdef __cplusplus
extern "C" {
#endif

#include <cmsis_compiler.h>
#include <stdio.h>

#ifndef TRACE_DUMP_CORE
#define TRACE_DUMP_CORE(...) fprintf (stderr, __VA_ARGS__ );
#endif

void __init_exception_handlers__(void)
{
	return;
}

#define SYS_SECTION(hsection) __attribute__(( section(hsection) )) __attribute__((used))
#define HANDLER_SECTION(hsection) __attribute__(( section(hsection) ))

static inline uint32_t StackUnwind(void);
SYS_SECTION(".sys") static void CoreDump( uint32_t *hardfault_args );

/**
 * \brief This function back trace the stack to give exact address where fault
 happened
**/
static inline uint32_t StackUnwind(void)
{
	uint32_t Fault_Add;
	__asm("mrs r0, msp ");
	__asm("ldr %0, [r0,#28]" : "=r" (Fault_Add));
	return Fault_Add;
}

HANDLER_SECTION(".exception")
void HardFault_Handler( void ) __attribute__((naked, noreturn));
HANDLER_SECTION(".exception")
void MemManage_Handler(void) __attribute__((naked, noreturn));
HANDLER_SECTION(".exception")
void BusFault_Handler(void) __attribute__((naked, noreturn));
HANDLER_SECTION(".exception")
void UsageFault_Handler(void) __attribute__((naked, noreturn));

SYS_SECTION(".sys")
static void CoreDump( uint32_t *hardfault_args )
{
    /* These are volatile to try and prevent the compiler/linker optimizing them
    away as the variables never actually get used.  If the debugger won't show the
    values of the variables, make them global my moving their declaration outside
    of this function. */
    volatile uint32_t stacked_r0;
    volatile uint32_t stacked_r1;
    volatile uint32_t stacked_r2;
    volatile uint32_t stacked_r3;
    volatile uint32_t stacked_r12;
    volatile uint32_t stacked_lr;
    volatile uint32_t stacked_pc;
    volatile uint32_t stacked_psr;
#ifdef DUMP_CORE_HAS_FAULT_STATUS_REGISTER
    volatile uint32_t _CFSR;
    volatile uint32_t _HFSR;
    volatile uint32_t _DFSR;
    volatile uint32_t _AFSR;
    volatile uint32_t _BFAR;
    volatile uint32_t _MMAR;
#endif

    stacked_r0 = ((uint32_t)hardfault_args[0]);
    stacked_r1 = ((uint32_t)hardfault_args[1]);
    stacked_r2 = ((uint32_t)hardfault_args[2]);
    stacked_r3 = ((uint32_t)hardfault_args[3]);
    stacked_r12 = ((uint32_t)hardfault_args[4]);
    stacked_lr = ((uint32_t)hardfault_args[5]);
    stacked_pc = ((uint32_t)hardfault_args[6]);
    stacked_psr = ((uint32_t)hardfault_args[7]);

#ifdef DUMP_CORE_HAS_FAULT_STATUS_REGISTER
    // Configurable Fault Status Register
    // Consists of MMSR, BFSR and UFSR
    _CFSR = (*((volatile uint32_t *)(0xE000ED28)));

    // Hard Fault Status Register
    _HFSR = (*((volatile uint32_t *)(0xE000ED2C)));

    // Debug Fault Status Register
    _DFSR = (*((volatile uint32_t *)(0xE000ED30)));

    // Auxiliary Fault Status Register
    _AFSR = (*((volatile uint32_t *)(0xE000ED3C)));

    // Read the Fault Address Registers. These may not contain valid values.
    // Check BFARVALID/MMARVALID to see if they are valid values
    // MemManage Fault Address Register
    _MMAR = (*((volatile uint32_t *)(0xE000ED34)));
    // Bus Fault Address Register
    _BFAR = (*((volatile uint32_t *)(0xE000ED38)));
#endif

#ifdef DUMP_CORE_HAS_TRACE
    TRACE_DUMP_CORE ("\nHard fault handler :\n");
    TRACE_DUMP_CORE ("R0       = 0x%lx\n", stacked_r0);
    TRACE_DUMP_CORE ("R1       = 0x%lx\n", stacked_r1);
    TRACE_DUMP_CORE ("R2       = 0x%lx\n", stacked_r2);
    TRACE_DUMP_CORE ("R3       = 0x%lx\n", stacked_r3);
    TRACE_DUMP_CORE ("R12      = 0x%lx\n", stacked_r12);
    TRACE_DUMP_CORE ("LR [R14] = 0x%lx  subroutine call return address\n", stacked_lr);
    TRACE_DUMP_CORE ("PC [R15] = 0x%lx  program counter\n", stacked_pc);
    TRACE_DUMP_CORE ("PSR      = 0x%lx\n", stacked_psr);

#ifdef DUMP_CORE_HAS_FAULT_STATUS_REGISTER
    TRACE_DUMP_CORE ("CFSR = 0x%lx\n", _CFSR);
    TRACE_DUMP_CORE ("- Usage  Fault Status Register = 0x%lx\n", (_CFSR >> 16));
    TRACE_DUMP_CORE ("- Bus    Fault Status Register = 0x%lx\n", ((_CFSR >> 8) & 0xFF) );
    TRACE_DUMP_CORE ("- Memory Fault Status Register = 0x%lx\n", (_CFSR & 0xFF) );

    TRACE_DUMP_CORE ("HFSR = 0x%lx\n", _HFSR);
    if (_HFSR & 0b10)
    {
    	TRACE_DUMP_CORE ("- BusFault on vector table read.\n");
    }

    TRACE_DUMP_CORE ("DFSR  = 0x%lx\n", _DFSR);
    TRACE_DUMP_CORE ("AFSR  = 0x%lx (Auxiliary Fault)\n", _AFSR);
    TRACE_DUMP_CORE ("MMFAR = 0x%lx (Mem Manager Fault Address)\n", _MMAR);
    TRACE_DUMP_CORE ("BFAR  = 0x%lx (Bus Fault Address)\n", _BFAR);
#endif
#else

	(void) stacked_r0;
	(void) stacked_r1;
	(void) stacked_r2;
	(void) stacked_r3;
	(void) stacked_r12;
	(void) stacked_lr;
	(void) stacked_pc;
	(void) stacked_psr;
#ifdef DUMP_CORE_HAS_FAULT_STATUS_REGISTER
    (void) _CFSR;
    (void) _HFSR;
    (void) _DFSR;
    (void) _AFSR;
    (void) _BFAR;
    (void) _MMAR;
#endif
    // TODO : Break into the debugger
    __asm("BKPT #0\n");
#endif
    while(1);
}

void HardFault_Handler( void )
{
    __asm volatile
    (
		" movs r0,#4      \n"  /* load bit mask into R0 */
		" mov  r1, lr     \n"  /* load link register into R1 */
		" tst r0, r1      \n"  /* compare with bitmask */
		" beq _MSP        \n"  /* if bitmask is set: stack pointer is in PSP. Otherwise in MSP */
		" mrs r0, psp     \n"  /* otherwise: stack pointer is in PSP */
		" b _GetPC        \n"  /* go to part which loads the PC */
		"_MSP:            \n"  /* stack pointer is in MSP register */
		" mrs r0, msp     \n"  /* load stack pointer into R0 */
		"_GetPC:          \n"  /* find out where the hard fault happened */
		" ldr r1,[r0,#20] \n"  /* load program counter into R1. R1 contains address of the next instruction where the hard fault happened */
		" ldr r2, =CoreDump \n"
		" bx r2            \n"
    );
}

/**
* @brief This function handles Memory management fault.
*/
void MemManage_Handler(void)
{
	register uint32_t fault_address;
	fault_address = StackUnwind();
#ifdef DUMP_CORE_HAS_TRACE
	TRACE_DUMP_CORE("\nMemory Fault (MPU fault) at address 0X%x\n", (int)fault_address);
#endif
	__ISB();
	__DMB();
#ifndef DUMP_CORE_HAS_TRACE
	(void) fault_address;
	__asm volatile("BKPT #01");
#endif
	while(1);
}

/**
* @brief This function handles Prefetch fault, memory access fault.
*/
void BusFault_Handler(void)
{
	__asm("nop");
	__asm("nop");
	register uint32_t fault_address;
	fault_address = StackUnwind();
#ifdef DUMP_CORE_HAS_TRACE
	TRACE_DUMP_CORE("\nBus Fault at address 0X%x\n", (int)fault_address);
#endif
	__ISB();
	__DMB();
#ifndef DUMP_CORE_HAS_TRACE
	(void) fault_address;
	__asm volatile("BKPT #01");
#endif
	while(1);
}

/**
* @brief This function handles Undefined instruction or illegal state.
*/
void UsageFault_Handler(void)
{
	register uint32_t fault_address;
	fault_address = StackUnwind();
#ifdef DUMP_CORE_HAS_TRACE
	TRACE_DUMP_CORE("\nUsage fault at address 0X%x", (int)fault_address);
#endif
	__ISB();
	__DMB();
#ifndef DUMP_CORE_HAS_TRACE
	(void) fault_address;
	__asm volatile("BKPT #01");
#endif
	while(1);
}

#ifdef __cplusplus
}
#endif
