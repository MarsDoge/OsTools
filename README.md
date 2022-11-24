# OsTools

## Usage

```
Usage: OsTools <command> [<args>]

All commands:
    conf       Config
    gpio       GPIO
    rtc        Read or write rtc
    acpi       Read ACPI or Reboot
    spi        Read or write spi flash
    pci        Read PCI Bus
    ht         Read HT
    mps        Read or write MPS
    spd        Read SPD


    -h, --help        show this help message and exit
    -v, --version     show version
```

## spi example

Update the Firmware from file:

```
sudo ./OsTools spi -u -f LS3A50007A.fd
```

Dump the current Firmware to file:

```
sudo ./OsTools spi -d -f LS3A50007A.dump.fd
```
