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

#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <thread>

// If windows include windows API and define handler
#ifdef _WIN32
#include <windows.h>
#define TYPE_FILE_DESCRIPTOR HANDLE
#define TYPE_FILE_CONFIG     DCB
#define INVALID_FILE_DESCRIPTOR INVALID_HANDLE_VALUE
#endif
// If linux include termios
#ifdef __linux__
#include <termios.h>
#include <fcntl.h>
#define TYPE_FILE_DESCRIPTOR     int
#define TYPE_FILE_CONFIG         struct termios
#define INVALID_FILE_DESCRIPTOR  (-1)
#endif

#define BUFFER_SIZE 1024

#include "common.hpp"

/**
 * @class Serial
 * @brief Manages serial port communication.
 */
class Serial
{
public:

    TYPE_FILE_DESCRIPTOR descriptor;  ///< File descriptor for the serial port.
    TYPE_FILE_CONFIG config;          ///< Configuration for the serial port.
    uint8_t buffer[BUFFER_SIZE];      ///< Buffer for serial data.
    std::string port;                 ///< Serial port identifier.

    /**
     * @brief Constructs a new Serial object.
     * 
     * @param port The serial port to connect to.
     */
    Serial(std::string port);

    /**
     * @brief Connects to the serial port.
     * 
     * @return true if connection is successful.
     * @return false if connection fails.
     */
    bool connect();

    /**
     * @brief Disconnects from the serial port.
     * 
     * @return true if disconnection is successful.
     * @return false if disconnection fails.
     */
    bool disconnect();

    /**
     * @brief Writes data to the serial port.
     * 
     * @param data The data to write.
     * @return true if write is successful.
     * @return false if write fails.
     */
    bool writeData(std::vector<uint8_t> data);

    /**
     * @brief Writes a single byte to the serial port.
     * 
     * @param data The byte to write.
     * @return true if write is successful.
     * @return false if write fails.
     */
    bool writeData(uint8_t data);

    /**
     * @brief Reads data from the serial port.
     * 
     * @return std::vector<uint8_t> The data read from the port.
     */
    std::vector<uint8_t> readData();

    /**
     * @brief Reads a single byte from the serial port.
     * 
     * @param byte Pointer to the byte where the read data will be stored.
     * @return true if read is successful.
     * @return false if read fails.
     */
    bool readByte(uint8_t* byte);

    /**
     * @brief Flushes the serial port buffer.
     */
    void flush();

    /**
     * @brief Purges the serial port.
     */
    void purge();

    /**
     * @brief Checks if the serial port is connected.
     * 
     * @return true if the port is connected.
     * @return false if the port is not connected.
     */
    bool is_connected();

private:
};
