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

#include <cstdint>
#include <vector>
#include <chrono>

#include "common.hpp"

/**
 * @struct phy_table_entry
 * @brief Represents an entry in the PHY table.
 */
struct phy_table_entry
{
    uint8_t phy;   ///< PHY identifier.
    float freq;    ///< Frequency value.
};

// These values were found at packet-tirpi.c from the TI wireshark dissector source code.
/* Protocol values */
const uint8_t PROTOCOL_GENERIC                      = 0;
const uint8_t PROTOCOL_IEEE_802_15_4_G              = 1;
const uint8_t PROTOCOL_IEEE_802_15_4                = 2;
const uint8_t PROTOCOL_BLE                          = 3;
const uint8_t PROTOCOL_WBMS                         = 4;

// These values were found at packet-tirpi.c from the TI wireshark dissector source code.
/* PHY type values */
const uint8_t PHY_TYPE_UNUSED                       = 0;
const uint8_t PHY_TYPE_50KBPS_GFSK                  = 1;
const uint8_t PHY_TYPE_SLR                          = 2;
const uint8_t PHY_TYPE_OQPSK                        = 3;
const uint8_t PHY_TYPE_200KBPS_GFSK                 = 4;
const uint8_t PHY_TYPE_BLE                          = 5;
const uint8_t PHY_TYPE_WBMS                         = 6;
const uint8_t PHY_TYPE_50KBPS_GFSK_WISUN_1A         = 7;
const uint8_t PHY_TYPE_50KBPS_GFSK_WISUN_1B         = 8;
const uint8_t PHY_TYPE_100KBPS_GFSK_WISUN_2A        = 9;
const uint8_t PHY_TYPE_100KBPS_GFSK_WISUN_2B        = 10;
const uint8_t PHY_TYPE_150KBPS_GFSK_WISUN_3         = 11;
const uint8_t PHY_TYPE_200KBPS_GFSK_WISUN_4A        = 12;
const uint8_t PHY_TYPE_200KBPS_GFSK_WISUN_4B        = 13;
const uint8_t PHY_TYPE_100KBPS_GFSK_ZIGBEE_R23      = 14;
const uint8_t PHY_TYPE_500KBPS_GFSK_ZIGBEE_R23      = 15;


// Delimiters
const std::vector<uint8_t> sof = {0x40, 0x53}; ///< Start of frame delimiter.
const std::vector<uint8_t> eof = {0x40, 0x45}; ///< End of frame delimiter.

// Raw frequency values
const float f433 = 433;     ///< 433 MHz frequency.
const float f868 = 868;     ///< 868 MHz frequency.
const float f915 = 915;     ///< 915 MHz frequency.
const float f2405 = 2405;   ///< 2405 MHz frequency.

// Command info bytes
const uint8_t info_ping = 0x40;  ///< Ping command identifier.
const uint8_t info_start = 0x41; ///< Start command identifier.
const uint8_t info_stop = 0x42;  ///< Stop command identifier.
const uint8_t info_freq = 0x45;  ///< Frequency command identifier.
const uint8_t info_phy = 0x47;   ///< PHY command identifier.

/**
 * @brief PHY table for different radio modes.
 * 
 * To insert more PHYs, you can transform the array into a matrix and add the new info.
 * For example:
 * phy_table_entry phy_table[20][2] = {LP-CC1352P7, Some other board}
 * ieee_868_915 {{0x00, F868}, {0x00, {F868}},
 */
const phy_table_entry radio_mode_table[22] = {   
    /* LP-CC1352P7 */      
    /* ieee_868_915 */          {0x00, f868},
    /* ieee_868_915 */          {0x00, f915},
    /* ieee_433 */              {0x01, f433},
    /* ieee_868_915_slr */      {0x02, f868},
    /* ieee_868_915_slr */      {0x02, f915},
    /* ieee_433_slr */          {0x03, f433},
    /* wiSun_868_915_50a */     {0x04, f868},
    /* wiSun_868_915_50b */     {0x05, f915},
    /* wiSun_868_915_100a */    {0x06, f868},
    /* wiSun_868_915_100b */    {0x07, f915},
    /* wiSun_868_915_150 */     {0x08, f868},
    /* wiSun_868_915_200a */    {0x09, f915},
    /* wiSun_868_915_200b */    {0x0A, f915},
    /* zigbee_868_915_100 */    {0x0B, f868},
    /* zigbee_868_915_500 */    {0x0C, f868},
    /* ieee_915 */              {0x0D, f915},
    /* easyLink_868_915_50 */   {0x0E, f868},
    /* easyLink_433_50 */       {0x0F, f433},
    /* easyLink_868_915_slr */  {0x10, f868},
    /* easyLink_433_slr */      {0x11, f433},
    /* ieee_2405 */             {0x12, f2405},
    /* ble_2405 */              {0x13, f2405}
};

/**
 * @class CommandAssembler
 * @brief Assembles and disassembles commands for packet sniffer.
 */
class CommandAssembler
{
public:
    /**
     * @brief Assembles a generic command with data.
     * This methods put the data in the format:
     * [SOF] [INFO] [LEN] [DATA] [FCS] [EOF]
     * 
     * @param info Command info byte.
     * @param data Data to be included in the command.
     * @return Assembled command.
     */
    std::vector<uint8_t> assemble_command(uint8_t info, std::vector<uint8_t> data);

    /**
     * @brief Assembles a command without data.
     * This methods put the data in the format:
     * [SOF] [INFO] [LEN (0x0000)] [FCS] [EOF]
     * 
     * @param info Command info byte.
     * @return Assembled command.
     */
    std::vector<uint8_t> assemble_command(uint8_t info);

    /**
     * @brief Assembles a start command.
     * 
     * @return Assembled start command.
     */
    std::vector<uint8_t> assemble_start();

    /**
     * @brief Assembles a stop command.
     * 
     * @return Assembled stop command.
     */
    std::vector<uint8_t> assemble_stop();

    /**
     * @brief Assembles a ping command.
     * 
     * @return Assembled ping command.
     */
    std::vector<uint8_t> assemble_ping();

    /**
     * @brief Assembles a set frequency command.
     * 
     * @param radio_mode Radio mode identifier.
     * @param channel Channel number.
     * @return Assembled set frequency command.
     */
    std::vector<uint8_t> assemble_set_freq(uint8_t radio_mode, int channel);

    /**
     * @brief Assembles a set PHY command.
     * 
     * @param radio_mode Radio mode identifier.
     * @return Assembled set PHY command.
     */
    std::vector<uint8_t> assemble_set_phy(uint8_t radio_mode);

    /**
     * @brief Verifies a response.
     * 
     * @param response Response to be verified.
     * @return true if the response is valid, false otherwise.
     */
    bool verify_response(std::vector<uint8_t> response);

    /**
     * @brief Disassembles a ping response.
     * 
     * @param response Response to be disassembled.
     * @return Disassembled ping data.
     */
    std::vector<uint8_t> disassemble_ping(std::vector<uint8_t> response);

    /**
     * @brief Converts data to a network packet.
     * 
     * @param data Data to be converted.
     * @param system_timestamp System timestamp.
     * @return Converted network packet.
     */
    packet_data convert_to_network_packet(std::vector<uint8_t> data, std::chrono::microseconds system_timestamp);

    /**
     * @brief Gets the device timestamp from data.
     * 
     * @param data Data containing the timestamp.
     * @return Device timestamp.
     */
    std::chrono::microseconds get_device_timestamp(std::vector<uint8_t> data);

    /**
     * @brief Calculates the final frequency.
     * 
     * @param mode Radio Mode value.
     * @param freq Frequency value.
     * @param channel Channel number.
     * @return Calculated final frequency.
     */
    float calculateFinalFreq(uint8_t mode, float freq, int channel);

    /**
     * @brief Converts frequency to byte format in little endian format.
     * 
     * @param freq Frequency value.
     * @return Frequency in byte format.
     */
    std::vector<uint8_t> convertFreqToByte(float freq);

    /**
     * @brief Gets the PHY TI value for a radio mode.
     * 
     * @param radio_mode Radio mode identifier.
     * @return PHY value that has to be sent to wireshark packet TI header info.
     */
    uint8_t get_ti_phy_value(uint8_t radio_mode);

    /**
     * @brief Gets the protocol TI value for a radio mode.
     * 
     * @param radio_mode Radio mode identifier.
     * @return Protocol value that has to be sent to wireshark packet TI header info.
     */
    uint8_t get_protocol_value(uint8_t radio_mode);

private:
    /**
     * @brief Calculates the Frame Check Sequence (FCS) for the data.
     * The equation used is (sum(bytes(data)) + info + len) & 0xFF. 
     * 
     * @param data Data for which the FCS is to be calculated.
     * @return Calculated FCS.
     */
    uint8_t calculate_fcs(std::vector<uint8_t> data);
};
