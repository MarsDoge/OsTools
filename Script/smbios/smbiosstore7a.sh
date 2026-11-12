#!/bin/bash

PCIE_DEV22_PATH="/sys/bus/pci/devices/0000:00:16.0"
get_pcie_bar0_base()
{
    local dev_path="$1"
    if [ ! -f "${dev_path}/resource" ];then
        echo "ERROR: ${dev_path}/resource not found!" >&2
        exit 1
    fi
    # first line of resource is BAR0, get first field
    cat "${dev_path}/resource" | head -n1 | cut -d' ' -f1
}
bar_base=$(get_pcie_bar0_base "$PCIE_DEV22_PATH")

./OsTools spi -a $bar_base -C -s 1 1 type1_index1_7a_Manufacture                  # update smbios type1 Manufacture
./OsTools spi -a $bar_base -C -s 1 2 type1_index2_7a_ProductName                  # update smbios type1 ProductName
./OsTools spi -a $bar_base -C -s 1 3 type1_index3_7a_Version                      # update smbios type1 Version
./OsTools spi -a $bar_base -C -s 1 4 type1_index4_7a_SerialNumber                 # update smbios type1 SerialNumber
./OsTools spi -a $bar_base -C -s 1 5 type1_index5_7a_SKUNumber                    # update smbios type1 SKUNumber
./OsTools spi -a $bar_base -C -s 1 6 type1_index6_7a_Family                       # update smbios type1 Family
./OsTools spi -a $bar_base -C -s 1 7 00112233-4455-6677-8899-AABBCCDDEEFF         # update smbios type1 UUID
./OsTools spi -a $bar_base -C -s 2 1 type2_index1_7a_Manufacture                  # update smbios type2 Manufacture
./OsTools spi -a $bar_base -C -s 2 2 type2_index2_7a_Product                      # update smbios type2 Product
./OsTools spi -a $bar_base -C -s 2 3 type2_index3_7a_Version                      # update smbios type2 Version
./OsTools spi -a $bar_base -C -s 2 4 type2_index4_7a_SerialNumber                 # update smbios type2 SerialNumber
./OsTools spi -a $bar_base -C -s 2 5 type2_index5_7a_AssetTag                     # update smbios type2 AssetTag
./OsTools spi -a $bar_base -C -s 3 1 type3_index1_7a_Manufacture                  # update smbios type3 Manufacture
./OsTools spi -a $bar_base -C -s 3 2 type3_index2_7a_Version                      # update smbios type3 Version
./OsTools spi -a $bar_base -C -s 3 3 type3_index3_7a_SerialNumber                 # update smbios type3 SerialNumber
./OsTools spi -a $bar_base -C -s 3 4 type3_index4_7a_AssetTag                     # update smbios type3 AssetTag
./OsTools spi -a $bar_base -C -s 3 5 type3_index5_7a_SKUNumber                    # update smbios type3 SKUNumber
