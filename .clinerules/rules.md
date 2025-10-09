# Embedded C Development Rules for Automotive ECU - UDS Bootloader on S32K344

## Development Guidelines

### Core Development Rules

- Development environment is Windows system; terminal commands should use PowerShell-compatible syntax
- Do not directly generate project structure and related configurations; use command-line approach to create the project structure
- Function-level comments must be provided when generating code
- When planning the project, provide analysis recommendations for frameworks to be used, and allow the user to select the final framework

### Windows PowerShell Environment
- using PowerShell on Windows 
- In PowerShell, commands need to be separated by ";".
- separated "&&" is invalid. 

### Build Code
-  Build Code use PowerShell command "cd .\Debug_FLASH\ ; make all"

### Coordinator Pattern (Boomerang Pattern) Requirements

- All task completion reports must be recorded in the task report file `report.md`
- After completing the architecture pattern, generate a specification document `spec.md` and task list `todolist.md`
- The specification document must include flowcharts, sequence diagrams, relationship diagrams, and other relevant UML diagrams


### Code Development Workflow

- Code mode must follow `spec.md` for development
- `spec.md` must be confirmed before each code modification
- Task progress `todolist.md` must be updated after task completion
- Upon project completion, write a `readme.md` that includes project description, installation instructions, and execution methods
- 請在每個task進行中時將產出的文件source code完整提供出來

***

## Target Platform Specifications

### Hardware Configuration

- **MCU:** NXP S32K344 (ARM Cortex-M7, 160MHz)
- **IDE:** S32 Design Studio
- **SDK:** S32K3 RTD (Real-Time Drivers)
- **Compiler:** GCC ARM or IAR
- **Network:** Ethernet (100BASE-T1)

***

## Functional Requirements

### 1. Ethernet Communication Layer

#### Network Interface Requirements

- Support 100BASE-TX or 100BASE-T1 (Automotive Ethernet)
- Implement TCP/IP protocol stack (use lwIP or other lightweight TCP/IP stack)
- Support IPv4 or IPv6


#### MAC Layer Integration

- MAC layer integration with S32K344 Ethernet controller (EMAC)
- ARP and ICMP support for network diagnostics

***

### 2. DoIP Protocol Layer (ISO 13400-2)

#### Protocol Configuration

- **DoIP Version:** 0x02 or 0x03


#### UDP Communication (Port 13400)

- Vehicle Identification Request/Response
- DoIP Entity Status Request/Response
- Diagnostic Power Mode Request/Response
- Vehicle Announcement/Vehicle Identification Response Message


#### TCP Communication (Port 13400 for unsecured / 3496 for TLS)

- Routing Activation Request/Response (with source address validation)
- Diagnostic Message transmission (Payload Type 0x8001/0x8002)
- Alive Check mechanism with configurable timeout
- Diagnostic Message Acknowledgement (Positive/Negative ACK)
- Generic DoIP Header NACK (Payload Type 0x0000)


#### Address Management

- Logical Address configuration and management
- Source Address (SA) and Target Address (TA) processing
- Support for multiple concurrent TCP connections

***

### 3. ISO-TP Transport Layer (ISO 15765-2)

#### Frame Types

- Single Frame (SF)
- First Frame (FF)
- Consecutive Frame (CF)
- Flow Control (FC)


#### Timing Parameters

- **Block Size (BS):** 8
- **STmin (Separation Time):** 5ms
- **Timeout Handling:** N_Bs, N_Cr, N_As timeouts

***

### 4. UDS Application Layer (ISO 14229-5: UDS on DoIP)

#### Required UDS Services

**Session and Reset Control**

- **0x10:** DiagnosticSessionControl (Default/Programming/Extended sessions)
- **0x11:** ECUReset (Hard/Soft/KeyOffOnReset)

**Security and Communication Control**

- **0x27:** SecurityAccess (Seed \& Key algorithm with configurable access levels)
- **0x28:** CommunicationControl (Enable/Disable TX/RX)

**Routine and Data Transfer**

- **0x31:** RoutineControl (Erase Flash, Check Programming Dependencies, Check Memory)
- **0x34:** RequestDownload (with memory address and size parameters)
- **0x36:** TransferData (support for large payloads beyond CAN 8-byte limit)
- **0x37:** RequestTransferExit (with CRC verification)

**Diagnostic Management**

- **0x3E:** TesterPresent (with suppress positive response support)
- **0x22:** ReadDataByIdentifier (Optional: ECU info, DID reading)
- **0x2E:** WriteDataByIdentifier (Optional: configuration parameters)


#### Timing Requirements

- **P2_server_max:** 50ms
- **P2*_server_max:** 5000ms
- **P4Server:** Maximum time to final response (excluding NRC 0x78)
- **P6:** Complete response reception timeout


#### Error Handling

- Negative Response Codes (NRC) handling: 0x7F with proper error codes
- Continuous NRC 0x78 interval not less than 0.3 * P2*_server_max

***

### 5. FreeRTOS Integration

#### Task Architecture

**High Priority Tasks**

- **Ethernet RX Task (Priority: High)** - Process incoming DoIP packets
- **TCP/IP Stack Task (Priority: High)** - Handle lwIP processing
- **Flash Programming Task (Priority: High during active programming)** - Execute flash operations
- **Watchdog Service Task (Priority: Highest)** - Feed watchdog timer

**Medium Priority Tasks**

- **UDS Diagnostic Handler Task (Priority: Medium)** - Process UDS requests
- **Bootloader Main Control Task (Priority: Medium)** - State machine management

**Low Priority Tasks**

- **Alive Check Task (Priority: Low)** - DoIP keep-alive mechanism


#### Synchronization Mechanisms

- **Message Queues:** DoIP message passing between tasks
- **Binary Semaphores:** Flash access protection during write/erase
- **Counting Semaphores:** TCP connection management
- **Mutexes:** Shared resource protection (buffers, diagnostic data)
- **Event Groups:** Programming session state synchronization
- **Software Timers:** P2/P2* timeout management, Alive Check intervals


#### Memory Management

- **Heap Implementation:** heap_4.c or heap_5.c
- **Stack Size Allocation:** Minimum 2KB per task for network tasks
- **Total Heap Size:** Configure based on available SRAM


#### FreeRTOS Configuration

- **Tick Rate:** 1ms (configTICK_RATE_HZ = 1000)
- **Preemption:** Enabled (configUSE_PREEMPTION = 1)
- **Idle Hook:** Optional for low-power mode
- **Stack Overflow Detection:** Enabled during development

***

### 6. OTA Firmware Upgrade via Ethernet

#### Flash Memory Architecture (Dual-Bank)

**Memory Layout**

- **Bank 0:** 0x00400000 - 0x004FFFFF (1MB) - Bootloader
- **Bank 1:** 0x00500000 - 0x005FFFFF (1MB) - Application Bank A
- **Bank 2:** 0x00600000 - 0x006FFFFF (1MB) - Application Bank B
- **Bank 3:** 0x00700000 - 0x007FFFFF (1MB) - OTA Buffer Area (optional)


#### Flash Driver (S32K3 C40 IP)

- **Erase Operations:** Sector erase with background operation support
- **Program Operations:** 128-bit (16 bytes) aligned writes
- **Verify Operations:** Post-programming blank check


#### Data Integrity Verification

- CRC32 checksum calculation and validation
- Application validity checks (magic number, version info)
- Support for large firmware files leveraging Ethernet high bandwidth (up to 10MB)
- Block sequence counter to prevent data corruption


#### Bootloader Jump Logic

- Vector table relocation to application start address
- Stack pointer and reset vector validation before jump
- Pre-jump integrity check (CRC, signature validation)
- Fallback mechanism to previous valid firmware on failure


#### OTA Download Flow

**Step-by-Step Process**

1. Establish TCP connection and Routing Activation (0x01 activation type)
2. Enter Programming Session (0x10 02)
3. Security Access authentication (0x27 Seed/Key exchange)
4. Disable non-diagnostic communication (0x28)
5. Request Download with memory address and size (0x34)
6. Transfer Data via DoIP (0x36 - single packet up to 4GB theoretical limit)
7. Request Transfer Exit with CRC verification (0x37)
8. Routine Control: Check Programming Dependencies (0x31)
9. ECU Reset to activate new firmware (0x11 01)

***

### 7. Additional Requirements

#### Timeout Handling

- **TCP Connection Timeout:** 30 seconds inactivity
- **DoIP Alive Check Interval:** 500ms with 3 retry attempts
- **UDS P2/P2* Timeout Management:** NRC 0x78 (ResponsePending)
- **Routing Activation Timeout:** 5 seconds


#### Error Handling

- DoIP NACK responses (Payload Type 0x0000) with proper error codes
- UDS Negative Response (0x7F) with specific NRC values
- Network disconnection recovery and reconnection handling
- Flash programming error recovery (erase retry, verify failure handling)
- Watchdog reset prevention during long operations


#### Security Features

- **Optional TLS Support:** Port 3496 for encrypted diagnostic communication
- **Seed \& Key Algorithm:** Implementation (custom or OEM-specific)
- **Access Level Management:** Prevent unauthorized firmware updates
- **Replay Attack Prevention:** Security measures implementation
- **Secure Boot Verification:** Optional with HSE integration


#### Diagnostic Logging

- Log critical operations to non-volatile memory
- Record programming attempts, failures, and successes
- Store DTC (Diagnostic Trouble Codes) for bootloader errors
- Timestamp logging for audit trail

***

### 8. Performance Targets

#### Speed and Timing Requirements

- **Firmware Download Speed:** > 100 KB/s over Ethernet
- **Flash Programming Time:** < 2 minutes for 1MB application
- **Boot-up Time to Application:** < 500ms
- **DoIP Response Latency:** < 10ms for diagnostic messages


#### Memory Footprint

- **Bootloader Size:** < 64KB
- **RAM Usage:** < 32KB

***

## Key Enhancements Summary

### Added Components

1. **FreeRTOS Multi-Task Architecture** - Real-time task management with proper prioritization for network handling, diagnostics processing, and flash operations
2. **Ethernet/DoIP Communication Stack** - Replaces CAN communication with high-bandwidth Ethernet, enabling faster and larger firmware transfers
3. **DoIP Protocol Implementation (ISO 13400)** - Complete UDP discovery and TCP diagnostic message handling with routing activation
4. **UDS over DoIP (ISO 14229-5)** - Ethernet-adapted UDS service implementation with extended payload support

### Retained Components

- Core UDS services (0x10, 0x11, 0x27, 0x31, 0x34, 0x36, 0x37, 0x3E)
- Dual-bank Flash architecture with CRC32 verification
- Bootloader jump logic and application validity checks
- S32K344 platform and development toolchain setup
