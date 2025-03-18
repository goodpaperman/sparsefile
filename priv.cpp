#include "priv.h" 

BOOL EnablePrivilege(LPCTSTR szPrivilege, BOOL fEnable) {
    BOOL fOk = FALSE;
    HANDLE hToken = NULL;

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) {
        TOKEN_PRIVILEGES tp;
        tp.PrivilegeCount = 1;
        LookupPrivilegeValue(NULL, szPrivilege, &tp.Privileges[0].Luid);
        tp.Privileges->Attributes = fEnable ? SE_PRIVILEGE_ENABLED : 0;
        AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
        fOk = (GetLastError() == ERROR_SUCCESS);

        CloseHandle(hToken);
    }

    return fOk;
}