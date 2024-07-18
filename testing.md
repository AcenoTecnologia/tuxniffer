# Tests

This file is a list of tests that should be made to ensure that the sniffer is working as expected. Tests are separated in:

- Basic usage: basic usage of the software and its funcionality;
- Intermediate usage: usage of all software funcionality, but wihout stress;
- Advanced usage: usage of all software funcionality with stress;

Tests should be performed accordingly with the situation and the changes made; 

## Notes

### WSL Tests:
- For WSL tests it is necessary to forward a port from Windows to Linux:

On powershell as administrator:
  ```sh
  usbipd list # To show currently connected USB devices;
  usbipd bind --busid 3-2 # Binds the port to be shared. 3-2 is the number of the port to be forwarded resulted from the first command;
  usbipd attach --wsl --busid 3-2 # Attach the port to WSL
  # Every time the device is reconnected the last command must be executed
  ```

  After that the port should be avaiable on WSL. To see the list of ports use:

  ```sh
  ls /dev/ # To see all IO devices.
  # Pay attention to the tty* and ttyACM*. The forwarded port should be one of them.
  ```

Each serial port forwarded is actually 2 ttys, thus, one device should map to ttyACM0 and ttyACM1.

### Windows tests
This software currently does not support Windows.

### Test Preparation
- Check if no pipe is already created (if there is `ls /tmp/` should show it. Use `rm <pipe> to delete it`);
- Check if port is correctly connected and forwarded (in WSL case);

## Basic Usage

Simple, easy commands that one can use without going too deep. Most of users fall in this category.

- Radio Mode: 20 - IEEE 802.15.4 2.4GHz; Channel: 25;

### Case 1:
- Most basic usage case. No optional parameters, only the bare minimum;

  ```sh
  ./snifferCPP -p /dev/ttyACM0 -m 20 -c 25
  ```

    - Check if program runs without error;
    - Check if .pcap file is generated without error;
    - Check if pipe is generated without error;
    - Check if the program closes correctly;

### Case 2:
- Basic usage, but with optional parameters;

  ```sh
  ./snifferCPP -p /dev/ttyACM0 -m 20 -c 25 -b aceno -P ./results/
  ```

    - Check if program runs without error;
    - Check if files are generated in the correct folder;
    - Check if it show errors if folder does not exist;
    - Check if .pcap file is generated without error;
    - Check if pipe is generated without error;
    - Check if the program closes correctly;

## Intermediate Usage

Users that want to explore and take advantage of the software capabilities, but will not stress it to its limits.

- Radio Mode: 20 - IEEE 802.15.4 2.4GHz; Channel: 25;
- For YAML cases the execution command should be `./snifferCPP -i config.yaml`;


### (CLI) Case 1:
- Basic case with parameters, but split devices log
  ```sh
  ./snifferCPP -p /dev/ttyACM0 -m 20 -c 25 -b aceno -P ./results/ -s
  ```

    - Basic usage checks;
    - Check if logs (files and pipes) are generated with correct device id;
    - Check if shows errors if device could not connect;

### (CLI) Case 2:
- Basic case with parameters, but split devices log and reset period enabled
  ```sh
  ./snifferCPP -p /dev/ttyACM0 -m 20 -c 25 -b aceno -P ./results/ -s -r hourly
  ```

    - Basic usage checks;
    - Check if logs (files and pipes) are generated with correct device id;
    - Check if reset period works
    - Check if shows errors if device could not connect;

### (YAML) Case 3:
- Most basic usage case. No optional parameters, only the bare minimum, but in YAML;
  ```yaml
    devices:
    - port: /dev/ttyACM0
        radio_mode: 20
        channel: 25
  ```

    - Basic usage checks;

### (YAML) Case 4:
- Basic usage, but with optional parameters in YAML;
  ```yaml
    devices:
      - port: /dev/ttyACM0
        radio_mode: 20
        channel: 25
    log:
        path: ./results/
        base_name: aceno
  ```

    - Basic usage checks;

### (YAML) Case 5:
- Basic case with parameters, but split devices log in YAML;
  ```yaml
    devices:
      - port: /dev/ttyACM0
        radio_mode: 20
        channel: 25
    log:
        path: ./results/
        base_name: 
        splitDevicesLog: true
  ```

    - Intermediate usage (case 1) checks;

### (YAML) Case 6:
- Basic case with parameters, but split devices log and reset period enabled in YAML;

  ```yaml
    devices:
      - port: /dev/ttyACM0
        radio_mode: 20
        channel: 25
    log:
        path: ./results/
        base_name: 
        splitDevicesLog: true
        resetPeriod: hourly
  ```

    - Intermediate usage (case 2) checks;

## Advanced Usage

Users that will use all of the software capabilities and will stress it to its limits.

### (YAML) Case 1:
- Test with 3 devices, all of them set to BLE (normally High packet flow), with optional parameters, reset period set do daily and a pipe open; 

  ```yaml
    devices:
      - port: /dev/ttyACM0
        radio_mode: 21
        channel: 37
      - port: /dev/ttyACM2
        radio_mode: 21
        channel: 38
      - port: /dev/ttyACM4
        radio_mode: 21
        channel: 39
    log:
        path: ./results/
        base_name: output
        splitDevicesLog: true
        resetPeriod: daily
    pipe:
        path: /tmp/
        base_name: output
        splitDevicesLog: false
  ```

  - Basic usage tests;
  - Intermediate usage tests;
  - Check if system has enough RAM to run at least during one day;
  - Check if packets are written to file faster then they are captured;
  - Check if queue overflow;
  - Open pipe and check how much RAM wireshark will consume over time;
  - Check the size of the .pcap files generated during one day;


## Additional Test Cases for Unexpected Events


1. **Test what happens if a device disconnects during use:**
   - Simulate a device (like `/dev/ttyACM0`) disconnecting during data transmission;
   - Check if the system logs the disconnection and attempts reconnection;
   - Verify that the data integrity is maintained for the already captured data;
   - Check if the software closes correctly.

2. **Test what happens if a device fails to connect:**
   - Attempt to start the system with a device that fails to connect;
   - Ensure that appropriate error messages are logged;
   - Verify that the system handles the failure without crashing;
   - Check if the software closes correctly;
   - Check what happens to other devices that connected correctly;

3. **Test what happens if no devices can connect:**
   - Simulate a scenario where none of the configured devices are available for connection;
   - Ensure that the system logs the failure;
   - Verify that the system does not crash;

4. **Test if a .pcap file can be deleted during use:**
   - Start the system and begin logging to a .pcap file;
   - Manually try to delete the .pcap file during the logging process;
   - Verify that the system detects the deletion and logs an appropriate error;
   - Check if the system creates a new .pcap file or stops logging;
   - Verify that the system does not crash;

5. **Test what happens if the device consuming the pipe is closed:**
   - Start the system with a pipe enabled;
   - Start Wireshark to consume the pipe data;
   - Close Wireshark during data transmission;
   - Ensure the system logs the event and handles the pipe closure;
   - Verify that data integrity is maintained up to the point of closure;
   - Check if other Wireshark can connect to the same pipe and continue getting packets that were on the queue waiting to be stored;
   - Verify that the system does not crash;
   - Verify that the system does not interrupt the other pipes or log process;

6. **Test what happens if the pipe is never consumed:**
   - Start the system with a pipe enabled but without any process consuming it.
   - Check how the system handles the accumulation of data. Is there a chance of overflow?
   - Verify that the system logs warnings about the unconsumed pipe and does not crash.

7. **Test what happens if the pipe is consumed over SSH:**
   - Set up a remote process over SSH to consume the pipe data.
   - Verify that the data transmission works correctly over SSH.
   - Check for any latency or data loss issues.

8. **Test what happens if the pipe is consumed over SSH via WSL:**
   - Set up a WSL environment on a Windows machine and use SSH within WSL to consume the pipe.
   - Verify the data integrity and transmission over this setup.
   - Check for compatibility issues or data loss.

9. **Test what happens if the sniffer process is terminated:**
   - Start the sniffer process and then terminate it abruptly (closing the terminal window, for example);
   - Verify the integrity of the .pcap files;

10. **Test what happens if the sniffer process is interrupted:**
    - Start the sniffer process and interrupt it (like via a keyboard interrupt with Ctrl + C);
    - Ensure that the system handles the interruption;
    - Verify that the .pcap files and pipes are not corrupted;


## Menu and Parameters

Tests to ensure that the menu and parameters are running correctly.

### CLI Tests


- Correct usage test:
  ```sh
  ./snifferCPP -h # shows help message and exit
  ./snifferCPP -p /dev/ttyACM0 -m 20 -c 25 -P ./results/ -b aceno -r hourly # run correctly
  ```

- Test without essential parameters:
  ```sh
  ./snifferCPP # missing essentials
  ./snifferCPP -b test # missing essentials but has optional
  ```

- Test with an incorrect essential parameter:
  ```sh
  ./snifferCPP -p invalid_port # incorrect essential and missing
  ./snifferCPP -p invalid_port -m 20 -c 25 # incorrect essential
  ```

- Test with an empty essential parameter:
  ```sh
  ./snifferCPP -p -c 20 -m 10 # empty essential
  ./snifferCPP -p # empty essential and missing
  ./snifferCPP -h -p # empty essential and help
  ```

  - Test with path ending without a "/" character:
  ```sh
  ./snifferCPP -p /dev/ttyACM0 -m 20 -c 25 -P ./results -b aceno -r hourly
  ```
  Should raise error or fix?

- Test with an essential parameter of the correct type but nonsensical value:
  ```sh
  ./snifferCPP -p /dev/ttyACM0 -m 20 -c 99 # 99 is not a channel

  ./snifferCPP -p /dev/ttyACM99 -m 20 -c 20 # /dev/ttyACM99 is not a realistic value

  ./snifferCPP -p /dev/ttyACM0 -m -1 -c 20 # -1 is not a radio_mode
  ```

- Test with more than one essential parameter:
  ```sh
  ./snifferCPP -p /dev/ttyACM0 -p /dev/ttyACM1 -m 20 -c 25 # 2 ports defined
  ./snifferCPP -p /dev/ttyACM0 -p INVALID -m 20 -c 25 # 2 ports defined and 1 is invalid
  ./snifferCPP -p INVALID -p /dev/ttyACM0 -m 20 -c 25 # 2 ports defined and 1 is invalid. The invalid comes first
  ```

- Test with only optional commands:
  ```sh
  ./snifferCPP -b logfile_base # missing essentials, only optionals
  ```

- Test with incorrect optional commands:
  ```sh
  ./snifferCPP -p /dev/ttyACM0 -m 20 -c 25 -b -r none # no -b parameter
  ./snifferCPP -p /dev/ttyACM0 -m 20 -c 25 -b test -r hourly -r daily # 2 -r parameters
  ./snifferCPP -p /dev/ttyACM0 -m 20 -c 25 -b ////test//// -r noTime # invalid -b parameter. OS could interpret as folder
  ./snifferCPP -p /dev/ttyACM0 -m 20 -c 25 -b 0101test///%/01 -r none # invalid -b parameter. With special characters
  ./snifferCPP -p /dev/ttyACM0 -m 20 -c 25 -b test.pdf -r test # should be a valid -b parameter despite ending with .pdf
  ```

- Test with an invalid path:
  ```sh
  ./snifferCPP -p COM0 -m 20 -c 25 -P /incorrect/path/ -r weekly # incorrect absolute path
  ```

- Test with a valid serial that does not work because it is another device:
  ```sh
  ./snifferCPP -p /dev/ttyUSB1 -m radio_mode -c channel_number # ttyUSB1 is not a sniffer device
  ```

## YAML Tests

- Correct usage test with only essential parameters
```yaml
devices:
  - port: /dev/ttyACM0          # (required)
    radio_mode: 20              # (required)
    channel: 25                 # (required)
```

- Correct usage test with all parameters
```yaml
devices:
  - port: /dev/ttyACM0          # (required) /dev/ttyACM0 (linux) or COM3 (windows)
    radio_mode: 20              # (required)
    channel: 25                 # (required)
log:
  enabled: true                 # (optional - default) true | false
  path: ./                      # (optional - default) ./
  base_name: aceno              # (optional - default) aceno
  splitDevicesLog: false         # true | (optional - default) false
  resetPeriod: hourly           # (optional - default) none | hourly | daily | weekly | monthly
pipe:
  enabled: true                 # (optional - default) true | false
  name: test                    # (optional - default) aceno
  path: /tmp/                   # (optional - default) /tmp/ (linux) or \\.\pipe\ (windows)
                                # Pipes can be created wherever the user has write permissions on linux
                                # However, on Windows the pipe must be created at \\.\pipe\
  splitDevicesPipe: false       # true | (optional - default) false
                                # Pipes are created as <path>/<name>[-<id>] on Linux or \\.\pipe\<name>[-<id>] on Windows
                                # Pipes does not support the resetPeriod option
```

- Test without essential parameters
```yaml
devices:
  - port: /dev/ttyACM0          # (required)
    radio_mode: 20              # (required)
    # channel is missing
```

- Test with incorrect essential parameter
```yaml
devices:
  - port: /dev/ttyACM0          # (required)
    radio_mode: incorrect_mode  # incorrect value
    channel: 25                 # (required)
```


- Test with empty essential parameter
```yaml
devices:
  - port: /dev/ttyACM0          # (required)
    radio_mode:                 # empty value
    channel: 25                 # (required)
```


- Test with essential parameter of the correct type but nonsensical value
```yaml
devices:
  - port: /dev/ttyACM0          # (required)
    radio_mode: 9999            # nonsensical value
    channel: 25                 # (required)
```


- Test with more than one essential parameter
```yaml
devices:
  - port: /dev/ttyACM0          # (required)
    radio_mode: 20              # (required)
    channel: 25                 # (required)
    port: value                 # extra port parameter
```


- Test with only optional commands
```yaml
# Should return an error because is missing required parameters
log:
  enabled: true
  path: ./
  base_name: aceno
pipe:
  enabled: true
  name: test
  path: /tmp/
```


- Test with incorrect optional commands and wrong value types
```yaml
log:
  enabled: yes                # should be true/false
  path: ./
  base_name: aceno
pipe:
  enabled: true
  name: test
  path: /tmp/
```

- Test with invalid path
```yaml
log:
  enabled: true
  path: /invalid/path/         # invalid path
  base_name: aceno
pipe:
  enabled: true
  name: test
  path: /tmp/
```


- Test with valid serial but not working because it's another device
```yaml
devices:
  - port: /dev/ttyUSB0          # valid serial but different device (like a mouse or keyboard)
    radio_mode: 20              # (required)
    channel: 25                 # (required)
```


- Test with logging enabled and disabled for both pcap and pipe
```yaml
devices:
  - port: /dev/ttyACM0          # (required)
    radio_mode: 20              # (required)
    channel: 25                 # (required)
log:
  enabled: true
  path: ./
  base_name: aceno
pipe:
  enabled: true
  name: test
  path: /tmp/
```

```yaml
devices:
  - port: /dev/ttyACM0          # (required)
    radio_mode: 20              # (required)
    channel: 25                 # (required)
log:
  enabled: true
  path: ./
  base_name: aceno
pipe:
  enabled: false
  name: test
  path: /tmp/
```

```yaml
devices:
  - port: /dev/ttyACM0          # (required)
    radio_mode: 20              # (required)
    channel: 25                 # (required)
log:
  enabled: false
  path: ./
  base_name: aceno
pipe:
  enabled: true
  name: test
  path: /tmp/
```

```yaml
devices:
  - port: /dev/ttyACM0          # (required)
    radio_mode: 20              # (required)
    channel: 25                 # (required)
log:
  enabled: false
  path: ./
  base_name: aceno
pipe:
  enabled: false
  name: test
  path: /tmp/
```