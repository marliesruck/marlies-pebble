/* 
 * Copyright (c) 1996-1995 The University of Utah and
 * the Computer Systems Laboratory at the University of Utah (CSL).
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software is hereby
 * granted provided that (1) source code retains these copyright, permission,
 * and disclaimer notices, and (2) redistributions including binaries
 * reproduce the notices in supporting documentation, and (3) all advertising
 * materials mentioning features or use of this software display the following
 * acknowledgement: ``This product includes software developed by the
 * Computer Systems Laboratory at the University of Utah.''
 *
 * THE UNIVERSITY OF UTAH AND CSL ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS
 * IS" CONDITION.  THE UNIVERSITY OF UTAH AND CSL DISCLAIM ANY LIABILITY OF
 * ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * CSL requests users of this software to return to csl-dist@cs.utah.edu any
 * improvements that they make and grant CSL redistribution rights.
 */

#include <simics.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>


#define MAX_MSG_LEN 80

/* @brief If a thread crashes dump the registers and vanish
 */
void panic(const char *fmt, ...)
{
  va_list vl;
  char msg[MAX_MSG_LEN];

  if (fmt) {
    /* Write error message to string */
    va_start(vl, fmt);
    vsnprintf(msg, MAX_MSG_LEN, fmt, vl);
    va_end(vl);

    /* Print to simics terminal */
    lprintf("Encountered fatal error:");
    lprintf("%s",msg);
  }
  lprintf("Aborting...");

  /* End it all, take your children with you */
  task_vanish(-1);

  return;
}

