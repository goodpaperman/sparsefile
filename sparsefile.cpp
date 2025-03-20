// sparsefile.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
#include "c_timer.h"
#include "priv.h"

int USE_FILE_END_OF_FILE_INFO = 0; 
int USE_SET_FILE_VALID_DATA = 0;
int USE_SPARSE_FILE = 1; 
int WRITE_FILE_MODE = 0;
char FILL_CHAR = '\0'; 

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cout << "Usage: sparsefile file length(in GB) [set-file-end-of-file-info] [set-file-valid-data] [sparse-file] [write-file-mode 0|1|2] [fill-char] [write-position(in GB)]\n";
        return 1; 
    }

    int ret = 0; 
    HANDLE file_handle = INVALID_HANDLE_VALUE;
    LARGE_INTEGER off = { 0 };
    long long file_size = atoll(argv[2]) * 1024 * 1024 * 1024;  // unit in GB
    off.QuadPart = file_size - 1; 

    if (argc > 3)
        USE_FILE_END_OF_FILE_INFO = atoi(argv[3]);

    if (argc > 4)
        USE_SET_FILE_VALID_DATA = atoi(argv[4]);

    if (argc > 5)
        USE_SPARSE_FILE = atoi(argv[5]);

    if (argc > 6)
        WRITE_FILE_MODE = atoi(argv[6]);

    if (argc > 7)
        FILL_CHAR = argv[7][0];

    if (argc > 8)
        off.QuadPart = atoll(argv[8]) * 1024 * 1024 * 1024;  // unit in GB

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

        if (USE_FILE_END_OF_FILE_INFO)
        {
            FILE_END_OF_FILE_INFO info;
            info.EndOfFile.QuadPart = file_size;
            if (!SetFileInformationByHandle(file_handle, FileEndOfFileInfo, &info, sizeof(info)))
            {
                std::cout << "SetFileInformationByHandle failed, error " << GetLastError() << std::endl;
                ret = 3;
                break;
            }
        }
        else
        {
            LARGE_INTEGER pos; 
            pos.QuadPart = file_size;
            if (::SetFilePointerEx(file_handle, pos, NULL, FILE_BEGIN) == 0)
            {
                std::cout << "SetFilePointerEx failed, error " << GetLastError() << std::endl;
                ret = 4;
                break;
            }

            if (!::SetEndOfFile(file_handle))
            {
                std::cout << "SetEndOfFile failed, error " << GetLastError() << std::endl;
                ret = 5;
                break;
            }
        }

        if (USE_SET_FILE_VALID_DATA)
        {
            if (!::SetFileValidData(file_handle, file_size - 1))
            {
                std::cout << "SetFileValidData failed, error " << GetLastError() << std::endl;
                ret = 11;
                break;
            }
        }

        if (USE_SPARSE_FILE)
        {
            DWORD temp = 0;
            if (!::DeviceIoControl(file_handle, FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &temp, NULL))
            {
                std::cout << "DeviceIoControl failed, error " << GetLastError() << std::endl;
                ret = 20;
                break;
            }

            std::cout << "set sparse file ok" << std::endl;
        }

        DWORD bytes = 0;
        if (WRITE_FILE_MODE == 1)
        {
            // write a byte at end
            if (::SetFilePointerEx(file_handle, off, NULL, FILE_BEGIN) == 0)
            {
                std::cout << "SetFilePointerEx failed, error " << GetLastError() << std::endl;
                ret = 30;
                break;
            }

            c_timer t;
            char buf[1] = { FILL_CHAR };
            if (!::WriteFile(file_handle, buf, 1, &bytes, NULL) || bytes != 1)
            {
                std::cout << "WriteFile failed, error " << GetLastError() << ", written " << bytes << std::endl;
                ret = 31;
                break;
            }

            int elapse = t.get_interval();
            std::cout << "write file end a byte elapse " << elapse << " ms, position " << off.QuadPart << std::endl;
        }
        else if (WRITE_FILE_MODE == 2)
        {
            // write whole file
            c_timer t;
            char buf[1] = { FILL_CHAR };
            for (long long i = 0; i < file_size; i += 1024*1024 /* write every 1M */)
            {
                off.QuadPart = i;
                if (::SetFilePointerEx(file_handle, off, NULL, FILE_BEGIN) == 0)
                {
                    std::cout << "SetFilePointerEx failed, error " << GetLastError() << std::endl;
                    ret = 32;
                    break;
                }

                if (!::WriteFile(file_handle, buf, 1, &bytes, NULL) || bytes != 1)
                {
                    std::cout << "WriteFile failed, error " << GetLastError() << ", written " << bytes << std::endl;
                    ret = 33;
                    break;
                }
            }

            int elapse = t.get_interval();
            std::cout << "write whole file elapse " << elapse << " ms" << std::endl;
        }
    } while (0); 

    if (ret == 0)
        std::cout << "create file with length " << file_size << " success, path " << argv[1] << std::endl;

    if (file_handle)
        CloseHandle(file_handle); 

    return ret; 
}
