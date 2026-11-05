#include "accelerometer.h"
#include "gnss.h"
#include "gyroscope.h"
#include "magnetometer.h"
#include "memory.h"
#include "radio.h"
#include "tusb.h"
#include <cstdint>
#include <hardware/gpio.h>
#include <hardware/pwm.h>
#include <hardware/uart.h>
#include <pico/stdio.h>

#include "hardware/clocks.h"

#define SPI0_MISO 20
#define SPI0_MOSI 23
#define SPI0_SCK 22

#define SPI1_MISO 8
#define SPI1_MOSI 11
#define SPI1_SCK 10

#define TX_PIN 18
#define RX_PIN 19

#define LED0_PIN 0
#define LED1_PIN 1

#define PWM4_PIN 29
#define PWM3_PIN 28
#define PWM2_PIN 27
#define PWM1_PIN 26

class Board {
public:
  void init_cs_pin(uint8_t pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, 1);
  }
  void init_pwm_pin(uint8_t pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    gpio_set_pulls(pin, false, true);
    uint slice = pwm_gpio_to_slice_num(pin);
    pwm_config cfg = pwm_get_default_config();

    pwm_config_set_clkdiv(&cfg, 45.8125);
    pwm_config_set_wrap(&cfg, 65483.0f);

    pwm_init(slice, &cfg, true);
    pwm_set_gpio_level(pin, 0);
  }
  void set_pwm_value(uint8_t pin, float value) {

    if (value > 1.0f)
      value = 1.0f;
    else if (value < 0)
      value = 0;

    pwm_set_gpio_level(pin, static_cast<uint16_t>((65484.0f * value)));
  }
  void init() {
    stdio_init_all();

    spi_init(spi0, 1 * 1000 * 1000);

    gpio_set_function(SPI0_MISO, GPIO_FUNC_SPI);
    gpio_set_function(SPI0_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI0_MOSI, GPIO_FUNC_SPI);

    spi_init(spi1, 1 * 1000 * 1000);
    gpio_set_function(SPI1_MISO, GPIO_FUNC_SPI);
    gpio_set_function(SPI1_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI1_MOSI, GPIO_FUNC_SPI);

    uart_init(uart0, 9600);
    gpio_set_function(TX_PIN, (gpio_function_t)11);
    gpio_set_function(RX_PIN, (gpio_function_t)11);

    gpio_set_dir(TX_PIN, GPIO_OUT);
    gpio_set_dir(RX_PIN, GPIO_IN);

    init_cs_pin(GYRO_CS);
    init_cs_pin(ACC_CS);
    init_cs_pin(FLASH_CS);
    init_cs_pin(RADIO_CS);
    init_cs_pin(MAG_CS);

    gpio_init(LED1_PIN);
    gpio_set_dir(LED1_PIN, GPIO_OUT);

    gpio_init(LED0_PIN);
    gpio_set_dir(LED0_PIN, GPIO_OUT);

    accelerometer.init();
    gyroscope.init();
    magnetometer.init();
    //  TODO: change data length
    radio.init(433000000ULL, 14, 7, 125, 64);

    init_pwm_pin(PWM1_PIN);
    init_pwm_pin(PWM2_PIN);
    init_pwm_pin(PWM3_PIN);
    init_pwm_pin(PWM4_PIN);
  }

  Accelerometer accelerometer;
  Gyroscope gyroscope;
  Memory memory;
  Radio radio;
  Magnetometer magnetometer;
  GNSS gnss;

private:
};
