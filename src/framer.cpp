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


#include <cstring>
#include <string>
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <vector>

#ifdef __linux__
#include <unistd.h>
#endif


#include "framer.hpp"

Framer::Framer()
{
    reset();
}

FrameState Framer::process(uint8_t byte)
{
    switch(state)
    {
        case FrameState::S_ERROR:
            return FrameState::S_ERROR;
        case FrameState::S_SUCCESS:
            reset();
            break;
        case FrameState::S_BOF_1:
            if(byte == 0x40) state = FrameState::S_BOF_2;
            else state = FrameState::S_ERROR;
            break;
        case FrameState::S_BOF_2:
            if(byte == 0x53) state = FrameState::S_INFO;
            else state = FrameState::S_ERROR;
            break;
        case FrameState::S_INFO:
            if(byte == 0xC0) data_length--;
            frame_type = byte;
            state = FrameState::S_LENGTH_1;
            break;
        case FrameState::S_LENGTH_1:
            data_length += byte;
            state = FrameState::S_LENGTH_2;
            break;
        case FrameState::S_LENGTH_2:
            data_length = (byte << 8) | data_length;
            state = FrameState::S_DATA;
            break;
        case FrameState::S_DATA:
            frame_payload[current_data_index] = byte;
            if(current_data_index >= data_length-1) state = FrameState::S_FCS;
            current_data_index++;
            break;
        case FrameState::S_FCS:
            frame_fcs = byte;
            if(byte != 0x00) state = FrameState::S_EOF_1;
            else state = FrameState::S_ERROR;
            break;
        case FrameState::S_EOF_1:
            if(byte == 0x40) state = FrameState::S_EOF_2;
            else state = FrameState::S_ERROR;
            break;
        case FrameState::S_EOF_2:
            if(byte == 0x45)
            {
                state = FrameState::S_SUCCESS;
                is_valid = true;
            }
            else state = FrameState::S_ERROR;
            break;
    }
    return state;
}

void Framer::reset()
{
    state = FrameState::S_BOF_1;
    // error = FrameError::NO_ERROR;
    is_valid = false;

    frame_type = 0;
    data_length = 0;
    frame_fcs = 0;
    frame_rssi = 0;
    frame_timestamp = 0;
    frame_status = 0;
    memset(frame_payload, 0, sizeof(frame_payload));
    payload_size = 0;
    current_data_index = 0;
}

void Framer::recover()
{
    reset();
}

std::string Framer::getStateString()
{
    switch(state)
    {
        case FrameState::S_BOF_1:
            return "S_BOF_1";
        case FrameState::S_BOF_2:
            return "S_BOF_2";
        case FrameState::S_DATA:
            return "S_DATA";
        case FrameState::S_INFO:
            return "S_INFO";
        case FrameState::S_FCS:
            return "S_FCS";
        case FrameState::S_LENGTH_1:
            return "S_LENGTH_1";
        case FrameState::S_LENGTH_2:
            return "S_LENGTH_2";
        case FrameState::S_EOF_1:
            return "S_EOF_1";
        case FrameState::S_EOF_2:
            return "S_EOF_2";
        case FrameState::S_ERROR:
            return "S_ERROR";
        case FrameState::S_SUCCESS:
            return "S_SUCCESS";
    }
    return "UNKNOWN";
}
