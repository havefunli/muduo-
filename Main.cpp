#include "TimerWheel.hpp" 

class Test
{
public:
    Test()
    {
        std::cout << "Test()" << std::endl;
    }

    void Del()
    {
        std::cout << "Del()" << std::endl;
    }

    ~Test()
    {
        std::cout << "~Test()" << std::endl;
    }
};

int main()
{
    Test t1;
    TimerWheel wheel(10);

    wheel.TimerAdd(1, 3, std::bind(&Test::Del, &t1));
    for (int i = 0; i < 2; i++)
    {
        wheel.Loop();
    }

    return 0;
}