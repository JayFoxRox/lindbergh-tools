// jvs <command> [<arguments>]
// Examples: jvs reset
//           jvs digital 1 1
//           jvs analog 8 1 1
//           jvs analog 16 1 1
//
// reset takes 0 arguments and returns the number of connected nodes
// digital takes 2 or more arguments, the node and the input bits (an argument each), returns the state as integer bit
// analog8 takes 3 arguments, input width in bit (should be 8 or 16), the node and the input channel, returns the state as float from 0.0 to 1.0
//

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "../api/baseboard.h"

void cleanup(void) {
  closeBaseboard();
}

bool waitForAcknowledge(uint32_t* data) {
  unsigned int timeout = 0;
  do {

    usleep(100);
/*
    printf("Got ack?!\n");
    memset(&data[1],0xAA,30);
*/
    int ret = ioctl(lbb,0xC020BC07,data);
/*
    printf("data[0] - ack? + command: %X\n",data[0]);
    printf("data[1] - status: %X\n",data[1]);
    printf("data[2] - addr: %X\n",data[2]);
    printf("data[3] - size: %X\n",data[3]);
    printf("data[4] - buffer-size: %X\n",data[4]);
    printf("data[5]: %X\n",data[5]);
    printf("data[6]: %X\n",data[6]);
    printf("\n");
*/

//FIXME: Timeouts were never tested!
#if 0
    if (timeout-- == 0) { return false; }
#endif

  } while(!(data[0] & 0x80008000));
  return true;
}

bool transfer(unsigned int lbb, void* src, size_t srcSize, void* dst, size_t* dstSize) {

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
/*
    printf("Returned %i\n",ret);
    printf("data[0] - ack + command: %X\n",data[0]);
    printf("data[1] - src: %X\n",data[1]);
    printf("data[2] - src-size: %X\n",data[2]);
    printf("data[3] - dst: %X\n",data[3]);
    printf("data[4] - dst-size: %X\n",data[4]);
    printf("data[5]: %X\n",data[5]);
    printf("data[6]: %X\n",data[6]);
*/
  }

  if (!waitForAcknowledge(data)) {
    return false;
  }

  if (dstSize) {
    *dstSize = data[3];
    readShm(dst,data[2],*dstSize);
  }

  return true;

}

unsigned int getSense(unsigned int lbb) {

/*
  printf("\n\n\n\n"); printf("\n\n\n\n");
  printf("Getting sense\n");
*/

  uint32_t data[64] = {0};
  data[0] = 0x210;
  int ret = ioctl(lbb,0xC020BC06,data);
  waitForAcknowledge(data);
/*
  printf("data[0] - status?: %X\n",data[0]);
  printf("data[1] - unk0: %X\n",data[1]);
  printf("data[2] - unk1: %X\n",data[2]);
  printf("data[3]: %X\n",data[3]);
  printf("data[4]: %X\n",data[4]);
  printf("data[5]: %X\n",data[5]);
  printf("data[6]: %X\n",data[6]);
*/
  return data[2];
}

unsigned int jvsReset(unsigned int lbb) {

  // Request data

  uint8_t reset[] = { 0xE0, 0xFF, 0x03, 0xF0, 0xD9, 0xCB };
  
  // Reset all I/O Slaves

  transfer(lbb,reset,sizeof(reset),NULL,NULL);
  usleep(1000*1000);
  transfer(lbb,reset,sizeof(reset),NULL,NULL);
  usleep(1000*1000);
  
  unsigned int nodes = 0;
  while(1) {

    // Check if the sense changed

    unsigned int sense = getSense(lbb);

    if (sense == 1) { // Address assigned
      break;
    }

    if (sense == 2) { // No I/O found [sense floating?]
      break;
    }

    if (sense == 3) { // No address yet

      // Node count up

      nodes++;

      // Send out one address

      uint8_t buffer[0x100];
      size_t size;
      
      uint8_t address[] = { 0xE0, 0xFF, 0x03, 0xF1, nodes, 0xF3+nodes };
      transfer(lbb, address,sizeof(address),buffer,&size);

    }

  }

  return nodes;

}

int main(int argc, char* argv[]) {
  
  if (argc < 2) {
    printf("invalid arguments (%i)\n",argc);
    return 1;
  }

  const char* command = argv[1];

  unsigned int lbb = openBaseboard();
  atexit(cleanup);

  if (!strcmp(command,"reset")) {
  
    if (argc != 2) { goto argumentError; }

    // Reset and return node count

    unsigned int nodes = jvsReset(lbb);
    printf("%i",nodes);
    return 0;

  }

  uint8_t buffer[0x100];
  size_t size;

  if (!strcmp(command,"digital")) {

    if (argc < 4) { goto argumentError; }
    unsigned int node = atoi(argv[2]);
    
    // Request data via jvs

    uint8_t input[] = { 0xE0, node, 0x04, 0x20, 0x02, 0x02, 0x28+node };
    transfer(lbb, input, sizeof(input), buffer, &size);

    // Get the input bit

    int i;
    for(i = 3; i < argc; i++) {
      unsigned int bit = atoi(argv[i]);

      uint8_t* buttons = &buffer[5];
      printf("%i",(buttons[bit / 8] & (1 << (bit % 8)))?1:0);
    }
    return 0;

  }
  if (!strcmp(command,"analog")) {

    if (argc != 5) { goto argumentError; }
    unsigned int width = atoi(argv[2]); 
    unsigned int node = atoi(argv[3]);
    unsigned int channel = atoi(argv[4]); 

    // Extend channel count if the channel number is too high
  
    unsigned int channels = 2;
    if (channel > channels) {
      channels = channel;
    }
    
    // Request data via jvs

    uint8_t input[] = { 0xE0, node, 0x04, 0x22, channels, 0x26+channels+node };
    transfer(lbb, input, sizeof(input), buffer, &size);

    // Get the input bit

    uint8_t* values = &buffer[5];
    uint16_t maximum = (1 << width) - 1;

    // Now find the channel value

    uint16_t value = (values[2*channel+0] << 8) | values[2*channel+1]; 

    // Data output

    printf("%f", value / (float)maximum);
    return 0;

  }

  printf("invalid command (%s)\n",command);
  return 1;

  argumentError:

  printf("invalid arguments (%i)\n",argc);
  return 1;

}


