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

#include "common.hpp"
#include "pcap_builder.hpp"
#include "pipe_packet_handler.hpp"
#include <csignal>
#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <chrono>
#include <vector>

volatile std::sig_atomic_t pipe_interrupted = 0;

#ifdef __linux__
    void pipe_signal_handler(int signal) {
        if (signal == SIGPIPE) {
            pipe_interrupted = 1;
        }
    }
#endif

PipePacketHandler::PipePacketHandler(std::string pipe_path, std::string base, std::chrono::time_point<std::chrono::system_clock> start_time)
{
    this->pipe_path = pipe_path;
    this->base = base;
    this->start_time = start_time;
}

void PipePacketHandler::add_packet(packet_queue_s packet)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Check if the queue size has reached the maximum limit
    if ((int)packet_queue.size() >= queue_max_size)
    {
        packet_queue.pop();
        D(std::cout << "[WARNING] Packet queue from pipe: " << pipe_path << base << " is full (over " << queue_max_size << " entries). Deleting oldest packet to add the new one." << std::endl;)
    }

    // Add the new packet to the queue
    packet_queue.push(packet);
}

void PipePacketHandler::run()
{
    // Set up signal handler for SIGPIPE
    #ifdef __linux__
        std::signal(SIGPIPE, pipe_signal_handler);
    #endif
    is_running = true;
    
    while (is_running)
    {
        std::string path = pipe_path + base;
        is_open = false;
        
        pipe.create(path);
        D(std::cout << "[INFO] Pipe was created." << std::endl;)
        D(std::cout << "[INFO] Will be waiting for consumer on: " << path << "." << std::endl;)
        // Check if the pipe is open each second until it's open or until is_running is false
        while (is_running && !is_open)
        {
            is_open = pipe.open(path);
            #ifdef __linux__
                std::this_thread::sleep_for(std::chrono::seconds(1));
            #endif
        }
        
        // If the pipe is not open, the pipe thread will be closed and the data will be discarded
        if (!is_open)
        {
            D(std::cout << "[INFO] Pipe data was discarded." << std::endl;)
            return;
        }

        D(std::cout << "[INFO] Pipe Packet Handler received a consumer." << std::endl;)

        // Write global header
        std::vector<uint8_t> global_header = PcapBuilder::get_global_header();
        pipe.write(global_header);

        // Handle packets and check for interruptions
        while (is_running && !pipe_interrupted)
        {
            if (!packet_queue.empty())
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                packet_queue_s packet = packet_queue.front();
                packet_queue.pop();
                auto start_time_micros = std::chrono::duration_cast<std::chrono::microseconds>(start_time.time_since_epoch());
                std::vector<uint8_t> packet_header = PcapBuilder::get_packet_header(packet, start_time_micros);
                pipe.write(packet_header);
                std::vector<uint8_t> packet_data = PcapBuilder::get_packet_data(packet);
                pipe.write(packet_data);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        if (pipe_interrupted)
        {
            D(std::cout << "[INFO] Pipe was interrupted. Reinitializing ALL pipe handlers." << std::endl;)
            D(std::cout << "[INFO] Please reconnect all pipes. Pipe streaming will be put on hold." << std::endl;)
            pipe_interrupted = 0;
            pipe.close();
            continue; // Restart the loop to wait for a new connection
        }

        // Close the pipe if no longer running
        pipe.close();
        is_open = false;
    }
}