/** @file 410user/progs/remove_pages_test2.c
 *  @author mpa
 *  @brief Tests remove_pages()
 *  @public yes
 *  @for p3
 *  @covers new_pages remove_pages
 *  @status done
 */

#include <syscall.h>
#include <stdio.h>
#include <report.h>
#include "../../410user/inc/410_tests.h"

#define ADDR 0x40000000
#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

DEF_TEST_NAME("rogue:");

int main() {
  report_start(START_ABORT);

  char *args[5];

  args[0] = "remove_pages_test2";
  args[1] = NULL;

  while(1){
    if(!fork()) exec(args[0], args);
    while(wait(NULL) >= 0);
  }
}
