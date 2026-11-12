#!/bin/bash

./OsTools spi -C -s 1 1 type1_index1_Manufacture                  # update smbios type1 Manufacture
./OsTools spi -C -s 1 2 type1_index2_ProductName                  # update smbios type1 ProductName
./OsTools spi -C -s 1 3 type1_index3_Version                      # update smbios type1 Version
./OsTools spi -C -s 1 4 type1_index4_SerialNumber                 # update smbios type1 SerialNumber
./OsTools spi -C -s 1 5 type1_index5_SKUNumber                    # update smbios type1 SKUNumber
./OsTools spi -C -s 1 6 type1_index6_Family                       # update smbios type1 Family
./OsTools spi -C -s 1 7 00112233-4455-6677-8899-AABBCCDDEEFF      # update smbios type1 UUID
./OsTools spi -C -s 2 1 type2_index1_Manufacture                  # update smbios type2 Manufacture
./OsTools spi -C -s 2 2 type2_index2_Product                      # update smbios type2 Product
./OsTools spi -C -s 2 3 type2_index3_Version                      # update smbios type2 Version
./OsTools spi -C -s 2 4 type2_index4_SerialNumber                 # update smbios type2 SerialNumber
./OsTools spi -C -s 2 5 type2_index5_AssetTag                     # update smbios type2 AssetTag
./OsTools spi -C -s 3 1 type3_index1_Manufacture                  # update smbios type3 Manufacture
./OsTools spi -C -s 3 2 type3_index2_Version                      # update smbios type3 Version
./OsTools spi -C -s 3 3 type3_index3_SerialNumber                 # update smbios type3 SerialNumber
./OsTools spi -C -s 3 4 type3_index4_AssetTag                     # update smbios type3 AssetTag
./OsTools spi -C -s 3 5 type3_index5_SKUNumber                    # update smbios type3 SKUNumber

# demo with '-c'
# ./OsTools spi -c 0x4000 -C -s 1 1 type1_index1_Manufacture
# ./OsTools spi -c 0x4000 -C -s 1 2 type1_index2_ProductName
# ./OsTools spi -c 0x4000 -C -s 1 3 type1_index3_Version
# ./OsTools spi -c 0x4000 -C -s 1 4 type1_index4_SerialNumber
# ./OsTools spi -c 0x4000 -C -s 1 5 type1_index5_SKUNumber
# ./OsTools spi -c 0x4000 -C -s 1 6 type1_index6_Family
# ./OsTools spi -c 0x4000 -C -s 1 7 00112233-4455-6677-8899-AABBCCDDEEFF
# ./OsTools spi -c 0x4000 -C -s 2 1 type2_index1_Manufacture
# ./OsTools spi -c 0x4000 -C -s 2 2 type2_index2_Product
# ./OsTools spi -c 0x4000 -C -s 2 3 type2_index3_Version
# ./OsTools spi -c 0x4000 -C -s 2 4 type2_index4_SerialNumber
# ./OsTools spi -c 0x4000 -C -s 2 5 type2_index5_AssetTag
# ./OsTools spi -c 0x4000 -C -s 3 1 type3_index1_Manufacture
# ./OsTools spi -c 0x4000 -C -s 3 2 type3_index2_Version
# ./OsTools spi -c 0x4000 -C -s 3 3 type3_index3_SerialNumber
# ./OsTools spi -c 0x4000 -C -s 3 4 type3_index4_AssetTag
# ./OsTools spi -c 0x4000 -C -s 3 5 type3_index5_SKUNumber