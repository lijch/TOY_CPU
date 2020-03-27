#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "rom_utils.h"

#define MAX_CNT 0x300

static uint32_t instruction[MAX_CNT] = {
  0x0310,0x200, //DATA R1
  0x0320,0x201, //DATA R2
  0x0330,0x202, //DATA R3
  0x0340,0x203, //DATA R4
  0x0010,0x204, //LDI R1
  0x0221, //STR R2, R1
  0x0131, //LDR R3, R1
  0x0721, //MOV R2, R1
  0x0823, //ADD R2, R3
  0x0922, //SUB R2, R2
  0x0A11, //SHL R1, R1
  0x0B12, //SHR R1, R2
  0x0C30, //NOT R3
  0x0D23, //AND R2, R3
  0x0E23, //OR R2, R3
  0x0F13, //XOR R1, R3
  0x0010, 0x19, //LDR R1, 0x19
  0x0410, //JMPR R1
  0x0600, //HLT
  0x0500, 0x1c, //JMPA 0x1c
  0x0600, //HLT
  0x0010, 0x3, //LDI R1, 0x3
  0x0020, 0x4, //LDI R2, 0x4
  0x0912, //SUB R1, R2
  0x0502, 0x1b, //JZ 0x1B
  0x0911, //SUB R1, R1
  0x0502, 0x27, //JZ 0x27
  0x0600, //HLT
  0x0020, 0xff00, //LDI R2, 0xff00
  0x0030, 0x04, //LDR R3, 0x04
  0x0832, //ADD R3, R2
  0x0501, 0x26, //JC 0x26
  0x0832, // ADD R3, R2
  0x0501, 0x32, //JC 0x31
  0x0600, //HTL
  0x0812, //ADD R0, R1
  0x0080, 0x18, //LDR R8, 0x18
  0x1000, 0x208, //BL 0x208
  0x0020, 0x30, //LDI R2, 0x30
  0x1220, // OUTPUT R2
  0x0600 //HLT
};

int main() {
  int fd;
  uint32_t *rom_data;

  fd = open("./instruction.data", O_CREAT | O_TRUNC | O_RDWR, S_IRWXU);
  if (fd < 0) {
    printf("Failed to open the micro instruction file:%d, %d.\n", fd, errno);
    return -1;
  }

  instruction[0x200] = 3;
  instruction[0x201] = 4;
  instruction[0x202] = 5;
  instruction[0x203] = 6;
  instruction[0x208] = 0x0812; //ADD R1, R2;
  instruction[0x209] = 0x1100; //RET
  write_rom_data(fd, instruction, MAX_CNT);
  close(fd);

  return 0;
}
