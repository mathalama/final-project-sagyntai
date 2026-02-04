# Final Project: Embedded Smart Security System
Author: Sagyntai (Mathalama)

## Project Overview
This project implements an Embedded Smart Security System using Buildroot, a custom Linux Kernel Driver, and a User-space Socket Server.

For detailed documentation, goals, and schedule, please visit the [Project Overview Wiki Page](https://github.com/mathalama/final-project-sagyntai/wiki/Project-Overview).

## System Architecture
1. **Kernel Driver (`security_mod`)**: A character driver that manages a circular buffer of security alerts.
2. **User-space App (`security_app`)**: A TCP socket server listening on port 9000. It reads alerts from the driver and broadcasts them to connected clients.
3. **Buildroot Integration**: The entire system is packaged using Buildroot for automated cross-compilation and deployment.

## How to Build
To build the project, run the following command in the root directory:
```bash
./build.sh
```

## How to Run & Test
1. **Start the system in QEMU**:
   ```bash
   ./runqemu.sh
   ```
2. **Verify the server is running**:
   The server starts automatically on boot. You can check it with:
   ```bash
   ps | grep security_app
   ```
3. **Trigger a security alert**:
   Write a message to the driver:
   ```bash
   echo "Movement detected in Zone 1" > /dev/security_sensor
   ```
4. **Connect to the server from the host or within QEMU**:
   ```bash
   nc localhost 9000
   ```
   You should receive the alert with a kernel timestamp.

## Project Structure
- `base_external/package/security-driver`: Kernel module source and Buildroot makefiles.
- `base_external/package/security-app`: Socket server source and Buildroot makefiles.
- `base_external/rootfs_overlay`: Configuration files and init scripts for the target system.