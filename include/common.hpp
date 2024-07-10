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
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <chrono>

#ifdef DEBUG 
#define D(x) x
#else 
#define D(x)
#endif

#define IS_FILE 0
#define IS_PIPE 1

// Timezone offset in seconds
#define TIMEZONE -10800

#define NO_IMPL {D(std::cout << "[ERROR] Function not implemented" << std::endl;)}

struct packet_data
{
    int64_t length;                                     //2B
    std::chrono::microseconds device_timestamp;         //6B
    std::chrono::microseconds system_timestamp;         //6B
    uint8_t rssi;                                       //1B
    std::vector<uint8_t> data;                          //nB
    uint8_t status;                                     //1B
    uint8_t fcs;                                        //1B
};

struct packet_queue_s {
    int id;
    std::string interface;
    int channel;
    uint8_t mode;
    std::vector<uint8_t> packet;
    std::chrono::time_point<std::chrono::system_clock> timestamp;
};;

struct device_s {
    std::string port;
    int radio_mode;
    int channel;
};

struct log_entry_s {
    bool enabled;
    std::string path;
    std::string base_name;
    bool split_devices_log;
    std::string reset_period;
};

struct log_s {
    log_entry_s file;
    log_entry_s pipe;
};