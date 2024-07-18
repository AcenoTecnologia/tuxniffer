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

/**
 * @brief Handles packets received that needs to be sent through a named pipe.
 */
class PipePacketHandler
{
public:
    bool is_running = false; ///< Flag indicating if the packet handler is running.
    std::queue<packet_queue_s> packet_queue; ///< Queue for storing incoming packets.
    std::chrono::time_point<std::chrono::system_clock> start_time; ///< Start time of packet handling.
    
    /**
     * @brief Constructs a PipePacketHandler object.
     * 
     * @param pipe_path The path to the named pipe.
     * @param base      The base string for the name. The id could be appended to this string.
     * @param start_time The start time of packet handling.
     */
    PipePacketHandler(std::string pipe_path, std::string base, std::chrono::time_point<std::chrono::system_clock> start_time);


    /**
     * @brief Adds a packet to the packet queue.
     * 
     * @param packet The packet to add.
     */
    void add_packet(packet_queue_s packet);

    /**
     * @brief Starts the packet handling process.
     */
    void run();

private:
    int queue_max_size = 500000; ///< Maximum size of the packet queue.
    std::mutex m_mutex; ///< Mutex for thread synchronization.
    Pipe pipe; ///< Named pipe interface.
    std::string pipe_path; ///< Path to the named pipe.
    std::string base; ///< Base string.
    bool is_open = false; ///< Flag indicating if the named pipe is open.
};
