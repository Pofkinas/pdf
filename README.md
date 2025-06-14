# Pofkinas Development Framework (PDF)

**PDF** is a modular and reusable embedded framework for STM32 microcontrollers (currently targeting STM32F4 series)
It provides lightweight, efficient low‐level hardware access via STM32 Low‑Layer (LL) drivers and clean separation between drivers, APIs, utilities, and application logic.

---

## Table of Contents

- [Key Features](#key-features)
- [Supported Platforms](#supported-platforms)
- [Repository Structure](#repository-structure)
- [Getting Started](#getting-started)
  - [1. Add `PDF` as a Submodule](#1-add-pdf-as-a-submodule)
  - [2. Project Layout](#2-project-layout)
  - [3. Configure Include Paths](#3-configure-include-paths)
  - [4. Define Platform Config](#4-define-platform-config)
- [Building](#building)
  - [STM32CubeIDE](#stm32cubeide)
  - [Makefile / CMake](#makefile--cmake)
- [Platform Configuration](#platform-configuration)
- [License](#license)

---

## Key Features

- Modular file structure: `/Driver`, `/API`, `/APP`, `/Libs`, `/Utility`
- Based on STM32 Low Layer (LL) drivers
- RTOS-compatible
- Peripheral drivers: UART, I²C, GPIO, Timer, PWM, DMA, EXTI, WS2812B (LED), Motor
- Utility modules: ring buffer, message, math utils, error messages, led color, led animations, system utils
- External Libraries: `ST VL53L0X`

## Supported Platforms
`STM32F4`

## Repository Structure

```text
pdf/
├── Source/
│   ├── API/           # Reusable APIs
│   ├── APP/           # Reusable application demos (e.g. CLI)
│   ├── Driver/        # Hardware drivers (LL-based)
│   ├── Libs/          # External libraries
│   │   └── VL53L0X
│   └── Utility/       # Utility modules
│       └── Led_animation
```

## Getting Started

Follow these steps to integrate PDF into your own project.

### 1. Add `PDF` as a Submodule

```bash
git submodule add https://github.com/Pofkinas/pdf.git Framework
git submodule update --init --recursive
```

### 2. Project Layout

A typical project structure:

```text
ProjectName/
├── Application/                         # User application
│   ├── main.c                           # Entry point
│   ├── platform_config.h                # Platform configuration header file
│   ├── project_cli_cmd_handlers.c/.h    # /* OPTIONAL */ Custom CLI command handlers
│   └── project_cli_lut.c/.h             # /* OPTIONAL */ Custom CLI command definitions
├── ThirdParty/                          # MPU middlewares
│   ├── Core
│   ├── Drivers
│   └── Middlewares
└── Framework                            # PDF submodule
```

### 3. Configure Include Paths

Modify GCC Compiler include paths to match framework directory (relative to your project root):

```text
../Application
../Framework/Source/API
../Framework/Source/APP
../Framework/Source/Driver
../Framework/Source/Libs/VL53L0X/platform/inc
../Framework/Source/Libs/VL53L0X/core/inc
../Framework/Source/Utility
../Framework/Source/Utility/Led_animation
```

> **Note**: `Application/` must come *before* `Framework/Source/...` in the include‑path order.

### 4. Define Platform Config

Add preprocessor define in ordef for the framework to pick up configuration `platform_config.h`:

- **GCC / Makefile / CMake**: `-DPROJECT_CONFIG_H="platform_config.h"`
- **CubeIDE**: In **Project → Properties → C/C++ Build → Settings → MCU GCC Compiler → Preprocessor**, add:
  ```text
  PROJECT_CONFIG_H="platform_config.h"
  ```

---

## Building

PDF supports build systems:

### STM32CubeIDE

1. Generate project using CubeMX.
2. Exclude from build generated `main.c` and comment out `stm32x_it.c` interupt callbacks.
3. Exclude from build `/vl53l0x_i2c_platform.c`, `vl53l0x_i2c_win_serial_comms.c`, `/vl53l0x_platform_log.c` found in `/Source/Libs/VL53L0X/platform`.
4. Set include paths and preprocessor defines as described above.
5. Clean and build.

### Makefile / CMake

- Adjust your Makefile to include `Framework/Source/...` in `SRCS` and include paths.

---

## Platform Configuration

`Application/platform_config.h` controls which modules to use (compile) and their parameters.  See `Framework/Source/Utility/example_config.h` for a template:

```c
// Feature flags
#define USE_UART_DEBUG
#define USE_ONBOARD_LED
// #define USE_MOTOR_A

// System clock (Hz)
#define SYSTEM_CLOCK_HZ 100000000UL

// Module settings...
```

Uncomment the `USE_...` macros you need, and tune each module’s constants.

---

## License

This project is licensed under the GNU General Public License v3.0. See the [LICENSE](LICENSE) file for more details
