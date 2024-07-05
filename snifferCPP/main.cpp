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


#include <iostream>
#include <getopt.h>
#include <cstdlib>
#include <string>
#include "common.hpp"
#include "json.hpp"
#include "sniffer.hpp"

using json = nlohmann::json;

void print_help()
{
    std::cout << "Usage: ./snifferCPP [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help\t\t\t\tShow this help message and exit" << std::endl;
    std::cout << "Device Settings" << std::endl;
    std::cout << "[REQUIRED]  -p, --port\t\t\t\tSerial port to connect to" << std::endl;
    std::cout << "[REQUIRED]  -m, --radio_mode\t\t\tRadio mode to use" << std::endl;
    std::cout << "[REQUIRED]  -c, --channel\t\t\tChannel to use" << std::endl;
    std::cout << "Log Settings" << std::endl;
    std::cout << "[OPTIONAL]  -b, --base\t\tBase file/ pipe name" << std::endl;
    std::cout << "[OPTIONAL]  -l, --log_hourly\t\t\tLog hourly" << std::endl;
    std::cout << "[OPTIONAL]  -L, --log_daily\t\t\tLog daily" << std::endl;
    std::cout << "[OPTIONAL]  -i, --input\t\t\t\tInput config file" << std::endl;
    std::cout << "(See config.json for a -i example file)" << std::endl;
}

std::vector<device_s> parse_input_file(const std::string& filePath, log_s* log)
{
    if(filePath.empty() || filePath == "" || log == nullptr)
    {
        D(std::cout << "[ERROR] Invalid file path or log pointer." << std::endl;)
        return std::vector<device_s>();
    }

    // Try to open the file
    std::ifstream f(filePath);
    if (!f.is_open()) {
        D(std::cout << "[ERROR] Could not open config file: " << filePath << std::endl;)
        // Return an empty vector
        return std::vector<device_s>();
    }
    
    // Parse the JSON file
    json data;
    try
    {
        data = json::parse(f);
    }
    catch(json::parse_error& e)
    {
        D(std::cout << "[ERROR] Could not parse JSON file: " << e.what() << std::endl;)
        // Return an empty vector
        return std::vector<device_s>();
    }

    // Check if JSON has devices field
    if (!data.contains("devices"))
    {
        D(std::cout << "[ERROR] Missing devices field in JSON file." << std::endl;)
        // Return an empty vector
        return std::vector<device_s>();
    }
    
    // Create a vector of devices
    std::vector<device_s> devices;

    // Parse the devices
    for (auto& device : data["devices"])
    {
        // Checks if all required fields exists
        if (!device.contains("port") || !device.contains("radio_mode") || !device.contains("channel"))
        {
            std::cout << "[ERROR] Missing required fields (port, radio_mode, or channel) for a device. Skipping device." << std::endl;
            continue;
        }
        
        devices.push_back(device_s{
            device["port"].get<std::string>(),
            device["radio_mode"].get<int>(),
            device["channel"].get<int>()
        });
    }

    // Parse the log settings
    const auto& logData = data.value("log", nlohmann::json::object()); // Get the log object or an empty object if it doesn't exist
    // If log object doesnt exist default values are used
    log->use_pipe = logData.value("use_pipe", true);
    log->perDevice = logData.value("perDevice", false);
    log->base = logData.value("base", "outputLog");
    log->log_hourly = logData.value("log_hourly", false);
    log->log_daily = logData.value("log_daily", false);

    // Return the vector of devices
    return devices;
}

int main(int argc, char* argv[])
{
    D(std::cout << "[INFO] Debug mode is on!" << std::endl;)

    const char* const short_opts = "hp:m:c:b:li:L";    
    const option long_opts[] = {
        {"help",        no_argument,       nullptr, 'h'},
        {"port",        required_argument, nullptr, 'p'},
        {"radio_mode",  required_argument, nullptr, 'm'},
        {"channel",     required_argument, nullptr, 'c'},
        {"base",        required_argument, nullptr, 'b'},
        {"log_hourly",  no_argument,       nullptr, 'l'},
        {"input",       required_argument, nullptr, 'i'},
        {"log_daily",   no_argument,       nullptr, 'L'},
        {nullptr,       no_argument,       nullptr, 0}
    };

    bool useInput = false;
    std::string configFilePath;
    std::vector<device_s> devices;
    device_s device;

    // If theres no input file to config the log, use default values
    log_s log;
    log.use_pipe = true;
    log.perDevice = false;

    while(true)
    {

        const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);
        if(-1 == opt || useInput) break;

        switch(opt)
        {
            case 'h':
                print_help();
                return 0;
            case 'p':
                D(std::cout << "Serial port: " << optarg << std::endl;)
                device.port = optarg;
                break;
            case 'm':
                D(std::cout << "Radio mode: " << optarg << std::endl;)
                device.radio_mode = std::stoi(optarg);
                break;
            case 'c':
                D(std::cout << "Channel: " << optarg << std::endl;)
                device.channel = std::stoi(optarg);
                break;
            case 'b':
                D(std::cout << "Base: " << optarg << std::endl;)
                log.base = optarg;
                break;
            case 'l':
                D(std::cout << "Log hourly" << std::endl;)
                log.log_hourly = true;
                break;
            case 'L':
                D(std::cout << "Log daily" << std::endl;)
                log.log_daily = true;
                break;
            case 'i':
                D(std::cout << "Input config file: " << optarg << ". Ignoring other input commands." << std::endl;)
                configFilePath = optarg;
                useInput = true;
                break;            
        }
    }

    // If useInput is false, add parameter device to device vector
    if(!useInput) devices.push_back(device); 
    // If useInput is true, parse JSON file and add devices to device vector and update log settings
    if(useInput) devices = parse_input_file(configFilePath, &log);

    // Run sniffer passing devices vector
    Sniffer sniffer(devices, log);
    sniffer.configureAllDevices();
    sniffer.initAllDevices();

    return 0;
}
