// build: cl /nologo /EHsc /MT /O2 /utf-8 probe3.cpp vssapi.lib ole32.lib
#include <windows.h>
#include <vss.h>
#include <vswriter.h>
#include <vsbackup.h>
#include <stdio.h>

int main()
{
    CoInitializeEx(NULL, COINIT_MULTITHREADED);

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
    if (FAILED(hr)) { printf("check err code\n"); p->Release(); CoUninitialize(); return 2; }

    VSS_SNAPSHOT_PROP prop = {0};
    BOOL got = SUCCEEDED(p->GetSnapshotProperties(snapId, &prop));
    wprintf(L"[7] device        %ls\n", got ? prop.m_pwszSnapshotDeviceObject : L"(failed)");
    fflush(stdout);

    Sleep(5000);

    LONG deleted = 0; VSS_ID notDeleted = GUID_NULL;
    hr = p->DeleteSnapshots(snapId, VSS_OBJECT_SNAPSHOT, TRUE, &deleted, &notDeleted);
    printf("[8] delete        hr=0x%08lX deleted=%ld\n", hr, deleted);  fflush(stdout);

    if (got) VssFreeSnapshotProperties(&prop);
    p->Release();
    CoUninitialize();
    return 0;
}
