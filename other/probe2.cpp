// build: cl /nologo /EHsc /MT /O2 probe2.cpp vssapi.lib ole32.lib
#include <windows.h>
#include <vss.h>
#include <vswriter.h>
#include <vsbackup.h>
#include <stdio.h>

int main()
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    printf("[1] CoInitializeEx            hr=0x%08lX\n", hr);          fflush(stdout);

    IVssBackupComponents *p = NULL;
    hr = CreateVssBackupComponents(&p);
    printf("[2] CreateVssBackupComponents hr=0x%08lX\n", hr);          fflush(stdout);

    HMODULE h = GetModuleHandleW(L"vssapi.dll");
    printf("[3] vssapi.dll base           %p\n", (void *)h);            fflush(stdout);

    Sleep(3000);
    if (p) p->Release();
    CoUninitialize();
    return 0;
}
