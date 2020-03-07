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

    static Sbus_Serial *getInstance()
    {
        static Sbus_Serial instance;
        return &instance;
    }


    int start(std::string dev_path);  // Initialization and begin()
    int stop();

    // Currently Write only
    int write(std::string strIn);
    int update();

private:
    int _fd = 0;
    const uint32_t _sbusBaud = 100000;
    uint16_t _ep_min_default = 172;  // FrSky default
    uint16_t _ep_max_default = 1811; // FrSky default
    uint16_t _ep_min = 193;  // My value
    uint16_t _ep_max = 1793; // My value
    uint16_t _channel[16];
    std::string _prev[4];
    unsigned char _sbusPacket[25];
    std::string _s_sbusPacket; // dummy for testing.
    int _connect(std::string dev_path);
    Sbus_Serial() = default;
    ~Sbus_Serial() = default;
};
}  // namespace SBUS_CONTROLLER
