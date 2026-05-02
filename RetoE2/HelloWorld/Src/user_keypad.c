#include <stdint.h>
#include "main.h"
#include "user_keypad.h"

void USER_Keypad_Init( void ){
    // Enable clock
    RCC->APB2ENR |= ( 0x1UL << 3U ); 
    
    // Rows are output push-pull
    // CNF[1:0] = 00
    // MODE[1:0] = 01
    // ODR = 0 or 1

    // PB0
    GPIOB->ODR  &=      ~( 0x1UL <<  0U );
    GPIOB->CRL  &=      ~( 0x3UL <<  2U )
                &       ~( 0x2UL <<  0U );
    GPIOB->CRL  |=       ( 0x1UL <<  0U );
    
    // PB1
    GPIOB->ODR  &=      ~( 0x1UL <<  1U );
    GPIOB->CRL  &=      ~( 0x3UL <<  6U )
                &       ~( 0x2UL <<  4U );
    GPIOB->CRL  |=       ( 0x1UL <<  4U );

    // PB2
    GPIOB->ODR  &=      ~( 0x1UL <<  2U );
    GPIOB->CRL  &=      ~( 0x3UL << 10U )
                &       ~( 0x2UL <<  8U );
    GPIOB->CRL  |=       ( 0x1UL <<  8U );
    
    // PB13
    GPIOB->ODR  &=      ~( 0x1UL << 13U );
    GPIOB->CRH  &=      ~( 0x3UL << 22U )
                &       ~( 0x2UL << 20U );
    GPIOB->CRH  |=       ( 0x1UL << 20U );

    // Columns are input pull-up
    // CNF[1:0] = 10
    // MODE[1:0] = 00
    // ODR = 1

    // PB4
    GPIOB->ODR  |=       ( 0x1UL <<  4U );
    GPIOB->CRL  &=      ~( 0x1UL << 18U )
                &       ~( 0x3UL << 16U );
    GPIOB->CRL  |=       ( 0x2UL << 18U );
    
    // PB5
    GPIOB->ODR  |=       ( 0x1UL <<  5U );
    GPIOB->CRL  &=      ~( 0x1UL << 22U )
                &       ~( 0x3UL << 20U );
    GPIOB->CRL  |=       ( 0x2UL << 22U );
    
    // PB6
    GPIOB->ODR  |=       ( 0x1UL <<  6U );
    GPIOB->CRL  &=      ~( 0x1UL << 26U )
                &       ~( 0x3UL << 24U );
    GPIOB->CRL  |=       ( 0x2UL << 26U );
    
    // PB7
    GPIOB->ODR  |=       ( 0x1UL <<  7U );
    GPIOB->CRL  &=      ~( 0x1UL << 30U )
                &       ~( 0x3UL << 28U );
    GPIOB->CRL  |=       ( 0x2UL << 30U );

    // LEDs, from most significant bit to least significant bit
	// As push-pull output
	// CNF[1:0] = 00
    // MODE[1:0] = 01
    // ODR = 0 or 1

	// PB8
	GPIOB->ODR &=		~( 0x1UL <<  8U );
	GPIOB->CRH &= 		~( 0x3UL <<  2U )
				&		~( 0x2UL <<  0U );
	GPIOB->CRH |=		 ( 0x1UL <<  0U );

	// PB9
	GPIOB->ODR &=		~( 0x1UL <<  9U );
	GPIOB->CRH &= 		~( 0x3UL <<  6U )
				&		~( 0x2UL <<  4U );
	GPIOB->CRH |=		 ( 0x1UL <<  4U );

	// PB10
	GPIOB->ODR &=		~( 0x1UL << 10U );
	GPIOB->CRH &= 		~( 0x3UL << 10U )
				&		~( 0x2UL <<  8U );
	GPIOB->CRH |=		 ( 0x1UL <<  8U );

	// PB11
	GPIOB->ODR &=		~( 0x1UL << 11U );
	GPIOB->CRH &= 		~( 0x3UL << 14U )
				&		~( 0x2UL << 12U );
	GPIOB->CRH |=		 ( 0x1UL << 12U );
}

// Row and column physical address arrays
Pin rows[4] = { 
    { GPIOB, ( 0x1UL <<   0U ) }, 
    { GPIOB, ( 0x1UL <<   1U ) },
    { GPIOB, ( 0x1UL <<   2U ) },
    { GPIOB, ( 0x1UL <<  13U ) }
};

Pin cols[4] = { 
    { GPIOB, ( 0x1UL <<  4U ) }, 
    { GPIOB, ( 0x1UL <<  5U ) },
    { GPIOB, ( 0x1UL <<  6U ) },
    { GPIOB, ( 0x1UL <<  7U ) }
};

uint8_t mapping[4][4] = {
    // Mapped * to 0xEU
    // Mapped # to 0xFU
    {0x1U, 0x2U, 0x3U, 0xAU},
    {0x4U, 0x5U, 0x6U, 0xBU},
    {0x7U, 0x8U, 0x9U, 0xCU},
    {0xEU, 0x0U, 0xFU, 0xDU}
};

uint8_t USER_Key( void ){
    // Sweep through the matrix
    for (uint8_t i = 0; i < 4; i++) {
        
        // Pull all rows high
        for (uint8_t j = 0; j < 4; j++) {
            rows[j].port->ODR |= rows[j].mask;
        }

        // Pull current row low
        rows[i].port->ODR &= ~rows[i].mask;

        for (uint8_t j = 0; j < 4; j++) {
            // Check if button is pressed
            if ( !(cols[j].port->IDR & cols[j].mask) ) {
                return mapping[i][j];
            }
        }
    }

    return 0xFFU; // Return FF if nothing is pressed
}