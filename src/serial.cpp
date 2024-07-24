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
#include <vector>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>  // for errno
#include <iostream>
#include <chrono>
#include <thread>

#include "common.hpp"
#include "serial.hpp"

Serial::Serial(std::string port)
{
    descriptor = INVALID_FILE_DESCRIPTOR;
    this->port = port;
}

bool Serial::connect()
{
    // Open serial port linux
    #ifdef __linux__
    D(std::cout << "[INFO] Opening serial port on LINUX: " << port << std::endl;)
    // Open serial port
    descriptor = open(port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    // Check if port was opened
    if (descriptor == INVALID_FILE_DESCRIPTOR) {
        D(std::cout << "[ERROR] Error opening serial port: " << port << std::endl;)
        return false;
    }
    // Set port configuration to 0 by default
    std::memset(&config, 0, sizeof(config));
    // Check if was able to get port configuration
    if (tcgetattr(descriptor, &config) != 0) {
        D(std::cout << "[ERROR] Error getting serial port configuration" << std::endl;);
        return false;
    }

    // Set the baud rates to 3_000_000
    cfsetispeed(&config, B3000000);
    cfsetospeed(&config, B3000000);
    // Enable the receiver and set local mode
    config.c_cflag |= (CLOCAL | CREAD);
    // Set 8N1 (8 data bits, no parity, 1 stop bit)
    config.c_cflag &= ~PARENB; // No parity
    config.c_cflag &= ~CSTOPB; // 1 stop bit
    config.c_cflag &= ~CSIZE; // Mask the character size bits
    config.c_cflag |= CS8; // 8 data bits
    // Disable hardware flow control
    // config.c_cflag &= ~CRTSCTS; // Change from c99 latter
    // Disable canonical mode, echo, and signal chars
    config.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    // Disable software flow control
    config.c_iflag &= ~(IXON | IXOFF | IXANY);
    // Disable special handling of bytes in output
    config.c_oflag &= ~OPOST;
    // Apply settings
    if (tcsetattr(descriptor, TCSANOW, &config) != 0) {
        D(std::cout << "[ERROR] Error setting serial port configuration" << std::endl;);
        return false;
    }

    #endif
    // Open serial port windows
    #ifdef _WIN32

    // Open serial port
    descriptor = CreateFile(port.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    // Check if port was opened
    if (descriptor == INVALID_FILE_DESCRIPTOR) {
        D(std::cout << "[ERROR] Error opening serial port: " << port << std::endl;)
        return false;
    }
    // Set port configuration to 0 by default
    std::memset(&config, 0, sizeof(config));
    // Get current port configuration
    if (!GetCommState(descriptor, &config)) {
        D(std::cout << "[ERROR] Error getting serial port configuration" << std::endl;);
        return false;
    }
    // Set the baud rates to 3_000_000
    config.BaudRate = CBR_3000000;
    // Set 8N1 (8 data bits, no parity, 1 stop bit)
    config.ByteSize = 8;
    config.Parity = NOPARITY;
    config.StopBits = ONESTOPBIT;
    // Apply settings
    if (!SetCommState(descriptor, &config)) {
        D(std::cout << "[ERROR] Error setting serial port configuration" << std::endl;);
        return false;
    }
    #endif

    return true;
}

bool Serial::disconnect()
{
    // Close serial port linux
    #ifdef __linux__
    D(std::cout << "[INFO] Closing serial port on LINUX: " << port << std::endl;)
    // Close serial port
    if (close(descriptor) != 0) {
        D(std::cout << "[ERROR] Error closing serial port" << std::endl;);
        return false;
    }
    #endif
    // Close serial port windows
    #ifdef _WIN32

    // Close serial port
    if (!CloseHandle(descriptor)) {
        D(std::cout << "[ERROR] Error closing serial port" << std::endl;);
        return false;
    }

    #endif

    return true;
}

bool Serial::writeData(std::vector<uint8_t> data)
{
    // Write to serial port linux
    #ifdef __linux__
    // Write data to serial port
    int bytes_written = write(descriptor, data.data(), data.size());
    // sleep_time(data.size());
    if (bytes_written != static_cast<int>(data.size())) {
        D(std::cout << "[ERROR] Error writing data to serial port" << std::endl;);
        return false;
    }
    #endif
    // Write to serial port windows
    #ifdef _WIN32

    DWORD bytes_written;
    if (!WriteFile(descriptor, data.data(), data.size(), &bytes_written, NULL)) {
        D(std::cout << "[ERROR] Error writing data to serial port" << std::endl;);
        return false;
    }
    if (bytes_written != data.size()) {
        D(std::cout << "[ERROR] Incomplete data written to serial port" << std::endl;);
        return false;
    }

    #endif

    return true;
}

bool Serial::writeData(uint8_t data)
{
    // Write byte to serial port linux
    #ifdef __linux__
    // Write byte to serial port
    if (write(descriptor, &data, 1) != 1) {
        // sleep_time(1);
        D(std::cout << "[ERROR] Error writing byte to serial port" << std::endl;);
        return false;
    }
    #endif
    // Write byte to serial port windows
    #ifdef _WIN32
    DWORD bytes_written;
    if (!WriteFile(descriptor, &data, 1, &bytes_written, NULL)) {
        D(std::cout << "[ERROR] Error writing byte to serial port" << std::endl;);
        return false;
    }
    if (bytes_written != 1) {
        D(std::cout << "[ERROR] Incomplete byte written to serial port" << std::endl;);
        return false;
    }
    #endif

    return true;
}

std::vector<uint8_t> Serial::readData()
{

    // Read data from serial port linux
    #ifdef __linux__
    // Read data from serial port
    int bytes_read = read(descriptor, buffer, BUFFER_SIZE);
    if (bytes_read == 0) {
        // End of file reached
        D(std::cout << "[INFO] There was no data to read from serial port" << std::endl;);
        return std::vector<uint8_t>();
    }
    if (bytes_read < 0) {
        D(std::cout << "[ERROR] Error reading data from serial port" << std::endl;);
        return std::vector<uint8_t>();
    }
    #endif
    // Read data from serial port windows
    #ifdef _WIN32
    DWORD bytes_read_dw;
    if (!ReadFile(descriptor, buffer, BUFFER_SIZE, &bytes_read_dw, NULL)) {
        D(std::cout << "[ERROR] Error reading data from serial port" << std::endl;);
        return std::vector<uint8_t>();
    }
    bytes_read = static_cast<int>(bytes_read_dw);
    #endif

    // Create a vector with the data read
    return std::vector<uint8_t>(buffer, buffer + bytes_read);
}

bool Serial::readByte(uint8_t* byte)
{
    // Read byte from serial port linux
    #ifdef __linux__
    // Read byte from serial port
    int bytes_read = read(descriptor, byte, 1);
    if (bytes_read > 0) {
        return true;
    }
    #endif
    // Read byte from serial port windows
    #ifdef _WIN32
    DWORD bytes_read_dw;
    if (!ReadFile(descriptor, byte, 1, &bytes_read_dw, NULL)) {
        D(std::cout << "[ERROR] Error reading byte from serial port" << std::endl;);
        return false;
    }
    if (bytes_read_dw == 1) {
        return true;
    }
    #endif

    return false;
}

void Serial::flush()
{
    // Flush serial port linux
    #ifdef __linux__
    // Flush serial port
    tcdrain(descriptor);
    #endif
    // Flush serial port windows
    #ifdef _WIN32
    FlushFileBuffers(descriptor);
    #endif
}

void Serial::purge()
{
    // Purge serial port linux
    #ifdef __linux__
    // Purge serial port
    tcflush(descriptor, TCIOFLUSH);
    #endif
    // Purge serial port windows
    #ifdef _WIN32
    PurgeComm(descriptor, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR);
    #endif
}

bool Serial::is_connected()
{
    return (descriptor != INVALID_FILE_DESCRIPTOR);
}