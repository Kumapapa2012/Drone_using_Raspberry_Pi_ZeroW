#include "sbus_service.hpp"
#include "sbus_coroutine.hpp"


// Main Implemantation
// Constructor()
SBUS_CONTROLLER::Sbus_Service::Sbus_Service() {
    if(!_sbus_serial){
        _sbus_serial = Sbus_Serial::getInstance();
    }
}
// Deconstructor()
SBUS_CONTROLLER::Sbus_Service::~Sbus_Service() {
    // nothing to do
}

// sbusWrite()
void SBUS_CONTROLLER::Sbus_Service::sbusWrite(std::string strIn){
    if(!_sbus_serial){
        printf("_sbus_serial is NULL!!\n");
    }
    _sbus_serial->write(strIn);
}

void SBUS_CONTROLLER::Sbus_Service::sbusUpdate()
{
    _sbus_serial->update();
}

int SBUS_CONTROLLER::Sbus_Service::setup(){
    return _sbus_serial->start(SBUS_SERIAL_DEV);
}

void SBUS_CONTROLLER::Sbus_Service::activate(){
    // Start Coroutine
    // set active flag to true  
    _active=true;
    // then start coroutine.
    _executor = new oatpp::async::Executor();
    _executor->execute<Sbus_Writer>(this);
}

void SBUS_CONTROLLER::Sbus_Service::deactivate(){
    // Stop Coroutine  
    // set active flag to false  
    _active=false;
    // then wait for termination
    _executor->waitTasksFinished();
    _executor->stop();
    _executor->join();
    delete(_executor);
}

bool SBUS_CONTROLLER::Sbus_Service::isActive()
{
    return _active;
}