#include <driverlib.h>
#include "reciever_pipeline.h"

//For the clk
#define OUTPUT_PIN_PORT GPIO_PORT_P2
#define OUTPUT_PIN GPIO_PIN0
#define COMPARE_VALUE_500kHZ 48 //Didn't prescalar the clk so 24MHz/48 = 500kHz
unsigned int ADC_Result;

void setup_pwm_output(void){
    WDTCTL = WDTPW | WDTHOLD;                 // Stop WDT

    //Datasheet pg 101
    P5DIR |= BIT0 | BIT1;                     // P1.6 and P1.7 output 01
    P5SEL0 |= BIT0 | BIT1;                    // P1.6 and P1.7 options select

    PM5CTL0 &= ~LOCKLPM5;

    TB2CCR0 = 1000-1;                         // PWM Period
    TB2CCTL1 = OUTMOD_7;                      // CCR1 reset/set
    TB2CCR1 = 750;                            // CCR1 PWM duty cycle
    TB2CCTL2 = OUTMOD_7;                      // CCR2 reset/set
    TB2CCR2 = 250;                            // CCR2 PWM duty cycle
    TB2CTL = TBSSEL__SMCLK | MC__UP | TBCLR;  // SMCLK, up mode, clear TBR
}

void setup_pwm_2(void){
    WDTCTL = WDTPW | WDTHOLD;                 // Stop WDT

    P1DIR |= BIT6 | BIT7;                     // P1.6 and P1.7 output
    P1SEL1 |= BIT6 | BIT7;                    // P1.6 and P1.7 options select

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    TB0CCR0 = 1000-1;                         // PWM Period
    TB0CCTL1 = OUTMOD_7;                      // CCR1 reset/set
    TB0CCR1 = 750;                            // CCR1 PWM duty cycle
    TB0CCTL2 = OUTMOD_7;                      // CCR2 reset/set
    TB0CCR2 = 250;                            // CCR2 PWM duty cycle
    TB0CTL = TBSSEL__SMCLK | MC__UP | TBCLR;  // SMCLK, up mode, clear TBR

}
void config_adc(void){
    // Configure ADC12
        ADCCTL0 |= ADCSHT_2 | ADCON;                             // ADCON, S&H=16 ADC clks
        ADCCTL1 |= ADCSHP;                                       // ADCCLK = MODOSC; sampling timer
        ADCCTL2 &= ~ADCRES;                                      // clear ADCRES in ADCCTL
        ADCCTL2 |= ADCRES_2;                                     // 12-bit conversion results
        ADCMCTL0 |= ADCINCH_1;                                   // A1 ADC input select; Vref=AVCC
        ADCIE |= ADCIE0;                                         // Enable ADC conv complete interrupt

        while(1)
        {
            ADCCTL0 |= ADCENC | ADCSC;                           // Sampling and conversion start
            __bis_SR_register(LPM0_bits | GIE);                  // LPM0, ADC_ISR will force exit
            __no_operation();                                    // For debug only
            if (ADC_Result < 0x7FF)
                P1OUT &= ~BIT0;                                  // Clear P1.0 LED off
            else
                P1OUT |= BIT0;                                   // Set P1.0 LED on
            __delay_cycles(5000);
        }
}
int main(void) {

    WDT_A_hold(WDT_A_BASE);
    setup_pwm_output();

    while(1){

    }
}


// ADC interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
    #pragma vector=ADC_VECTOR
    __interrupt void ADC_ISR(void)
#elif defined(__GNUC__)
    void __attribute__ ((interrupt(ADC_VECTOR))) ADC_ISR (void)
#else
    #error Compiler not supported!
#endif
    {
        switch(__even_in_range(ADCIV,ADCIV_ADCIFG))
        {
            case ADCIV_NONE:
                break;
            case ADCIV_ADCOVIFG:
                break;
            case ADCIV_ADCTOVIFG:
                break;
            case ADCIV_ADCHIIFG:
                break;
            case ADCIV_ADCLOIFG:
                break;
            case ADCIV_ADCINIFG:
                break;
            case ADCIV_ADCIFG:
                ADC_Result = ADCMEM0;
                __bic_SR_register_on_exit(LPM0_bits);            // Clear CPUOFF bit from LPM0
                break;
            default:
                break;
        }
    }