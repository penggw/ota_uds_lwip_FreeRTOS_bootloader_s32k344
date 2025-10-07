# OTA UDS lwIP FreeRTOS Bootloader for S32K344

This project implements an embedded network communication application based on the NXP S32K344 microcontroller, featuring FreeRTOS real-time operating system and lwIP TCP/IP stack. The application provides various network services and serves as a foundation for OTA (Over-The-Air) updates and UDS (Unified Diagnostic Services) protocol implementation.

## Features

### Core Components
- **FreeRTOS**: Real-time operating system for multitasking
- **lwIP Stack**: Full-featured TCP/IP protocol stack
- **RTD Drivers**: NXP Real-Time Drivers for S32K344 platform
- **Ethernet Support**: GMAC (Gigabit MAC) Ethernet controller

### Network Services
- **HTTP Server**: Web server with file system support
- **Echo Services**: TCP and UDP echo servers for connectivity testing
- **DHCP Client**: Dynamic IP address configuration
- **DNS Client**: Domain name resolution
- **TFTP Server**: Trivial File Transfer Protocol for file operations
- **mDNS**: Multicast DNS (Bonjour) service discovery
- **SNMP**: Simple Network Management Protocol
- **NTP Client**: Network Time Protocol synchronization
- **Performance Tools**: iPerf, RTP for network performance testing

### Protocol Support
- IPv4/IPv6 Dual Stack
- AutoIP for automatic IP configuration
- ICMP Ping support
- TCP/UDP socket programming
- Network interface management

## Hardware Requirements

- **MCU**: NXP S32K344 (ARM Cortex-M7)
- **Ethernet**: GMAC controller with RJ45 Ethernet interface
- **Memory**: 4MB Flash, 128KB Data Flash, 64KB ITCM
- **Power Supply**: Appropriate voltage regulation for S32K344

## Software Requirements

### Development Tools
- **S32 Design Studio**: Version 3.6.2 or compatible
- **GCC ARM Compiler**: ARM GCC toolchain
- **PEMicro Debugger**: For flashing and debugging

### Dependencies
- **FreeRTOS Kernel**: v10.4.3+
- **lwIP Stack**: v2.1.3+
- **RTD (Real-Time Drivers)**: NXP S32K3 RTD bundle

## Project Structure

```
ota_uds_lwip_FreeRTOS_bootloader_s32k344/
├── board/                     # Board-specific configurations
│   ├── Siul2_Port_Ip_Cfg.c/h  # Port configurations
│   └── Tspc_Port_Ip_Cfg.c/h   # Touch Sense configurations
├── Debug_FLASH/               # Build output and debug artifacts
├── FreeRTOS/                  # FreeRTOS kernel source
├── generate/                  # Auto-generated configuration files
├── include/                   # User header files
│   ├── device.h              # Device initialization
│   └── ...                   # Other user headers
├── Project_Settings/          # Project configuration files
│   ├── Debugger/             # Debugger launch configurations
│   ├── Linker_Files/         # Linker scripts
│   ├── Startup_Code/         # Startup and vector table code
│   └── ...
├── RTD/                      # NXP Real-Time Drivers
│   ├── include/              # Driver headers
│   ├── src/                  # Driver implementations
│   └── ...
├── src/                      # User application source
│   ├── main.c                # Application entry point
│   ├── device.c              # Device initialization
│   └── test.c                # Network test application
└── stacks/                   # Protocol stacks
    └── tcpip/                # lwIP stack and applications
```

## Build and Run

### 1. Import Project
1. Launch S32 Design Studio 3.6
2. Select "File" → "Import" → "Existing Projects into Workspace"
3. Navigate to the project root directory
4. Select and import the project

### 2. Configure Build Settings
1. Right-click project → "Properties"
2. Navigate to "C/C++ Build" → "Settings"
3. Verify toolchain paths and options
4. Ensure RTD and FreeRTOS paths are correctly set

### 3. Build Project
1. Right-click project → "Build Project"
2. Monitor build output for any compilation errors
3. Check `Debug_FLASH/` directory for output files

### 4. Debug and Run
1. Right-click project → "Debug As" → "S32DS Application"
2. Use PEMicro debugger for flashing and debugging
3. Set breakpoints as needed
4. Launch debug session

## Application Flow

1. **System Initialization**: MCU and peripherals initialization
2. **OS Setup**: FreeRTOS task scheduler initialization
3. **Network Stack Setup**: lwIP TCPIP thread initialization
4. **Interface Configuration**: Ethernet interface setup with DHCP/AutoIP
5. **Service Startup**: Launch network services (HTTP, DNS, etc.)
6. **Main Loop**: Process network events and timers
7. **Timeout Handling**: Automatic shutdown after test period

## Configuration

### Network Configuration
Edit `generate/include/netifcfg.h` for network interface settings:
```c
// Ethernet interface configuration
#define ETHIF_NUMBER           1U
#define IFACE0_IP_ADDR         "10.42.0.200"
#define IFACE0_NETMASK         "255.255.255.0"
#define IFACE0_GATEWAY         "10.42.0.1"
```

### FreeRTOS Configuration
Edit `generate/include/FreeRTOSConfig.h` for OS parameters:
```c
#define configTOTAL_HEAP_SIZE               ((size_t)65535)
#define configMAX_PRIORITIES                (32)
#define configTICK_RATE_HZ                  ((TickType_t)1000)
```

### lwIP Configuration
Edit `generate/include/lwipopts.h` for TCP/IP stack settings:
```c
#define TCP_WND                        (8 * TCP_MSS)
#define TCP_SND_BUF                    (8 * TCP_MSS)
#define MEM_SIZE                       16384
```

## Testing and Verification

### Network Connectivity Testing
1. **Ping Test**: Verify IP connectivity
2. **HTTP Test**: Access web interface at device IP
3. **Echo Test**: Use TCP/UDP echo services
4. **DNS Test**: Domain name resolution verification

### Performance Testing
- Use iPerf for bandwidth testing
- NTP sync for timing verification
- SNMP for system monitoring

## Debugging Tips

### Common Issues
1. **Network Interface Not Coming Up**: Check Ethernet cable connection
2. **DHCP Not Working**: Verify router DHCP server settings
3. **FreeRTOS Asserts**: Check stack/heap usage in tasks
4. **lwIP Memory Errors**: Adjust memory pool sizes

### Debug Output
Enable printf output in `lwipcfg.h` for network debugging:
```c
#define PRINTF_SUPPORT                  1
#define LWIP_DEBUG                      LWIP_DBG_ON
```

## OTA and UDS Support

This project provides the network infrastructure for OTA (Over-The-Air) updates and UDS (Unified Diagnostic Services). While the actual OTA/UDS protocol implementations are not included in this code base, the lwIP stack and FreeRTOS environment serve as the foundation for such additions.

Potential extensions:
- **OTA**: Firmware update over HTTP/TFTP
- **UDS**: Diagnostic communication over DoIP or TCP/IP
- **Security**: TLS/SSL for secure communications
- **CAN**: Integration with vehicle networks

## License

This software is provided by NXP "AS IS" and any expressed or implied warranties, including, but not limited to, the implied warranties of merchantability and fitness for a particular purpose are disclaimed. In no event shall NXP or its contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages.

## Version

- **MCU Platform**: NXP S32K344
- **FreeRTOS**: v10.4.3
- **lwIP**: v2.1.3
- **RTD Bundle**: Latest for S32K3 family

## Support

For technical support and documentation:
- NXP S32K3 Reference Manual
- FreeRTOS Documentation
- lwIP Documentation
- NXP S32 Design Studio User Guide

## Contributors

- Based on NXP Automotive Software Reference Platform
- lwIP TCP/IP Stack integration
- FreeRTOS real-time operating system integration
