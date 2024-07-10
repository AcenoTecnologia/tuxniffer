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

#include <cstdint>
#include <vector>
#include <chrono>

#include "common.hpp"

struct phy_table_entry
{
    uint8_t phy;
    float freq;
};


//Delimitors
const std::vector<uint8_t> sof          = {0x40, 0x53};
const std::vector<uint8_t> eof          = {0x40, 0x45};
// Raw frequency values
const float f433                         = 433;
const float f868                         = 868;
const float f915                         = 915;
const float f2405                        = 2405;
// Commands info bytes
const uint8_t info_ping =                  0x40;
const uint8_t info_start =                 0x41;
const uint8_t info_stop =                  0x42;
const uint8_t info_freq =                  0x45;
const uint8_t info_phy =                   0x47;

/*
    To insert more PHYs, you can transform the array into a matrix and add the new info.
    The first line would be, for example:

    phy_table_entry phy_table[20][2] = {LP-CC1352P7             Some other board
    ieee_868_915                      {{0x00, F868},              {0x00, {F868}},
*/

const phy_table_entry radio_mode_table[22] = {   /*LP-CC1352P7*/      
/* ieee_868_915         */          {0x00, f868},
/* ieee_868_915         */          {0x00, f915},
/* ieee_433             */          {0x01, f433},
/* ieee_868_915_slr     */          {0x02, f868},
/* ieee_868_915_slr     */          {0x02, f915},
/* ieee_433_slr         */          {0x03, f433},
/* wiSun_868_915_50a    */          {0x04, f868},
/* wiSun_868_915_50b    */          {0x05, f915},
/* wiSun_868_915_100a   */          {0x06, f868},
/* wiSun_868_915_100b   */          {0x07, f915},
/* wiSun_868_915_150    */          {0x08, f868},
/* wiSun_868_915_200a   */          {0x09, f915},
/* wiSun_868_915_200b   */          {0x0A, f915},
/* zigbee_868_915_100   */          {0x0B, f868},
/* zigbee_868_915_500   */          {0x0C, f868},
/* ieee_915             */          {0x0D, f915},
/* easyLink_868_915_50  */          {0x0E, f868},
/* easyLink_433_50      */          {0x0F, f433},
/* easyLink_868_915_slr */          {0x10, f868},
/* easyLink_433_slr     */          {0x11, f433},
/* ieee_2405            */          {0x12, f2405},
/* ble_2405             */          {0x13, f2405}
};

class CommandAssembler
{
public:
    // Command assemblers
    std::vector<uint8_t> assemble_command(uint8_t info, std::vector<uint8_t> data);
    std::vector<uint8_t> assemble_command(uint8_t info);
    std::vector<uint8_t> assemble_start();
    std::vector<uint8_t> assemble_stop();
    std::vector<uint8_t> assemble_ping();
    std::vector<uint8_t> assemble_set_freq(uint8_t radio_mode, int channel);
    std::vector<uint8_t> assemble_set_phy(uint8_t radio_mode);

    // Command disassemblers
    bool verify_response(std::vector<uint8_t> response);
    std::vector<uint8_t> disassemble_ping(std::vector<uint8_t> response);

    // Command converters
    packet_data convert_to_network_packet(std::vector<uint8_t> data, std::chrono::microseconds system_timestamp);
    std::chrono::microseconds get_device_timestamp(std::vector<uint8_t> data);
    float calculateFinalFreq(uint8_t phy, float freq, int channel);
    std::vector<uint8_t> convertFreqToByte(float freq);
private:
    uint8_t calculate_fcs(std::vector<uint8_t> data);
};

