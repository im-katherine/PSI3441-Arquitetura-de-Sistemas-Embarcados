#include <zephyr/kernel.h>

#include <zephyr/device.h>

#include <zephyr/drivers/gpio.h>


#define SLEEP_TIME_MS 1000


// Define o LED usando Device Tree

#define LED_GREEN_NODE  DT_ALIAS(led0)

#define LED_RED_NODE    DT_ALIAS(led2)


// Verifica se os LEDs estão definidos no Device Tree

#if DT_NODE_HAS_STATUS(LED_RED_NODE, okay) && DT_NODE_HAS_STATUS(LED_GREEN_NODE, okay)

    static const struct gpio_dt_spec led_red = GPIO_DT_SPEC_GET(LED_RED_NODE, gpios);

    static const struct gpio_dt_spec led_green = GPIO_DT_SPEC_GET(LED_GREEN_NODE, gpios);

#else

    #error "LED aliases não definidos corretamente"

#endif


// Máquina de estados

typedef enum {

    GREEN,

    YELLOW,

    RED

} traffic_states;


int main(void) {

    // Verifica se o device está pronto

    if (!gpio_is_ready_dt(&led_red)) {

        printk("Error: LED device %s is not ready\n", led_red.port->name);

        return 0;

    }

    else if (!gpio_is_ready_dt(&led_green)) {

        printk("Error: LED device %s is not ready\n", led_green.port->name);

        return 0;

    }

   

    // Configura o pino como saída

    if (gpio_pin_configure_dt(&led_red, GPIO_OUTPUT_ACTIVE) < 0) {

        printk("Erro ao configurar LED red\n");

    }  

    if (gpio_pin_configure_dt(&led_green, GPIO_OUTPUT_ACTIVE) < 0) {

        printk("Erro ao configurar LED green\n");

    }


    traffic_states state = GREEN;


    while (1) {

        switch (state) {

            case GREEN:

                gpio_pin_set_dt(&led_green, 1); // ativo low

                gpio_pin_set_dt(&led_red, 0);

                k_msleep(SLEEP_TIME_MS);

                state = YELLOW;

                break;


            case YELLOW:

                gpio_pin_set_dt(&led_green, 1);

                gpio_pin_set_dt(&led_red, 1);

                k_msleep(SLEEP_TIME_MS);

                state = RED;

                break;


            case RED:

                gpio_pin_set_dt(&led_green, 0);

                gpio_pin_set_dt(&led_red, 1);

                k_msleep(SLEEP_TIME_MS);

                state = GREEN;

                break;

        }

    }

    return 0;

}

