#include "sbus_serial.hpp"

#include <unistd.h>     // UNIX standard function definitions
#include <fcntl.h>      // File control definitions
#include <errno.h>      // Error number definitions
#include <inttypes.h>
#include <asm/ioctls.h>
#include <asm/termbits.h>
#include <sys/ioctl.h>
#include <iostream>

int SBUS_CONTROLLER::Sbus_Serial::start(std::string dev_path)
{
    if(fd>0){
      cerr << "Error : Stop first" << endl;
      return -1;
    }
    // Setup Serial Connection
    //
    // open file descriptor
    _fd = open(dev_path.c_str(), O_RDWR | O_NOCTTY);
    if (_fd < 0)
    {
        cerr << "Error " << errno << " opening " << dev_path << ": " << strerror(errno)
             << endl;
        _fd = 0;
        return -1;
    }

    // set custom baudrate
    struct termios2 tio;
    int r = ioctl(_fd, TCGETS2, &tio);
    if (r)
    {
        cerr << "TCGETS2" << endl;
        close(_fd);
        return -1;
    }

    tio.c_cflag &= ~CBAUD;
    tio.c_cflag |= BOTHER;
    tio.c_cflag |= CSTOPB; // 2 stop bits
    tio.c_cflag |= PARENB; // enable parity bit, even by default
    tio.c_ispeed = tio.c_ospeed = _sbusBaud;

    r = ioctl(_fd, TCSETS2, &tio);
    if (r)
    {
        cerr << "TCSETS2" << endl;
        close(_fd);
        return -1;
    }
    return 0;
}

int SBUS_CONTROLLER::Sbus_Serial::stop()
{
  close(_fd);
  _fd=0;
}