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
    #include <windows.h>
    #include <tchar.h>
    #define DEFAULT_PIPE_PATH "\\\\.\\pipe\\"
#endif

void print_version() {
    std::cout << "Tuxniffer version: 1.1.1." << std::endl;
}

void print_help()
{
    std::cout << "Usage: ./tuxniffer -p <port> -m <mode> -c <channel> [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help          \tShow this help message and exit." << std::endl;
    std::cout << "  -l, --list_modes    \tShow radio mode table and exit." << std::endl;
    std::cout << "  -v, --version       \tShow tuxniffer version and exit." << std::endl;
    std::cout << "Device Settings (required)" << std::endl;
    std::cout << "  -p, --port          \tSerial port to connect to." << std::endl;
    std::cout << "  -m, --radio_mode    \tRadio mode to use." << std::endl;
    std::cout << "  -c, --channel       \tChannel to use." << std::endl;
    std::cout << "Log Settings (optional)" << std::endl;
    std::cout << "  -n, --name          \tPipe name / log file name (.pcap)." << std::endl;
    std::cout << "  -P, --path          \tPath to save file." << std::endl;
    std::cout << "  -r, --reset_period  \tLog file reset period (none | hourly | daily | weekly | monthly)." << std::endl;
    std::cout << "Others (optional)" << std::endl;
    std::cout << "  -k, --key_extraction\tTry to decrypt zigbee packets and print keys extracted from transport packets. Save extracted keys in keys.txt." << std::endl;
    std::cout << "  -t, --time_duration \tSniffing duration in seconds. Runs indefinitely when missing." << std::endl;
    std::cout << "  -i, --input         \tInput config file. When present Device Settings flags are no longer required." << std::endl;
    std::cout << "  -y, --yaml_example  \tShow default .yaml config file and exit." << std::endl;
    std::cout << "(See README.md for more information)." << std::endl;
}

void print_radio_mode_table()
{
    std::cout << std::endl << "Supported Radio Modes:" << std::endl;
    std::cout << "|--------------------------------------------------------------------------------------------------|" << std::endl;
    std::cout << "| Mode Name                                                      | Supported Channels | Radio Mode |" << std::endl;
    std::cout << "|----------------------------------------------------------------|--------------------|------------|" << std::endl;
    std::cout << "| IEEE 802.15.4g - GFSK 50 Kbps - 915 MHz                        | 0-128              | 0          |" << std::endl;
    std::cout << "| IEEE 802.15.4g - GFSK 50 Kbps - 868 MHz                        | 0-33               | 1          |" << std::endl;
    std::cout << "| IEEE 802.15.4g - GFSK 50 Kbps - 433 MHz                        | 0-6                | 2          |" << std::endl;
    std::cout << "| IEEE 802.15.4g - Simplelink Long Range 5 Kbps - 915 MHz        | 0-128              | 3          |" << std::endl;
    std::cout << "| IEEE 802.15.4g - Simplelink Long Range 5 Kbps - 868 MHz        | 0-33               | 4          |" << std::endl;
    std::cout << "| IEEE 802.15.4g - Simplelink Long Range 5 Kbps - 433 MHz        | 0-6                | 5          |" << std::endl;
    std::cout << "| IEEE 802.15.4g - GFSK 50 Kbps Wi-SUN PHY #1a (ID 1) - 868 MHz  | 0-128              | 6          |" << std::endl;
    std::cout << "| IEEE 802.15.4g - GFSK 50 Kbps Wi-SUN PHY #1b (ID 2) - 915 MHz  | 0-128              | 7          |" << std::endl;
    std::cout << "| IEEE 802.15.4g - GFSK 100 Kbps Wi-SUN PHY #2a (ID 3) - 868 MHz | 0-128              | 8          |" << std::endl;
    std::cout << "| IEEE 802.15.4g - GFSK 100 Kbps Wi-SUN PHY #2b (ID 4) - 915 MHz | 0-128              | 9          |" << std::endl;
    std::cout << "| IEEE 802.15.4g - GFSK 150 Kbps Wi-SUN PHY #3 (ID 5) - 868 MHz  | 0-128              | 10         |" << std::endl;
    std::cout << "| IEEE 802.15.4g - GFSK 200 Kbps Wi-SUN PHY #4a (ID 6) - 915 MHz | 0-128              | 11         |" << std::endl;
    std::cout << "| IEEE 802.15.4g - GFSK 200 Kbps Wi-SUN PHY #4b (ID 7) - 915 MHz | 0-128              | 12         |" << std::endl;
    std::cout << "| IEEE 802.15.4g - GFSK 100 Kbps ZigBee R23 - 868 MHz            | 0-128              | 13         |" << std::endl;
    std::cout << "| IEEE 802.15.4g - GFSK 500 Kbps ZigBee R23 - 868 MHz            | 0-128              | 14         |" << std::endl;
    std::cout << "| IEEE 802.15.4g - GFSK 200 Kbps - 915 MHz                       | 0-63               | 15         |" << std::endl;
    std::cout << "| Generic - GFSK 50 Kbps - 868 MHz                               | 0                  | 16         |" << std::endl;
    std::cout << "| Generic - GFSK 50 Kbps - 433 MHz                               | 0                  | 17         |" << std::endl;
    std::cout << "| Generic - Simplelink Long Range 5 Kbps - 868 MHz               | 0                  | 18         |" << std::endl;
    std::cout << "| Generic - Simplelink Long Range 5 Kbps - 433 MHz               | 0                  | 19         |" << std::endl;
    std::cout << "| IEEE 802.15.4 - O-QPSK - 2405 MHz                              | 11-26              | 20         |" << std::endl;
    std::cout << "| BLE - BLE 1 Mbps - 2402 MHz                                    | 37,38,39           | 21         |" << std::endl;
    std::cout << "|--------------------------------------------------------------------------------------------------|" << std::endl;
}

void print_default_config_file() {
    std::cout << "## List of devices that you want to sniff. At least one device is required.\n"
              << "devices:                #required\n"
              << "  - port: /dev/ttyACM0  #required\n"
              << "    radio_mode: 20      #required\n"
              << "    channel: 20         #required\n"
              << "# - port: /dev/ttyACM2\n"
              << "#   radio_mode: 20\n"
              << "#   channel: 25\n"
              << "\n"
              << "## Optional log parameters. Values below are the default ones.\n"
              << "# log:\n"
              << "#   enabled: false            # Set true to create a .pcap log file.\n"
              << "#   path: ./                  # Path to save log file.\n"
              << "#   base_name: aceno          # Log file name.\n"
              << "#   splitDevicesLog: false    # Set true to create a separate log file for each device ([name]_[device_id].pcap).\n"
              << "#   resetPeriod: none         # Log file reset period  (none | hourly | daily | weekly | monthly).\n"
              << "\n"
              << "## Optional pipe parameters. Values below are the default ones.\n"
              << "# pipe:\n"
              << "#   enabled: true             # Set false to not create a pipe.\n"
              << "#   name: aceno               # Pipe file name.\n"
              << "#   path: /tmp/               # Path to save pipe file. On Windows the path name is ignored because the only path is \\\\.\\pipe\\.\n"
              << "#   splitDevicesPipe: false   # Set true to create a separate log file for each device ([name]_[device_id]).\n"
              << "\n"
              << "## Optional crypto parameters. Values below are the default ones.\n"
              << "# crypto:\n"
              << "#   key_extraction: false                     # Set true to try to decrypt packets and extract keys from them.\n"
              << "#   save_keys: false                          # Set true to save extracted keys on a .txt file.\n"
              << "#   keys_path: keys                           # Path to file where keys will be saved if save_keys is true.\n"
              << "#   save_packets: false                       # Set true to save decrypted transport key packets on a bin file.\n"
              << "#   packets_path: transport_key_packets       # Path to file where transport key packets will be saved if save_packets is true.\n"
              << "#                                             # If equal to simulation_path the packets will be written on append mode.\n"
              << "#   simulation: false                         # Set true to read transport key packets from a bin file and add them on the log\n"
              << "#                                             # and pipe. (It allows Wireshark to decrypt packets automatically).\n"
              << "#   simulation_path: transport_key_packets    # Path to bin file where transport key packets will be loaded and simulated if\n"
              << "#                                             # simulation is true.\n"
              << "#   security_level: -1                        # Zigbee network level of security for decryption. Accepted values between 4 and 7.\n"
              << "#                                             # Levels 0 to 3 do not have encryption and 4 is not supported because the lack\n"
              << "#                                             # of authentication. If not informed levels 4 to 7 will be tried until a match.\n"
              << "\n"
              << "## Optional time in seconds to execute the sniffer. When -1 runs indefinitely (default).\n"
              << "# duration: -1\n"
              << "\n"
              << "## You can add more devices to the list, but for each one, you need to set valid\n"
              << "## values for all three required parameters (port, radio mode, and channel).\n"
              << "## For log and pipe options, you can only set the parameters you want to change.\n";
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
    log->file.enabled =             yaml_log.contains("enabled")            ? yaml_log["enabled"].get_value<bool>()             : false;
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

    yaml_log = yaml["crypto"];
    // Property                     Optional Field                              Read Value                                                  Default Value 
    log->crypto.key_extraction =    yaml_log.contains("key_extraction")         ? yaml_log["key_extraction"].get_value<bool>()              : false;
    log->crypto.security_level =    yaml_log.contains("security_level")         ? yaml_log["security_level"].get_value<int>()               : -1;
    log->crypto.save_keys =         yaml_log.contains("save_keys")              ? yaml_log["save_keys"].get_value<bool>()                   : false;
    log->crypto.keys_path =         yaml_log.contains("keys_path")              ? yaml_log["keys_path"].get_value<std::string>()            : "keys";
    log->crypto.save_packets =      yaml_log.contains("save_packets")           ? yaml_log["save_packets"].get_value<bool>()                : false;
    log->crypto.packets_path =      yaml_log.contains("packets_path")           ? yaml_log["packets_path"].get_value<std::string>()         : "transport_key_packets";
    log->crypto.simulation =        yaml_log.contains("simulation")             ? yaml_log["simulation"].get_value<bool>()                  : false;
    log->crypto.simulation_path =   yaml_log.contains("simulation_path")        ? yaml_log["simulation_path"].get_value<std::string>()      : "transport_key_packets";

    if (log->crypto.security_level > 7 || (log->crypto.security_level < 5 && log->crypto.security_level != -1))
    {
        log->crypto.security_level = -1;
    }

    // Takes the duration from the yaml file
    *duration =                     yaml.contains("duration")                   ? yaml["duration"].get_value<int>()                         : -1;
    std::cout << "[INFO] Duration: " << *duration;
    if (*duration == -1)
    {
        std::cout << " (runs indefinitely)." << std::endl;
    }
    else
    {
        std::cout << " seconds." << std::endl;
    }
    

    // Return the vector of devices
    return devices;
}

std::vector<std::string> parse_arguments(int argc, char* argv[]) {
    std::vector<std::string> args;
    for (int i = 0; i < argc; ++i) {
        args.push_back(std::string(argv[i]));
    }
    return args;
}


int main(int argc, char* argv[])
{
    D(std::cout << "[INFO] Debug mode is on!" << std::endl;)

    std::vector<std::string> args = parse_arguments(argc, argv);

    bool useInput = false;
    std::string configFilePath;
    std::vector<device_s> devices;
    device_s device;
    int duration = -1;

    // If theres no input file to config the log, use default values
    log_s log = {
        {false, "./", "aceno", false, "none"},
        {true, DEFAULT_PIPE_PATH, "aceno", false, "none"},
        {false, -1, false, "keys", false, "", false, ""}
    };

    for (size_t i = 1; i < args.size(); ++i) {
        const std::string& arg = args[i];
        if (arg == "-h" || arg == "--help") {
            print_help();
            return 0;
        }
        else if (arg == "-l" || arg == "--list_radio") {
            print_radio_mode_table();
            return 0;
        }
        else if (arg == "-v" || arg == "--version") {
            print_version();
            return 0;
        }
        else if (arg == "-y" || arg == "--yaml_example") {
            print_default_config_file();
            return 0;
        }
        else if (arg == "-p" || arg == "--port") {
            ++i;
            D(std::cout << "[CONFIG] Serial port: " << args[i] << std::endl;)
            device.port = args[i];
        }
        else if (arg == "-m" || arg == "--radio_mode") {
            ++i;
            D(std::cout << "[CONFIG] Radio mode: " << args[i] << std::endl;)
            device.radio_mode = std::stoi(args[i]);
        }
        else if (arg == "-c" || arg == "--channel") {
            ++i;
            D(std::cout << "[CONFIG] Channel: " << args[i] << std::endl;)
            device.channel = std::stoi(args[i]);
        }
        else if (arg == "-n" || arg == "--name") {
            ++i;
            D(std::cout << "[CONFIG] Name: " << args[i] << std::endl;)
            log.file.base_name = args[i];
            log.pipe.base_name = args[i];
        }
        else if (arg == "-r" || arg == "--reset_period") {
            ++i;
            D(std::cout << "[CONFIG] Reset period: " << args[i] << std::endl;)
            std::string resetPeriod = args[i];
            if (resetPeriod == "none" || resetPeriod == "hourly" || resetPeriod == "daily" || resetPeriod == "weekly" || resetPeriod == "monthly") {
                log.file.reset_period = resetPeriod;
            }
            else {
                std::cout << "[ERROR] Invalid reset period. Please choose from: none, hourly, daily, weekly, or monthly." << std::endl;
                return 0;
            }
        }
        else if (arg == "-P" || arg == "--path") {
            ++i;
            D(std::cout << "[CONFIG] Path " << args[i] << std::endl;)
            log.file.path = args[i];
        }
        else if (arg == "-t" || arg == "--time_duration") {
            ++i;
            D(std::cout << "[CONFIG] Time duration: " << args[i] << std::endl;)
            duration = std::stoi(args[i]);
        }
        else if (arg == "-k" || arg == "--key_extraction") {
            D(std::cout << "[CONFIG] Key extraction enabled" << std::endl;)
            log.crypto.key_extraction = true;
            log.crypto.save_keys = true;
        }
        else if (arg == "-i" || arg == "--input") {
            ++i;
            D(std::cout << "[CONFIG] Input config file: " << args[i] << ". Ignoring other input commands." << std::endl;)
            configFilePath = args[i];
            useInput = true;
        }
    }

    // If useInput is false and there are no port, radio_mode or channel, print help and exit
    if (!useInput && (device.port.empty() || !device.radio_mode || !device.channel)) {
        print_version();
        cout << std::endl;
        print_help();
        return 0;
    }

    // If useInput is false, add parameter device to device vector
    if (!useInput) devices.push_back(device);
    // If useInput is true, parse input file based on file extension and add devices to device vector and update log settings
    if (useInput) {
        std::string fileExtension = configFilePath.substr(configFilePath.find_last_of(".") + 1);
        if (fileExtension.compare(("yaml")) == 0)
            devices = parse_input_file_yaml(configFilePath, &log, &duration);
        else {
            std::cout << "[ERROR] Invalid file extension. Please use a .yaml file." << std::endl;
            return 0;
        }
    }

    // Run sniffer passing devices vector
    Sniffer sniffer(devices, log);

    sniffer.configureAllDevices();
    sniffer.initAllDevices();
    if (duration != -1) sniffer.streamAll(std::chrono::seconds(duration));
    if (duration == -1) sniffer.streamAll();

    return 0;
}