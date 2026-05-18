#include <stdint.h>
#include "main.h"
#include "user_timer.h"

void USER_SystemClock_Config( void ){
	FLASH->ACR	&=	~( 0x5UL <<  0U );//		two wait states latency, if SYSCLK > 48MHz
	FLASH->ACR	|=	 ( 0x2UL <<  0U );//		two wait states latency, if SYSCLK > 48MHz
	RCC->CFGR	&=	~( 0x1UL << 16U )//			PLL HSI oscillator clock /2 selected as PLL input clock
				&	~( 0x7UL << 11U )// 		APB2 prescaler /1
				&	~( 0x3UL <<  8U );// 		APB1 prescaler /2
	RCC->CFGR	|=	 ( 0xFUL << 18U )//			PLL input clock x 16 (PLLMUL bits)
				|	 ( 0x4UL <<  8U );//		APB1 prescaler /2
	RCC->CR		|=	 ( 0x1UL << 24U );//		PLL2 ON
	while( !(RCC->CR & ~( 0x1UL << 25U )));//	wait until PLL is locked
	RCC->CFGR	&=	~( 0x1UL << 0U  );//		PLL used as system clock (SW bits)
	RCC->CFGR	|=	 ( 0x2UL << 0U  );//		PLL used as system clock (SW bits)
	while( 0x8UL != ( RCC->CFGR & 0xCUL ));//	wait until PLL is switched
}

void USER_TIM2_Init( void ){
  // Enable clock
  RCC->APB1ENR |= ( 0x1UL << 0U );
  TIM2->SMCR &= ~( 0x7UL << 0U );//                                       select internal CLK source
  TIM2->CR1 &= ~( 0x3UL << 5U ) & ~( 0x1UL << 4U ) & ~( 0x1UL << 1U );// mode edge-upcounter
}

void USER_TIM2_Delay_10us( void ){
  TIM2->SR  &= ~( 0x1UL << 0U );//       clear the overflow flag
  TIM2->PSC  = TIM2_PSC_10US;
  TIM2->CNT  = TIM2_CNT_10US;
  TIM2->CR1 |=  ( 0x1UL << 0U );//       start the timer
  while(!( TIM2->SR & ( 0x1UL << 0U )));
  TIM2->CR1 &= ~( 0x1UL << 0U );//       stop the timer
}

void USER_TIM2_Delay_53us( void ){
  TIM2->SR  &= ~( 0x1UL << 0U );//       clear the overflow flag
  TIM2->PSC  = TIM2_PSC_53US;
  TIM2->CNT  = TIM2_CNT_53US;
  TIM2->CR1 |=  ( 0x1UL << 0U );//       start the timer
  while(!( TIM2->SR & ( 0x1UL << 0U )));
  TIM2->CR1 &= ~( 0x1UL << 0U );//       stop the timer
}

void USER_TIM2_Delay_100us( void ){
  TIM2->SR  &= ~( 0x1UL << 0U );//       clear the overflow flag
  TIM2->PSC  = TIM2_PSC_100US;
  TIM2->CNT  = TIM2_CNT_100US;
  TIM2->CR1 |=  ( 0x1UL << 0U );//       start the timer
  while(!( TIM2->SR & ( 0x1UL << 0U )));
  TIM2->CR1 &= ~( 0x1UL << 0U );//       stop the timer
}

void USER_TIM2_Delay_1ms( void ){
  TIM2->SR  &= ~( 0x1UL << 0U );//       clear the overflow flag
  TIM2->PSC  = TIM2_PSC_1MS;
  TIM2->CNT  = TIM2_CNT_1MS;
  TIM2->CR1 |=  ( 0x1UL << 0U );//       start the timer
  while(!( TIM2->SR & ( 0x1UL << 0U )));
  TIM2->CR1 &= ~( 0x1UL << 0U );//       stop the timer
}

void USER_TIM2_Delay_4_1ms( void ){
  TIM2->SR  &= ~( 0x1UL << 0U );//       clear the overflow flag
  TIM2->PSC  = TIM2_PSC_4_1MS;
  TIM2->CNT  = TIM2_CNT_4_1MS;
  TIM2->CR1 |=  ( 0x1UL << 0U );//       start the timer
  while(!( TIM2->SR & ( 0x1UL << 0U )));
  TIM2->CR1 &= ~( 0x1UL << 0U );//       stop the timer
}

void USER_TIM2_Delay_10ms( void ){
  TIM2->SR  &= ~( 0x1UL << 0U );//       clear the overflow flag
  TIM2->PSC  = TIM2_PSC_10MS;
  TIM2->CNT  = TIM2_CNT_10MS;
  TIM2->CR1 |=  ( 0x1UL << 0U );//       start the timer
  while(!( TIM2->SR & ( 0x1UL << 0U )));
  TIM2->CR1 &= ~( 0x1UL << 0U );//       stop the timer
}

void USER_TIM2_Delay_40ms( void ){
  TIM2->SR  &= ~( 0x1UL << 0U );//       clear the overflow flag
  TIM2->PSC  = TIM2_PSC_40MS;
  TIM2->CNT  = TIM2_CNT_40MS;
  TIM2->CR1 |=  ( 0x1UL << 0U );//       start the timer
  while(!( TIM2->SR & ( 0x1UL << 0U )));
  TIM2->CR1 &= ~( 0x1UL << 0U );//       stop the timer
}

void USER_TIM2_Delay_100ms( void ){
  TIM2->SR  &= ~( 0x1UL << 0U );//       clear the overflow flag
  TIM2->PSC  = TIM2_PSC_100MS;
  TIM2->CNT  = TIM2_CNT_100MS;
  TIM2->CR1 |=  ( 0x1UL << 0U );//       start the timer
  while(!( TIM2->SR & ( 0x1UL << 0U )));
  TIM2->CR1 &= ~( 0x1UL << 0U );//       stop the timer
}

void USER_TIM2_Delay_200ms( void ){
  TIM2->SR  &= ~( 0x1UL << 0U );//       clear the overflow flag
  TIM2->PSC  = TIM2_PSC_200MS;
  TIM2->CNT  = TIM2_CNT_200MS;
  TIM2->CR1 |=  ( 0x1UL << 0U );//       start the timer
  while(!( TIM2->SR & ( 0x1UL << 0U )));
  TIM2->CR1 &= ~( 0x1UL << 0U );//       stop the timer
}

void USER_TIM2_Delay_500ms( void ){
  TIM2->SR  &= ~( 0x1UL << 0U );//       clear the overflow flag
  TIM2->PSC  = TIM2_PSC_500MS;
  TIM2->CNT  = TIM2_CNT_500MS;
  TIM2->CR1 |=  ( 0x1UL << 0U );//       start the timer
  while(!( TIM2->SR & ( 0x1UL << 0U )));
  TIM2->CR1 &= ~( 0x1UL << 0U );//       stop the timer
}

void USER_TIM2_Delay_1sec( void ){
  TIM2->SR  &= ~( 0x1UL << 0U );//       clear the overflow flag
  TIM2->PSC  = TIM2_PSC_1S;
  TIM2->CNT  = TIM2_CNT_1S;
  TIM2->CR1 |=  ( 0x1UL << 0U );//       start the timer
  while(!( TIM2->SR & ( 0x1UL << 0U )));
  TIM2->CR1 &= ~( 0x1UL << 0U );//       stop the timer
}

void USER_TIM2_Delay_2sec( void ){
  TIM2->SR  &= ~( 0x1UL << 0U );//       clear the overflow flag
  TIM2->PSC  = TIM2_PSC_2S;
  TIM2->CNT  = TIM2_CNT_2S;
  TIM2->CR1 |=  ( 0x1UL << 0U );//       start the timer
  while(!( TIM2->SR & ( 0x1UL << 0U )));
  TIM2->CR1 &= ~( 0x1UL << 0U );//       stop the timer
}

void USER_TIM3_Init( void ){
  // Enable clock
  RCC->APB1ENR |= ( 0x1UL << 1U );
  TIM3->SMCR &= ~( 0x7UL << 0U );//                                       select internal CLK source
  TIM3->CR1 &= ~( 0x3UL << 5U ) & ~( 0x1UL << 4U ) & ~( 0x1UL << 1U );// mode edge-upcounter
}
    
void USER_TIM3_Delay_40ms( void ){
  TIM3->SR  &= ~( 0x1UL << 0U );//       clear the overflow flag
  TIM3->PSC  = TIM3_PSC_40MS;
  TIM3->CNT  = TIM3_CNT_40MS;
  TIM3->DIER |= ( 0x1UL << 0U );
  NVIC->ISER[0] |= ( 0x1UL << 29U );
  TIM3->CR1 |=  ( 0x1UL << 0U );//       start the timer
}

void USER_TIM4_Init( void ){
  // Enable clock
  RCC->APB1ENR |= ( 0x1UL << 2U );
  TIM4->SMCR &= ~( 0x7UL << 0U );//                                       select internal CLK source
  TIM4->CR1 &= ~( 0x3UL << 5U ) & ~( 0x1UL << 4U ) & ~( 0x1UL << 2U ) & ~( 0x1UL << 1U );// mode edge-upcounter
  TIM4->CR1 |= ( 0x1UL << 7U );

  // Step 2
  // Channel 1
  TIM4->CCMR1 &=  ~( 0x1UL << 4U ) & ~( 0x3UL << 0U );
  TIM4->CCMR1 |= (0x3UL << 5U) | (0x2UL << 2U);

  // Channel 2
  TIM4->CCMR1 &=  ~( 0x1UL << 12U ) & ~( 0x3UL << 8U );
  TIM4->CCMR1 |= (0x3UL << 13U) | (0x2UL << 10U);

  // Channel 3
  TIM4->CCMR2 &=  ~( 0x1UL << 4U ) & ~( 0x3UL << 0U );
  TIM4->CCMR2 |= (0x3UL << 5U) | (0x2UL << 2U);

  // Channel 4
  TIM4->CCMR2 &=  ~( 0x1UL << 12U ) & ~( 0x3UL << 8U );
  TIM4->CCMR2 |= (0x3UL << 13U) | (0x2UL << 10U);

  // Step 3
  TIM4->PSC     = TIM4_PSC_PWM;
  TIM4->ARR     = TIM4_ARR_PWM;

  // Step 4
  TIM4->EGR |= (0x1UL << 0U);

  // Step 5
  TIM4->SR  &= ~( 0x1UL << 0U );//       clear the overflow flag

  // Step 6
  // Channel 1
  TIM4->CCER &= ~( 0x1UL << 1U );
  TIM4->CCER |=  ( 0x1UL << 0U );

  // Channel 2
  TIM4->CCER &= ~( 0x1UL << 5U );
  TIM4->CCER |=  ( 0x1UL << 4U );

  // Channel 3
  TIM4->CCER &= ~( 0x1UL << 9U );
  TIM4->CCER |=  ( 0x1UL << 8U );

  // Channel 4
  TIM4->CCER &= ~( 0x1UL << 13U );
  TIM4->CCER |=  ( 0x1UL << 12U );

  // Step 7
  TIM4->CR1 |= ( 0x1UL << 0U );
} 