
# Grove RGB LCD V4.0 Driver for BeagleBone Black

This repository contains a simple I2C driver for the Grove RGB LCD V4.0 Display Interface, designed to be used with a BeagleBone Black running Linux. The driver enables basic operations such as initializing the LCD and RGB backlight, writing text to the LCD, and controlling the RGB backlight color via sysfs, procfs, and ioctl interfaces.

## Table of Contents

- [Features](#features)
- [Requirements](#requirements)
- [Installation](#installation)
- [Usage](#usage)
  - [Sysfs Interface](#sysfs-interface)
  - [Procfs Interface](#procfs-interface)
  - [IOCTL Interface](#ioctl-interface)
- [License](#license)

## Features

- Initializes the LCD with default settings.
- Controls the RGB backlight.
- Allows writing text to the LCD.
- Provides interfaces to control and read backlight color and displayed text using sysfs, procfs, and ioctl.

## Requirements

- BeagleBone Black running a Linux distribution (tested with Debian).
- I2C enabled in the device tree.
- Grove RGB LCD V4.0 module connected to I2C bus 2.

## Installation

1. Clone this repository to your BeagleBone Black:
   ```bash
   git clone https://github.com/divyeshkapadiya-moschip/grove_rgb_lcd_device_driver.git
   cd grove_rgb_lcd_device_driver
   ```

2. Build the driver:
   ```bash
   make
   ```

3. Insert the driver module:
   ```bash
   sudo insmod lcd_driver.ko
   ```

4. Verify that the device nodes and sysfs entries are created:
   ```bash
   ls /dev/rgb_lcd
   ls /sys/kernel/rgb_lcd_sysfs
   ```

## Usage

### Sysfs Interface

The sysfs interface allows you to read and write the RGB backlight color and the text displayed on the LCD.

- Read the current RGB backlight color:
  ```bash
  cat /sys/kernel/rgb_lcd_sysfs/rgb_color
  ```

- Set the RGB backlight color :
  ```bash
  echo "Red : 100, Green : 500, Blue : 500" > /sys/kernel/rgb_lcd_sysfs/rgb_color
  ```

- Read the current text displayed on the LCD:
  ```bash
  cat /sys/kernel/rgb_lcd_sysfs/lcd_text
  ```

- Set the text displayed on the LCD:
  ```bash
  echo "Hello, BeagleBone!" > /sys/kernel/rgb_lcd_sysfs/lcd_text
  ```

### Procfs Interface

The procfs interface provides a read-only file that combines the RGB backlight color and the displayed text.

- Read the RGB backlight color and the text displayed on the LCD:
  ```bash
  cat /proc/rgb_lcd
  ```

### IOCTL Interface

The ioctl interface allows more advanced control, such as setting and getting the RGB backlight color and the text displayed on the LCD from a user-space application.

- Example usage in a C program:
  ```bash
  cd app
  ./lcd_test_app
  ```

- Upon running the application, you will be presented with a menu to choose an operation:
    - Read RGB values
    - Write RGB values
    - Read LCD text value
    - Write LCD text value
    - Exit

- Follow the on-screen prompts to perform the desired operation.

- Ensure proper permissions to access the `/dev/rgb_lcd` device file.
- The RGB values range from 0 to 255, and the LCD text input is limited to 32 characters.



## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.

---

**Author**: Divyesh Kapadiya

**Contact**: divyesh.kapadiya@moschip.com

**Tested with**: Linux BeagleBone Black

---

For any questions or issues, please open an issue in the repository or contact the author.
