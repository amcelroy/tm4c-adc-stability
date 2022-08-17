#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "PLL.h"

void DisableInterrupts(void){
    asm("\t cpsid i\n");
}

void EnableInterrupts(void){
    asm("\t cpsie i\n");
}

int timeout() {
    int x = 10000;
    while(x > 0){
        x--;
    }
    return x;
}

void adc0_ss3_isr_handler() {
    ADC0_ISC_R |= 0x8; //Clear the interrupt, only SS3 is active
    GPIO_PORTC_DATA_R |= 0x40; // Toggle Port C6 high
    volatile uint16_t sample = ADC0_SSFIFO3_R;
    GPIO_PORTC_DATA_R &= ~0x40; //Toggle Port C6 low
}

void init_adc() {
    SYSCTL_RCGCADC_R |= 0x01;     // 1) activate ADC0
    volatile int x = timeout();

    ADC0_PC_R = 0x07;             // 2) configure for 1M samples/sec
    ADC0_SSPRI_R = 0x3210;        // 3) sequencer 0 is highest, sequencer 3 is lowest

    ADC0_ACTSS_R &= ~0x08;        // 5) disable sample sequencer 3
    ADC0_EMUX_R = (ADC0_EMUX_R&0xFFFF0FFF)+0x5000; // 6) timer trigger event
    ADC0_SSMUX3_R = 3;            // 7) PE0 is channel 3
    ADC0_SSCTL3_R = 0x06;         // 8) set flag and end
    ADC0_IM_R |= 0x08;            // 9) enable SS3 interrupts
    ADC0_SAC_R = 0x00;            // no averaging
    ADC0_ACTSS_R |= 0x08;         // 10) enable sample sequencer 3
    NVIC_PRI4_R = (NVIC_PRI4_R&0xFFFF00FF)|0x00002000; // 11)priority 1
    NVIC_EN0_R = 1<<17;           // 12) enable interrupt 17 in NVIC
}

void init_gpio() {
    SYSCTL_RCGCGPIO_R |= 0x10;    // Port E clock
    volatile int x = timeout();

    GPIO_PORTE_DIR_R &= ~0x01;    // make PE0 input
    GPIO_PORTE_AFSEL_R |= 0x01;   // enable alternate function on PE0
    GPIO_PORTE_DEN_R &= ~0x01;    // disable digital I/O on PE0
    GPIO_PORTE_AMSEL_R |= 0x01;   // enable analog functionality on PE0

    SYSCTL_RCGCGPIO_R |= 0x4;    // Port C clock
    x = timeout();

    GPIO_PORTC_DIR_R = 0xFF;    // make Port C output
    GPIO_PORTC_AFSEL_R = 0x0;   // disable alternate function on PE0
    GPIO_PORTC_DEN_R = 0xFF;    // enable digital I/O on PE0
    GPIO_PORTC_AMSEL_R = 0x01;    // disable analog functionality on PE0
}

void init_timer(uint32_t period){
    SYSCTL_RCGCTIMER_R |= 0x01;   // 4) activate timer0
    volatile int x = timeout();

    TIMER0_CTL_R = 0x00000000;    // disable timer0A during setup
    TIMER0_CTL_R |= 0x00000020;   // enable timer0A trigger to ADC
    TIMER0_CFG_R = 0;             // configure for 32-bit timer mode
    TIMER0_TAMR_R = 0x00000002;   // configure for periodic mode, default down-count settings
    TIMER0_TAPR_R = 0;            // prescale value for trigger
    TIMER0_TAILR_R = period-1;    // start value for trigger
    TIMER0_IMR_R = 0x00000000;    // disable all interrupts
}

int main(void)
{

    PLL_Init(80000000);
    int x = timeout();

    init_gpio();

    init_timer(124);   // <--- !!! Change me

    init_adc();

    TIMER0_CTL_R |= 0x00000001;   // enable timer0A 32-b, periodic, no interrupts
    ADC0_ACTSS_R = 0x8;             //Enable SS3

    EnableInterrupts();

    for(;;){

    }


	return 0;
}
