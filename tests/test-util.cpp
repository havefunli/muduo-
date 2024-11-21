#include "../src/Util.hpp"
#include <iostream>

void test_1()
{
    std::string str = "abc,,,,,edf,abc,";
    std::vector<std::string> array;
    Util::Split(str, ",", array);
    for (auto str : array)
    {
        std::cout << str << " ";
    }
    std::cout << std::endl;
}

void test_2()
{
    std::string buff;
    Util::ReadFile("test.txt", buff);
    std::cout << buff << std::endl;
}

void test_3()
{
    std::string buff = "ABCD";
    Util::WriteFile("test.txt", buff);
}

void test_4()
{
    std::string buff = "https://gitee.com/qigezi/tcp-server/blob/master/source/http/http.hpp";
    std::string result;
    Util::UrlEncode(buff, result, true);
    std::cout << result << std::endl;
    std::cout << result.size() << std::endl;
}

void test_5()
{
    std::string buff = "12345%2B++____";
    std::string result;
    Util::UrlDecode(buff, result, true);
    std::cout << result << std::endl;
    std::cout << result.size() << std::endl;
}

void test_6()
{
    std::string dir = "index/../../";
    if (Util::IsValidPath(dir)) { std::cout << "Its a valid path..." << std::endl; }
    else { std::cout << "Its not a valid path..." << std::endl; }
}

int main()
{
    // test_1();
    // test_2();
    // test_3();
    // test_4();
    // test_5();
    test_6();
    return 1;
}