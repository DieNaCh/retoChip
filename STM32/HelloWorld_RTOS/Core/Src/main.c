/* **************** START *********************** */
/* Libraries, Definitions and Global Declarations */
#include <stdint.h>
#include <stdio.h>
#include "main.h"
#include "user_uart.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

TaskHandle_t Task1Handle;

void USER_SystemClock_Config( void );
void StartTask1( void *pvParameters );

/* Superloop structure */
int main(void)
{
	/* Declarations and Initializations */
  HAL_Init( );
  USER_SystemClock_Config( );
	USER_UART2_Init( );

	/* Create a task with a priority of 0 (idle), 1 (belowNormal), 2 (Normal), 3 (High), 4 (VeryHigh) */
	xTaskCreate(StartTask1, "Task1", 128, NULL, 2, &Task1Handle);

	/* Start the scheduler */
	printf("Heap Available: %u bytes\r\n", xPortGetFreeHeapSize());
	printf("Initializing Scheduler...\r\n");
	vTaskStartScheduler();

	/* Repetitive block */
	for(;;){

	}
}

// Task1 function
void StartTask1(void *pvParameters) {

  /* Infinite loop */
  for(;;) {
	  printf("Task 1\r\n");
	  vTaskDelay(1000);
  }
}

void USER_SystemClock_Config( void ){
	FLASH->ACR	&=	~( 0x5UL <<  0U );//		two wait states latency, if SYSCLK > 48MHz
	FLASH->ACR	|=	 ( 0x2UL <<  0U );//		two wait states latency, if SYSCLK > 48MHz
	RCC->CFGR	  &=	~( 0x1UL << 16U )//			PLL HSI oscillator clock /2 selected as PLL input clock
				      &	  ~( 0x7UL << 11U )// 		APB2 prescaler /1
				      &	  ~( 0x3UL <<  8U );// 		APB1 prescaler /2
	RCC->CFGR	  |=	 ( 0xFUL << 18U )//			PLL input clock x 16 (PLLMUL bits)
				      |	   ( 0x4UL <<  8U );//		APB1 prescaler /2
	RCC->CR		  |=	 ( 0x1UL << 24U );//		PLL2 ON
	while( !(RCC->CR & ~( 0x1UL << 25U )));// wait until PLL is locked
	RCC->CFGR	  &=	~( 0x1UL << 0U  );//		PLL used as system clock (SW bits)
	RCC->CFGR	  |=	 ( 0x2UL << 0U  );//		PLL used as system clock (SW bits)
	while(0x8UL !=   ( RCC->CFGR & 0xCUL ));// wait until PLL is switched
  SystemCoreClock = 64000000U;
}