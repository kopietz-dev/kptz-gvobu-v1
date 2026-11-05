#include <cstdint>
#include <hardware/gpio.h>
#include <hardware/spi.h>
#define MAG_CS 12
#define MAG_INT 13

#define CTRL0_TM_M (1 << 0)       // Take magnetic measurement
#define CTRL0_TM_T (1 << 1)       // Take temperature measurement
#define CTRL0_INT_EN (1 << 2)     // Enable interrupt
#define CTRL0_SET (1 << 3)        // Perform SET
#define CTRL0_RESET (1 << 4)      // Perform RESET
#define CTRL0_AUTO_SR_EN (1 << 5) // Auto set/reset

#define REG_CTRL0 0x09
#define REG_CTRL1 0x0A
#define REG_CTRL2 0x0B

#define CTRL2_CMM_EN (1 << 7) // Continuous measurement mode
#define CTRL2_EN_PRD_SET (1 << 3)
#define CTRL2_ODR_1000HZ 0x07
#define CTRL2_ODR_200HZ 0x06
#define CTRL2_ODR_100HZ 0x05
#define CTRL2_ODR_50HZ 0x04

class Magnetometer {
public:
  uint8_t getID() {
    uint8_t tx[] = {0x2F | 0x80, 0x00};
    uint8_t rx[2];
    gpio_put(MAG_CS, 0);
    spi_write_read_blocking(spi1, tx, rx, 2);
    gpio_put(MAG_CS, 1);
    return rx[1];
  }
  void writeReg(uint8_t reg, uint8_t data) {
    uint8_t buf[2] = {reg, data};
    gpio_put(MAG_CS, 0);
    spi_write_blocking(spi1, buf, 2);
    gpio_put(MAG_CS, 1);
  }

  uint8_t readReg(uint8_t reg) {
    uint8_t tx = reg | 0x80;
    uint8_t rx = 0;

    gpio_put(MAG_CS, 0);

    spi_write_blocking(spi1, &tx, 1);
    spi_read_blocking(spi1, 0x00, &rx, 1);

    gpio_put(MAG_CS, 1);

    return rx;
  }

  void readReg(uint8_t startReg, uint8_t *buf, size_t len) {
    uint8_t tx = startReg | 0x80;

    gpio_put(MAG_CS, 0);

    spi_write_blocking(spi1, &tx, 1);
    spi_read_blocking(spi1, 0x00, buf, len);

    gpio_put(MAG_CS, 1);
  }
  void set() { writeReg(REG_CTRL0, CTRL0_SET); }
  void reset() { writeReg(REG_CTRL0, CTRL0_RESET); }
  void readAxis(float *res) {
    while (!(readReg(0x08) & 0x01))
      ;

    uint8_t buf[7];
    readReg(0x00, buf, 7);

    uint32_t raw[3];

    raw[0] = ((uint32_t)buf[0] << 10) | ((uint32_t)buf[1] << 2) |
             ((buf[6] >> 6) & 0x03);

    raw[1] = ((uint32_t)buf[2] << 10) | ((uint32_t)buf[3] << 2) |
             ((buf[6] >> 4) & 0x03);

    raw[2] = ((uint32_t)buf[4] << 10) | ((uint32_t)buf[5] << 2) |
             ((buf[6] >> 2) & 0x03);

    for (int i = 0; i < 3; i++)
      res[i] = ((int32_t)raw[i] - 131072) / 16384.0f;
  }
  void init() {
    writeReg(REG_CTRL1, 0x00);
    writeReg(REG_CTRL2, CTRL2_CMM_EN | CTRL2_EN_PRD_SET | CTRL2_ODR_100HZ);
  }

private:
};
