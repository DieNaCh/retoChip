/* **************** START *********************** */
/* Libraries, Definitions and Global Declarations */
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "main.h"
#include "FreeRTOS.h"
#include "portmacro.h"
#include "projdefs.h"
#include "user_uart.h"
#include "lcd.h"
#include "user_timer.h"
#include "user_adc.h"
#include "EngTrModel.h"
#include "rtwtypes.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"

#define BUTTON (GPIOA->IDR & ( 0x1UL << 4U ))
#define MAX_VEH_SPEED 134.0 // based on testing
#define CCR_STEP  (1.0 * TIM4_ARR_PWM) / MAX_VEH_SPEED
#define MIN_THROTTLE 1.45
#define MAX_THROTTLE 50.0
#define MIN_BRAKE_TORQUE 0.0
#define MAX_BRAKE_TORQUE 100.0
#define MIN_ADC_VALUE 8.0
#define MAX_ADC_VALUE 4100.0
#define MAX_WAIT_CYCLES 100000

#define MODEL_UPDATE_TASK_PERIOD 40
#define MODEL_INPUT_TASK_PERIOD 60
#define LCD_TASK_PERIOD 80
#define COMMUNICATION_TASK_PERIOD 120
#define MODEL_QUEUE_SIZE 4

// Gamma correction LUT for LEDs
const uint8_t gamma8[] = {
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2,
2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5,
5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10,
10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

TaskHandle_t CommunicationTaskHandle;
TaskHandle_t LCDTaskHandle;
TaskHandle_t ModelInputTaskHandle;
TaskHandle_t ModelUpdateTaskHandle;

QueueHandle_t xUARTQueue;
QueueHandle_t xLCDQueue;

typedef struct {
	// Inputs
	real_T Throttle;
	real_T BrakeToggle;

	// Outputs
	real_T EngineSpeed;
	real_T VehicleSpeed;
	real_T Gear;
} ModelData;

void USER_SystemClock_Config( void );
void StartTask1( void *pvParameters );

void CommunicationTask( void *pvParameters );
void LCDTask( void *pvParameters );
void ModelInputTask( void *pvParameters );
void ModelUpdateTask( void *pvParameters );

// Currently not necessary, reimplement if model update task priority changes!
// void TIM3_IRQHandler( void ) {
// 	if ( TIM3->SR & ( 0x1UL << 0U ) ) {
// 		ModelUpdateTask();
// 		TIM3->SR &= ~( 0x1UL << 0U );
// 		TIM3->CNT = TIM3_CNT_40MS;
// 	}
// }

// Linear interpolation between a and b
float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

// Function that converts vehicle velocity to LED brightness based on Gamma Correction
uint32_t vel_to_brightness(uint32_t vel) {
	uint32_t ccr_val = CCR_STEP * vel;

	// The CCR goes up to approximately 2^16, while the LUT holds up to 2^8
	// We shift right by 8 bits to read a somewhat accurate value in the LUT.
	// This value then needs to be scaled by 2^8, and fit within the ARR value.
	uint32_t corrected_brightness = gamma8[ccr_val >> 8U] << 8U;

	corrected_brightness = (corrected_brightness > TIM4_ARR_PWM ? TIM4_ARR_PWM : corrected_brightness);

	return corrected_brightness;
}

// Function that updates model data based on received values
void update_data(ModelData *source, ModelData *target) {
	target->BrakeToggle += source->BrakeToggle;
	target->EngineSpeed += source->EngineSpeed;
	target->VehicleSpeed += source->VehicleSpeed;
	target->Gear += source->Gear;
	target->Throttle += source->Throttle;
}

// Function that averages model data
void get_average(ModelData *data, uint8_t n) {
	data->BrakeToggle /= n;
	data->EngineSpeed /= n;
	data->VehicleSpeed /= n;
	data->Gear /= n;
	data->Throttle /= n;
}

// Function that gets and averages all model data available in a queue
// Only call when data has been confirmed to exist
void get_queue_data(ModelData *data, ModelData *retVal, QueueHandle_t queueHandle) {
	memset(data, 0, sizeof ( ModelData ));
	uint8_t takenValues = 0;

	// Take all and get average on all elements
	while (xQueueReceive(queueHandle, retVal, 0) == pdTRUE) {
		takenValues++;
		update_data(retVal, data);
	}

	/* DEBUG ONLY: print number of values from queue
		printf("used %u vals\r\n", takenValues);
	*/
	
	get_average(data, takenValues);
}

// Function that fetches remote control values, received through UART
void read_remote_control(uint8_t *accel, uint8_t *brake, uint8_t *remote_control) {
	// Prompt ESP32 to send data
	char req[] = "?\n"; 
	USER_USART1_Transmit((uint8_t *)req, 2);

	// static to have the values persist over function calls
	static char rx_buf[64];
	static uint8_t rx_idx = 0;

	uint32_t timeout = 0;

	while (timeout < MAX_WAIT_CYCLES) {
		if (USART1->SR & USART_SR_RXNE) {
			char rec = USART1->DR;

			// Endline character implies the message is done
			if (rec == '\n') {
				// Process message end and reset idx
				rx_buf[rx_idx] = '\0';

				int t_accel, t_brake, t_remote_control;

				uint8_t read_items = sscanf(rx_buf, "A,%d,B,%d,C,%d", &t_accel, &t_brake, &t_remote_control);

				if (read_items == 3) {
					*accel = (uint8_t)t_accel;
					*brake = (uint8_t)t_brake;
					*remote_control = (uint8_t)t_remote_control;

					printf("[RC_VALID] A:%d B:%d R:%d\r\n", t_accel, t_brake, t_remote_control);
				}
				else {
					printf("[RC_FAIL] Got: '%s'\r\n", rx_buf);
				}

				rx_idx = 0;
				return;
			}
			else {
				// We are not done yet
				if (rx_idx < sizeof(rx_buf) - 1) {
					rx_buf[rx_idx++] = rec;
				}
				else {
					// Data is trash, get rid of it
					rx_idx = 0;
					return;
				}
			}

			timeout = 0;
		}
		else {
			timeout++;
		}
	}
}

/* Superloop structure */
int main(void)
{
	/* Declarations and Initializations */
	HAL_Init( );
	USER_SystemClock_Config( );
	USER_GPIO_Init( );
	USER_TIM2_Init( );
	USER_TIM4_Init( );
    USER_ADC_Init( );
	USER_USART1_Init( );
    USER_UART2_Init( );
	LCD_Init( ); // MUST GO AFTER TIM2 INIT
	EngTrModel_initialize( );
	LCD_Clear( );

	// Uncomment these if interrupt is added back:
	// USER_TIM3_Init( );
	// USER_TIM3_Delay_40ms();

	/* Create a task with a priority of 0 (idle), 1 (belowNormal), 2 (Normal), 3 (High), 4 (VeryHigh) */
	xTaskCreate(CommunicationTask, "CommunicationTask", 512, NULL, 1, &CommunicationTaskHandle);
	xTaskCreate(LCDTask, "LCDTask", 512, NULL, 2, &LCDTaskHandle);
	xTaskCreate(ModelInputTask, "ModelInputTask", 256, NULL, 3, &ModelInputTaskHandle);
	xTaskCreate(ModelUpdateTask, "ModelUpdateTask", 128, NULL, 4, &ModelUpdateTaskHandle);

	xUARTQueue = xQueueCreate(MODEL_QUEUE_SIZE, sizeof ( ModelData ));
	xLCDQueue = xQueueCreate(MODEL_QUEUE_SIZE, sizeof ( ModelData ));

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

void CommunicationTask( void *pvParameters ) {
	// Only run if model has been updated
	ModelData retVal, UARTData;
	TickType_t xLastWakeTime = xTaskGetTickCount();

	for(;;) {
		/* -------------- UART Transmission of data to ESP32 ------------- */
		// Only fetch data if it's available
		if (xQueuePeek(xUARTQueue, &retVal, 0) == pdTRUE) {
			get_queue_data(&UARTData, &retVal, xUARTQueue);
			
			char uart_buf[64]; // Buffer for transmission

			// Format and pack data into buffer
			uint16_t msg_len = snprintf(uart_buf, sizeof(uart_buf), "T: %.2f | S: %.1f | R: %.1f | G: %.0f\r\n", 
						UARTData.Throttle, 
						UARTData.VehicleSpeed,
						UARTData.EngineSpeed, 
						UARTData.Gear);

			USER_USART1_Transmit((uint8_t *)uart_buf, msg_len);

			/* DEBUG ONLY: transmits buffer to terminal
				USER_USART2_Transmit((uint8_t *)uart_buf, msg_len);
			*/
		}

		/* DEBUG ONLY: Prints last wake time to terminal */
		// printf("[%lu] Comm\r\n", xLastWakeTime);
		vTaskDelayUntil(&xLastWakeTime, COMMUNICATION_TASK_PERIOD);
	}
}

void ModelUpdateTask( void *pvParameters ) {
	TickType_t xLastWakeTime = xTaskGetTickCount();

	for(;;) {
		// Step into the model
		EngTrModel_step();

		// Output PWM
		/* ---------------- Display velocity in LEDs ------------------- */
		uint32_t vel = round(EngTrModel_Y.VehicleSpeed); // Rounded vehicle speed
		uint32_t PWM = CCR_STEP * vel;

		TIM4->CCR1 = PWM;
		TIM4->CCR2 = PWM;
		TIM4->CCR3 = PWM;
		TIM4->CCR4 = PWM;

		// Write model data to queue
		ModelData updatedData = {
			.Throttle 		= EngTrModel_U.Throttle,
			.BrakeToggle 	= EngTrModel_U.BrakeTorque,
			
			.EngineSpeed 	= EngTrModel_Y.EngineSpeed,
			.VehicleSpeed 	= EngTrModel_Y.VehicleSpeed,
			.Gear 			= EngTrModel_Y.Gear,
		};

		xQueueSend(xUARTQueue, &updatedData, 0);
		xQueueSend(xLCDQueue, &updatedData, 0);

		/* DEBUG ONLY: Prints last wake time to terminal */
		// printf("[%lu] MU\r\n", xLastWakeTime);
		vTaskDelayUntil(&xLastWakeTime, MODEL_UPDATE_TASK_PERIOD);
	}
}

void LCDTask( void *pvParameters ) {
	TickType_t xLastWakeTime = xTaskGetTickCount();
	ModelData retVal, LCDData;

	for(;;) {
		// Only run if there's data to fetch
		if (xQueuePeek(xLCDQueue, &retVal, 0) == pdTRUE) {
			get_queue_data(&LCDData, &retVal, xLCDQueue);
			
			/* ---------------- Display data in LCD Display ------------------- */
			/* 	Display Format:
				
				T: xx.x  G: x
				R: xxxx S: xxx
			*/

			char lcd_buf[16]; // Buffer to hold data, using snprintf

			// --- DISPLAY LINE 1 ---
			LCD_Set_Cursor( 1, 1 );

			// We use extra spaces at the end for formatting

			// Throttle
			LCD_Put_Str( "T: " ); 
			snprintf(lcd_buf, sizeof(lcd_buf), "%.1f  ", LCDData.Throttle);
			LCD_Put_Str( lcd_buf ); 

			// Gear
			LCD_Put_Str( "G: " );
			snprintf(lcd_buf, sizeof(lcd_buf), "%.0f ", LCDData.Gear);
			LCD_Put_Str( lcd_buf );
			
			// --- DISPLAY LINE 2 ---
			LCD_Set_Cursor( 2, 1 );
			
			// RPM
			LCD_Put_Str( "R: " );
			snprintf(lcd_buf, sizeof(lcd_buf), "%.0f  ", LCDData.EngineSpeed);
			LCD_Put_Str( lcd_buf );

			// Speed
			LCD_Put_Str( "S: " );
			snprintf(lcd_buf, sizeof(lcd_buf), "%.0f  ", LCDData.VehicleSpeed);
			LCD_Put_Str( lcd_buf );
		}

		/* DEBUG ONLY: Prints last wake time to terminal */
		// printf("[%lu] LCD\r\n", xLastWakeTime);
		vTaskDelayUntil(&xLastWakeTime, LCD_TASK_PERIOD);
	}
}

void ModelInputTask( void *pvParameters ) {
	TickType_t xLastWakeTime = xTaskGetTickCount();
	bool button_pressed_previous_cycle = 0;
	uint8_t rx_accel = 0, rx_brake = 0, rx_remote = 0;

	for(;;) {
		/* --------------- Remote control through Grafana ------------------------ */
		read_remote_control(&rx_accel, &rx_brake, &rx_remote);
		
		// Only override if remote control is active
		if (rx_remote) {

			// Set brake/acceleration values based on received info
			if (rx_brake ){
				EngTrModel_U.Throttle = MIN_THROTTLE;
				EngTrModel_U.BrakeTorque = MAX_BRAKE_TORQUE;
			}
			else {
				EngTrModel_U.BrakeTorque = MIN_BRAKE_TORQUE;
				EngTrModel_U.Throttle = lerp(MIN_THROTTLE, MAX_THROTTLE, (float)rx_accel / 100);
			}
		}
		else {
			// If no remote control override, use physical inputs

			/* --------------- Throttle through potentiometer action ----------------- */
			if ((ADC1->SR & ( 0x1UL << 1U ))) {
				// Read the raw 12-bit ADC value
				uint32_t result = ADC1->DR;

				// Normalize result and update throttle and brake values
				float relative_result = ( (float)result - MIN_ADC_VALUE ) / ( MAX_ADC_VALUE - MIN_ADC_VALUE );
				
				EngTrModel_U.Throttle = lerp(MIN_THROTTLE, MAX_THROTTLE, relative_result);
			}

			/* --------------- Brake torque through push button ----------------- */
			if (!BUTTON) {
				// We need to debounce. We'd usually do this through a hardware delay with TIM2.
				// However, we can use the task's period itself as a debounce.
				if (button_pressed_previous_cycle) {
					EngTrModel_U.Throttle = MIN_THROTTLE;
					EngTrModel_U.BrakeTorque = MAX_BRAKE_TORQUE;
				}
				
				button_pressed_previous_cycle = 1;
			}
			else {
				EngTrModel_U.BrakeTorque = MIN_BRAKE_TORQUE;
				button_pressed_previous_cycle = 0;
			}
		}

		/* DEBUG ONLY: Prints last wake time to terminal */
		// printf("[%lu] MI\r\n", xLastWakeTime);
		vTaskDelayUntil(&xLastWakeTime, MODEL_INPUT_TASK_PERIOD);
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

void USER_ADC_Init( void ) {
    // Step 0a: Enable clock for ADC1
    RCC->APB2ENR |= ( 0x1UL << 9U );

    // Step 0b: Adjust ADC input clock
    RCC->CFGR |= ( 0x3UL << 14U );

    // Step 1: Select operation mode
    ADC1->CR1 &= ~( 0x3UL << 18U );
    ADC1->CR1 &= ~( 0x3UL << 16U );

    // Step 2: Determine the result format
    ADC1->CR2 &= ~( 0x1UL << 11U );
    ADC1->CR2 |= ( 0x1UL << 1U );

    // Step 3: Determine the sample time for the ADC conversion
    ADC1->SMPR2 &= ~( 0x7UL << 0U );

    // Step 4: Select the sequence and/or number of conversions for the ADC regular channels
    ADC1->SQR1 &= ~( 0xFUL << 20U );

    // Step 5: Select channel 0 for conversion
    ADC1->SQR3 &= ~( 0x1F << 0U );

    // Step 6: Enable the ADC module
    ADC1->CR2 |= ( 0x1UL << 0U );
    USER_TIM2_Delay_10ms();

    // Step 7: Calibration
    ADC1->CR2 |= ( 0x1UL << 2U );
    while (ADC1->CR2 & ( 0x1UL << 2U ));

    // Step 8: Start conversion
    ADC1->CR2 |= ( 0x1UL << 0U );
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