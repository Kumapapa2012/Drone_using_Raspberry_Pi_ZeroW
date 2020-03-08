#include "sbus_serial.hpp"

#include <errno.h>
#include <unistd.h>

#include <cstring>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>

// Serial Connection
#include <asm/ioctls.h>
#include <asm/termbits.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/ioctl.h>

// this is for testing.
#include <time.h>

int SBUS_CONTROLLER::Sbus_Serial::start(std::string dev_path)
{
    if (_connect(dev_path.c_str()) == -1)
    {
        return -1;
    }
    // for toggles
    for (int i = 0; i < 4; i++)
    {
        _prev[i] = "000";
    }
    return 0;
}

int SBUS_CONTROLLER::Sbus_Serial::_connect(std::string dev_path)
{
    if (_fd > 0)
    {
        std::cerr << "Error : Ruuning...Stop first" << std::endl;
        return -1;
    }
    // Serial Connection Setup
    // open serial device
    _fd = open(dev_path.c_str(), O_RDWR | O_NOCTTY);
    if (_fd < 0)
    {
        std::cerr << "Error " << errno << " opening " << dev_path << ": " << std::strerror(errno)
                  << std::endl;
        _fd = 0;
        return -1;
    }

    // set up the device
    struct termios2 tio;
    int r = ioctl(_fd, TCGETS2, &tio);
    if (r)
    {
        std::cerr << "TCGETS2" << std::endl;
        close(_fd);
        return -1;
    }

    tio.c_cflag &= ~CBAUD;
    tio.c_cflag |= BOTHER;
    tio.c_cflag |= CSTOPB;  // 2 stop bits
    tio.c_cflag |= PARENB;  // enable parity bit, even by default
    tio.c_ispeed = tio.c_ospeed = _sbusBaud;

    r = ioctl(_fd, TCSETS2, &tio);
    if (r)
    {
        std::cerr << "TCSETS2" << std::endl;
        close(_fd);
        return -1;
    }
    return 0;
}

int SBUS_CONTROLLER::Sbus_Serial::write(std::string strIn)
{
    int numAxes = 0;
    int numButtons = 0;
    // for axes mapping.
    // must be 0 to 3.
    // This Mapping is Mode 2.
    int idxRoll = 2;      // [A]
    int idxPitch = 3;     // [E]
    int idxThrottle = 1;  // [T]
    int idxYaw = 0;       // [R]

    // for debugging...
    _s_sbusPacket = strIn;

    // check the format.
    // if wrong, print error and return.
    std::regex re("all-([0-9A-F]{2}([0-9A-F]{3})+-){2}");
    if (strIn.length() == 0 || !std::regex_match(strIn.c_str(), re))
    {
        std::cout << "Invalid Format! : " << strIn;
        return -1;
    }

    // Now packet format correctness is guaranteed.
    // Start Parsing

    // take # of axes. must be >4. if >=4 take first 4 else fail.
    numAxes = std::stoi(strIn.substr(4, 2),nullptr,16);
    if (numAxes < 4)
    {
        std::cout << "Too few axes: " << numAxes;
        return -1;
    }

    // lambda function to calc channel value
    // apply default to all channels for now.
    auto calcRawValue = [this](std::string strControllerVal, bool reverse) {
        uint16_t rawChannelVal = std::stoi(strControllerVal, nullptr, 16);
        if (reverse)
        {
            rawChannelVal = 0xfff - rawChannelVal;
        }
        float normalizedVal = (float)rawChannelVal / 0xfff;
        return _ep_min + (uint16_t)((_ep_max - _ep_min) * normalizedVal);
    };

    // axes - use 4 uint16_t each holds 11 bit (AETR)
    _channel[0] = calcRawValue(strIn.substr(6 + (idxRoll * 3), 3), false);
    _channel[1] = calcRawValue(strIn.substr(6 + (idxPitch * 3), 3), true);
    _channel[2] = calcRawValue(strIn.substr(6 + (idxThrottle * 3), 3), true);
    _channel[3] = calcRawValue(strIn.substr(6 + (idxYaw * 3), 3), false);

    // now find the start position of buttons
    int button_start = strIn.find('-', 5) + 1;

    // get numbottons. only 12 buttons are supported since using 4 channels already(axes)
    // Total 16 Channels
    numButtons = std::min(12, std::stoi(strIn.substr(button_start, 2),nullptr,16));

    button_start += 2;
    for (int i = 0; i < numButtons; i++)
    {
        if (numButtons > 4 && i < numButtons - 4)
        {
            _channel[i + 4] = calcRawValue(strIn.substr(button_start + (3 * i), 3), false);
        }
        // Make last 4 buttons are toggle switch.
        // (There shold be a mapping from gamepad to FC. Will be implemented.)
        else 
        {
            std::string inToggle = strIn.substr(button_start + (3 * i), 3);
            std::cout << i - (numButtons - 4);
            if (inToggle == "FFF" && inToggle != _prev[i - (numButtons - 4)])
            {
                if (_channel[i + 4] == _ep_max)
                {
                    _channel[i + 4] = _ep_min;
                }
                else
                {
                    _channel[i + 4] = _ep_max;
                }
            }
            _prev[i - (numButtons - 4)] = inToggle;
        }
    }
    std::cout << "\r" << std::flush;
    //
    //build an SBUS packet
    // Byte[0]: SBUS Header, 0x0F
    _sbusPacket[0] = 0x0F;
    // Byte[1-22]: 16 servo channels, 11 bits per servo channel
    _sbusPacket[1] = (uint8_t)(_channel[0] & 0x7ff);
    _sbusPacket[2] = (uint8_t)((_channel[0] & 0x7ff) >> 8 | (_channel[1] & 0x7ff) << 3);
    _sbusPacket[3] = (uint8_t)((_channel[1] & 0x7ff) >> 5 | (_channel[2] & 0x7ff) << 6);
    _sbusPacket[4] = (uint8_t)((_channel[2] & 0x7ff) >> 2);
    _sbusPacket[5] = (uint8_t)((_channel[2] & 0x7ff) >> 10 | (_channel[3] & 0x7ff) << 1);
    _sbusPacket[6] = (uint8_t)((_channel[3] & 0x7ff) >> 7 | (_channel[4] & 0x7ff) << 4);
    _sbusPacket[7] = (uint8_t)((_channel[4] & 0x7ff) >> 4 | (_channel[5] & 0x7ff) << 7);
    _sbusPacket[8] = (uint8_t)((_channel[5] & 0x7ff) >> 1);
    _sbusPacket[9] = (uint8_t)((_channel[5] & 0x7ff) >> 9 | (_channel[6] & 0x7ff) << 2);
    _sbusPacket[10] = (uint8_t)((_channel[6] & 0x7ff) >> 6 | (_channel[7] & 0x7ff) << 5);
    _sbusPacket[11] = (uint8_t)((_channel[7] & 0x7ff) >> 3);
    _sbusPacket[12] = (uint8_t)((_channel[8] & 0x7ff));
    _sbusPacket[13] = (uint8_t)((_channel[8] & 0x7ff) >> 8 | (_channel[9] & 0x7ff) << 3);
    _sbusPacket[14] = (uint8_t)((_channel[9] & 0x7ff) >> 5 | (_channel[10] & 0x7ff) << 6);
    _sbusPacket[15] = (uint8_t)((_channel[10] & 0x7ff) >> 2);
    _sbusPacket[16] = (uint8_t)((_channel[10] & 0x7ff) >> 10 | (_channel[11] & 0x7ff) << 1);
    _sbusPacket[17] = (uint8_t)((_channel[11] & 0x7ff) >> 7 | (_channel[12] & 0x7ff) << 4);
    _sbusPacket[18] = (uint8_t)((_channel[12] & 0x7ff) >> 4 | (_channel[13] & 0x7ff) << 7);
    _sbusPacket[19] = (uint8_t)((_channel[13] & 0x7ff) >> 1);
    _sbusPacket[20] = (uint8_t)((_channel[13] & 0x7ff) >> 9 | (_channel[14] & 0x7ff) << 2);
    _sbusPacket[21] = (uint8_t)((_channel[14] & 0x7ff) >> 6 | (_channel[15] & 0x7ff) << 5);
    _sbusPacket[22] = (uint8_t)((_channel[15] & 0x7ff) >> 3);
    // Byte[23]:
    // Bit 7: digital channel 17 (0x80)
    // Bit 6: digital channel 18 (0x40)
    // Bit 5: frame lost (0x20)
    // Bit 4: failsafe activated (0x10)
    // Bit 0 - 3: n/a
    // -> 0
    _sbusPacket[23] = 0;
    // Byte[24]: SBUS End Byte, 0x00
    _sbusPacket[24] = 0;

    update();
    return 0;
}

int SBUS_CONTROLLER::Sbus_Serial::update()
{
    time_t tt;
    time(&tt);

    //std::cout << "TEST: " << std::dec << tt << _s_sbusPacket << "\r" << std::flush;
    std::stringstream ss;

    ss << "Packet :";
    for (int i = 0; i < 25; i++)
    {
        ss << std::setfill('0') << std::right << std::setw(2) << std::hex << (uint)_sbusPacket[i];
    }
    ss << " - " << _s_sbusPacket;
    ss << "\r";
    std::cout << tt << " - " << ss.str() << std::flush;

    ::write(_fd, _sbusPacket, sizeof(_sbusPacket));
    return 0;
}