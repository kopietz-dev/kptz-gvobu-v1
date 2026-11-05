#include "pico/stdlib.h"
#include <hardware/gpio.h>
#include <hardware/spi.h>
#include <stdint.h>

#define FLASH_CS 25

class Memory {
public:
  void startSPI() { gpio_put(FLASH_CS, 0); }
  void finishSPI() { gpio_put(FLASH_CS, 1); }

  void writeInstruction(uint8_t cmd) { spi_write_blocking(spi0, &cmd, 1); }
  uint8_t getID() {
    uint8_t rx[3];
    startSPI();
    writeInstruction(0x9F);
    spi_read_blocking(spi0, 0x00, rx, 3);
    finishSPI();

    return rx[0];
  }
  void readFromAddress(uint8_t addr, uint8_t *rx, size_t len) {
    startSPI();
    writeInstruction(0x03);
    spi_read_blocking(spi0, 0x00, rx, len);
    finishSPI();
  }
  uint8_t readStatus() {
    uint8_t cmd[2] = {0x05, 0x00};
    uint8_t rx[2];

    gpio_put(FLASH_CS, 0);
    spi_write_read_blocking(spi0, cmd, rx, 2);
    gpio_put(FLASH_CS, 1);

    return rx[1];
  }
  void eraseSector(uint32_t addr) {
    uint8_t cmd[5] = {0x06, 0x20, // sector erase
                      (uint8_t)((addr >> 16) & 0xFF),
                      (uint8_t)((addr >> 8) & 0xFF), (uint8_t)(addr & 0xFF)};

    gpio_put(FLASH_CS, 0);
    spi_write_blocking(spi0, cmd, 5);
    gpio_put(FLASH_CS, 1);
  }
  void eraseAll() {
    uint8_t cmd[2] = {0x06, 0x60};
    startSPI();
    spi_write_blocking(spi0, cmd, 2);
    finishSPI();
  }
  void writeToAddress(uint32_t addr, uint8_t *tx, size_t len) {
    uint8_t cmd[5] = {0x06, 0x02, (uint8_t)((addr >> 16) & 0xFF),
                      (uint8_t)((addr >> 8) & 0xFF), (uint8_t)(addr & 0xFF)};
    startSPI();
    spi_write_blocking(spi0, cmd, 5);
    spi_write_blocking(spi0, tx, len);
    finishSPI();
  }

private:
};
