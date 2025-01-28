////////////////////////////////////////////////////////////////////////////////////////////////////
// Company:  Aceno Digital Tecnologia em Sistemas Ltda.
// Homepage: http://www.aceno.com
// Project:  Tuxniffer
// Version:  1.1.3
// Date:     2025
//
// Copyright (C) 2002-2025 Aceno Tecnologia.
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <ctime>
#include <iomanip>
#include <memory>
#include <errno.h>
#include <cstring> 

#include "pipe.hpp"
#include "common.hpp"
#include "output_manager.hpp"
#include "pcap_builder.hpp"
#include "command_assembler.hpp"
#include "pipe_packet_handler.hpp"

OutputManager::OutputManager(log_s log_settings)
{
    log = log_settings;
    crypto_handler.security_level = log_settings.crypto.security_level;
}


void OutputManager::add_packet(packet_queue_s packet)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Check if the queue size has reached the maximum limit
    if ((int)packet_queue.size() >= queue_max_size)
    {
        packet_queue.pop();
        D(std::cout << "[WARNING] File packet queue is full (over " << queue_max_size << " entries). Deleting oldest packet to add the new one." << std::endl;)
    }

    // Add the new packet to the queue
    packet_queue.push(packet);
}

bool OutputManager::configure_files(std::vector<bool> readyDevices)
{
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
            base_filename += timestamp + "_";
        }
        base_filename += log.file.base_name;

        if (log.file.split_devices_log)
        {
            for (int i = 0; i < num_devices; ++i)
            {
                if (!readyDevices[i])
                {
                    continue;
                }
                std::string filename = base_filename + "_" + std::to_string(i) + ".pcap";
                FILE* log_file = fopen(filename.c_str(), "wb");
                if (!log_file)
                {
                    D(char* errmsg = custom_strerror(errno);
                        std::cout << "[ERROR] Could not open log file: " << filename << errmsg << ". Packets will be discarded." << std::endl;
                        free(errmsg);)
                    return false;
                }
                // Write global header
                std::vector<uint8_t> global_header = PcapBuilder::get_global_header();
                fwrite(global_header.data(), 1, global_header.size(), log_file);
                log_files.push_back(log_file);
                D(std::cout << "[INFO] Log file created: " << filename << "." << std::endl;);
            }
        }
        else
        {
            std::string filename = base_filename + ".pcap";
            FILE* log_file = fopen(filename.c_str(), "wb");
            if (!log_file)
            {
                D(char* errmsg = custom_strerror(errno);
                    std::cout << "[ERROR] Could not open log file: " << filename << errmsg << ". Packets will be discarded." << std::endl;
                    free(errmsg);)
                return false;
            }
            // Write global header
            std::vector<uint8_t> global_header = PcapBuilder::get_global_header();
            fwrite(global_header.data(), 1, global_header.size(), log_file);
            log_files.push_back(log_file);
            D(std::cout << "[INFO] Log file created: " << filename << "." << std::endl;);
        }
    last_update = std::chrono::system_clock::now();
    }

    return true;
}

bool OutputManager::configure_pipes(std::vector<bool> readyDevices)
{

    D(std::cout << "[INFO] Initializing Pipe threads." << std::endl;)
    if(log.pipe.enabled)
    {
        if(log.pipe.split_devices_log)
        {
            for (int i = 0; i < num_devices; ++i)
            {
                if (!readyDevices[i])
                {
                    continue;
                }
                std::string pipe_path = log.pipe.path;
                std::string pipe_base_name =  log.file.base_name + "_" + std::to_string(i);
                std::shared_ptr<PipePacketHandler> pipe_packet_handler = std::make_shared<PipePacketHandler>(pipe_path, pipe_base_name, start_time);
                log_pipes_handlers.push_back(pipe_packet_handler);
                std::thread pipe_thread(&PipePacketHandler::run, pipe_packet_handler);
                log_pipes_threads.push_back(std::move(pipe_thread));
            }
        }
        else
        {
            std::string pipe_path = log.pipe.path;
            std::shared_ptr<PipePacketHandler> pipe_packet_handler = std::make_shared<PipePacketHandler>(pipe_path, log.file.base_name, start_time);
            log_pipes_handlers.push_back(pipe_packet_handler);
            std::thread pipe_thread(&PipePacketHandler::run, pipe_packet_handler);
            log_pipes_threads.push_back(std::move(pipe_thread));
        }
    }
    
    return true;
}

bool OutputManager::configure(int num_devices, std::vector<bool> readyDevices)
{
    this->num_devices = num_devices;

    if(log.file.enabled)
        if(!configure_files(readyDevices)) return false;

    if(log.pipe.enabled)
        if(!configure_pipes(readyDevices)) return false;

    can_run = true;
    return true;
}

void OutputManager::run()
{
    // Starts to run
    is_running = true;
    if(log.crypto.simulation)
    {
        loadAndSimulateKeyPackets();
    }

    while ((is_running || !packet_queue.empty()) && can_run)
    {
        if(!packet_queue.empty())
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            packet_queue_s packet = packet_queue.front();
            packet_queue.pop();
            // D(std::cout << "[INFO] Packet processed by Output Manager. Size: " << packet.packet.size()  << "." << std::endl;)
            handle_packet(packet);
        }
        // Sleep to avoid busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Close log files
    for (auto log_file : log_files)
    {
        fclose(log_file);
    }

    if(log.crypto.save_packets)
    {
        saveKeyPackets();
    }

    // The PipePacketHandler to is_running = false
    for (auto pipe_packet_handler : log_pipes_handlers)
    {
        pipe_packet_handler->is_running = false;
    }
    // Join the threads from the pipes
    for (auto& pipe_thread : log_pipes_threads)
    {
        pipe_thread.join();
    }

    if(log.crypto.save_keys)
    {
        std::string filename = log.crypto.keys_path + ".txt";
        if (crypto_handler.link_keys.size() + crypto_handler.nwk_keys.size() == 1)
        {
            D(std::cout << "[INFO] No keys to be saved in: " << filename << ". File will not be created" << std::endl;)
        }
        else
        {
            std::ofstream keys_file(filename);
            if (!keys_file.is_open()) {
                std::cout << "[ERROR] Could not open file to save keys: " << filename << std::endl;
            }
            if (crypto_handler.link_keys.size() > 1)
            {
                keys_file << "Link Keys:";
                for(int i = 1; i < crypto_handler.link_keys.size(); i++){
                    keys_file << std::endl << " - " << crypto_handler.bytesToHexString(crypto_handler.link_keys[i]);
                }
            }
            if (crypto_handler.nwk_keys.size() > 0)
            {
                keys_file << std::endl << "Network Keys:";
                for(int i = 0; i < crypto_handler.nwk_keys.size(); i++){
                    keys_file << std::endl << " - " << crypto_handler.bytesToHexString(crypto_handler.nwk_keys[i]);
                }
            }
            keys_file.close();
        }
    }

    D(std::cout << "[INFO] Output Manager stopped. Queues empty." << std::endl;)
}

void OutputManager::handle_packet(packet_queue_s packet, bool isTransportKey)
{   
    // Check if it is the first packet
    // If so, defines the start-time to system time - first packet timestamp
    // This base will be summed to the timestamp of each packet to get the correct timestamp
    CommandAssembler command_assembler;
    if(is_first_packet)
    {        
        auto now = std::chrono::system_clock::now();
        start_time = now - command_assembler.get_device_timestamp(packet.packet) + std::chrono::seconds(TIMEZONE);
        is_first_packet = false;
        // Update the time on the PipePacketHandler
        for (auto pipe_packet_handler : log_pipes_handlers)
        {
            pipe_packet_handler->start_time = start_time;
        }
    }

    // Recreate log files if necessary (based on reset period)
    recreate_log_files();

    std::vector<uint8_t> payload = command_assembler.get_payload(packet);
    if (crypto_handler.extract_key(payload))
    {
        key_packets.push_back(packet);
        isTransportKey = true;
    }
    if(log.file.enabled)
    {
        // Check if i have more than one log file
        if(log.file.split_devices_log)
        {
            // If so, write to the correct file
            FILE* log_file = log_files[packet.id];
            auto start_time_micros = std::chrono::duration_cast<std::chrono::microseconds>(start_time.time_since_epoch());
            std::vector<uint8_t> packet_header = PcapBuilder::get_packet_header(packet, start_time_micros);
            fwrite(packet_header.data(), 1, packet_header.size(), log_file);
            std::vector<uint8_t> packet_data = PcapBuilder::get_packet_data(packet);
            fwrite(packet_data.data(), 1, packet_data.size(), log_file);
        }
        // If not, write to the only file
        if(!log.file.split_devices_log)
        {
            FILE* log_file = log_files[0];
            auto start_time_micros = std::chrono::duration_cast<std::chrono::microseconds>(start_time.time_since_epoch());
            std::vector<uint8_t> packet_header = PcapBuilder::get_packet_header(packet, start_time_micros);
            fwrite(packet_header.data(), 1, packet_header.size(), log_file);
            std::vector<uint8_t> packet_data = PcapBuilder::get_packet_data(packet);
            fwrite(packet_data.data(), 1, packet_data.size(), log_file);
        }
    }

    if(log.pipe.enabled)
    {
        if(log.pipe.split_devices_log)
        {
            PipePacketHandler* pipe_packet_handler = log_pipes_handlers[packet.id].get();
            pipe_packet_handler->add_packet(packet, isTransportKey);
        }
        else
        {
            PipePacketHandler* pipe_packet_handler = log_pipes_handlers[0].get();
            pipe_packet_handler->add_packet(packet, isTransportKey);
        }
    }
}

void OutputManager::recreate_log_files()
{
    // Check if the reset period is none
    if (log.file.reset_period == "none" || !log.file.enabled) return;

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

    // TODO: Empty the packet queue here before recreating the log files
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
                D(char* errmsg = custom_strerror(errno);
                    std::cout << "[ERROR] Could not open log file: " << filename << errmsg << "." << std::endl;
                    free(errmsg);)
                return;
            }
            // Write global header
            std::vector<uint8_t> global_header = PcapBuilder::get_global_header();
            fwrite(global_header.data(), 1, global_header.size(), new_log_file);


            log_files.push_back(new_log_file);
            D(std::cout << "[INFO] Log file recreated: " << filename << "." << std::endl;)
        }

        return;
    }

    std::string filename = base_filename + ".pcap";
    FILE* new_log_file = fopen(filename.c_str(), "wb");
    if (!new_log_file) {
        D(char* errmsg = custom_strerror(errno);
            std::cout << "[ERROR] Could not open log file: " << filename << errmsg << "." << std::endl;
            free(errmsg);)
        return;
    }
    // Write global header
    std::vector<uint8_t> global_header = PcapBuilder::get_global_header();
    fwrite(global_header.data(), 1, global_header.size(), new_log_file);

    log_files.push_back(new_log_file);
    D(std::cout << "[INFO] Log file recreated: " << filename << "." << std::endl;)

}

void OutputManager::saveKeyPackets() {
    if (key_packets.size() == 0)
    {
        D(std::cout << "[INFO] No transport key packets to be saved in: " << log.crypto.packets_path << ". File will not be created" << std::endl;)
        return;
    }
    std::ofstream outFile;
    outFile = ofstream(log.crypto.packets_path, std::ios::binary);
    if (!outFile.is_open()) {
        std::cout << "[ERROR] Could not open file to save key packets: " << log.crypto.packets_path << std::endl;
        return;
    }

    for (const auto& p : key_packets) {
        // Serializar os dados
        //outFile.write(reinterpret_cast<const char*>(&p.id), sizeof(p.id));
        int interfaceSize = p.serial_interface.size();
        outFile.write(reinterpret_cast<const char*>(&interfaceSize), sizeof(interfaceSize));
        outFile.write(p.serial_interface.data(), interfaceSize);
        outFile.write(reinterpret_cast<const char*>(&p.channel), sizeof(p.channel));
        outFile.write(reinterpret_cast<const char*>(&p.mode), sizeof(p.mode));

        int packetSize = p.packet.size();
        outFile.write(reinterpret_cast<const char*>(&packetSize), sizeof(packetSize));
        outFile.write(reinterpret_cast<const char*>(p.packet.data()), packetSize);
        //auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        //    p.timestamp.time_since_epoch()).count();
        //outFile.write(reinterpret_cast<const char*>(&timestamp), sizeof(timestamp));
    }

    outFile.close();
}


void OutputManager::loadAndSimulateKeyPackets() {
    std::ifstream inFile(log.crypto.simulation_path, std::ios::binary);
    if (!inFile.is_open()) {
        std::cout << "[ERROR] Could not open file to load key packets: " << log.crypto.simulation_path << std::endl;
        return;
    }
    
    if (inFile.peek() == EOF)
    {
        D(std::cout << "[INFO] File " << log.crypto.simulation_path << "is empty. No packets to simulate." << std::endl;)
        return;
    }
    try
    {
        while (inFile.peek() != EOF) 
        {
            packet_queue_s p;

            // Desserializar os dados
            //inFile.read(reinterpret_cast<char*>(&p.id), sizeof(p.id));
            int interfaceSize;
            inFile.read(reinterpret_cast<char*>(&interfaceSize), sizeof(interfaceSize));
            p.serial_interface.resize(interfaceSize);
            inFile.read(&p.serial_interface[0], interfaceSize);
            inFile.read(reinterpret_cast<char*>(&p.channel), sizeof(p.channel));
            inFile.read(reinterpret_cast<char*>(&p.mode), sizeof(p.mode));

            int packetSize;
            inFile.read(reinterpret_cast<char*>(&packetSize), sizeof(packetSize));
            p.packet.resize(packetSize);
            inFile.read(reinterpret_cast<char*>(p.packet.data()), packetSize);

            //long long timestampSeconds;
            //inFile.read(reinterpret_cast<char*>(&timestampSeconds), sizeof(timestampSeconds));
            p.timestamp = std::chrono::system_clock::now();
            std::lock_guard<std::mutex> lock(m_mutex);
            for (int i = 0; i < log_pipes_handlers.size(); i++){
                p.id = i;
                handle_packet(p, true);
            }
            
        }
        inFile.close();
    }
    catch(const std::exception& e)
    {
        D(std::cout << "[Error] No packets to read in: " << log.crypto.simulation_path << "." << std::endl;)
    }
        
}