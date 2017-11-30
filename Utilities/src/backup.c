// backup <mode> <device> <file> <address> <size>
// Examples: backup load sram sbgx-sram1.bin 0x0C000 0x20000
//           backup save eeprom sbgx-eeprom1.bin 0x1000 0x100
//
// SRAM seems to be 2048kiB, most games store their stuff at 0x0C000 with 0x20000 bytes (2 blocks, each 0x10000).
// Some games also store their stuff at 0x10000 with an unknown size.
// I believe it should work to backup 0x0C000 with a size of 0x24000 to cover most games
// Reads zero beyond region
//
// EEPROM seems to be 8kiB, used from 0x1000 on with 0x100 bytes in most games (2 blocks, each 0x80)
//  backup save eeprom eeprom.bin 0x1000 0x1000
//  backup load eeprom eeprom.bin 0x1000 0x1000
// Reads mirror beyond region

#include <stdio.h>
#include <string.h>

#include "../api/eeprom.h"
#include "../api/baseboard.h"
#include "../api/files.h"

void cleanupBaseboard(void) {
  closeBaseboard();
}

void cleanupI2c(void) {
  closeI2c();
}

unsigned int readValue(const char* str) {

  unsigned int value;

/*
  if (sscanf(str," 0x%X ",&value) == 1) { // Hex
    return value;
  }
*/
  char* hex;
  if ((hex = strstr(str,"0x"))) {
    return strtoul(&hex[2],NULL,16);
  }

  return atoi(str);
/*
  if (sscanf(str," %i ",&value) == 1) { // Dec
    return value;
  }
*/
  printf("invalid value (%s)\n",str);
  exit(1);  

}

int main(int argc, char* argv[]) {

  if (argc != 6) {
    printf("invalid arguments (%i)\n",argc);
    return 1;
  }

  const char* mode = argv[1];
  const char* device = argv[2];
  const char* file = argv[3];

  unsigned int offset = readValue(argv[4]);
  unsigned int size = readValue(argv[5]);

  printf("0x%X to 0x%X (0x%X bytes)\n",offset,offset+size-1,size);

  void(*writeFunction)(unsigned int offset, int fd, unsigned int size);
  void(*readFunction)(int fd, unsigned int offset, unsigned int size);

  if (!strcmp(device,"sram")) {

    writeFunction = writeSramFromFile;
    readFunction = readSramToFile;

    openBaseboard();
    atexit(cleanupBaseboard);

  } else if (!strcmp(device,"eeprom")) {

    writeFunction = writeEepromFromFile;
    readFunction = readEepromToFile;

    openI2c();
    atexit(cleanupI2c);

  } else {

    printf("invalid device (%s)\n",device);
    return 1;

  }

  if (!strcmp(mode,"load")) {

    int f = openForInput(file);
    if (f < 0) { printf("invalid source file (%s)\n",file); return 1; }
    writeFunction(offset,f,size);    
    close(f);  
  
    return 0;

  }

  if (!strcmp(mode,"save")) {

    int f = openForOutput(file);
    if (f < 0) { printf("invalid destination file (%s)\n",file); return 1; }
    readFunction(f,offset,size);    
    close(f);  
  
    return 0;

  }


  printf("invalid mode (%s)\n",mode);
  return 1;

}
