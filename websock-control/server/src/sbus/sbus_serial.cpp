#include "sbus_serial.hpp"

#include <asm/ioctls.h>
#include <asm/termbits.h>
#include <errno.h>  // Error number definitions
#include <fcntl.h>  // File control definitions
#include <inttypes.h>
#include <sys/ioctl.h>
#include <unistd.h>  // UNIX standard function definitions

#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

// this is for testing. Remove it.
#include <time.h>
// sscanf
//#include <stdio.h>

int SBUS_CONTROLLER::Sbus_Serial::start(std::string dev_path)
{
    if (_connect(dev_path.c_str()) == -1)
    {
        return -1;
    }
    // start coroutine

    return 0;
}

int SBUS_CONTROLLER::Sbus_Serial::_connect(std::string dev_path)
{
    if (_fd > 0)
    {
        std::cerr << "Error : Stop first" << std::endl;
        return -1;
    }
    // Setup Serial Connection
    //
    // open file descriptor
    _fd = open(dev_path.c_str(), O_RDWR | O_NOCTTY);
    if (_fd < 0)
    {
        std::cerr << "Error " << errno << " opening " << dev_path << ": " << std::strerror(errno)
                  << std::endl;
        _fd = 0;
        return -1;
    }

    // set custom baudrate
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
    // these are for axis mapping.
    // must be 0 to 3.
    int idxRoll = 0;      // [A]
    int idxPitch = 1;     // [E]
    int idxThrottle = 2;  // [T]
    int idxYaw = 3;       // [R]

    uint16_t axes[4];
    uint16_t buttons[12];

    // for debugging...
    _s_sbusPacket = strIn;

    //"all-044044003FC424-11000000000000000000000000000000000000000000000000000-"
    // take # of axes. must be >4. if >=4 take first 4 else fail.
    numAxes = std::stoi(strIn.substr(4, 2));
    if (numAxes < 4)
        return -1;

    // let's build an SBUS packet for flight controller.
    // Byte[0]: SBUS Header, 0x0F
    _sbusPacket[0] = 0x0F;
    // Byte[1-22]: 16 servo channels, 11 bits per servo channel
    // axes - use 4 uint16_t each holds 11 bit (AETR)
    axes[0] = std::stoi(strIn.substr(6 + (idxRoll * 3), 3), nullptr, 16) + 1000;
    axes[1] = std::stoi(strIn.substr(6 + (idxPitch * 3), 3), nullptr, 16) + 1000;
    axes[2] = std::stoi(strIn.substr(6 + (idxThrottle * 3), 3), nullptr, 16) + 1000;
    axes[3] = std::stoi(strIn.substr(6 + (idxYaw * 3), 3), nullptr, 16) + 1000;

    // 11111111111 11111111111 11111111111 11111111111
    // 12345678|9ab 12345|6789ab 12|3456789a|b 1234567|89ab
    // ba987654|321 ba987|654321 ba|98765432|1 ba98765|4321

    _sbusPacket[1] = (uint8_t)(axes[0] & 0xff);
    _sbusPacket[2] = (uint8_t)(axes[0] >> 8 | (axes[1] & 0x1F) << 3);
    _sbusPacket[3] = (uint8_t)(axes[1] >> 5 | axes[2] & 0x3 << 6);
    _sbusPacket[4] = (uint8_t)((axes[2] >> 2) * 0xff);
    _sbusPacket[5] = (uint8_t)(axes[2] >> 10 | (axes[3] & 0x7f) << 1);
    _sbusPacket[6] = (uint8_t)(axes[3] >> 7);

    // now find the start point of buttons
    int button_start = strIn.find('-', 5) + 1;

    // get numbottons. only 12 buttons are supported since using 4 channels already(axes)
    numButtons = std::max(12, std::stoi(strIn.substr(button_start, 2)));

    button_start += 2;
    for (int i = 0; i < numButtons; i++)
    {
        buttons[i] = std::stoi(strIn.substr(button_start + (3 * i), 3), nullptr, 16);
    }

    // set the values for 12 buttons.
    _sbusPacket[6] |= (uint8_t)((buttons[0]) << 4);
    _sbusPacket[7] = (uint8_t)((buttons[0]) >> 4 | (buttons[1]) << 7);
    _sbusPacket[8] = (uint8_t)((buttons[1]) >> 1);
    _sbusPacket[9] = (uint8_t)((buttons[1]) >> 9 | (buttons[2]) << 2);
    _sbusPacket[10] = (uint8_t)((buttons[2]) >> 6 | (buttons[3]) << 5);
    _sbusPacket[11] = (uint8_t)((buttons[3]) >> 3);
    _sbusPacket[12] = (uint8_t)((buttons[4]));
    _sbusPacket[13] = (uint8_t)((buttons[4]) >> 8 | (buttons[5]) << 3);
    _sbusPacket[14] = (uint8_t)((buttons[5]) >> 5 | (buttons[6]) << 6);
    _sbusPacket[15] = (uint8_t)((buttons[6]) >> 2);
    _sbusPacket[16] = (uint8_t)((buttons[6]) >> 10 | (buttons[7]) << 1);
    _sbusPacket[17] = (uint8_t)((buttons[7]) >> 7 | (buttons[8]) << 4);
    _sbusPacket[18] = (uint8_t)((buttons[8]) >> 4 | (buttons[9]) << 7);
    _sbusPacket[19] = (uint8_t)((buttons[9]) >> 1);
    _sbusPacket[20] = (uint8_t)((buttons[9]) >> 9 | (buttons[10]) << 2);
    _sbusPacket[21] = (uint8_t)((buttons[10]) >> 6 | (buttons[11]) << 5);
    _sbusPacket[22] = (uint8_t)((buttons[11]) >> 3);
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
        ss << std::setfill('0')  << std::right << std::setw(2) << std::hex << (uint)_sbusPacket[i];
    }
    ss << " - " << _s_sbusPacket;
    ss << "\r";
    std::cout << tt << " - " << ss.str() << std::flush;

    ::write(_fd, _sbusPacket, sizeof(_sbusPacket));
    return 0;
}