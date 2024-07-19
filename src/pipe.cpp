////////////////////////////////////////////////////////////////////////////////////////////////////
// Company:  Aceno Digital Tecnologia em Sistemas Ltda.
// Homepage: http://www.aceno.com
// Project:  C++ Interface TI Packet Sniffer
// Version:  1.0
// Date:     2024
//
// Copyright (C) 2002-2024 Aceno Tecnologia.
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
            D(std::cout << "[ERROR] Linux Pipe: mkfifo creation failed: " << strerror(errno) << std::endl;)
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
            // D(std::cout << "[ERROR] Linux Pipe: open failed: " << strerror(errno) << std::endl;)
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
            D(std::cout << "[ERROR] Windows Pipe: CreateNamedPipe failed" << std::endl;)
            return false;
        }

        if (!ConnectNamedPipe(pipeWrite, NULL))
        {
            D(std::cout << "[ERROR] Windows Pipe: ConnectNamedPipe failed" << std::endl;)
            CloseHandle(pipeWrite);
            pipeWrite = INVALID_PIPE_DESCRIPTOR;
            return false;
        }
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
void Pipe::write(const std::vector<uint8_t>& data)
{
    if (pipeWrite == INVALID_PIPE_DESCRIPTOR)
    {
        D(std::cout << "[ERROR] Pipe: Invalid pipe descriptor" << std::endl;)
        return;
    }

    #ifdef __linux__
        ssize_t result = ::write(pipeWrite, data.data(), data.size());
        if (result == -1)
        {
            D(std::cout << "[ERROR] Linux Pipe: write failed" << std::endl;)
        }
    #endif

    #ifdef _WIN32
        DWORD bytesWritten;
        if (!WriteFile(pipeWrite, data.data(), data.size(), &bytesWritten, NULL))
        {
            D(std::cout << "[ERROR] Windows Pipe: WriteFile failed" << std::endl;)
        }
    #endif
}