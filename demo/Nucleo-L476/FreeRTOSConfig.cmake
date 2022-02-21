
set( configUSE_PREEMPTION                     1)
set( configSUPPORT_STATIC_ALLOCATION          1)
set( configSUPPORT_DYNAMIC_ALLOCATION         0)

# For Dynamic memory use
# set( configSUPPORT_DYNAMIC_ALLOCATION         1)
# set( configUSE_FreeRTOS_HEAP                  4)
# set( configTOTAL_HEAP_SIZE                    5*1024)
#set( configUSE_MALLOC_FAILED_HOOK             0)

# For Memory Protection Unit
# set( configENABLE_MPU                         1)

set( configUSE_DAEMON_TASK_STARTUP_HOOK       1)
set( configUSE_IDLE_HOOK                      1)
set( configUSE_PORT_OPTIMISED_TASK_SELECTION  1)

set( configUSE_MUTEXES                        1)
set( configUSE_COUNTING_SEMAPHORES            1)
set( configUSE_TASK_NOTIFICATIONS             1)
set( configUSE_TASK_FPU_SUPPORT               1)

#set( configUSE_TICKLESS_IDLE                  0)
set( configCPU_CLOCK_HZ                       "SystemCoreClock" )
set( configTICK_RATE_HZ                       1000)


set( configMAX_PRIORITIES                     7 )

#******************************************************************************
# Stack overflow detection method. 
set( configCHECK_FOR_STACK_OVERFLOW 1)
set( configMINIMAL_STACK_SIZE       128)

#******************************************************************************
# Software timer definitions.
set( configUSE_TIMERS             1)
set( configTIMER_TASK_PRIORITY    2)
set( configTIMER_QUEUE_LENGTH     10)
set( configTIMER_TASK_STACK_DEPTH (configMINIMAL_STACK_SIZE * 2) )

#******************************************************************************
set( configUSE_TRACE_FACILITY 1)

set( configMAX_TASK_NAME_LEN                  16 )
set( configQUEUE_REGISTRY_SIZE                8)
set( configRECORD_STACK_HIGH_ADDRESS  1)

set( configINCLUDE_FREERTOS_TASK_C_ADDITIONS_H 1)

# RTOS scheduler will always run the highest priority task that is in the Ready 
# state, and will switch between tasks of equal priority on every RTOS tick 
# interrupt. 
set( configUSE_TIME_SLICING 1)

# Set the following definitions to 1 to include the API function, or zero to exclude the API function. 
set( INCLUDE_vTaskPrioritySet            1)
set( INCLUDE_uxTaskPriorityGet           1)
set( INCLUDE_vTaskDelete                 1)
set( INCLUDE_vTaskCleanUpResources       0)
set( INCLUDE_vTaskSuspend                1)
set( INCLUDE_vTaskDelayUntil             1)
set( INCLUDE_vTaskDelay                  1)
set( INCLUDE_xTaskGetSchedulerState      1)

set( INCLUDE_eTaskGetState               1)
set( INCLUDE_xTaskGetCurrentTaskHandle   1)
set( INCLUDE_xTaskGetHandle              1)

# The lowest interrupt priority that can be used in a call to a "set priority" function. 
set( configLIBRARY_LOWEST_INTERRUPT_PRIORITY   0xf)

# The highest interrupt priority that can be used by any interrupt service
# routine that makes calls to interrupt safe FreeRTOS API functions.
set( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5)

# A newlib reent structure will be allocated for each created task. 
set( configUSE_NEWLIB_REENTRANT 1)

# Definitions that map the FreeRTOS port interrupt handlers to their CMSIS standard names.
set( vPortSVCHandler    "SVC_Handler")
set( xPortPendSVHandler "PendSV_Handler")
set( xPortSysTickHandler "SysTick_Handler")
