// build: cl /nologo /EHsc /MT /O2 /utf-8 probe5.cpp advapi32.lib
#include <windows.h>
#include <stdio.h>

static const char *ErrName(LONG rc)
{
    switch (rc) {
    case ERROR_SUCCESS:            return "ERROR_SUCCESS";
    case ERROR_FILE_NOT_FOUND:     return "ERROR_FILE_NOT_FOUND";
    case ERROR_PATH_NOT_FOUND:     return "ERROR_PATH_NOT_FOUND";
    case ERROR_ACCESS_DENIED:      return "ERROR_ACCESS_DENIED";
    case ERROR_INVALID_PARAMETER:  return "ERROR_INVALID_PARAMETER";
    case ERROR_SHARING_VIOLATION:  return "ERROR_SHARING_VIOLATION";
    default:                       return "(other)";
    }
}

static void Hop(const char *tag, const char *sub)
{
    HKEY h = NULL;
    LONG rc = RegOpenKeyExA(HKEY_LOCAL_MACHINE, sub, 0, KEY_READ, &h);
    printf("%-5s HKLM\\%-45s rc=%-2ld 0x%08lX  %s\n", tag, sub, rc, (DWORD)rc, ErrName(rc));
    if (h) RegCloseKey(h);
}

int main()
{
    Hop("[1]", "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion");
    Hop("[2]", "SAM");
    Hop("[3]", "SAM\\SAM");
    Hop("[4]", "SAM\\SAM\\Domains\\Account\\Users");
    return 0;
}
