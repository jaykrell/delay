#include <stdio.h>

int main()
{
    int i;
    FILE* c = fopen("delaydll.c", "w");
    FILE* h = fopen("delay.h", "w");

    for (i = 0; i < 4097; ++i)
    {
        fprintf(c, "__declspec(dllexport) void delay%d() { }\n", i);
        fprintf(h, " void delay%d();\n", i);
    }
}
