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

#pragma once

#include <iostream> 
#include <thread> 
#include <mutex> 
#include <queue>
#include <chrono>


#include "pipe.hpp"
#include "common.hpp"
#include "pcap_builder.hpp"

class PipePacketHandler
{
public:
    PipePacketHandler(std::string pipe_path, std::string base, std::chrono::time_point<std::chrono::system_clock> start_time);
    bool is_running = false;
    std::queue<packet_queue_s> packet_queue;

    void add_packet(packet_queue_s packet);
    void run();

private:
    std::chrono::time_point<std::chrono::system_clock> start_time;
    std::mutex m_mutex;
    Pipe pipe;
    std::string pipe_path;
    std::string base;
    bool is_open = false;
};
