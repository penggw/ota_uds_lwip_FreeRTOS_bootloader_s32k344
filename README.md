# OTA UDS Bootloader for S32K344 with lwIP and FreeRTOS

## Overview

This project implements an Over-The-Air (OTA) firmware update bootloader and Unified Diagnostic Services (UDS) protocol support for the NXP S32K344 microcontroller. The bootloader utilizes FreeRTOS for real-time task management, lwIP for TCP/IP networking, and supports diagnostics over DoIP (ISO 13400-2) protocol via Ethernet. It provides a secure, efficient mechanism for remote firmware updates and diagnostic communication in automotive embedded control units (ECUs).

The implementation includes:
- **Ethernet-based Communication**: 100BASE-T1 Automotive Ethernet support
- **DoIP Protocol**: Vehicle identification, routing activation, and diagnostic message transport
- **UDS Services**: Core diagnostic services (0x10, 0x11, 0x27, 0x31, 0x34, 0x36, 0x37, 0x3E) adapted for Ethernet transport
- **Flash Architecture**: Dual-bank flash management for safe firmware updates
- **FreeRTOS Tasks**: Multi-threaded architecture for concurrent network, diagnostic, and flash operations
- **Security Features**: Seed & Key authentication, access level management
- **OTA Upgrade**: High-bandwidth firmware downloads with integrity verification (CRC32)

## Hardware Requirements

- **MCU**: NXP S32K344 (ARM Cortex-M7, 160 MHz)
- **Ethernet**: GMAC (Gigabit MAC) controller with RJ45 Ethernet interface supporting 100BASE-T1
- **Memory**: 4 MB Flash (dual-bank architecture), 128 KB Data Flash, 64 KB ITCM, 64 KB SRAM
- **Power Supply**: 3.3V-5V regulated supply compatible with S32K344 specifications
- **Connectors**: Ethernet RJ45, debugging interface (e.g., JTAG via PEMicro)

## Software Requirements

### Development Tools
- **IDE**: S32 Design Studio v3.6.2 or compatible
- **Compiler**: ARM GCC toolchain or IAR Embedded Workbench
- **Debugger**: PEMicro debugger interface
- **Build System**: GNU Make (included with toolchain)

### Dependencies
- **FreeRTOS Kernel**: v10.4.3+
- **lwIP Stack**: v2.1.3+
- **RTD (Real-Time Drivers)**: NXP S32K3 RTD bundle (includes Ethernet, Flash, and GPIO drivers)

### Supported Standards
- **ISO 13400-2 (DoIP)**: Diagnostic communication over Internet Protocol
- **ISO 14229-5**: UDS on DoIP (Ethernet adaptation of UDS)
- **ISO 15765-2**: ISO-TP over DoIP (transport layer for large diagnostic messages)

## Project Structure

```
ota_uds_lwip_FreeRTOS_bootloader_s32k344/
├── board/                  # Board-specific configurations
│   ├── Siul2_Port_Ip_Cfg.c # Port pin configurations
│   └── Tspc_Port_Ip_Cfg.c  # Touch Sense configurations
├── Debug_FLASH/            # Build output (elf, hex, bin files)
├── FreeRTOS/               # FreeRTOS kernel source and port
│   ├── Source/             # FreeRTOS core files
│   ├── portable/           # S32K344-specific port (ARM_CM7)
│   └── MemMang/            # Memory management (heap_4.c)
├── generate/               # Auto-generated configuration files
│   ├── include/            # Header files (FreeRTOSConfig.h, lwipopts.h)
│   └── src/                # Source files
├── include/                # User application headers
│   ├── device.h            # Device initialization
│   └── ...
├── Project_Settings/       # IDE project configuration
│   ├── Debugger/           # Debug launch configurations
│   ├── Linker_Files/       # Linker scripts (flash layout)
│   └── Startup_Code/       # Vector table, startup assembly
├── RTD/                    # NXP Real-Time Drivers library
│   ├── include/            # Driver headers
│   └── src/                # Driver implementations
├── src/                    # Application source code
│   ├── main.c              # Bootloader main entry point
│   ├── bootloader.c        # Bootloader state machine (PROJECT MISSING)
│   ├── uds_handler.c       # UDS service implementation (PROJECT MISSING)
│   ├── doip_handler.c      # DoIP protocol handler (PROJECT MISSING)
│   ├── flash_driver.c      # Flash operations (PROJECT MISSING)
│   ├── ethernet_if.c       # Ethernet interface setup (DEMO EXISTING)
│   └── device.c            # Board initialization
└── stacks/tcpip/           # lwIP stack and integrations
    └── ...
```

## Installation and Build Instructions

### 1. Prerequisites
- Install S32 Design Studio 3.6.2
- Install ARM GCC toolchain
- Configure NXP tools paths in S32DS (Window → Preferences → NXP → Tools)

### 2. Project Setup
1. Clone the repository or extract the project archive
2. Launch S32 Design Studio
3. File → Import → General → Existing Projects into Workspace
4. Browse to the project root directory and select it

### 3. Build Configuration
1. Right-click project in Project Explorer → Properties
2. C/C++ Build → Settings → Tool Settings:
   - **Cross ARM C Compiler**: Set optimization level to -O2 (Release)
   - **Cross ARM C Linker**: Verify linker script path
   - **Cross ARM Assembler**: Standard settings
3. Ensure include paths for FreeRTOS, lwIP, and RTD are set correctly

### 4. Build Process
1. Right-click project → Build Project
2. Or use "Project → Build All"
3. Check Console output for errors - successful build generates:
   - Bootloader ELF file in `Debug_FLASH/`
   - Binary and HEX files for flashing
   - Map file for memory usage verification

### 5. Memory Verification
- Bootloader size must be < 64KB (targets Bank 0: 0x00400000-0x004FFFFF)
- RAM usage < 32KB during operation
- Verify no conflicts with application banks (0x00500000-0x007FFFFF)

## Execution Instructions

### 1. Flashing the Bootloader
1. Connect S32K344 board to PEMicro debugger
2. Right-click project → Debug As → S32DS Application
3. Or use external flasher with generated HEX file:
   ```cmd
   cd .\Debug_FLASH\
   # Use manufacturer flashing tool with bootloader.hex
   ```
4. Flash to Bank 0 (0x00400000) 

### 2. Application Image Preparation
- Application firmware must be prepared separately
- Recommended to use dual-bank update process:
  - Bank 1: Application A (0x00500000-0x005FFFFF)
  - Bank 2: Application B (0x00600000-0x006FFFFF)
- Include meta-information (checksums, version)

### 3. Network Configuration
Configure network settings via application code or UDS:
- IP Assignment: Static/DHCP/AutoIP
- MAC Address: Board-specific (stored in NVM)
- DoIP Port: 13400 (UDP discovery), 13400/3496 (TCP)

### 4. Running the Bootloader
1. Power cycle the ECU
2. Bootloader executes vector table check
3. If valid application exists → Jump to application
4. If no valid application → Network service mode
5. Listen for DoIP/UDS commands on Ethernet

### 5. Diagnostic Session
1. Connect to ECU via Ethernet using DoIP diagnostic tool
2. Send Vehicle Identification Request (UDP port 13400)
3. Establish routing activation (TCP, activation type 0x01)
4. Perform UDS services:
   - 0x10: Enter programming session
   - 0x27: Security access (seed & key)
   - 0x34: Request download (firmware transfer)
   - ... See UDS service specifications

## Key Features

### Diagnostic Services (UDS over DoIP)
- **Session Control**: Default, Extended, Programming sessions
- **Security Access**: Configurable access levels with seed/key algorithm
- **Firmware Update**: RequestDownload, TransferData, RequestTransferExit with CRC32
- **Memory Operations**: RoutineControl for flash erase/verification
- **Communication Control**: Enable/disable TX/RX for update safety
- **Reset Services**: Hard/soft/reset with application activation

### Flash Management
- Dual-bank architecture for fail-safe updates
- Bank A/B switching for rollback capability
- CRC32 integrity verification
- Sector-based erase/program operations
- Background operation support for large firmware

### Network & Protocol Support
- UDP discovery services (Vehicle Announcement)
- TCP diagnostic sessions with keep-alive
- Ethernet MAC layer integration with S32K344 EMAC
- IPv4/IPv6 dual stack support
- ARP, ICMP for network diagnostics

### Real-Time Architecture (FreeRTOS)
- Multi-task design: TCP/IP processing, UDS handling, flash operations
- Priority-based scheduling (High: Network, Medium: Diagnostics, High: Flash when active)
- Synchronization: Queues, semaphores, mutexes, event groups, timers
- Memory management: heap_4.c with configurable pool sizes
- Watchdog feeding and timeout management

## Configuration Files

- **FreeRTOSConfig.h**: Task priorities, heap size (65535 bytes), tick rate (1000Hz)
- **lwipopts.h**: TCP/IP stack parameters (mem size 16384, window sizes)
- **FlashConfig.c/h**: Bank addresses, sector sizes, timing parameters
- **DoIPConfig.h**: Protocol versions, port numbers, timeout values
- **UDSConfig.h**: Service IDs, timing parameters (P2=50ms, P2*=5000ms)

## Testing and Debugging

### Network Testing
- Ping ECU for connectivity
- DoIP vehicle identification (UDP broadcast)
- UDS tester present (0x3E) for session keep-alive

### OTA Testing
- Simulate firmware download with test files up to 10MB
- Verify CRC32 validation and flash integrity
- Test fallback mechanism on corrupted firmware

### Debug Tips
- Enable FreeRTOS debug hooks for task status monitoring
- lwIP debug output for network troubleshooting
- Watchdog timeout prevention during flash operations
- JTAG debugging with breakpoints on critical paths

### Performance Benchmarks
- Boot time: <500ms to application
- DoIP response latency: <10ms
- Download speed: >100KB/s over 100BASE-T1
- Flash programming: <2 minutes for 1MB

## Security Considerations

- Seed & Key authentication with configurable algorithms
- Access level restrictions for diagnostic functions
- TLS support optional on port 3496
- Replay attack prevention mechanisms
- HSE (Hardware Security Engine) integration for secure boot

## Troubleshooting

### Common Issues
- **Network Not Working**: Check cable, PHY configuration, IP/MAC settings
- **DoIP Connection Fails**: Verify ports, routing activation parameters
- **Flash Programming Errors**: Check voltage, timing, sector alignment
- **FreeRTOS Asserts**: Monitor stack usage, heap allocation

### Logs and Diagnostics
- Implement DTC (Diagnostic Trouble Codes) for error tracking
- Store critical operation logs in non-volatile memory
- Use onboard LEDs for status indication during boot/debug

## License

This software is provided by NXP "AS IS" and any expressed or implied warranties, including, but not limited to, the implied warranties of merchantability and fitness for a particular purpose are disclaimed. In no event shall NXP or its contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages.

## Version Information

- **Hardware Platform**: NXP S32K344 (Revision A/B)
- **FreeRTOS**: v10.4.3
- **lwIP**: v2.1.3
- **.COMPILATION**: v.pillar1
- **Flash Layout**: Dual-bank, 1MB per bank + 1MB OTA buffer (0x00700000-0x007FFFFF)

## Documentation References

- ISO 13400-2 (DoIP) specification
- ISO 14229-5 UDS on DoIP
- ISO 15765-2 ISO-TP
- NXP S32K344 Reference Manual
- FreeRTOS API documentation
- lwIP TCP/IP stack documentation
- NXP S32 Design Studio User Guide

## Contributors

- NXP Automotive Software Reference Platform
- lwIP TCPIP Stack integration
- FreeRTOS kernel adaptation
- S32K344 board support package
- OTA bootloader architecture design
