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
 * - This class can write the packets to log files or pipes.
 * - Log files are managed in this thread.
 * - Pipes are managed each in a separate thread to avoid blocking.
 *
 * This class is responsible for managing the queue of captured packets and writing these packets to log files.
 */
class OutputManager
{
public:
    /**
     * @brief Indicates if the manager is running (executing the run method).
     */
    bool is_running = false;

    /**
     * @brief Indicates if the manager can run (has a file attached to it).
     * - Is false when no log is enabled.
     */
    bool can_run = false;

    /**
     * @brief Constructor for the OutputManager class.
     * 
     * @param log_settings Log settings.
     */
    OutputManager(log_s log_settings);

    /**
     * @brief Adds a packet to the packet queue.
     * - This queue is processed by the run method.
     * - Any packet that is added to the queue will be processed by the output manager and written to the log files or pipes if necessary.
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
     * - Create the pipe threads (with the wrapper PipePacketHandler)
     * 
     * @return true if the configuration was successful, false otherwise.
     */
    bool configure_pipes();

    /**
     * @brief Configures the output manager (log files and pipes)
     * - Will set a num_devices number of log files and pipes according to the configuration.
     * - Files could end up not being used if the device is not ready. This will leave an empty file.
     * 
     * @param num_devices Number of devices to be configured.
     * @return true if the configuration was successful, false otherwise.
     */
    bool configure(int num_devices);

    /**
     * @brief Starts the execution of the output manager.
     * - Is the main method of the output manager. It will process the packet queue and write the packets to the log files or pipes.
     */
    void run();

    /**
     * @brief Handles a packet from the queue.
     * - Decides if a packet should be written to the log files or pipes, or both.
     * - Handles the first packet differently to set the start time.
     * - Check if the log files need to be recreated according to the reset period.
     * 
     * @param packet Packet to be handled.
     */
    void handle_packet(packet_queue_s packet);

    /**
     * @brief Check if the log files needs to be recreated.
     */
    void recreate_log_files();

private:
    /**
     * @brief Indicates if it is the first packet being processed.
     * - Is used to set the start time of the processing.
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
     * @brief Last update time if the log files need to be recreated more than once.
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
     * - Each pipe will have a separate thread to avoid blocking.
     * - The pipe will be handled by the PipePacketHandler class.
     * - Is a shared pointer because PipePacketHandler uses mutexes, that are not copyable.
     */
    std::vector<std::shared_ptr<PipePacketHandler>> log_pipes_handlers;

    /**
     * @brief Vector of threads for pipe logging.
     */
    std::vector<std::thread> log_pipes_threads;

    /**
     * @brief Maximum size of the packet queue.
     */
    int queue_max_size = 500000;
};
