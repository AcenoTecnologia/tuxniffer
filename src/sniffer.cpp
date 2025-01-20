////////////////////////////////////////////////////////////////////////////////////////////////////
// Company:  Aceno Digital Tecnologia em Sistemas Ltda.
// Homepage: http://www.aceno.com
// Project:  Tuxniffer
// Version:  1.1
// Date:     2025
//
// Copyright (C) 2002-2025 Aceno Tecnologia.
// All rights reserved.
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
: output_manager(log_settings)
{
    // Initialize device settings
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
            D(std::cout << "[INFO] Device connected. ID: " << device.id << "." << std::endl;)
        }
    }
}

void Sniffer::initAllDevices()
{
    // Check if all devices are ready. At least one must be to start streaming.
    if (std::all_of(devices.begin(), devices.end(), [](Device& device) { return !device.is_ready; })) {
        D(std::cout << "[ERROR] No devices are ready." << std::endl;)
        return;
    }

    // Initialize log settings
    D(std::cout << "[INFO] Configuring output manager." << std::endl;)
    D(std::cout << "[INFO] Initializing Pipe threads." << std::endl;)
    output_manager.configure(devices.size());

    // Preallocates vector for threads
    threads.reserve(devices.size());

    // Define a thread for each device
    for (auto& device : devices) {
        if(!device.is_ready) continue;
        // Pass the output manager reference to the device
        device.output_manager = &output_manager;
        threads.push_back(std::thread([&device]() {
            device.init();
        }));
        D(std::cout << "[INFO] Init thread for device ID: " << device.id << " started." << std::endl;)
    }

    // Wait for all threads to finish
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    if(threads.size() == 0)
    {
        D(std::cout << "[ERROR] There are no ready devices." << std::endl;)
        return;
    }

    D(std::cout << "[INFO] All ready devices initialized." << std::endl;)

}

void Sniffer::streamAll()
{
    // Check if all devices are ready. At least one must be to start streaming.
    if (std::all_of(devices.begin(), devices.end(), [](Device& device) { return !device.is_ready; })) {
        std::cout << "[ERROR] No devices are ready to start streaming." << std::endl;
        return;
    }

    // Start the output manager thread
    output_manager_thread = std::thread(&OutputManager::run, &output_manager);

    // Preallocates vector for threads
    threads.reserve(devices.size());

    // Define a thread for each device
    for (auto& device : devices) {
        if(!device.is_ready) continue;
        threads.push_back(std::thread([&device]() {
            device.start();
            device.stream();
            device.stop();
        }));
        D(std::cout << "[INFO] Stream thread for device ID: " << device.id << " started." << std::endl;)
    }

    // Wait for all threads to finish
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    output_manager.is_running = false;
    // Join the output manager thread
    if (output_manager_thread.joinable()) {
        output_manager_thread.join();
    }
    
    D(std::cout << "[INFO] All ready devices finished streaming." << std::endl;)
}

void Sniffer::streamAll(std::chrono::seconds duration)
{
    // Check if all devices are ready. At least one must be to start streaming.
    if (std::all_of(devices.begin(), devices.end(), [](Device& device) { return !device.is_ready; })) {
        std::cout << "[ERROR] No devices are ready to start streaming." << std::endl;
        return;
    }

    // Start the output manager thread
    output_manager_thread = std::thread(&OutputManager::run, &output_manager);

    // Preallocates vector for threads
    threads.reserve(devices.size());

    // Define a thread for each device
    for (auto& device : devices) {
        if(!device.is_ready) continue;
        threads.push_back(std::thread([&device, duration]() {
            device.start();
            device.stream(duration);
            device.stop();
        }));
        D(std::cout << "[INFO] Stream thread for device ID: " << device.id << " started." << std::endl;)
    }

    // Wait for all threads to finish
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    output_manager.is_running = false;
    // Join the output manager thread
    if (output_manager_thread.joinable()) {
        output_manager_thread.join();
    }

    D(std::cout << "[INFO] All ready devices finished streaming." << std::endl;)
}