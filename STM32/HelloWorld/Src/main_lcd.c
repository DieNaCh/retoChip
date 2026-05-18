#include <stdint.h>
#include "main.h"
#include "user_timer.h"
#include "lcd.h"

int main(void)
{
	USER_SystemClock_Config( ); // 				configure the system clock to 64 MHz
	USER_TIM2_Init( );
	uint8_t col = 16;
 	LCD_Init( );
    for(;;){
    	LCD_Clear( );
    	LCD_Set_Cursor( 1, 1 );
    	LCD_Put_Str( "TE" );
    	LCD_Put_Num( 2003 );
    	LCD_Put_Char( 'B' );
    	LCD_Put_Str( " SoC" );
    	LCD_Set_Cursor( 2, col-- );
    	LCD_Put_Str( "Prueba de LCD ");
    	LCD_BarGraphic( 0, 64 );
	USER_TIM2_Delay_200ms( );// 200ms
    	if( col == 0 ){
    		USER_TIM2_Delay_500ms( );// 500ms
    		col = 16;
    	}
    }
}