#include <cstdint>
#include <cstring>
#include <string.h>
#include <type_traits>

template <size_t N> struct StaticStr {
  char buf[N] = {};
  size_t len = 0;

  void set(const char *s, size_t n) {
    len = n < N ? n : N - 1;
    memcpy(buf, s, len);
    buf[len] = '\0';
  }

  const char *c_str() const { return buf; }
  char *ptr() { return buf; }
  size_t size() const { return len; }
  bool empty() const { return len == 0; }

  void operator=(const char *s) { set(s); }
  void clear() { len = 0; }
  void operator+=(const char *s) {
    int nLen = strlen(s);
    memcpy(buf + len, s, nLen);

    len += nLen;
  }
  void operator+=(char c) {
    buf[len] = c;
    buf[len + 1] = '\0';
    len++;
  }
  char &operator[](int i) { return buf[i]; }
  bool operator==(const char *s) const { return strncmp(buf, s, N) == 0; }
  bool operator==(StaticStr<N> s) const {
    return strncmp(buf, s.c_str(), N) == 0;
  }
  StaticStr<N> substr(int start, int len) const {
    StaticStr<N> result;
    strncpy(result.ptr(), buf + start, len);

    return result;
  }
};
