#include <stdint.h>
#include "main.h"
#include "user_uart.h"

/* ----------------- USART 1 ----------------- */
static void USER_USART1_Send_8bit( uint8_t Data );

void USER_USART1_Init( void ){
	RCC->APB2ENR	|=	 ( 0x1UL << 14U );//	USART 1 clock enable	
	USART1->CR1		|=	 USART_CR1_UE;//		Step 1 Usart enabled
	USART1->CR1		&=	~USART_CR1_M;//			Step 2 8 Data bits
	USART1->CR2		&=	~USART_CR2_STOP;//		Step 3 1 Stop bit
	USART1->BRR	 	|=	 USARTDIV;//			Step 5 Desired baud rate
	USART1->CR1		|= 	 USART_CR1_TE;//		Step 6 Transmitter enabled
	//pin PA9 (USART1_TX) as alternate function output push-pull, max speed 10MHz
	GPIOA->CRH		&=	~( 0x1UL <<  6U ) &	~( 0x2UL <<  4U );
	GPIOA->CRH		|=	 ( 0x2UL <<  6U ) |	 ( 0x1UL <<  4U );
	//pin PA10 (USART1_RX) as alternate function output push-pull, max speed 10MHz
	GPIOA->CRH		&=	~( 0x3UL <<  10U ) &	~( 0x3UL <<  8U );
	GPIOA->CRH		|=	 ( 0x1UL <<  10U );
	USART1->CR1		|= 	 USART_CR1_RE;//		Reception enabled
}

void USER_USART1_Transmit( uint8_t *pData, uint16_t size ){
	for( int i = 0; i < size; i++ ){
		USER_USART1_Send_8bit( *pData++ );
	}
}

static void USER_USART1_Send_8bit( uint8_t Data ){
	while(!( USART1->SR & USART_SR_TXE ));//	wait until next data can be written
	USART1->DR = Data;//				Step 7 Data to send
}

uint8_t USER_UART1_Receive_8bit( void ){                           
	// wait until a data is received (ISR register)
	while(!( USART1->SR & USART_SR_RXNE ));
	// return data (RDR register)
	return USART1->DR;
}

/* ----------------- USART 2 ----------------- */
static void USER_USART2_Send_8bit( uint8_t Data );

void USER_USART2_Init( void ){
	RCC->APB1ENR	|=	 ( 0x1UL << 17U );//	USART 2 clock enable	
	USART2->CR1		|=	 USART_CR1_UE;//		Step 1 Usart enabled
	USART2->CR1		&=	~USART_CR1_M;//			Step 2 8 Data bits
	USART2->CR2		&=	~USART_CR2_STOP;//		Step 3 1 Stop bit
	USART2->BRR	 	|=	 USARTDIV;//			Step 5 Desired baud rate
	USART2->CR1		|= 	 USART_CR1_TE;//		Step 6 Transmitter enabled
	//pin PA2 (USART2_TX) as alternate function output push-pull, max speed 10MHz
	GPIOA->CRL		&=	~( 0x1UL <<  10U ) &	~( 0x2UL <<  8U );
	GPIOA->CRL		|=	 ( 0x2UL <<  10U ) |	 ( 0x1UL <<  8U );
	//pin PA3 (USART2_RX) as alternate function output push-pull, max speed 10MHz
	GPIOA->CRL		&=	~( 0x3UL <<  14U ) &	~( 0x3UL <<  12U );
	GPIOA->CRL		|=	 ( 0x1UL <<  14U );
	USART2->CR1		|= 	 USART_CR1_RE;//		Reception enabled
}

void USER_USART2_Transmit( uint8_t *pData, uint16_t size ){
	for( int i = 0; i < size; i++ ){
		USER_USART2_Send_8bit( *pData++ );
	}
}

static void USER_USART2_Send_8bit( uint8_t Data ){
	while(!( USART2->SR & USART_SR_TXE ));//	wait until next data can be written
	USART2->DR = Data;//				Step 7 Data to send
}

uint8_t USER_UART2_Receive_8bit( void ){                           
	// wait until a data is received (ISR register)
	while(!( USART2->SR & USART_SR_RXNE ));
	// return data (RDR register)
	return USART2->DR;
}