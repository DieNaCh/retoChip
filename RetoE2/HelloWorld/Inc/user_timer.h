#ifndef USER_TIMER_H_
#define USER_TIMER_H_

// Timer constants
// Formulas:
// PSC = ceil(time in seconds / ( period of oscillator * (( ARR + 1 )))) - 1
// CNT = (ARR + 1) - time in seconds / ( period of oscillator * ( PSC + 1 ))
#define TIM2_PSC_10US 0
#define TIM2_CNT_10US 64896

#define TIM2_PSC_53US 0
#define TIM2_CNT_53US 62144

#define TIM2_PSC_100US 0
#define TIM2_CNT_100US 59136

#define TIM2_PSC_1MS 0
#define TIM2_CNT_1MS 1536

#define TIM2_PSC_4_1MS 4
#define TIM2_CNT_4_1MS 13056

#define TIM2_PSC_10MS 9
#define TIM2_CNT_10MS 1536

#define TIM2_PSC_40MS 39
#define TIM2_CNT_40MS 1536

#define TIM2_PSC_100MS 97
#define TIM2_CNT_100MS 230

#define TIM2_PSC_200MS 195
#define TIM2_CNT_200MS 230

#define TIM2_PSC_500MS 488
#define TIM2_CNT_500MS 96

#define TIM2_PSC_1S 976
#define TIM2_CNT_1S 29

#define TIM2_PSC_2S 1953
#define TIM2_CNT_2S 29

#define TIM3_PSC_40MS 39
#define TIM3_CNT_40MS 1536

// PWM constants
#define TIM4_PSC_PWM 0
#define TIM4_ARR_PWM 63999

void USER_TIM2_Init( void );
void USER_TIM2_Delay_10us( void );
void USER_TIM2_Delay_53us( void );
void USER_TIM2_Delay_100us( void );
void USER_TIM2_Delay_1ms( void );
void USER_TIM2_Delay_4_1ms( void );
void USER_TIM2_Delay_10ms( void );
void USER_TIM2_Delay_40ms( void );
void USER_TIM2_Delay_100ms( void );
void USER_TIM2_Delay_200ms( void );
void USER_TIM2_Delay_500ms( void );
void USER_TIM2_Delay_1sec( void );
void USER_TIM2_Delay_2sec( void );

void USER_TIM3_Init( void );
void USER_TIM3_Delay_40ms( void );

void USER_TIM4_Init( void );

#endif /* USER_TIMER_H_ */