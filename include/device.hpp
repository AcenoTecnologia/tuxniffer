////////////////////////////////////////////////////////////////////////////////////////////////////
// Company:  Aceno Digital Tecnologia em Sistemas Ltda.
// Homepage: http://www.aceno.com
// Project:  Tuxniffer
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
    WAITING_FOR_COMMAND, ///< Firmware state: Waiting for a command. The sniffer starts in this state.
    INIT,                ///< Firmware state: Initializing. This state is entered after any command is sent and the device is in the WAITING_FOR_COMMAND state.
    STARTED,             ///< Firmware state: Started. In this state the sniffer is streaming data.
    STOPPED,             ///< Firmware state: Stopped. The sniffer is not streaming data. Configuration can be changed.
};

/**
 * @class Device
 * @brief Manages the communication and operations with a device.
 * The sniffer class is responsible for managing the communication with the device, sending commands, and receiving responses.
 * One Sniffer manages many devices.
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
     * @brief Connects to the device serial.
     * 
     * @return true if the connection was successful, false otherwise.
     */
    bool connect();

    /**
     * @brief Initializes the device:
     * - Sends a stop command to the device.
     * - Sends a ping command to the device.
     * - Sends a configuration command to the device.
     * 
     * @return true if initialization was successful, false otherwise.
     */
    bool init();

    /**
     * @brief Starts the device operation.
     * - Sends a start command to the device.
     * 
     * @return true if the start was successful, false otherwise.
     */
    bool start();

    /**
     * @brief Stops the device operation.
     * - Sends a stop command to the device.
     * 
     * @return true if the stop was successful, false otherwise.
     */
    bool stop();

    /**
     * @brief Sends a ping command to the device.
     * - The ping command is used to check if the device is responsive.
     * - It also retrieves the firmware version, revision, and other board information.
     * 
     * @return true if the ping was successful, false otherwise.
     */
    bool ping();

    /**
     * @brief Configures the device settings.
     * - Sets the radio mode, channel and frequency.
     * - Avaiable radio modes can be found in the comman_assembler.hpp file.
     * 
     * @return true if configuration was successful, false otherwise.
     */
    bool configure();

    /**
     * @brief Streams data from the device.
     * - The device must be in the STARTED state.
     * - The device will stream data until the stop command is sent.
     * - The data is sent to the output manager.
     */
    void stream();

    /**
     * @brief Streams data from the device for a specified duration.
     * - The device must be in the STARTED state.
     * - The device will stream data until the defined time is up.
     * - Can have a offset of 10 seconds to account for the time it takes to timeout the receive_response function.
     * - The data is sent to the output manager.
     * 
     * @param seconds Duration for streaming data.
     */
    void stream(std::chrono::seconds seconds);

    /**
     * @brief Disconnects from the device serial.
     * 
     * @return true if disconnection was successful, false otherwise.
     */
    bool disconnect();

    /**
     * @brief Receives a response from the device.
     * - The response is stored in the ret vector.
     * - Is a state machine that only stops when a valid response is received or the timeout is reached.
     * - In case of failure, the function will return false, the vector will be empty and the packet will be discarded.
     * 
     * @param ret Vector to store the received response.
     * @return true if the response was successfully received, false otherwise.
     */
    bool receive_response(std::vector<uint8_t> &ret);


    bool reconnect();
};
