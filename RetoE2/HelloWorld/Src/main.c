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
#define BUTTON (GPIOA->IDR & ( 0x1UL << 4U ))
#define CCR_STEP  426 // Assumes max speed of 150
#define MIN_THROTTLE 1.45
#define MAX_THROTTLE 50.0
#define MIN_BRAKE_TORQUE 0.0
#define MAX_BRAKE_TORQUE 100.0
#define MIN_ADC_VALUE 10.0
#define MAX_ADC_VALUE 4100.0

bool model_updated = false;

void TIM3_IRQHandler( void ) {
	if ( TIM3->SR & ( 0x1UL << 0U ) ) {
		EngTrModel_step();
		model_updated = true;
		TIM3->SR &= ~( 0x1UL << 0U );
		TIM3->CNT = TIM3_CNT_40MS;
	}
}

// Linear interpolation between a and b
float lerp(float a, float b, float t) {
    return a + t * (b - a);
}


int main(void){
    USER_SystemClock_Config();
    USER_GPIO_Init();
	USER_TIM2_Init( );
	USER_TIM3_Init( );
	USER_TIM4_Init( );
    USER_ADC_Init();
    USER_USART2_Init();
	LCD_Init(); // MUST GO AFTER TIM2 INIT
	EngTrModel_initialize();
	USER_TIM3_Delay_40ms();
	LCD_Clear( );
	
    /* Repetitive block */
    for(;;){
		/* --------------- Throttle through potentiometer action ----------------- */
		if ((ADC->SR & ( 0x1UL << 1U ))) {
            // Read the raw 12-bit ADC value
            uint32_t result = ADC->DR;

			// Normalize result and update throttle and brake values
			float relative_result = ( (float)result - MIN_ADC_VALUE ) / ( MAX_ADC_VALUE - MIN_ADC_VALUE );
			
			EngTrModel_U.Throttle = lerp(MIN_THROTTLE, MAX_THROTTLE, relative_result);
			EngTrModel_U.BrakeTorque = lerp(MIN_BRAKE_TORQUE, MAX_BRAKE_TORQUE, relative_result);
        }

		if (model_updated == 1) {
			/* ---------------- Display velocity in LEDs ------------------- */
			uint32_t vel = EngTrModel_Y.VehicleSpeed; // Rounded vehicle speed
			uint32_t ccr_val = CCR_STEP * vel;

			TIM4->CCR1 = ccr_val;
			TIM4->CCR2 = ccr_val;
			TIM4->CCR3 = ccr_val;
			TIM4->CCR4 = ccr_val;
			
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
	RCC->APB2ENR	|=	 ( 0x1UL <<  3U );//	IO port B clock enable

	// PA0 as analog input
	GPIOA->CRL      &=  ~( 0x3UL << 0U );
    GPIOA->CRL      &=  ~( 0x3UL << 2U );

	// PB6 as alternate-function push-pull
	GPIOB->CRL      &=  ~( 0x1UL << 26U ); 
    GPIOB->CRL      |=   ( 0x2UL << 26U ) | ( 0x3UL << 24U );

	// PB7 as alternate-function push-pull
	GPIOB->CRL      &=  ~( 0x1UL << 30U ); 
    GPIOB->CRL      |=   ( 0x2UL << 30U ) | ( 0x3UL << 28U );
	
	// PB8 as alternate-function push-pull
	GPIOB->CRH      &=  ~( 0x1UL << 2U ); 
    GPIOB->CRH      |=   ( 0x2UL << 2U ) | ( 0x3UL << 0U );
	
	// PB9 as alternate-function push-pull
	GPIOB->CRH      &=  ~( 0x1UL << 6U ); 
    GPIOB->CRH      |=   ( 0x2UL << 6U ) | ( 0x3UL << 4U );
	
	// PA4 as input pull up
	GPIOA->ODR 		|= 	 ( 0x1UL << 4U );
	GPIOA->CRL 		&=	~( 0x1UL << 18U );
	GPIOA->CRL 		&=	~( 0x3UL << 16U );
	GPIOA->CRL		|= 	 ( 0x2UL << 18U );
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