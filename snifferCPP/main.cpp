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
#include "fkYAML.hpp"
#include "sniffer.hpp"

#ifdef __linux__
#define DEFAULT_PIPE_PATH "/tmp/"
#endif
#ifdef _WIN32
#define DEFAULT_PIPE_PATH "\\\\.\\pipe\\"
#endif

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
    std::cout << "(See config.yaml for a -i example file)" << std::endl;
}

std::vector<device_s> parse_input_file_yaml(const std::string& filePath, log_s* log)
{
    if (filePath.empty() || log == nullptr)
    {
        std::cout << "[ERROR] Invalid file path or log pointer." << std::endl;
        return std::vector<device_s>();
    }

    // Try to open the file
    std::ifstream f(filePath);
    if (!f.is_open()) {
        std::cout << "[ERROR] Could not open config file: " << filePath << std::endl;
        return std::vector<device_s>();
    }


    fkyaml::node yaml;
    try
    {
        // Load the YAML file
        std::ifstream ifs(filePath);
        // deserialize the loaded file contents.
        yaml = fkyaml::node::deserialize(ifs);
    }
    catch(const std::exception& e)
    {
        std::cout << "[ERROR] Could not open or parse YAML file: " << e.what() << std::endl;
        // Return an empty vector
        return std::vector<device_s>();
    }

    // Check if has field devices
    if (!yaml.contains("devices")) {
        std::cout << "[ERROR] Missing devices field in YAML file." << std::endl;
        return std::vector<device_s>();
    }
    // Create a vector of devices
    std::vector<device_s> devices;

    // Parse the devices
    for (auto& device : yaml["devices"]) {
        // Checks if all required fields exists
        if (!device.contains("port") || !device.contains("radio_mode") || !device.contains("channel")) {
            std::cout << "[ERROR] Missing required fields (port, radio_mode, or channel) for a device. Skipping device." << std::endl;
            continue;
        }
        devices.push_back(device_s{
            device["port"].get_value<std::string>(),
            device["radio_mode"].get_value<int>(),
            device["channel"].get_value<int>()
        });
    }

    // Parse the log settings
    auto& yaml_log = yaml["log"];
    // Property                     Optional Field                          Read Value                                          Default Value 
    log->file.enabled =             yaml_log.contains("enabled")            ? yaml_log["enabled"].get_value<bool>()             : true;
    log->file.path =                yaml_log.contains("path")               ? yaml_log["path"].get_value<std::string>()         : "./";
    log->file.base_name =           yaml_log.contains("base_name")          ? yaml_log["base_name"].get_value<std::string>()    : "aceno";
    log->file.split_devices_log =   yaml_log.contains("splitDevicesLog")    ? yaml_log["splitDevicesLog"].get_value<bool>()     : false;
    log->file.reset_period =        yaml_log.contains("resetPeriod")        ? yaml_log["resetPeriod"].get_value<std::string>()  : "none";

    yaml_log = yaml["pipe"];
    // Property                     Optional Field                          Read Value                                          Default Value 
    log->pipe.enabled =             yaml_log.contains("enabled")            ? yaml_log["enabled"].get_value<bool>()             : true;
    log->pipe.path =                yaml_log.contains("path")               ? yaml_log["path"].get_value<std::string>()         : DEFAULT_PIPE_PATH;
    log->pipe.base_name =           yaml_log.contains("base_name")          ? yaml_log["base_name"].get_value<std::string>()    : "aceno";
    log->pipe.split_devices_log =   yaml_log.contains("splitDevicesLog")    ? yaml_log["splitDevicesLog"].get_value<bool>()     : false;
    log->pipe.reset_period = "none";

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
    // If useInput is true, parse input file based on file extension and add devices to device vector and update log settings
    if(useInput) {
        std::string fileExtension = configFilePath.substr(configFilePath.find_last_of(".") + 1);
        if(fileExtension == "yaml")
            devices = parse_input_file_yaml(configFilePath, &log);
        else
            std::cout << "[ERROR] Invalid file extension. Please use a .json or .yaml file." << std::endl;
    }

    // Run sniffer passing devices vector
    Sniffer sniffer(devices, log);
    sniffer.configureAllDevices();
    sniffer.initAllDevices();
    sniffer.streamAll(std::chrono::seconds(20));

    return 0;
}
