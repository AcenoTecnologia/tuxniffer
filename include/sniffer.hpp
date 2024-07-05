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
#include <thread>

#include "device.hpp"
#include "common.hpp"

class Sniffer
{
public:
    std::vector<std::thread> threads;
    std::vector<Device> devices;

    Sniffer(std::vector<device_s> devices_info, log_s log_settings);

    void configureAllDevices();
    void initAllDevices();

private:
    int device_id_counter = 0;
};