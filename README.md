# ASM2 - Embedded Linux Sensor Driver

## 1. Overview

This project focuses on Embedded Linux device driver development and communication between kernel space and user space.

## 2. Objectives

- Develop and test a Linux kernel driver.
- Exchange sensor-related data between the kernel and a user-space application.
- Maintain shared-buffer statistics.
- Practice kernel and user-space debugging.

## 3. Technologies Used

- Embedded Linux
- Linux Kernel
- C Programming Language
- Linux Device Drivers
- GNU Make
- ARM Cross-Compilation Toolchain
- QEMU, if used by the project

## 4. Main Components

- `sms_sensor`: Kernel-side sensor driver.
- `sms_app`: User-space application.
- Shared buffer: Stores data exchanged between components.
- Statistics: Tracks relevant counters and temperature extremes.

## 5. Build and Run

Follow the build instructions provided in the project Makefile and assignment specification.

Build the kernel driver and user-space application separately using their respective build configurations.

Run the application in the configured target environment and verify the output.

## 6. Testing

Testing should verify:

- Successful driver initialization.
- Correct communication between kernel space and user space.
- Accurate shared-buffer statistics.
- Correct reporting of counters when the application exits.
- Appropriate handling of errors.

## 7. Learning Outcomes

- Understand kernel and user-space interaction.
- Practice device driver development.
- Learn to collect and report runtime statistics.
- Improve embedded software testing skills.

## 8. Author

**Name:** Le Minh Dang  
**University:** FPT University (FPTU)
