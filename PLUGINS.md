# Compiling SmartRF Packet Sniffer 2 Wireshark plugins on Linux

[Texas Instruments](https://www.ti.com) provides, with the [SmartRF Packet Sniffer 2](https://www.ti.com/tool/PACKET-SNIFFER), Wireshark plugins to interpret the packets *(see how to configure Wireshark on Windows [here](https://dev.ti.com/tirex4-desktop/explore/node?node=A__ACyc5n.8sICFMAbB44vblg__com.ti.SIMPLELINK_CC13XX_CC26XX_SDK__BSEc4rl__LATEST&placeholder=true))*. However, despite these plugins being semi-compatible with Linux, they only make it available for Windows. **This document contains instructions for using those plugins on Linux.**  

The information gathered in this file was partially presented on `.../wireshark/doc/README.plugins`.

## Prerequisites
This guide assumes that you have Wireshark 4.0 installed. You can access the [Wireshark repository](https://github.com/wireshark/wireshark) to see instalation instructions. You can download id from a package manager or build it from source. Make sure that the version you using is the same version that you use to compile the plugins.


## Wireshark Plugins

The plugins developed by [Texas Instruments](https://www.ti.com) are available at `C:\Program Files (x86)\Texas Instruments\SmartRF Tools\SmartRF Packet Sniffer 2\wireshark\plugins` on Windows. This folder contains the .dll files and the source code of the plugins as a zip file. You need to extract and copy the source code from here to your Linux system.

The list of plugins that need to be recompiled on Linux is:
- ``tirpi`` - Responsible for dissecting the TI Radio Packet Info layer. Forwards the packet data to the correct plugin afterwards.
- ``ti-ble-packet-info`` - Responsible for dissecting the BLE Radio Packet Info layer.
- ``ti802154ge`` - Responsible for dissecting IEEE 802.15.4 packets.

It is necessary to have all Wireshark source code and all its dependencies for this proccess to work (see [Wireshark repository](https://github.com/wireshark/wireshark)).

## Steps to Recompile

### 1 - Clone Wireshark 4.0
```sh
git clone -b release-4.0 https://github.com/wireshark/wireshark
```

### 2 - Copy source files to your system 

You have to copy the source file from a Windows machine.

```
ti802154ge_wireshark_dissector_1.0.12_source
tirpi
ti-ble
```

### 3 - Copy plugins source code to Wireshark plugins folder
```sh
cd wireshark/plugins/epan # Enters the plugins/epan folder

# Create a folder and copy the source code for each plugin
mkdir ti802154ge
cp -r .../path/to/ti802154ge_wireshark_dissector_1.0.12_source/epan/* ./ti802154ge

mkdir ti_rpi
cp -r .../path/to/ti-rpi-wireshark-dissector-plugin-1.7.0-source/* ./ti_rpi

mkdir ti_ble_packet_info
cp -r .../path/to/ti-ble-wireshark-dissector-plugin-1.5.0-source/* ./ti_ble_packet_info
```

### 4 - Update Wireshark files
This steps updates the current Wireshark files to indicate the plugins during build and compilation steps.

#### Copy of CMakeListsCustom.txt.example

Copy the top-level file `wireshark/CMakeListsCustom.txt.example` to `wireshark/CMakeListsCustom.txt` (also in the top-level source directory).

```sh
	cd wireshark
	cp CMakeListsCustom.txt.example CMakeListsCustom.txt
```

#### CMakeListsCustom.txt file updated:

Edit `CMakeListsCustom.txt` so that `CUSTOM_PLUGIN_SRC_DIR` is set to the relative path of your plugin, e.g., `set(CUSTOM_PLUGIN_SRC_DIR plugins/epan/ti802154ge)`.

```cmake

# [...]

# Fail CMake stage if any of these plugins are missing from the source tree
set(CUSTOM_PLUGIN_SRC_DIR
#	private_plugins/foo
# or
#	plugins/epan/foo
	plugins/epan/ti802154ge
	plugins/epan/tirpi
	plugins/epan/ti-ble-packet-info
)

# [...]
```

### 5 - Re-run CMake generation step 

After that, re-run the CMake generation step. To build the plugin, run your normal Wireshark build step first.

### Building Wireshark
```sh
cd wireshark
mkdir build
cd build
cmake ..
```

If `cmake ..` returns an error like:
```sh
CMake Error at CMakeLists.txt:1239 (find_package): By not providing "FindQt5LinguistTools.cmake" in CMAKE_MODULE_PATH
```

Install the library:
```sh
sudo apt-get install qttools5-dev
```

Other similar errors should be related to Wireshark dependencies. Verify if all of them are installed correctly.

### 6 - Compiling the Plugin

After that you need to compile each plugin using makefile. 

```sh
cd wireshark/build/plugins/epan/ti802154ge/
make
cd..

cd tirpi/
make
cd ..

cd ti_ble_packet_info/
make
```

If executing make results in an exception regarding warnings being considered as errors, check your Wireshark version and make sure that it is 4.0. If the error persists, you can edit the following file:

```
wireshark/build/plugins/epan/ti802154ge/CMakeFiles/ti802154ge-x64-2x.dir/build.make
```

And use `Ctrl+F` to remove all `-Werror` flags from the makefile and run make again.

### 7 - Installing the plugin

To install the plugin you `cd` into its directory and run the command `sudo make install`.
An example of the command is:

```
ti802154ge git:(release-4.0) ✗ sudo make install
[sudo] password for user:
[  0%] Built target wmem
[  4%] Built target wsutil
[  4%] Built target crypt
[  4%] Built target lemon
[  5%] Built target dfilter
[  7%] Built target dissectors-corba
[  7%] Built target dissector-registration
[ 89%] Built target dissectors
[ 89%] Built target ftypes
[ 94%] Built target wiretap
[100%] Built target epan
[100%] Built target ti802154ge-x64-2x
Install the project...
-- Install configuration: "RelWithDebInfo"
-- Installing: /usr/local/lib/wireshark/plugins/4.0/epan/ti802154ge-x64-2x.so
-- Set runtime path of "/usr/local/lib/wireshark/plugins/4.0/epan/ti802154ge-x64-2x.so" to ""
```

The same process need to be repeated for all the plugins.

### 8 - Verify Installation
After `sudo make install`, you can open `Wireshark -> Help -> About Wireshark -> Plugins`. If everything went well, you should see the plugins on the list.

**Note:** The .so files generated are only for x64 systems!

## Extras

### Color Rules
You can [install color rules to facilitate ZigBee packet view](https://github.com/akestoridis/wireshark-zigbee-profile).

MAC Src, NWK Src, MAC Dst, NWK Dst, MAC SN, and NWK SN will only be visible in the Packet Summary if the Color Rules are enabled. If after adding the colors they don't show automatically, go to ``Wireshark -> Edit -> Configuration Profiles`` and select ZigBee. The same applies to BLE and other packet types.

### LUA dissectors
LUA scripts for WBMS packets were found in the same folder as the source code. While they were not used or tested, Wireshark LUA plugins are cross-platform and it should be able to dissect packets using them. If you want to use them you may try putting the `.lua` files on the plugins folder of the Wireshark installation, similar to where the other plugins were placed.

### Additional Info
You can configure a ZigBee Network Layer AES Key to see packet content. For this, you can follow step 6 of the tutorial on [Texas Instruments - Packet Sniffer - Wireshark](https://dev.ti.com/tirex4-desktop/explore/node?node=A__ACyc5n.8sICFMAbB44vblg__com.ti.SIMPLELINK_CC13XX_CC26XX_SDK__BSEc4rl__LATEST&placeholder=true).

