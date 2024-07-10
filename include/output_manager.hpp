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

#include <iostream> 
#include <thread> 
#include <mutex> 
#include <queue>
#include "common.hpp"

class OutputManager
{
public:
    bool is_running = false;

    OutputManager(log_s log_settings);
    void add_packet(packet_queue_s packet);
    bool configure(int num_devices);
    void run();
    void handle_packet(packet_queue_s packet);
private:
    bool is_first_packet = true;
    int num_devices;
    std::mutex m_mutex;
    std::queue<packet_queue_s> packet_queue;
    // Start time
    std::chrono::time_point<std::chrono::system_clock> start_time;

    log_s log;
    std::vector<FILE*> log_files;
    // std::vector<FILE*> log_pipes;

    void sendPacket(std::string packet);
};