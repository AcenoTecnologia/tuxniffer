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
#include <string>

enum class FrameState
{
    // Begin of frame
    S_BOF_1,
    S_BOF_2,
    // Data
    S_DATA,
    // Frame info
    S_INFO,
    // Frame FCS
    S_FCS,
    // Frame RSSI
    // Frame length
    S_LENGTH_1,
    S_LENGTH_2,
    // End of frame
    S_EOF_1,
    S_EOF_2,
    // Extras
    S_ERROR,
    S_SUCCESS,
};

class Framer
{
public:
    Framer();

    FrameState process(uint8_t byte);
    void reset();
    void recover();
    std::string getStateString();
private:
    FrameState state;
    // FrameError error;
    bool is_valid;

    uint8_t frame_type;
    uint16_t data_length;
    uint8_t frame_fcs;
    uint8_t frame_rssi;
    uint64_t frame_timestamp;
    uint8_t frame_status;
    uint8_t frame_payload[256];
    uint16_t payload_size;
    int current_data_index = 0;

    FrameState process_ping(uint8_t byte);
    FrameState process_stream(uint8_t byte);
    FrameState process_stream_error(uint8_t byte);
    FrameState process_basic(uint8_t byte);


};