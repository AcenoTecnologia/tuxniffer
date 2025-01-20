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


#pragma once

#include <vector>
#include <cstdint> // for uint8_t
#include <string>  // for std::string

#ifdef __linux__
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #define PIPE_DESCRIPTOR int
    #define INVALID_PIPE_DESCRIPTOR -1
#endif

#ifdef _WIN32
    #include <windows.h>
    #define PIPE_DESCRIPTOR HANDLE
    #define INVALID_PIPE_DESCRIPTOR INVALID_HANDLE_VALUE
#endif

/**
 * @class Pipe
 * @brief A class for handling named pipes (FIFOs) for inter-process communication.
 */
class Pipe
{
public:
    /**
     * @brief Constructs a Pipe object.
     */
    Pipe();

    /**
     * @brief Destroys the Pipe object and closes the pipe if it's open.
     */
    ~Pipe();

    /**
     * @brief Creates a named pipe (FIFO) or a named pipe in Windows.
     * 
     * On Linux, this function creates a named pipe (FIFO) at the specified path.
     * On Windows it is not necessary to create a named pipe before opening it. This function is a no-op on Windows.
     * 
     * @param pipePath The path or name of the pipe to be created.
     * @return true if the pipe was successfully created, false otherwise.
     */
    bool create(const std::string& pipePath);

    /**
     * @brief Opens an existing named pipe (FIFO) or a named pipe in Windows.
     * 
     * On Linux, this function opens a FIFO with write-only access.
     * On Windows, it creates a named pipe in the `\\.\pipe\` namespace and opens it for writing.
     * 
     * @param pipePath The path or name of the pipe to be opened.
     * @return true if the pipe was successfully opened, false otherwise.
     */
    bool open(const std::string& pipePath);

    /**
     * @brief Writes data to the pipe.
     * 
     * @param data A vector of bytes to be written to the pipe.
     * @return true if the data was successfully written on the pipe, false otherwise.
     */
    bool write(const std::vector<uint8_t>& data);

    /**
     * @brief Closes the pipe.
     * 
     * @return true if the pipe was successfully closed, false otherwise.
     */
    bool close();


private:
    PIPE_DESCRIPTOR pipeWrite; /**< The pipe descriptor for writing. */
};

