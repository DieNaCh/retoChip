#ifndef USER_TIMER_H_
#define USER_TIMER_H_

// 2 seconds
#define TIM2_PSC_PWM 0
#define TIM2_ARR_PWM 63999
#define TIM2_CCR1_PWM 16000

void USER_TIM2_Init( );
void USER_TIM2_Delay_2s( );

#endif /* USER_TIMER_H_ */