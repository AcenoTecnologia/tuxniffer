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
#include <string>

/**
 * @enum FrameState
 * @brief Represents the state of the frame processing.
 */
enum class FrameState
{
    S_BOF_1,       ///< Begin of frame, first byte.
    S_BOF_2,       ///< Begin of frame, second byte.
    S_DATA,        ///< Processing frame data.
    S_INFO,        ///< Processing frame info.
    S_FCS,         ///< Processing frame FCS (Frame Check Sequence).
    S_LENGTH_1,    ///< Processing frame length, first byte.
    S_LENGTH_2,    ///< Processing frame length, second byte.
    S_EOF_1,       ///< End of frame, first byte.
    S_EOF_2,       ///< End of frame, second byte.
    S_ERROR,       ///< Error state.
    S_SUCCESS,     ///< Successful frame processing.
};

/**
 * @class Framer
 * @brief Manages the framing process for packet data.
 */
class Framer
{
public:
    /**
     * @brief Constructs a new Framer object.
     */
    Framer();

    /**
     * @brief Processes a byte of data.
     * 
     * @param byte The byte to process.
     * @return FrameState The current state after processing the byte.
     */
    FrameState process(uint8_t byte);

    /**
     * @brief Resets the framer to its initial state.
     */
    void reset();

    /**
     * @brief Recovers the framer to handle errors.
     * - Resets to the initial state of the framer and keep reading until a new frame is found.
     */
    void recover();

    /**
     * @brief Gets the current state as a string.
     * 
     * @return std::string The current state in string format.
     */
    std::string getStateString();
    
private:
    FrameState state;           ///< Current state of the framer.
    bool is_valid;              ///< Indicates if the frame is valid.

    uint8_t frame_type;         ///< Type of the frame.
    uint16_t data_length;       ///< Length of the data in the frame.
    uint8_t frame_fcs;          ///< Frame Check Sequence value.
    uint8_t frame_rssi;         ///< RSSI value of the frame.
    uint64_t frame_timestamp;   ///< Timestamp of the frame.
    uint8_t frame_status;       ///< Status of the frame.
    uint8_t frame_payload[256]; ///< Payload data of the frame.
    uint16_t payload_size;      ///< Size of the payload.
    int current_data_index = 0; ///< Index of the current data byte being processed.
};
