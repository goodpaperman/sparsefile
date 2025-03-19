// sparsefile.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
#include "c_timer.h"
#include "priv.h"

int USE_SET_FILE_VALID_DATA = 0;
int WRITE_WHOLE_FILE = 0;
char FILL_CHAR = '\0'; 

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cout << "Usage: sparsefile file length(in GB) [set-file-valid-data] [write-whole-file] [fill-char]\n";
        return 1; 
    }

    int ret = 0; 
    HANDLE file_handle = INVALID_HANDLE_VALUE;
    LARGE_INTEGER pos = { 0 };

    if (argc > 3)
        USE_SET_FILE_VALID_DATA = atoi(argv[3]);

    if (argc > 4)
        WRITE_WHOLE_FILE = atoi(argv[4]);

    if (argc > 5)
        FILL_CHAR = argv[5][0];

    do
    {
        if (USE_SET_FILE_VALID_DATA)
        {
            if (!EnablePrivilege(SE_MANAGE_VOLUME_NAME, TRUE))
            {
                std::cout << "EnablePrivilege failed, error " << GetLastError() << std::endl;
                ret = 10;
                break;
            }
        }

        file_handle = CreateFileA(argv[1], (GENERIC_READ | GENERIC_WRITE),
            FILE_SHARE_READ, 0, OPEN_ALWAYS, 0, 0);
        if (file_handle == INVALID_HANDLE_VALUE)
        {
            std::cout << "CreateFile failed, error " << GetLastError() << std::endl;
            ret = 2;
            break;
        }

        pos.QuadPart = atoll(argv[2]) * 1024 * 1024 * 1024;  // unit in GB
        if (::SetFilePointerEx(file_handle, pos, NULL, FILE_BEGIN) == 0)
        {
            std::cout << "SetFilePointerEx failed, error " << GetLastError() << std::endl;
            ret = 3; 
            break;
        }

        if (!::SetEndOfFile(file_handle))
        {
            std::cout << "SetEndOfFile failed, error " << GetLastError() << std::endl;
            ret = 4; 
            break;
        }

        if (USE_SET_FILE_VALID_DATA)
        {
            if (!::SetFileValidData(file_handle, pos.QuadPart - 1))
            {
                std::cout << "SetFileValidData failed, error " << GetLastError() << std::endl;
                ret = 11;
                break;
            }
        }
        else
        {
            DWORD temp = 0;
            if (!::DeviceIoControl(file_handle, FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &temp, NULL))
            {
                std::cout << "DeviceIoControl failed, error " << GetLastError() << std::endl;
                ret = 12;
                break;
            }
        }

        if (WRITE_WHOLE_FILE)
        {
            // write whole file
            long long file_size = pos.QuadPart;
            pos.QuadPart = 0;
            if (::SetFilePointerEx(file_handle, pos, NULL, FILE_BEGIN) == 0)
            {
                std::cout << "SetFilePointerEx failed, error " << GetLastError() << std::endl;
                ret = 20;
                break;
            }

            char buf[4096] = { FILL_CHAR }; 
            c_timer t;
            DWORD bytes = 0;
            for (long long i = 0; i < file_size; i += 4096)
            {
                if (!::WriteFile(file_handle, buf, 4096, &bytes, NULL) || bytes != 4096)
                {
                    std::cout << "WriteFile failed, error " << GetLastError() << ", written " << bytes << std::endl;
                    ret = 21;
                    break;
                }
            }

            int elapse = t.get_interval();
            std::cout << "write file elapse " << elapse << " ms" << std::endl;
        }
        else
        {
            // write a byte at end
            pos.QuadPart--;
            if (::SetFilePointerEx(file_handle, pos, NULL, FILE_BEGIN) == 0)
            {
                std::cout << "SetFilePointerEx failed, error " << GetLastError() << std::endl;
                ret = 5;
                break;
            }

            DWORD bytes = 0;
            c_timer t;
            char buf[1] = { FILL_CHAR }; 
            if (!::WriteFile(file_handle, buf, 1, &bytes, NULL) || bytes != 1)
            {
                std::cout << "WriteFile failed, error " << GetLastError() << ", written " << bytes << std::endl;
                ret = 6;
                break;
            }

            int elapse = t.get_interval();
            std::cout << "write file elapse " << elapse << " ms" << std::endl;
        }
    } while (0); 

    if (ret == 0)
        std::cout << "create file with length " << pos.QuadPart << " success, path " << argv[1] << std::endl;

    if (file_handle)
        CloseHandle(file_handle); 

    return ret; 
}
