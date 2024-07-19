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

uint8_t CommandAssembler::get_protocol_value(uint8_t radio_mode)
{    
    // TODO: Check each and every value.
    // Only PROTOCOL_IEEE_802_15_4 and PROTOCOL_BLE are tested.
    switch (radio_mode)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
        return PROTOCOL_IEEE_802_15_4_G;
    case 16:
    case 17:
    case 18:
    case 19:
        return PROTOCOL_GENERIC;
    case 20:
        return PROTOCOL_IEEE_802_15_4;
    case 21:
        return PROTOCOL_BLE;
    default:
        return PROTOCOL_GENERIC;
    }

    return PROTOCOL_GENERIC;
}

uint8_t CommandAssembler::get_ti_phy_value(uint8_t radio_mode)
{
    // TODO: Check each and every value.
    // Only PHY_TYPE_OQPSK and PHY_TYPE_BLE are tested.
    switch (radio_mode)
    {
    case 0:
    case 1:
    case 2:
        return PHY_TYPE_50KBPS_GFSK;
    case 3:
    case 4:
    case 5:
        return PHY_TYPE_SLR;
    case 6:
        return PHY_TYPE_50KBPS_GFSK_WISUN_1A;
    case 7:
        return PHY_TYPE_50KBPS_GFSK_WISUN_1B;
    case 8:
        return PHY_TYPE_100KBPS_GFSK_WISUN_2A;
    case 9:
        return PHY_TYPE_100KBPS_GFSK_WISUN_2B;
    case 10:
        return PHY_TYPE_150KBPS_GFSK_WISUN_3;
    case 11:
        return PHY_TYPE_200KBPS_GFSK_WISUN_4A;
    case 12:
        return PHY_TYPE_200KBPS_GFSK_WISUN_4B;
    case 13:
        return PHY_TYPE_100KBPS_GFSK_ZIGBEE_R23;
    case 14:
        return PHY_TYPE_500KBPS_GFSK_ZIGBEE_R23;
    case 15:
        return PHY_TYPE_200KBPS_GFSK;
    case 16:
    case 17:
        return PHY_TYPE_50KBPS_GFSK;
    case 18:
        return PHY_TYPE_SLR;
    case 19:
        return PHY_TYPE_SLR;
    case 20:
        return PHY_TYPE_OQPSK;
    case 21:
        return PHY_TYPE_BLE;
    default:
        return PHY_TYPE_UNUSED;
    }
    return PHY_TYPE_UNUSED;
}

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
float CommandAssembler::calculateFinalFreq(uint8_t mode, float freq, int channel)
{
    float finalFreq = freq;
    // Could aggregate similar options, but it would make the code less readable
    // TODO: Check each and every value.
    // Only 20 (802.15.4 2.4GHz) and 21 (BLE) are tested.
    // Values taken from the SmartRF Packet Sniffer 2 from Texas Instruments
    switch (mode)
    {
    // IEEE 802.15.4ge
    case 0:
        if(channel >= 0 && channel <= 128) finalFreq = 902.2 + (channel * 0.2);
        else {
            D(std::cout << "[ERROR] Invalid channel for Radio Mode. Check table to see available values with ./tuxniffer -l." << std::endl;)
            exit(-1);
        }
        break;
    case 1:
        if(channel >= 0 && channel <= 33) finalFreq = 863.125 + (channel * 0.2);
        else {
            D(std::cout << "[ERROR] Invalid channel for Radio Mode. Check table to see available values with ./tuxniffer -l." << std::endl;)
            exit(-1);
        }
        break;
    case 2:
        if(channel >= 0 && channel <= 6) finalFreq = 433.3 + (channel * 0.2);
        else {
            D(std::cout << "[ERROR] Invalid channel for Radio Mode. Check table to see available values with ./tuxniffer -l." << std::endl;)
            exit(-1);
        }
        break;
    case 3:
        if(channel >= 0 && channel <= 128) finalFreq = 902.2 + (channel * 0.2);
        else {
            D(std::cout << "[ERROR] Invalid channel for Radio Mode. Check table to see available values with ./tuxniffer -l." << std::endl;)
            exit(-1);
        }
        break;
    case 4:
        if(channel >= 0 && channel <= 33) finalFreq = 863.125 + (channel * 0.2); 
        else {
            D(std::cout << "[ERROR] Invalid channel for Radio Mode. Check table to see available values with ./tuxniffer -l." << std::endl;)
            exit(-1);
        }
        break;
    case 5:
        if(channel >= 0 && channel <= 6) finalFreq = 433.3 + (channel * 0.2);
        else {
            D(std::cout << "[ERROR] Invalid channel for Radio Mode. Check table to see available values with ./tuxniffer -l." << std::endl;)
            exit(-1);
        }
        break;
    // Wi-SUN 
    case 6:
        if(channel >= 0 && channel <= 128) finalFreq = 863.1 + (channel * 0.1);
        else {
            D(std::cout << "[ERROR] Invalid channel for Radio Mode. Check table to see available values with ./tuxniffer -l." << std::endl;)
            exit(-1);
        }
        break;
    case 7:
        if(channel >= 0 && channel <= 128) finalFreq = 902.2 + (channel * 0.2);
        else {
            D(std::cout << "[ERROR] Invalid channel for Radio Mode. Check table to see available values with ./tuxniffer -l." << std::endl;)
            exit(-1);
        }
        break;
    case 8:
        if(channel >= 0 && channel <= 128) finalFreq = 863.1 + (channel * 0.2);
        else {
            D(std::cout << "[ERROR] Invalid channel for Radio Mode. Check table to see available values with ./tuxniffer -l." << std::endl;)
            exit(-1);
        }
        break;
    case 9:
        if(channel >= 0 && channel <= 128) finalFreq = 902.2 + (channel * 0.2);
        else {
            D(std::cout << "[ERROR] Invalid channel for Radio Mode. Check table to see available values with ./tuxniffer -l." << std::endl;)
            exit(-1);
        }
        break;
    case 10:
        if(channel >= 0 && channel <= 128) finalFreq = 863.1 + (channel * 0.2);
        else {
            D(std::cout << "[ERROR] Invalid channel for Radio Mode. Check table to see available values with ./tuxniffer -l." << std::endl;)
            exit(-1);
        }
        break;
    case 11:
        if(channel >= 0 && channel <= 128) finalFreq = 902.4 + (channel * 0.4);
        else {
            D(std::cout << "[ERROR] Invalid channel for Radio Mode. Check table to see available values with ./tuxniffer -l." << std::endl;)
            exit(-1);
        }
        break;
    case 12:
        if(channel >= 0 && channel <= 128) finalFreq = 920.8 + (channel * 0.6);
        else {
            D(std::cout << "[ERROR] Invalid channel for Radio Mode. Check table to see available values with ./tuxniffer -l." << std::endl;)
            exit(-1);
        }
        break;
    // Zigbee
    case 13:
    case 14:
        if(channel >= 0 && channel <= 128) finalFreq = 863.1 + (channel * 0.2);
        else {
            D(std::cout << "[ERROR] Invalid channel for Radio Mode. Check table to see available values with ./tuxniffer -l." << std::endl;)
            exit(-1);
        }
        break;
    // IEEE 915
    case 15:
        if(channel >= 0 && channel <= 63) finalFreq = 902.4 + (channel * 0.4);
        else {
            D(std::cout << "[ERROR] Invalid channel for Radio Mode. Check table to see available values with ./tuxniffer -l." << std::endl;)
            exit(-1);
        }
        break;
    // EasyLink/ Generic
    case 16:
        if(channel == 0) finalFreq = 863.125;
        else {
            D(std::cout << "[ERROR] Invalid channel for Radio Mode. Check table to see available values with ./tuxniffer -l." << std::endl;)
            exit(-1);
        }
        break;
    case 17:
        if(channel == 0) finalFreq = 433.3;
        else {
            D(std::cout << "[ERROR] Invalid channel for Radio Mode. Check table to see available values with ./tuxniffer -l." << std::endl;)
            exit(-1);
        }
        break;
    case 18:
        if(channel == 0) finalFreq = 863.125;
        else {
            D(std::cout << "[ERROR] Invalid channel for Radio Mode. Check table to see available values with ./tuxniffer -l." << std::endl;)
            exit(-1);
        }
        break;
    case 19:
        if(channel == 0) finalFreq = 433.3;
        else {
            D(std::cout << "[ERROR] Invalid channel for Radio Mode. Check table to see available values with ./tuxniffer -l." << std::endl;)
            exit(-1);
        }
        break;
    case 20:
        if(channel >= 11 && channel <= 26) finalFreq = 2405 + ((channel - 11) * 5);
        else {
            D(std::cout << "[ERROR] Invalid channel for Radio Mode. Check table to see available values with ./tuxniffer -l." << std::endl;)
            exit(-1);
        }
        break;
    case 21:
        // Channels 37, 38 and 39 are used for advertising
        // There are the only channels that can be used in BLE
        if(channel == 37) finalFreq = 2402;
        else if(channel == 38) finalFreq = 2426;
        else if(channel == 39) finalFreq = 2480;
        else {
            D(std::cout << "[ERROR] Invalid channel for Radio Mode. Check table to see available values with ./tuxniffer -l." << std::endl;)
            exit(-1);
        }
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
    float freq = radio_mode_table[radio_mode].freq;
    std::vector<uint8_t> freq_bytes = convertFreqToByte(calculateFinalFreq(radio_mode, freq, channel));
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
    uint8_t fcs;
    // Exclude SOF, INFO and EOF
    data = std::vector<uint8_t>(data.begin() + 3, data.end() - 2);

    // LENGHT 2B | TIMESTAMP 6B | RSSI 1B | DATA N B | ??? 1B | FCS 1B
    // LENGHT 2B
    length.insert(length.end(), data.begin(), data.begin() + 2);
    // TIMESTAMP 6B
    timestamp.insert(timestamp.end(), data.begin() + 2, data.begin() + 8);

    // NOTE: data[8] does not represent anything in specific. It is not clear what it is.

    // DATA N B
    payload.insert(payload.end(), data.begin() + 9, data.end() - 2);
    // RSSI 1B
    rssi = data[data.size() - 2];
    // FCS
    fcs = data.back();

    // Length and timestamp are in little endian
    std::reverse(length.begin(), length.end());
    std::reverse(timestamp.begin(), timestamp.end());

    // Convert to packet
    packet_data packet = {
        // Packetsize - (1 from RSSI + 1 from FCS + 6 from TIMESTAMP + 1 from ???? (see note above))
        .length = ((length[0] << 8) | length[1]) - 9,
        .device_timestamp = std::chrono::microseconds((static_cast<uint64_t>(timestamp[0]) << 40) | (static_cast<uint64_t>(timestamp[1]) << 32) | (static_cast<uint64_t>(timestamp[2]) << 24) | (static_cast<uint64_t>(timestamp[3]) << 16) | (static_cast<uint64_t>(timestamp[4]) << 8) | static_cast<uint64_t>(timestamp[5])),
        .system_timestamp = system_timestamp,
        .rssi = rssi,
        .data = payload,
        .status = fcs,
        .fcs = fcs
    };

    return packet;
}

// Get device timestamp
std::chrono::microseconds CommandAssembler::get_device_timestamp(std::vector<uint8_t> data)
{
    const size_t expected_length = 13; // 3 (SOF, INFO) + 6 (TIMESTAMP) + 2 (EOF) + 2 (padding or other data)
    const size_t timestamp_offset = 5; // 3 (SOF, INFO) + 2 (padding or other data)

    if (data.size() < expected_length) {
        D(std::cout << "[ERROR] Data vector is too short to contain a valid timestamp." << std::endl;)
        return std::chrono::microseconds(0);
    }

    // Extract the 6-byte timestamp directly from the data, considering the offset
    uint64_t timestamp = 0;
    for (size_t i = 0; i < 6; ++i) {
        timestamp |= static_cast<uint64_t>(data[timestamp_offset + i]) << (i * 8);
    }

    return std::chrono::microseconds(timestamp);
}