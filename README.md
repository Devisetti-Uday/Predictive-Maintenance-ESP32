# Predictive Maintenance for Industrial Machines

An ESP32-based machine condition monitoring system that detects early signs of machine problems using vibration and temperature analysis. The complete project is simulated in **Wokwi**, so no physical hardware is required.

## 🔗 Wokwi Simulation

The complete project can be opened and tested directly in Wokwi.

**▶ [Open Wokwi Simulation](https://wokwi.com/projects/477229362545609729)**

## Problem Statement

Unexpected industrial machine failures can cause production downtime, maintenance costs, and safety risks. Traditional maintenance methods often depend on fixed schedules or manual inspection, which may fail to identify developing problems early.

This project provides a **sensor-based predictive maintenance system** that continuously monitors machine conditions and identifies abnormal behavior before it develops into a serious failure.

## Project Overview

The system uses an **ESP32** with an **MPU6050 accelerometer** and two **DHT22 temperature sensors**.

- One DHT22 represents the **machine temperature**.
- The second DHT22 represents the **ambient temperature**.
- The MPU6050 monitors **acceleration and vibration deviations**.
- A baseline is established while the machine is assumed to be operating normally.
- New measurements are compared with this baseline.
- A health score is calculated based on temperature deviation, vibration, and vibration trend.
- The system classifies the machine into:
  - HEALTHY
  - MAINTENANCE REQUIRED
  - CRITICAL
- Multiple consecutive observations are used before confirming an abnormal condition to reduce false alarms.

The system provides visual and audio alerts using an RGB LED and buzzer and displays the current condition on a 16x2 I2C LCD.

## Features

- ESP32-based machine monitoring
- Vibration monitoring using MPU6050
- Machine temperature monitoring using DHT22
- Ambient temperature monitoring using a second DHT22
- Automatic normal-condition baseline calibration
- Machine-to-ambient temperature comparison
- Vibration deviation analysis
- Vibration trend monitoring
- Health score from 0 to 100
- Three-level machine condition classification
- Multiple-measurement confirmation
- RGB LED status indication
- Buzzer alert for critical conditions
- 16x2 LCD status display
- Complete Wokwi simulation
- No physical hardware required

## Components Used

| Component | Purpose |
|---|---|
| ESP32 | Main microcontroller |
| MPU6050 | Vibration and acceleration monitoring |
| DHT22 #1 | Machine temperature |
| DHT22 #2 | Ambient temperature |
| 16x2 I2C LCD | Displays machine status and temperature |
| RGB LED | Visual machine health indication |
| Buzzer | Critical condition alert |
| 220Ω Resistors | RGB LED current limiting |

## System Architecture

```text
                ┌──────────────────────┐
                │        ESP32         │
                │   Main Controller    │
                └──────────┬───────────┘
                           │
          ┌────────────────┼────────────────┐
          │                │                │
          ▼                ▼                ▼
     ┌─────────┐      ┌─────────┐     ┌──────────┐
     │ MPU6050 │      │ DHT22 #1│     │ DHT22 #2 │
     │Vibration│      │ Machine │     │ Ambient  │
     │         │      │   Temp  │     │   Temp   │
     └─────────┘      └─────────┘     └──────────┘
          │                │                │
          └────────────────┼────────────────┘
                           ▼
                ┌──────────────────────┐
                │ Condition Analysis   │
                │                      │
                │ Baseline Comparison  │
                │ Vibration Analysis   │
                │ Temperature Analysis │
                │ Trend Analysis       │
                │ Health Score         │
                └──────────┬───────────┘
                           │
             ┌─────────────┼─────────────┐
             ▼             ▼             ▼
        ┌─────────┐   ┌──────────┐   ┌─────────┐
        │   LCD   │   │ RGB LED  │   │ Buzzer  │
        │ Display │   │  Status  │   │  Alert  │
        └─────────┘   └──────────┘   └─────────┘