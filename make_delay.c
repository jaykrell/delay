#include <stdio.h>

int main()
{
    int N = 4096 / 8;
    int i;
    FILE* c = fopen("delaydll.c", "w");
    FILE* r = fopen("delayref.c", "w");
    FILE* h = fopen("delay.h", "w");

    for (i = 0; i <= N; ++i)
    {
        fprintf(c, "__declspec(dllexport) void delay%03X() { }\n", i);
        fprintf(h, " void delay%03X();\n", i);
        fprintf(h, " void *__imp_delay%03X;\n", i);
        fprintf(r, " void delay%03X();\n", i);
    }

    fprintf(r, "__declspec(dllexport) void* delayref[] = {\n");

    for (i = 0; i <= N; ++i)
    {
        fprintf(r, "delay%03X,\n", i);
    }
    fprintf(r, "0};\n");
}
