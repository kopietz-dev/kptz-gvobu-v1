#include "pico/stdlib.h"
#include <hardware/gpio.h>
#include <hardware/spi.h>
#include <pico/time.h>
#include <stdint.h>

#define ACC_CS 21

class Accelerometer {
public:
  void writeReg(uint8_t reg, uint8_t value) {
    gpio_put(ACC_CS, 0);
    spi_write_blocking(spi0, &reg, 1);
    spi_write_blocking(spi0, &value, 1);
    gpio_put(ACC_CS, 1);
  }

  void init() {
    // Soft reset
    writeReg(0x7E, 0xB6);
    sleep_ms(50);

    // Perform dummy read to switch accelerometer to SPI mode
    getID();
    sleep_ms(50);

    writeReg(0x7D, 0x04); // ACC_PWR_CTRL: enable accelerometer
    sleep_ms(5);

    writeReg(0x7C, 0x00); // ACC_PWR_CONF: active mode
    sleep_ms(10);

    writeReg(0x40, 0xA8); // ACC_CONF: ODR and bandwidth
    sleep_ms(2);

    writeReg(0x41, 0x02); // +-12g

    sleep_ms(50);
  }

  uint8_t getID() {
    uint8_t tx[3] = {0x00 | 0x80, 0x00, 0x00};
    uint8_t rx[3];

    gpio_put(ACC_CS, 0);
    spi_write_read_blocking(spi0, tx, rx, 3);
    gpio_put(ACC_CS, 1);

    return rx[2];
  }

  void readAxis(float *result) {
    constexpr static uint8_t tx[8] = {0x12 | 0x80, 0x00, 0x00, 0x00,
                                      0x00,        0x00, 0x00, 0x00};
    uint8_t rx[8];

    constexpr static float scale_mg = 1.0f / 2730.0f;
    gpio_put(ACC_CS, 0);
    spi_write_read_blocking(spi0, tx, rx, 8);
    gpio_put(ACC_CS, 1);

    int16_t x = (int16_t)((rx[3] << 8) | rx[2]);
    int16_t y = (int16_t)((rx[5] << 8) | rx[4]);
    int16_t z = (int16_t)((rx[7] << 8) | rx[6]);

    float x_mg = x * scale_mg;
    float y_mg = y * scale_mg;
    float z_mg = z * scale_mg;

    result[0] = x_mg;
    result[1] = y_mg;
    result[2] = z_mg;
  }

private:
};
