#include "./../em-kptz/binaryWorker.h"
#include "board.h"
#include "hardware/spi.h"
#include "pico/multicore.h"
#include "tusb.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <math.h>
#include <pico/platform/common.h>
#include <pico/time.h>
#include <pico/types.h>

#define MAX_PACKET_LENGTH 64
uint8_t buffer[MAX_PACKET_LENGTH];
uint8_t bufferLen;
void write_cdc(const void *v, uint8_t n) {
  if (tud_cdc_connected() && tud_cdc_write_available() >= n) {
    tud_cdc_write(v, n);
    tud_cdc_write_flush();
  }
}

void tud_mount_cb(void) {}
void tud_umount_cb(void) {}
void tud_suspend_cb(bool) {}
void tud_resume_cb(void) {}

void tud_cdc_line_state_cb(uint8_t, bool, bool) {}
void tud_cdc_rx_cb(uint8_t itf) {
  bufferLen = tud_cdc_read(buffer, sizeof(buffer));
  if (bufferLen == 0)
    return;
}
uint8_t outBuffer[3][MAX_PACKET_LENGTH];

void core1_entry() {
  gpio_init(LED0_PIN);
  gpio_set_dir(LED0_PIN, GPIO_OUT);
  while (true) {
    gpio_put(LED0_PIN, 1);
    sleep_ms(50);
    gpio_put(LED0_PIN, 0);
    sleep_ms(500);
  }
}
void blink0() {
  gpio_put(LED0_PIN, 1);
  sleep_ms(500);
  gpio_put(LED0_PIN, 0);
  sleep_ms(500);
}
void blink1() {
  gpio_put(LED1_PIN, 1);
  sleep_ms(500);
  gpio_put(LED1_PIN, 0);
  sleep_ms(500);
}

int main() {
  Board board;
  board.init();

  // multicore_launch_core1(core1_entry);
  //  measure_noise(board, 1000);

  float dt = 0, counter = 0;

  BinaryWorker worker(buffer, 64);

  tusb_init();

  float gyro[3], acc[3], mag[3];

  float v = 0;
  float timer = 0.0f;

  constexpr float maxThrottle = 0.097f;
  constexpr float minThrottle = 0.055f;

  for (int i = 0; i < 3; i++)
    blink1();

  board.set_pwm_value(PWM4_PIN, minThrottle);
  absolute_time_t t_prev = get_absolute_time();

  while (1) {
    tight_loop_contents();

    tud_task();

    absolute_time_t t_now = get_absolute_time();
    int64_t dt_us = absolute_time_diff_us(t_prev, t_now);
    t_prev = t_now;
    dt = dt_us * 1e-6f;

    timer += dt;

    if (timer >= 10.0f) {
      blink0();

      board.set_pwm_value(PWM4_PIN, maxThrottle * 0.9f);

      timer = 0;
    }
  }
}
