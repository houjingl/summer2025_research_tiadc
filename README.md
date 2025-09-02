# Summer 2025 Reasearch Summary - Time-Interleaved ADC Calibrator Project

This repository documents my summer research contributions related to the FPGA side of the **Time-Interleaved (TI) ADC calibration project**.

---

## 📌 Project Summary

The main objective of this project is to design and implement a **hardware calibration algorithm** that corrects **timing, gain, offset, and bandwidth mismatches** in high-speed TI ADCs.  

The approach involves injecting a **reference dither signal** into each time-interleaved channel to assist in calibration. By recognizing the peak and ideal amplitude of the dither signal, the calibration model gradually tunes ADC parameters using a **negative feedback loop**, ultimately correcting mismatches and improving overall ADC performance.

---

## 📖 What I Learned

- **TI ADC Concepts**
  - Time-interleaving technique for increasing ADC sampling frequency
  - Different types of TI ADC mismatches (timing, gain, offset, bandwidth)

- **FPGA & SoC Development**
  - Xilinx Vivado & Vitis development workflow
  - Zynq Processing System (PS) and PS–PL integration
  - ARM Cortex-A9 MCU hardware control using C

- **Networking Fundamentals**
  - TCP/IP protocol stack and layered model
  - Ethernet frames and IPv4 addressing
  - TCP vs. UDP and their application scenarios
  - Lightweight IP (lwIP) stack for embedded networking

- **AXI Protocols**
  - AXI Full, AXI Lite, AXI Stream interfaces
  - AXI4 handshake mechanism and dependency rules

- **High-Speed Data Transmission**
  - CMOS, LVDS, and JESD204B standards
  - SERDES, embedded clocking, and CDR (clock data recovery)
  - JESD204B layered model for ADC data transport

- **Tools & Algorithms**
  - ADI ACE and DPG tools for signal generation
  - MATLAB for simulation, correlation, and FIR filters
  - Gradient Descent, LMS, and Genetic Algorithm optimization methods

---

## ⚙️ What I Contributed

- **Pre-silicon Design Verification**
  - Built an FPGA-based verification platform using **FIFO, AXI DMA, Ethernet, and Zynq PS**  
  - Enabled real-time ADC sample acquisition and monitoring

- **Networking Integration**
  - Deployed **lwIP** on ARM PS core  
  - Wrote Python scripts to exchange UDP packets with FPGA and call MATLAB functions  
  - Established a hardware-in-the-loop environment for calibration algorithm testing

- **FPGA/RTL Development**
  - Designed an **AXI Lite wrapper** for the TI ADC calibrator IP  
  - Verified functionality using **SystemVerilog testbench**

- **Algorithm Simulation**
  - Simulated **Gradient Descent** for clock skew calibration in MATLAB
  - Developed a **2D Genetic Algorithm** to calibrate both clock skew and gain mismatch

- **Signal Generation**
  - Generated critical **dither signals** using ADI’s ACE tool and MATLAB

---

## 🛠️ Tech Stack

- **Hardware**: Xilinx Zynq SoC (PS + PL), JESD204B-based ADC/DAC  
- **FPGA Tools**: Vivado, Vitis  
- **Languages**: SystemVerilog, Verilog, C, Python, MATLAB  
- **Networking**: lwIP, UDP/TCP/IP  
- **Signal Tools**: ADI ACE, DPG, MATLAB DSP toolbox

---

## 📜 Acknowledgments

This project was conducted as part of my summer research under **Prof. Xilin Liu** at the University of Toronto.  
Special thanks to **Bowen Liu** for technical guidance and mentorship.  

---
