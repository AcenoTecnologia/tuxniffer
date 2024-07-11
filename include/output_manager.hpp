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
#include <memory>

#include "common.hpp"
#include "pipe_packet_handler.hpp"

/**
 * @class OutputManager
 * @brief Manages the output of captured packets.
 *
 * This class is responsible for managing the queue of captured packets and writing these packets to log files.
 */
class OutputManager
{
public:
    /**
     * @brief Indicates if the manager is running.
     */
    bool is_running = false;

    /**
     * @brief Constructor for the OutputManager class.
     * 
     * @param log_settings Log settings.
     */
    OutputManager(log_s log_settings);

    /**
     * @brief Adds a packet to the packet queue.
     * 
     * @param packet Packet to be added.
     */
    void add_packet(packet_queue_s packet);

    /**
     * @brief Configures the log files.
     * 
     * @return true if the configuration was successful, false otherwise.
     */
    bool configure_files();

    /**
     * @brief Configures the pipes for logging.
     * 
     * @return true if the configuration was successful, false otherwise.
     */
    bool configure_pipes();

    /**
     * @brief Configures the output manager (log files and pipes)
     * 
     * @param num_devices Number of devices to be configured.
     * @return true if the configuration was successful, false otherwise.
     */
    bool configure(int num_devices);

    /**
     * @brief Starts the execution of the output manager.
     */
    void run();

    /**
     * @brief Handles a packet from the queue.
     * 
     * @param packet Packet to be handled.
     */
    void handle_packet(packet_queue_s packet);

    /**
     * @brief Recreates the log files.
     */
    void recreate_log_files();

private:
    /**
     * @brief Indicates if it is the first packet being processed.
     */
    bool is_first_packet = true;

    /**
     * @brief Number of configured devices.
     */
    int num_devices;

    /**
     * @brief Mutex for protecting the packet queue.
     */
    std::mutex m_mutex;

    /**
     * @brief Queue of packets to be processed.
     */
    std::queue<packet_queue_s> packet_queue;

    /**
     * @brief Start time of the processing.
     */
    std::chrono::time_point<std::chrono::system_clock> start_time;

    /**
     * @brief Last update time.
     */
    std::chrono::time_point<std::chrono::system_clock> last_update;

    /**
     * @brief Log settings.
     */
    log_s log;

    /**
     * @brief Vector of pointers to log files.
     */
    std::vector<FILE*> log_files;

    /**
     * @brief Vector of pipes for logging.
     */
    std::vector<std::shared_ptr<PipePacketHandler>> log_pipes_handlers;

    /**
     * @brief Vector of threads for pipe logging.
     */
    std::vector<std::thread> log_pipes_threads;
};
