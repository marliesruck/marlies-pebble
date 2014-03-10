/** @file garrulous.c
 *
 *  @brief Tests print().
 *
 *  @author esn
 */

#include <simics.h>

#include <ctype.h>
#include <limits.h>
#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CASES 4
#define LEN 256
#define CONSOLE_HEIGHT 25
#define CONSOLE_WIDTH 80


void rnd_string(char *buf, size_t len)
{
  char ch;
  int i;

  for (i = 0; i < len; ++i) {
    ch = rand();
    if (isprint(ch))
      buf[i] = ch;
    else --i;
  }

  return;
}

int main()
{
  int row, col, color;
  char str[LEN];

  while (1) switch (rand() % CASES)
  {
  case 0:
    rnd_string(str, rand() % LEN);
    lprintf("print(%d, %p:\"%s\")", strlen(str), str, str);
    print(strlen(str), str);
    break;
  case 1:
    color = rand() % UCHAR_MAX;
    lprintf("set_term_color(%d(0x%08x))", color, color);
    set_term_color(color);
    break;
  case 2:
    row = rand() % CONSOLE_HEIGHT;
    col = rand() % CONSOLE_WIDTH;
    lprintf("set_cursor(%d, %d)", row, col);
    set_cursor_pos(row, col);
    break;
  case 3:
    lprintf("get_cursor_pos(%p, %p)", &row, &col);
    get_cursor_pos(&row, &col);
    lprintf("  = %d(0x%08x),%d(0x%08x)", row, row, col, col);
    break;
  }

  exit(0);
}
