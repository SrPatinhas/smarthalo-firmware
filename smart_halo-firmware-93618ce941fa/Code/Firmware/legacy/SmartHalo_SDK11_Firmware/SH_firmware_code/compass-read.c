compass-read/                                                                                       0000755 0001750 0001750 00000000000 12305430071 012375  5                                                                                                    ustar   henke                           henke                                                                                                                                                                                                                  compass-read/diag.h                                                                                 0000644 0001750 0001750 00000000600 12305426361 013455  0                                                                                                    ustar   henke                           henke                                                                                                                                                                                                                  #ifndef DIAG_H
#define DIAG_H

#include <stdarg.h>

int diag(char *fmt, ...) __attribute__ ((format(printf,1,2)));
void *diagp(char *fmt, ...) __attribute__ ((format(printf,1,2)));
int sysdiag(char *syscallname, char *fmt, ...)
 __attribute__ ((format(printf,2,3)));
void *sysdiagp(char *syscallname, char *fmt, ...)
 __attribute__ ((format(printf,2,3)));

#endif /* ifndef DIAG_H */
                                                                                                                                compass-read/COPYING                                                                                0000644 0001750 0001750 00000002766 12305426361 013452  0                                                                                                    ustar   henke                           henke                                                                                                                                                                                                                  Copyright (c) 2014 Douglas Henke <dhenke@mythopoeic.org>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the original author nor the names of other
      contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
          compass-read/README                                                                                 0000664 0001750 0001750 00000002433 12305430071 013261  0                                                                                                    ustar   henke                           henke                                                                                                                                                                                                                  This is an example program intended to demonstrate how to take readings
from an HMC5883L magnetometer hooked up to an I2C bus on Linux.

This is free software. See COPYING for details.

To build, run "make" as a normal user.

If you get errors like:

hmc5883l.c: In function ‘hmc5883l_chk’:
hmc5883l.c:46:4: error: implicit declaration of function ‘i2c_smbus_read_byte_data’ [-Werror=implicit-function-declaration]

then the problem is you don't have the libi2c-dev package installed (for
Debian-like things including Raspbian and Ubuntu). (If you're on another
type of system, the general problem is that there are two headers called
linux/i2c-dev.h -- one for the kernel, and one for user-space programs like
this one. The above error means you have the former but need the latter.)

To run, launch as root with no arguments, with stdout directed to a file:

   sudo ./compass-read >mydata

The program will take readings as fast as it can, and write them one
per line to the output, until you stop it or until it has 100K, whichever
comes first.

The output is human-readable text, three numbers per line separated by
spaces. The numbers are X, Y and Z components of the sensed field, in
guass.

More info: http://mythopoeic.org/magnetometer-real-data/

Contact the author: dhenke@mythopoeic.org
                                                                                                                                                                                                                                     compass-read/debug.h                                                                                0000644 0001750 0001750 00000010356 12305426361 013650  0                                                                                                    ustar   henke                           henke                                                                                                                                                                                                                  #ifndef DEBUG_H
#define DEBUG_H

#include "diag.h"

/* IMPORTANT: Never use these macros to perform a test that you expect
   might fail for an actual user. Use them only to test for program bugs
   and "never happens" conditions. */

/* This header defines a set of macros that allows the same (terse) code to
   use thorough and paranoid assertion checking everywhere, or to be as
   ruthlessly space- and time-efficient as possible, according to the
   definition of the PARANOIA_LEVEL preprocessor constant in effect.

   The general case is that you can write something like:

      TST(var=foo()+bar(), <= junk+2);

   If error checking is turned up, this expands to (approximately):

      if((var = foo() + bar()) <= junk + 2) { BOMB(); }

   where BOMB() is a notional function that emits an error message giving
   the module name and line number, then forces program termination. If
   error checking is off, the same line expands to:

      var = foo() + bar();

   In general, the first argument to TST can be any expression. If
   it has side-effects, they will happen exactly once regardless of
   the paranoia level.

   The second argument is text which can be appended to the parenthesized
   first expression to form a conditional which, if true, causes the
   program to abort. Side-effects here happen only if error checking is
   turned on, thus are best avoided.

   Some other shortcut macros are defined in terms of TST:

   TSTP(x); -- assert that x evaluates to non-NULL
   TSTR(x); -- assert that x evaluates to 0

   This is amazingly useful, as you can replace the usual:

      #if PARANOIA_LEVEL == 0
         (void)SomeDumbFunction(arg);
      #else
         if(SomeDumbFunction(arg)) bomb("unhappy thing");
      #endif

   with the equivalent:

      TSTR(SomeDumbFunction(arg));

   which is less than half as long and much easier to read. It also makes
   it harder to make mistakes which cause your program to work when paranoid
   but break when not, or to run more slowly than it needs to in production.

   A special assertion macro TSTA(x) is also defined. If error checking
   is enabled, it evaluates x as an expression which must be false or the
   program aborts. If error checking is off, it expands to nothing. Use
   this _only_ when x doesn't have side-effects. It is useful for things
   like sanity-checking function arguments.

   The macro UNREACHABLE, if error checking is enabled, always aborts
   with a message indicating a section of code marked unreachable was
   in fact reached. If error checking is disbaled, it expands to nothing.

   The astute reader will note that the TSTx macros do not let you specify
   a message. That is intentional; the idea is that you'll be given the
   module name and line number, and will have to RTFS anyway to fix the
   problem, so you might as well have to look there to see what went wrong.
*/

/* Note for advanced users: The various TSTx macros behave marginally
   differently under PARANOIA_SOME versus PARANOIA_UTMOST. The
   difference is in the message displayed: UTMOST gives the whole
   expression that evaluated true to cause the bomb, while SOME
   just says "oops". The UTMOST version is clearly more useful, but
   bloats your binary size. */

/* some preliminaries */
#define PARANOIA_NONE 0
#define PARANOIA_SOME 1
#define PARANOIA_UTMOST 2
#ifndef PARANOIA_LEVEL
#define PARANOIA_LEVEL PARANOIA_NONE
#endif /* ifndef PARANOIA_LEVEL */

#if PARANOIA_LEVEL == PARANOIA_UTMOST || PARANOIA_LEVEL == PARANOIA_SOME
#if PARANOIA_LEVEL == PARANOIA_UTMOST
#define TST(x,y) do{ if((x)y) BOMB("(" #x ")" #y); } while(0)
#else
#define TST(x,y) do{ if((x)y) BOMB("oops"); } while(0)
#endif
#define TSTP(x) TST(x,==NULL)
#define TSTR(x) TST(x,!=0)
#define TSTA(x) TST(x,!=0)
#define UNREACHABLE BOMB("unreachable block executed")
#elif PARANOIA_LEVEL == PARANOIA_NONE
#define TST(x,y) x
#define TSTP(x) x
#define TSTR(x) x
#define TSTA(x)
#define UNREACHABLE
#else /* if PARANOIA_LEVEL isn't one of _UTMOST, _SOME or _NONE */
#error unrecognized PARANOIA_LEVEL setting
#endif /* if PARANOIA_LEVEL == various things */

/* If they call BOMB() explicitly, they probably want it to do that even
   if running in non-paranoid mode. */
#define BOMB(x) do{diag(__FILE__":%d "x, __LINE__); abort();} while(0)

#endif /* ifndef DEBUG_H */
                                                                                                                                                                                                                                                                                  compass-read/i2c_ops.c                                                                              0000644 0001750 0001750 00000010601 12305426361 014104  0                                                                                                    ustar   henke                           henke                                                                                                                                                                                                                  /* This is free software. See COPYING for details. */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <dirent.h>
#include <linux/i2c-dev.h>
#include "diag.h"
#include "debug.h"
#include "i2c_ops.h"

#define DEVDIR "/dev"

/*** try_dev() -- check if desired I2C device is on a given bus

Given an I2C device file (/dev/i2c-x), this function attempts to
determine if a specific I2C device is present on the associated bus.

Arguments:
   fname -- the path to the device-special file associated with the
      I2C bus to be tested
   addr -- the (unshifted) address of the I2C device we're looking for
   devname -- pointer to a nul-terminated string containing the human-
      readable name of the device (typically a part number like "DS1307")
   callback -- a callback function which should return 0 IFF the
      desired device is present

The callback function is called with a single argument: a file descriptor
open on the specified bus, which has been initialized to communicate with
a slave device at the specified address.

Note that in some cases it is possible to lock the I2C bus or crash the
whole system through ill-advised probing. The callback function should
only read or write registers which are generally safe for any of the devices
that use the given address. It should also 1) perform at least one read
or write operation -- not necessarily both -- to verify it can actually
talk to something and 2) attempt to distinguish amongst the various
devices that can use the address in question, if there are more than one.
Some devices have ID registers that are ideal for this purpose.

On success, returns a descriptor open and ready to talk to the intended
slave device (which has been verified as present by the callback).

On failure, returns a negative value.
***/
static int try_dev(const char * const fname, const uint8_t addr,
 const char * const devname, int(*callback)(const int fd)) {
   int fd = -1, rtn = 0;

   TSTA(!fname); TSTA(!devname); TSTA(!callback);

   if((fd = open(fname, O_RDWR)) < 0) {
      rtn = sysdiag("open", "can't open %s", fname); goto done;
   }

   if(ioctl(fd, I2C_SLAVE, addr)) {
      rtn = sysdiag("ioctl", "no I2C slave %02x on %s", addr, fname); goto done;
   }

   if(callback(fd)) {
      rtn = diag("no %s at address %02x on %s", devname, addr, fname);
      goto done;
   }

   diag("%s found at %02x on %s", devname, addr, fname);

   done:
   if(rtn && fd >= 0) {
      if(close(fd)) sysdiag("close", "can't close I2C device");
      fd = -1;
   }
   return fd;
}

/*** i2c_find() -- find and open an I2C device

This function attempts to find a specific I2C device connected to any one of
the available busses.

Arguments:
   addr -- the unshifted address of the desired I2C device
   devname -- pointer to a nul-terminated string containing the human-
      readable name of the device (typically a part number like "DS1307")
   callback -- pointer to a callback function used to test for the
      presence of the desired device; see try_dev() above for details

On success, returns an open file descriptor ready for communication with
the specified device. The caller must close()(2) this descriptor when it
is no longer needed.

On failure, returns a negative value.
***/
int i2c_find(const uint8_t addr, const char * const devname,
int(*callback)(const int fd)) {
   DIR *dir=NULL;
   int fd = -1, rtn = 0;
   struct dirent *ent;
   struct stat sbuf;

   TSTA(!devname); TSTA(!callback);

   if(chdir(DEVDIR)) {
      rtn = sysdiag("chdir", "can't chdir to %s", DEVDIR); goto done;
   }

   if((dir = opendir(".")) == NULL) {
      rtn = sysdiag("opendir", "can't open %s", DEVDIR); goto done;
   }

   while((ent = readdir(dir)) != NULL) {
      if(strlen(ent->d_name) < 5) continue;
      if(memcmp(ent->d_name, "i2c-", 4)) continue;
      if(stat(ent->d_name, &sbuf)) {
         sysdiag("stat", "can't stat %s", ent->d_name);
         continue;
      }
      if(!S_ISCHR(sbuf.st_mode)) continue;

      if((fd = try_dev(ent->d_name, addr, devname, callback)) >= 0) goto done;
   }

   rtn = diag("no I2C device found with address %02x", addr);

   done:
   if(dir && closedir(dir)) rtn = sysdiag("closedir", "can't close %s", DEVDIR);
   if(rtn && fd >= 0) {
      if(close(fd)) sysdiag("close", "can't close I2C device");
      fd = -1;
   }
   return fd;
}
                                                                                                                               compass-read/hmc5883l.h                                                                             0000644 0001750 0001750 00000001346 12305426361 014034  0                                                                                                    ustar   henke                           henke                                                                                                                                                                                                                  #ifndef HMC5883L_H
#define HMC5883L_H

typedef struct { double x, y, z; } hmc5883l_vec3_t;

typedef struct {
  /* Number of samples averaged per reading: */
  enum avg {AVG_1=0, AVG_2=1, AVG_4=2, AVG_8=3} avg;

  /* Load bias direction: */
  enum bias {BIAS_NONE=0, BIAS_POS=1, BIAS_NEG=2} bias;

  /* Gain (in LSb per Gauss): */
  enum gain {GAIN_1370=0, GAIN_1090=1, GAIN_820=2, GAIN_660=3, GAIN_440=4,
   GAIN_390=5, GAIN_330=6, GAIN_230=7} gain;
} hmc5883l_config_t;

extern const hmc5883l_config_t _hmc5883l_config_default;

int hmc5883l_open(void);
int hmc5883l_read(const int fd, hmc5883l_vec3_t * const p,
 const enum gain gain);
int hmc5883l_config(const int fd, const hmc5883l_config_t * const cfg);

#endif /* ifndef HMC5883L_H */
                                                                                                                                                                                                                                                                                          compass-read/Makefile                                                                               0000644 0001750 0001750 00000000631 12305426361 014044  0                                                                                                    ustar   henke                           henke                                                                                                                                                                                                                  CFLAGS+=-O3 -Wall -Werror -DPARANOIA_LEVEL=PARANOIA_UTMOST
MAKEDEP=$(CC) $(CFLAGS) -MM
SRC=diag.c i2c_ops.c hmc5883l.c main.c
TARGET=compass-read

all: $(TARGET)

clean:
	rm -f $(TARGET) *.[oad] core

$(TARGET): $(SRC:.c=.o)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.d:%.c
	$(MAKEDEP) $< >$@

ifneq ($(MAKECMDGOALS),clean)
include $(SRC:.c=.d) # always include dependency files for all .c files
endif
                                                                                                       compass-read/i2c_ops.h                                                                              0000644 0001750 0001750 00000000241 12305426361 014110  0                                                                                                    ustar   henke                           henke                                                                                                                                                                                                                  #ifndef I2C_OPS_H
#define I2C_OPS_H

int i2c_find(const uint8_t addr, const char * const devname,
 int(*callback)(const int fd));

#endif /* ifndef I2C_OPS_H */
                                                                                                                                                                                                                                                                                                                                                               compass-read/diag.c                                                                                 0000644 0001750 0001750 00000007110 12305426361 013453  0                                                                                                    ustar   henke                           henke                                                                                                                                                                                                                  /*****************************************************************************
diag -- diagnostic library

This library implements a set of four functions which make it easy to
report and return errors using compact code.

The functions are:

   diag() -- emit a diagnostic message; arguments are like printf; always
             returns -1
   diagp() -- like diag() but always returns NULL
   sysdiag() -- takes an additional argument before the format string: the
             name of a function; produces additional output based on errno;
             always returns -1
   sysdiagp() -- like sysdiag() but always returns NULL

This allows you to replace code like:

   if((infile = fopen(fname, "r")) == NULL) {
      fprintf(stderr, "can't open file %s\n", fname);
      perror("fopen");
      return -1;
   }

with the equivalent but much cleaner:

   if((infile = fopen(fname, "r")) == NULL)
      return sysdiag("fopen", "can't open file %s", fname);

This is free software. See COPYING for details.
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "debug.h"
#include "diag.h"

/***
static void diagmsg(char *syscallname, char *fmt, va_list args) -- emit diag

Function used internally by the diag() family of functions. The first
argument is a pointer to a string containing the name of the system or
library function which failed (or NULL if this is an application-level
error). The second argument is a printf()-style format string. The
third argument is a list of additional args as required by the format.

If the syscallname (first argument) is non-NULL, an additional message is
printed containing the syscallname string and the text corresponding to the
current value of the global errno.
***/
static void diagmsg(char *syscallname, char *fmt, va_list args) {
   TSTA(fmt==NULL);

   vfprintf(stderr, fmt, args);
   fputc('\n', stderr);
   if(syscallname == NULL) return;
   fprintf(stderr, "%s(): %s\n", syscallname, strerror(errno));
}

/***
int diag(char *fmt, ...) -- emit diagnostic message

This function emits a diagnostic message. The arguments are as per
printf()(3). Always returns -1.
***/
int diag(char *fmt, ...) {
   va_list args;

   TSTA(fmt==NULL);

   va_start(args,fmt);
   diagmsg(NULL, fmt, args);
   va_end(args);
   return -1;
}

/***
void *diagp(char *fmt, ...) -- emit diagnostic message

Identical to diag(), above, but returns NULL instead of -1.
***/
void *diagp(char *fmt, ...) {
   va_list args;

   TSTA(fmt==NULL);

   va_start(args,fmt);
   diagmsg(NULL, fmt, args);
   va_end(args);
   return NULL;
}

/***
int sysdiag(char *syscallname, char *fmt, ...) -- emit diagnostic message

Emits a diagnostic message suitable for reporting the failure of a system
or library function which sets the global variable errno. The first argument
is a pointer to a string containing the name of the failed function. The
subsequent arguments are as per printf()(3). Always returns -1.
***/
int sysdiag(char *syscallname, char *fmt, ...) {
   va_list args;

   TSTA(syscallname==NULL); TSTA(fmt==NULL);

   va_start(args,fmt);
   diagmsg(syscallname, fmt, args);
   va_end(args);
   return -1;
}

/***
void *sysdiagp(char *syscallname, char *fmt, ...) -- emit diagnostic message

Identical to sysdiag(), above, but always returns NULL rather than -1.
***/
void *sysdiagp(char *syscallname, char *fmt, ...) {
   va_list args;

   TSTA(syscallname==NULL); TSTA(fmt==NULL);

   va_start(args,fmt);
   diagmsg(syscallname, fmt, args);
   va_end(args);
   return NULL;
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                        compass-read/hmc5883l.c                                                                             0000644 0001750 0001750 00000012730 12305426361 014026  0                                                                                                    ustar   henke                           henke                                                                                                                                                                                                                  /* This is free software. See COPYING for details. */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <linux/i2c-dev.h>
#include <arpa/inet.h>
#include "diag.h"
#include "debug.h"
#include "i2c_ops.h"
#include "hmc5883l.h"

/* Register addresses on HMC5883L: */
#define REG_CRA 0  /* config A */
#define REG_CRB 1  /* config B */
#define REG_MR 2   /* mode */
#define REG_DATA 3 /* registers 3-8 inclusive are data, read as a block */
#define REG_SR 9   /* status */
#define REG_IRA 10 /* ID A = 'H' */
#define REG_IRB 11 /* ID B = '4' */
#define REG_IRC 12 /* ID C = '3' */

const hmc5883l_config_t _hmc5883l_config_default = {
   AVG_1, BIAS_NONE, GAIN_1090
};

/*** hmc5883l_chk() -- check for HMC5883L magnetometer on I2C bus

This callback function is intended for use with i2c_find() to test for
the presence of a Honeywell HMC5883L 3-axis digital compass on the I2C
bus.

The argument is a file descriptor open on an I2C bus an initialized for
communication with a slave device at 0x1E.

The author does not know of any I2C devices using 7-bit address 0x1E which
react badly to this probe. (Nor, indeed, does he know of any other I2C
devices which use address 0x1E.)

Returns 0 IFF an HMC5883L device is found on the bus associated with
the given descriptor.
***/
static int hmc5883l_chk(const int fd) {
   TSTA(fd < 0);
   if(i2c_smbus_read_byte_data(fd, REG_IRA) != 'H') return -1;
   if(i2c_smbus_read_byte_data(fd, REG_IRB) != '4') return -1;
   if(i2c_smbus_read_byte_data(fd, REG_IRC) != '3') return -1;
   return 0;
}

/*** hmc5883l_open() -- open fd to talk to HMC5883L

Locates an HMC5883L magentometer on one of the available I2C busses and
returns an open descriptor prepared to communicate with it. (If there are
several busses with an HMC5883L connected, one will be chosen arbitrarily.
But why would that situation exist?)

The caller is responsible for close()(2)-ing the descriptor when it is no
longer needed.

On error (or if no such device could be located) returns a negative value.
***/
int hmc5883l_open(void) {
   return i2c_find(0x1e, "HMC5883L", hmc5883l_chk);
}

/*** hmc5883l_read() -- read sensor values from HMC5883L

Read the sensor values from the HMC5883L.

Note: Results are undefined if HMC5883L has not yet been configured
using hmc5883l_config().

Arguments:
   fd -- descriptor returned by previous call to hmc5883l_open()
   p -- pointer to data structure into which sensor data is to be
      stored, or NULL if the readings are to be discarded
   gain -- gain value used to convert raw readings to Gauss; must
      be the same as that in the gain field of the hmc5883l_config_t
      structure passed to hmc5883l_config()

Returns 0 on success, non-0 on failure.
***/
int hmc5883l_read(const int fd, hmc5883l_vec3_t * const p,
 const enum gain gain) {
   int i;
   __u8 buf[6];
   int16_t *val = (int16_t *)buf;
   static const int bits_per_gauss[] = { /* indices are enum gain values */
      1370, 1090, 820, 660, 440, 390, 330, 230
   };

   TSTA(fd < 0);

   /* Start a measurement in single measurement mode: */
   if(i2c_smbus_write_byte_data(fd, REG_MR, 0x01))
      return sysdiag("i2c_smbus_write_byte_data", "can't write to Mode");

   /* Tell the device we want to start reading at DXRA (register 3): */
   if(i2c_smbus_write_byte(fd, REG_DATA))
      return sysdiag("i2c_smbus_write_byte", "failed writing position");

   /* We could poll the status register here, but on a time-sharing
      system, we could easily miss the very brief period during which
      it's low. So instead, we'll just block for a minimum of
      (1Hz/s)/160Hz = 6250us, after which we know the reading will be
      ready. */
   usleep(6250);

   /* Read all six positions: */
   if(read(fd, buf, 6) != 6)
      return sysdiag("read", "failed reading position values");

   if(!p) return 0;

   for(i = 0; i < 3; i++) val[i] = ntohs(val[i]);

   p->x = (double)(val[0]) / (double)bits_per_gauss[gain];
   p->y = (double)(val[1]) / (double)bits_per_gauss[gain];
   p->z = (double)(val[2]) / (double)bits_per_gauss[gain];
   return 0;
}

/*** hmc5883l_config() -- configure HMC5883L sensor

Performs initialization and configuration on HMC5883L sensor.

Arguments:
   fd -- descriptor returned by previous call to hmc5883l_open()
   cfg -- pointer to structure containing desired configuration, or
      NULL to use the default configuration from _hmc5883l_config_default

Returns 0 on success or non-0 on error.
***/
int hmc5883l_config(const int fd, const hmc5883l_config_t * const cfg) {
   const hmc5883l_config_t * mycfg = cfg;
   TSTA(fd < 0);

   if(!mycfg) mycfg = &_hmc5883l_config_default;

   /* Set config register A for desired averaging and bias. We leave
      the output rate all-bits-zero since we're just going to be using
      single measurement mode: */
   if(i2c_smbus_write_byte_data(fd, REG_CRA, (mycfg->avg)<<5 | (mycfg->bias)))
      return sysdiag("i2c_smbus_write_byte_data", "can't write to CRA");

   /* Set config register B for desired gain: */
   if(i2c_smbus_write_byte_data(fd, REG_CRB, (mycfg->gain)<<5))
      return sysdiag("i2c_smbus_write_byte_data", "can't write to CRB");

   /* Set mode register to force idle mode: */
   if(i2c_smbus_write_byte_data(fd, REG_MR, 0x03))
      return sysdiag("i2c_smbus_write_byte_data", "can't write to Mode");

   /* If the gain was changed, the next reading will use the old gain.
      Perform a reading and discard the results so the caller doesn't
      have to worry about this. */
   return hmc5883l_read(fd, NULL, mycfg->gain);
}
                                        compass-read/main.c                                                                                 0000644 0001750 0001750 00000001257 12305426361 013501  0                                                                                                    ustar   henke                           henke                                                                                                                                                                                                                  /* This is free software. See COPYING for details. */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "diag.h"
#include "debug.h"
#include "hmc5883l.h"

int main(void) {
   int fd, i;
   hmc5883l_vec3_t v;
   hmc5883l_config_t cfg;

   memcpy(&cfg, &_hmc5883l_config_default, sizeof(cfg));
   cfg.avg = AVG_8;
   //cfg.gain = GAIN_1370;

   if((fd = hmc5883l_open()) < 0) return -1;
   if(hmc5883l_config(fd, &cfg)) return -1;

   for(i = 0; i < 100000; i++) {
      if(hmc5883l_read(fd, &v, cfg.gain)) return -1;
      printf("%lf %lf %lf\n", v.x, v.y, v.z);
   }

   if(close(fd)) return sysdiag("close", "can't close I2C bus");
   return 0;
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 