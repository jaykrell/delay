#define PAGE_SIZE 0x1000
#include <stdio.h>
#include "delay.h"
#include "windows.h"
#include <assert.h>

HANDLE event, thread1, thread2;
void* initial_imp_delay000;

void ExceptionHandler()
// Step until __imp_delay000 changes. Then let thread2 resume and run forever.
// With the bug, thread2 should access violate.
{
    if (initial_imp_delay000 != __imp_delay000)
    {
        SetEvent(event);
        Sleep(-1);
        return;
    }
}

DWORD Thread1(PVOID p)
{
    delay000();
    return 0;
}

DWORD Thread2(PVOID p)
{
    WaitForSingleObject(event, -1);
    delay200();
    return 0;
}

// Reference in order, to establish order, to establish page separation.
#include "delayref.c"

void* page_of(void* p)
{
    return (char*)((size_t)p & ~(PAGE_SIZE - 1));
}

int main()
{
    // delayload import address table pages.
    void* diat_page0 = page_of(&__imp_delay000);
    void* diat_page1 = page_of(&__imp_delay200);

    assert(diat_page0 != diat_page1);

    initial_imp_delay000 = __imp_delay000;

    event = CreateEventW(0, 0, 0, 0);
    thread1 = CreateThread(0, 0, &Thread1, 0, 0, 0);
    thread2 = CreateThread(0, 0, &Thread2, 0, 0, 0);

    return 0;
}
