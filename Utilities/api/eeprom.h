#include <unistd.h>
#include <stdlib.h>

#include "smbus.h"

int eepromSeek(uint16_t address) {
  return i2c_smbus_write_byte_data(i2c, (address >> 8) & 0xFF, address & 0xFF);
}

int eepromRead(void) {
  return i2c_smbus_read_byte(i2c);
}

int eepromWrite(uint16_t address, uint8_t value) {
  return i2c_smbus_write_word_data(i2c, (address >> 8) & 0xFF, (value << 8) | (address & 0xFF));
}

int eepromWritePage(uint16_t address, const uint8_t* buffer, unsigned int size) {
  uint8_t* bytes = alloca(size + 1);
  bytes[0] = address & 0xFF;
  memcpy(&bytes[1],buffer,size);
  return i2c_smbus_write_block_data(i2c, (address >> 8) & 0xFF, size + 1, bytes);
}

int readEeprom(void* buffer, unsigned int offset, unsigned int size) {
  uint8_t* bytes = buffer;  
  unsigned int i;
  int ret;
  // Initial seek
  ret = eepromSeek(offset);
  if (ret < 0) {
    printf("EEPROM seek error! (Offset: 0x%X)\n",offset);
    return ret;
  }
  // Now the main loop
  for(i = 0; i < size; i++) {
    ret = eepromRead();
    if (ret >= 0) {
      bytes[i] = ret;
    } else {
      static unsigned int errors = 0;
      printf("Read error at 0x%X!\n",offset+i);
      if (errors > (size / 10)) {
        printf("Too many read errors!\n");
        return ret;
      }
      i--; // Attempt the same address again
      usleep(10*1000); // Wait 10ms to loosen the bus stress
    }
  }
  return ret;
}

void readEepromToFile(int fd, unsigned int offset, unsigned int size) {
  void* buffer = malloc(size);
  readEeprom(buffer,offset,size);
//lseek(fd,offset,SEEK_SET);
  lseek(fd,0,SEEK_SET);
  write(fd,buffer,size);
  free(buffer);  
  return;
}

int writeEeprom(unsigned int offset, const void* buffer, unsigned int size) {
  const unsigned int pageSize = 32; //NOTE: From datasheet
  const uint8_t* bytes = buffer;  
  unsigned int i;
  int ret;
  for(i = 0; i < size; i++) {
    uint16_t address = offset + i;
    // Attempt a page write
    //NOTE: Due to a kernel error you can only write more than 31 bytes per page, and actually, you can't write less either!
    //      Very dangerous!
    //      (Source: http://lists.lm-sensors.org/pipermail/i2c/2007-May/001368.html )
    if (((address % pageSize) == 0) && ((size - i) >= 31) && (pageSize > 31)) {
      //printf("0x%X: Page write..\n",address);
      ret = eepromWritePage(address, &bytes[i], 31);
      if (ret >= 0) {
        //printf("OK!\n");
        i += 31;
        i--; // Revert auto increment!
      }
    } else {
      //printf("0x%X: Single byte write\n",address);
      ret = eepromWrite(address,bytes[i]);
    }
    if (ret < 0) {
      static unsigned int errors = 0;
      printf("Write error at 0x%X: %i!\n",address,ret);
      if (errors > (size / 10)) {
        printf("Too many write errors!\n");
        return ret;
      }
      i--; // Attempt the same address again
      usleep(10*1000); // Wait 10ms to loosen the bus stress
    }
    usleep(3*1000); // Wait 3ms to loosen the bus stress - without this, the write will almost always fail..
                    // 2ms would also work, but I had a high error rate which seemed to risky to me
  }
  return ret;
}

void writeEepromFromFile(unsigned int offset, int fd, unsigned int size) {
  void* buffer = malloc(size);
//  lseek(fd,offset,SEEK_SET);
  lseek(fd,0,SEEK_SET);
  read(fd,buffer,size);
  writeEeprom(offset,buffer,size);
  free(buffer);
  return;
}

