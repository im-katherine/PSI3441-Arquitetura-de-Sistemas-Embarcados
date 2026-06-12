#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <stdlib.h>
#include <stdbool.h>

// =========================================================================
// CONFIGURAÇÃO MANUAL DO BOTÃO (Pode alterar para qualquer porta/pino aqui)
// =========================================================================
#define BUTTON_GPIO_NODE      DT_NODELABEL(gpiod) // gpiow, gpiob, gpioc, gpiod, gpioe
#define BUTTON_PIN            4                   // Número do pino na porta selecionada
// PTD4 correspondente ao conector J1, Pino 6 da FRDM-KL25Z

// LED On-board (Mantido via alias padrão da placa)
#define LED_C_NODE DT_ALIAS(led1)
static const struct gpio_dt_spec ledc = GPIO_DT_SPEC_GET(LED_C_NODE, gpios);

// Estruturas de controle para o botão manual
static const struct device *const button_dev = DEVICE_DT_GET(BUTTON_GPIO_NODE);
static struct gpio_callback button_cb_data;

// =========================================================================
// CONFIGURAÇÕES DO ADC
// =========================================================================
#define ADC_RESOLUTION      12
#define ADC_GAIN            ADC_GAIN_1
#define ADC_REFERENCE       ADC_REF_INTERNAL
#define ADC_ACQUISITION_TIME ADC_ACQ_TIME_DEFAULT
#define ADC_CHANNEL_ID      0
#define ADC_VREF_MV         3300

static int16_t sample_buffer;

// =========================================================================
// CONFIGURAÇÕES DO ACELERÔMETRO
// =========================================================================
static const struct device *const accel = DEVICE_DT_GET(DT_NODELABEL(mma8451q));

// =========================================================================
// VARIÁVEL DE CONTROLE DE MODO (false = ADC / true = Completo)
// =========================================================================
volatile bool modo_completo = false; 

// =========================================================================
// INTERRUPÇÃO DO BOTÃO
// =========================================================================
void button_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    // Alterna o estado do LED
    gpio_pin_toggle_dt(&ledc);
    
    // Alterna o modo de exibição
    modo_completo = !modo_completo;
    
    printk("\n========================================\n");
    if (modo_completo) {
        printk(">>> MODO ATUAL: COMPLETO (ADC + ACCEL) <<<\n");
    } else {
        printk(">>> MODO ATUAL: ADC (APENAS ADC) <<<\n");
    }
    printk("========================================\n\n");
}

// =========================================================================
// THREAD 1: LEITURA DO ADC (A cada 500 ms)
// =========================================================================
void thread_adc(void *arg1, void *arg2, void *arg3)
{
    const struct device *adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc0));
    if (!device_is_ready(adc_dev)) {
        printk("Erro: ADC não está pronto\n");
        return;
    }

    struct adc_channel_cfg channel_cfg = {
        .gain = ADC_GAIN,
        .reference = ADC_REFERENCE,
        .acquisition_time = ADC_ACQUISITION_TIME,
        .channel_id = ADC_CHANNEL_ID,
        .differential = 0,
    };

    if (adc_channel_setup(adc_dev, &channel_cfg) != 0) {
        printk("Erro ao configurar canal ADC\n");
        return;
    }

    struct adc_sequence sequence = {
        .channels    = BIT(ADC_CHANNEL_ID),
        .buffer      = &sample_buffer,
        .buffer_size = sizeof(sample_buffer),
        .resolution  = ADC_RESOLUTION,
    };

    while (1) {
        int err = adc_read(adc_dev, &sequence);
        if (err != 0) {
            printk("Falha na leitura do ADC: %d\n", err);
        } else {
            int32_t mv = sample_buffer;
            adc_raw_to_millivolts(ADC_VREF_MV, ADC_GAIN, ADC_RESOLUTION, &mv);
            printk("ADC: %d (raw), %d mV\n", sample_buffer, mv);
        }

        k_sleep(K_MSEC(500));
    }
}

// =========================================================================
// THREAD 2: LEITURA DO ACELERÔMETRO (A cada 1000 ms)
// =========================================================================
void thread_accel(void *arg1, void *arg2, void *arg3)
{
    struct sensor_value accel_x, accel_y, accel_z;
    int ret;
    uint32_t tempo_ms = 0;

    if (!device_is_ready(accel)) {
        printk("ERRO: Acelerometro nao esta pronto!\n");
        return;
    }

    k_sleep(K_MSEC(1000)); 

    while (1) {
        // Exibe apenas se estiver no Modo Completo
        if (modo_completo) {
            ret = sensor_sample_fetch(accel);
            if (ret) {
                printk("Erro ao ler sensor: %d\n", ret);
            } else {
                sensor_channel_get(accel, SENSOR_CHAN_ACCEL_X, &accel_x);
                sensor_channel_get(accel, SENSOR_CHAN_ACCEL_Y, &accel_y);
                sensor_channel_get(accel, SENSOR_CHAN_ACCEL_Z, &accel_z);

                printk("ACCEL -> T: %u, X: %d.%06d, Y: %d.%06d, Z: %d.%06d\n", 
                       tempo_ms,
                       accel_x.val1, abs(accel_x.val2),
                       accel_y.val1, abs(accel_y.val2),
                       accel_z.val1, abs(accel_z.val2));
            }
        }

        k_sleep(K_MSEC(1000));
        tempo_ms += 1000;
    }
}

// =========================================================================
// DEFINIÇÃO NATIVA DAS THREADS (Zephyr RTOS)
// =========================================================================
#define STACK_SIZE 1024
#define THREAD_PRIORITY 7

K_THREAD_DEFINE(adc_tid, STACK_SIZE, thread_adc, NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);
K_THREAD_DEFINE(accel_tid, STACK_SIZE, thread_accel, NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);

// =========================================================================
// FUNÇÃO MAIN (Configuração de Periféricos e Inicialização)
// =========================================================================
void main(void)
{
    printk("\n==================================================\n");
    printk("  FRDM-KL25Z - ADC + Acelerometro + Botao Interno \n");
    printk("==================================================\n");
    printk("Iniciando no MODO ADC (Apenas leituras ADC)\n");
    printk("Pressione o botao para alternar os modos.\n\n");

    // 1. Configurar LED de Status
    if (!gpio_is_ready_dt(&ledc)) {
        printk("Erro: LED de status nao esta pronto\n");
        return;
    }
    gpio_pin_configure_dt(&ledc, GPIO_OUTPUT_INACTIVE);

    // 2. Verificar se a porta selecionada para o botao está pronta
    if (!device_is_ready(button_dev)) {
        printk("Erro: Controlador GPIO selecionado nao esta pronto\n");
        return;
    }

    // 3. Configurar pino como entrada com Pull-up interno ativado
    int ret = gpio_pin_configure(button_dev, BUTTON_PIN, GPIO_INPUT | GPIO_PULL_UP);
    if (ret != 0) {
        printk("Erro ao configurar pino do botao: %d\n", ret);
        return;
    }
    
    // 4. Configurar interrupção por borda de descida (GND)
    ret = gpio_pin_interrupt_configure(button_dev, BUTTON_PIN, GPIO_INT_EDGE_FALLING);
    if (ret != 0) {
        printk("Erro ao configurar interrupcao: %d\n", ret);
        return;
    }

    // 5. Vincular a função ISR ao evento do pino
    gpio_init_callback(&button_cb_data, button_isr, BIT(BUTTON_PIN));
    gpio_add_callback(button_dev, &button_cb_data);
    
    printk("Configuracao concluida com sucesso!\n\n");

    // Coloca a Main em suspensão perpétua (O controle fica com as threads e ISR)
    while (1) {
        k_sleep(K_FOREVER);
    }
}