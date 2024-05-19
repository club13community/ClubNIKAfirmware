//
// Created by independent-variable on 3/15/2024.
//

#pragma once
#include "stm32f10x.h"
#include "FreeRTOSConfig.h"

#define SIM900_UART				USART2
#define SIM900_UART_TX_DMA		DMA1_Channel7
// UART's "RX not empty ISR" will trigger this to handle received message
#define SIM900_EXTI_LINE		EXTI_Line0
#define SUPPLY_SYSTEM_TIMER		TIM2
#define FINE_TIMER				TIM3
#define TIMING_TIMER			TIM4

#define DAC_TIMER				TIM7
#define DAC_DMA					DMA2_Channel3

// ADC which monitors battery and wired sensors
#define VMETER_ADC				ADC3
#define VMETER_ADC_TIMER		TIM8

#define FLASH_SPI				SPI1
#define FLASH_TX_DMA_CHANNEL	DMA1_Channel3
#define FLASH_RX_DMA_CHANNEL	DMA1_Channel2

#define SD_DMA_CHANNEL			DMA2_Channel4

#define FINE_TIMER_IRQ_PRIORITY		13U
#define COARSE_TIMER_IRQ_PRIORITY	13U

#define SIM900_UART_IRQ_PRIORITY	3U
#define SIM900_DMA_IRQ_PRIORITY		14U
#define SIM900_EXTI_IRQ_PRIORITY	14U
// same for DMA and timer
#define DAC_IRQ_PRIORITY	4U
#define FLASH_IRQ_PRIORITY	6U

#define SD_IRQ_PRIORITY			11U
// for EXTI
#define KEYBOARD_IRQ_PRIORITY	13U

#define VMETER_ADC_IRQ_PRIORITY	12U

#define SIM900_UART_TX_DMA_PRIORITY	DMA_Priority_Low
#define DAC_DMA_PRIORITY			DMA_Priority_Medium
#define FLASH_TX_DMA_PRIORITY		DMA_Priority_Medium
#define FLASH_RX_DMA_PRIORITY		DMA_Priority_High
#define SD_DMA_PRIORITY				DMA_Priority_High

#define TASK_NORMAL_PRIORITY		1U
// mounts and unmounts SD card
#define SD_SERVICE_PRIORITY			2U
// parses received messages, handles timeouts, etc.
#define SIM900_DRIVER_PRIORITY		2U
// detects what was pressed
#define KEYBOARD_SERVICE_PRIORITY	3U
// starts application(mounts flash, loads settings, etc.); should have higher priority, than normal task
#define APP_STARTER_PRIORITY		4U

#if configTIMER_TASK_PRIORITY <= KEYBOARD_SERVICE_PRIORITY || configTIMER_TASK_PRIORITY <= KEYBOARD_SERVICE_PRIORITY
// highest priority guaranties that timer control commands are executed right after issuing
#error "Timer task priority is not the highest"
#endif