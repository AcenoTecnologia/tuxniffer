////////////////////////////////////////////////////////////////////////////////////////////////////
// Company:  Aceno Digital Tecnologia em Sistemas Ltda.
// Homepage: http://www.aceno.com
// Project:  Tuxniffer
// Version:  1.1
// Date:     2025
//
// Copyright (C) 2002-2025 Aceno Tecnologia.
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////


#include <string>
#include <string.h>
#include <fstream>
#include <sstream>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#endif

#include "common.hpp"

char* custom_strerror(int n_error){
    
    char *res;
    #ifdef __WIN32
        if (n_error == 0)
        {
            n_error = GetLastError();

            if (n_error == 0)
            {
                res = (char*)malloc(sizeof(char));
                strcpy(res, "");
                return res;
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
            char buf[128];
            strerror_s(buf, sizeof(buf), n_error);
            res = (char*)malloc(sizeof(char)*(strlen(buf) + 4));
            sprintf(res, " - %s", buf); 
        }
    #else
        char buf[128];
        if (n_error == 0)
        {
            res = (char*)malloc(sizeof(char));
            strcpy(res, "");
            return res;
        }
        #if ((_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE)
            strerror_r(n_error, buf, sizeof(buf));
            res = (char*)malloc(sizeof(char)*(strlen(buf) + 4));
            sprintf(res, " - %s", buf);
        #else
            char* auxStr = strerror_r(n_error, buf, sizeof(buf));
            res = (char*)malloc(sizeof(char)*(strlen(auxStr) + 4));
            sprintf(res, " - %s", auxStr);  
        #endif
    #endif
    return res;
    
    
    
}