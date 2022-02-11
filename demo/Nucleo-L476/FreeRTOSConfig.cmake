
set( configUSE_PREEMPTION                     1U)

set( configSUPPORT_STATIC_ALLOCATION          1U)
set( configSUPPORT_DYNAMIC_ALLOCATION         0U)

#set( configSUPPORT_DYNAMIC_ALLOCATION         1U)
#set( configUSE_FreeRTOS_HEAP                  4)
#set( configTOTAL_HEAP_SIZE                    5*1024)

#set( configENABLE_MPU                         1)

set( configUSE_DAEMON_TASK_STARTUP_HOOK       1U)
set( configUSE_IDLE_HOOK                      1U)
set( configUSE_PORT_OPTIMISED_TASK_SELECTION  1U)
set( configUSE_MALLOC_FAILED_HOOK             0U)

set( configUSE_MUTEXES                        1U)
set( configUSE_COUNTING_SEMAPHORES            1U)
set( configUSE_TASK_NOTIFICATIONS             1U)
set( configUSE_TASK_FPU_SUPPORT               1U)

set( configENABLE_BACKWARD_COMPATIBILITY      0U)

set( configUSE_TICK_HOOK                      0U)
set( configUSE_16_BIT_TICKS                   0U)
set( configUSE_TICKLESS_IDLE                  0U)
set( configCPU_CLOCK_HZ                       "SystemCoreClock" )
set( configTICK_RATE_HZ                       1000)

set( configMAX_PRIORITIES                     7 )


# Co-routine definitions.
set( configUSE_CO_ROUTINES           0U) 
set( configMAX_CO_ROUTINE_PRIORITIES 2)

# Software timer definitions.
set( configUSE_TIMERS             1U)
set( configTIMER_TASK_PRIORITY    2)
set( configTIMER_QUEUE_LENGTH     10)
set( configTIMER_TASK_STACK_DEPTH (configMINIMAL_STACK_SIZE * 2) )

#******************************************************************************
# Stack overflow detection method. 
set( configCHECK_FOR_STACK_OVERFLOW 1U)
set( configMINIMAL_STACK_SIZE                 128)

#******************************************************************************
set( configUSE_TRACE_FACILITY 1U)

set( configMAX_TASK_NAME_LEN                  16 )
set( configQUEUE_REGISTRY_SIZE                8)
set( configRECORD_STACK_HIGH_ADDRESS  1U)

set( configINCLUDE_FREERTOS_TASK_C_ADDITIONS_H 1U)

# RTOS scheduler will always run the highest priority task that is in the Ready 
# state, and will switch between tasks of equal priority on every RTOS tick 
# interrupt. 
set( configUSE_TIME_SLICING 1U)

# Set the following definitions to 1 to include the API function, or zero to exclude the API function. 
set( INCLUDE_vTaskPrioritySet            1U)
set( INCLUDE_uxTaskPriorityGet           1U)
set( INCLUDE_vTaskDelete                 1U)
set( INCLUDE_vTaskCleanUpResources       0U)
set( INCLUDE_vTaskSuspend                1U)
set( INCLUDE_vTaskDelayUntil             1U)
set( INCLUDE_vTaskDelay                  1U)
set( INCLUDE_xTaskGetSchedulerState      1U)

set( INCLUDE_eTaskGetState               1U)
set( INCLUDE_xTaskGetCurrentTaskHandle   1U)
set( INCLUDE_xTaskGetHandle              1U)

# The lowest interrupt priority that can be used in a call to a "set priority" function. 
set( configLIBRARY_LOWEST_INTERRUPT_PRIORITY   0xf)

# The highest interrupt priority that can be used by any interrupt service
# routine that makes calls to interrupt safe FreeRTOS API functions.
set( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5)

# A newlib reent structure will be allocated for each created task. 
set( configUSE_NEWLIB_REENTRANT 1U)

# Definitions that map the FreeRTOS port interrupt handlers to their CMSIS standard names.
set( vPortSVCHandler    "SVC_Handler")
set( xPortPendSVHandler "PendSV_Handler")
set( xPortSysTickHandler "SysTick_Handler")
