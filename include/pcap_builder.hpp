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
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <iomanip>

#include "common.hpp"

/**
 * @struct pcap_hdr_s
 * @brief Represents the global header of a pcap file.
 */
struct pcap_hdr_s {
    uint32_t magic_number;   ///< Magic number identifying the file format.
    uint16_t version_major;  ///< Major version number.
    uint16_t version_minor;  ///< Minor version number.
    int32_t thiszone;        ///< GMT to local correction.
    uint32_t sigfigs;        ///< Accuracy of timestamps.
    uint32_t snaplen;        ///< Max length of captured packets.
    uint32_t network;        ///< Data link type.
};

/**
 * @struct pcaprec_hdr_s
 * @brief Represents the header of a packet record in a pcap file.
 */
struct pcaprec_hdr_s {
    uint32_t ts_sec;   ///< Timestamp seconds.
    uint32_t ts_usec;  ///< Timestamp microseconds.
    uint32_t incl_len; ///< Number of bytes of packet saved in file.
    uint32_t orig_len; ///< Actual length of the packet.
};

/// IPv4 header template.
const std::vector<uint8_t> ipv4_header = {0x45, 0x00, 0x00, 0x5B, 0x00, 0x00, 0x00, 0x00, 0x80, 0x11, 0xB7, 0x3B, 0xC0, 0xA8, 0x01, 0x03, 0xC0, 0xA8, 0x01, 0x03};

/// UDP header template.
const std::vector<uint8_t> udp_header = {0x45, 0x60, 0x45, 0x60, 0x00, 0x47, 0x1D, 0x82};

/// TI header template.
const std::vector<uint8_t> ti_header = {0x00, 0x3c, 0x00, 0x00};

/// TI separator template.
const std::vector<uint8_t> ti_separator = {0x02};

/**
 * @class PcapBuilder
 * @brief Provides methods to build and write pcap files.
 */
class PcapBuilder
{
public:
    /**
     * @brief Writes the global header to a pcap file.
     * 
     * @param file The file to write to.
     */
    static void write_global_header(FILE* file);

    /**
     * @brief Writes the packet header to a pcap file.
     * 
     * @param file The file to write to.
     * @param packet The packet information.
     * @param start_time The start time of the capture.
     */
    static void write_packet_header(FILE* file, packet_queue_s packet, std::chrono::microseconds start_time);

    /**
     * @brief Writes the packet data to a pcap file.
     * 
     * @param file The file to write to.
     * @param packet The packet information.
     */
    static void write_packet_data(FILE* file, packet_queue_s packet);
};
