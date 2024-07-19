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

/*
This file uses the fkYAML library to parse a YAML file and configure the sniffer.

MIT License

Copyright (c) 2023 Kensuke Fukutani

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <iostream>
#include <getopt.h>
#include <stdlib.h>
#include <cstdlib>
#include <string>
#include <signal.h>
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
    std::cout << "[OPTIONAL]  -b, --base\t\t\t\tBase file/ pipe name" << std::endl;
    std::cout << "[OPTIONAL]  -P, --path\t\t\t\tPath to save file" << std::endl;
    std::cout << "[OPTIONAL]  -r, --reset_period\t\t\tReset period" << std::endl;
    std::cout << "[OPTIONAL]  -s, --log_split\t\t\tSplit log files by device" << std::endl;
    std::cout << "[OPTIONAL]  -t, --time_duration\t\t\tSniffing duration in seconds" << std::endl;
    std::cout << "[OPTIONAL]  -i, --input\t\t\t\tInput config file" << std::endl;
    std::cout << "(See config.yaml for a -i example file)" << std::endl;
}

std::vector<device_s> parse_input_file_yaml(const std::string& filePath, log_s* log, int* duration)
{
    if (filePath.empty() || log == nullptr)
    {
        D(std::cout << "[ERROR] Invalid file path or log pointer." << std::endl;)
        return std::vector<device_s>();
    }

    // Try to open the file
    std::ifstream f(filePath);
    if (!f.is_open()) {
        D(std::cout << "[ERROR] Could not open config file: " << filePath << std::endl;)
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
        D(std::cout << "[ERROR] Could not open or parse YAML file: " << e.what() << std::endl;)
        // Return an empty vector
        return std::vector<device_s>();
    }

    // Check if has field devices
    if (!yaml.contains("devices")) {
        D(std::cout << "[ERROR] Missing devices field in YAML file." << std::endl;)
        return std::vector<device_s>();
    }
    // Create a vector of devices
    std::vector<device_s> devices;

    // Parse the devices
    for (auto& device : yaml["devices"]) {
        // Checks if all required fields exists
        if (!device.contains("port") || !device.contains("radio_mode") || !device.contains("channel")) {
            D(std::cout << "[ERROR] Missing required fields (port, radio_mode, or channel) for a device. Skipping device." << std::endl;)
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

    // Checks if reset period is valid, if not, default to none
    if(log->file.reset_period != "none" && log->file.reset_period != "hourly" && log->file.reset_period != "daily" && log->file.reset_period != "weekly" && log->file.reset_period != "monthly") {
        D(std::cout << "[ERROR] Invalid reset period for file log. Defaulting to none." << std::endl;)
        log->file.reset_period = "none";
    }

    yaml_log = yaml["pipe"];
    // Property                     Optional Field                          Read Value                                          Default Value 
    log->pipe.enabled =             yaml_log.contains("enabled")            ? yaml_log["enabled"].get_value<bool>()             : true;
    log->pipe.path =                yaml_log.contains("path")               ? yaml_log["path"].get_value<std::string>()         : DEFAULT_PIPE_PATH;
    log->pipe.base_name =           yaml_log.contains("base_name")          ? yaml_log["base_name"].get_value<std::string>()    : "aceno";
    log->pipe.split_devices_log =   yaml_log.contains("splitDevicesPipe")   ? yaml_log["splitDevicesPipe"].get_value<bool>()    : false;
    log->pipe.reset_period = "none";

    // Takes the duration from the yaml file
    *duration =                     yaml.contains("duration")               ? yaml["duration"].get_value<int>()                 : -1;
    std::cout << "[INFO] Duration: " << *duration << std::endl;

    // Return the vector of devices
    return devices;
}

int main(int argc, char* argv[])
{
    D(std::cout << "[INFO] Debug mode is on!" << std::endl;)

    const char* const short_opts = "hp:m:c:b:i:r:P:t:s";    
    const option long_opts[] = {
        {"help",         no_argument,       nullptr, 'h'},
        {"port",         required_argument, nullptr, 'p'},
        {"radio_mode",   required_argument, nullptr, 'm'},
        {"channel",      required_argument, nullptr, 'c'},
        {"base",         required_argument, nullptr, 'b'},
        {"input",        required_argument, nullptr, 'i'},
        {"reset_period", no_argument,       nullptr, 'r'},
        {"path",         no_argument,       nullptr, 'P'},
        {"time_duration",required_argument, nullptr, 't'},
        {"log_split",    no_argument,       nullptr, 's'},
        {nullptr,        no_argument,       nullptr,  0 }
    };

    bool useInput = false;
    std::string configFilePath;
    std::vector<device_s> devices;
    device_s device;
    int duration = -1;

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
                D(std::cout << "[CONFIG] Serial port: " << optarg << std::endl;)
                device.port = optarg;
                break;
            case 'm':
                D(std::cout << "[CONFIG] Radio mode: " << optarg << std::endl;)
                device.radio_mode = std::stoi(optarg);
                break;
            case 'c':
                D(std::cout << "[CONFIG] Channel: " << optarg << std::endl;)
                device.channel = std::stoi(optarg);
                break;
            case 'b':
                D(std::cout << "[CONFIG] Base: " << optarg << std::endl;)
                log.file.base_name = optarg;
                log.pipe.base_name = optarg;
                break;
            case 'r':
                D(std::cout << "[CONFIG] Reset period: " << optarg << std::endl;)
                if (std::string(optarg) == "none" || std::string(optarg) == "hourly" || std::string(optarg) == "daily" || std::string(optarg) == "weekly" || std::string(optarg) == "monthly") {
                    log.file.reset_period = optarg;
                } else {
                    std::cout << "[ERROR] Invalid reset period. Please choose from: none, hourly, daily, weekly, or monthly." << std::endl;
                    return 0;
                }
                break;
            case 'P':
                D(std::cout << "[CONFIG] Path " << optarg << std::endl;)
                log.file.path = optarg;
                break;
            case 't':
                D(std::cout << "[CONFIG] Time duration: " << optarg << std::endl;)
                duration = std::stoi(optarg);
                break;
            case 's':
                D(std::cout << "[CONFIG] Split log files by device" << std::endl;)
                log.file.split_devices_log = true;
                log.pipe.split_devices_log = true;
                break;
            case 'i':
                D(std::cout << "[CONFIG] Input config file: " << optarg << ". Ignoring other input commands." << std::endl;)
                configFilePath = optarg;
                useInput = true;
                break;            
        }
    }

    // If useInput is false and there are no port, radio_mode or channel, print help and exit
    if(!useInput && (device.port.empty() || !device.radio_mode || !device.channel)) {
        print_help();
        return 0;
    }

    // If useInput is false, add parameter device to device vector
    if(!useInput) devices.push_back(device); 
    // If useInput is true, parse input file based on file extension and add devices to device vector and update log settings
    if(useInput) {
        std::string fileExtension = configFilePath.substr(configFilePath.find_last_of(".") + 1);
        if(fileExtension == "yaml")
            devices = parse_input_file_yaml(configFilePath, &log, &duration);
        else{
            std::cout << "[ERROR] Invalid file extension. Please use a .yaml file." << std::endl;
            return 0;
        }
    }

    // Run sniffer passing devices vector
    Sniffer sniffer(devices, log);

    sniffer.configureAllDevices();
    sniffer.initAllDevices();
    if(duration != -1) sniffer.streamAll(std::chrono::seconds(duration));
    if(duration == -1) sniffer.streamAll();

    return 0;
}
