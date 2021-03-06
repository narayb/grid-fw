/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */
#ifndef ATMEL_START_PINS_H_INCLUDED
#define ATMEL_START_PINS_H_INCLUDED

#include <hal_gpio.h>

// SAMD51 has 14 pin functions

#define GPIO_PIN_FUNCTION_A 0
#define GPIO_PIN_FUNCTION_B 1
#define GPIO_PIN_FUNCTION_C 2
#define GPIO_PIN_FUNCTION_D 3
#define GPIO_PIN_FUNCTION_E 4
#define GPIO_PIN_FUNCTION_F 5
#define GPIO_PIN_FUNCTION_G 6
#define GPIO_PIN_FUNCTION_H 7
#define GPIO_PIN_FUNCTION_I 8
#define GPIO_PIN_FUNCTION_J 9
#define GPIO_PIN_FUNCTION_K 10
#define GPIO_PIN_FUNCTION_L 11
#define GPIO_PIN_FUNCTION_M 12
#define GPIO_PIN_FUNCTION_N 13

#define LED0 GPIO(GPIO_PORTA, 5)
#define PA07 GPIO(GPIO_PORTA, 7)
#define PA08 GPIO(GPIO_PORTA, 8)
#define PA09 GPIO(GPIO_PORTA, 9)
#define PA10 GPIO(GPIO_PORTA, 10)
#define PA11 GPIO(GPIO_PORTA, 11)
#define PA20 GPIO(GPIO_PORTA, 20)
#define PIN_UI_SPI_CS0 GPIO(GPIO_PORTA, 21)
#define PA22 GPIO(GPIO_PORTA, 22)
#define PA23 GPIO(GPIO_PORTA, 23)
#define PA24 GPIO(GPIO_PORTA, 24)
#define PA25 GPIO(GPIO_PORTA, 25)
#define PA30 GPIO(GPIO_PORTA, 30)
#define PIN_GRID_SYNC_2 GPIO(GPIO_PORTB, 7)
#define PB08 GPIO(GPIO_PORTB, 8)
#define PB09 GPIO(GPIO_PORTB, 9)
#define PB10 GPIO(GPIO_PORTB, 10)
#define PB11 GPIO(GPIO_PORTB, 11)
#define HWCFG_SHIFT GPIO(GPIO_PORTB, 13)
#define HWCFG_CLOCK GPIO(GPIO_PORTB, 14)
#define HWCFG_DATA GPIO(GPIO_PORTB, 15)
#define PB20 GPIO(GPIO_PORTB, 20)
#define PB21 GPIO(GPIO_PORTB, 21)
#define PB24 GPIO(GPIO_PORTB, 24)
#define PB25 GPIO(GPIO_PORTB, 25)
#define PB30 GPIO(GPIO_PORTB, 30)
#define PB31 GPIO(GPIO_PORTB, 31)
#define PC02 GPIO(GPIO_PORTC, 2)
#define MAP_MODE GPIO(GPIO_PORTC, 11)
#define PC12 GPIO(GPIO_PORTC, 12)
#define PC13 GPIO(GPIO_PORTC, 13)
#define UI_PWR_EN GPIO(GPIO_PORTC, 14)
#define PC16 GPIO(GPIO_PORTC, 16)
#define PC17 GPIO(GPIO_PORTC, 17)
#define PIN_GRID_SYNC_1 GPIO(GPIO_PORTC, 18)
#define MUX_A GPIO(GPIO_PORTC, 19)
#define MUX_B GPIO(GPIO_PORTC, 20)
#define MUX_C GPIO(GPIO_PORTC, 21)
#define PC27 GPIO(GPIO_PORTC, 27)
#define PC28 GPIO(GPIO_PORTC, 28)

#endif // ATMEL_START_PINS_H_INCLUDED
