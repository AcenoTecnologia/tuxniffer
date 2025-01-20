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

#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <chrono>

#ifdef DEBUG 
#define D(x) x
#else 
#define D(x)
#endif

#define IS_FILE 0
#define IS_PIPE 1

// Timezone offset in seconds
#define TIMEZONE -10800

#define NO_IMPL {D(std::cout << "[ERROR] Function not implemented" << std::endl;)}

/**
 * @struct packet_data
 * @brief Represents the data of a packet.
 * It's used to pass data around the application between the command_assembler and the output_manager
 */
struct packet_data
{
    int64_t length;                                     ///< Packet length in bytes.
    std::chrono::microseconds device_timestamp;         ///< Timestamp from the device.
    std::chrono::microseconds system_timestamp;         ///< System timestamp.
    uint8_t rssi;                                       ///< RSSI value.
    std::vector<uint8_t> data;                          ///< Packet data.
    uint8_t status;                                     ///< Packet status.
    uint8_t fcs;                                        ///< Frame Check Sequence.
};

/**
 * @struct packet_queue_s
 * @brief Represents a packet in the queue.
 * It's used to pass data around the application between the device, the output_manager and the pipe_packet_handler
 */
struct packet_queue_s {
    int id;                                             ///< Packet ID.
    std::string serial_interface;                              ///< Interface name.
    int channel;                                        ///< Channel number.
    uint8_t mode;                                       ///< Mode of the packet.
    std::vector<uint8_t> packet;                        ///< Packet data.
    std::chrono::time_point<std::chrono::system_clock> timestamp; ///< Timestamp when the packet was queued.
};

/**
 * @struct device_s
 * @brief Represents a device configuration.
 * It's used to store the configuration of a device that is captured through the CLI or the configuration file.
 */
struct device_s {
    std::string port;                                   ///< Device port.
    int radio_mode;                                     ///< Radio mode.
    int channel;                                        ///< Channel number.
};

/**
 * @struct log_entry_s
 * @brief Represents a log entry configuration.
 * It's used to store the configuration of a log entry that is captured through the CLI or the configuration file.
 */
struct log_entry_s {
    bool enabled;                                       ///< Indicates if logging is enabled.
    std::string path;                                   ///< Log file path.
    std::string base_name;                              ///< Base name for the log files.
    bool split_devices_log;                             ///< Indicates if log files should be split by device.
    std::string reset_period;                           ///< Log reset period.
};

struct crypto_entry_s {
    bool key_extraction;                                ///< Indicates if key extraction is enabled.
    int security_level;                                 ///< Indicates the network security level.
    bool save_keys;                                     ///< Indicates if extracted keys will be saved.
    std::string keys_path;                              ///< keys output file path.
    bool save_packets;                                  ///< Indicates if extracted transport keys packets will be saved.
    std::string packets_path;                           ///< transpot key output file path.
    bool simulation;                                    ///< Indicates if transport key packets simulation is enabled.
    std::string simulation_path;                        ///< transpot key input file path for simulation.
    bool append_mode;                                   ///< Indicates if packets will be written in append mode on packets_path.
    
};

/**
 * @struct log_s
 * @brief Represents the logging configuration.
 * It's only job is to hold the log entry configurations for the file and pipe logs.
 */
struct log_s {
    log_entry_s file;                                   ///< File log entry configuration.
    log_entry_s pipe;                                   ///< Pipe log entry configuration.
    crypto_entry_s crypto;                              ///< Crypto log entry configuration.
};

char* custom_strerror(int n_error);