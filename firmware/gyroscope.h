#include "pico/stdlib.h"
#include <hardware/gpio.h>
#include <hardware/spi.h>
#include <stdint.h>

#define GYRO_CS 24

class Gyroscope {
public:
  void writeReg(uint8_t reg, uint8_t val) {
    uint8_t cmd = (uint8_t)(reg & 0x7F);
    uint8_t tx[2] = {cmd, val};

    gpio_put(GYRO_CS, 0);
    spi_write_blocking(spi0, tx, 2);
    gpio_put(GYRO_CS, 1);
  }

  void readReg(uint8_t reg, uint8_t *val) {
    uint8_t cmd = (uint8_t)(reg | 0x80);
    uint8_t tx[2] = {cmd, 0x00};
    uint8_t rx[2];

    gpio_put(GYRO_CS, 0);
    spi_write_read_blocking(spi0, tx, rx, 2);
    gpio_put(GYRO_CS, 1);

    *val = rx[1];
  }

  void init() {
    // soft reset
    writeReg(0x14, 0xB6);
    sleep_ms(50);

    // power up gyro
    writeReg(0x0E, 0x00); // exit suspend
    sleep_ms(5);

    writeReg(0x11, 0x00); // normal mode
    sleep_ms(50);

    // range: ±500 dps
    writeReg(0x0F, 0x02);

    // bandwidth: 47 Hz
    writeReg(0x10, 0x03);

    sleep_ms(20);
  }
  uint8_t getID() {
    uint8_t tx[2] = {0x00 | 0x80, 0x00};
    uint8_t rx[2];

    gpio_put(GYRO_CS, 0);
    spi_write_read_blocking(spi0, tx, rx, 2);
    gpio_put(GYRO_CS, 1);

    return rx[1];
  }

  void readAxis(float *result) {
    uint8_t tx[7] = {(uint8_t)(0x80 | 0x02), 0, 0, 0, 0, 0, 0};
    uint8_t rx[7];

    gpio_put(GYRO_CS, 0);
    spi_write_read_blocking(spi0, tx, rx, 7);
    gpio_put(GYRO_CS, 1);

    int16_t x = (int16_t)((rx[2] << 8) | rx[1]);
    int16_t y = (int16_t)((rx[4] << 8) | rx[3]);
    int16_t z = (int16_t)((rx[6] << 8) | rx[5]);

    const float scale = (1.0f / 65.536f) * (3.14159f / 180.0f);

    result[0] = x * scale;
    result[1] = y * scale;
    result[2] = z * scale;
  }

private:
};
