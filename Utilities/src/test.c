// (c)2012 Jannik Vogel

// Compile using: reset && clear && clang backup.c /usr/lib32/crt1.o /usr/lib32/crti.o /usr/lib32/crtn.o -m32 -nostdlib -O0 -g -lgcc_s -lc -o backup && gdb ./backup save
// Fixed multilib: clang -m32 -O0 -g test.c -o /mnt/Temporary/tmp/test



#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stropts.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>




#include "../api/eeprom.h"
#include "../api/baseboard.h"
#include "../api/files.h"

static void printBytes(const void* address, int size) {
  const uint8_t* b = address; 
  printf("{");
  unsigned int i;
  for(i = 0; i < size; i++) {
    printf(" 0x%02X",b[i]);
  }
  printf(" }");
  return;
}

/*

  This file contains necessary tests to find out more about SEGAs custom interfaces

*/

void filesTest(void) {
  printf("Creating file tree from /dev/\n");
  system("ls /dev/ -R -1 -l -a > devfiles.txt");
  printf("Creating file tree from /etc/\n");
  system("ls /etc/ -R -1 -l -a > etcfiles.txt");
  return;
}

void sysinfoTest(void) {
  printf("Read sysinfo!\n");  

  openBaseboard();

  // Request data
  
  uint32_t data[64] = {0};
  data[0] = 0x120;
  data[1] = 0x1000;
  data[2] = 0x400;
  int ret = ioctl(lbb,0xC020BC06,data);
  printf("Returned %i\n",ret);

  // Read data

  int f = openForOutput("sysinfo.bin");
  printf("Size: %i, result: %i\n",data[2],f);
  readShmToFile(f,data[1],data[2]);
  close(f);

  closeBaseboard();

  return;
}

unsigned int getSize(int(*readf)(void* buffer, unsigned int offset, unsigned int size), int(*writef)(unsigned int offset, void* buffer, unsigned int size)) {
  unsigned int addr = 1;
  uint8_t zero; uint8_t oldzero;
  uint8_t test; uint8_t oldtest;

  // Get the lowest byte

  readf(&oldzero,0,1);

  while(1) {

    int ret;

    // Read the address to test
  
    if (readf(&test,addr,1) < 0) {
      printf("Read was rejected!\n");
      break;
    } 
    printf("Read 0x%02X from %i\n",test,addr);    

    // Flip its value    

    test ^= 0xFF;
    ret = writef(addr,&test,1);
    printf("Wrote 0x%02X to %i, %i\n",test,addr,ret);    
    usleep(1000*100);

    // Remember the new value and also remember the current value at the lowest byte

    ret = readf(&oldtest,addr,1);
    printf("Read 0x%02X from %i, %i\n",oldtest,addr,ret);    
    readf(&zero,0,1);

    // Flip it back  

    test ^= 0xFF;
    writef(addr,&test,1);
    printf("Wrote 0x%02X to %i\n",test,addr);  
  
    // Check if it holds it value

    test ^= 0xFF;
    if (test != oldtest) {
      printf("Memory didn't hold value, 0x%02X != 0x%02X\n", test, oldtest);
      break;
    }

    // Make sure that the lowest byte wasn't changed either

    if (zero != oldzero) {
      printf("Memory wrapped!\n");
      break;
    }

    addr++;
  }
  return addr;
}

void sramTest(void) {
  openBaseboard();
  printf("SRAM Test: ");
  unsigned int s = getSize(readSram,writeSram);
  printf("SRAM seems to be %i bytes\n",s);
  closeBaseboard();
  return;
}

void eepromTest(void) {
  openI2c();
  printf("EEPROM Test: ");
  unsigned int s = getSize(readEeprom,writeEeprom);
  printf("EEPROM seems to be %i bytes\n",s);
//  readEepromToFile();
  closeI2c();
  return;
}

void sendRecvJvs(void* src, size_t srcSize, bool ack, void* dst, size_t* dstSize) {
  if (ack) {

    uint32_t data[64] = {0};

    writeShm(0x1000,src,srcSize);

    {    
      data[0] = 0x220;
      data[1] = 0x1000;
      data[2] = srcSize;
      data[3] = 0x1100;
      data[4] = 0x100;
      data[5] = 0;
      data[6] = 0;
      int ret = ioctl(lbb,0xC020BC06,data);
      printf("Returned %i\n",ret);
      printf("data[0] - ack + command: %X\n",data[0]);
      printf("data[1] - src: %X\n",data[1]);
      printf("data[2] - src-size: %X\n",data[2]);
      printf("data[3] - dst: %X\n",data[3]);
      printf("data[4] - dst-size: %X\n",data[4]);
      printf("data[5]: %X\n",data[5]);
      printf("data[6]: %X\n",data[6]);
    }

    do {

      usleep(1000);

      printf("Got ack?!\n");
      memset(&data[1],0xAA,30);
      int ret = ioctl(lbb,0xC020BC07,data);
      printf("data[0] - ack? + command: %X\n",data[0]);
      printf("data[1] - status: %X\n",data[1]);
      printf("data[2] - addr: %X\n",data[2]);
      printf("data[3] - size: %X\n",data[3]);
      printf("data[4] - buffer-size: %X\n",data[4]);
      printf("data[5]: %X\n",data[5]);
      printf("data[6]: %X\n",data[6]);
      printf("\n");
    } while(!(data[0] & 0x80008000));
  usleep(100*100);

    if (dstSize) {
      *dstSize = data[3];
      readShm(dst,data[2],*dstSize);
    }

  }
}

void jvsTest(void) {
  int i;

  int lbb = openBaseboard();

  // Check the weird getVersion command

  {
    uint8_t data[64];

    ioctl(lbb,0x8004BC02,data);

    printf("Version bytes: ");
    for(i = 0; i < 4; i++) {
      printf(" 0x%02X",data[i]);
    }
    printf("\n");
  }

  // Maybe some sort of init?

  ioctl(lbb,0x300,0);
  printf("Sent the weird 0x300 cmd\n");
  
  // Request data

  uint8_t _reset[] = { 0xE0, 0xFF, 0x03, 0xF0, 0xD9, 0xCB };
  uint8_t _address1[] = { 0xE0, 0xFF, 0x03, 0xF1, 0x01, 0xF4 };
  uint8_t _input1[] = { 0xE0, 0x01, 0x04, 0x20, 0x02, 0x02, 0x29 };

  // Actual fake protocol :P
  
  {
    
    // Reset all I/O Slaves

    sendRecvJvs(_reset,sizeof(_reset),true,NULL,NULL);
    usleep(1000*1000);
    sendRecvJvs(_reset,sizeof(_reset),true,NULL,NULL);
    usleep(1000*2000);
    printf("Reset!\n");

    int j = 1;
    while(1) {

      printf("\n\n\n\n"); printf("\n\n\n\n");
      printf("Getting sense\n");

      uint32_t data[64] = {0};
      data[0] = 0x210;
      int ret = ioctl(lbb,0xC020BC06,data);
      usleep(1000*100);
      ret = ioctl(lbb,0xC020BC07,data);
      printf("data[0] - status?: %X\n",data[0]);
      printf("data[1] - unk0: %X\n",data[1]);
      printf("data[2] - unk1: %X\n",data[2]);
      printf("data[3]: %X\n",data[3]);
      printf("data[4]: %X\n",data[4]);
      printf("data[5]: %X\n",data[5]);
      printf("data[6]: %X\n",data[6]);

      if (data[2] == 1) {
        break;
      }

      printf("\n\n\n\n"); printf("\n\n\n\n");

      // Send out one address

      printf("Requesting address\n");

      size_t s;
      uint8_t buf[100];
      memset(buf,0xBB,100);
      sendRecvJvs(_address1,sizeof(_address1),true,buf,&s);
      printf("Size: %i\n",s);
      for(i = 0; i < s; i++) {
        printf(" 0x%02X",buf[i]);
      }
      printf("\n");
    
      printf("Got acknowledge maybe?\n");
      
      j++;

    }

    printf("Got %i jvs nodes!\n",j);

    while(1) {
      size_t s;
      uint8_t buf[100];
      memset(buf,0xBB,100);
      sendRecvJvs(_input1,sizeof(_input1),true,buf,&s);
      printf("Size: %i\n",s);
      for(i = 0; i < s; i++) {
        printf(" 0x%02X",buf[i]);
      }
      printf("\n");
      usleep(1000*100);
    }

  }

  // Be nice, close baseboard again

  closeBaseboard();
  return;
}

int main(int argc, char* argv[]) {

  if (argc < 2) {
    printf("%s <test1> [<test2> ...]\n",argv[0]);
    printf("Valid tests: sysinfo, eeprom, sram, files, jvs\n");
    return 1;
  }

  int i;
  for(i = 1; i < argc; i++) {

    printf("\n\nTest %i:\n\n\n",i);

    const char* test = argv[i];

    //TODO: Macro for this?
    if (!strcmp(test,"sysinfo")) { sysinfoTest(); }
    if (!strcmp(test,"sram")) { sramTest(); }
    if (!strcmp(test,"files")) { filesTest(); }
    if (!strcmp(test,"eeprom")) { eepromTest(); }
    if (!strcmp(test,"jvs")) { jvsTest(); }
  
  }
  return 0;
}
