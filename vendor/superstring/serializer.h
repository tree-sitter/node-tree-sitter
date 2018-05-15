#ifndef SERIALIZER_H_
#define SERIALIZER_H_

#include <vector>
#include <cstdint>

class Serializer {
  std::vector<uint8_t> &vector;

 public:
  inline Serializer(std::vector<uint8_t> &output) :
    vector(output) {};

  template <typename T>
  void append(T value) {
    for (auto i = 0u; i < sizeof(T); i++) {
      vector.push_back(value & 0xFF);
      value >>= 8;
    }
  }
};

class Deserializer {
  const uint8_t *read_ptr;
  const uint8_t *end_ptr;

 public:
  inline Deserializer(const std::vector<uint8_t> &input) :
    read_ptr(input.data()),
    end_ptr(input.data() + input.size()) {};

  template <typename T>
  T peek() const {
    T value = 0;
    const uint8_t *temp_ptr = read_ptr;
    if (static_cast<unsigned>(end_ptr - temp_ptr) >= sizeof(T)) {
      for (auto i = 0u; i < sizeof(T); i++) {
        value |= static_cast<T>(*(temp_ptr++)) << static_cast<T>(8 * i);
      }
    }
    return value;
  }

  template <typename T>
  T read() {
    T value = peek<T>();
    read_ptr += sizeof(T);
    return value;
  }
};

#endif // SERIALIZER_H_
