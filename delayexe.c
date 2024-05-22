#define PAGE_SIZE 0x1000
#include <stdio.h>
#include "delay.h"
#include "windows.h"
#include <assert.h>

DWORD threadId[3];
HANDLE event, thread1, thread2;
void* initial_imp_delay000;

void LocalSleep()
{
    Sleep(-1);
}

long ExceptionHandler(PEXCEPTION_POINTERS exception)
// Step thread1 until __imp_delay000 changes. Then let thread2 resume and run forever.
// With the bug, thread2 should access violate.
// 
{
    printf("ExceptionHandler: %d %p %X\n", GetCurrentThreadId(), (void*)exception->ContextRecord->Rip, *(unsigned char*)exception->ContextRecord->Rip);
    if (GetCurrentThreadId() != threadId[1])
    {
        //return EXCEPTION_CONTINUE_SEARCH;
    }
    if (initial_imp_delay000 != __imp_delay000)
    {
        SetEvent(event);
        //exception->ContextRecord->Rip = (size_t)&LocalSleep;
        //return EXCEPTION_CONTINUE_EXECUTION;
    }
    //exception->ContextRecord->EFlags = (size_t)&LocalSleep;
    ++(exception->ContextRecord->Rip);
    return EXCEPTION_CONTINUE_EXECUTION;
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

    AddVectoredExceptionHandler(0, ExceptionHandler);
    event = CreateEventW(0, 0, 0, 0);
    threadId[0] = GetCurrentThreadId();
    thread1 = CreateThread(0, 0, &Thread1, 0, CREATE_SUSPENDED, &threadId[1]);
    thread2 = CreateThread(0, 0, &Thread2, 0, CREATE_SUSPENDED, &threadId[2]);
    ResumeThread(thread1);
    ResumeThread(thread2);

    __debugbreak();
    __debugbreak();
    __debugbreak();
    SetEvent(event);

    return 0;
}
