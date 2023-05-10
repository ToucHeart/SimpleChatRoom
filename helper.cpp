#include <stdarg.h>
#include <iostream>
using namespace std;

void printStrs(int newLineNum, int argc, ...)
{
    va_list ap;
    va_start(ap, argc);
    for (int i = 0; i < argc; i++)
    {
        cout << va_arg(ap, char *);
    }
    va_end(ap);
    for (int i = 0; i < newLineNum; i++)
    {
        cout << endl;
    }
}