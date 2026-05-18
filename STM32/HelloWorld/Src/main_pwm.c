#include <stdint.h>
#include <stdio.h>
#include "main.h"
#include "user_timer.h"

// Define button macro
#define BUTTON (GPIOA->IDR & ( 0x1UL << 1U ))

int main(void){
    USER_SystemClock_Config();
    USER_GPIO_Init();
    USER_TIM2_Init( );
	
    /* Repetitive block */
    uint8_t brightness = 0;

    for(;;){
        if( !BUTTON ){//                  if button is pressed
            USER_Delay_10ms( );//      10ms
            if( !BUTTON ){//                double checking
				
                // Increment brightness by 10%
                brightness = (brightness + 1) % 11;
                // Increments of 6400 in CCR, max of 63999
                TIM2->CCR1 = ((6400 * brightness) - (brightness == 10));

                while( !BUTTON );//           waits until button released
                USER_Delay_10ms( );//    10ms
            }
        }
    }
}

void USER_GPIO_Init( void ){
	RCC->APB2ENR	|=	 ( 0x1UL <<  2U );//	IO port A clock enable
	// PA0 as alternate function push pull
	GPIOA->CRL      &=  ~( 0x1UL << 2U ); 
    GPIOA->CRL      |=   ( 0x2UL << 2U ) | ( 0x3UL << 0U );
	// PA1 as input pull up
	GPIOA->ODR 		|= 	 ( 0x1UL << 1U );
	GPIOA->CRL 		&=	~( 0x1UL << 6U );
	GPIOA->CRL 		&=	~( 0x3UL << 4U );
	GPIOA->CRL		|= 	 ( 0x2UL << 6U );
}

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

void USER_Delay_10ms( void ){
	__asm(" 			ldr r0, =71111UL	");//	load the value to be used as delay count
	__asm(" again10:	sub r0, r0, #1		");//	decrement the delay count
	__asm("				cmp r0, #0			");//	check if the delay count has reached zero
	__asm("				bne again10			");//	if not, repeat the process
	__asm("				nop					");//	no operation (to ensure exact timing)
}