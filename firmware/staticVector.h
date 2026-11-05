#include <cstddef>
#include <cstdint>
#include <cstring>

template <typename T, size_t N> class StaticVector {
public:
  void push_back(const T &v) {
    if (m_Size < N) {
      m_Buffer[m_Size] = v;
      m_Size++;
    }
  }
  T pop_back() {
    if (m_Size > 0) {
      m_Size--;
      return m_Buffer[m_Size];
    }
    return T{};
  }
  size_t size() const { return m_Size; }
  T &operator[](int i) { return m_Buffer[i]; }

private:
  size_t m_Size = 0;
  T m_Buffer[N];
};
