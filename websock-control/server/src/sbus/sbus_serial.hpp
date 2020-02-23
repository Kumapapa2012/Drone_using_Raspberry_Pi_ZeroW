namespace SBUS_CONTROLLER {
class Sbus_Serial 
{
public:
    static Sbus_Serial *GetInstance()
    {
        return s_pInstance;
    }

   static int begin(std::string dev_path); // Instantiation and begin()
   static int end();

    // Currently Write only
   int update(unsigned char *); 
   
private:
    static Sbus_Serial *s_pInstance;
    Sbus_Serial();
    ~Sbus_Serial();
};
} //Name Space
