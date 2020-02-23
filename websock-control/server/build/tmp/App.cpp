#include "SingletonTest.hpp"

int main() {
    std::cout << "Entering App::main()\n";
    SingletonTest& obj = SingletonTest::get_instance();
    obj.method1();
    obj.method2();
    SingletonTest& obj2 = SingletonTest::get_instance();
    std::cout << "&obj=" << &obj <<"\n";
    std::cout << "&obj2=" << &obj2 <<"\n";
    return 0;
}