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


#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <thread>
#include <chrono>

#include "common.hpp"
#include "device.hpp"
#include "sniffer.hpp"


Sniffer::Sniffer(std::vector<device_s> devices_info, log_s log_settings)
{
    for (auto& device_info : devices_info) {
        devices.emplace_back(device_info, device_id_counter);
        device_id_counter++;
    }
}


void Sniffer::configureAllDevices()
{
    for(auto& device : devices)
    {
        if(device.connect())
        {
            D(std::cout << "[INFO] Device connected. ID: " << device.id << std::endl;)
        }
    }
}

void Sniffer::initAllDevices()
{
    // Preallocates vector for threads
    threads.reserve(devices.size());

    // Define a thread for each device
    for (auto& device : devices) {
        if(!device.is_ready) continue;
        threads.push_back(std::thread([&device]() {
            device.init();
            device.start();
            device.stream();
        }));
        D(std::cout << "[INFO] Init thread for device ID: " << device.id << " started." << std::endl;)
    }

    // Wait for all threads to finish
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    D(std::cout << "[INFO] All ready devices initialized." << std::endl;)
}