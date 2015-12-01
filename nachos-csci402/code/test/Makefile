# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# If you are running on SunOS 5.x machines, uncomment the next line
GCCDIR = ../gnu/
LDFLAGS = -T newscript -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR =
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld
CPP = $(GCCDIR)cpp

INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR)

all: halt matmult sort testfiles locksandcvs exec print fork passportoffice passexec testexec test_locks test_broadcast test_broadcast_wait customer appclerk picclerk passclerk cashier manager senator execcustomers execappclerks execpicclerks execpassclerks execcashiers execsenators

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult

testfiles.o: testfiles.c
	$(CC) $(CFLAGS) -c testfiles.c
testfiles: testfiles.o start.o
	$(LD) $(LDFLAGS) start.o testfiles.o -o testfiles.coff
	../bin/coff2noff testfiles.coff testfiles

locksandcvs.o: locksandcvs.c
	$(CC) $(CFLAGS) -c locksandcvs.c
locksandcvs: locksandcvs.o start.o
	$(LD) $(LDFLAGS) start.o locksandcvs.o -o locksandcvs.coff
	../bin/coff2noff locksandcvs.coff locksandcvs

exec.o: exec.c
	$(CC) $(CFLAGS) -c exec.c
exec: exec.o start.o
	$(LD) $(LDFLAGS) start.o exec.o -o exec.coff
	../bin/coff2noff exec.coff exec

print.o: print.c
	$(CC) $(CFLAGS) -c print.c
print: print.o start.o
	$(LD) $(LDFLAGS) start.o print.o -o print.coff
	../bin/coff2noff print.coff print

fork.o: fork.c
	$(CC) $(CFLAGS) -c fork.c
fork: fork.o start.o
	$(LD) $(LDFLAGS) start.o fork.o -o fork.coff
	../bin/coff2noff fork.coff fork

passportoffice.o: passportoffice.c
	$(CC) $(CFLAGS) -c passportoffice.c
passportoffice: passportoffice.o start.o
	$(LD) $(LDFLAGS) start.o passportoffice.o -o passportoffice.coff
	../bin/coff2noff passportoffice.coff passportoffice

passexec.o: passexec.c
	$(CC) $(CFLAGS) -c passexec.c
passexec: passexec.o start.o
	$(LD) $(LDFLAGS) start.o passexec.o -o passexec.coff
	../bin/coff2noff passexec.coff passexec

#mp_wait.o: mp_wait.c
#	$(CC) $(CFLAGS) -c mp_wait.c
#mp_wait: mp_wait.o start.o
#	$(LD) $(LDFLAGS) start.o mp_wait.o -o mp_wait.coff
#	../bin/coff2noff mp_wait.coff mp_wait

#mp_signal.o: mp_signal.c
#	$(CC) $(CFLAGS) -c mp_signal.c
#mp_signal: mp_signal.o start.o
#	$(LD) $(LDFLAGS) start.o mp_signal.o -o mp_signal.coff
#	../bin/coff2noff mp_signal.coff mp_signal

#mp_broadcast.o: mp_broadcast.c
#	$(CC) $(CFLAGS) -c mp_broadcast.c
#mp_broadcast: mp_broadcast.o start.o
#	$(LD) $(LDFLAGS) start.o mp_broadcast.o -o mp_broadcast.coff
#	../bin/coff2noff mp_broadcast.coff mp_broadcast

#mp_lock.o: mp_lock.c
#	$(CC) $(CFLAGS) -c mp_lock.c
#mp_lock: mp_lock.o start.o
#	$(LD) $(LDFLAGS) start.o mp_lock.o -o mp_lock.coff
#	../bin/coff2noff mp_lock.coff mp_lock

testexec.o: testexec.c
	$(CC) $(CFLAGS) -c testexec.c
testexec: testexec.o start.o
	$(LD) $(LDFLAGS) start.o testexec.o -o testexec.coff
	../bin/coff2noff testexec.coff testexec

test_locks.o: test_locks.c
	$(CC) $(CFLAGS) -c test_locks.c
test_locks: test_locks.o start.o
	$(LD) $(LDFLAGS) start.o test_locks.o -o test_locks.coff
	../bin/coff2noff test_locks.coff test_locks

test_broadcast.o: test_broadcast.c
	$(CC) $(CFLAGS) -c test_broadcast.c
test_broadcast: test_broadcast.o start.o
	$(LD) $(LDFLAGS) start.o test_broadcast.o -o test_broadcast.coff
	../bin/coff2noff test_broadcast.coff test_broadcast

test_broadcast_wait.o: test_broadcast_wait.c
	$(CC) $(CFLAGS) -c test_broadcast_wait.c
test_broadcast_wait: test_broadcast_wait.o start.o
	$(LD) $(LDFLAGS) start.o test_broadcast_wait.o -o test_broadcast_wait.coff
	../bin/coff2noff test_broadcast_wait.coff test_broadcast_wait

customer.o: customer.c
	$(CC) $(CFLAGS) -c customer.c
customer: customer.o start.o
	$(LD) $(LDFLAGS) start.o customer.o -o customer.coff
	../bin/coff2noff customer.coff customer

appclerk.o: appclerk.c
	$(CC) $(CFLAGS) -c appclerk.c
appclerk: appclerk.o start.o
	$(LD) $(LDFLAGS) start.o appclerk.o -o appclerk.coff
	../bin/coff2noff appclerk.coff appclerk

picclerk.o: picclerk.c
	$(CC) $(CFLAGS) -c picclerk.c
picclerk: picclerk.o start.o
	$(LD) $(LDFLAGS) start.o picclerk.o -o picclerk.coff
	../bin/coff2noff picclerk.coff picclerk

passclerk.o: passclerk.c
	$(CC) $(CFLAGS) -c passclerk.c
passclerk: passclerk.o start.o
	$(LD) $(LDFLAGS) start.o passclerk.o -o passclerk.coff
	../bin/coff2noff passclerk.coff passclerk

cashier.o: cashier.c
	$(CC) $(CFLAGS) -c cashier.c
cashier: cashier.o start.o
	$(LD) $(LDFLAGS) start.o cashier.o -o cashier.coff
	../bin/coff2noff cashier.coff cashier

manager.o: manager.c
	$(CC) $(CFLAGS) -c manager.c
manager: manager.o start.o
	$(LD) $(LDFLAGS) start.o manager.o -o manager.coff
	../bin/coff2noff manager.coff manager

senator.o: senator.c
	$(CC) $(CFLAGS) -c senator.c
senator: senator.o start.o
	$(LD) $(LDFLAGS) start.o senator.o -o senator.coff
	../bin/coff2noff senator.coff senator

execcustomers.o: execcustomers.c
	$(CC) $(CFLAGS) -c execcustomers.c
execcustomers: execcustomers.o start.o
	$(LD) $(LDFLAGS) start.o execcustomers.o -o execcustomers.coff
	../bin/coff2noff execcustomers.coff execcustomers

execappclerks.o: execappclerks.c
	$(CC) $(CFLAGS) -c execappclerks.c
execappclerks: execappclerks.o start.o
	$(LD) $(LDFLAGS) start.o execappclerks.o -o execappclerks.coff
	../bin/coff2noff execappclerks.coff execappclerks

execpicclerks.o: execpicclerks.c
	$(CC) $(CFLAGS) -c execpicclerks.c
execpicclerks: execpicclerks.o start.o
	$(LD) $(LDFLAGS) start.o execpicclerks.o -o execpicclerks.coff
	../bin/coff2noff execpicclerks.coff execpicclerks

execpassclerks.o: execpassclerks.c
	$(CC) $(CFLAGS) -c execpassclerks.c
execpassclerks: execpassclerks.o start.o
	$(LD) $(LDFLAGS) start.o execpassclerks.o -o execpassclerks.coff
	../bin/coff2noff execpassclerks.coff execpassclerks

execcashiers.o: execcashiers.c
	$(CC) $(CFLAGS) -c execcashiers.c
execcashiers: execcashiers.o start.o
	$(LD) $(LDFLAGS) start.o execcashiers.o -o execcashiers.coff
	../bin/coff2noff execcashiers.coff execcashiers

execsenators.o: execsenators.c
	$(CC) $(CFLAGS) -c execsenators.c
execsenators: execsenators.o start.o
	$(LD) $(LDFLAGS) start.o execsenators.o -o execsenators.coff
	../bin/coff2noff execsenators.coff execsenators

clean:
	rm -f *.o *.coff
