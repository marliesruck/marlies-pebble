###########################################################################
# This is the include file for the make file.
# You should have to edit only this file to get things to build.
###########################################################################

###########################################################################
# Tab stops
###########################################################################
# If you use tabstops set to something other than the international
# standard of eight characters, this is your opportunity to inform
# our print scripts.
TABSTOP = 8

###########################################################################
# The method for acquiring project updates.
###########################################################################
# This should be "afs" for any Andrew machine, "web" for non-andrew machines
# and "offline" for machines with no network access.
#
# "offline" is strongly not recommended as you may miss important project
# updates.
#
UPDATE_METHOD = web

###########################################################################
# WARNING: When we test your code, the two TESTS variables below will be
# blanked.  Your kernel MUST BOOT AND RUN if 410TESTS and STUDENTTESTS
# are blank.  It would be wise for you to test that this works.
###########################################################################

###########################################################################
# Test programs provided by course staff you wish to run
###########################################################################
# A list of the test programs you want compiled in from the 410user/progs
# directory.
#
410TESTS = ck1 merchant peon knife fork_test1 exec_basic_helper exec_basic exec_nonexist coolness new_pages remove_pages_test1 remove_pages_test2 mem_permissions print_basic readline_basic deschedule_hang loader_test1 loader_test2 fork_wait wait_getpid sleep_test1 minclone_mem swexn_basic_test swexn_cookie_monster swexn_dispatch swexn_stands_for_swextensible swexn_uninstall_test swexn_regs make_crash_helper make_crash register_test fork_exit_bomb fork_bomb fork_wait_bomb cho cho2 cho_variant yield_desc_mkrun actual_wait agility_drill beady_test bistromath cat cvar_test cyclone excellent getpid_test1 halt_test join_specific_test juggle largetest mandelbrot misbehave misbehave_wrap multitest mutex_destroy_test nibbles paraguay racer stack_test1 startle switzerland thr_exit_join wild_test1 rwlock_downgrade_read_test slaughter

###########################################################################
# Test programs you have written which you wish to run
###########################################################################
# A list of the test programs you want compiled in from the user/progs
# directory.
#
STUDENTTESTS =  introspective schizo introvert garrulous mimic zfod cooperative annoying coquettish coy cooperative_terminate merchant_terminate peon_terminate coolness_terminate coy_terminate regression epileptic rogue intrepid carpe_diem 

###########################################################################
# Data files provided by course staff to build into the RAM disk
###########################################################################
# A list of the data files you want built in from the 410user/files
# directory.
#
410FILES =

###########################################################################
# Data files you have created which you wish to build into the RAM disk
###########################################################################
# A list of the data files you want built in from the user/files
# directory.
#
STUDENTFILES =

###########################################################################
# Object files for your thread library
###########################################################################
THREAD_OBJS = malloc.o panic.o asm_thr.o thread.o tcb.o stack_alloc.o synch/atomic.o synch/cvar.o synch/mutex.o synch/sem.o synch/spin.o util/cllist.o synch/rwlock.o 

# Thread Group Library Support.
#
# Since libthrgrp.a depends on your thread library, the "buildable blank
# P3" we give you can't build libthrgrp.a.  Once you install your thread
# library and fix THREAD_OBJS above, uncomment this line to enable building
# libthrgrp.a:
410USER_LIBS_EARLY += libthrgrp.a

###########################################################################
# Object files for your syscall wrappers
###########################################################################
SYSCALL_OBJS = set_status.o vanish.o print.o fork.o exec.o wait.o task_vanish.o gettid.o yield.o deschedule.o make_runnable.o get_ticks.o sleep.o new_pages.o remove_pages.o get_cursor_pos.o getchar.o halt.o readfile.o readline.o set_cursor_pos.o set_term_color.o swexn.o misbehave.o

###########################################################################
# Object files for your automatic stack handling
###########################################################################
AUTOSTACK_OBJS = autostack.o

###########################################################################
# Parts of your kernel
###########################################################################
#
# Kernel object files you want included from 410kern/
#
410KERNEL_OBJS = load_helper.o
#
# Kernel object files you provide in from kern/
#
KERNEL_OBJS = kernel.o lib/page_util.o lib/atomic.o lib/spin.o lib/cvar.o lib/mutex.o lib/cllist.o sched/process.o sched/dispatch_wrapper.o sched/sched.o sched/dispatch.o lib/malloc_wrappers.o vm/vm.o vm/asm_tlb.o vm/tlb.o vm/page_alloc.o vm/frame_alloc.o vm/pg_table.o loader/loader.o loader/usr_stack.o entry/drivers/keyboard.o entry/drivers/timer.o entry/idt.o entry/drivers/driver_wrappers.o entry/drivers/console.o entry/drivers/cursor.o entry/syscall/lifecycle.o entry/syscall/swexn.o entry/syscall/threadmgmt.o entry/syscall/mem_mgmt.o entry/syscall/misc_syscalls.o entry/syscall/console_io.o entry/syscall/sc_utils.o entry/syscall/syscall_wrappers.o entry/faults/faults.o entry/faults/fault_wrappers.o entry/drivers/drivers.o sched/thread.o 

###########################################################################
# WARNING: Do not put **test** programs into the REQPROGS variables.  Your
#          kernel will probably not build in the test harness and you will
#          lose points.
###########################################################################

###########################################################################
# Mandatory programs whose source is provided by course staff
###########################################################################
# A list of the programs in 410user/progs which are provided in source
# form and NECESSARY FOR THE KERNEL TO RUN.
#
# The shell is a really good thing to keep here.  Don't delete idle
# or init unless you are writing your own, and don't do that unless
# you have a really good reason to do so.
#
410REQPROGS = idle init shell

###########################################################################
# Mandatory programs whose source is provided by you
###########################################################################
# A list of the programs in user/progs which are provided in source
# form and NECESSARY FOR THE KERNEL TO RUN.
#
# Leave this blank unless you are writing custom init/idle/shell programs
# (not generally recommended).  If you use STUDENTREQPROGS so you can
# temporarily run a special debugging version of init/idle/shell, you
# need to be very sure you blank STUDENTREQPROGS before turning your
# kernel in, or else your tweaked version will run and the test harness
# won't.
#
STUDENTREQPROGS = 
