#ifndef _SBUS_CONTROLLER
#define _SBUS_CONTROLLER

#include "sbus_serial.hpp"

#include "defaults.h"
#include <regex>
#include "oatpp/core/async/Executor.hpp"

namespace SBUS_CONTROLLER {
class Sbus_Service {
public:
  Sbus_Service();
  ~Sbus_Service();
  // settingup connection to sbus service(using socket)
  int setup();
  void activate();
  void deactivate();
  bool isActive();
  void sbusWrite(std::string strIn);
  void sbusUpdate();
private:
  // singleton instance
  SBUS_CONTROLLER::Sbus_Serial *_sbus_serial = nullptr;

  bool _setup = false;
  std::string _strBuf;
  //struct ::timespec ts_prev;
  // for a Toggle Button.
  int _btn1_prev;
  float sbus_channels_f[NUM_CHANNELS];
  bool _active = false;
  oatpp::async::Executor *_executor;
};

} // namespace SBUS_CONTROLLER

#endif