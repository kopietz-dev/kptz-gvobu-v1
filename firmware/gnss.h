#include "staticString.h"
#include "staticVector.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <hardware/gpio.h>
#include <hardware/spi.h>
#include <hardware/uart.h>

class GNSS {
public:
  void update() {
    static char buf[128];
    static int idx = 0;

    while (uart_is_readable(uart0)) {
      char c = uart_getc(uart0);

      if (idx < sizeof(buf) - 1)
        buf[idx++] = c;

      if (c == '\n') {
        buf[idx] = '\0';

        parseNMEA(buf);

        idx = 0;
      }
    }
  }
  void sendCmd(const char *body) {
    uint8_t cs = 0;
    for (const char *p = body; *p; p++)
      cs ^= (uint8_t)*p;

    char cmd[64];
    snprintf(cmd, sizeof(cmd), "$%s*%02X", body, cs);
    uart_puts(uart0, cmd);
    uart_puts(uart0, "\r\n");
    sleep_ms(100);
  }
  StaticStr<16> getTag(const char *nmea) {
    const char *ptr = nmea;

    StaticStr<16> tag;
    while (*ptr != '\0' && *ptr != ',') {
      tag += *ptr;
    }
    return tag;
  }
  StaticVector<StaticStr<16>, 10> splitNMEA(const char *sequence) {
    StaticVector<StaticStr<16>, 10> fields;
    const char *ptr = sequence;

    StaticStr<16> buf;

    int i = 0;
    while (*ptr != '\0') {
      if (*ptr == ',') {
        fields.push_back(buf);
        buf.clear();
        i++;
      } else {
        buf += *ptr;
      }

      ptr++;
    }
    if (!buf.empty())
      fields.push_back(buf);

    return fields;
  }
  float latToDec(const StaticStr<16> &pos, const StaticStr<16> &hem) {

    if (pos.empty())
      return 0.0f;

    float dec = (float)atof(pos.substr(0, 2).c_str()) +
                ((float)atof(pos.substr(2, 14).c_str()) / 60.0f);

    if (hem == "S")
      return -dec;
    else
      return dec;
  }
  float lonToDec(const StaticStr<16> &pos, const StaticStr<16> &hem) {

    if (pos.empty())
      return 0.0f;

    float dec = (float)atof(pos.substr(0, 3).c_str()) +
                ((float)atof(pos.substr(3, 14).c_str()) / 60.0f);

    if (hem == "W")
      return -dec;
    else
      return dec;
  }
  void parseNMEA(const char *sequence) {
    auto field = splitNMEA(sequence);

    if (field.size() == 0)
      return;

    if (field[0] == "$GNGLL") {
      const StaticStr<16> &sLat = field[1];
      const StaticStr<16> &hLat = field[2];
      const StaticStr<16> &sLon = field[3];
      const StaticStr<16> &hLon = field[4];

      lat = latToDec(sLat, hLat);
      lon = lonToDec(sLon, hLon);
    }
  }
  float getLon() const { return lon; }
  float getLat() const { return lat; }

private:
  float lat = 0, lon = 0;
};
