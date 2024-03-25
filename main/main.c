/*
 * LED blink with FreeRTOS
 */
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/adc.h"

#include <math.h>
#include <stdlib.h>

QueueHandle_t xQueueAdc;

typedef struct adc {
    int axis;
    int val;
} adc_t;

void x_task(void *p) {
    adc_init();
    adc_gpio_init(27);

    // 12-bit conversion, assume max value == ADC_VREF == 3.3 V
    const float conversion_factor = 3.3f / (1 << 12);

    uint16_t result;
    while (1) {
        adc_select_input(1); // Select ADC input 1 (GPIO27)
        result = adc_read();
        result = ((result * conversion_factor)-2047)/8;
        if (30 > result > - 30){
            result = 0;
        }
        printf("Valor em x: %f V\n", result);
        adc_t data;
        data.axis = 1;
        data.val = result;
        xQueueSend(xQueueAdc, &data, 100);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void y_task(void *p) {
    adc_init();
    adc_gpio_init(26);

    // 12-bit conversion, assume max value == ADC_VREF == 3.3 V
    const float conversion_factor = 3.3f / (1 << 12);

    uint16_t result;
    while (1) {
        adc_select_input(0); // Select ADC input 0 (GPIO26)
        result = adc_read();
        result = ((result * conversion_factor)-2047)/8;
        if (30 > result > - 30){
            result = 0;
        }
        printf("Valor em y: %f V\n", result);
        adc_t data;
        data.axis = 0;
        data.val = result;
        xQueueSend(xQueueAdc, &data, 100);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void uart_task(void *p) {
    adc_t data;

    while (1) {
        xQueueReceive(xQueueAdc, &data, portMAX_DELAY);
    }
}

int main() {
    stdio_init_all();

    xQueueAdc = xQueueCreate(32, sizeof(adc_t));

    xTaskCreate(uart_task, "uart_task", 4096, NULL, 1, NULL);
    xTaskCreate(x_task, "x_task", 4095, NULL, 1, NULL);
    xTaskCreate(y_task, "y_task", 4095, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
