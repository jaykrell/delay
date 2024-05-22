#define PAGE_SIZE 0x1000
#include <stdio.h>
#include "delay.h"
#include "windows.h"

HANDLE event, thread1, thread2;

void ExceptionHandler()
{
}

DWORD Thread1(PVOID p)
{
    return 0;
}

DWORD Thread2(PVOID p)
{
    WaitForSingleObject(event, -1);
    return 0;
}

int main()
{
    printf("%p %p %p\n", &__imp_delay000, (char*)((size_t)&__imp_delay000 & ~(PAGE_SIZE - 1)), __imp_delay000);
    printf("%p %p %p\n", &__imp_delay200, (char*)((size_t)&__imp_delay200 & ~(PAGE_SIZE - 1)), __imp_delay200);
    delay000();
    printf("%p %p %p\n", &__imp_delay000, (char*)((size_t)&__imp_delay000 & ~(PAGE_SIZE - 1)), __imp_delay000);
    printf("%p %p %p\n", &__imp_delay200, (char*)((size_t)&__imp_delay200 & ~(PAGE_SIZE - 1)), __imp_delay200);
    event = CreateEventW(0, 0, 0, 0);
    thread1 = CreateThread(0, 0, &Thread1, 0, 0, 0);
    thread2 = CreateThread(0, 0, &Thread2, 0, 0, 0);
    delay200();
    return 0;
}
