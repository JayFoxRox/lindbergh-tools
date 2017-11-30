#!/bin/bash

# (c)2012 Jannik Vogel
#
# Outrun 2 - EEPROM: { 0x1000, 0x76 } SRAM: { 0x010000, 0x021700 }
#
# From this the following save regions were extracted:
# 
# EEPROM: { 0x1000, 0x1000 } (every game seems to use this)
# SRAM: { 0x00C000, 0x026000 } (a bit more than what outrun uses, aligned)
#

echo "Goin to start $1 operation on $2"

# Check if the operation is valid and do the SRAM stuff
if [ "$1" == "load" ]; then
  # We only flash parts of it to make sure that we don't overwrite anything critical
  ./backup "$1" sram "$2-sram_00C000-03FFFF.bin" 0x00C000 0x026000
elif [ "$1" == "save" ]; then
  # We dump the entire chip section though - nothin to loose here because this is really fast
  ./backup "$1" sram "$2-sram_000000-00BFFF.bin" 0x000000 0x00C000
  # Various errors when reading beyond than 0x40000, to still read full SRAM use a size of 0x1F4000
  ./backup "$1" sram "$2-sram_00C000-03FFFF.bin" 0x00C000 0x034000
# The following line is slow, so we should avoid reading that region - it's probably not neccessary anyway (SEGABOOT region)
#  ./backup "$1" eeprom "$2-eeprom_0000-0FFF.bin" 0x0000 0x1000
else
  echo "Unknown operation"
  exit 1
fi

# We are rather sure about the EEPROM region, the lower half seems to be used by the OS only
./backup "$1" eeprom "$2-eeprom_1000-1FFF.bin" 0x1000 0x1000

exit 0

