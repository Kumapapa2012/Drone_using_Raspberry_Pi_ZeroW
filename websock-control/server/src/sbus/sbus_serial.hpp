#include <iostream>
#include <string>

namespace SBUS_CONTROLLER {
class Sbus_Serial {
 public:
  // removing these constructors.
  Sbus_Serial(const Sbus_Serial&) = delete;
  Sbus_Serial& operator=(const Sbus_Serial&) = delete;
  Sbus_Serial(Sbus_Serial&&) = delete;
  Sbus_Serial& operator=(Sbus_Serial&&) = delete;

  static Sbus_Serial* GetInstance() {
    static Sbus_Serial instance;
    return &instance;
  }

  static int start(std::string dev_path);  // Initialization and begin()
  static int stop();

  // Currently Write only
  int update(unsigned char*);

 private:
  int _fd;
  const uint32_t _sbusBaud = 100000;
  Sbus_Serial() : _fd(0);
  ~Sbus_Serial();
};
}  // namespace SBUS_CONTROLLER
