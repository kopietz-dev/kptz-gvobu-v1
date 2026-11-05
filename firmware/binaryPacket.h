#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>

template <size_t N> class BinaryPacket {
public:
  BinaryPacket() { clear(); }
  BinaryPacket(void *src, size_t len) {
    clear();
    pushBack(src, len);
  }
  void pushBack(const void *src, size_t len) {
    size_t available = N - getSize();
    if (len > available)
      len = available;

    std::memcpy(m_Pointer, src, len);
    m_Pointer += len;
  }
  template <typename T> void pushBack(const T &value) {
    static_assert(std::is_trivially_copyable_v<T>);
    const size_t typeSize = sizeof(T);
    if (getSize() + typeSize > N)
      return;

    std::memcpy(m_Pointer, &value, typeSize);
    m_Pointer += typeSize;
  }

  size_t popBack(void *dest, size_t len) {
    if (getSize() < len)
      len = getSize();

    m_Pointer -= len;
    std::memcpy(dest, m_Pointer, len);

    return len;
  }

  template <typename T> T popBack() {
    static_assert(std::is_trivially_copyable_v<T>);
    T value;

    const size_t typeSize = sizeof(T);

    if (getSize() < typeSize)
      return {};

    m_Pointer -= typeSize;
    std::memcpy(&value, m_Pointer, typeSize);

    return value;
  }

  void clear() { m_Pointer = m_Buffer; }
  const uint8_t *getData() { return m_Buffer; }
  uint8_t *getBuffer() { return m_Buffer; }
  size_t getSize() const { return m_Pointer - m_Buffer; }

private:
  uint8_t m_Buffer[N];
  uint8_t *m_Pointer;
};
