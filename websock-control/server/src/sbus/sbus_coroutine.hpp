#include "sbus_service.hpp"
#include <chrono>
#include "oatpp/core/async/Coroutine.hpp"

// Coroutine
//bool stopFlag = false;
class Sbus_Writer : public oatpp::async::Coroutine<Sbus_Writer>{
    private:
        SBUS_CONTROLLER::Sbus_Service *_sbus_service;
    public:
        Sbus_Writer(SBUS_CONTROLLER::Sbus_Service *param) : _sbus_service(param) {}
        Action act() override{
            std::chrono::microseconds ms =
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::system_clock::now().time_since_epoch());

            if (_sbus_service->isActive()) {
                _sbus_service->sbusUpdate();  
                v_int64 tick = ms.count();
                // 10 ms interval -- frsky
                return Action::createWaitRepeatAction(tick + 10000);
            } else {
                // Termination!
                std::cout << "Finishing\n";
                return Action::createActionByType(Action::TYPE_FINISH);
            }
        }
};