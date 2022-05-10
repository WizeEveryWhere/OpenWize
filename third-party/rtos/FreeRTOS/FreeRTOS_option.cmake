if(NOT configUSE_PREEMPTION)
    set( configUSE_PREEMPTION 0U)
endif()

if(NOT configSUPPORT_DYNAMIC_ALLOCATION)
    set( configSUPPORT_DYNAMIC_ALLOCATION 0U)
endif()

if(NOT configUSE_IDLE_HOOK)
    set( configUSE_IDLE_HOOK 0U)
endif()

if(NOT configUSE_TICK_HOOK)
    set( configUSE_TICK_HOOK 0U)
endif()

if(NOT configUSE_16_BIT_TICKS)
    set( configUSE_16_BIT_TICKS 0U)
endif()

if(NOT configMAX_PRIORITIES)
    set( configMAX_PRIORITIES 7)
endif()

if(NOT configMINIMAL_STACK_SIZE)
    set( configMINIMAL_STACK_SIZE 128)
endif()

if(NOT configTIMER_TASK_STACK_DEPTH)
    set( configTIMER_TASK_STACK_DEPTH (configMINIMAL_STACK_SIZE * 2))
endif()

if(configUSE_TIMERS)
    if(NOT configTIMER_TASK_PRIORITY)
        set( configTIMER_TASK_PRIORITY 2)
    endif()
    if(NOT configTIMER_QUEUE_LENGTH)
        set( configTIMER_QUEUE_LENGTH 10)
    endif()  
endif()


