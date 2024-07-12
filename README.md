# SnifferCPP

## Overview

**This software aims to replicate the capabilities of the *SmartRF Packet Sniffer 2* but through a terminal on Linux.**

The SmartRF Packet Sniffer 2 from Texas Instruments is a useful tool for sniffing packet data using their devices and streaming it to a .pcap file through a pipe. The communication between the device and the computer is done using a serial port. However, the software is only available for Windows.

## Features

This software is capable of:
- Sniffing network packets sent by Texas Instruments sniffers.
- Supporting multiple devices.
- Storing packets in a .pcap file that can be opened using Wireshark.
- Viewing packets live in Wireshark through pipes.
- Configuring Radio Mode for different packet types.
- Configuring the time interval between file generations.
- Splitting device data into different files or pipes.
- Support for .YAML config file and CLI usage.

## Build Instructions

### Folder Creation

Create a directory named `build`:

```bash
mkdir build
cd build
```

### Run CMake

From within the build directory, run:

```bash
cmake ..
```

### Build the Project

Now, you can build your project using:

```bash
cmake --build .
```

### Specify Debug Mode

By default, CMake builds in Release mode. To build in Debug mode, specify the build type when running cmake:

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

Then build the project as usual:

```bash
cmake --build .
```

The binary file is now stored in `build/bin`.

## Usage

### CLI

Usage: `.build/bin/sniffer [options]`

#### Options:

- `-h, --help`: Show help message and exit.
- **[REQUIRED]** `-p, --port`: Serial port to connect to.
- **[REQUIRED]** `-m, --radio_mode`: Radio mode to use.
- **[REQUIRED]** `-c, --channel`: Channel to use.
- **[OPTIONAL]** `-b, --base`: Base file/pipe name.
- **[OPTIONAL]** `-l, --log_hourly`: Log hourly.
- **[OPTIONAL]** `-L, --log_daily`: Log daily.
- **[OPTIONAL]** `-w, --log_weekly`: Log weekly.
- **[OPTIONAL]** `-M, --log_monthly`: Log monthly.
- **[OPTIONAL]** `-s, --log_split`: Split log files by device.
- **[OPTIONAL]** `-i, --input`: Input config file.

#### Usage Example:

```bash
# Display help message
./snifferCPP --help

# Connect to a serial port, set radio mode, and channel
./snifferCPP -p /dev/ttyUSB0 -m 20 -c 25

# Specify base name and log hourly
./snifferCPP -p /dev/ttyUSB0 -m 20 -c 25 -b sniffer -l
```

### .YAML Config File

Using the parameter `-i [config_file_dir/config.yaml]`, you can use a separate config file for defining multiple devices and different log options. An example of a config file explaining each parameter, the supported, and default values can be seen at `./config.yaml`.

**Note: when the parameter `-i` is used, other CLI parameters are ignored.**

#### Usage Example:

```bash
# Display help message
./snifferCPP --help

# Use input config file (config.yaml) to configure devices and logging
./snifferCPP -i config.yaml
```

## Pipes

This software allows live packet viewing using Pipes. The live viewing can be used along with the .pcap log files without any conflict. To run the software along with Wireshark, some possibilities are:

```bash
# Display help message
./snifferCPP --help

# Use input config file (config.yaml) to configure devices and open Wireshark
./snifferCPP -i config.yaml | wireshark -k -i /tmp/aceno

# Connect to a serial port, set radio mode and channel, and open Wireshark
./snifferCPP -p /dev/ttyUSB0 -m 20 -c 25 | wireshark -k -i /tmp/aceno

# Specify custom name pipe and open Wireshark
./snifferCPP -p /dev/ttyUSB0 -m 20 -c 25 -b test | wireshark -k -i /tmp/teste
```

Wireshark doesn't need to be open at the start of the sniffer execution. The sniffer will store a queue of packets during its running time, and as long as Wireshark is open while the sniffer is running, the packets will be sent in order to it.

## Compatibility

This software was compiled using gcc version 11.4.0 (Ubuntu 11.4.0-1ubuntu1~22.04) (WSL).


## Texas Instruments Documentation Issues

During development it was found some inconsistencies between the sniffer documentation presented by Texas Instruments and the device functioning (tested on the CC1352P7):

- The baudrate presented in the documentation is `921600`, but the firmware source code uses `3000000`.
- Despite showing a state machine with a `PAUSED` state in the documentation, the firmware source code doesn't have one. Therefore, neither `pause` and `resume` commands exists.
- The PHY code informed by the documentation for `IEEE 802.15.4 2.4 GHz band O-QPSK` is `0x11`, but in reality it is `0x12`.
- This happens because the `Smart RF Sniffer Agent software` by TI has a Radio Configuration for `IEEE 802.15.4 915 MHz GSFK 200 kbps` after `0x0C`, which causes a offset of `0x01` to all subsequent values. This configuration is not on the reference, but can be selected on the software. The Radio Mode table bellow fixes that.
- The packet response documentation also informs that the response frame data payload has the format: `Timestamp (6B) | Payload (0-2047B) | RSSI (1B) | Status (1B)`. But, in reality is `Timestamp (6B) | Separator (1B) | Payload (0-2047B) | RSSI (1B) | Status (1B)`. It was not found the usage of the Separator. However, neither considering it as Timestamp or Payload work. The Timestamp gets incorrect and the Payload doesn't match the FCS at the end of the frame (last 2B of payload).

## Radio Mode

This program currently supports the following Radio Modes for different kinds of packets:

| Mode Name                | PHY ID | Frequency | Radio Mode |
|--------------------------|--------|-----------|------------|
| LP-CC1352P7              |        |           |            |
| ieee_868_915             | 0x00   | f868      | 0          |
| ieee_868_915             | 0x00   | f915      | 1          |
| ieee_433                 | 0x01   | f433      | 2          |
| ieee_868_915_slr         | 0x02   | f868      | 3          |
| ieee_868_915_slr         | 0x02   | f915      | 4          |
| ieee_433_slr             | 0x03   | f433      | 5          |
| wiSun_868_915_50a        | 0x04   | f868      | 6          |
| wiSun_868_915_50b        | 0x05   | f915      | 7          |
| wiSun_868_915_100a       | 0x06   | f868      | 8          |
| wiSun_868_915_100b       | 0x07   | f915      | 9          |
| wiSun_868_915_150        | 0x08   | f868      | 10         |
| wiSun_868_915_200a       | 0x09   | f915      | 11         |
| wiSun_868_915_200b       | 0x0A   | f915      | 12         |
| zigbee_868_915_100       | 0x0B   | f868      | 13         |
| zigbee_868_915_500       | 0x0C   | f868      | 14         |
| ieee_915                 | 0x0D   | f915      | 15         |
| easyLink_868_915_50      | 0x0E   | f868      | 16         |
| easyLink_433_50          | 0x0F   | f433      | 17         |
| easyLink_868_915_slr     | 0x10   | f868      | 18         |
| easyLink_433_slr         | 0x11   | f433      | 19         |
| ieee_2405                | 0x12   | f2405     | 20         |
| ble_2405                 | 0x13   | f2405     | 21         |

## Dependencies
[fkYAML - Header Only Library for parsing YAML file format](https://github.com/fktn-k/fkYAML) (MIT License).
