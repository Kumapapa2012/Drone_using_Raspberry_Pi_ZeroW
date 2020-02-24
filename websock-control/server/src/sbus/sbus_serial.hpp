#include <iostream>

namespace SBUS_CONTROLLER
{
class Sbus_Serial
{
public:
    // removing these constructors.
    Sbus_Serial(const Sbus_Serial &) = delete;
    Sbus_Serial &operator=(const Sbus_Serial &) = delete;
    Sbus_Serial(Sbus_Serial &&) = delete;
    Sbus_Serial &operator=(Sbus_Serial &&) = delete;

    static Sbus_Serial *GetInstance()
    {
        static Sbus_Serial instance;
        return &instance;
    }

    int start(std::string dev_path);  // Initialization and begin()
    int stop();

    // Currently Write only
    int update(unsigned char *);

private:
    int _fd = 0;
    const uint32_t _sbusBaud = 100000;
    int _connect(u_char *dev_path);
    Sbus_Serial();
    ~Sbus_Serial()
    {
        stop();
    };
};
}  // namespace SBUS_CONTROLLER
