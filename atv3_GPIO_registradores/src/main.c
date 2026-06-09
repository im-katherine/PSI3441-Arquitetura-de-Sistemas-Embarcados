#include <stdint.h>
/* ── Definição dos endereços dos registradores ── */
/* SIM: habilitar clock das portas */
#define SIM_SCGC5 (*((volatile unsigned int *)0x40048038))
/* PORTB: Pin Control Register do pino 19 */
#define PORTB_PCR19 (*((volatile unsigned int *)0x4004A04C))
/* GPIO Port B */
#define GPIOB_PDDR (*((volatile unsigned int *)0x400FF054)) /* Data
Direction */
#define GPIOB_PSOR (*((volatile unsigned int *)0x400FF044)) /* Set
Output */
#define GPIOB_PCOR (*((volatile unsigned int *)0x400FF048)) /* Clear
Output */

/* ── Protótipo ── */
void delayMs(int n);
int main(void) {
    /* (1) Habilitar clock da Porta B (bit 10 de SIM_SCGC5) */
    SIM_SCGC5 |= (1 << 10);
    /* (2) Configurar Pino 19 como GPIO (MUX = 001 → bits 10:8) */
    PORTB_PCR19 = 0x00000100;
    /* (3) Setar direção do pino como saída (bit 19 do PDDR) */
    GPIOB_PDDR |= (1 << 19);
    /* (4)-(7) Loop infinito: LED verde pisca com período de 2 s */
    while (1) {
        GPIOB_PCOR = (1 << 19);
        delayMs(1000);
        GPIOB_PSOR = (1 << 19);
        delayMs(1000);
    }
    return 0;
}
    /*Função de espera*/
    void delayMs(int n) {
        volatile int i;
        volatile int j;
        for (i = 0; i < n; i++)
            for (j = 0; j < 7000; j++) { /* busy-wait */ }
    }
