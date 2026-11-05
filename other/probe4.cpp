// build: cl /nologo /EHsc /MT /O2 /utf-8 probe4.cpp vssapi.lib ole32.lib
#include <windows.h>
#include <vss.h>
#include <vswriter.h>
#include <vsbackup.h>
#include <stdio.h>

static const wchar_t *Why(DWORD e)
{
    switch (e) {
    case 2:  return L"ERROR_FILE_NOT_FOUND";
    case 3:  return L"ERROR_PATH_NOT_FOUND";
    case 5:  return L"ERROR_ACCESS_DENIED";
    case 32: return L"ERROR_SHARING_VIOLATION";
    case 87: return L"ERROR_INVALID_PARAMETER";
    default: return L"None message";
    }
}

static void TryRead(const wchar_t *path, const wchar_t *tag)
{
    HANDLE h = CreateFileW(path, GENERIC_READ,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD err = GetLastError();
    wprintf(L"%-7ls %-4ls %ls\n", tag,
            h == INVALID_HANDLE_VALUE ? L"FAIL" : L"OK", path);
    if (h == INVALID_HANDLE_VALUE) {
        wprintf(L"        GetLastError=%lu (0x%08lX)\n", err, err);
        wprintf(L"        %ls\n", Why(err));
    } else {
        char buf[128] = {0};
        DWORD n = 0;
        ReadFile(h, buf, sizeof(buf) - 1, &n, NULL);
        wprintf(L"        first %lu bytes: %hs\n", n, buf);
        CloseHandle(h);
    }
    fflush(stdout);
}

int main()
{
    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    TryRead(L"C:\\Windows\\System32\\drivers\\etc\\hosts", L"normal");

    IVssBackupComponents *p = NULL;
    HRESULT hr = CreateVssBackupComponents(&p);
    printf("[1] create        hr=0x%08lX\n", hr);              fflush(stdout);
    if (FAILED(hr)) return 1;

    hr = p->InitializeForBackup();
    printf("[2] initBackup    hr=0x%08lX\n", hr);              fflush(stdout);

    hr = p->SetContext(VSS_CTX_CLIENT_ACCESSIBLE);
    printf("[3] setContext    hr=0x%08lX\n", hr);              fflush(stdout);

    VSS_ID setId = GUID_NULL, snapId = GUID_NULL;
    hr = p->StartSnapshotSet(&setId);
    printf("[4] startSet      hr=0x%08lX\n", hr);              fflush(stdout);

    hr = p->AddToSnapshotSet(L"C:\\", GUID_NULL, &snapId);
    printf("[5] addToSet(C:)  hr=0x%08lX\n", hr);              fflush(stdout);

    IVssAsync *pAsync = NULL;
    hr = p->DoSnapshotSet(&pAsync);
    printf("[6] doSnapshot    hr=0x%08lX (only)\n", hr);    fflush(stdout);
    if (SUCCEEDED(hr) && pAsync) {
        hr = pAsync->Wait(); 
        pAsync->Release();
    }
    printf("[6b] wait         hr=0x%08lX\n", hr);               fflush(stdout);
    if (FAILED(hr)) { p->Release(); CoUninitialize(); return 2; }

    VSS_SNAPSHOT_PROP prop = {0};
    BOOL got = SUCCEEDED(p->GetSnapshotProperties(snapId, &prop));
    wprintf(L"[7] device        %ls\n", got ? prop.m_pwszSnapshotDeviceObject : L"(failed)");
    fflush(stdout);
    if (!got) { p->Release(); CoUninitialize(); return 3; }

    Sleep(10000);

    wchar_t shadow[MAX_PATH];
    swprintf_s(shadow, MAX_PATH, L"%ls\\Windows\\System32\\drivers\\etc\\hosts",
               prop.m_pwszSnapshotDeviceObject);

    wprintf(L"[8] wait          10s\n");                          fflush(stdout);
    TryRead(shadow, L"shadow");

    LONG deleted = 0; VSS_ID notDeleted = GUID_NULL;
    hr = p->DeleteSnapshots(snapId, VSS_OBJECT_SNAPSHOT, TRUE, &deleted, &notDeleted);
    printf("[9] delete        hr=0x%08lX deleted=%ld\n", hr, deleted);  fflush(stdout);

    VssFreeSnapshotProperties(&prop);
    p->Release();
    CoUninitialize();
    return 0;
}
