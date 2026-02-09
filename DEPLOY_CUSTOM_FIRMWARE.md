# How to Deploy "Jailbroken" Custom Firmware to IoTaWatt

This guide details how to flash the custom, signature-bypass firmware to your IoTaWatt device. This "jailbreak" is a **one-time physical requirement**. Once this firmware is running, future custom updates can be performed over-the-air (OTA).

## Prerequisites

1.  **Physical Access**: You need access to the IoTaWatt unit.
2.  **Micro-USB Cable**: Ensure it is a data cable, not just a charging cable.
3.  **Computer**: A machine with Python and USB ports.
4.  **Drivers**: You may need CH340 or CP210x drivers depending on the USB-Serial chip inside your IoTaWatt (usually NodeMCU-style).

## Step 1: Locate the Firmware Binary

The custom firmware has been built and is located at:
`Firmware/.pio/build/iotawatt/firmware.bin`

## Step 2: Install Flashing Tool (`esptool`)

You need `esptool` to communicate with the ESP8266 bootloader.

```bash
pip install esptool
```

## Step 3: Put IoTaWatt in Bootloader Mode

1.  Unplug the IoTaWatt from USB/Power.
2.  Open the IoTaWatt case (if necessary) to access the NodeMCU/ESP8266 module.
3.  **Hold down the FLASH button** (often labeled `FLASH` or `D3` on NodeMCU modules).
4.  While holding FLASH, plug the USB cable into your computer.
5.  Release the FLASH button after 1-2 seconds.

*Note: Some modern dev boards enter bootloader mode automatically via the DTR/RTS USB signals. You can try skipping step 3 first.*

## Step 4: Identify COM Port

Run the following to find your device port:

*   **Linux/Mac**: `ls /dev/tty*` (look for `/dev/ttyUSB0` or `/dev/tty.SLAB_USBtoUART`)
*   **Windows**: Check Device Manager (e.g., `COM3`, `COM4`)

## Step 5: Flash the Firmware

Run the flash command. Replace `YOUR_PORT` with the port found in Step 4 (e.g., `/dev/ttyUSB0` or `COM3`).

```bash
esptool.py --port YOUR_PORT --baud 921600 write_flash -fm dio 0x00000 Firmware/.pio/build/iotawatt/firmware.bin
```

*   `0x00000`: The start address for the firmware on ESP8266.
*   `-fm dio`: Flash mode (Dual I/O).

## Step 6: Verify and Reboot

1.  Watch the progress bar reach 100%.
2.  Once finished, unplug the USB cable and plug it back in (power cycle) to boot the new firmware.
3.  Connect to the device's WiFi AP or access it via its existing IP if credentials persisted.

## Future Updates (OTA)

Now that your device is running the "Jailbroken" firmware with signature verification disabled:

1.  **Build** your new custom firmware changes.
2.  **Pack** the firmware into a standard release `.bin` (using the IoTaWatt release packer tool, or by mocking the header structure - `unpackUpdate` still expects the file format, it just won't verify the signature).
3.  **Upload** via the IoTaWatt File Manager to `/updates/02_06_00.bin` (increment version).
4.  **Restart** the device to trigger the update.
