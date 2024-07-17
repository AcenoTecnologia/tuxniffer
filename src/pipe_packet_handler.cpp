////////////////////////////////////////////////////////////////////////////////////////////////////
// Company:  Aceno Digital Tecnologia em Sistemas Ltda.
// Homepage: http://www.aceno.com
// Project:  C++ Interface TI Packet Sniffer
// Version:  1.0
// Date:     2024
//
// Copyright (C) 2002-2024 Aceno Tecnologia.
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "common.hpp"
#include "pcap_builder.hpp"
#include "pipe_packet_handler.hpp"

PipePacketHandler::PipePacketHandler(std::string pipe_path, std::string base, std::chrono::time_point<std::chrono::system_clock> start_time)
{
    this->pipe_path = pipe_path;
    this->base = base;
    this->start_time = start_time;
}

void PipePacketHandler::add_packet(packet_queue_s packet)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    packet_queue.push(packet);
}

void PipePacketHandler::run()
{
    is_running = true;
    std::string path = pipe_path + base;

    pipe.create(path);
    D(std::cout << "[INFO] Pipe was created." << std::endl;)
    D(std::cout << "[INFO] Will be waiting consumer on: " << path << std::endl;)
    
    // Check if the pipe is open each second until its open or until is_running is false
    while (is_running && !is_open)
    {
        is_open = pipe.open(path);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // When passing this point, the pipe is open or is_running is false
    // If is_open is false, the pipe thread will be closed and the data will be discarded
    if(!is_open)
    {
        D(std::cout << "[INFO] Pipe data was discarded." << std::endl;);
        return;
    }

    D(std::cout << "[INFO] Pipe Packet Handler recieved a consumer." << std::endl;)


    // If pipe is open iterate over the packet queue and send each packet to the pipe
    
    // Write global header
    std::vector<uint8_t> global_header = PcapBuilder::get_global_header();
    pipe.write(global_header);
    while (is_running || !packet_queue.empty())
    {
        // Check if the pipe is open
        if (!packet_queue.empty())
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            packet_queue_s packet = packet_queue.front();
            packet_queue.pop();
            // D(std::cout << "[INFO] Packet processed by Pipe Packet Handler. Size: " << packet.packet.size() << std::endl;)
            auto start_time_micros = std::chrono::duration_cast<std::chrono::microseconds>(start_time.time_since_epoch());
            std::vector<uint8_t> packet_header = PcapBuilder::get_packet_header(packet, start_time_micros);
            pipe.write(packet_header);
            std::vector<uint8_t> packet_data = PcapBuilder::get_packet_data(packet);
            pipe.write(packet_data);
        }
        // Sleep to avoid busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Close the pipe
    pipe.close();
    is_open = false;
}