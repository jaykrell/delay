#define PAGE_SIZE 0x1000
#include <stdio.h>
#include "delay.h"
#include "windows.h"
#include <assert.h>
#define TRAP_FLAG 0x100
#include "excpt.h"
void DbgPrintEx(int, int, const char*, ...);

#define VEH 1
#define SEH 0

DWORD threadId[3];
HANDLE event, thread[3];
void* volatile initial_imp_delay000;
__int64 sequence;

void Print(const char* a, ...)
{
    va_list va;
    va_start(va, a);
#if 1
    char buf[999] = {0};
    DWORD dw = {0};
    vsprintf(buf, a, va);
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), buf, strlen(buf), &dw, 0);
#else
    // This deadlocks due to critical section reentrance.
    vprintf(a, va);
#endif
}

void SleepInfinite()
{
    Print("SleepInfinite\n");
    SetEvent(event);
    Sleep(INFINITE);
}

// Stepping with vectored exception handling does not work, due to reentrance (RtlEnterCriticalSection).
//// Therefore instead we detour RtlDispatchException?
//// Try SEH instead (which depends on a bugfix I made a few years ago, to step through epilogs).
//
//// This also makes it OK to Sleep here.
//
long ExceptionHandler(PEXCEPTION_POINTERS exception)
// Step thread1 until __imp_delay000 changes. Then let thread2 resume and run forever.
// With the bug, thread2 should access violate.
{
    //Print("ExceptionHandler: %I64x %X %p %X\n", InterlockedIncrement64(&sequence), GetCurrentThreadId(), (void*)exception->ContextRecord->Rip, *(unsigned char*)exception->ContextRecord->Rip);
    if (GetCurrentThreadId() != threadId[1])
    {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    //Print("ExceptionHandler: %I64x %X %X %p %X\n", InterlockedIncrement64(&sequence), GetCurrentThreadId(), exception->ExceptionRecord->ExceptionCode, (void*)exception->ContextRecord->Rip, *(unsigned char*)exception->ContextRecord->Rip);
    if (initial_imp_delay000 != __imp_delay000)
    {
        Print("initial_imp_delay000 != __imp_delay000: %p %p\n", initial_imp_delay000, __imp_delay000);
#if VEH
        // make home space
        for (int i = 0; i < 0x20; ++i)
        {
            *(volatile char*)exception->ContextRecord->Rsp;
            --exception->ContextRecord->Rsp;
            *(volatile char*)exception->ContextRecord->Rsp;
        }
        exception->ContextRecord->Rip = (size_t)&SleepInfinite;
#else
        SleepInfinite();
#endif
        //if (IsDebuggerPresent()) __debugbreak();
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    exception->ContextRecord->EFlags |= TRAP_FLAG;
    return EXCEPTION_CONTINUE_EXECUTION;
}

#define SetTrapFlag() \
    __writeeflags(__readeflags() | TRAP_FLAG)

DWORD Thread1(PVOID p)
{
    __try
    {
        Print("Thread1 %d\n", __LINE__);
        SetTrapFlag();
        Print("Thread1 %d\n", __LINE__);
        delay000();
        Print("Thread1 %d Should not get here\n", __LINE__);
    }
#if SEH
    __except(ExceptionHandler(GetExceptionInformation())) { }
#else
    __except(1) { }
#endif
    return 0;
}

DWORD Thread2(PVOID p)
{
    DWORD exceptionCode = {0};
    Print("Thread2 %d\n", __LINE__);
    WaitForSingleObject(event, INFINITE);
    Print("Thread2 %d\n", __LINE__);
    __try
    {
        delay200();
    }
    __except(
        Print("line%d:%X\n", __LINE__, GetExceptionCode()),
        TerminateProcess(GetCurrentProcess(), GetExceptionCode()), 1)
    {
    }
    Print("Thread2 %d\n", __LINE__);
    TerminateThread(thread[1], 0);
    Print("Thread2 %d\n", __LINE__);
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

#if VEH
    AddVectoredExceptionHandler(0, ExceptionHandler);
#endif
    event = CreateEventW(0, 0, 0, 0);
    threadId[0] = GetCurrentThreadId();
    thread[1] = CreateThread(0, 0, &Thread1, 0, CREATE_SUSPENDED, &threadId[1]);
    thread[2] = CreateThread(0, 0, &Thread2, 0, CREATE_SUSPENDED, &threadId[2]);
    Print("threads %X %X %X\n", GetCurrentThreadId(), threadId[1], threadId[2]);
    ResumeThread(thread[1]);
    ResumeThread(thread[2]);

    WaitForMultipleObjects(2, &thread[1], TRUE, INFINITE);

    // This does not work with the buggy delayimp.lib.
    DWORD exitCode = 0;
    GetExitCodeThread(thread[2], &exitCode);
    Print("exitCode: %X\n", exitCode);

    return exitCode;
}
