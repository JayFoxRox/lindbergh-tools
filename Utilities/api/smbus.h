#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <stropts.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>

// SMBUS access via I2C

static int i2c = -1;

void closeI2c(void) {
  if (i2c >= 0) {
    close(i2c);
    i2c = -1;
  }
  return;
}


void openI2c(void) {
  i2c = open("/dev/i2c/0",O_RDWR);
  if (i2c < 0) {
    printf("Failed to access i2c!\n");
    exit(1);
  }
  ioctl(i2c, I2C_SLAVE, 0x57);
  atexit(closeI2c);
  return;
}

int32_t i2c_smbus_xfer(int fd,
                    char read_write, uint8_t command, int protocol,
                   union i2c_smbus_data *data) {


    struct i2c_smbus_ioctl_data _data;
_data.read_write = read_write;
_data.command = command;
_data.size = protocol;
_data.data = data;

return ioctl(fd,I2C_SMBUS,&_data);

}

int32_t i2c_smbus_write_byte_data(int fd, uint8_t command, uint8_t value)
{
       union i2c_smbus_data data;
        data.byte = value;
         return i2c_smbus_xfer(fd,
                               I2C_SMBUS_WRITE,command,
                               I2C_SMBUS_BYTE_DATA,&data);
}

int32_t i2c_smbus_read_byte(int fd)
 {
         union i2c_smbus_data data;
         int status;
 
         status = i2c_smbus_xfer(fd,
                                 I2C_SMBUS_READ, 0,
                                 I2C_SMBUS_BYTE, &data);
         return (status < 0) ? status : data.byte;
 }

int32_t i2c_smbus_write_word_data(int fd, uint8_t command, uint16_t value)
 {
         union i2c_smbus_data data;
         data.word = value;
         return i2c_smbus_xfer(fd,
                               I2C_SMBUS_WRITE,command,
                               I2C_SMBUS_WORD_DATA,&data);
}

int32_t i2c_smbus_write_block_data(int fd, uint8_t command,
                               uint8_t length, const uint8_t *values)
{
        union i2c_smbus_data data;

/*
        if (length > I2C_SMBUS_BLOCK_MAX)
                length = I2C_SMBUS_BLOCK_MAX;
*/

        if (length != 32) {
          printf("WARNING: Error in kernel prevents this call from working properly!\n");
          length = 32;
        }
        data.block[0] = length;
        memcpy(&data.block[1], values, length);
        return i2c_smbus_xfer(fd,
                              I2C_SMBUS_WRITE,command,
                              I2C_SMBUS_I2C_BLOCK_BROKEN,&data); //NOTE: We use the broken version which only supports 32 byte writes!
}
