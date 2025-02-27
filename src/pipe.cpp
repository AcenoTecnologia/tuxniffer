////////////////////////////////////////////////////////////////////////////////////////////////////
// Company:  Aceno Digital Tecnologia em Sistemas Ltda.
// Homepage: http://www.aceno.com
// Project:  Tuxniffer
// Version:  1.1.3
// Date:     2025
//
// Copyright (C) 2002-2025 Aceno Tecnologia.
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////


#include "pipe.hpp"
#include <iostream>
#include <cstring> // for std::memcpy
#include <errno.h>
#include "common.hpp"

#ifdef __linux__
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif

// Constructor initializes pipeWrite to INVALID_PIPE_DESCRIPTOR.
Pipe::Pipe() : pipeWrite(INVALID_PIPE_DESCRIPTOR) {}

// Destructor closes the pipe if it's open.
Pipe::~Pipe()
{
    close();
}

// Creates a new named pipe or FIFO.
bool Pipe::create(const std::string& pipePath)
{
    #ifdef __linux__
        // Delete the FIFO file if it already exists.
        unlink(pipePath.c_str());

        // Create a FIFO with read and write permissions.
        int response = mkfifo(pipePath.c_str(), 0666);
        if (response == -1)
        {
            D(char* errmsg = custom_strerror(errno);
                std::cout << "[ERROR] Linux Pipe: mkfifo creation failed" << errmsg << "." << std::endl;
                free(errmsg);)
            return false;
        }
    #endif

    #ifdef _WIN32
        // Named pipes in Windows are created on demand.
        // No separate creation step is needed.
        // We can just proceed to open it later.
    #endif

    return true;
}

// Opens an existing named pipe or FIFO.
bool Pipe::open(const std::string& pipePath)
{
    #ifdef __linux__
        pipeWrite = ::open(pipePath.c_str(), O_WRONLY | O_NONBLOCK);
        if (pipeWrite == INVALID_PIPE_DESCRIPTOR)
        {
            //D(char* errmsg = custom_strerror(errno); std::cout << "[ERROR] Linux Pipe: open failed" << errmsg << "." << std::endl; free(errmsg);)
            return false;
        }
    #endif

    #ifdef _WIN32
        pipeWrite = CreateNamedPipe(
            pipePath.c_str(),
            PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            1,
            65536,
            65536,
            0,
            NULL
        );
        if (pipeWrite == INVALID_PIPE_DESCRIPTOR)
        {
            D(char* errmsg = custom_strerror(errno);
                std::cout << "[ERROR] Windows Pipe: CreateNamedPipe failed" << errmsg << "." << std::endl;
                free(errmsg);)
            return false;
        }
        OVERLAPPED overlapped = {};
        overlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

        if (!overlapped.hEvent) {
            std::cerr << "Erro ao criar o evento overlapped: " << GetLastError() << std::endl;
            CloseHandle(pipeWrite);
            pipeWrite = INVALID_PIPE_DESCRIPTOR;
            return false;
        }
        if (!ConnectNamedPipe(pipeWrite, &overlapped))
        {
            DWORD error = GetLastError();
            if (error == ERROR_IO_PENDING) {
                // Espera até que a conexão seja estabelecida ou o timeout expire
                DWORD waitResult = WaitForSingleObject(overlapped.hEvent, 2000); // Timeout de 2 segundos
                if (waitResult != WAIT_OBJECT_0) {
                    if (waitResult != WAIT_TIMEOUT) {
                        D(char* errmsg = custom_strerror(errno);
                            std::cout << "[ERROR] Windows Pipe: ConnectNamedPipe failed" << errmsg << "." << std::endl;
                            free(errmsg);)
                    }
                    CloseHandle(overlapped.hEvent);
                    CloseHandle(pipeWrite);
                    pipeWrite = INVALID_PIPE_DESCRIPTOR;
                    return false;
                }
            }
        }
        CloseHandle(overlapped.hEvent);
    #endif
    return true;
}

// Closes the pipe.
bool Pipe::close()
{
    if (pipeWrite != INVALID_PIPE_DESCRIPTOR)
    {
        #ifdef __linux__
            ::close(pipeWrite);
        #endif
        #ifdef _WIN32
            CloseHandle(pipeWrite);
        #endif
        pipeWrite = INVALID_PIPE_DESCRIPTOR;
    }

    return true;
}

// Writes data to the pipe.
bool Pipe::write(const std::vector<uint8_t>& data)
{
    if (pipeWrite == INVALID_PIPE_DESCRIPTOR)
    {
        D(std::cout << "[ERROR] Pipe: Invalid pipe descriptor." << std::endl;)
        return false;
    }

    #ifdef __linux__
        ssize_t result = ::write(pipeWrite, data.data(), data.size());
        if (result == -1)
        {
            D(char* errmsg = custom_strerror(errno);
                std::cout << "[ERROR] Linux Pipe: write failed" << errmsg << "." << std::endl;
                free(errmsg);)
            return false;
        }
    #endif

    #ifdef _WIN32
        DWORD bytesWritten;
        if (!WriteFile(pipeWrite, data.data(), data.size(), &bytesWritten, NULL))
        {
            D(char* errmsg = custom_strerror(errno);
                std::cout << "[ERROR] Windows Pipe: WriteFile failed" << errmsg << "." << std::endl;
                free(errmsg);)
            return false;
        }
    #endif
    return true;
}

bool Pipe::isPipeOpen() {
    #ifdef _WIN32
    // Verifica o estado do handle do pipe
        if (GetNamedPipeHandleStateA(pipeWrite, NULL, NULL, NULL, NULL, NULL, 0)) {
            return true; // O pipe ainda está válido
        } else {
            DWORD error = GetLastError();
            if (error == ERROR_BROKEN_PIPE || error == ERROR_INVALID_HANDLE) {
                return false; // O pipe foi fechado ou não é mais válido
            }
        }
        return false;
    #endif
    #ifdef __linux__
        if (fcntl(pipeWrite, F_GETFD) == -1) {
            if (errno == EBADF) {
                return false; // O descritor é inválido, o pipe está fechado
            }
        }
        return true; // O pipe está aberto
    #endif
}