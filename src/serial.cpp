////////////////////////////////////////////////////////////////////////////////////////////////////
// Company:  Aceno Digital Tecnologia em Sistemas Ltda.
// Homepage: http://www.aceno.com
// Project:  C++ Interface TI Packet Sniffer
// Version:  1.0
// Date:     2024
//
// Copyright (C) 2002-2024 Aceno Tecnologia.
// Todos os direitos reservados.
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

Serial::Serial(std::string port, std::chrono::milliseconds timeout)
{
    descriptor = INVALID_FILE_DESCRIPTOR;
    this->port = port;
    millisec_timeout = timeout;
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
    NO_IMPL
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
    NO_IMPL
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
    NO_IMPL
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
    NO_IMPL
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
    NO_IMPL
    #endif

    // Create a vector with the data read
    return std::vector<uint8_t>(buffer, buffer + bytes_read);
}

std::vector<uint8_t> Serial::readUntil(std::vector<uint8_t> end, std::chrono::milliseconds timeout)
{
    std::vector<uint8_t> data;
    uint8_t byte;
    auto start_time = std::chrono::steady_clock::now();
    auto last_read_time = start_time;

    // Read data from serial port linux
    #ifdef __linux__
    while (true) {
        // Read byte from serial port
        int bytes_read = read(descriptor, &byte, 1);
        if (bytes_read > 0) {
            // Add byte to data
            D(std::cout << "[INFO] Byte read: " << std::hex << static_cast<int>(byte) << std::endl;)
            data.push_back(byte);

            // Reset the last read time on successful read
            last_read_time = std::chrono::steady_clock::now();
        }

        // Check if end of data was reached
        if (data.size() >= end.size() && std::equal(data.end() - end.size(), data.end(), end.begin())) {
            break;
        }

        // Check for timeout since last read
        auto elapsed_time = std::chrono::steady_clock::now() - last_read_time;
        if (elapsed_time >= timeout) {
            D(std::cout << "[ERROR] Timeout occurred while reading from serial port" << std::endl;)
            return std::vector<uint8_t>();
        }

        // Sleep for a short time to avoid busy-waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    #endif

    // Read data from serial port windows
    #ifdef _WIN32
    NO_IMPL
    #endif

    return data;
}

std::vector<uint8_t> Serial::readUntil(std::vector<uint8_t> end)
{
    std::vector<uint8_t> data;
    uint8_t byte;
    auto start_time = std::chrono::steady_clock::now();

    // Read data from serial port linux
    #ifdef __linux__
    while (true) {
        // Read byte from serial port
        int bytes_read = read(descriptor, &byte, 1);
        if (bytes_read > 0) {
            // Add byte to data
            data.push_back(byte);
        }

        // Check if end of data was reached
        if (data.size() >= end.size() && std::equal(data.end() - end.size(), data.end(), end.begin())) {
            break;
        }

        // Check for timeout
        auto elapsed_time = std::chrono::steady_clock::now() - start_time;
        if (elapsed_time >= millisec_timeout) {
            D(std::cout << "[ERROR] Timeout occurred while reading from serial port" << std::endl;)
            return std::vector<uint8_t>();
        }

        // Sleep for a short time to avoid busy-waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    #endif

    // Read data from serial port windows
    #ifdef _WIN32
    NO_IMPL
    #endif

    return data;
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
    NO_IMPL
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

// TODO: Automate sleep time
// void Serial::sleep_time(int n)
// {
//     double sleep_time = n * bit_time;
    
//     std::chrono::microseconds duration(static_cast<int64_t>(sleep_time));

//     usleep(sleep_time);
// }