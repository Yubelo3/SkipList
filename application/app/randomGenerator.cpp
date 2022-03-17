#include <fstream>
#include <iostream>
#include <random>

#define MAX_OPERATIONS 1
#define MIN_OPERATIONS 1
#define MAX_ELEMENTS 10000
#define MIN_ELEMENTS 10000
#define MAX_KEY 50000

//用来生成跳表随机操作的文件

int main()
{
    freopen("data/input/mytestout.txt", "w", stdout);
    srand(time(NULL));
    int operations, elements;
    while ((operations = rand() % MAX_OPERATIONS) < MIN_OPERATIONS - 1)
        ;
    while ((elements = rand() % MAX_ELEMENTS) < MIN_ELEMENTS - 1)
        ;
    std::cout << operations << " " << elements << std::endl;
    //生成随机元素
    for (int i = 0; i < elements; i++)
        std::cout << rand() % MAX_KEY << " ";
    //生成随机操作
    for (int i = 0; i < operations; i++)
    {
        //随机操作
        int op = rand() % 5 + 1;
        std::cout << op << " ";
        //如果是123操作，还有操作数
        if (op <= 3)
        {
            int num = rand() % MAX_KEY;
            std::cout << num;
        }
        std::cout << std::endl;
    }
    return 0;
}