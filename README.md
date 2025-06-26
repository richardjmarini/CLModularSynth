# CLModularSynth

**CLModularSynth** is a modular synthesizer toolkit written in C++. It includes command-line tools for generating control voltages (`cv`), producing voltage-controlled oscillator waveforms (`vco`), and visualizing waveforms in real-time (`scope`).

---

## 🛠 Dependencies

This project requires the following dependencies:

### 🧱 Runtime & Build

- **C++17 or higher**  
  Modern C++ standard for core language features.

- **SDL2**  
  Used by `scope` to render real-time waveform visualization.  
  Install it with your system package manager:

  - **Ubuntu/Debian**:
    ```bash
    sudo apt install libsdl2-dev
    ```
  - **macOS** (via Homebrew):
    ```bash
    brew install sdl2
    ```

  - **Windows**:
    Download from [https://libsdl.org](https://libsdl.org) or use with vcpkg:
    ```bash
    vcpkg install sdl2
    ```

### 🧩 Header-only Library

- **[argparse](https://github.com/p-ranav/argparse)**  
  A modern C++ header-only argument parser.  
  To use it, clone the repo or copy `argparse.hpp` into your `include/` directory.

  ```bash
  git clone https://github.com/p-ranav/argparse.gi
  ```

## Build Instructions

To build the project, use:

```bash
make
```

## Usage
```bash
./cv [--sampleRate VAR] [--amplitude VAR] [--duration VAR]
```
| Option          | Description              | Default |
| --------------- | ------------------------ | ------- |
| `--sampleRate`  | Sampling rate (Hz)       | 48000   |
| `--amplitude`   | Output amplitude         | 1       |
| `--duration`    | Duration in seconds      | 1       |
| `-h, --help`    | Show help message        |         |
| `-v, --version` | Show version information |         |


```bash
./vco [--sensitivity VAR] [--sampleRate VAR] [--amplitude VAR]
```
| Option          | Description                 | Default |
| --------------- | --------------------------- | ------- |
| `--sensitivity` | Control voltage sensitivity | 1       |
| `--sampleRate`  | Sampling rate (Hz)          | 48000   |
| `--amplitude`   | Output amplitude            | 1       |
| `-h, --help`    | Show help message           |         |
| `-v, --version` | Show version information    |         |

```bash
./scope [--horizontal_scale VAR] [--trigger] [--trigger_threshold VAR] [--trigger_offset VAR] [--buffer_size VAR] [--window_width VAR] [--window_height VAR]
```
| Option                | Description                                    | Default |
| --------------------- | ---------------------------------------------- | ------- |
| `--horizontal_scale`  | Time scale of the horizontal axis              | 48000   |
| `--trigger`           | Enable trigger mode                            |         |
| `--trigger_threshold` | Trigger threshold level                        | 0.01    |
| `--trigger_offset`    | Samples to offset after trigger before display | 0       |
| `--buffer_size`       | Size of the internal ring buffer               | 800     |
| `--window_width`      | Width of the display window (pixels)           | 800     |
| `--window_height`     | Height of the display window (pixels)          | 400     |
| `-h, --help`          | Show help message                              |         |
| `-v, --version`       | Show version information                       |         |


## Exmaple
```bash
./cv --duration 2 | ./vco --sensitivity 100 | ./scope --trigger --horizontal_scale 800
```

![scope_screenshot](images/scope_screenshot.jpg)
