#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/console/console.h>
#include <stdlib.h>
#include "pwm_z42.h"

// Define o topo do contador do PWM. 1000 garante uma boa precisão e 
// uma frequência alta o suficiente para não piscar aos olhos humanos.
#define TPM_MODULE 1000 

int main(void)
{
    // 1. Inicializa o hardware (Usando a biblioteca do professor)
    // Inicializa o TPM2 com clock da placa e prescaler de 128
    pwm_tpm_Init(TPM2, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);
    
    // Canal 0 (Pino PTB18 = LED Vermelho)
    pwm_tpm_Ch_Init(TPM2, 0, TPM_PWM_L, GPIOB, 18);
    // Canal 1 (Pino PTB19 = LED Verde)
    pwm_tpm_Ch_Init(TPM2, 1, TPM_PWM_L, GPIOB, 19);

    // Começa com os LEDs apagados (Duty cycle 0)
    pwm_tpm_CnV(TPM2, 0, 0);
    pwm_tpm_CnV(TPM2, 1, 0);

    // 2. Inicializa o terminal do Zephyr
    console_getline_init();
    printk("=== Controle de Cor: Laranja (Hibrido) ===\n");

    while (1) {
        printk("\nDigite a intensidade total (0 a 100%%): ");
        
        // Fica aguardando o usuário digitar no terminal
        char *input_str = console_getline();
        int intensidade = atoi(input_str);

        // Trava de segurança para valores fora do limite
        if (intensidade < 0) intensidade = 0;
        if (intensidade > 100) intensidade = 100;

        printk("Ajustando brilho global para: %d%%\n", intensidade);

        /* * 3. Cálculo de Duty Cycle para a cor Laranja:
         * O Laranja exige Vermelho no máximo (100% da intensidade pedida)
         * E o Verde em uma fração (aprox. 30% da intensidade pedida)
         */
        
        // Pulso do Vermelho
        uint16_t pulse_red = (TPM_MODULE * intensidade) / 100;
        
        // Pulso do Verde (30% do vermelho)
        uint16_t pulse_green = (TPM_MODULE * intensidade * 30) / (100 * 100);

        // Envia os valores fisicamente para os pinos
        pwm_tpm_CnV(TPM2, 0, pulse_red);
        pwm_tpm_CnV(TPM2, 1, pulse_green);
    }

    return 0;
}