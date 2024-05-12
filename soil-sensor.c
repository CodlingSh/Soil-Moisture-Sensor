/**
 * Soil Moisture Sensor
 * By Sheldon Codling
 * May 11, 2024
 **/

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/cyw43_arch.h"

#define gpio_pin 22

int main() {
	 
	// Init all GPIO pins
	stdio_init_all();
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed");
        return -1;
    }

	// Set gpio pin for input
	gpio_set_dir(gpio_pin, 0);
	
	// Turn on LED to show board is on
    // cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

	while(1) {

		while(gpio_get(gpio_pin)) {
			cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
		}
		cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
	}

}

