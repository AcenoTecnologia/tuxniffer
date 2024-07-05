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
#include "fkYAML.hpp"
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

std::vector<device_s> parse_input_file_json(const std::string& filePath, log_s* log)
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
    log->file.enabled = logData.value("enabled", true);
    log->file.path = logData.value("path", "./");
    log->file.base_name = logData.value("base_name", "aceno");
    log->file.split_devices_log = logData.value("splitDevicesLog", false);
    log->file.reset_period = logData.value("resetPeriod", "none");

    const auto& pipeData = data.value("pipe", nlohmann::json::object()); // Get the log object or an empty object if it doesn't exist
    // If log object doesnt exist default values are used
    log->file.enabled = pipeData.value("enabled", true);
    log->file.path = pipeData.value("path", "/tmp/");
    log->file.base_name = pipeData.value("base_name", "aceno");
    log->file.split_devices_log = pipeData.value("splitDevicesLog", false);
    log->file.reset_period = "none";

    // Return the vector of devices
    return devices;
}
// std::vector<device_s> parse_input_file_yaml(const std::string& filePath, log_s* log)
// {
//     if (filePath.empty() || log == nullptr)
//     {
//         std::cout << "[ERROR] Invalid file path or log pointer." << std::endl;
//         return {};
//     }

//     // Try to open the file
//     std::ifstream f(filePath);
//     if (!f.is_open()) {
//         std::cout << "[ERROR] Could not open config file: " << filePath << std::endl;
//         return {};
//     }
    
//     // Read the file content into a string
//     std::string fileContent((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

//     // Parse the YAML file
//     fkYAML::Node data;
//     try
//     {
//         data = fkYAML::Node::Load(fileContent);
//     }
//     catch (const std::exception& e)
//     {
//         std::cout << "[ERROR] Could not parse YAML file: " << e.what() << std::endl;
//         return {};
//     }

//     // Check if YAML has devices field
//     if (!data["devices"].IsDefined())
//     {
//         std::cout << "[ERROR] Missing or empty 'devices' field in YAML file." << std::endl;
//         return {};
//     }

//     // Create a vector of devices
//     std::vector<device_s> devices;

//     // Parse the devices
//     for (const auto& deviceNode : data["devices"])
//     {
//         device_s device;
        
//         // Checks if all required fields exist
//         if (!deviceNode["port"].IsDefined() || !deviceNode["radio_mode"].IsDefined() || !deviceNode["channel"].IsDefined())
//         {
//             std::cout << "[ERROR] Missing required fields (port, radio_mode, or channel) for a device. Skipping device." << std::endl;
//             continue;
//         }

//         // Read device details
//         device.port = deviceNode["port"].as<std::string>();
//         device.radio_mode = deviceNode["radio_mode"].as<int>();
//         device.channel = deviceNode["channel"].as<int>();

//         devices.push_back(device);
//     }

//     // Parse the log settings if available
//     if (data["log"].IsDefined()) {
//         const auto& logData = data["log"];
//         log->file.enabled = logData["enabled"].IsDefined() ? logData["enabled"].as<bool>() : true;
//         log->file.path = logData["path"].IsDefined() ? logData["path"].as<std::string>() : "./";
//         log->file.base_name = logData["base_name"].IsDefined() ? logData["base_name"].as<std::string>() : "aceno";
//         log->file.split_devices_log = logData["splitDevicesLog"].IsDefined() ? logData["splitDevicesLog"].as<bool>() : false;
//         log->file.reset_period = logData["resetPeriod"].IsDefined() ? logData["resetPeriod"].as<std::string>() : "none";
//     }

//     // Parse the pipe settings if available
//     if (data["pipe"].IsDefined()) {
//         const auto& pipeData = data["pipe"];
//         log->file.enabled = pipeData["enabled"].IsDefined() ? pipeData["enabled"].as<bool>() : true;
//         log->file.path = pipeData["path"].IsDefined() ? pipeData["path"].as<std::string>() : "/tmp/";
//         log->file.base_name = pipeData["base_name"].IsDefined() ? pipeData["base_name"].as<std::string>() : "aceno";
//         log->file.split_devices_log = pipeData["splitDevicesPipe"].IsDefined() ? pipeData["splitDevicesPipe"].as<bool>() : false;
//         // Reset period remains "none" based on original code
//     }

//     return devices;
// }

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
    log_s log = {
        {true, "./", "aceno", false, "none"},
        {true, "/tmp/", "aceno", false, "none"}
    };

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
                log.file.base_name = optarg;
                log.pipe.base_name = optarg;
                break;
            case 'l':
                D(std::cout << "Log hourly" << std::endl;)
                log.file.reset_period = "hourly";
                break;
            case 'L':
                D(std::cout << "Log daily" << std::endl;)
                log.file.reset_period = "daily";
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
    if(useInput) devices = parse_input_file_json(configFilePath, &log);

    // Run sniffer passing devices vector
    Sniffer sniffer(devices, log);
    sniffer.configureAllDevices();
    sniffer.initAllDevices();

    return 0;
}
