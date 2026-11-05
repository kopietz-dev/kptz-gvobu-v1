#include "pico/stdlib.h"
#include <cstdint>
#include <hardware/gpio.h>
#include <hardware/spi.h>
#include <pico/types.h>
#define RADIO_CS 9
#define RADIO_DIO1 4
#define RADIO_BUSY 6
#define RADIO_DIO3 2

class Radio {
public:
  void set_standby(uint8_t mode) { sendCmd(0x80, mode); }
  void set_packet_type() { sendCmd(0x8A, 0x01); }

  void set_rf_freq(uint32_t freq_hz) {
    uint32_t freq_reg =
        (uint32_t)((uint64_t)freq_hz * 33554432ULL / 32000000ULL);

    uint8_t data[4];
    data[0] = (freq_reg >> 24) & 0xFF;
    data[1] = (freq_reg >> 16) & 0xFF;
    data[2] = (freq_reg >> 8) & 0xFF;
    data[3] = freq_reg & 0xFF;

    sendCmd(0x86, data, 4);
  }
  void set_pa_config(uint8_t power_dbm) {
    uint8_t tx[4];
    switch (power_dbm) {

    case 17:
      tx[0] = 0x02;
      tx[1] = 0x03;
      tx[2] = 0x00;
      tx[3] = 0x01;
      break;
    case 20:
      tx[0] = 0x03;
      tx[1] = 0x05;
      tx[2] = 0x00;
      tx[3] = 0x01;
      break;
    case 22:
      tx[0] = 0x04;
      tx[1] = 0x07;
      tx[2] = 0x00;
      tx[3] = 0x01;
      break;
    case 14:
    default:
      tx[0] = 0x02;
      tx[1] = 0x02;
      tx[2] = 0x00;
      tx[3] = 0x01;
      break;
    }
    sendCmd(0x95, tx, 4);
  }
  void set_tx_params() {
    uint8_t tx[2] = {0x16, 0x07};
    sendCmd(0x8E, tx, 2);
  }
  void set_buffer_base_address() {
    uint8_t tx[2] = {0x00, 0x00};
    sendCmd(0x8F, tx, 2);
  }
  void write_buffer(const uint8_t *data, uint8_t n) {
    uint8_t cmd = 0x0E, offset = 0x00;

    gpio_put(RADIO_CS, 0);
    spi_write_blocking(spi1, &cmd, 1);
    spi_write_blocking(spi1, &offset, 1);
    spi_write_blocking(spi1, data, n);
    gpio_put(RADIO_CS, 1);
  }

  void read_buffer(uint8_t *data, uint8_t n) {
    uint8_t cmd = 0x1E, offset = 0x00;

    gpio_put(RADIO_CS, 0);
    spi_write_blocking(spi1, &cmd, 1);
    spi_write_blocking(spi1, &offset, 1);
    spi_write_blocking(spi1, 0x00, 1);
    spi_read_blocking(spi1, 0x00, data, n);
    gpio_put(RADIO_CS, 1);
  }

  void set_modulation_params(uint8_t sf, uint16_t bw_khz) {
    uint8_t bw_b = 0x00;
    switch (bw_khz) {
    case 500:
      bw_b = 0x06;
      break;
    case 250:
      bw_b = 0x05;
      break;
    default:
    case 125:
      bw_b = 0x04;
      break;
    }

    uint8_t ldro = 0x00;
    if (sf > 9 || bw_khz == 125)
      ldro = 0x01;

    uint8_t tx[4] = {sf, bw_b, 0x04, ldro};
    sendCmd(0x8B, tx, 4);
  }

  void set_packet_params(uint8_t dataLength) {
    uint8_t tx[6] = {0x00, 0x08, 0x00, dataLength, 0x01, 0x00};

    sendCmd(0x8C, tx, 6);
  }

  void set_tx() {
    uint8_t tx[3] = {0x00, 0x00, 0x00};
    sendCmd(0x83, tx, 3);
  }

  void calibration_image() {
    uint8_t tx[2] = {0x6B, 0x6F};
    sendCmd(0x98, tx, 2);
  }

  void getDeviceError(uint8_t *rx) {
    uint8_t cmd = 0x17;

    gpio_put(RADIO_CS, 0);
    spi_write_blocking(spi1, &cmd, 1);
    spi_read_blocking(spi1, 0x00, rx, 3);
    gpio_put(RADIO_CS, 1);
  }

  void setRx(uint32_t timeout_ms) {
    // timeout is in units of 15.625 us
    // 0x000000 = no timeout (continuous RX)
    // 0xFFFFFF = single mode (RX until packet received)
    uint32_t timeout_val;
    if (timeout_ms == 0) {
      timeout_val = 0x000000;
    } else {
      timeout_val = (timeout_ms * 64);
    }

    uint8_t tx[3];
    tx[0] = (timeout_val >> 16) & 0xFF;
    tx[1] = (timeout_val >> 8) & 0xFF;
    tx[2] = timeout_val & 0xFF;
    sendCmd(0x82, tx, 3);
  }

  void get_irq_status(uint8_t *irq_status) {
    uint8_t cmd = 0x12;
    gpio_put(RADIO_CS, 0);
    spi_write_blocking(spi1, &cmd, 1);
    spi_read_blocking(spi1, 0x00, &cmd, 1);
    spi_read_blocking(spi1, 0x00, irq_status, 2);
    gpio_put(RADIO_CS, 1);
  }
#define IRQ_TX_DONE 0x0001
#define IRQ_RX_DONE 0x0002
#define IRQ_PREAMBLE_DETECTED 0x0004
#define IRQ_SYNC_WORD_VALID 0x0008
#define IRQ_HEADER_VALID 0x0010
#define IRQ_HEADER_ERROR 0x0020
#define IRQ_CRC_ERROR 0x0040
#define IRQ_CAD_DONE 0x0080
#define IRQ_CAD_DETECTED 0x0100
#define IRQ_TIMEOUT 0x0200
#define IRQ_ALL 0x03FF
  void set_dio_irq_params(uint16_t irq_mask, uint16_t dio1_mask,
                          uint16_t dio2_mask, uint16_t dio3_mask) {
    uint8_t tx[8];
    tx[0] = (irq_mask >> 8) & 0xFF;
    tx[1] = irq_mask & 0xFF;
    tx[2] = (dio1_mask >> 8) & 0xFF;
    tx[3] = dio1_mask & 0xFF;
    tx[4] = (dio2_mask >> 8) & 0xFF;
    tx[5] = dio2_mask & 0xFF;
    tx[6] = (dio3_mask >> 8) & 0xFF;
    tx[7] = dio3_mask & 0xFF;
    sendCmd(0x08, tx, 8);
  }

  void get_rx_buffer_status(uint8_t *payload_length, uint8_t *rx_start_addr) {
    uint8_t cmd = 0x13;
    uint8_t rx[3];
    gpio_put(RADIO_CS, 0);
    spi_write_blocking(spi1, &cmd, 1);
    spi_write_blocking(spi1, rx, 3);
    gpio_put(RADIO_CS, 1);

    *payload_length = rx[1];
    *rx_start_addr = rx[2];
  }

  void getPacketStatus(int8_t *rssi, int8_t *snr) {
    uint8_t cmd = 0x14;
    uint8_t rx[4];
    gpio_put(RADIO_CS, 0);
    spi_write_blocking(spi1, &cmd, 1);
    spi_write_blocking(spi1, rx, 4);
    gpio_put(RADIO_CS, 1);

    *rssi = -rx[1] / 2;
    *snr = rx[2] / 4;
  }

  void clearIrqStatus(uint16_t irq_mask) {
    uint8_t tx[2];
    tx[0] = (irq_mask >> 8) & 0xFF;
    tx[1] = irq_mask & 0xFF;
    sendCmd(0x02, tx, 2);
  }

  void wait_for_llcc68() {
    while (gpio_get(RADIO_BUSY))
      sleep_us(10);
  }
  uint8_t getRandomInt() {

    uint8_t tx[] = {0x1D, 0x08, 0x19, 0x00};
    uint8_t rx;

    gpio_put(RADIO_CS, 0);
    spi_write_blocking(spi1, tx, 4);
    spi_read_blocking(spi1, 0x00, &rx, 1);
    gpio_put(RADIO_CS, 1);

    return rx;
  }
  void init(uint32_t freq_hz, uint8_t power_dbm, uint8_t sf, uint16_t bw_khz,
            uint8_t dataLength) {

    wait_for_llcc68();
    set_standby(0x00);
    wait_for_llcc68();
    set_standby(0x01);

    wait_for_llcc68();
    set_packet_type();

    wait_for_llcc68();
    set_rf_freq(freq_hz);

    wait_for_llcc68();
    calibration_image();

    wait_for_llcc68();
    set_pa_config(power_dbm);

    wait_for_llcc68();
    set_tx_params();

    wait_for_llcc68();
    set_buffer_base_address();

    wait_for_llcc68();
    set_modulation_params(sf, bw_khz);

    wait_for_llcc68();
    set_packet_params(dataLength);

    wait_for_llcc68();
    set_dio_irq_params(IRQ_TX_DONE | IRQ_RX_DONE, IRQ_RX_DONE, 0, IRQ_TX_DONE);

    wait_for_llcc68();
    clearIrqStatus(IRQ_ALL);

    wait_for_llcc68();
    set_standby(0x00);
  }

private:
  void sendCmd(uint8_t cmd, void *v, uint8_t n) {
    gpio_put(RADIO_CS, 0);
    spi_write_blocking(spi1, &cmd, 1);
    spi_write_blocking(spi1, (uint8_t *)v, n);
    gpio_put(RADIO_CS, 1);
  }

  void sendCmd(uint8_t cmd, uint8_t v) { sendCmd(cmd, &v, 1); }
};
