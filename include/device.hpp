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

#include <string>
#include <vector>

#include "command_assembler.hpp"
#include "serial.hpp"
#include "common.hpp"
#include "output_manager.hpp"

/**
 * @enum State
 * @brief Represents the state of the device.
 */
enum class State
{
    WAITING_FOR_COMMAND, ///< Firmware state: Waiting for a command.
    INIT,                ///< Firmware state: Initializing.
    STARTED,             ///< Firmware state: Started.
    STOPPED,             ///< Firmware state: Stopped.
    READING,             ///< Software state: Reading data.
    WRITING,             ///< Software state: Writing data.
};

/**
 * @class Device
 * @brief Manages the communication and operations with a device.
 */
class Device
{
public:
    Serial serial;                 ///< Serial communication handler.
    CommandAssembler cmd;          ///< Command assembler for creating and parsing commands.
    State state;                   ///< Current state of the device.
    OutputManager* output_manager; ///< Pointer to the output manager.

    bool is_ready = false;         ///< Indicates if the device is ready.
    bool is_streaming = false;     ///< Indicates if the device is currently streaming data.
    int id;                        ///< Unique identifier for the device.
    std::string port;              ///< Port name for the device.
    uint8_t radio_mode;            ///< Radio mode setting for the device.
    uint8_t channel;               ///< Channel setting for the device.

    /**
     * @brief Constructor for the Device class.
     * 
     * @param device Device settings.
     * @param id_counter ID counter for assigning a unique ID.
     */
    Device(device_s device, int id_counter);

    /**
     * @brief Connects to the device.
     * 
     * @return true if the connection was successful, false otherwise.
     */
    bool connect();

    /**
     * @brief Initializes the device.
     * 
     * @return true if initialization was successful, false otherwise.
     */
    bool init();

    /**
     * @brief Starts the device operation.
     * 
     * @return true if the start was successful, false otherwise.
     */
    bool start();

    /**
     * @brief Stops the device operation.
     * 
     * @return true if the stop was successful, false otherwise.
     */
    bool stop();

    /**
     * @brief Sends a ping command to the device.
     * 
     * @return true if the ping was successful, false otherwise.
     */
    bool ping();

    /**
     * @brief Configures the device settings.
     * 
     * @return true if configuration was successful, false otherwise.
     */
    bool configure();

    /**
     * @brief Streams data from the device.
     */
    void stream();

    /**
     * @brief Streams data from the device for a specified duration.
     * 
     * @param seconds Duration for streaming data.
     */
    void stream(std::chrono::seconds seconds);

    /**
     * @brief Disconnects from the device.
     * 
     * @return true if disconnection was successful, false otherwise.
     */
    bool disconnect();

    /**
     * @brief Receives a response from the device.
     * 
     * @param ret Vector to store the received response.
     * @return true if the response was successfully received, false otherwise.
     */
    bool receive_response(std::vector<uint8_t> &ret);
};
