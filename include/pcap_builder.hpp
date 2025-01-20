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

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <iomanip>

#include "common.hpp"

/*
 * The following code is based on the following sources:
 * - https://wiki.wireshark.org/Development/LibpcapFileFormat
 *  Texas Instruments SmartRF Packet Sniffer 2 User Guide
 * 
 * Texas Instruments has its own wireshark dissector that needs the pcap file to be in a specific format.
 * The header order is:
 * - Global header
 * - Packet header
 * - IPV4 header (mockup because the packet is not an IP packet. TI uses it to send the TI Radio Packet Info layer via UDP)
 * -- UDP header
 * --- TI header
 * ---- IEEE 802.15.4 header
 * ----- Zigbee (Optional)
 * 
 * The packet data is actually the IEEE 802.15.4 packet data with extra information from TI.
 * Because of that, the IPV4 and UDP headers needs to have the same size as the real headers.
 * The IP and PORT fields are fake and are not being used.
*/

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

/// TI protocol template.
const std::vector<uint8_t> ti_protocol = {0x00};

/**
 * @class PcapBuilder
 * @brief Provides methods to build and write pcap files.
 */
class PcapBuilder
{
public:
    /**
     * @brief Gets the global header to a pcap file.
     * 
     * @return std::vector<uint8_t> The global header data.
     */
    static std::vector<uint8_t> get_global_header();

    /**
     * @brief Gets the packet header to a pcap file.
     * - The start time is simply added as system time. No operation is performed with it.
     * 
     * @param packet The packet information.
     * @param start_time The start time of the capture.
     * @return std::vector<uint8_t> The packet header data.
     */
    static std::vector<uint8_t> get_packet_header(packet_queue_s packet, std::chrono::microseconds start_time);

    /**
     * @brief Gets the packet data to a pcap file.
     * 
     * @param packet The packet information.
     * @return std::vector<uint8_t> The packet data.
     */
    static std::vector<uint8_t> get_packet_data(packet_queue_s packet);
};

