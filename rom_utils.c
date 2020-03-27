#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "rom_utils.h"

static uint32_t same_count = 0;
static uint32_t prev_character = 0;
static uint32_t init = 0;

static void write_header(int fd) {
  char *header = "v2.0 raw\n";
  write(fd, header, strlen(header));
}

static void write_single_data(int fd, uint32_t data, char *last) {
  char buf[9];
  uint32_t loop;
  uint32_t i;
  uint32_t j = 0;
  char zero_buf[2] = {'0'};
  int total_write = 9;

  if (data == 0 ) {
    if (last != NULL) {
      zero_buf[1] = last[0];
      write(fd, zero_buf, 2);
      return;
    } else {
      write(fd, zero_buf, 1);
      return;
    }
  }

  for(int i = 7; i >= 0; i--) {
    loop = (data >> (4 * i)) & 0xf;
    if (loop < 10) {
      buf[j++] = 48 + loop;
    } else {
      buf[j++] = 97 + loop - 0xa;
    }
  }
  if (last != NULL) {
    buf[8] = last[0];
  } else {
    total_write--;
  }

  for(i = 0; i < 8; i++) {
    if(buf[i] != 48) {
      break;
    }
  }

  write(fd, buf + i, total_write - i);
}

static void write_count_data(int fd, uint32_t data, char last) {
  char buf[6];
  uint32_t loop;

  sprintf(buf, "%d", data);

  write(fd, buf, strlen(buf));
  write(fd, &last, 1);
}


static void write_data(int fd, uint32_t data) {
  if(init == 0) {
    init = 1;
    prev_character = data;
    same_count = 1;
    return;
  }

  if (data == prev_character) {
    same_count++;
    return;
  }

  if (same_count <= 3) {
    for(int i = 0; i < same_count; i++) {
      write_single_data(fd, prev_character, " ");
    }
  } else {
    write_count_data(fd, same_count, '*');
    write_single_data(fd, prev_character,  " ");
  }
  same_count = 1;
  prev_character = data;
}

void write_rom_data(int fd, uint32_t data[], size_t size) {
  prev_character = 0;
  same_count = 0;
  init = 0;

  write_header(fd);
  for(int i = 0; i < size; i++) {
    write_data(fd, data[i]);
  }
  if (same_count <= 3) {
    for(int i = 0; i < same_count; i++) {
      write_single_data(fd, prev_character, NULL);
    }
  } else {
    write_count_data(fd, same_count, '*');
    write_single_data(fd, prev_character, NULL);
  }
  write(fd, "\n", 1);
}
