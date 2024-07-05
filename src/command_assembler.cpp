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


#include <cstdint>
#include <vector>
#include <iostream>
#include <algorithm>
#include <chrono>

#include "command_assembler.hpp"
#include "common.hpp"

// Calculate FCS
uint8_t CommandAssembler::calculate_fcs(std::vector<uint8_t> data)
{
    uint8_t fcs = 0;
    for (uint8_t byte : data)
    {
        fcs += byte;
    }
    return fcs & 0xFF;
}

// Assemble command
std::vector<uint8_t> CommandAssembler::assemble_command(uint8_t info, std::vector<uint8_t> data)
{
    std::vector<uint8_t> command;
    // Insert sof
    command.insert(command.end(), sof.begin(), sof.end());
    // Insert command info
    command.push_back(info);
    // Insert length of data (2 bytes in little endian)
    command.push_back(data.size() & 0xFF);
    command.push_back((data.size() >> 8) & 0xFF);
    // Insert data
    command.insert(command.end(), data.begin(), data.end());
    // Insert FCS
    // Exclude sof bytes when calculating FCS
    std::vector<uint8_t> fcsData(command.begin() + sof.size(), command.end());
    command.push_back(calculate_fcs(fcsData));
    // Insert EOF
    command.insert(command.end(), eof.begin(), eof.end());
    return command;
}

std::vector<uint8_t> CommandAssembler::assemble_command(uint8_t info)
{
    std::vector<uint8_t> command;
    // Insert sof
    command.insert(command.end(), sof.begin(), sof.end());
    // Insert command info
    command.push_back(info);
    // Insert length of data (in this case 0)
    command.push_back(0x00);
    command.push_back(0x00);
    // No data to insert
    // Insert FCS, in this case the info itself
    command.push_back(info);
    // Insert eof
    command.insert(command.end(), eof.begin(), eof.end());
    return command;
}

// Calculate final frequency
float CommandAssembler::calculateFinalFreq(uint8_t phy, float freq, int channel)
{
    float finalFreq;

    switch (phy)
    {
    // IEEE 802.15.4ge
    case 0x00:
    case 0x01:
    case 0x02:
    case 0x03:
        if (freq == 868)
        {
            finalFreq = 868;
        }
        else if (freq == 915)
        {
            finalFreq = 915;
        }
        break;
    // Wi-SUN 
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
    case 0x08:
    case 0x09:
    case 0x0A:
        if (freq == 868)
        {
            finalFreq = 868;
        }
        else if (freq == 915)
        {
            finalFreq = 915;
        }
        break;
    // Zigbee
    case 0x0B:
    case 0x0C:
        break;
    // IEEE 915
    case 0x0D:
        break;
    // EasyLink
    case 0x0E:
    case 0x0F:
    case 0x10:
    case 0x11:
        break;
    // IEEE 2.4GHz
    case 0x12:
        finalFreq = 2405 + ((channel - 11) * 5);
        break;
    // BLE
    case 0x13:
        break;
    default:
        break;
    }

    return finalFreq;
}

// Convert frequency to byte
std::vector<uint8_t> CommandAssembler::convertFreqToByte(float freq)
{
    // Calculate frequency to byte using the formula: (frequency + fractFrequency/65536) MHz.
    // Where the first 2 bytes are the integer part and the last 2 bytes are the fractional part.
    uint32_t freqInt = (uint32_t)freq;
    uint32_t freqFrac = (uint32_t)((freq - freqInt) * 65536);
    std::vector<uint8_t> freq_bytes;
    freq_bytes.push_back(freqInt & 0xFF);
    freq_bytes.push_back((freqInt >> 8) & 0xFF);
    freq_bytes.push_back(freqFrac & 0xFF);
    freq_bytes.push_back((freqFrac >> 8) & 0xFF);
    return freq_bytes;
}

// Assemble start command
std::vector<uint8_t> CommandAssembler::assemble_start()
{
    D(std::cout << "[INFO] Assembling start command." << std::endl;);
    return assemble_command(info_start);
}

// Assemble stop command
std::vector<uint8_t> CommandAssembler::assemble_stop()
{
    D(std::cout << "[INFO] Assembling stop command." << std::endl;);
    return assemble_command(info_stop);
}

// Assemble ping command
std::vector<uint8_t> CommandAssembler::assemble_ping()
{
    D(std::cout << "[INFO] Assembling ping command." << std::endl;);
    return assemble_command(info_ping);
}

// Assemble set frequency command
std::vector<uint8_t> CommandAssembler::assemble_set_freq(uint8_t radio_mode, int channel)
{
    D(std::cout << "[INFO] Assembling set frequency command." << std::endl;);
    // Convert frequency to byte
    uint8_t phy = radio_mode_table[radio_mode].phy;
    float freq = radio_mode_table[radio_mode].freq;
    std::vector<uint8_t> freq_bytes = convertFreqToByte(calculateFinalFreq(phy, freq, channel));
    return assemble_command(info_freq, freq_bytes);
}

// Assemble set PHY command
std::vector<uint8_t> CommandAssembler::assemble_set_phy(uint8_t radio_mode)
{
    uint8_t phy = radio_mode_table[radio_mode].phy;
    D(std::cout << "[INFO] Assembling set PHY command." << std::endl;);
    std::vector<uint8_t> data;
    data.push_back(phy);
    return assemble_command(info_phy, data);
}

// Verify response
bool CommandAssembler::verify_response(std::vector<uint8_t> response)
{
    if (response.size() == 0) return false;

    // Check if response status is ok
    // If the third byte is 0x80, the response is not a data stream
    switch(response[2])
    {
        case 0x80:
            return response[5] == 0x00;
        case 0xC0:
            return true;
        case 0xC1:
        default:
            return false;
    }
    return true;
}

// Disassemble ping response
std::vector<uint8_t> CommandAssembler::disassemble_ping(std::vector<uint8_t> response)
{
    std::vector<uint8_t> data;
    // Exclude sof, command info, length, fcs and eof
    data.insert(data.end(), response.begin() + 6, response.end() - 3);

    return data;
}

// Convert to packet
packet_data CommandAssembler::convert_to_network_packet(std::vector<uint8_t> data, std::chrono::microseconds system_timestamp)
{
    std::vector<uint8_t> length;
    std::vector<uint8_t> timestamp;
    uint8_t rssi;
    std::vector<uint8_t> payload;
    uint8_t status;
    uint8_t fcs;
    // Exclude SOF, INFO and EOF
    data = std::vector<uint8_t>(data.begin() + 3, data.end() - 2);

    // LENGHT 2B | TIMESTAMP 6B | RSSI 1B | DATA N B | STATUS 1B | FCS 1B
    // LENGHT
    length.insert(length.end(), data.begin(), data.begin() + 2);
    // TIMESTAMP
    timestamp.insert(timestamp.end(), data.begin() + 2, data.begin() + 8);
    // RSSI
    rssi = data[8];
    // DATA
    payload.insert(payload.end(), data.begin() + 9, data.end() - 2);
    // STATUS
    status = data[data.size() - 2];
    // FCS
    fcs = data[data.size() - 1];

    // Length and timestamp are in little endian
    std::reverse(length.begin(), length.end());
    std::reverse(timestamp.begin(), timestamp.end());

    // Convert to packet


    packet_data packet = {
        .length = (length[0] << 8) | length[1],
        .device_timestamp = std::chrono::microseconds((static_cast<uint64_t>(timestamp[0]) << 40) | (static_cast<uint64_t>(timestamp[1]) << 32) | (static_cast<uint64_t>(timestamp[2]) << 24) | (static_cast<uint64_t>(timestamp[3]) << 16) | (static_cast<uint64_t>(timestamp[4]) << 8) | static_cast<uint64_t>(timestamp[5])),
        .system_timestamp = system_timestamp,
        .rssi = rssi,
        .data = payload,
        .status = status,
        .fcs = fcs
    };

    return packet;
}