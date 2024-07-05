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

class Serial
{
public:

    TYPE_FILE_DESCRIPTOR descriptor;
    TYPE_FILE_CONFIG config;
    uint8_t buffer[BUFFER_SIZE];

    std::string port;

    Serial(std::string port);

    bool connect();
    bool disconnect();
    bool writeData(std::vector<uint8_t> data);
    bool writeData(uint8_t data);
    std::vector<uint8_t> readData();
    bool readByte(uint8_t* byte);
    void flush();
    void purge();
    bool is_connected();

private:
};