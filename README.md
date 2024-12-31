# linux-server-rapl-powermonitor

Power efficiency is an important factor for small-scale servers, such as those running in a home or lab environment. While a hardware watt-meter is the most direct way to measure power usage, not everyone has access to one (I certainly don't!). This script provides a software-based solution to measure the power consumption of your Intel CPU(s) using Intel's Running Average Power Limit (RAPL) interface.

This utility is specifically designed for Linux servers and offers an easy-to-use, accurate way to monitor CPU package power consumption.

---

## How It Works

The script leverages Intel's RAPL feature, which provides hardware-level energy usage data for various domains of a CPU, including the entire CPU package. It achieves this by accessing the Model-Specific Registers (MSRs) exposed by the processor. 

### Key Steps:
1. **Detect CPU Packages**: 
   It identifies all CPU packages using the topology data available in `/sys/devices/system/cpu/cpu%d/topology/physical_package_id`.
   
2. **Read MSR Values**: 
   It reads energy consumption data from the MSR registers (`0x611`) of each package and calculates power usage using the energy unit provided by the `0x606` register.
   
3. **Calculate Power**: 
   By sampling the energy values over a defined interval, it computes the average power usage in Watts.

> **Note**: The methodology is derived from a RAPL MSR reading utility created by Vince Weaver. You can find the original code and further details [here](https://web.eece.maine.edu/~vweaver/projects/rapl/).

---

## Requirements

To use this utility, ensure your system meets the following prerequisites:

- **Intel RAPL Support**: Only Intel processors with RAPL (typically Sandy Bridge and newer) are supported. Unfortunately, AMD CPUs are not supported.
- **MSR Module**: The Linux MSR driver must be enabled. Load it using:
  ```bash
  sudo modprobe msr
  chmod +x rapl_power
  sudo ./rapl_power

## Explanation

The energy unit is calculated from `MSR_RAPL_POWER_UNIT (0x606)`. It is extracted using the bitmask `0x1F00` and shifted by 8 bits. This value varies between CPUs and defines the smallest measurable energy unit.

The script maps packages to logical CPUs using the topology data. This ensures correct power readings on systems with multiple CPU sockets.

## Disclaimer

I have not tested the complete accuracy of this script using a watt-meter. This script is only meant to give a rough estimate of the power used by your CPUs.

Furthermore, this has only been tested on Ubuntu 22.04 LTS.


