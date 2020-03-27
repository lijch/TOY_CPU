#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "rom_utils.h"

#define IR_IE	(1 << 0)
#define MEM_AE (1 << 1)
#define MEM_IE	(1 << 2)
#define MEM_OE	(1 << 3)
#define PC_CE	(1 << 4)
#define PC_OE	(1 << 5)
#define PC_JUMP	(1 << 6)
#define ALU_AI	(1 << 7)
#define ALU_BI	(1 << 8)
#define ALU_O	(1 << 9)
#define ALU_S1	(1 << 10)
#define ALU_S2	(1 << 11)
#define ALU_S3	(1 << 12)
#define SERIAL_E (1 << 13)
#define HLT (1 << 14)
#define RX_S_SHIFT 28
#define RX_E_SHIFT 24

#define FLAG_REG 12
#define LR_REG 9

#define bool uint8_t

#define LDI 0x0
#define LOAD 0x1
#define STORE 0x2
#define DATA 0x3
#define JMP_R 0x4
#define JMP_A 0x5
#define MOV 0x7
#define ADD 0x8
#define SUB 0x9
#define SHL 0xA
#define SHR 0xB
#define NOT 0xC
#define AND 0xD
#define OR 0xE
#define XOR 0xF
#define BL 0x10
#define RET 0x11
#define OUTPUT 0x12
#define CMP 0x13

#define ZFLAG 2
#define CFLAG 1

#define false 0
#define true 1

static bool do_print = false;

uint32_t MICRO_INSTRUCTION_STEP_0_2[] = {
  MEM_AE | PC_OE, // 0x48
  MEM_OE | IR_IE, // 0x11
  PC_CE //0x20
};

uint32_t MICRO_INSTRUCTION[256][5] = {
  {
    MEM_AE | PC_OE,
    MEM_OE,
    PC_CE
  }, // LDI
  {
    MEM_AE,
    MEM_OE
  }, // LOAD
  {
    MEM_AE,
    MEM_IE,
  }, // STORE
  {
    MEM_AE | PC_OE,
    MEM_OE,
    MEM_AE,
    MEM_OE,
    PC_CE
  }, // DATA
  {
    PC_JUMP,
  }, // JMP_R
  {
    MEM_AE | PC_OE,
    MEM_OE | PC_JUMP,
  }, // JMP_A
  {
    HLT
  }, // HLT
  {
    0
  }, // MOV
  {
    ALU_AI,
    ALU_BI | (FLAG_REG << RX_S_SHIFT),
    ALU_O
  }, // ADD
  {
    ALU_AI,
    ALU_BI | ALU_S1 | ALU_S2 | ALU_S3 | (FLAG_REG << RX_S_SHIFT),
    ALU_O | ALU_S1 | ALU_S2 | ALU_S3
  }, // SUB
  {
    ALU_AI,
    ALU_BI,
    ALU_O | ALU_S1 | ALU_S3
  }, // SHL
  {
    ALU_AI,
    ALU_BI,
    ALU_O  | ALU_S1 | ALU_S2
  }, // SHR
  {
    ALU_AI,
    ALU_O  | ALU_S3
  }, // NOT
  {
    ALU_AI,
    ALU_BI,
    ALU_O  | ALU_S2
  }, // AND
  {
    ALU_AI,
    ALU_BI,
    ALU_O | ALU_S2 | ALU_S3
  }, // OR
  {
    ALU_AI,
    ALU_BI,
    ALU_O | ALU_S1
  }, // XOR
  {
    PC_OE | MEM_AE,
    PC_CE,
    PC_OE | (LR_REG << RX_S_SHIFT),
    MEM_OE | PC_JUMP
  }, // BL
  {
    PC_JUMP | (LR_REG << RX_E_SHIFT)
  }, // RET
  {
    SERIAL_E
  }, // OUTPUT
  {
    ALU_AI,
    ALU_BI | ALU_S1 | ALU_S2 | ALU_S3 | (FLAG_REG << RX_S_SHIFT),
  } // CMP
};

static uint32_t get_register_x(uint32_t operator, bool output) {
  uint32_t register_inx = (operator >> 4) & 0xf;
  if (output) {
    return register_inx << RX_E_SHIFT;
  } else {
    return register_inx << RX_S_SHIFT;
  }
}

static uint32_t get_register_y(uint32_t operator, bool output) {
  uint32_t register_inx = operator & 0xf;
  if (do_print) {
    printf("####register y indx:%d\n", register_inx);
  }
  if (output) {
    return register_inx << RX_E_SHIFT;
  } else {
    return register_inx << RX_S_SHIFT;
  }
}

static void fix_register(uint32_t *data, uint32_t instruction, uint32_t step, uint32_t operator, uint32_t flags) {
  uint32_t register_x_in = get_register_x(operator, false);
  uint32_t register_x_out = get_register_x(operator, true);
  uint32_t register_y_in = get_register_y(operator, false);
  uint32_t register_y_out = get_register_y(operator, true);

  uint32_t alu_ins[7] = {
    0,
    0,
    0,
    register_x_out,
    register_y_out,
    register_x_in,
    0
  };

  if (do_print) {
    printf("#####instruction:%d, operator:%d, step:%d\n", instruction, operator, step);
    printf("####x:%x,%x, y:%x,%x\n", register_x_in, register_x_out, register_y_in, register_y_out);
  }

  switch(instruction) {
  case LDI:
    if (step == 4) {
      *data = *data | register_x_in;
    }
    break;
  case LOAD:
    if (step == 3) {
      *data = *data | register_y_out;
    }
    if (step == 4) {
      *data = *data | register_x_in;
    }
    break;
  case STORE:
    if (step == 3) {
      *data = *data | register_y_out;
    }
    if (step == 4) {
      *data = *data | register_x_out;
    }
    break;
  case DATA:
    if (step == 4) {
      *data = *data | register_x_in;
    }
    if (step == 5) {
      *data = *data | register_x_out;
    }
    if (step == 6) {
      *data = *data | register_x_in;
    }
    break;
  case JMP_R:
    if (step == 3) {
      *data = *data | register_x_out;
    }
    break;
  case JMP_A:
    if(operator == 0) {
      break;
    }
    if (operator & ZFLAG) { //JZ
      if (!(flags & 0x2)) {
        *data = 0;
        if (step == 3) {
          *data = *data | PC_CE;
        }
      }
      break;
    }
    if (operator & CFLAG) { //JC
      if (!(flags & 0x1)) {
        *data = 0;
        if (step == 3) {
          *data = *data | PC_CE;
        }
      }
      break;
    }
    break;
  case MOV:
    if(step == 3) {
      *data = *data | register_y_out | register_x_in;
    }
    break;
  case ADD:
    *data = *data | alu_ins[step];
    break;
  case SUB:
    *data = *data | alu_ins[step];
    break;
  case SHL:
    *data = *data | alu_ins[step];
    break;
  case SHR:
    *data = *data | alu_ins[step];
    break;
  case NOT:
    if (step == 3) {
      *data = *data | register_x_out;
    }
    if (step == 4) {
      *data = *data | register_x_in;
    }
    break;
  case AND:
    *data = *data | alu_ins[step];
    break;
  case OR:
    *data = *data | alu_ins[step];
    break;
  case XOR:
    *data = *data | alu_ins[step];
    break;
  case OUTPUT:
    if (step == 3) {
      *data = *data | register_x_out;
    }
    break;
  case CMP:
    if (step == 3 || step == 4) {
      *data = *data | alu_ins[step];
    }
  }
}

int main() {
  int fd;
  uint32_t instruction;
  uint32_t step;
  uint32_t data;
  uint32_t operator;
  uint32_t flags;
  uint32_t max_address = 1 << 21;
  uint32_t *rom_data;

  fd = open("./micro_instruction.data", O_CREAT | O_TRUNC | O_RDWR, S_IRWXU);
  if (fd < 0) {
    printf("Failed to open the micro instruction file:%d, %d.\n", fd, errno);
    return -1;
  }

  rom_data = malloc(max_address * sizeof(uint32_t));
  if (rom_data == NULL) {
    printf("Failed to alloc memory for rom data.");
    return -1;
  }

  for(int i = 0; i < max_address; i++) {
    instruction = i >> 13 & 0xff;
    operator = (i >> 5) & 0xff;
    step = (i >> 2) & 0x7;
    flags = i & 0x3;
    /* if (i == 0x20010) { */
    /*   printf("###########data:%x, operator:%d\n", data, operator); */
    /*   do_print = true; */
    /* } */
    if(step < 3) {
      data = MICRO_INSTRUCTION_STEP_0_2[step];
    } else {
      data = MICRO_INSTRUCTION[instruction][step - 3];
      if (do_print) {
        printf("###########step:%d, data:%x\n", step, data);
      }
      fix_register(&data, instruction, step, operator, flags);
    }
    /* if (i == 0x20010) { */
    /*   printf("###########data:%x\n", data); */
    /*   do_print = false; */
    /* } */
    rom_data[i] = data;
  }

  write_rom_data(fd, rom_data, max_address);
  close(fd);


  /* for(int i = 0; i < 16; i++) { */
  /*   for(int j = 0; j < 5; j++) { */
  /*     printf("####1111 %d %d %x \n", i, j, MICRO_INSTRUCTION[i][j]); */
  /*   } */
  /* } */

  return 0;
}
