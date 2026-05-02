#ifndef USER_UART_H_
#define USER_UART_H_

#define USARTDIV        0x022B//				    115200 baud rate at 64 MHz
#define USART_CR1_UE    ( 0x1UL << 13U )
#define USART_CR1_M     ( 0x1UL << 12U )
#define USART_CR1_TE    ( 0x1UL <<  3U )
#define USART_CR1_RE    ( 0x1UL <<  2U )
#define USART_CR2_STOP  ( 0x3UL << 12U )
#define USART_SR_TXE    ( 0x1UL <<  7U )
#define USART_SR_RXNE   ( 0x1UL <<  5U )

void USER_USART2_Init( void );
void USER_USART2_Transmit( uint8_t *pData, uint16_t size );
uint8_t USER_UART2_Receive_8bit( void );

#endif /* USER_UART_H_ */