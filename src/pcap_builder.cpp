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


#include <iostream>
#include <algorithm>

#include "pcap_builder.hpp"
#include "command_assembler.hpp"
#include "common.hpp"

std::vector<uint8_t> PcapBuilder::get_global_header()
{
    pcap_hdr_s pcap_hdr;
    pcap_hdr.magic_number = 0xa1b2c3d4;
    pcap_hdr.version_major = 2;
    pcap_hdr.version_minor = 4;
    pcap_hdr.thiszone = TIMEZONE;
    pcap_hdr.sigfigs = 0;
    pcap_hdr.snaplen = 65535;
    pcap_hdr.network = 228;

    return std::vector<uint8_t>(reinterpret_cast<uint8_t*>(&pcap_hdr), reinterpret_cast<uint8_t*>(&pcap_hdr) + sizeof(pcap_hdr_s));
}

std::vector<uint8_t> PcapBuilder::get_packet_header(packet_queue_s packet, std::chrono::microseconds start_time)
{
    CommandAssembler command_assembler;
    packet_data data = command_assembler.convert_to_network_packet(packet.packet, start_time);

    pcaprec_hdr_s pcaprec_hdr;
    pcaprec_hdr.ts_sec = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(start_time).count()) + std::chrono::duration_cast<std::chrono::seconds>(data.device_timestamp).count();
    pcaprec_hdr.ts_usec = (static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::microseconds>(start_time).count()) + std::chrono::duration_cast<std::chrono::microseconds>(data.device_timestamp).count()) % 1000000;
    
    int total_length = (
        ipv4_header.size() +
        udp_header.size() +
        ti_header.size() +
        ti_protocol.size() +
        2 + // interface
        1 + // phy
        4 + // frequency
        2 + // channel
        1 + // rssi
        1 +  // fcs
        (data.length)
    );

    pcaprec_hdr.incl_len = total_length;
    pcaprec_hdr.orig_len = total_length;

    return std::vector<uint8_t>(reinterpret_cast<uint8_t*>(&pcaprec_hdr), reinterpret_cast<uint8_t*>(&pcaprec_hdr) + sizeof(pcaprec_hdr_s));
}

std::vector<uint8_t> PcapBuilder::get_packet_data(packet_queue_s packet)
{
    CommandAssembler command_assembler;

    packet_data data = command_assembler.convert_to_network_packet(packet.packet, std::chrono::microseconds(0));

    std::vector<uint8_t> final_packet;

    int total_length = (
        ipv4_header.size() +
        udp_header.size() +
        ti_header.size() +
        ti_protocol.size() +
        2 + // interface
        1 + // phy
        4 + // frequency
        2 + // channel
        1 + // rssi
        1 +  // fcs
        (data.length)
    );

    // Create copy of ipv4_header
    std::vector<uint8_t> ip = ipv4_header;
    // Substitute the 3rd and 4th bytes of the ipv4_header with the total_length
    ip[2] = total_length >> 8;
    ip[3] = total_length & 0xFF;
    final_packet.insert(final_packet.end(), ip.begin(), ip.end());


    // Create copy of udp_header
    std::vector<uint8_t> udp = udp_header;
    // Substitute the 3rd and 4th bytes of the udp_header with the total_length
    udp[4] = (total_length-20) >> 8;
    udp[5] = (total_length-20) & 0xFF;
    final_packet.insert(final_packet.end(), udp.begin(), udp.end());

    // Create copy of ti_header
    std::vector<uint8_t> ti = ti_header;
    final_packet.insert(final_packet.end(), ti.begin(), ti.end());

    // Get only number from string packet.interface
    std::string interface_string = packet.interface;
    // Remove anything that is not a number
    interface_string.erase(std::remove_if(interface_string.begin(), interface_string.end(), [](char c) { return !std::isdigit(c); }), interface_string.end());
    // Convert string to int
    int interface = std::stoi(interface_string);
    // Add interface to final_packet as 16 bits
    final_packet.push_back(interface >> 8);
    final_packet.push_back(interface & 0xFF);


    // Add separator
    uint8_t protocol = command_assembler.get_protocol_value(packet.mode);
    final_packet.push_back(protocol);
    // Add phy
    int phy_ti = command_assembler.get_ti_phy_value(packet.mode);
    final_packet.push_back(phy_ti);
    // Add frequency
    std::vector<uint8_t> final_frequency = command_assembler.convertFreqToByte(command_assembler.calculateFinalFreq(packet.mode, radio_mode_table[packet.mode].freq, packet.channel));
    final_packet.insert(final_packet.end(), final_frequency.begin(), final_frequency.end());
    // Add channel as 16 bits
    final_packet.push_back(packet.channel & 0xFF);
    final_packet.push_back(packet.channel >> 8);
    // Add rssi as 8 bits
    final_packet.push_back(data.rssi);
    // Add fcs as 8 bits
    final_packet.push_back(data.fcs);
    // Add payload
    final_packet.insert(final_packet.end(), data.data.begin(), data.data.end());

    return final_packet;
}