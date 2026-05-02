#include <cstdint>
#include <stdint.h>
#include <stdio.h>
#include "main.h"
#include "user_timer.h"
#include "user_uart.h"
#include "user_adc.h"

// Define button macro
#define BUTTON (GPIOA->IDR & ( 0x1UL << 1U ))

int main(void){
    USER_SystemClock_Config();
    USER_GPIO_Init();
    USER_ADC_Init();
    USER_USART2_Init();
	
    /* Repetitive block */
    for(;;){
        if ((ADC->SR & ( 0x1UL << 1U ))) {
            // 1. Read the raw 12-bit ADC value
            uint32_t result = ADC1->DR;

            // 2. Create a character buffer
            char msgBuffer[32];

            // 3. Format the integer into a readable ASCII string
            snprintf(msgBuffer, sizeof(msgBuffer), "ADC Value: %lu\r\n", result);

            // 4. Calculate the exact length of the formatted string
            uint16_t messageLength = strlen(msgBuffer);

            // 5. Send it using your custom function!
            // Note: We cast msgBuffer to (uint8_t *) to keep the compiler happy, 
            // since snprintf uses standard 'char' but your function strictly asks for 'uint8_t'.
            USER_USART2_Transmit((uint8_t *)msgBuffer, messageLength);
        }
    }
}

void USER_ADC_Init( void ) {
    // Step 0a: Enable clock for ADC1
    RCC->APB2ENR |= ( 0x1UL << 9U );

    // Step 0b: Adjust ADC input clock
    RCC->CFGR |= ( 0x3UL << 14U );

    // Step 1: Select operation mode
    ADC->CR1 &= ~( 0x3UL << 18U );
    ADC->CR1 &= ~( 0x3UL << 16U );

    // Step 2: Determine the result format
    ADC->CR2 &= ~( 0x1UL << 11U );
    ADC->CR2 |= ( 0x1UL << 1U );

    // Step 3: Determine the sample time for the ADC conversion
    ADC->SMPR2 &= ~( 0x7UL << 0U );

    // Step 4: Select the sequence and/or number of conversions for the ADC regular channels
    ADC->SQR1 &= ~( 0xFUL << 20U );

    // Step 5: Select channel 0 for conversion
    ADC->SQR3 &= ~( 0x1F << 0U );

    // Step 6: Enable the ADC module
    ADC->CR2 |= ( 0x1UL << 0U );

    // Step 7: Calibration
    ADC->CR2 |= ( 0x1UL << 2U );
    while (ADC->CR2 & ( 0x1UL << 2U ));

    // Step 8: Start conversion
    ADC->CR2 |= ( 0x1UL << 0U );
}

void USER_GPIO_Init( void ){
	RCC->APB2ENR	|=	 ( 0x1UL <<  2U );//	IO port A clock enable
	// PA0 as analog input
	GPIOA->CRL      &=  ~( 0x3UL << 0U );
    GPIOA->CRL      &=  ~( 0x3UL << 2U );
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