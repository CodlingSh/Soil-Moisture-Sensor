/**
 * Soil Moisture Sensor
 * By Sheldon Codling
 * May 11, 2024
 **/

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"

#define gpio_pin 22

int main() {
	 
	// Init all GPIO pins and turn on ADC
	stdio_init_all();
	adc_init();
	adc_gpio_init(28);
	adc_select_input(2);

    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed");
        return -1;
    }

	// Set gpio pin for input
	gpio_set_dir(gpio_pin, 0);
	
	// Turn on LED to show board is on
    // cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

	while(1) {
		
		sleep_ms(1000);

		uint16_t result = adc_read();
		printf("value: %d \n", result);
		
//		while(gpio_get(gpio_pin)) {
//			cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
//			uint16_t result = adc_read();
//			printf("value: %d \n", result);
//		}
//		cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
	}
}

