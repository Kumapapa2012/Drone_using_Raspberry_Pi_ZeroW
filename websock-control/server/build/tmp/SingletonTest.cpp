#include "SingletonTest.hpp"

SingletonTest::SingletonTest() {
    std::cout << "Entering constroctor\n";
}

SingletonTest::~SingletonTest() {
    std::cout << "Entering deconstroctor\n";
}

int SingletonTest::method1() {
    std::cout << "Entering method1\n";
    return 1;
}

int SingletonTest::method2() {
    std::cout << "Entering method2\n";
    return 1;
}