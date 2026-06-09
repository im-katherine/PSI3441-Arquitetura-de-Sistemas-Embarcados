#include <zephyr.h>
#include <sys/printk.h>
#include <stdio.h>

void main(void)
{
    int counter = 0;
    char *nome = "Zephyr";
    float pi = 3.14159;
    int valor = 255;

    while (1) {
        printk("=== printk() ===\n");
        printk("Contador: %d\n", counter);
        printk("Nome: %s, Endereço: %p\n", nome, nome);
        printk("Hex: 0x%x\n", valor);
#if defined(CONFIG_NEWLIB_LIBC_FLOAT_PRINTF)
        printk("Float (se suportado): %f\n", pi);
#else
        printk("Float não suportado em printk\n"); 		
#endif

       
        printf("Contador: %d\n", counter);
        printf("Nome: %s, Endereço: %p\n", nome, nome);
        printf("Hex: 0x%x\n", valor);
        printf("Float: %.2f\n", pi);

        counter++;
        k_sleep(K_SECONDS(2));
    }
}