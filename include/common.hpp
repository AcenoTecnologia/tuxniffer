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
 */
struct packet_queue_s {
    int id;                                             ///< Packet ID.
    std::string interface;                              ///< Interface name.
    int channel;                                        ///< Channel number.
    uint8_t mode;                                       ///< Mode of the packet.
    std::vector<uint8_t> packet;                        ///< Packet data.
    std::chrono::time_point<std::chrono::system_clock> timestamp; ///< Timestamp when the packet was queued.
};

/**
 * @struct device_s
 * @brief Represents a device configuration.
 */
struct device_s {
    std::string port;                                   ///< Device port.
    int radio_mode;                                     ///< Radio mode.
    int channel;                                        ///< Channel number.
};

/**
 * @struct log_entry_s
 * @brief Represents a log entry configuration.
 */
struct log_entry_s {
    bool enabled;                                       ///< Indicates if logging is enabled.
    std::string path;                                   ///< Log file path.
    std::string base_name;                              ///< Base name for the log files.
    bool split_devices_log;                             ///< Indicates if log files should be split by device.
    std::string reset_period;                           ///< Log reset period.
};

/**
 * @struct log_s
 * @brief Represents the logging configuration.
 */
struct log_s {
    log_entry_s file;                                   ///< File log entry configuration.
    log_entry_s pipe;                                   ///< Pipe log entry configuration.
};
