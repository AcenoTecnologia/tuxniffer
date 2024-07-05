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

#ifdef DEBUG 
#define D(x) x
#else 
#define D(x)
#endif

#define IS_FILE 0
#define IS_PIPE 1

#define NO_IMPL {D(std::cout << "[ERROR] Function not implemented" << std::endl;)}

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