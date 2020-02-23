#include <iostream>

class SingletonTest
{
private:
    SingletonTest();
    ~SingletonTest();

public:
    // removing these constructors.
    SingletonTest(const SingletonTest&) = delete;
    SingletonTest& operator=(const SingletonTest&) = delete;
    SingletonTest(SingletonTest&&) = delete;
    SingletonTest& operator=(SingletonTest&&) = delete;

    // returns static instance 
    static SingletonTest& get_instance()
    {
        static SingletonTest instance;
        return instance;
    }
    int method1();
    int method2();
};