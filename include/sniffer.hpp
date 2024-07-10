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
#include <chrono>

#include "device.hpp"
#include "common.hpp"
#include "output_manager.hpp"

class Sniffer
{
public:
    OutputManager output_manager;

    std::vector<std::thread> threads;
    std::thread output_manager_thread;
    std::vector<Device> devices;

    Sniffer(std::vector<device_s> devices_info, log_s log_settings);

    void configureAllDevices();
    void initAllDevices();
    void streamAll();
    void streamAll(std::chrono::seconds duration);

private:
    int device_id_counter = 0;
};