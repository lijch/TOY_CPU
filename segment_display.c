#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "rom_utils.h"

#define MAX_ADDRESS 1024
#define ONE_PREFIX (0 << 8)
#define TEN_PREFIX (1 << 8)
#define HUN_PREFIX (2 << 8)

static uint8_t DISPLAY_LOGIC[] = {
  0x7e,
  0x12,
  0xbc,
  0xb6,
  0xd2,
  0xe6,
  0xee,
  0x72,
  0xfe,
  0xf6
};

int main() {
  int fd1;
  int fd2;
  int fd3;
  uint32_t data1[256];
  uint32_t data2[256];
  uint32_t data3[256];
  uint8_t one;
  uint8_t ten;
  uint8_t hundred;
  uint32_t address;

  fd1 = open("./segment_display1.data", O_CREAT | O_TRUNC | O_RDWR, S_IRWXU);
  if (fd1 < 0) {
    printf("Failed to open the segment display file:%d, %d.\n", fd1, errno);
    return -1;
  }

  fd2 = open("./segment_display2.data", O_CREAT | O_TRUNC | O_RDWR, S_IRWXU);
  if (fd2 < 0) {
    printf("Failed to open the segment display file:%d, %d.\n", fd2, errno);
    return -1;
  }

  fd3 = open("./segment_display3.data", O_CREAT | O_TRUNC | O_RDWR, S_IRWXU);
  if (fd3 < 0) {
    printf("Failed to open the segment display file:%d, %d.\n", fd3, errno);
    return -1;
  }

  memset(data1, 0, 256);
  memset(data2, 0, 256);
  memset(data3, 0, 256);
  for(int i = 0; i < 256; i++) {
    one = i % 10;
    ten = (i / 10) % 10;
    hundred = i / 100;
    data1[i] = DISPLAY_LOGIC[one];
    data2[i] = DISPLAY_LOGIC[ten];
    data3[i] = DISPLAY_LOGIC[hundred];
  }

  write_rom_data(fd1, data1, 256);
  //write_rom_data(fd1, data2, 256);
  //write_rom_data(fd1, data3, 256);

  close(fd1);
  close(fd2);
  close(fd3);

  return 0;
}
