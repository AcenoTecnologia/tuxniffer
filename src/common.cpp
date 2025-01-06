////////////////////////////////////////////////////////////////////////////////////////////////////
// Company:  Aceno Digital Tecnologia em Sistemas Ltda.
// Homepage: http://www.aceno.com
// Project:  Tuxniffer
// Version:  1.0
// Date:     2024
//
// Copyright (C) 2002-2024 Aceno Tecnologia.
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////


#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <windows.h>

#include "common.hpp"

char* custom_strerror(int n_error){
    
    char buf[128];
    #ifndef __WIN32__
        if (n_error == 0)
        {
            n_error = GetLastError();

            if (n_error == 0)
            {
                return "";
            }
            
            char* messageBuffer = nullptr;

            size_t size = FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr,
                n_error,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPSTR)&messageBuffer,
                0,
                nullptr
            );
            sprintf(buf, " - %s", messageBuffer);
            LocalFree(messageBuffer); // Libera o buffer alocado pelo FormatMessage
            return buf;
        } 
        else
        {
            strerror_s(buf, sizeof(buf), n_error);
        }
    #else
        if (n_error == 0)
        {
            return "";
        }
        #ifndef (POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE
            strerror_r(n_error, buf, sizeof(buf));
        #else
            buf = strerror_r(n_error, buf, sizeof(buf));
        #endif
    #endif
    
    char res[128];
    sprintf(res, " - %s", buf);
    return res;
}