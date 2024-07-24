////////////////////////////////////////////////////////////////////////////////////////////////////
// Company:  Aceno Digital Tecnologia em Sistemas Ltda.
// Homepage: http://www.aceno.com
// Project:  Tuxniffer
// Version:  1.0
// Date:     2024
//
// Copyright (C) 2002-2024 Aceno Tecnologia.
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <vector>
#include <thread>
#include <chrono>

#include "device.hpp"
#include "common.hpp"
#include "output_manager.hpp"

/**
 * @class Sniffer
 * @brief Manages a collection of devices for packet sniffing.
 */
class Sniffer
{
public:
    OutputManager output_manager;               ///< Manages output operations for captured packets.
    std::vector<std::thread> threads;           ///< Threads for handling device operations.
    std::thread output_manager_thread;          ///< Thread for handling the output manager.
    std::vector<Device> devices;                ///< List of devices used for packet sniffing.

    /**
     * @brief Constructs a new Sniffer object.
     * 
     * @param devices_info Information about the devices to be used.
     * @param log_settings Settings for logging.
     */
    Sniffer(std::vector<device_s> devices_info, log_s log_settings);

    /**
     * @brief Configures all devices.
     */
    void configureAllDevices();

    /**
     * @brief Initializes all devices.
     */
    void initAllDevices();

    /**
     * @brief Starts streaming on all devices.
     */
    void streamAll();

    /**
     * @brief Starts streaming on all devices for a specified duration.
     * 
     * @param duration The duration to stream for.
     */
    void streamAll(std::chrono::seconds duration);

private:
    int device_id_counter = 0;  ///< Counter for assigning unique IDs to devices.
};
