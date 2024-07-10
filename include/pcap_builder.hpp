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
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <iomanip>

#include "common.hpp"

struct pcap_hdr_s {
    uint32_t magic_number;
    uint16_t version_major;
    uint16_t version_minor;
    int32_t thiszone;
    uint32_t sigfigs;
    uint32_t snaplen;
    uint32_t network;
};

struct pcaprec_hdr_s {
    uint32_t ts_sec;
    uint32_t ts_usec;
    uint32_t incl_len;
    uint32_t orig_len;
};

const std::vector<uint8_t> ipv4_header = {0x45, 0x00, 0x00, 0x5B, 0x00, 0x00, 0x00, 0x00, 0x80, 0x11, 0xB7, 0x3B, 0xC0, 0xA8, 0x01, 0x03, 0xC0, 0xA8, 0x01, 0x03};
const std::vector<uint8_t> udp_header = {0x45, 0x60, 0x45, 0x60, 0x00, 0x47, 0x1D, 0x82};
const std::vector<uint8_t> ti_header = {0x00, 0x3c, 0x00, 0x00};
const std::vector<uint8_t> ti_separator = {0x02};

// Length of the ipv4_header + udp_header + ti_header + ti_separator + interface_length +
//              + phy, frequency, channel, rssi, fcs

class PcapBuilder
{
public:
    static void write_global_header(FILE* file);
    static void write_packet_header(FILE* file, packet_queue_s packet, std::chrono::microseconds start_time);
    static void write_packet_data(FILE* file, packet_queue_s packet);
};