#include "MKL25Z4.h"

#define ADC_MAX_VALUE 4095U
#define TH_VALUE     (ADC_MAX_VALUE * 80U / 100U)
#define TL_VALUE     (ADC_MAX_VALUE * 20U / 100U)

#define LED_BLUE_PIN  1
#define LED_GREEN_PIN 19
#define ADC_CHANNEL   8 

void config_leds(void);
void config_adc(void);
uint16_t read_adc(uint8_t channel);

int main(void)
{
    config_leds();
    config_adc();

    while(1)
    {
        uint16_t value_adc = read_adc(ADC_CHANNEL);

        if (value_adc >= TH_VALUE)
        {
            GPIOB->PSOR = (1 << LED_GREEN_PIN); 
            GPIOD->PCOR = (1 << LED_BLUE_PIN);  
        }
        else if (value_adc <= TL_VALUE)
        {
            GPIOB->PCOR = (1 << LED_GREEN_PIN); 
            GPIOD->PSOR = (1 << LED_BLUE_PIN);  
        }
        else
        {
            GPIOB->PSOR = (1 << LED_GREEN_PIN); 
            GPIOD->PSOR = (1 << LED_BLUE_PIN);  
        }
    }

    return 0;
}

void config_leds(void)
{
    SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK; 
    PORTD->PCR[LED_BLUE_PIN] = PORT_PCR_MUX(1);      
    GPIOD->PDDR |= 1 << LED_BLUE_PIN;    
    GPIOD->PSOR = 1 << LED_BLUE_PIN;    

    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK; 
    PORTB->PCR[LED_GREEN_PIN] = PORT_PCR_MUX(1);      
    GPIOB->PDDR |= 1 << LED_GREEN_PIN;   
    GPIOB->PSOR = 1 << LED_GREEN_PIN;   
}

void config_adc(void)
{
    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK; 
    PORTB->PCR[0] = PORT_PCR_MUX(0);      
    SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK;  
    
    ADC0->CFG1 = ADC_CFG1_ADIV(1) | ADC_CFG1_MODE(1) | ADC_CFG1_ADICLK(0);
    ADC0->CFG2 = ADC_CFG2_ADLSTS(3);
    ADC0->SC2 = 0;
    ADC0->SC3 = 0;
}

uint16_t read_adc(uint8_t channel)
{
    ADC0->SC1[0] = ADC_SC1_ADCH(channel);
    while(!(ADC0->SC1[0] & ADC_SC1_COCO_MASK));
    return ADC0->R[0];
}