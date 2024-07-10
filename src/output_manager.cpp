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
#include <thread>
#include <mutex>
#include <queue>
#include <ctime>
#include <iomanip>
#include "common.hpp"
#include "output_manager.hpp"
#include "pcap_builder.hpp"
#include "command_assembler.hpp"

OutputManager::OutputManager(log_s log_settings)
{
    log = log_settings;
}


void OutputManager::add_packet(packet_queue_s packet)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    packet_queue.push(packet);
}

bool OutputManager::configure(int num_devices)
{
    this->num_devices = num_devices;

    if (log.file.enabled)
    {
        std::time_t t = std::time(nullptr);
        std::tm tm = *std::localtime(&t);
        std::stringstream ss;
        ss << std::put_time(&tm, "%Y-%m-%d_%H-%M");

        std::string timestamp = ss.str();
        std::string base_filename = log.file.path;
        if (log.file.reset_period != "none")
        {
            base_filename += timestamp;
        }
        base_filename += "_" + log.file.base_name;

        if (log.file.split_devices_log)
        {
            for (int i = 0; i < num_devices; ++i)
            {
                std::string filename = base_filename + "_" + std::to_string(i) + ".pcap";
                FILE* log_file = fopen(filename.c_str(), "wb");
                if (!log_file)
                {
                    D(std::cout << "[ERROR] Could not open log file: " << filename << std::endl;)
                    return false;
                }
                // Write global header
                PcapBuilder::write_global_header(log_file);
                log_files.push_back(log_file);
                D(std::cout << "[INFO] Log file created: " << filename << std::endl;);
            }
        }
        if(!log.file.split_devices_log)
        {
            std::string filename = base_filename + ".pcap";
            FILE* log_file = fopen(filename.c_str(), "wb");
            if (!log_file)
            {
                D(std::cout << "[ERROR] Could not open log file: " << filename << std::endl;)
                return false;
            }
            // Write global header
            PcapBuilder::write_global_header(log_file);
            log_files.push_back(log_file);
            D(std::cout << "[INFO] Log file created: " << filename << std::endl;);
        }
    last_update = std::chrono::system_clock::now();
    }

    return true;
}

void OutputManager::run()
{
    is_running = true;
    while (is_running || !packet_queue.empty())
    {
        if(!packet_queue.empty())
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            packet_queue_s packet = packet_queue.front();
            packet_queue.pop();
            D(std::cout << "[INFO] Packet processed by Output Manager. Size: " << packet.packet.size() << std::endl;)
            handle_packet(packet);
        }
        // Sleep to avoid busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    D(std::cout << "[INFO] Output Manager stopped." << std::endl;)
}

void OutputManager::handle_packet(packet_queue_s packet)
{   
    // Check if it is the first packet
    // If so, defines the start-time to system time - first packet timestamp
    // This base will be summed to the timestamp of each packet to get the correct timestamp
    if(is_first_packet)
    {
        CommandAssembler command_assembler;
        auto now = std::chrono::system_clock::now();
        start_time = now - command_assembler.get_device_timestamp(packet.packet);
        is_first_packet = false;
    }

    // Recreate log files if necessary (based on reset period)
    recreate_log_files();

    if(log.file.enabled)
    {
        // Check if i have more than one log file
        if(log.file.split_devices_log)
        {
            // If so, write to the correct file
            FILE* log_file = log_files[packet.id];
            auto start_time_micros = std::chrono::duration_cast<std::chrono::microseconds>(start_time.time_since_epoch());
            PcapBuilder::write_packet_header(log_file, packet, start_time_micros);
            PcapBuilder::write_packet_data(log_file, packet);
        }
        // If not, write to the only file
        if(!log.file.split_devices_log)
        {
            FILE* log_file = log_files[0];
            auto start_time_micros = std::chrono::duration_cast<std::chrono::microseconds>(start_time.time_since_epoch());
            PcapBuilder::write_packet_header(log_file, packet, start_time_micros);
            PcapBuilder::write_packet_data(log_file, packet);
        }
    }

    if(log.pipe.enabled)
    {
        // Check if i have more than one pipe
        // If so, write to the correct pipe
        // If not, write to the only pipe
    }
}

void OutputManager::recreate_log_files()
{
// Check if the reset period is none
    if (log.file.reset_period == "none") return;

    // Calculate the time difference since the last update
    auto now = std::chrono::system_clock::now();
    auto diff = now - last_update;
    
    bool recreate = false;
    if (log.file.reset_period == "hourly" && diff > std::chrono::hours(1)) recreate = true;
    if (log.file.reset_period == "daily" && diff > std::chrono::hours(24)) recreate = true;
    if (log.file.reset_period == "weekly" && diff > std::chrono::hours(24 * 7)) recreate = true;
    if (log.file.reset_period == "monthly" && diff > std::chrono::hours(24 * 30)) recreate = true;

    // If the time has not come yet, return
    if (!recreate) return;
    D(std::cout << "[INFO] Recreating log files..." << std::endl;)


    // Update the last update time
    last_update = now;

    // Generate the timestamp
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d_%H-%M");
    std::string timestamp = ss.str();

    // Construct the base filename
    std::string base_filename = log.file.path;
    if (log.file.reset_period != "none") base_filename += timestamp;
    base_filename += "_" + log.file.base_name;

    // Close old log files
    for (auto log_file : log_files) {
        fclose(log_file);
    }
    log_files.clear();

    // Recreate log files
    if (log.file.split_devices_log)
    {
        for (int i = 0; i < num_devices; ++i) {
            std::string filename = base_filename + "_" + std::to_string(i) + ".pcap";
            FILE* new_log_file = fopen(filename.c_str(), "wb");
            if (!new_log_file) {
                D(std::cout << "[ERROR] Could not open log file: " << filename << std::endl;)
                return;
            }
            // Write global header
            // PcapBuilder::write_global_header(new_log_file);  // Assuming this function is defined elsewhere
            log_files.push_back(new_log_file);
            D(std::cout << "[INFO] Log file recreated: " << filename << std::endl;)
        }

        return;
    }

    std::string filename = base_filename + ".pcap";
    FILE* new_log_file = fopen(filename.c_str(), "wb");
    if (!new_log_file) {
        D(std::cout << "[ERROR] Could not open log file: " << filename << std::endl;)
        return;
    }
    // Write global header
    // PcapBuilder::write_global_header(new_log_file);  // Assuming this function is defined elsewhere
    log_files.push_back(new_log_file);
    D(std::cout << "[INFO] Log file recreated: " << filename << std::endl;)

}