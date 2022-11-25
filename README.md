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

```
$ OsTools spi -h
Usage: OsTools spi <args>

    -h, --help            show this help message and exit

Options:
    -r, --read            read ls7a spi from address
    -u, --update          update ls3a spi flash
    -d, --dump            dump the ls3a spi flash
    -g, --gmac            update gmac flash
    -t, --tcm             read ls7a tcm from address

Arguments:
    -f, --file=<str>      file path to read/write
    -a, --address=<str>   Pci's spi control address(e.g. 1fe001f0)
    -i, --id=<int>        Mac id
    -m, --mac=<str>       Mac address(e.g. 00:11:22:33:44:55)
    -c, --count=<int>     read count
```


Update the Firmware from file:

```
sudo ./OsTools spi -u -f LS3A50007A.fd
```

Dump the current Firmware to file:

```
sudo ./OsTools spi -d -f LS3A50007A.dump.fd
```
