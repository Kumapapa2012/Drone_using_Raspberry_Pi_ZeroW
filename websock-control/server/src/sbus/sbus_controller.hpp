#ifndef _SBUS_CONTROLLER
#define _SBUS_CONTROLLER

#include "SBUS.h"
#include "defaults.h"
//#include <chrono>
#include <regex>

#include "oatpp/core/async/Executor.hpp"
//#include "oatpp/core/async/Coroutine.hpp"

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

  SBUS::SBUS *_sbus = NULL;
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