#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include "main.h"
#include "lcd.h"
#include "user_timer.h"
#include "user_uart.h"
#include "user_adc.h"
#include "EngTrModel.h"
#include "rtwtypes.h"

// Define button macro
#define BUTTON (GPIOA->IDR & ( 0x1UL << 1U ))

bool model_updated = false;

void TIM3_IRQHandler( void ) {
	if ( TIM3->SR & ( 0x1UL << 0U ) ) {
		EngTrModel_step();
		model_updated = true;
		TIM3->SR &= ~( 0x1UL << 0U );
		TIM3->CNT = TIM3_CNT_40MS;
	}
}

int main(void){
    USER_SystemClock_Config();
    USER_GPIO_Init();
	USER_TIM2_Init( );
	USER_TIM3_Init( );
    USER_ADC_Init();
    USER_USART2_Init();
	LCD_Init(); // MUST GO AFTER TIM2 INIT
	EngTrModel_initialize();
	USER_TIM3_Delay_40ms();
	LCD_Clear( );
	
    /* Repetitive block */
    for(;;){
        // if ((ADC->SR & ( 0x1UL << 1U ))) {
        //     // 1. Read the raw 12-bit ADC value
        //     uint32_t result = ADC->DR;

        //     // 2. Create a character buffer
        //     char msgBuffer[32];

        //     // 3. Format the integer into a readable ASCII string
        //     snprintf(msgBuffer, sizeof(msgBuffer), "ADC Value: %lu\r\n", result);

        //     // 4. Calculate the exact length of the formatted string
        //     uint16_t messageLength = strlen(msgBuffer);

        //     // 5. Send it using your custom function!
        //     // Note: We cast msgBuffer to (uint8_t *) to keep the compiler happy, 
        //     // since snprintf uses standard 'char' but your function strictly asks for 'uint8_t'.
        //     USER_USART2_Transmit((uint8_t *)msgBuffer, messageLength);
        // }

		/* --------------- Throttle through push button action ----------------- */
		if (!BUTTON) {
			USER_TIM2_Delay_10ms();
			if (!BUTTON) {
				EngTrModel_U.Throttle = 1.45;
				EngTrModel_U.BrakeTorque = 100.0;
			}
		}
		else {
			EngTrModel_U.Throttle = 50.0;
			EngTrModel_U.BrakeTorque = 0.0;
		}

		if (model_updated == 1) {
			/* ---------------- Display data in LCD Display ------------------- */

            char lcd_buf[16]; // Buffer to hold data, using snprintf

			// --- DISPLAY LINE 1 ---
			LCD_Set_Cursor( 1, 1 );
			
			// Format: 
			// Thr: xx.xx  G: x
			// We use extra spaces at the end for formatting

			// Throttle
			LCD_Put_Str( "Thr: " ); 
			snprintf(lcd_buf, sizeof(lcd_buf), "%.2f  ", EngTrModel_U.Throttle);
			LCD_Put_Str( lcd_buf ); 

			// Gear
			LCD_Put_Str( "G: " );
			snprintf(lcd_buf, sizeof(lcd_buf), "%.0f ", EngTrModel_Y.Gear);
			LCD_Put_Str( lcd_buf );
			
			// --- DISPLAY LINE 2 ---
			LCD_Set_Cursor( 2, 1 );
			
			// RPM
			LCD_Put_Str( "RPM: " );
			snprintf(lcd_buf, sizeof(lcd_buf), "%.1f  ", EngTrModel_Y.EngineSpeed);
			LCD_Put_Str( lcd_buf );

            /* -------------- UART Transmission of data to ESP8266 ------------- */
            char uart_buf[64]; // Buffer for transmission

            // Format and pack data into buffer
            uint16_t msg_len = snprintf(uart_buf, sizeof(uart_buf), "Thr: %.2f | RPM: %.1f | Gear: %.0f\r\n", 
                     EngTrModel_U.Throttle, 
                     EngTrModel_Y.EngineSpeed, 
                     EngTrModel_Y.Gear);

            USER_USART2_Transmit((uint8_t *)uart_buf, msg_len);

            // Reset flag
            model_updated = 0; 
        }

		// Debug
		// printf("Vehicle Speed: %f\r\n", EngTrModel_Y.VehicleSpeed);
		// printf("Engine Speed: %f\r\n", EngTrModel_Y.EngineSpeed);
		// printf("Gear: %f\r\n", EngTrModel_Y.Gear);
		// USER_Delay_40ms();
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
    USER_TIM2_Delay_10ms();

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

// void USER_Delay_10ms( void ){
// 	__asm(" 			ldr r0, =71111UL	");//	load the value to be used as delay count
// 	__asm(" again10:	sub r0, r0, #1		");//	decrement the delay count
// 	__asm("				cmp r0, #0			");//	check if the delay count has reached zero
// 	__asm("				bne again10			");//	if not, repeat the process
// 	__asm("				nop					");//	no operation (to ensure exact timing)
// }

// void USER_Delay_40ms( void ){
// 	__asm(" 			ldr r0, =284444UL	");//	load the value to be used as delay count
// 	__asm(" again40:	sub r0, r0, #1		");//	decrement the delay count
// 	__asm("				cmp r0, #0			");//	check if the delay count has reached zero
// 	__asm("				bne again40			");//	if not, repeat the process
// 	__asm("				nop					");//	no operation (to ensure exact timing)
// }