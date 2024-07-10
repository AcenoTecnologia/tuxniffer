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

void PcapBuilder::write_global_header(FILE* file)
{
    pcap_hdr_s pcap_hdr;
    pcap_hdr.magic_number = 0xa1b2c3d4;
    pcap_hdr.version_major = 2;
    pcap_hdr.version_minor = 4;
    pcap_hdr.thiszone = TIMEZONE;
    pcap_hdr.sigfigs = 0;
    pcap_hdr.snaplen = 65535;
    pcap_hdr.network = 228;
    fwrite(&pcap_hdr, sizeof(pcap_hdr_s), 1, file);
}

void PcapBuilder::write_packet_header(FILE* file, packet_queue_s packet, std::chrono::microseconds start_time)
{
    CommandAssembler command_assembler;
    packet_data data = command_assembler.convert_to_network_packet(packet.packet, start_time);

    pcaprec_hdr_s pcaprec_hdr;
    pcaprec_hdr.ts_sec = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(start_time).count()) + std::chrono::duration_cast<std::chrono::seconds>(data.device_timestamp).count();
    pcaprec_hdr.ts_usec = (static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(start_time).count()) + std::chrono::duration_cast<std::chrono::seconds>(data.device_timestamp).count()) % 1000000;
    
    int total_length = (
        ipv4_header.size() +
        udp_header.size() +
        ti_header.size() +
        ti_separator.size() +
        2 + // interface
        1 + // phy
        4 + // frequency
        2 + // channel
        1 + // rssi
        1 +  // fcs
        // -15 counting BOF, EOF, INFO, TIMESTAMP, RSSI, FCS, LENGTH...
        (data.length)
    );

    pcaprec_hdr.incl_len = total_length;
    pcaprec_hdr.orig_len = total_length;

    std::cout << "incl_len: " << pcaprec_hdr.incl_len << std::endl;

    fwrite(&pcaprec_hdr, sizeof(pcaprec_hdr_s), 1, file);
}

void PcapBuilder::write_packet_data(FILE* file, packet_queue_s packet)
{
    CommandAssembler command_assembler;

    packet_data data = command_assembler.convert_to_network_packet(packet.packet, std::chrono::microseconds(0));

    std::vector<uint8_t> final_packet;

    int total_length = (
        ipv4_header.size() +
        udp_header.size() +
        ti_header.size() +
        ti_separator.size() +
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
    final_packet.insert(final_packet.end(), ti_separator.begin(), ti_separator.end());
    // Add phy
    int phy = radio_mode_table[packet.mode].phy;
    final_packet.push_back(phy);
    // Add frequency
    std::vector<uint8_t> final_frequency = command_assembler.convertFreqToByte(command_assembler.calculateFinalFreq(phy, radio_mode_table[packet.mode].freq, packet.channel));
    final_packet.insert(final_packet.end(), final_frequency.begin(), final_frequency.end());
    // Add channel as 16 bits
    final_packet.push_back(packet.channel >> 8);
    final_packet.push_back(packet.channel & 0xFF);
    // Add rssi as 8 bits
    final_packet.push_back(data.rssi);
    // Add fcs as 8 bits
    final_packet.push_back(data.fcs);
    // Add payload
    final_packet.insert(final_packet.end(), data.data.begin(), data.data.end());

    std::cout << "final size: " << final_packet.size() << std::endl;

    fwrite(final_packet.data(), final_packet.size(), 1, file);
}



/*

incl_len: 55
4053
c0
3800
f52568020000
2f
4188fce1fbffff00000912fcff000001c6d5ebc629004b120028852a0100d5ebc629004b12000045d119ca2c92fa32d2
80
4045
ipv4_header: 45000055000000008011b73bc0a80103c0a80103
udp_header: 4560456000411d82
ti_header: 003c0000
final size: 5b


final packet: 45000055000000008011b73bc0a80103c0a80103
4560456000411d82
003c0000
0000 - interface
02 - separator
12 - phy
ab090000 - freq
0019 - channel
2f
80
4188fce1fbffff00000912fcff000001c6d5ebc629004b120028852a0100d5ebc629004b12000045d119ca2c92fa32

*/