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

#include "command_assembler.hpp"
#include "serial.hpp"
#include "common.hpp"

// Enum State
enum class State
{
    // Firmware states
    WAITING_FOR_COMMAND,
    INIT,
    STARTED,
    STOPPED,
    // Software states
    READING,
    WRITING,
};

class Device
{
public:
    Serial serial;
    CommandAssembler cmd;
    State state;

    bool is_ready = false;
    bool is_streaming = false;
    int id;
    std::string port;
    uint8_t radio_mode;
    uint8_t channel;
    
    Device(device_s device, int id_counter);
    bool connect();
    bool init();
    bool start();
    bool stop();
    bool ping();
    bool configure();
    void stream();
    void stream(std::chrono::seconds seconds);
    bool disconnect();

    bool receive_response(std::vector<uint8_t> &ret);



};