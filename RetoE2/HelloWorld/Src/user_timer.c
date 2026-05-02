#include <stdint.h>
#include "main.h"
#include "user_timer.h"

void USER_TIM2_Init( ){
  // Enable clock
  RCC->APB1ENR |= ( 0x1UL << 0U );
  TIM2->SMCR &= ~( 0x7UL << 0U );//                                       select internal CLK source
  TIM2->CR1 &= ~( 0x3UL << 5U ) & ~( 0x1UL << 4U ) & ~( 0x1UL << 2U ) & ~( 0x1UL << 1U );// mode edge-upcounter
  TIM2->CR1 |= ( 0x1UL << 7U );

  // Step 2
  TIM2->CCMR1 &=  ~( 0x1UL << 4U ) & ~( 0x3UL << 0U );
  TIM2->CCMR1 |= (0x3UL << 5U) | (0x2UL << 2U);

  // Step 3
  TIM2->PSC     = TIM2_PSC_PWM;
  TIM2->ARR     = TIM2_ARR_PWM;

  // Step 4
  TIM2->EGR |= (0x1UL << 0U);

  // Step 5
  TIM2->SR  &= ~( 0x1UL << 0U );//       clear the overflow flag

  // Step 6
  TIM2->CCER &= ~( 0x1UL << 1U );
  TIM2->CCER |=  ( 0x1UL << 0U );

  // Step 7
  TIM2->CR1 |= ( 0x1UL << 0U );
} 
    
// void USER_TIM2_Delay_2s( ){
//   TIM2->SR  &= ~( 0x1UL << 0U );//       clear the overflow flag
//   TIM2->PSC  = TIM2_PSC_2S;
//   TIM2->CNT  = TIM2_CNT_2S;
//   TIM2->DIER |= ( 0x1UL << 0U );
//   NVIC->ISER[0] |= ( 0x1UL << 28U );
//   TIM2->CR1 |=  ( 0x1UL << 0U );//       start the timer
// }