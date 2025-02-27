////////////////////////////////////////////////////////////////////////////////////////////////////
// Company:  Aceno Digital Tecnologia em Sistemas Ltda.
// Homepage: http://www.aceno.com
// Project:  Tuxniffer
// Version:  1.1.3
// Date:     2025
//
// Copyright (C) 2002-2025 Aceno Tecnologia.
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////


#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <signal.h>
#include <chrono>
#include <thread>
#include <cstring> 

#include "common.hpp"
#include "device.hpp"
#include "framer.hpp"

#ifdef __linux__
#include <unistd.h> 
#endif

int interruption = 0;

void signal_handler(int sig)
{
    if(interruption)
    {
        exit(0);
    }
    sig = sig;
	interruption = 1;
    std::cout << std::endl;
    std::cout << "[INTERRUPTION] Signal " << sig << " received. The stream will stop after recieving the next packet." << std::endl;
    std::cout << "[INTERRUPTION] It may take a while to close if is saving queued packets to file/ pipe." << std::endl;
    std::cout << "[INTERRUPTION] Press CTRL+C again to kill program. May cause data loss!" << std::endl;

}


Device::Device(device_s device, int id_counter, std::mutex &coutMutex)
    : serial(device.port),  // Initialize serial with device.port
      cmd(),  // Initialize CommandAssembler
      coutMutex(coutMutex)
{
    state = State::WAITING_FOR_COMMAND;
    id = id_counter;
    port = device.port;
    radio_mode = device.radio_mode;
    channel = device.channel;
}

bool Device::connect()
{
    is_ready = serial.connect();
    return is_ready;
}

bool Device::init()
{
    serial.purge();
    uint8_t fwID;
    if(!stop()) return false;
    if(!ping(&fwID)) return false;
    if(!configure(fwID)) return false;

   return true;
}

bool Device::start()
{
    std::vector<uint8_t> response;
    // Send start command
    serial.flush();
    D({std::lock_guard<std::mutex> lock(coutMutex); std::cout << "[INFO] Assembling start command." << std::endl;});
    std::vector<uint8_t> start_command = cmd.assemble_start();
    serial.writeData(start_command);
    receive_response(response);
    if(!cmd.verify_response(response)) return false;
    std::lock_guard<std::mutex> lock(coutMutex);
    std::cout << "[INFO] Device [" << id << "] started." << std::endl;
    state = State::STARTED;

    return true;
}

bool Device::stop()
{
    std::vector<uint8_t> response;
    // Send stop command
    serial.flush();
    D({std::lock_guard<std::mutex> lock(coutMutex); std::cout << "[INFO] Assembling stop command." << std::endl;});
    std::vector<uint8_t> stop_command = cmd.assemble_stop();
    serial.writeData(stop_command);
    receive_response(response);
    if(!cmd.verify_response(response)) return false;
    D({std::lock_guard<std::mutex> lock(coutMutex); std::cout << "[INFO] Device [" << id << "] stopped." << std::endl;})

    state = State::STOPPED;

    return true;
}

bool Device::ping(uint8_t* fwID)
{

    if(state != State::STOPPED)
    {
        D(std::lock_guard<std::mutex> lock(coutMutex); std::cout << "[ERROR] Device [" << id << "] must be stopped before pinging." << std::endl;)
        return false;
    }

    std::vector<uint8_t> response;
    // Send ping command
    serial.flush();
    D({std::lock_guard<std::mutex> lock(coutMutex); std::cout << "[INFO] Assembling ping command." << std::endl;});
    std::vector<uint8_t> ping_command = cmd.assemble_ping();
    serial.writeData(ping_command);
    receive_response(response);
    if(!cmd.verify_response(response)) return false;
    // TODO: Update board info
    std::vector<uint8_t> board_info = cmd.disassemble_ping(response);
    *fwID = board_info[3];
    D(std::lock_guard<std::mutex> lock(coutMutex); 
    std::cout << "[INFO] Device [" << id << "] pinged." << std::endl;
    std::cout << "[INFO] Device [" << id << "] board info: " << std::endl;
    std::cout << "[INFO] ---Chip ID: " << std::hex << (int)board_info[0] << (int)board_info[1] << "." << std::endl;
    std::cout << "[INFO] ---Chip Rev: " << std::hex << (int)board_info[2] << "." << std::endl;
    std::cout << "[INFO] ---FW ID: " << std::hex << (int)board_info[3] << "." << std::endl;
    std::cout << "[INFO] ---FW Rev: " << std::hex << (int)board_info[4] << "." << (int)board_info[5] << std::endl;)
    return true;
}

bool Device::configure(uint8_t fwID)
{

    if(state != State::STOPPED)
    {
        D(std::lock_guard<std::mutex> lock(coutMutex); std::cout << "[ERROR] Device [" << id << "] must be stopped before configuring." << std::endl;)
        return false;
    }

    std::vector<uint8_t> response;
    // Send phy command
    serial.flush();

    D({std::lock_guard<std::mutex> lock(coutMutex); std::cout << "[INFO] Assembling set PHY command." << std::endl;});
    std::vector<uint8_t> set_phy_command = cmd.assemble_set_phy(radio_mode, fwID);
    if (set_phy_command.empty())
    {
        D({std::lock_guard<std::mutex> lock(coutMutex); std::cout << "[ERROR] Radio mode" << radio_mode << "not avaliable on device (firmware ID : " << fwID << ")." << std::endl;})
    }
    serial.writeData(set_phy_command);
    receive_response(response);
    if(!cmd.verify_response(response)) return false;
    D({std::cout << "[INFO] Device [" << id << "] set PHY." << std::endl;})

    // Send frequency command
    serial.flush();
    D({std::lock_guard<std::mutex> lock(coutMutex); std::cout << "[INFO] Assembling set frequency command." << std::endl;});
    std::vector<uint8_t> set_freq_command = cmd.assemble_set_freq(radio_mode, channel, fwID);
    if (set_phy_command.empty())
    {
        D({std::lock_guard<std::mutex> lock(coutMutex); std::cout << "[ERROR] Radio mode" << radio_mode << "not avaliable on device (firmware ID : " << fwID << ")." << std::endl;})
    }
    serial.writeData(set_freq_command);
    receive_response(response);
    if(!cmd.verify_response(response)) return false;
    D({std::lock_guard<std::mutex> lock(coutMutex); std::cout << "[INFO] Device [" << id << "] set frequency." << std::endl;})

    return true;
}

void Device::stream()
{

    // ctrl-c
	signal(SIGINT, signal_handler);
	// killall
	signal(SIGTERM, signal_handler);

    is_streaming = true;
    int totalPackets = 0;
    while(is_streaming)
    {
        std::vector<uint8_t> response;
        if(!receive_response(response) && interruption)
        {
            // Reset SIGINT to default behavior
            signal(SIGINT, SIG_DFL);
            // Reset SIGTERM to default behavior
            signal(SIGTERM, SIG_DFL);
            return;
        }
        if(!cmd.verify_response(response)) continue;
        totalPackets++;
        D({std::lock_guard<std::mutex> lock(coutMutex); std::cout << "[INFO] Device [" << id << "] received packet (" << std::dec << totalPackets << " received)." << std::endl;})
        if (output_manager != nullptr) output_manager->add_packet(
            {id, port, channel, radio_mode, response, std::chrono::system_clock::now()}
        );
        if(interruption) is_streaming = false;
    }

    // Reset SIGINT to default behavior
    signal(SIGINT, SIG_DFL);
    // Reset SIGTERM to default behavior
    signal(SIGTERM, SIG_DFL);
}

void Device::stream(std::chrono::seconds seconds)
{
    
    // ctrl-c
	signal(SIGINT, signal_handler);
	// killall
	signal(SIGTERM, signal_handler);

    is_streaming = true;
    int totalPackets = 0;
    auto start_time = std::chrono::steady_clock::now();
    while(is_streaming)
    {
        std::vector<uint8_t> response;
        if(!receive_response(response) && interruption)
        {
            // Check if time has elapsed
            //auto current_time = std::chrono::steady_clock::now();
            //auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time);
            //if (elapsed_time >= seconds) is_streaming = false;
            //if (interruption) is_streaming = false;
            is_streaming = false;
        }
        if(!cmd.verify_response(response)) continue;
        totalPackets++;
        D({std::lock_guard<std::mutex> lock(coutMutex); std::cout << "[INFO] Device [" << id << "] received packet (" << std::dec << totalPackets << " received)." << std::endl;})
        if (output_manager != nullptr) output_manager->add_packet(
            {id, port, channel, radio_mode, response, std::chrono::system_clock::now()}
        );
        // Check if time has elapsed
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time);
        if (elapsed_time >= seconds) is_streaming = false;
        if (interruption) is_streaming = false;
    }

    
    // Reset SIGINT to default behavior
    signal(SIGINT, SIG_DFL);
    // Reset SIGTERM to default behavior
    signal(SIGTERM, SIG_DFL);
}

bool Device::disconnect()
{
    return serial.disconnect();
}

bool Device::receive_response(std::vector<uint8_t>& ret)
{
    std::vector<uint8_t> response;
    Framer framer;

    // Set timeout duration to 10 seconds (adjust as needed)
    auto timeout_duration = std::chrono::seconds(10);
    auto start_time = std::chrono::steady_clock::now();

    while (true)
    {
        // Read byte from serial
        uint8_t byte;
        int read_byte = serial.readByte(&byte);

        if(read_byte == 0)
        {

            // Check if timeout has occurred
            auto current_time = std::chrono::steady_clock::now();
            auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time);

            if (elapsed_time >= timeout_duration)
            {
                D({std::lock_guard<std::mutex> lock(coutMutex); std::cout << "[INFO] Timeout reached, no response received from Device [" << id << "] within 10 seconds." << std::endl;})
                return false;
            }

            // Sleep for a short period before checking again
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        if (read_byte == -1)
        {
            if (!reconnect()){
                return false;
            }
            return receive_response(ret);
        }

        // Push byte to response
        response.push_back(byte);

        // Process byte
        std::string oldState = framer.getStateString();
        FrameState state = framer.process(byte);
        // std::cout << oldState << "-(" << std::hex << static_cast<int>(byte) << ")->" << framer.getStateString() << std::endl;

        // If the frame is complete
        if (state == FrameState::S_SUCCESS)
        {
            // Copy response to ret
            ret = response;
            return true;
        }
        if (state == FrameState::S_ERROR)
        {
            response.clear();
            framer.recover();
        }

        // Reset timeout start time since we received a byte
        start_time = std::chrono::steady_clock::now();
    }

    return false;
}

bool Device::reconnect()
{
    {
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cout << "[ERROR] Connection lost with Device [" << id << "]." << std::endl;
    }
    if (!disconnect())
    {
        D({std::lock_guard<std::mutex> lock(coutMutex); std::cout << "[ERROR] Error closing serial port. Impossible to reconect Device [" << id << "]." << std::endl;})
        interruption = 1;
        return false;
    }
    D({std::lock_guard<std::mutex> lock(coutMutex); std::cout << "[INFO] Trying to reconnect with Device [" << id << "] in 10 seconds." << std::endl;})
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    while (true)
    {
        
        serial.closePort();
        if(connect()){
            if(init()){
                if(start())
                {
                    std::lock_guard<std::mutex> lock(coutMutex);
                    std::cout << "[INFO] Reconnected with Device [" << id << "]." << std::endl;
                    return true;
                }
            }
        }    
        D({std::lock_guard<std::mutex> lock(coutMutex); std::cout << "[ERROR] Reconnection with Device [" << id << "] failed. Trying again in 10 seconds." << std::endl;})
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
        if (interruption) return false;
        
    }
    return false;
}
