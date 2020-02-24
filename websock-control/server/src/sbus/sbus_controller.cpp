#include "sbus_controller.hpp"
#include "sbus_coroutine.hpp"


// Main Implemantation
// Constructor()
SBUS_CONTROLLER::Sbus_Service::Sbus_Service() {
    if(!_sbus){
        _sbus = new SBUS::SBUS(SBUS_SERIAL_DEV);
    }
}
// Deconstructor()
SBUS_CONTROLLER::Sbus_Service::~Sbus_Service() {
    if(!_sbus){
        delete(_sbus);
    }
}

// sbusWrite()
void SBUS_CONTROLLER::Sbus_Service::sbusWrite(std::string strIn){
    // Check frames 
    std::regex re("frm[0-9A-F]{34}-");
    std::smatch m;
    std::string s_frame, s_channels;
    bool found=false;
    //Just Pick the last match.
    while (std::regex_search(strIn, m, re)) {
        found=true;
        s_frame = m.str();
        strIn = m.suffix();
    }
    if(found){
        s_channels = s_frame;
    }
    else {
        //newtral for now. maybe all down is better.
        s_channels = "frm0000000000000000000000000000000000-";
    }
    printf("setBuf - %s \r",s_channels.c_str());
    fflush(stdout);

    int _a,_t,_r,_e,_btn1;
    sscanf(s_channels.c_str(),"frm%08X%08X%08X%08X%02X-",&_a, &_t, &_r, &_e, &_btn1);

    //
    // start processing sbus data
    //
    float __sbus_channels_f[NUM_CHANNELS];
    // process 4 axis   >> A(0) E(1) R(2) T(3)
    __sbus_channels_f[3]=(float)_a / 32767.9999; //a
    __sbus_channels_f[1]=(float)_e*-1.0 / 32767.9999; // rrinverted!!
    __sbus_channels_f[2]=(float)_t*-1.0 / 32767.9999; //t
    __sbus_channels_f[0]=(float)_r / 32767.9999; //r

    // process 1 button >> AUX1 -  toggle
    if(_btn1){
        if(_btn1_prev==0) 
        {
            // casting 'int' and half up.
            if((int)(sbus_channels_f[4]+0.5f) == 1)
            {
                printf("disarmed!!\n");
                __sbus_channels_f[4]=0.00;
            }
            else{
                printf("armed!!\n");
                __sbus_channels_f[4]=1.00;
            }
        }
    }
    _btn1_prev = _btn1;
    // memcpy to production buffer
    memcpy(sbus_channels_f,__sbus_channels_f,sizeof(__sbus_channels_f));
}

void SBUS_CONTROLLER::Sbus_Service::sbusUpdate()
{
    // Send an SBUS packet.
    _sbus->writeCal(sbus_channels_f); 
}

int SBUS_CONTROLLER::Sbus_Service::setup(){
    // Setup FC
    // float channels[NUM_CHANNELS];
    bool failSafe;
    bool lostFrame;
    std::cout << "Beginning SBUS...";
    if(_setup == true) {
        return 0;
    }

    if(_sbus->begin()==-1){
        std::cout << "FAILED to begin";
        return -1;
    }

    // SBUS_PACKET
    memset(sbus_channels_f,0,sizeof(sbus_channels_f));

    _setup = true;
    return 0;
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