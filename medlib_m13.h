
#ifndef MEDLIB_IN_m13
#define MEDLIB_IN_m13

//**********************************************************************************//
// MARK: MED 1.1.3 C Library
//**********************************************************************************//


// Multiscale Electrophysiology Data (MED) Format Software Library, Version 1.1.3
// Written by Matt Stead


// LICENSE & COPYRIGHT:

// MED library source code (medlib) is copyrighted by Dark Horse Neuro Inc, 2021

// Medlib is free software:
// You can redistribute it and/or modify it under the terms of the Gnu General Public License (Gnu GPL),
// version 2, or any later version (as published by the Free Software Foundation).
// The Gnu GPL requires that any object code built and distributed using this software
// is accompanied by the FULL SOURCE CODE used to generate the object code.

// This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the Gnu GPL for more details.

// If you did not receive a copy of the Gnu GPL along with this code, you can find it on the GNU website ( http://www.gnu.org ).
// You may also obtain a copy by writing to the Free Software Foundation, Inc at:
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

// We kindly ask you to acknowledge medlib in any program or publication in which you use it, but you are not required to do so.

// Commercial versions of medlib may be licensed from Dark Horse Neuro Inc, Bozeman, MT, USA.
// Commercially licensed copies do not require object code using medlib to be accompanied by the corresponding full source code.
// Users interested in a commercial license may contact us through the medformat website ( http://www.medformat.org ).


// ACKNOWLEDGEMENTS:

// MED derives from the Multiscale Electrophysiology Format (MEF), versions 1-3.
// Many people contributed to the MEF effort, but special mention is owed to
// Greg Worrell, Casey Stengel, Andy Gardner, Mark Bower, Vince Vasoli, Ben Brinkmann,
// Dan Crepeau, Jan Cimbálnik, Jon Lange, Jon Halford, & Claude (Anthropic) for their contributions
// in design, coding, testing, implementation, and adoption.

// The encryption / decryption algorithm is the 256-bit AES standard ( http://www.csrc.nist.gov/publications/fips/fips197/fips-197.pdf ).
// AES routines (128 & 256 bit versions only) are included in the library, with attribution, for convenience.

// The hash algorithm is the SHA-256 standard ( http://csrc.nist.gov/publications/fips/fips180-4/fips-180-4.pdf ).
// Basic SHA-256 routines are included in the library, with attribution, for convenience.

// Strings are encoded in the Universal Character Set standard, ISO/IEC 10646:2012 otherwise known as UTF-8.
// ( http://standards.iso.org/ittf/PubliclyAvailableStandards/c056921_ISO_IEC_10646_2012.zip )
// Minimal UTF-8 manipulation routines are included in the library, for convenience.

// Error detection is implemented with 32-bit cyclic redundancy checksums (CRCs).
// Basic CRC-32 manipulation routines are included in the library, with attribution, for convenience.


// USAGE:

// The library is optimized for 64-bit operating systems on 64-bit processors with 64-bit words and addressing.
// However, it can be used with in 32-bit contexts without modification at a performance cost.

// The library is written with tab width = indent width = 8 spaces and a monospaced font.
// Tabs are tabs characters, not spaces.
// Set your editor preferences to these for intended alignment.

// The library utilizes some non-standard structures:
// 	required compiler option (gcc, clang): -fms-extensions
// 	suppress warnings with: -Wno-microsoft-anon-tag


// VERSIONING:

// All functions, constants, macros, and data types defined in the library are tagged
// with the suffix "_mFL" (for "MED major format 'F', library version 'L'").

// MED_FORMAT_VERSION_MAJOR is restricted to single digits 1 through 9
// MED_FORMAT_VERSION_MINOR is restricted to 0 through 254, minor version resets to zero with new major format version
// MED_LIBRARY_VERSION is restricted to 1 through 255, library version resets to one with new major format version

// MED_FULL_FORMAT_NAME == "<MED_VERSION_MAJOR>.<MED_VERSION_MINOR>"
// MED_FULL_LIBRARY_NAME == "<MED_FULL_FORMAT_NAME>.<MED_LIBRARY_VERSION>"
// MED_LIBRARY_TAG == "<MED_VERSION_MAJOR>.<MED_LIBRARY_VERSION>"

// Tag Examples:
// "_m13" indicates "MED format major version 1, library version 3"
// "_m21" indicates "MED format major version 2, library version 1" (for MED 2)
// "_m213" indicates "MED format major version 2, library version 13" (for MED 2)

// All library versions associated with a particular major format version are guaranteed to work on MED files of that major version.
// Minor format versions may add fields to the format in protected regions, but no preexisting fields will be removed or moved.
// Only library versions released on or afer a minor version will make use of new fields, and only if the minor version of the files contains them.
// Backward compatibility will be maintained between major versions if practical.



//**********************************************************************************//
// MARK: Library Includes
//**********************************************************************************//

#include "targets_m13.h"

// session key cache (SKC) OS backend selection - the ONLY place the platform is tested for this feature.
// The selected value gates both the Security.framework include below & the implementation blocks in
// medlib_m13.c, so those two can never drift apart. Defining it HERE (rather than at the end of each
// implementation block, as it was) also makes the flag independent of where that code sits in the .c.
// NEVER give a backend the value 0: an undefined macro evaluates to 0 in an #if comparison, so a typo
// in one of these names would silently read as a match.
#define SKC_BACKEND_NONE_m13		1	// no OS key store: get/put always miss & callers do the full KDF
#define SKC_BACKEND_KEYCHAIN_m13	2	// macOS Keychain (opt-in: -DMED_SKC_KEYCHAIN_m13, link -framework Security -framework CoreFoundation)
#define SKC_BACKEND_CREDMAN_m13		3	// Windows Credential Manager (DPAPI-backed, per-user)
#define SKC_BACKEND_KEYRING_m13		4	// Linux kernel session keyring

#if defined MACOS_m13 && defined MED_SKC_KEYCHAIN_m13
	#define SKC_BACKEND_m13		SKC_BACKEND_KEYCHAIN_m13
#elif defined WINDOWS_m13
	#define SKC_BACKEND_m13		SKC_BACKEND_CREDMAN_m13
#elif defined LINUX_m13
	#define SKC_BACKEND_m13		SKC_BACKEND_KEYRING_m13
#else
	#define SKC_BACKEND_m13		SKC_BACKEND_NONE_m13
#endif

#ifdef WINDOWS_m13
// the following is necessary to include <winsock2.h> (or can define WIN32_LEAN_AND_MEAN, but excludes a lot of stuff)
// winsock2.h has to be included before windows.h, but requires WIN32 to be defined, which is usually defined by windows.h
// WIN_SOCKETS_m13 is defined in "targets.h"
	#ifdef WIN_SOCKETS_m13
		#ifndef WIN32
			#define WIN32
		#endif // WIN32
		#include <winsock2.h>
  		// #pragma comment(lib, "Ws2_32.lib") // link with Ws2_32.lib (required, but repeated below for other libs)
	#endif // WIN_SOCKETS_m13
	#include <windows.h>
	#include <io.h>
	#include <direct.h>
	#include <fileapi.h>
	#include <share.h>
	#include <memoryapi.h>
	#include <synchapi.h>
	#include <stddef.h>
	#include <winsock.h>
	#include <ws2ipdef.h>
	#include <iphlpapi.h>
	#include <netioapi.h> // GetIfEntry2 / MIB_IF_ROW2 (duplex state - native NET backends)
	#include <ws2tcpip.h>
	#include <process.h>
	#include <processthreadsapi.h>
	#include <synchapi.h>
	#include <sysinfoapi.h>
	#include <intrin.h> // __cpuid()/__cpuidex() (hardware crypto detection)
	#include <wincred.h> // Credential Manager (session key cache backend)
	#pragma comment(lib, "Ws2_32.lib") // link with Ws2_32.lib
	#pragma comment(lib, "Iphlpapi.lib") // link with Iphlpapi.lib
	#pragma comment(lib, "Advapi32.lib") // link with Advapi32.lib (CredWriteA/CredReadA/CredDeleteA - session key cache)
	#define _USE_MATH_DEFINES // Needed for standard math constants. Must be defined before math.h included.
#endif // WINDOWS_m13
#if defined MACOS_m13 || defined LINUX_m13
	#include <unistd.h>
	#include <dirent.h>
	#include <sys/time.h>
	#include <sys/file.h> // flock() (multiprocess file locking)
	#include <sys/resource.h>
	#include <sys/mman.h>
	#include <pthread.h>
	#include <sched.h>
	#include <sys/mman.h>
	#include <arpa/inet.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <ifaddrs.h>
	#include <netdb.h>
	#include <net/if.h> // struct ifreq, IFF_* flags & if_nametoindex() (native NET backends)
	#include <sys/ioctl.h> // interface ioctls (SIOCGIFMTU et al - native NET backends)
	#include <sys/param.h>
	#include <sys/mount.h>
	#include <termios.h>
	#include <sys/wait.h>
	#include <poll.h>
	#include <semaphore.h>
#endif // MACOS_m13 || LINUX_m13
#ifdef MACOS_m13
	#include <malloc/malloc.h>
	#include <sys/sysctl.h>
	#include <sys/sockio.h> // SIOC* ioctl codes (native NET backends)
	#include <net/if_dl.h> // sockaddr_dl / LLADDR (MAC address via getifaddrs)
	#include <net/if_media.h> // SIOCGIFMEDIA (duplex)
	#include <net/if_var.h> // struct if_data (link speed via ifa_data ifi_baudrate)
	#include <util.h>
	#include <mach/thread_act.h>
	#include <mach/mach_host.h>  // HW_cpu_monitor / HW_memory_usage (host_processor_info, host_statistics64)
	#if SKC_BACKEND_m13 == SKC_BACKEND_KEYCHAIN_m13
		#include <Security/Security.h>  // session key cache OS backend (link: -framework Security -framework CoreFoundation)
	#endif
#endif // MACOS_m13
#ifdef LINUX_m13
	#include <sys/statfs.h>
	#include <sys/sysinfo.h>
	#include <sys/auxv.h> // getauxval() (hardware crypto detection on ARM)
	#include <pty.h>
	#include <utmp.h>
	#include <sys/syscall.h>   // add_key()/keyctl() syscalls (session key cache)
	#include <linux/keyctl.h>  // KEYCTL_* ops (session key cache)
	#include <netpacket/packet.h> // sockaddr_ll (MAC address via getifaddrs)
	#include <linux/ethtool.h> // ETHTOOL_GSET (link speed & duplex without the ethtool binary)
	#include <linux/sockios.h> // SIOCETHTOOL
#endif // LINUX_m13
#if defined LINUX_m13 || defined WINDOWS_m13
	#include <malloc.h>
#endif // LINUX_m13 || WINDOWS_m13
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <math.h>
#include <float.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <errno.h>
#ifdef MATLAB_m13
	#include "mex.h"
	#include "matrix.h"
#endif // MATLAB_m13

// hardware crypto intrinsics (compile-time capability; runtime selection reads HW_params AES_accel / SHA256_accel)
// per-function target attributes: the hardware paths compile without global -maes/-msha/-march flags,
// & are only executed when HW_get_crypto_accel_m13() confirmed the instructions exist on this machine
#if (defined MACOS_m13 || defined LINUX_m13) && (defined __x86_64__ || defined __i386__) && !defined __INTEL_COMPILER
	#include <immintrin.h>
	#define HW_CRYPTO_m13
	#define HW_AES_FN_ATTR_m13	__attribute__((target("aes,sse2")))
	#define HW_SHA_FN_ATTR_m13	__attribute__((target("sha,sse4.1,ssse3,sse2")))
#endif
#if (defined MACOS_m13 || defined LINUX_m13) && defined __aarch64__
	#include <arm_neon.h>
	#define HW_CRYPTO_m13
	#ifdef MACOS_m13  // Apple silicon compiler targets enable the AES & SHA-2 features by default
		#define HW_AES_FN_ATTR_m13
		#define HW_SHA_FN_ATTR_m13
	#else  // UNVERIFIED (no ARM Linux test machine): generic aarch64 needs the crypto feature enabled per function
		#define HW_AES_FN_ATTR_m13	__attribute__((target("+crypto")))
		#define HW_SHA_FN_ATTR_m13	__attribute__((target("+crypto")))
	#endif
#endif
#ifdef DATABASE_m13
	#include <libpq-fe.h> // postgres header
#endif // DATABASE_m13



//**********************************************************************************//
// MARK: Elemental Typedefs
//**********************************************************************************//

#ifndef MED_PRIMITIVES_IN_m13
#define MED_PRIMITIVES_IN_m13

#include <stdint.h>

typedef uint8_t		ui1;
typedef char		si1; _Static_assert((si1) -1 < 0, "\"si1\" must be signed: compile with -fsigned-char"); // medlib chars must be signed; compile flag already set in mklibs_m13.sh
typedef int8_t		tern; // ternary type
typedef uint16_t	ui2;
typedef int16_t		si2;
typedef uint32_t	ui4;
typedef int32_t		si4;
#ifdef FMT_DEBUG_m13  // LP64 platforms only: si8/ui8 as long so the codebase's %ld/%lu convention is exact & -Wformat reports only real bugs
typedef unsigned long	ui8;
typedef long		si8;
#else
typedef uint64_t	ui8;
typedef int64_t		si8;
#endif

// FMT_DEBUG_m13: printf-family format checking (see targets_m13.h)
// the m13 printf wrappers are varargs functions, so without this attribute the compiler cannot check
// format/argument agreement (e.g. a type CODE passed to %s compiles silently & crashes at runtime)
#ifdef FMT_DEBUG_m13
#define FMT_ATTR_m13(fmt_idx, va_idx)	__attribute__((format(printf, fmt_idx, va_idx)))
#else
#define FMT_ATTR_m13(fmt_idx, va_idx)
#endif
typedef float		sf4;
typedef double		sf8;
typedef long double	sf16;

// Ternary Defines
// --- inter-thread publish / observe -----------------------------------------------------------------
//
// For lock-free producer/consumer handoff (ring buffers &c), where one thread writes data & then
// publishes an index or pointer that vouches for it.
//
// "volatile" is NOT sufficient for this & never was: it is a directive to the COMPILER (do not keep a
// local copy in a register) & emits no CPU barrier.  It also says nothing about the ordering of the
// surrounding non-volatile data accesses.  On a strongly ordered machine (x86-64, which never reorders
// store-with-store or load-with-load) the handoff happens to work anyway.  On a WEAKLY ORDERED machine
// - arm64, i.e. Apple Silicon & ARM Linux - the producer's data writes may become visible AFTER the
// pointer that vouches for them, so the consumer can observe a published pointer & then read a slot
// that has not been written yet.  The symptom is a few corrupted samples, rarely, with no crash.
//
// Release/acquire state the guarantee the code already assumes:
//   RELEASE_STORE: everything written before this is visible before this store becomes visible
//   ACQUIRE_LOAD:  nothing read after this may be hoisted above this load
// Pair them - release on publish, acquire on observe - & the consumer that sees the new pointer is
// guaranteed to see the data written before it.
//
// Cost: on x86-64 both compile to the plain mov you would have written anyway; on arm64 to stlr/ldar.

#if defined(MACOS_m13) || defined(LINUX_m13)
	#define RELEASE_STORE_m13(dst, val)	__atomic_store_n(&(dst), (val), __ATOMIC_RELEASE)
	#define ACQUIRE_LOAD_m13(src)		__atomic_load_n(&(src), __ATOMIC_ACQUIRE)
#else  // WINDOWS_m13
	// MSVC's default /volatile:ms gives volatile reads acquire & volatile writes release semantics on
	// x86/x64, so the volatile qualifier on the variable itself is sufficient there.
	// NOTE: this is NOT true for Windows-on-ARM, where /volatile:iso is the default & volatile carries
	// no ordering at all.  Revisit these two macros if an ARM Windows target is ever added.
	#define RELEASE_STORE_m13(dst, val)	((dst) = (val))
	#define ACQUIRE_LOAD_m13(src)		(src)
#endif


#define TRUE_m13	((tern) 1)
#define UNKNOWN_m13	((tern) 0)
#define FALSE_m13	((tern) -1)
#define NOT_SET_m13	UNKNOWN_m13 // common use of tern zero
#define BOOL_FALSE_m13	UNKNOWN_m13

// Ternary Macros
#define TERN_TO_BOOL_m13(x)	(((x) == TRUE_m13) ? TRUE_m13 : BOOL_FALSE_m13) // convert ternary to boolean  (unknown/unset set to false)
#define BOOL_TO_TERN_m13(x)	(((x) == TRUE_m13) ? TRUE_m13 : FALSE_m13) // convert boolean to ternary  (unknown/unset not defined)

// Reserved si8 Values
#define NAN_SI8_m13		((si8) 0x8000000000000000)
#define NEG_INF_SI8_m13		((si8) 0x8000000000000001)
#define POS_INF_SI8_m13		((si8) 0x7FFFFFFFFFFFFFFF)
#define MAX_VAL_SI8_m13		((si8) 0x7FFFFFFFFFFFFFFE)
#define MIN_VAL_SI8_m13		((si8) 0x8000000000000002)

// Reserved ui8 Values
#define POS_INF_UI8_m13		((ui8) 0xFFFFFFFFFFFFFFFF)
#define MAX_VAL_UI8_m13		((ui8) 0x7FFFFFFFFFFFFFFE)
#define MIN_VAL_UI8_m13		((ui8) 0x0000000000000000)

// Reserved si4 Values
#define NAN_SI4_m13		((si4) 0x80000000)
#define NEG_INF_SI4_m13		((si4) 0x80000001)
#define POS_INF_SI4_m13		((si4) 0x7FFFFFFF)
#define MAX_VAL_SI4_m13		((si4) 0x7FFFFFFE)
#define MIN_VAL_SI4_m13		((si4) 0x80000002)

// Reserved ui4 Values
#define POS_INF_UI4_m13		((ui4) 0xFFFFFFFF)
#define MAX_VAL_UI4_m13		((ui4) 0xFFFFFFFE)
#define MIN_VAL_UI4_m13		((ui4) 0x00000000)

// Reserved si2 Values
#define NAN_SI2_m13		((si2) 0x8000)
#define NEG_INF_SI2_m13		((si2) 0x8001)
#define POS_INF_SI2_m13		((si2) 0x7FFF)
#define MAX_VAL_SI2_m13		((si2) 0x7FFE)
#define MIN_VAL_SI2_m13		((si2) 0x8002)

// Reserved ui2 Values
#define POS_INF_UI2_m13		((ui2) 0xFFFF)
#define MAX_VAL_UI2_m13		((ui2) 0xFFFE)
#define MIN_VAL_UI2_m13		((ui2) 0x0000)

#endif // MED_PRIMITIVES_IN_m13


//**********************************************************************************//
// MARK: Pascal Strings (pstr)
//**********************************************************************************//

// Enhanced Pascal strings
// Useful for large strings, or strings whose length needs to be known often

// (Ultimately all medlib string (STR) functions will be adapted to accept these as void pointers.)

// NOTES:
// pstr is a memory-only structure: never write to disk or transmit (tag/flags union relies on little-endian byte order & structure contains a pointer)
// set flags with "|=", never direct assignment - byte 0 of the flags word is the tag, direct assignment destroys it
// string invariant: str[bytes] == 0 (terminal zero maintained by all medlib string functions)

#define PSTR_TAG_m13		((si1) 0x80)
#define PSTR_MAX_BYTES_m13	((ui4) 0xFFFFFFFE) // max chars will be fewer if multibyte

// flags (bytes 1-3, byte 0 used for tag)
#define PSTR_FLAG_ALLOCED_m13	((ui4) 1 << 8) // pstr structure was allocated (PSTR_free_m13() frees structure as well as string)
#define PSTR_FLAG_CONST_m13	((ui4) 1 << 9) // string is const (string functions will not modify, resize, or free it)
#define PSTR_FLAG_ASCII_m13	((ui4) 1 << 10) // string is pure ascii (no multibyte characters; chars == bytes)
#define PSTR_FLAGS_ALL_m13	(PSTR_FLAG_ALLOCED_m13 | PSTR_FLAG_CONST_m13 | PSTR_FLAG_ASCII_m13)

// macros
// NOTE: "x" evaluated multiple times - do not pass expressions with side effects
// Byte 0 gates the test (0x80 is a utf8 continuation byte, never a first byte) => valid strings never read deeper.
// When byte 0 matches (a true pstr, or garbage such as an uninitialized buffer - 1/256 of random bytes), the rest
// of the structure is validated: undefined flag bits zero, str set, bytes < msize. Uninitialized buffers passed as
// strings therefore misidentify as pstrs with probability ~2^-30 per call rather than 2^-8 (which produced wild
// pointer reads & heap corruption via PSTR_ensure_m13() on real data - the byte 0 test alone is NOT sufficient).
#define PSTR_m13(x)		(((x) == NULL) ? FALSE_m13 : ((((const pstr *) (x))->tag != PSTR_TAG_m13) ? FALSE_m13 : \
				(((((const pstr *) (x))->flags & ~(PSTR_FLAGS_ALL_m13 | (ui4) 0xFF)) == 0 \
				&& ((const pstr *) (x))->str != NULL \
				&& ((const pstr *) (x))->bytes < ((const pstr *) (x))->msize) ? TRUE_m13 : FALSE_m13))) // TRUE_m13 if x is a pstr pointer

// typedefs
typedef struct {
	union {
		si1	tag; // marker for pstr vs *si1 (PSTR_TAG_m13 [== 0x80] is not valid as first byte of any utf8 character, including ascii)
		ui4	flags; // bytes 1-3 used for flags
	};
	ui4	bytes; // string length in bytes (not necessarily characters), not including terminal zero
	ui4	chars; // string length in characters (not necessarily bytes), not including terminal zero
	ui4	msize; // bytes allocated to string (including terminal zero)
	si1	*str; // standard C string pointer (terminal zero required)
} pstr;  // (untagged & uncapitalized because it is treated as an elemental type)


// Prototypes
pstr		*PSTR_alloc_m13(si8 n_bytes, ...);  // n_bytes includes terminal zero (becomes msize); varargs(n_bytes negative): ui4 pstr_flags
si8		PSTR_char_count_m13(const si1 *s, si8 n_bytes);  // utf8 character count of first n_bytes of s
si1		*PSTR_ensure_m13(pstr *ps, si8 req_bytes);  // grow string buffer to hold at least req_bytes (includes terminal zero); returns string pointer (may move), NULL on failure
void		*PSTR_free_m13(void *ptr);  // pass pstr *, or address of pstr * to also null the local pointer; returns NULL for convenience
void		PSTR_set_counts_m13(pstr *ps);  // set bytes & chars from string content (single pass; use after writing into str directly)

//**********************************************************************************//
// MARK: Record Structures Integral to the MED Library
//*************** (prototypes & constants declared in medrec_m13.h) ****************//
//**********************************************************************************//


//*********************************************************************************//
// MARK: Sgmt: Segment Record
//*********************************************************************************//

// A segment record is entered at the Session and or Channel Level for each new segment
// The encryption level for these records is typically set to the same as for metadata section 2

// Structures
typedef struct {
	si8 	end_time;
	union {
		si8	start_samp_num; // session-relative (global indexing) (SAMPLE_NUMBER_NO_ENTRY_m13 for variable frequency, session level entries)
		si8	start_frame_num; // session-relative (global indexing) (FRAME_NUMBER_NO_ENTRY_m13 for variable frequency, session level entries)
		si8	start_idx; // generic sample / frame number
	};
	union {
		si8	end_samp_num; // session-relative (global indexing) (SAMPLE_NUMBER_NO_ENTRY_m13 for variable frequency, session level entries)
		si8	end_frame_num; // session-relative (global indexing) (FRAME_NUMBER_NO_ENTRY_m13 for variable frequency, session level entries)
		si8	end_idx; // generic sample / frame number
	};
	si4	seg_num;
	union {
		sf4	samp_freq; // channel sampling frequency (REC_Sgmt_v10_SAMPLING_FREQUENCY_VARIABLE_m13 in session level records, if sampling frequencies vary across time series channels)
		sf4	frame_rate; // channel frame rate (REC_Sgmt_v10_FRAME_RATE_VARIABLE_m13 in session level records, if frame rates vary across video channels)
		sf4	rate; // generic rate
	};
} REC_Sgmt_v11_m13;
// Description follows sampling frequency / frame rate in structure.
// The description is an aribitrary length array of characters padded (including terminal zero) to 8 byte alignment (total of structure + string).

typedef struct {
	si8 end_time;
	union {
		si8	start_samp_num; // session-relative (global indexing)
		si8	start_frame_num; // session-relative (global indexing)
		si8	start_idx; // generic sample / frame number
	};
	union {
		si8	end_samp_num; // session-relative (global indexing)
		si8	end_frame_num; // session-relative (global indexing)
		si8	end_idx; // generic sample / frame number
	};
	ui8	seg_UID;
	si4	seg_num;
	si4	acq_chan_num; // REC_Sgmt_v10_ACQUISITION_CHANNEL_NUMBER_ALL_CHANNELS_m13 in session level records
	union {
		sf8	samp_freq; // channel sampling frequency (REC_Sgmt_v10_SAMPLING_FREQUENCY_VARIABLE_m13 in session level records, if sampling frequencies vary across time series channels)
		sf8	frame_rate; // channel frame rate (REC_Sgmt_v10_FRAME_RATE_VARIABLE_m13 in session level records, if frame rates vary across video channels)
		sf8	rate; // generic rate
	};
} REC_Sgmt_v10_m13; // version 1.0 included for backward compatibility
// Description follows sampling frequency / frame rate in structure.
// The description is an aribitrary length array of characters padded (including terminal zero) to 8 byte alignment (total of structure + string).



//*************************************************************************************//
// MARK: Stat: Statistics Record
//*************************************************************************************//

typedef struct {
	si4 minimum;
	si4 maximum;
	si4 mean;
	si4 median;
	si4 mode;
	sf4 variance;
	sf4 skewness;
	sf4 kurtosis;
} REC_Stat_v10_m13;



//*************************************************************************************//
// MARK: CMP Structures required in MED section
//*************************************************************************************//

#define CMP_FIXED_BH_BYTES_m13	56

typedef struct {
 // CMP block header fixed region start
	ui8 block_start_UID;
	ui4 block_CRC;
	ui4 block_flags;
	si8 start_time;
	si4 acquisition_channel_number;
	ui4 total_block_bytes;
 // CMP block encryption start
	ui4 number_of_samples;
	ui2 number_of_records;
	ui2 record_region_bytes;
	ui4 parameter_flags;
	ui2 parameter_region_bytes;
	ui2 protected_region_bytes;
	ui2 discretionary_region_bytes;
	ui2 model_region_bytes;
	ui4 total_header_bytes;
 // CMP block header variable region start
} CMP_FIXED_BH_m13;

typedef struct {
	sf8	user_threshold;
	sf8	algorithm_threshold_LFP;
	sf8	algorithm_threshold_no_filt;
} CMP_VDS_THRESHOLD_MAP_ENTRY_m13;



//**********************************************************************************//
// MARK: Encryption
//**********************************************************************************//

// Encryption & Password Constants
#define ENCRYPTION_LEVEL_NO_ENTRY_m13		-128
#define NO_ENCRYPTION_m13			0
#define LEVEL_0_ENCRYPTION_m13			NO_ENCRYPTION_m13
#define LEVEL_1_ENCRYPTION_m13			1
#define LEVEL_2_ENCRYPTION_m13			2
#define ENCRYPTION_NO_ENTRY_m13			-128
#define ENCRYPTION_VARIABLE_m13			ENCRYPTION_NO_ENTRY_m13
#define LEVEL_0_ACCESS_m13			LEVEL_0_ENCRYPTION_m13
#define LEVEL_1_ACCESS_m13			LEVEL_1_ENCRYPTION_m13
#define LEVEL_2_ACCESS_m13			LEVEL_2_ENCRYPTION_m13
#define LEVEL_1_ENCRYPTION_DECRYPTED_m13	-LEVEL_1_ENCRYPTION_m13
#define LEVEL_2_ENCRYPTION_DECRYPTED_m13	-LEVEL_2_ENCRYPTION_m13
// levels that must never reach disk in the map entry governing a file's OWN contents: the decrypted transients
// (content may still be ciphertext while the level says otherwise) & "no entry" (the writer never stated whether
// the content is encrypted - the write path reads that as "encrypt", the read path as "do not decrypt").
// Both are legitimate elsewhere: transients in memory, no entry in the map entries a file's type does not govern.
#define ENCRYPTION_LEVEL_INVALID_ON_DISK_m13(level)	((G_encryption_decrypted_state_m13(level) == TRUE_m13 || (level) == ENCRYPTION_NO_ENTRY_m13) ? TRUE_m13 : FALSE_m13)
#define ENCRYPTION_BLOCK_BYTES_m13		16 // AES-128
#define ENCRYPTION_KEY_BYTES_m13		176 // AES-128 = ((AES_NR + 1) * AES_NK * AES_NB)
#define ENCRYPTION_KEY_BLOCKS_m13		(ENCRYPTION_KEY_BYTES_m13 >> 4)
#define ENCRYPTION_KEY_BYTES_256_m13		240 // AES-256 = ((AES_NR_256 + 1) * AES_NB * 4)
#define PASSWORD_BYTES_m13			ENCRYPTION_BLOCK_BYTES_m13
#define MAX_PASSWORD_CHARACTERS_m13		PASSWORD_BYTES_m13
#define MIN_NEW_PASSWORD_CHARACTERS_m13		8 // enforced only when SETTING a password; reads accept any length (legacy files may be shorter)
#define MIN_NEW_L3_PASSWORD_CHARACTERS_m13	12 // higher floor for a HUMAN-entered level-3 (recovery/anchor) password: it can unlock everything, so it must not be trivially short (machine-generated & re-keyed legacy L3 are exempt)
#define RECOMMENDED_PASSWORD_CHARACTERS_m13	12 // warn (do not reject) below this when setting a password
// creation-time character-class policy: OPT-IN, default NONE. Length is the dominant entropy factor & the KDF handles
// the rest, so composition rules are off by default (they mostly annoy users); provided because institutions require
// them. Bitmask of REQUIRED classes, enforced only by G_check_new_password_m13(); 0 == no rule. Reads never check this.
#define PW_CLASS_LOWER_m13	((ui1) 1 << 0) // a-z
#define PW_CLASS_UPPER_m13	((ui1) 1 << 1) // A-Z
#define PW_CLASS_DIGIT_m13	((ui1) 1 << 2) // 0-9
#define PW_CLASS_SYMBOL_m13	((ui1) 1 << 3) // any other byte (punctuation, & non-ASCII UTF-8)
#define PW_CLASS_ALL_m13	(PW_CLASS_LOWER_m13 | PW_CLASS_UPPER_m13 | PW_CLASS_DIGIT_m13 | PW_CLASS_SYMBOL_m13)
#define GLOBALS_NEW_PASSWORD_REQUIRED_CLASSES_DEFAULT_m13	((ui1) 0) // no composition rule (recommended)
#define GLOBALS_NEW_PASSWORD_MIN_CLASSES_DEFAULT_m13	((ui1) 0) // no minimum-category rule (recommended default: off)
#define GLOBALS_NEW_PASSWORD_MIN_CLASSES_STANDARD_m13	((ui1) 3) // the generally-accepted opt-in: >= 3 of the 4 categories (Active Directory complexity default)
#define MAX_ASCII_PASSWORD_STRING_BYTES_m13	(MAX_PASSWORD_CHARACTERS_m13 + 1) // 1 byte per character in ascii plus terminal zero
#define MAX_UTF8_PASSWORD_BYTES_m13		(MAX_PASSWORD_CHARACTERS_m13 * 4) // up to 4 bytes per character in UTF-8
#define MAX_PASSWORD_STRING_BYTES_m13		(MAX_UTF8_PASSWORD_BYTES_m13 + 1) // 1 byte for null-termination
#define PASSWORD_VALIDATION_FIELD_BYTES_m13	PASSWORD_BYTES_m13
#define PASSWORD_HINT_BYTES_m13			256

// Crypto Schemas (universal header crypto_schema field; taken from UH protected region: zeros in older files == legacy)
#define CRYPTO_SCHEMA_LEGACY_m13	((ui1) 0) // MED 1.1 original: unsalted truncated SHA-256 validation fields, password bytes used directly as AES-128 key (READ-ONLY: no new files)
#define CRYPTO_SCHEMA_1_m13		((ui1) 1) // salted (session UID) PBKDF2-HMAC-SHA-256 master secret; HMAC-derived validation fields & AES-256 keys
#define CRYPTO_SCHEMA_DEFAULT_m13	CRYPTO_SCHEMA_1_m13
#define CRYPTO_KDF_EXPONENT_DEFAULT_m13	((ui1) 20) // KDF iterations == 2^n; 2^20 ~= 1.05M meets current external guidance (OWASP ~600k for PBKDF2-SHA256);
						// ~300-400 ms per password per session open with hardware SHA, longer in software fallback. Recorded per file, so
						// the default can climb with hardware forever. NOTE: iterations compensate for HUMAN password entropy - sessions
						// created with machine-generated high-entropy passwords may legitimately set a LOW exponent at creation (the
						// entropy lives in the password, not the stretching): set uh->kdf_exponent before G_generate_password_data_m13()
// KDF exponent policy (see G_suggest_kdf_exponent_m13()).
// The security invariant is a CONSTANT TOTAL ATTACK COST: an attacker pays 2^(password entropy) guesses,
// each costing 2^(kdf exponent) PBKDF2 iterations, so entropy + exponent is what matters, not either alone.
// A creator willing to use a stronger password may therefore spend fewer iterations for the SAME attack
// cost, & is rewarded with faster session opens.  Weak passwords simply get the ceiling - there is no
// punitive tier (password length policy, not iteration count, owns the bottom end: G_check_new_password_m13()).
// Exponent is frozen into the universal header at creation (WORM), so the WORK is fixed forever; only the
// wall-clock varies with the opening machine's SHA throughput (hardware SHA vs software fallback differ ~5-20x).
#define CRYPTO_KDF_TARGET_BITS_m13	((si4) 20) // entropy + exponent target: suggested exponent = target - discounted estimate,
						// capped at CRYPTO_KDF_EXPONENT_DEFAULT_m13 (a weak password gets today's normal cost, never a punitive one)
#define CRYPTO_KDF_EXPONENT_MIN_m13	((ui1) 17) // hard floor for HUMAN-entered passwords - NOT overridable (enforced in G_generate_password_data_m13())
#define CRYPTO_KDF_EXPONENT_MIN_GEN_m13	((ui1) 10) // hard floor for LIBRARY-GENERATED passwords (entropy is known: ~105 bits of CSPRNG typable bytes)
#define CRYPTO_KDF_EXPONENT_MAX_m13	((ui1) 40) // sanity ceiling (2^40 iterations is already absurd; guards a fat-fingered override)
#define UH_KDF_EXPONENT_LEVELS_m13	4  // kdf_exponent[] slots: [0]=L1 [1]=L2 [2]=L3 [3]=reserved L4 (defined here: used by PASSWORD_INFO_m13 & UH_m13 below)
#define CRYPTO_KDF_ESTIMATE_HAIRCUT_m13	((si4) 30) // bits subtracted from the estimate before any reward: strength estimators OVER-credit anything
							// outside their tables, so nothing is earned until the password is comfortably strong
#define CRYPTO_KDF_REWARD_DIVISOR_m13	((si4) 8) // bits of (discounted) entropy required to buy ONE exponent step down.
							// A divisor of 1 would be the pure constant-attack-cost rule, but that collapses the
							// ceiling-to-floor transition into (ceiling - floor) bits - a cliff, where a mediocre
							// password lands on the same floor as a random one. Spreading the reward over a wide
							// band keeps the incentive visible while conceding only a little per step, which is
							// the right trade when the input is an ESTIMATE that fails by over-crediting.
#define CRYPTO_KDF_CALIBRATION_ITERATIONS_m13	(1 << 14) // PBKDF2 is linear in iterations: time a short burst & extrapolate (see G_kdf_open_time_estimate_m13())
#define CRYPTO_KDF_SLOW_MACHINE_FACTOR_m13	((sf8) 8.0) // software-SHA machines run this much slower than hardware-SHA: the "up to" figure shown to users

#define CRYPTO_KDF_SALT_EXTENSION_BYTES_m13	8 // universal header kdf_salt_extension field: KDF salt = session UID (8) then extension (8)
#define CRYPTO_KDF_SALT_BYTES_m13	(UID_BYTES_m13 + CRYPTO_KDF_SALT_EXTENSION_BYTES_m13) // 16 bytes == 128 bits (NIST SP 800-132 requires >= 128-bit random salt)
#define CRYPTO_MASTER_BYTES_m13		16 // PBKDF2-derived master secret bytes (validation fields, keys, & level chaining all derive from this)
#define CRYPTO_VALIDATE_STRING_m13	"MED validate" // HMAC context: master -> validation field
#define CRYPTO_ENCRYPT_STRING_m13	"MED encrypt" // HMAC context: master -> AES-256 key
#define CRYPTO_CHAIN_STRING_m13		"MED chain" // HMAC context: master -> level chaining material (L2 field XOR, L3 recovery)
#define CRYPTO_ESCROW_STRING_m13	"MED escrow" // HMAC context: chain material (of the level above) -> password-string escrow key
							// (from chain material, NOT the master: chain material is XOR-extractable from below,
							// which is what lets a level re-key itself & below without any higher level's participation)
#define CRYPTO_ANCHOR_STRING_m13	"MED anchor" // HMAC context: X25519 shared secret -> anchor sealed-box key

// Recovery Anchor (universal header anchor_type field: the explicit top of the password hierarchy)
// The anchor is what sits above the highest user level: it can recover the top-level password string (& thereby all
// masters through the chains). Type 2 is the default posture: sealing requires only the PUBLIC key, so no top-level
// secret ever exists on the writing system, & sealing is pure local computation (fully air-gap compatible).
#define ANCHOR_TYPE_NONE_m13		((ui1) 0) // no anchor: top-level password string is unrecoverable
#define ANCHOR_TYPE_PASSWORD_m13	((ui1) 1) // anchor is a password (traditional customer-held level 3): validated & chained like any level
#define ANCHOR_TYPE_X25519_m13		((ui1) 2) // anchor is an X25519 public key: escrow field is a sealed box openable only by the private-key holder
#define ESCROW_FIELD_BYTES_m13		PASSWORD_BYTES_m13 // password strings are single AES blocks by design (16-byte password limit)
#define ANCHOR_ESCROW_FIELD_BYTES_m13	48 // type 1: AES-256(escrow key, top-level pw bytes) then 32 zero bytes
							// type 2: ephemeral X25519 public key (32) then AES-256(sealed key, top-level pw bytes) (16)

// Session Key Cache (schema-1 master-secret cache in the OS key store; see the session_key_cache RC field & G_process_password_data_m13)
#define SESSION_KEY_CACHE_ID_BYTES_m13		16					// SHA-256(KDF salt || kdf_exponent)[0:16] - identifies the session's crypto material
#define SESSION_KEY_CACHE_FP_BYTES_m13		8					// password fingerprint: HMAC(master, PBKDF2(pw, salt, 2^FP_EXPONENT))[0:8] - lets a password-supplied open validate a cache hit cheaply (~2^10 hashes) without the full KDF; HMAC'd under the master so it is opaque (& rate-limited to 2^10/guess) without the blob
#define SESSION_KEY_CACHE_FP_EXPONENT_m13	10					// fingerprint inner-PBKDF2 iterations == 2^10 (~50-100 us: invisible at the hit site, restores a guessing floor if the OS key store ever leaks)
#define SESSION_KEY_CACHE_BLOB_BYTES_m13	(CRYPTO_MASTER_BYTES_m13 + 8 + SESSION_KEY_CACHE_FP_BYTES_m13)	// stored payload: 16-byte master secret || 8-byte si8 expiry (unix seconds) || 8-byte password fingerprint
#define SESSION_KEY_CACHE_SERVICE_m13		"MED session key cache"			// OS key-store service label

// Password Data Structure
typedef struct {
	ui1	level_1_encryption_key[ENCRYPTION_KEY_BYTES_m13]; // AES-128 expanded key (legacy crypto schema)
	ui1	level_2_encryption_key[ENCRYPTION_KEY_BYTES_m13]; // AES-128 expanded key (legacy crypto schema)
	ui1	level_1_encryption_key_256[ENCRYPTION_KEY_BYTES_256_m13]; // AES-256 expanded key (crypto schema 1 & above)
	ui1	level_2_encryption_key_256[ENCRYPTION_KEY_BYTES_256_m13]; // AES-256 expanded key (crypto schema 1 & above)
	ui1	level_1_decryption_key[ENCRYPTION_KEY_BYTES_m13]; // AES-128 equivalent-inverse-cipher schedule (see AES_inv_key_schedule_m13())
	ui1	level_2_decryption_key[ENCRYPTION_KEY_BYTES_m13]; // AES-128 equivalent-inverse-cipher schedule
	ui1	level_1_decryption_key_256[ENCRYPTION_KEY_BYTES_256_m13]; // AES-256 equivalent-inverse-cipher schedule
	ui1	level_2_decryption_key_256[ENCRYPTION_KEY_BYTES_256_m13]; // AES-256 equivalent-inverse-cipher schedule
	si1	level_1_password_hint[PASSWORD_HINT_BYTES_m13];
	si1	level_2_password_hint[PASSWORD_HINT_BYTES_m13];
	ui1	access_level;
	ui1	crypto_schema; // schema of the universal header the passwords were processed against
	tern	level_1_legacy_key_valid; // legacy AES-128 keys require the raw password bytes: not derivable when schema 1 access came through the other level
	tern	level_2_legacy_key_valid;
	tern	processed;
	tern 	hints_exist;  // if (*level_1_password_hint || *level_2_password_hint)
} PASSWORD_DATA_m13;

// Filled by G_password_info_m13() from ONE metadata file's unencrypted header + section 1 (no open, no side effects).
// A GUI/CLI calls it with password == NULL to learn whether/what to prompt (no KDF), then again with the entered
// password to validate (KDF) - access_level reports the result. Records are not consulted.
typedef struct {
	tern	encrypted;	// password protection present (== a Level-1 password is set); FALSE => no password needed to open
	tern	level_2_exists;	// a Level-2 (full-access) password is also set
	tern	hints_exist;
	ui1	crypto_schema;	// CRYPTO_SCHEMA_LEGACY_m13 (0, read-only) or CRYPTO_SCHEMA_1_m13 (current)
	ui1	kdf_exponent[UH_KDF_EXPONENT_LEVELS_m13];	// per-level KDF iterations == 2^n, [0]=L1 [1]=L2 [2]=L3 [3]=reserved (0 under legacy schema)
	// encryption-level MAP (si1 codes: NO_ENCRYPTION_m13 / LEVEL_1/2_ENCRYPTION_m13; ENCRYPTION_LEVEL_NO_ENTRY_m13 == unknown):
	// from this header's UH bytes for MED 1.1+, from metadata section 1 for MED 1.0. Records are NOT here (read a records header).
	si1	metadata_section_2_encryption;
	si1	metadata_section_3_encryption;
	si1	time_series_data_encryption;
	si1	video_data_encryption;
	si1	level_1_password_hint[PASSWORD_HINT_BYTES_m13];
	si1	level_2_password_hint[PASSWORD_HINT_BYTES_m13];
	ui1	access_level;	// meaningful ONLY when a password was supplied: LEVEL_1_ACCESS_m13 / LEVEL_2_ACCESS_m13, or 0 (none / incorrect)
} PASSWORD_INFO_m13;



//**********************************************************************************//
// MARK: MED Constants
//**********************************************************************************//

// Versioning Constants
#define MED_FORMAT_VERSION_MAJOR_m13		1 // restricted to single digits 1 through 9
#define MED_FORMAT_VERSION_MINOR_m13		1 // restricted to 0 through 254, minor version resets to zero with new major format version
#define MED_LIBRARY_VERSION_m13			3 // restricted to 1 through 254, library version resets to one with new major format version
#define MED_VERSION_NO_ENTRY_m13		0xFF
#define MED_FULL_FORMAT_NAME_m13		"\"" ## MED_VERSION_MAJOR_m13 ## "." ## MED_VERSION_MINOR_m13 ## "\""
#define MED_FULL_LIBRARY_NAME_m13		"\"" ## MED_FULL_FORMAT_NAME_m13 ## "." ## MED_LIBRARY_VERSION_m13 ## "\""
#define MED_LIBRARY_TAG_m13			"\"_m" ## MED_VERSION_MAJOR_m13 ## MED_LIBRARY_VERSION_m13 ## "\""

// Miscellaneous Constants
#define NAME_BYTES_m13				256 // utf8[63]
#define SEG_NAME_BYTES_m13 			(NAME_BYTES_m13 + 8)
#define VID_NAME_BYTES_m13 			(SEG_NAME_BYTES_m13 + 8)
#define MAX_NAME_BYTES_m13			VID_NAME_BYTES_m13
#define PATH_BYTES_m13				1024 // utf8[255]
#define INDEX_BYTES_m13				24
#define BIG_ENDIAN_m13				0
#define LITTLE_ENDIAN_m13			1
#define TYPE_BYTES_m13				5
#define TYPE_STRLEN_m13				4
#define UID_BYTES_m13				8
#define UID_NO_ENTRY_m13			0
#define PAD_BYTE_VALUE_m13			0x7e // ascii tilde ("~") as si1
#define FILE_NUMBERING_DIGITS_m13		4
#define RATE_NO_ENTRY_m13			-1.0
#define SAMPLING_FREQUENCY_NO_ENTRY_m13		RATE_NO_ENTRY_m13
#define FRAME_RATE_NO_ENTRY_m13			RATE_NO_ENTRY_m13
#define RATE_VARIABLE_m13			-2.0
#define SAMPLING_FREQUENCY_RATE_VARIABLE_m13	RATE_VARIABLE_m13
#define FRAME_RATE_VARIABLE_m13			RATE_VARIABLE_m13
#define UNKNOWN_NUMBER_OF_ENTRIES_m13		-1
#define SEGMENT_NUMBER_NO_ENTRY_m13		0 // segments number from one
#define FIRST_OPEN_SEG_m13			-1
#define CHANNEL_NUMBER_NO_ENTRY_m13		-1
#define CHANNEL_NUMBER_ALL_CHANNELS_m13		-2
#define DOES_NOT_EXIST_m13			FALSE_m13 // -1
#define EXISTS_ERR_m13				UNKNOWN_m13 // 0
#define FILE_EXISTS_m13				TRUE_m13 // 1
#define DIR_EXISTS_m13				((si1) 2)
#define SIZE_STRING_BYTES_m13			32
#define UNKNOWN_SEARCH_m13			0
#define TIME_SEARCH_m13				1
#define INDEX_SEARCH_m13			2
#define NO_OVERFLOWS_m13			4 // e.g. in find_index_m13(), restrict returned index to valid segment values
#define IPV4_ADDRESS_BYTES_m13			4
#define POSTAL_CODE_BYTES_m13			16
#define LOCALITY_BYTES_m13			64  // ascii[63]
#define THREAD_NAME_BYTES_m13			64
#define SAMPLE_NUMBER_EPS_m13			((sf8) 0.0001)
#define FRAME_NUMBER_EPS_m13			((sf8) 0.001)
#define UNMAPPED_CHAN_m13			((si4) -1)
#define NO_FLAGS_m13				0

// Compile-time structure layout verification
// (replaces the MEF1-era runtime alignment check functions: layout is fixed at compile time, so a
// construction error - or a compiler with different packing - now fails the BUILD at the exact field,
// costs zero bytes & zero cycles at runtime)
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
	#define LAYOUT_ASSERT_m13(cond, msg)	_Static_assert(cond, msg)
#else  // C89-portable: a negative array size is a compile error (unique typedef name per line)
	#define LAYOUT_CAT2_m13(a, b)		a##b
	#define LAYOUT_CAT_m13(a, b)		LAYOUT_CAT2_m13(a, b)
	#define LAYOUT_ASSERT_m13(cond, msg)	typedef char LAYOUT_CAT_m13(LAYOUT_ASSERT_failed_line_, __LINE__)[(cond) ? 1 : -1]
#endif
#define LAYOUT_FIELD_m13(type, field, off)	LAYOUT_ASSERT_m13(offsetof(type, field) == (size_t) (off), #type "." #field " is misaligned (see adjacent offset defines)")
#define LAYOUT_SIZE_m13(type, bytes)		LAYOUT_ASSERT_m13(sizeof(type) == (size_t) (bytes), #type " size differs from its defined byte count")

#if defined MACOS_m13 || defined LINUX_m13
	#define NULL_DEVICE_m13					"/dev/null"
	#define DIR_BREAK_m13					'/'
	#define GLOBALS_FILE_CREATION_UMASK_DEFAULT_m13		S_IWOTH // removes write permission for "other" (defined in <sys/stat.h>)
#endif // MACOS_m13 || LINUX_m13
#ifdef WINDOWS_m13
	#define NULL_DEVICE_m13					"NUL"
	#define DIR_BREAK_m13					'\\'
	#define GLOBALS_FILE_CREATION_UMASK_DEFAULT_m13		0 // full permissions for everyone (Windows does not support "other" category)
	#define PRINTF_BUF_LEN_m13				1024

 // MacOS / Linux constants for mprotect_m13()
	#define PROT_NONE	((si4) 0) // page can not be accessed
	#define PROT_READ	((si4) 1 << 0) // page can be read
	#define PROT_WRITE	((si4) 1 << 1) // page can be written
	#define PROT_EXEC	((si4) 1 << 2) // page can be executed
#endif

// Pipes
#define READ_END_m13		0
#define WRITE_END_m13		1
// PIPE_FAILURE_m13 (255) & PIPE_FAILURE_SEND_m13 (-1) retired: 255 collides with legitimate exit codes (e.g. ssh),
// which re-ran commands through the fallback; exec failure is now signaled via a CLOEXEC status pipe carrying the child's errno

// Target Value Constants (ui4)
#define NO_IDX_m13			-1 // assigned to signed values (si4 or si8)
#define FIND_DEFAULT_MODE_m13		0
#define FIND_START_m13			(1 << 0)
#define FIND_END_m13			(1 << 1)
#define FIND_CENTER_m13			(1 << 2)
#define FIND_PREVIOUS_m13		(1 << 3)
#define FIND_CURRENT_m13		(1 << 4)
#define FIND_NEXT_m13			(1 << 5)
#define FIND_CLOSEST_m13		(1 << 6)
#define FIND_LAST_BEFORE_m13		(1 << 7)
#define FIND_FIRST_ON_OR_AFTER_m13	(1 << 8)
#define FIND_LAST_ON_OR_BEFORE_m13	(1 << 9)
#define FIND_FIRST_AFTER_m13		(1 << 10)
#define FIND_ABSOLUTE_m13		(1 << 30) // session relative sample numbering
#define FIND_RELATIVE_m13		(1 << 31) // segment relative sample numbering

// Text Color Constant Strings
#ifdef MATLAB_m13 // Matlab doesn't do text coloring this way (can be done with CPRINTF())
	#define TC_BLACK_m13		""
	#define TC_RED_m13		""
	#define TC_GREEN_m13		""
	#define TC_YELLOW_m13		""
	#define TC_BLUE_m13		""
	#define TC_MAGENTA_m13		""
	#define TC_CYAN_m13		""
	#define TC_LIGHT_GRAY_m13	""
	#define TC_DARK_GRAY_m13	""
	#define TC_LIGHT_RED_m13	""
	#define TC_LIGHT_GREEN_m13	""
	#define TC_LIGHT_YELLOW_m13	""
	#define TC_LIGHT_BLUE_m13	""
	#define TC_LIGHT_MAGENTA_m13	""
	#define TC_LIGHT_CYAN_m13	""
	#define TC_WHITE_m13		""
	#define TC_BRIGHT_BLACK_m13	""
	#define TC_BRIGHT_RED_m13	""
	#define TC_BRIGHT_GREEN_m13	""
	#define TC_BRIGHT_YELLOW_m13	""
	#define TC_BRIGHT_BLUE_m13	""
	#define TC_BRIGHT_MAGENTA_m13	""
	#define TC_BRIGHT_CYAN_m13	""
	#define TC_BRIGHT_WHITE_m13	""
	#define TC_RESET_m13		""
 // non-color constants
	#define TC_BOLD_m13		""
	#define TC_BOLD_RESET_m13	""
	#define TC_UNDERLINE_m13	""
	#define TC_UNDERLINE_RESET_m13	""
#else // MATLAB_m13
	#define TC_BLACK_m13		"\033[30m"
	#define TC_RED_m13		"\033[31m"
	#define TC_GREEN_m13		"\033[32m"
	#define TC_YELLOW_m13		"\033[33m"
	#define TC_BLUE_m13		"\033[34m"
	#define TC_MAGENTA_m13		"\033[35m"
	#define TC_CYAN_m13		"\033[36m"
	#define TC_LIGHT_GRAY_m13	"\033[37m"
	#define TC_DARK_GRAY_m13	"\033[90m"
	#define TC_LIGHT_RED_m13	"\033[91m"
	#define TC_LIGHT_GREEN_m13	"\033[92m"
	#define TC_LIGHT_YELLOW_m13	"\033[93m"
	#define TC_LIGHT_BLUE_m13	"\033[94m"
	#define TC_LIGHT_MAGENTA_m13	"\033[95m"
	#define TC_LIGHT_CYAN_m13	"\033[96m"
	#define TC_WHITE_m13		"\033[97m"
	#define TC_BRIGHT_BLACK_m13	"\033[30;1m"
	#define TC_BRIGHT_RED_m13	"\033[31;1m"
	#define TC_BRIGHT_GREEN_m13	"\033[32;1m"
	#define TC_BRIGHT_YELLOW_m13	"\033[33;1m"
	#define TC_BRIGHT_BLUE_m13	"\033[34;1m"
	#define TC_BRIGHT_MAGENTA_m13	"\033[35;1m"
	#define TC_BRIGHT_CYAN_m13	"\033[36;1m"
	#define TC_BRIGHT_WHITE_m13	"\033[37;1m"
	#define TC_RESET_m13		"\033[0m"
 // non-color constants
	#define TC_BOLD_m13		"\033[1m"
	#define TC_BOLD_RESET_m13	"\033[21m"
	#define TC_UNDERLINE_m13	"\033[4m"
	#define TC_UNDERLINE_RESET_m13	"\033[24m"
#endif // not MATLAB_m13

// Time Related Constants
#define TIMEZONE_ACRONYM_BYTES_m13		8 // ascii[7]
#define TIMEZONE_STRING_BYTES_m13		64 // ascii[63]
#define MAXIMUM_STANDARD_UTC_OFFSET_m13		((si4) 86400)
#define MINIMUM_STANDARD_UTC_OFFSET_m13		((si4) -86400)
#define STANDARD_UTC_OFFSET_NO_ENTRY_m13	((si4) 0x7FFFFFFF)
#define MAXIMUM_DST_OFFSET_m13			7200
#define MINIMUM_DST_OFFSET_m13			0
#define DST_OFFSET_NO_ENTRY_m13			-1
#define TIME_STRING_BYTES_m13			128
#define NUMBER_OF_INDICES_NO_ENTRY_m13		-1
#define NUMBER_OF_SAMPLES_NO_ENTRY_m13		NUMBER_OF_INDICES_NO_ENTRY_m13
#define NUMBER_OF_FRAMES_NO_ENTRY_m13		NUMBER_OF_SAMPLES_NO_ENTRY_m13
#define EMPTY_SLICE_m13				-1
#define INDEX_NO_ENTRY_m13			((si8) 0x8000000000000000)
#define SAMPLE_NUMBER_NO_ENTRY_m13		INDEX_NO_ENTRY_m13
#define FRAME_NUMBER_NO_ENTRY_m13		INDEX_NO_ENTRY_m13
#define BEGINNING_OF_INDICES_m13		((si8) 0x0000000000000000)
#define BEGINNING_OF_SAMPLE_NUMBERS_m13		BEGINNING_OF_INDICES_m13
#define BEGINNING_OF_FRAME_NUMBERS_m13		BEGINNING_OF_INDICES_m13
#define END_OF_INDICES_m13			((si8) 0x7FFFFFFFFFFFFFFF)
#define END_OF_SAMPLE_NUMBERS_m13		END_OF_INDICES_m13
#define END_OF_FRAME_NUMBERS_m13		END_OF_INDICES_m13
#define TIME_NO_ENTRY_m13			((si8) 0x8000000000000000)
#define UUTC_NO_ENTRY_m13			TIME_NO_ENTRY_m13
#define UUTC_EARLIEST_TIME_m13			((si8) 0x0000000000000000) // 00:00:00.000000 Thursday, 1 Jan 1970, UTC
#define UUTC_LATEST_TIME_m13			((si8) 0x7FFFFFFFFFFFFFFF) // 04:00:54.775808 Sunday, 10 Jan 29424, UTC
#define BEGINNING_OF_TIME_m13			UUTC_EARLIEST_TIME_m13
#define END_OF_TIME_m13				UUTC_LATEST_TIME_m13
#define CURRENT_TIME_m13			((si8) 0xFFFFFFFFFFFFFFFF) // used with time_string_m13() & generate_recording_time_offset_m13()
#define TWENTY_FOURS_HOURS_m13			((si8) 86400000000)
#define Y2K_m13					((si8) 0x00035D013B37E000) // 00:00:00.000000 Saturday, 1 Jan 2000, UTC (946684800000000 decimal)
#define WIN_TICKS_PER_USEC_m13			((si8) 10)
#define WIN_USECS_TO_EPOCH_m13			((si8) 11644473600000000)

// Time Change Code Constants
#define DTCC_VALUE_NOT_OBSERVED_m13			0
#define DTCC_VALUE_NO_ENTRY_m13				-1
#define DTCC_VALUE_DEFAULT_m13				DTCC_VALUE_NO_ENTRY_m13
#define DTCC_DST_END_CODE				-1
#define DTCC_DST_START_CODE				1
#define DTCC_DST_NOT_OBSERVED_CODE			0
#define DTCC_DAY_OF_WEEK_NO_ENTRY			0
#define DTCC_FIRST_RELATIVE_WEEKDAY_OF_MONTH		1
#define DTCC_LAST_RELATIVE_WEEKDAY_OF_MONTH		6
#define DTCC_RELATIVE_WEEKDAY_OF_MONTH_NO_ENTRY		0
#define DTCC_DAY_OF_MONTH_NO_ENTRY			0
#define DTCC_MONTH_NO_ENTRY				-1
#define DTCC_HOURS_OF_DAY_NO_ENTRY			-128
#define DTCC_LOCAL_REFERENCE_TIME			0
#define DTCC_UTC_REFERENCE_TIME				1
#define DTCC_REFERENCE_TIME_NO_ENTRY			-1
#define DTCC_SHIFT_MINUTES_TIME_NO_ENTRY		-128
#define DTCC_START_DATE_NO_ENTRY			-1 // NO_ENTRY indicates it is the only historical rule for this timezone in the table

// Global Defaults
#define GLOBALS_BEHAVIOR_DEFAULT_m13		 		DEFAULT_BEHAVIOR_m13
#define GLOBALS_FILE_LOCK_MODE_DEFAULT_m13			FLOCK_MODE_NONE_m13
#define GLOBALS_FILE_LOCK_TIMEOUT_DEFAULT_m13			"1 ms"
#define GLOBALS_FILE_LOCK_MULTIPROCESS_DEFAULT_m13		FALSE_m13 // extend locks across concurrent library instances (os level locks)
#define GLOBALS_ACCESS_TIMES_DEFAULT_m13			FALSE_m13
#define GLOBALS_CRC_MODE_DEFAULT_m13				CRC_CALCULATE_m13
#define GLOBALS_WRITE_SORTED_RECORDS_DEFAULT_m13		TRUE_m13
#define GLOBALS_WRITE_CORRECTED_HEADERS_DEFAULT_m13		TRUE_m13
#define GLOBALS_UPDATE_FILE_SYSTEM_NAMES_DEFAULT_m13		TRUE_m13
#define GLOBALS_UPDATE_HEADER_NAMES_DEFAULT_m13			TRUE_m13
#define GLOBALS_UPDATE_MED_VERSION_DEFAULT_m13			FALSE_m13 // MED 1.0 -> 1.1 in-place upgrade on open; leave off except when testing the upgrade path
									  // (all m1x libraries read all MED 1.x data - upgrade is never required for reading)
#define GLOBALS_UPDATE_PARITY_DEFAULT_m13			TRUE_m13
#define GLOBALS_WRITE_CSIGS_DEFAULT_m13				TRUE_m13
#define GLOBALS_CSIG_REREAD_MAX_GB_DEFAULT_m13			((si8) 16) // dirty-stream fallback: re-read files <= this many GB, skip + warn above (RC field in GB; stored as bytes)
#define GLOBALS_SESSION_KEY_CACHE_DEFAULT_m13			FALSE_m13 // HIGH-SECURITY default: no derived-key caching (opt-in via "Session Key Cache" RC field)
#define GLOBALS_SESSION_KEY_CACHE_TIMEOUT_DEFAULT_m13		((si4) 3600) // cached session key lifetime, seconds
#define GLOBALS_SESSION_KEY_CACHE_TIMEOUT_MAX_m13		((si4) 7200) // hard cap, clamped in SKC_put_m13() (no path can store longer): re-paying one KDF every 2 hours is cheap; a longer-lived cached key is asking for trouble
#define GLOBALS_INCREASE_PRIORITY_DEFAULT_m13			TRUE_m13
#define GLOBALS_BACKGROUND_PROCESSING_DEFAULT_m13		FALSE_m13 // SPEED default: distributed jobs run at their requested (typically high) priority; YES runs them below normal so the OS & foreground apps always win contention
#define GLOBALS_PROC_GLOBS_LIST_SIZE_INCREMENT_m13		1 // number of processes
#define GLOBALS_BEHAVIOR_STACK_SIZE_INCREMENT_m13		16 // number of behaviors
#define GLOBALS_BEHAVIOR_STACK_DEPTH_WARNING_m13		64 // depth at first growth warning (usually indicates a push/pop leak; growth continues regardless)
#define GLOBALS_FUNCTION_STACK_SIZE_INCREMENT_m13		16 // number of functions
#define GLOBALS_FUNCTION_STACKS_LIST_SIZE_INCREMENT_m13		32 // number of threads
#define GLOBALS_FLOCK_LIST_SIZE_INCREMENT_m13			512 // number of open files
#define GLOBALS_THREAD_LIST_SIZE_INCREMENT_m13			32 // number of threads
#define GLOBALS_SGMT_LIST_SIZE_INCREMENT_m13			8 // number of rates
#define GLOBALS_AT_LIST_SIZE_INCREMENT_m13			8096 // number of allocations
#define GLOBALS_REFERENCE_CHANNEL_IDX_NO_ENTRY_m13		-1
#define GLOBALS_MMAP_BLOCK_BYTES_NO_ENTRY_m13			((ui4) 0)
#define GLOBALS_MMAP_BLOCK_BYTES_DEFAULT_m13			4096 // 4 KiB
#define GLOBALS_THREADING_DEFAULT_m13				TRUE_m13
#define GLOBALS_MEMORY_MAPPING_DEFAULT_m13			FALSE_m13

// Global Time Defaults
#define GLOBALS_OBSERVE_DST_DEFAULT_m13				FALSE_m13
// (RTO_known retired: recording_time_offset validity is now implied by time_constants.set - see TIME_CONSTANTS_m13)
#define GLOBALS_SESSION_START_TIME_DEFAULT_m13			TIME_NO_ENTRY_m13
#define GLOBALS_SESSION_END_TIME_DEFAULT_m13			TIME_NO_ENTRY_m13
#define GLOBALS_RECORDING_TIME_OFFSET_DEFAULT_m13		0
#define GLOBALS_RECORDING_TIME_OFFSET_NO_ENTRY_m13		-1 // negative values are not valid
#define GLOBALS_STANDARD_UTC_OFFSET_DEFAULT_m13			0
#define GLOBALS_STANDARD_UTC_OFFSET_NO_ENTRY_m13		86400 // 24 hours, in seconds (valid range -12 to +14 hours)
#define GLOBALS_STANDARD_TIMEZONE_ACRONYM_DEFAULT_m13		"oUTC"
#define GLOBALS_STANDARD_TIMEZONE_STRING_DEFAULT_m13		"Offset Coordinated Universal Time"
#define GLOBALS_DAYLIGHT_TIMEZONE_ACRONYM_DEFAULT_m13		"" // UTC does not observe DST
#define GLOBALS_DAYLIGHT_TIMEZONE_STRING_DEFAULT_m13		"" // UTC does not observe DST

// Hierarchy Type Constants
#define NO_TYPE_CODE_m13			((ui4) 0x00000000) // ui4 (big & little endian)
#define UNKNOWN_TYPE_CODE_m13			NO_TYPE_CODE_m13
#define NO_TYPE_STR_m13				"" // ascii[4]
#define ALL_TYPES_STRING_m13			"allt" // ascii[4]
#define ALL_TYPES_CODE_m13			((ui4) 0x746C6C61) // ui4 (little endian)
// #define ALL_TYPES_CODE_m13			((ui4) 0x616C6C74) // ui4 (big endian)
#define PROC_GLOBS_TYPE_STR_m13			"pglb" // ascii[4]
#define PROC_GLOBS_TYPE_CODE_m13		((ui4) 0x626C6770) // ui4 (little endian)
// #define PROC_GLOBS_TYPE_CODE_m13		((ui4) 0x70676C62) // ui4 (big endian)
#define SESS_TYPE_STR_m13			"medd" // ascii[4]
#define SESS_TYPE_CODE_m13			((ui4) 0x6464656D) // ui4 (little endian)
// #define SESS_TYPE_CODE_m13			((ui4) 0x6D656464) // ui4 (big endian)
#define TS_CHAN_TYPE_STR_m13			"ticd" // ascii[4]
#define TS_CHAN_TYPE_CODE_m13			((ui4) 0x64636974) // ui4 (little endian)
// #define TS_CHAN_TYPE_CODE_m13		((ui4) 0x74696364) // ui4 (big endian)
#define TS_SEG_TYPE_STR_m13			"tisd" // ascii[4]
#define TS_SEG_TYPE_CODE_m13			((ui4) 0x64736974) // ui4 (little endian)
// #define TS_SEG_TYPE_CODE_m13			((ui4) 0x74697364) // ui4 (big endian)
#define TS_METADATA_TYPE_STR_m13		"tmet" // ascii[4]
#define TS_METADATA_TYPE_CODE_m13		((ui4) 0x74656D74) // ui4 (little endian)
// #define TS_METADATA_TYPE_CODE_m13		((ui4) 0x746D6574) // ui4 (big endian)
#define TS_DATA_TYPE_STR_m13			"tdat" // ascii[4]
#define TS_DATA_TYPE_CODE_m13			((ui4) 0x74616474) // ui4 (little endian)
// #define TS_DATA_TYPE_CODE_m13		((ui4) 0x74646174) // ui4 (big endian)
#define TS_INDS_TYPE_STR_m13			"tidx" // ascii[4]
#define TS_INDS_TYPE_CODE_m13			((ui4) 0x78646974) // ui4 (little endian)
// #define TS_INDS_TYPE_CODE_m13		((ui4) 0x74696478) // ui4 (big endian)
#define VID_SEG_TYPE_STR_m13			"visd" // ascii[4]
#define VID_SEG_TYPE_CODE_m13			((ui4) 0x64736976) // ui4 (little endian)
// #define VID_SEG_TYPE_CODE_m13		((ui4) 0x76697364) // ui4 (big endian)
#define VID_CHAN_TYPE_STR_m13			"vicd" // ascii[4]
#define VID_CHAN_TYPE_CODE_m13			((ui4) 0x64636976) // ui4 (little endian)
// #define VID_CHAN_TYPE_CODE_m13		((ui4) 0x76696364) // ui4 (big endian)
#define VID_METADATA_TYPE_STR_m13		"vmet" // ascii[4]
#define VID_METADATA_TYPE_CODE_m13		((ui4) 0x74656D76) // ui4 (little endian)
// #define VID_METADATA_TYPE_CODE_m13		((ui4) 0x766D6574) // ui4 (big endian)
#define VID_DATA_TYPE_STR_m13			"vdat" // ascii[4]; the video data TYPE tag (level & universal headers) - actual video data files keep native
						// container extensions (.mp4 etc.) so players recognize & play them (UH at file END, content at offset 0);
						// AS AN EXTENSION "vdat" names video data PARITY files (MED 1.1 spec, 2026-07-15; replaced "vpar"):
						// parity files carry their member type's extension (cf. time-series parity: .tdat/.tidx/...), &
						// video members have no single native extension to carry - PRTY_is_parity_m13() disambiguates by name
#define VID_DATA_TYPE_CODE_m13			((ui4) 0x74616476) // ui4 (little endian)
// #define VID_DATA_TYPE_CODE_m13		((ui4) 0x76646174) // ui4 (big endian)
#define VID_INDS_TYPE_STR_m13			"vidx" // ascii[4]
#define VID_INDS_TYPE_CODE_m13			((ui4) 0x78646976) // ui4 (little endian)
// #define VID_INDS_TYPE_CODE_m13		((ui4) 0x76696478) // ui4 (big endian)
// (the former VID_PARITY type ["vpar"] was removed 2026-07-16: video data parity files are named with the
// member type's extension ["vdat"], like all other parity files; no vpar file was ever written in the wild)
#define SSR_TYPE_STR_m13			"recd" // ascii[4]
#define SSR_TYPE_CODE_m13			((ui4) 0x64636572) // ui4 (little endian)
// #define SSR_TYPE_CODE_m13			((ui4) 0x72656364) // ui4 (big endian)
#define REC_DATA_TYPE_STR_m13			"rdat" // ascii[4]
#define REC_DATA_TYPE_CODE_m13			((ui4) 0x74616472) // ui4 (little endian)
// #define REC_DATA_TYPE_CODE_m13		((ui4) 0x72646174) // ui4 (big endian)
#define REC_INDS_TYPE_STR_m13			"ridx" // ascii[4]
#define REC_INDS_TYPE_CODE_m13			((ui4) 0x78646972) // ui4 (little endian)
// #define REC_INDS_TYPE_CODE_m13		((ui4) 0x72696478) // ui4 (big endian)

// Channel Types
#define UNKNOWN_CHANNEL_TYPE_m13	NO_TYPE_CODE_m13
#define TS_CHAN_TYPE_m13		TS_CHAN_TYPE_CODE_m13
#define VID_CHAN_TYPE_m13		VID_CHAN_TYPE_CODE_m13

// Reference Channel Types (used in change_index_chan_m13())
#define DEFAULT_CHAN_m13		0
#define DEFAULT_TS_CHAN_m13		1
#define DEFAULT_VID_CHAN_m13		2
#define HIGHEST_RATE_TS_CHAN_m13	3
#define LOWEST_RATE_TS_CHAN_m13		4
#define HIGHEST_RATE_VID_CHAN_m13	5
#define LOWEST_RATE_VID_CHAN_m13	6

// Generate File List flags
 // Path Parts
#define GFL_PATH_m13 			((ui4) 1)
#define GFL_NAME_m13 			((ui4) 2)
#define GFL_EXTENSION_m13 		((ui4) 4)
#define GFL_FULL_PATH_m13 		(GFL_PATH_m13 | GFL_NAME_m13 | GFL_EXTENSION_m13)
#define GFL_PATH_PARTS_MASK_m13 	GFL_FULL_PATH_m13
 // Other Options
#define GFL_FREE_INPUT_LIST_m13		((ui4) 16)
#define GFL_INCLUDE_PARITY_m13		((ui4) 32) // files or directories
#define GFL_INCLUDE_INVISIBLE_m13	((ui4) 64) // files or directories

// System Pipe flags
#define SP_DEFAULT_m13			0 // no flags set (default)
#define SP_TEE_TO_TERMINAL_m13		1 // print buffer(s) to terminal in addition to returning
#define SP_BEHAVIOR_PASSED_m13		2 // behavior passed as first vararg
#define SP_SEPARATE_STREAMS_m13		4 // return seprate "stdout" & "stderr" buffers (buffer = stdout, e_buffer = stderr), otherwise ganged

// Spaces Constants
#define NO_SPACES_m13			((ui4) 0)
#define ESCAPED_SPACES_m13		((ui4) 1)
#define UNESCAPED_SPACES_m13		((ui4) 2)
#define ALL_SPACES_m13			(ESCAPED_SPACES_m13 | UNESCAPED_SPACES_m13)

// Universal Header: File Format Constants
#define UH_OFFSET_m13						0  // except video data files where it is after video data & before pcrc data (if it exists)
#define UH_BYTES_m13						1024 // 1 KiB
#define UH_HEADER_CRC_OFFSET_m13				0 // ui4
#define UH_BODY_CRC_OFFSET_m13					4 // ui4
#define UH_HEADER_CRC_START_OFFSET_m13				UH_BODY_CRC_OFFSET_m13
#define UH_BODY_CRC_START_OFFSET_m13				UH_BYTES_m13
#define UH_FILE_END_TIME_OFFSET_m13				8 // si8
#define UH_NUMBER_OF_ENTRIES_OFFSET_m13				16 // si8
#define UH_NUMBER_OF_ENTRIES_NO_ENTRY_m13			-1
#define UH_MAXIMUM_ENTRY_SIZE_OFFSET_m13			24 // ui4
#define UH_MAXIMUM_ENTRY_SIZE_NO_ENTRY_m13			0
#define UH_SEGMENT_NUMBER_OFFSET_m13				28 // si4
#define UH_SEGMENT_NUMBER_NO_ENTRY_m13				SEGMENT_NUMBER_NO_ENTRY_m13
#define UH_SEGMENT_LEVEL_CODE_m13				-1
#define UH_CHANNEL_LEVEL_CODE_m13				-2
#define UH_SESSION_LEVEL_CODE_m13				-3
#define UH_TYPE_STR_OFFSET_m13				32 // ascii[4]
#define UH_TYPE_CODE_OFFSET_m13					UH_TYPE_STR_OFFSET_m13 // ui4
#define UH_TYPE_NO_ENTRY_m13					0 // zero as ui4 or zero-length string as ascii[4]
#define UH_MED_VERSION_MAJOR_OFFSET_m13				37 // ui1
#define UH_MED_VERSION_MAJOR_NO_ENTRY_m13			MED_VERSION_NO_ENTRY_m13
#define UH_MED_VERSION_MINOR_OFFSET_m13				38 // ui1
#define UH_MED_VERSION_MINOR_NO_ENTRY_m13			MED_VERSION_NO_ENTRY_m13
#define UH_BYTE_ORDER_CODE_OFFSET_m13				39 // ui1
#define UH_BYTE_ORDER_CODE_NO_ENTRY_m13				0xFF
#define UH_SESSION_START_TIME_OFFSET_m13			40 // si8
#define UH_FILE_START_TIME_OFFSET_m13				48 // si8
#define UH_SESSION_NAME_OFFSET_m13				56 // utf8[63]
#define UH_CHANNEL_NAME_OFFSET_m13				312 // utf8[63]
#define UH_SUPPLEMENTARY_PROTECTED_REGION_OFFSET_m13		568 // Anonymized Subject ID in MED 1.0 (moved to metadata section 1 in MED 1.1)
#define UH_SUPPLEMENTARY_PROTECTED_REGION_BYTES_m13		208 // (256 in MED 1.0: the final 48 bytes are now the anchor escrow field)
#define UH_ANCHOR_ESCROW_FIELD_OFFSET_m13			776 // final 48 bytes of the MED 1.0 supplementary protected region (see ANCHOR_ESCROW_FIELD_BYTES_m13)
#define UH_SESSION_UID_OFFSET_m13				824 // ui8
#define UH_CHANNEL_UID_OFFSET_m13				832 // ui8
#define UH_SEGMENT_UID_OFFSET_m13				840 // ui8
#define UH_FILE_UID_OFFSET_m13					848 // ui8
#define UH_PROVENANCE_UID_OFFSET_m13				856 // ui8
#define UH_LEVEL_1_PASSWORD_VALIDATION_FIELD_OFFSET_m13		864 // ui1
#define UH_LEVEL_2_PASSWORD_VALIDATION_FIELD_OFFSET_m13		880 // ui1
#define UH_LEVEL_3_PASSWORD_VALIDATION_FIELD_OFFSET_m13		896 // ui1
#define UH_VIDEO_DATA_FILE_NUMBER_OFFSET_m13 			912 // ui4, MED 1.1 & above
#define UH_LIVE_OFFSET_m13					916 // tern, MED 1.1 & above
#define UH_ORDERED_OFFSET_m13					917 // tern, MED 1.1 & above
// Encryption bytes (offsets 918-923): each names EXACTLY what it encrypts, one consistent meaning in every file's
// universal header. A given file populates the fields relevant to it; the metadata UH is the authoritative map for
// metadata sections & data (records self-report - see below). Encryption levels are si1: NO_ENCRYPTION_m13 (0),
// LEVEL_1/2_ENCRYPTION_m13, negative == currently-decrypted, ENCRYPTION_VARIABLE_m13 (-128) for the section cases.
#define UH_METADATA_SECTION_2_ENCRYPTION_OFFSET_m13		918 // si1
#define UH_METADATA_SECTION_3_ENCRYPTION_OFFSET_m13		919 // si1
#define UH_TIME_SERIES_DATA_ENCRYPTION_OFFSET_m13		920 // si1
#define UH_VIDEO_DATA_ENCRYPTION_OFFSET_m13			921 // si1
#define UH_MAXIMUM_RECORD_ENCRYPTION_LEVEL_OFFSET_m13		922 // si1: records (index & data) files ONLY - highest encryption level among the file's records (max() on append, recompute on rewrite); 0 => no encrypted records
#define UH_ENCRYPTION_RESERVED_OFFSET_m13			923 // si1, reserved for a future encryptable region
#define UH_CRYPTO_SCHEMA_OFFSET_m13				924 // ui1, taken from protected region (zeros in files written before this field == CRYPTO_SCHEMA_LEGACY_m13)
#define UH_KDF_EXPONENT_OFFSET_m13				925 // ui1[UH_KDF_EXPONENT_LEVELS_m13] (925-928): per-level KDF exponents (2^n; unused under legacy schema). UH_KDF_EXPONENT_LEVELS_m13 defined up in the KDF-policy block
#define UH_ANCHOR_TYPE_OFFSET_m13				929 // ui1, taken from protected region (see ANCHOR_TYPE_*_m13; zeros in older files == no anchor)
#define UH_ANCHOR_KEY_ID_OFFSET_m13				930 // ui1, taken from protected region (identifies the anchor public key under type 2; zero otherwise)
#define UH_LEVEL_1_ESCROW_FIELD_OFFSET_m13			931 // 16 bytes, taken from protected region (AES-256(escrow key from L2 chain material, L1 password bytes))
#define UH_RESERVED_LEVEL_4_ESCROW_FIELD_OFFSET_m13		947 // 16 bytes, EXPLICITLY RESERVED for a future level 4 escrow field: zero until then
#define UH_RESERVED_LEVEL_4_VALIDATION_FIELD_OFFSET_m13		963 // 16 bytes, EXPLICITLY RESERVED for a future level 4 validation field: zero until then
#define UH_KDF_SALT_EXTENSION_OFFSET_m13			979 // 8 bytes (first bytes of the former discretionary region): KDF salt = session UID (8) then this (8) == 128 bits (NIST SP 800-132)
#define UH_LIB_MOD_TIME_OFFSET_m13				1016 // si8 uutc, taken from protected region 2026-07-21: file system modification time DICTATED by the library at its
									// last write (G_file_times_m13() set after close) - a match with the file system proves the library was the last
									// writer, so the "ordered" flag can be trusted without scanning (zeros in older files == unstamped => scan)
#define UH_PROTECTED_REGION_OFFSET_m13				987
#define UH_PROTECTED_REGION_BYTES_m13				29 // the remainder of the former discretionary region, reclaimed as protected 2026-07-18 (lib_mod_time taken 2026-07-21): the universal
									// header is entirely machine space - user extension space is metadata discretionary regions & records
// (universal header discretionary region removed 2026-07-18: 48 bytes @ 976 reclaimed into the protected region - see above)

// defaults
#define UH_DATA_ENCRYPTION_DEFAULT_m13				NO_ENCRYPTION_m13 // si1, MED 1.1 & above
#define UH_METADATA_SECTION_2_ENCRYPTION_DEFAULT_m13		LEVEL_1_ENCRYPTION_m13 // si1, MED 1.1 & above
#define UH_METADATA_SECTION_3_ENCRYPTION_DEFAULT_m13		LEVEL_2_ENCRYPTION_m13 // si1, MED 1.1 & above

// Metadata: File Format Constants
#define METADATA_BYTES_m13			15360 // 15 KiB
#define FPS_PROTOTYPE_BYTES_m13			METADATA_BYTES_m13
#define METADATA_FILE_BYTES_m13			(METADATA_BYTES_m13 + UH_BYTES_m13) // 16 KiB
#define METADATA_SECTION_1_OFFSET_m13		1024
#define METADATA_SECTION_1_BYTES_m13		1024 // 1 KiB
#define METADATA_SECTION_2_OFFSET_m13		2048
#define METADATA_SECTION_2_BYTES_m13		10240 // 10 KiB
#define METADATA_SECTION_3_OFFSET_m13		12288
#define METADATA_SECTION_3_BYTES_m13		4096 // 4 KiB

// Metadata: File Format Constants - Section 1 Fields
#define METADATA_LEVEL_1_PASSWORD_HINT_OFFSET_m13		1024 // utf8[63]
#define METADATA_LEVEL_1_PASSWORD_HINT_BYTES_m13		PASSWORD_HINT_BYTES_m13
#define METADATA_LEVEL_2_PASSWORD_HINT_OFFSET_m13		1280 // utf8[63]
#define METADATA_LEVEL_2_PASSWORD_HINT_BYTES_m13		PASSWORD_HINT_BYTES_m13
#define METADATA_ANONYMIZED_SUBJECT_ID_OFFSET_m13		1536 // MED 1.1 & above
#define METADATA_ANONYMIZED_SUBJECT_ID_BYTES_m13		256 // utf8[63]
#define METADATA_SECTION_1_PROTECTED_REGION_OFFSET_m13		1792
#define METADATA_SECTION_1_PROTECTED_REGION_BYTES_m13		128
#define METADATA_SECTION_1_DISCRETIONARY_REGION_OFFSET_m13	1920
#define METADATA_SECTION_1_DISCRETIONARY_REGION_BYTES_m13	128

// Metadata: File Format Constants - Section 2 Channel Type Independent Fields
#define METADATA_SESSION_DESCRIPTION_OFFSET_m13			2048 // utf8[511]
#define METADATA_SESSION_DESCRIPTION_BYTES_m13			2048
#define METADATA_CHANNEL_DESCRIPTION_OFFSET_m13			4096 // utf8[255]
#define METADATA_CHANNEL_DESCRIPTION_BYTES_m13			1024
#define METADATA_SEGMENT_DESCRIPTION_OFFSET_m13			5120 // utf8[255]
#define METADATA_SEGMENT_DESCRIPTION_BYTES_m13			1024
#define METADATA_EQUIPMENT_DESCRIPTION_OFFSET_m13		6144 // utf8[510]
#define METADATA_EQUIPMENT_DESCRIPTION_BYTES_m13		2044
#define METADATA_ACQUISITION_CHANNEL_NUMBER_OFFSET_m13		8188 // si4
#define METADATA_ACQUISITION_CHANNEL_NUMBER_NO_ENTRY_m13	CHANNEL_NUMBER_NO_ENTRY_m13

// Metadata: File Format Constants - Time Series Section 2 Fields
#define TS_METADATA_REFERENCE_DESCRIPTION_OFFSET_m13			8192 // utf8[255]
#define TS_METADATA_REFERENCE_DESCRIPTION_BYTES_m13			1024
#define TS_METADATA_SAMPLING_FREQUENCY_OFFSET_m13			9216 // sf8
#define TS_METADATA_FREQUENCY_NO_ENTRY_m13				FREQUENCY_NO_ENTRY_m13
#define TS_METADATA_FREQUENCY_VARIABLE_m13				RATE_VARIABLE_m13
#define TS_METADATA_LOW_FREQUENCY_FILTER_SETTING_OFFSET_m13		9224 // sf8
#define TS_METADATA_HIGH_FREQUENCY_FILTER_SETTING_OFFSET_m13		9232 // sf8
#define TS_METADATA_NOTCH_FILTER_FREQUENCY_SETTING_OFFSET_m13		9240 // sf8
#define TS_METADATA_AC_LINE_FREQUENCY_OFFSET_m13			9248 // sf8
#define TS_METADATA_AMPLITUDE_UNITS_CONVERSION_FACTOR_OFFSET_m13	9256 // sf8
#define TS_METADATA_AMPLITUDE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m13	0.0
#define TS_METADATA_AMPLITUDE_UNITS_DESCRIPTION_OFFSET_m13		9264 // utf8[31]
#define TS_METADATA_AMPLITUDE_UNITS_DESCRIPTION_BYTES_m13		128
#define TS_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_OFFSET_m13	9392 // sf8
#define TS_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m13	0.0
#define TS_METADATA_TIME_BASE_UNITS_DESCRIPTION_OFFSET_m13		9400 // utf8[31]
#define TS_METADATA_TIME_BASE_UNITS_DESCRIPTION_BYTES_m13		128
#define TS_METADATA_SESSION_START_SAMPLE_NUMBER_OFFSET_m13		9528
#define TS_METADATA_SESSION_START_SAMPLE_NUMBER_NO_ENTRY_m13		INDEX_NO_ENTRY_m13
#define TS_METADATA_NUMBER_OF_SAMPLES_OFFSET_m13			9536 // si8
#define TS_METADATA_NUMBER_OF_SAMPLES_NO_ENTRY_m13			NUMBER_OF_SAMPLES_NO_ENTRY_m13
#define TS_METADATA_NUMBER_OF_BLOCKS_OFFSET_m13				9544 // si8
#define TS_METADATA_NUMBER_OF_BLOCKS_NO_ENTRY_m13			-1
#define TS_METADATA_MAXIMUM_BLOCK_BYTES_OFFSET_m13			9552 // si8
#define TS_METADATA_MAXIMUM_BLOCK_BYTES_NO_ENTRY_m13			-1
#define TS_METADATA_MAXIMUM_BLOCK_SAMPLES_OFFSET_m13			9560 // ui4
#define TS_METADATA_MAXIMUM_BLOCK_SAMPLES_NO_ENTRY_m13			0xFFFFFFFF
#define TS_METADATA_MAXIMUM_BLOCK_KEYSAMPLE_BYTES_OFFSET_m13		9564 // ui4
#define TS_METADATA_MAXIMUM_BLOCK_KEYSAMPLE_BYTES_NO_ENTRY_m13 		0xFFFFFFFF
#define TS_METADATA_MAXIMUM_BLOCK_DURATION_OFFSET_m13			9568 // sf8
#define TS_METADATA_MAXIMUM_BLOCK_DURATION_NO_ENTRY_m13			-1.0
#define TS_METADATA_NUMBER_OF_DISCONTINUITIES_OFFSET_m13		9576 // si8
#define TS_METADATA_NUMBER_OF_DISCONTINUITIES_NO_ENTRY_m13		-1
#define TS_METADATA_MAXIMUM_CONTIGUOUS_BLOCKS_OFFSET_m13		9584 // si8
#define TS_METADATA_MAXIMUM_CONTIGUOUS_BLOCKS_NO_ENTRY_m13		-1
#define TS_METADATA_MAXIMUM_CONTIGUOUS_BLOCK_BYTES_OFFSET_m13		9592 // si8
#define TS_METADATA_MAXIMUM_CONTIGUOUS_BLOCK_BYTES_NO_ENTRY_m13		-1
#define TS_METADATA_MAXIMUM_CONTIGUOUS_SAMPLES_OFFSET_m13		9600 // si8
#define TS_METADATA_MAXIMUM_CONTIGUOUS_SAMPLES_NO_ENTRY_m13		-1
#define TS_METADATA_SECTION_2_PROTECTED_REGION_OFFSET_m13		9608
#define TS_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m13		1344
#define TS_METADATA_SECTION_2_DISCRETIONARY_REGION_OFFSET_m13		10952
#define TS_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m13		1336

// Metadata: File Format Constants - Video Section 2 Fields
#define VID_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_OFFSET_m13	8192  // sf8
#define VID_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m13	0.0
#define VID_METADATA_TIME_BASE_UNITS_DESCRIPTION_OFFSET_m13		8200  // utf8[31]
#define VID_METADATA_TIME_BASE_UNITS_DESCRIPTION_BYTES_m13		128
#define VID_METADATA_SESSION_START_FRAME_NUMBER_OFFSET_m13		8328
#define VID_METADATA_SESSION_START_FRAME_NUMBER_NO_ENTRY_m13		FRAME_NUMBER_NO_ENTRY_m13
#define VID_METADATA_NUMBER_OF_FRAMES_OFFSET_m13			8336  // si8
#define VID_METADATA_NUMBER_OF_FRAMES_NO_ENTRY_m13			NUMBER_OF_FRAMES_NO_ENTRY_m13
#define VID_METADATA_FRAME_RATE_OFFSET_m13				8344  // sf8
#define VID_METADATA_FRAME_RATE_NO_ENTRY_m13				FRAME_RATE_NO_ENTRY_m13
#define VID_METADATA_FRAME_RATE_VARIABLE_m13				FRAME_RATE_VARIABLE_m13
#define VID_METADATA_NUMBER_OF_CLIPS_OFFSET_m13				8352 // si8
#define VID_METADATA_NUMBER_OF_CLIPS_NO_ENTRY_m13			-1
#define VID_METADATA_MAXIMUM_CLIP_BYTES_OFFSET_m13			8360 // si8
#define VID_METADATA_MAXIMUM_CLIP_BYTES_NO_ENTRY_m13			-1
#define VID_METADATA_MAXIMUM_CLIP_FRAMES_OFFSET_m13			8368 // ui4
#define VID_METADATA_MAXIMUM_CLIP_FRAMES_NO_ENTRY_m13			0xFFFFFFFF
#define VID_METADATA_NUMBER_OF_VIDEO_FILES_OFFSET_m13			8372  // si4
#define VID_METADATA_NUMBER_OF_VIDEO_FILES_NO_ENTRY_m13			-1
#define VID_METADATA_MAXIMUM_CLIP_DURATION_OFFSET_m13			8376 // sf8
#define VID_METADATA_MAXIMUM_CLIP_DURATION_NO_ENTRY_m13			-1.0
#define VID_METADATA_NUMBER_OF_DISCONTINUITIES_OFFSET_m13		8384 // si8
#define VID_METADATA_NUMBER_OF_DISCONTINUITIES_NO_ENTRY_m13		-1
#define VID_METADATA_MAXIMUM_CONTIGUOUS_CLIPS_OFFSET_m13		8392 // si8
#define VID_METADATA_MAXIMUM_CONTIGUOUS_CLIPS_NO_ENTRY_m13		-1
#define VID_METADATA_MAXIMUM_CONTIGUOUS_CLIP_BYTES_OFFSET_m13		8400 // si8
#define VID_METADATA_MAXIMUM_CONTIGUOUS_CLIP_BYTES_NO_ENTRY_m13		-1
#define VID_METADATA_MAXIMUM_CONTIGUOUS_FRAMES_OFFSET_m13		8408 // si8
#define VID_METADATA_MAXIMUM_CONTIGUOUS_FRAMES_NO_ENTRY_m13		-1
#define VID_METADATA_HORIZONTAL_PIXELS_OFFSET_m13			8416  // ui4
#define VID_METADATA_HORIZONTAL_PIXELS_NO_ENTRY_m13			0
#define VID_METADATA_VERTICAL_PIXELS_OFFSET_m13				8420  // ui4
#define VID_METADATA_VERTICAL_PIXELS_NO_ENTRY_m13			0
#define VID_METADATA_VIDEO_FORMAT_OFFSET_m13				8424  // utf8[31] (human-readable container/codec tag; machine truth in codec_config)
#define VID_METADATA_VIDEO_FORMAT_BYTES_m13				128
#define VID_METADATA_DISPLAY_TRANSFORM_OFFSET_m13			8552  // ui1
#define VID_METADATA_DISPLAY_TRANSFORM_NO_ENTRY_m13			((ui1) 0)  // display as stored
// display transform values 1-8 are numerically identical to EXIF/TIFF Orientation (the complete, closed set of
// axis-aligned rectangle transforms); camera SEMANTICS (mounting, viewpoint) belong in equipment_description
#define VID_METADATA_DISPLAY_TRANSFORM_IDENTITY_m13			((ui1) 1)  // top / left
#define VID_METADATA_DISPLAY_TRANSFORM_MIRROR_H_m13			((ui1) 2)  // top / right (mirrored horizontal)
#define VID_METADATA_DISPLAY_TRANSFORM_ROT_180_m13			((ui1) 3)  // bottom / right (rotated 180)
#define VID_METADATA_DISPLAY_TRANSFORM_MIRROR_V_m13			((ui1) 4)  // bottom / left (mirrored vertical)
#define VID_METADATA_DISPLAY_TRANSFORM_MIRROR_H_ROT_270_m13		((ui1) 5)  // left / top (mirrored horizontal, rotated 270 CW)
#define VID_METADATA_DISPLAY_TRANSFORM_ROT_90_m13			((ui1) 6)  // right / top (rotated 90 CW)
#define VID_METADATA_DISPLAY_TRANSFORM_MIRROR_H_ROT_90_m13		((ui1) 7)  // right / bottom (mirrored horizontal, rotated 90 CW)
#define VID_METADATA_DISPLAY_TRANSFORM_ROT_270_m13			((ui1) 8)  // left / bottom (rotated 270 CW)
#define VID_METADATA_VIDEO_FLAGS_OFFSET_m13				8553  // ui1
#define VID_METADATA_VIDEO_FLAGS_NO_ENTRY_m13				((ui1) 0)
#define VID_METADATA_VIDEO_FLAGS_EMBEDDED_AUDIO_MASK_m13		((ui1) 1)  // container carries audio stream(s): unindexed, travel with video bytes
#define VID_METADATA_CODEC_CONFIG_BYTES_OFFSET_m13			8554  // ui2
#define VID_METADATA_CODEC_CONFIG_BYTES_NO_ENTRY_m13			((ui2) 0)
#define VID_METADATA_CODEC_CONFIG_OFFSET_m13				8556  // ui1[512]
#define VID_METADATA_CODEC_CONFIG_MAX_BYTES_m13				512  // if a codec's private data exceeds this, encoder MUST use in-band parameter sets & leave this empty
#define VID_METADATA_SECTION_2_PROTECTED_REGION_OFFSET_m13		9068
#define VID_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m13		1420
#define VID_METADATA_SECTION_2_DISCRETIONARY_REGION_OFFSET_m13		10488
#define VID_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m13		1800

// Metadata: File Format Constants - Section 3 Fields
#define METADATA_RECORDING_TIME_OFFSET_OFFSET_m13		12288 // si8
#define METADATA_RECORDING_TIME_OFFSET_NO_ENTRY_m13		GLOBALS_RECORDING_TIME_OFFSET_NO_ENTRY_m13
#define METADATA_DAYLIGHT_TIME_START_CODE_OFFSET_m13		12296 // DAYLIGHT_TIME_CHANGE_CODE_m13 (si1[8])
#define METADATA_DAYLIGHT_TIME_START_CODE_NO_ENTRY_m13		DTCC_VALUE_NO_ENTRY_m13
#define METADATA_DAYLIGHT_TIME_END_CODE_OFFSET_m13		12304 // DAYLIGHT_TIME_CHANGE_CODE_m13 (si1[8])
#define METADATA_DAYLIGHT_TIME_END_CODE_NO_ENTRY_m13		DTCC_VALUE_NO_ENTRY_m13
#define METADATA_STANDARD_TIMEZONE_ACRONYM_OFFSET_m13		12312 // ascii[7]
#define METADATA_STANDARD_TIMEZONE_ACRONYM_BYTES_m13		TIMEZONE_ACRONYM_BYTES_m13
#define METADATA_STANDARD_TIMEZONE_STRING_OFFSET_m13		12320 // ascii[63]
#define METADATA_STANDARD_TIMEZONE_STRING_BYTES_m13		TIMEZONE_STRING_BYTES_m13
#define METADATA_DAYLIGHT_TIMEZONE_ACRONYM_OFFSET_m13		12384 // ascii[7]
#define METADATA_DAYLIGHT_TIMEZONE_ACRONYM_BYTES_m13		TIMEZONE_ACRONYM_BYTES_m13
#define METADATA_DAYLIGHT_TIMEZONE_STRING_OFFSET_m13		12392 // ascii[63]
#define METADATA_DAYLIGHT_TIMEZONE_STRING_BYTES_m13		TIMEZONE_STRING_BYTES_m13
#define METADATA_SUBJECT_NAME_1_OFFSET_m13			12456 // utf8[31]
#define METADATA_SUBJECT_NAME_BYTES_m13				128
#define METADATA_SUBJECT_NAME_2_OFFSET_m13			12584 // utf8[31]
#define METADATA_SUBJECT_NAME_3_OFFSET_m13			12712 // utf8[31]
#define METADATA_SUBJECT_ID_OFFSET_m13				12840 // utf8[31]
#define METADATA_SUBJECT_ID_BYTES_m13				128
#define METADATA_RECORDING_COUNTRY_OFFSET_m13			12968 // utf8[63]
#define METADATA_RECORDING_TERRITORY_OFFSET_m13			13224 // utf8[63]
#define METADATA_RECORDING_LOCALITY_OFFSET_m13			13480 // utf8[63]
#define METADATA_RECORDING_INSTITUTION_OFFSET_m13		13736 // utf8[63]
#define METADATA_RECORDING_LOCATION_BYTES_m13			256
#define METADATA_GEOTAG_FORMAT_OFFSET_m13			13992 // ascii[31]
#define METADATA_GEOTAG_FORMAT_BYTES_m13			32
#define METADATA_GEOTAG_DATA_OFFSET_m13				14024 // ascii[1023]
#define METADATA_GEOTAG_DATA_BYTES_m13				1024
#define METADATA_STANDARD_UTC_OFFSET_OFFSET_m13			15048 // si4
#define METADATA_STANDARD_UTC_OFFSET_NO_ENTRY_m13		STANDARD_UTC_OFFSET_NO_ENTRY_m13
#define METADATA_SECTION_3_PROTECTED_REGION_OFFSET_m13		15052
#define METADATA_SECTION_3_PROTECTED_REGION_BYTES_m13		668
#define METADATA_SECTION_3_DISCRETIONARY_REGION_OFFSET_m13	15720
#define METADATA_SECTION_3_DISCRETIONARY_REGION_BYTES_m13	664

// Records: Header Format Constants
#define REC_HDR_BYTES_m13					24
#define REC_HDR_CRC_OFFSET_m13				0 // ui4
#define REC_HDR_CRC_NO_ENTRY_m13			CRC_NO_ENTRY_m13
#define REC_HDR_TOTAL_RECORD_BYTES_OFFSET_m13		4 // ui4
#define REC_HDR_TOTAL_RECORD_BYTES_NO_ENTRY_m13		0
#define REC_HDR_CRC_START_OFFSET_m13			REC_HDR_TOTAL_RECORD_BYTES_OFFSET_m13
#define REC_HDR_START_TIME_OFFSET_m13			8 // si8
#define REC_HDR_START_TIME_NO_ENTRY_m13			TIME_NO_ENTRY_m13 // si8
#define REC_HDR_TYPE_STR_OFFSET_m13			16	 // ascii[4]
#define REC_HDR_TYPE_CODE_OFFSET_m13			REC_HDR_TYPE_STR_OFFSET_m13  // ui4
#define REC_HDR_TYPE_CODE_NO_ENTRY_m13			0	 // ui4
#define REC_HDR_VERSION_MAJOR_OFFSET_m13		21	 // ui1
#define REC_HDR_VERSION_MAJOR_NO_ENTRY_m13		0xFF
#define REC_HDR_VERSION_MINOR_OFFSET_m13		22	 // ui1
#define REC_HDR_VERSION_MINOR_NO_ENTRY_m13		0xFF
#define REC_HDR_ENCRYPTION_LEVEL_OFFSET_m13		23	 // si1
#define REC_HDR_ENCRYPTION_LEVEL_NO_ENTRY_m13		ENCRYPTION_LEVEL_NO_ENTRY_m13

// Record Index: Format Constants
#define REC_IDX_BYTES_m13				INDEX_BYTES_m13
#define REC_IDX_FILE_OFFSET_OFFSET_m13			0 // si8
#define REC_IDX_FILE_OFFSET_NO_ENTRY_m13		-1
#define REC_IDX_START_TIME_OFFSET_m13			8 // si8
#define REC_IDX_START_TIME_NO_ENTRY_m13			TIME_NO_ENTRY_m13
#define REC_IDX_TYPE_STR_OFFSET_m13			16 // ascii[4]
#define REC_IDX_TYPE_CODE_OFFSET_m13			REC_IDX_TYPE_STR_OFFSET_m13  // as ui4
#define REC_IDX_TYPE_CODE_NO_ENTRY_m13			0 // as ui4
#define REC_IDX_VERSION_MAJOR_OFFSET_m13		21 // ui1
#define REC_IDX_VERSION_MAJOR_NO_ENTRY_m13		0xFF
#define REC_IDX_VERSION_MINOR_OFFSET_m13		22 // ui1
#define REC_IDX_VERSION_MINOR_NO_ENTRY_m13		0xFF
#define REC_IDX_ENCRYPTION_LEVEL_OFFSET_m13		23 // si1
#define REC_IDX_ENCRYPTION_LEVEL_NO_ENTRY_m13		ENCRYPTION_LEVEL_NO_ENTRY_m13

// Time Series Index: Format Constants
#define TS_IDX_BYTES_m13				INDEX_BYTES_m13
#define TS_IDX_FILE_OFFSET_OFFSET_m13			0 // si8
#define TS_IDX_FILE_OFFSET_NO_ENTRY_m13			-1
#define TS_IDX_START_TIME_OFFSET_m13			 8 // si8
#define TS_IDX_START_TIME_NO_ENTRY_m13			TIME_NO_ENTRY_m13
#define TS_IDX_START_SAMPLE_NUMBER_OFFSET_m13		16 // si8
#define TS_IDX_START_SAMPLE_NUMBER_NO_ENTRY_m13		-1

// Video Index: Format Constants
#define VID_IDX_BYTES_m13				INDEX_BYTES_m13
#define VID_IDX_FILE_OFFSET_OFFSET_m13			0 // si8
#define VID_IDX_FILE_OFFSET_NO_ENTRY_m13		-1
#define VID_IDX_START_TIME_OFFSET_m13			8 // si8
#define VID_IDX_START_TIME_NO_ENTRY_m13			TIME_NO_ENTRY_m13
#define VID_IDX_START_FRAME_OFFSET_m13			16 // ui4
#define VID_IDX_START_FRAME_NO_ENTRY_m13		0xFFFFFFFF
#define VID_IDX_VIDEO_FILE_NUMBER_OFFSET_m13		20 // ui4
#define VID_IDX_VID_FILE_NUMBER_NO_ENTRY_m13		0
#define VID_IDX_TERMINAL_VID_FILE_NUMBER_m13		0xFFFFFFFF

// Prior MED version offsets
#define MED_10_METADATA_SECTION_2_ENCRYPTION_LEVEL_OFFSET_m13		1536 // si1
#define MED_10_METADATA_SECTION_3_ENCRYPTION_LEVEL_OFFSET_m13		1537 // si1
#define MED_10_METADATA_TS_DATA_ENCRYPTION_LEVEL_OFFSET_m13		1538 // si1
#define MED_10_UH_ANONYMIZED_SUBJECT_ID_OFFSET_m13			568 // utf8[63]
#define MED_10_UH_ANONYMIZED_SUBJECT_ID_FIELD_BYTES_m13			256 // the full MED 1.0 region: now supplementary protected (208) + anchor escrow field (48) - the updater must zero ALL of it
#define MED_10_CMP_BF_LEVEL_1_ENCRYPTION_BIT_m13			((ui4) 1 << 4)
#define MED_10_CMP_BF_LEVEL_2_ENCRYPTION_BIT_m13			((ui4) 1 << 4)

// Level Header (LH) Flags Definitions:

// level header flags
// READ == on open: open file & read universal header (applies to data files, index files are always read in full)
// on read: set FPS pointer to section specified by time slice (decrpyting if necessary)
// READ_FULL == on open: read full file (no memory mapping required, & closing)
// MMAP == allocate memory for full file, but only read on demand, (no re-reading occurs, but potentially memory expensive, good paired with VDS)
// ACTIVE == applies only to channels. Mark a CHANNEL as active to return data. Marking a channel as inactive does not free or close it.
// EPHEMERAL DATA == if GENERATE_EPHEMERAL_DATA_m13 is set, ephemeral data is created if it does not exist.
//	If UPDATE_EPHEMERAL_DATA is set, the data is updated whenever the channel or segment open set changes (opening of new elements, not the active status)
//	The UPDATE_EPHEMERAL_DATA bit is set by the lower levels and reset by the higher level once the data has been updated.
//	i.e read_channel_m13() checks the segment bits (e.g. read_segment_m13() opened a new segment) & if update required, it does the channel level update & clears the segment bit.
//	It then sets it's bit to trigger update at the session level. After updating, the session will clear the channel level bit.

// all levels
#define LH_NO_FLAGS_m13				((ui8) NO_FLAGS_m13)
#define LH_FLAGS_SET_m13			((ui8) 1 << 0) // flags have been set (distinguishes empty flags from flags not set)
#define LH_GENERATE_EPHEMERAL_DATA_m13		((ui8) 1 << 1) // implies all level involvement
#define LH_UPDATE_EPHEMERAL_DATA_m13		((ui8) 1 << 2) // signal to higher level from lower level (reset by higher level after update)

// session level
#define LH_SESS_OPEN_m13			((ui8) 1 << 8) // session has been opened
#define LH_EXCLUDE_TS_CHANS_m13			((ui8) 1 << 9) // useful when session directory passed, but don't want time series channels
#define LH_EXCLUDE_VID_CHANS_m13		((ui8) 1 << 10) // useful when session directory passed, but don't want video channels
#define LH_MAP_ALL_TS_CHANS_m13			((ui8) 1 << 11) // useful when time series channels may be added to open session
#define LH_MAP_ALL_VID_CHANS_m13		((ui8) 1 << 12) // useful when video channels may be added to open session

#define LH_READ_SLICE_SESS_RECS_m13		((ui8) 1 << 16) // read full record indices file (close file); open data, read universal header, leave open
#define LH_READ_FULL_SESS_RECS_m13		((ui8) 1 << 17) // read full recordindices & data files, close all files
#define LH_MMAP_SESS_RECS_m13			((ui8) 1 << 18) // allocate, but don't read full file

// segmented session records level
#define LH_SSR_OPEN_m13				((ui8) 1 << 24) // segmented session records has been opened
#define LH_READ_SLICE_SEG_SESS_RECS_m13		((ui8) 1 << 25) // read full indices file (close file); open data, read universal header, leave open
#define LH_READ_FULL_SEG_SESS_RECS_m13		((ui8) 1 << 26) // read full indices file & data files, close all files
#define LH_MMAP_SEG_SESS_RECS_m13		((ui8) 1 << 27) // allocate, but don't read full data file

// channel level
#define LH_CHAN_OPEN_m13			((ui8) 1 << 32) // channel has been opened
#define LH_CHAN_ACTIVE_m13			((ui8) 1 << 33) // include channel in current read set
#define LH_IDX_CHAN_INACTIVE_m13		((ui8) 1 << 34)
// bit 35 free (was LH_MAP_ALL_SEGS: retired - all segments are always mapped now, so the flag is unconditional & unnecessary)
// (active channels only)
#define LH_READ_SLICE_CHAN_RECS_m13		((ui8) 1 << 40) // read full record indices file (close file); open record data, read universal header, leave open
#define LH_READ_FULL_CHAN_RECS_m13		((ui8) 1 << 41) // read full record indices & data files, close all files
#define LH_MMAP_CHAN_RECS_m13			((ui8) 1 << 42) // allocate, but don't read full file
#define LH_THREAD_SEG_READS_m13			((ui8) 1 << 43) // set if likely to cross many segment boundaries in read (e.g. one channel, long reads or short segments)

// segment level
#define LH_SEG_OPEN_m13				((ui8) 1 << 48) // segment has been opened
#define LH_NO_CPS_PTR_RESET_m13			((ui8) 1 << 49) // caller will update pointers
#define LH_NO_CPS_CACHING_m13			((ui8) 1 << 50) // set cps_caching parameter to FALSE
// (active channels only)
#define LH_READ_SLICE_SEG_DATA_m13		((ui8) 1 << 56) // read full metadata & indices files, close files; open data, read universal header, leave open
#define LH_READ_FULL_SEG_DATA_m13		((ui8) 1 << 57) // read full metadata, indices, & data files, close all files
#define LH_MMAP_SEG_DATA_m13			((ui8) 1 << 58) // allocate, but don't read full file
#define LH_READ_SLICE_SEG_RECS_m13		((ui8) 1 << 59) // read full indices file (close file); open data, read universal header, leave open
#define LH_READ_FULL_SEG_RECS_m13		((ui8) 1 << 60) // read full indices file & data files, close all files
#define LH_MMAP_SEG_RECS_m13			((ui8) 1 << 61) // allocate, but don't read full file
#define LH_READ_SEG_METADATA_m13		((ui8) 1 << 62) // read segment metadata

// flag groups
#define LH_MAP_ALL_CHANNELS_m13 	 	( LH_MAP_ALL_TS_CHANS_m13 | LH_MAP_ALL_VID_CHANS_m13 )

// reading masks (not to be used as flags: SLICE/FULL mutually exclusive)
#define LH_READ_SESS_RECS_MASK_m13 		( LH_READ_SLICE_SESS_RECS_m13 | LH_READ_FULL_SESS_RECS_m13 )
#define LH_READ_SEG_SESS_RECS_MASK_m13 		( LH_READ_SLICE_SEG_SESS_RECS_m13 | LH_READ_FULL_SEG_SESS_RECS_m13 )
#define LH_READ_CHAN_RECS_MASK_m13 		( LH_READ_SLICE_CHAN_RECS_m13 | LH_READ_FULL_CHAN_RECS_m13 )
#define LH_READ_SEG_RECS_MASK_m13		( LH_READ_SLICE_SEG_RECS_m13 | LH_READ_FULL_SEG_RECS_m13 )
#define LH_READ_SEG_DATA_MASK_m13		( LH_READ_SLICE_SEG_DATA_m13 | LH_READ_FULL_SEG_DATA_m13 )
#define LH_READ_RECS_MASK_m13			( LH_READ_SESS_RECS_MASK_m13 | LH_READ_SEG_SESS_RECS_MASK_m13 | LH_READ_CHAN_RECS_MASK_m13 | LH_READ_SEG_RECS_MASK_m13 )
#define LH_ALL_READ_FLAGS_MASK_m13		( LH_READ_RECS_MASK_m13 | LH_READ_SEG_DATA_MASK_m13 )
#define LH_READ_FULL_RECS_MASK_m13		( LH_READ_FULL_SESS_RECS_m13 | LH_READ_FULL_SEG_SESS_REC_m13 | LH_READ_FULL_CHAN_RECS_m13 | LH_READ_FULL_SEG_RECS_m13 )
#define LH_READ_FULL_FILES_MASK_m13		( LH_READ_FULL_RECORDS_MASK_m13 | LH_READ_FULL_SEGMENT_DATA_m13 )
#define LH_READ_METADATA_MASK_m13		( LH_READ_SEG_DATA_MASK_m13 | LH_READ_SEG_METADATA_m13 | LH_GENERATE_EPHEMERAL_DATA_m13 )
// memory map flags & masks
#define LH_MMAP_ALL_RECS_m13			( LH_MMAP_SESS_RECS_m13 | LH_MMAP_SEG_SESS_RECS_m13 | LH_MMAP_CHAN_RECS_m13 | LH_MMAP_SEG_RECS_m13 )
#define LH_MMAP_ALL_m13				( LH_MMAP_ALL_RECS_m13 | LH_MMAP_SEG_DATA_m13 )
#define LH_ALL_MMAP_FLAGS_MASK_m13		LH_MMAP_ALL_m13

// record reading groups
#define LH_READ_SLICE_ALL_RECS_m13		( LH_READ_SLICE_SESS_RECS_m13 | LH_READ_SLICE_SEG_SESS_RECS_m13 | LH_READ_SLICE_CHAN_RECS_m13 | LH_READ_SLICE_SEG_RECS_m13 )
#define LH_READ_FULL_ALL_RECS_m13		( LH_READ_FULL_SESS_RECS_m13 | LH_READ_FULL_SEG_SESS_RECS_m13 | LH_READ_FULL_CHAN_RECS_m13 | LH_READ_FULL_SEG_RECS_m13 )

// default
#define LH_DEFAULT_FLAGS_m13			( LH_READ_SLICE_SEG_DATA_m13 | LH_READ_SLICE_ALL_RECS_m13 )



//**********************************************************************************//
// MARK: File Functions (FILE & FLOCK)
//**********************************************************************************//

// FILE_m13 is an enhanced FILE pointer for use in medlib functions
// Note: functions requiring FILE pointers accept either, FILE_m13 * or FILE * wherever appropriate
// These arguments are declared as void pointers for use of either without casting or compiler warnings


// Defines
#define FILE_TAG_m13			((ui4) 0x87654321) // ui4 (decimal 2,271,560,481)

#define FILE_PERM_OTH_EXEC_m13		((ui2) 1 << 0) // == S_IXOTH
#define FILE_PERM_OTH_WRITE_m13		((ui2) 1 << 1) // == S_IWOTH
#define FILE_PERM_OTH_READ_m13		((ui2) 1 << 2) // == S_IROTH
#define FILE_PERM_GRP_EXEC_m13		((ui2) 1 << 3) // == S_IXGRP
#define FILE_PERM_GRP_WRITE_m13		((ui2) 1 << 4) // == S_IWGRP
#define FILE_PERM_GRP_READ_m13		((ui2) 1 << 5) // == S_IRGRP
#define FILE_PERM_USR_EXEC_m13		((ui2) 1 << 6) // == S_IXUSR
#define FILE_PERM_USR_WRITE_m13		((ui2) 1 << 7) // == S_IWUSR
#define FILE_PERM_USR_READ_m13		((ui2) 1 << 8) // == S_IRUSR
#define FILE_PERM_STICKY_m13		((ui2) 1 << 9) // == S_ISVTX (sticky bit)
#define FILE_PERM_SET_GRP_m13		((ui2) 1 << 10) // == S_ISGID (set group bit)
#define FILE_PERM_SET_USR_m13		((ui2) 1 << 11) // == S_ISUID (set user bit)
#define FILE_STAT_PERM_MASK_m13		((ui2) 0x01FF) // lower 9 bits of stat member st_mode (as ui2)
#define FILE_STAT_MODE_MASK_m13		((ui2) 0x0FFF) // lower 12 bits of stat member st_mode (as ui2)

// Convenience
#define FILE_PERM_NONE_m13		0
#define FILE_PERM_UG_R_m13		( FILE_PERM_USR_READ_m13 | FILE_PERM_GRP_READ_m13 )
#define FILE_PERM_UG_W_m13		( FILE_PERM_USR_WRITE_m13 | FILE_PERM_GRP_WRITE_m13 )
#define FILE_PERM_UG_X_m13		( FILE_PERM_USR_EXEC_m13 | FILE_PERM_GRP_EXEC_m13 )
#define FILE_PERM_UG_RW_m13		( FILE_PERM_UG_R_m13 | FILE_PERM_UG_W_m13 )
#define FILE_PERM_UG_RWX_m13		( FILE_PERM_UG_RW_m13 | FILE_PERM_UG_X_m13 )
#define FILE_PERM_UGO_R_m13		( FILE_PERM_UG_R_m13 | FILE_PERM_OTH_READ_m13 )
#define FILE_PERM_UGO_W_m13		( FILE_PERM_UG_W_m13 | FILE_PERM_OTH_WRITE_m13 )
#define FILE_PERM_UGO_X_m13		( FILE_PERM_UG_X_m13 | FILE_PERM_OTH_EXEC_m13 )
#define FILE_PERM_UGO_RW_m13		( FILE_PERM_UGO_R_m13 | FILE_PERM_UGO_W_m13 )
#define FILE_PERM_UGO_RWX_m13		( FILE_PERM_UGO_RW_m13 | FILE_PERM_UGO_X_m13 )
#if defined MACOS_m13 || defined LINUX_m13
	#define FILE_PERM_DEFAULT_m13		FILE_PERM_UG_RW_m13 // rw- rw- ---
#endif
#ifdef WINDOWS_m13
	#define FILE_PERM_DEFAULT_m13		FILE_PERM_UGO_RW_m13 // rw- rw- rw-
	#define WN_PERM_MODE_DEFAULT_m13	( (si4) (_S_IREAD | _S_IWRITE) ) // read & write for all (no "other" in Windows); does not replace FILE_PERM_DEFAULT_m13, but should match it
#endif

#define FILE_FLAGS_NONE_m13		((ui2) NO_FLAGS_m13)
#define FILE_FLAGS_SET_m13		((ui2) 1 << 0) // to distinguish set bits zero from flags not set
#define FILE_FLAGS_ALLOCED_m13		((ui2) 1 << 1) // file structure was allocated
#define FILE_FLAGS_LOCK_m13		((ui2) 1 << 2) // file is subject to locking
#define FILE_FLAGS_STD_STREAM_m13	((ui2) 1 << 3) // file is a standard stream (stdin, stdout, stderr converted to FILE_m13)
#define FILE_FLAGS_MED_m13		((ui2) 1 << 4) // file is a MED file
#define FILE_FLAGS_PARITY_m13		((ui2) 1 << 5) // file is a parity file
#define FILE_FLAGS_READ_m13		((ui2) 1 << 6) // file is open for reading
#define FILE_FLAGS_WRITE_m13		((ui2) 1 << 7) // file is open for writing
#define FILE_FLAGS_APPEND_m13		((ui2) 1 << 8) // file is open in append mode (all writes will append regardless of fp; "append" is treated as a modifier of "write" mode)
#define FILE_FLAGS_LEN_m13		((ui2) 1 << 9) // update len with each operation
#define FILE_FLAGS_POS_m13		((ui2) 1 << 10) // update pos with each operation
#define FILE_FLAGS_TIME_m13		((ui2) 1 << 11) // update access time with each operation (global sets flag here, but flag supersedes global in execution)
#define FILE_FLAGS_DIGEST_m13		((ui2) 1 << 12) // streaming canonical digest attached (DGST module; fwrite_m13() absorbs written bytes)
#define FILE_FLAGS_DEFAULT_m13		( FILE_FLAGS_SET_m13 | FILE_FLAGS_LEN_m13 | FILE_FLAGS_POS_m13 )
#define FILE_FLAGS_MODE_MASK_m13	( FILE_FLAGS_READ_m13 | FILE_FLAGS_WRITE_m13 | FILE_FLAGS_APPEND_m13 )

#define FILE_FD_EPHEMERAL_m13		((si4) -2)
#define FILE_FD_CLOSED_m13		((si4) -1)
#define FILE_FD_STDIN_m13		((si4) 0)
#define FILE_FD_STDOUT_m13		((si4) 1)
#define FILE_FD_STDERR_m13		((si4) 2)

// thread local storage
#ifdef WINDOWS_m13
	#define thread_local_m13	__declspec(thread)
#else
	#define thread_local_m13	_Thread_local
#endif

// Locking operations  [si4 to match standard flock()]
// NOTE: locking only possible with FILE_m13 file pointers
// NOTE: read -> write lock upgrade is supported: a thread holding read locks may take a write lock on the same file;
// its own read locks are absorbed into the write lock & survive it (released by their own read unlocks)
// NOTE: multiprocess locking (globals miscellaneous.file_lock_multiprocess == TRUE_m13) extends locks across
// concurrent library instances (e.g. acquisition writing while separate reader processes follow live):
// os level whole file locks (flock() in MacOS/Linux, LockFileEx() in Windows - both per open description) are
// held on a dedicated duplicated descriptor (user closes cannot drop them) & changed only at lock state
// transitions (first reader in, writer in, last holder out) => no cost per operation, none at all when off;
// between processes the os locks are advisory - as all cooperating processes use this library, enforcement
// is absolute within them; CAVEAT: lock upgrades & downgrades briefly release the os lock (flock() semantics);
// CAVEAT: locks do NOT span machines over network file systems (verified: MacOS smbfs client locks are not
// visible to processes on the file server & vice versa) - multiprocess locking is a same-machine guarantee

// multiprocess (os level) lock states
#define FLOCK_OS_NONE_m13		((si1) 0)
#define FLOCK_OS_SHARED_m13		((si1) 1)
#define FLOCK_OS_EXCLUSIVE_m13		((si1) 2)
#define FLOCK_OPEN_m13			((si4) 1 << 0) // increment open count +/- create lock
#define FLOCK_CLOSE_m13			((si4) 1 << 1) // decrement open count +/- destroy lock
#define FLOCK_LOCK_m13			((si4) 1 << 2) // lock (with mode)
#define FLOCK_UNLOCK_m13		((si4) 1 << 3) // unlock (with mode)
#define FLOCK_READ_m13			((si4) 1 << 4) // read lock mode
#define FLOCK_WRITE_m13			((si4) 1 << 5) // write lock mode
#define FLOCK_NON_BLOCKING_m13		((si4) 1 << 6) // do not block for lock, return FLOCK_LOCKED_m13 immediately
#define FLOCK_TIMEOUT_m13		((si4) 1 << 7) // blocking (looped) or non-blocking (once) sleep time (as nap string) included as vararg

// Return values
#define FLOCK_SUCCESS_m13		((si4) 0) // locking operation succeeded
#define FLOCK_ERR_m13			((si4) -1) // locking operation generated error
#define FLOCK_LOCKED_m13		((si4) -2) // can't unlock due to ownership, or still locked in non-blocking mode

// Convenience
#define FLOCK_READ_LOCK_m13		( FLOCK_LOCK_m13 | FLOCK_READ_m13 )
#define FLOCK_WRITE_LOCK_m13		( FLOCK_LOCK_m13 | FLOCK_WRITE_m13 )
#define FLOCK_READ_LOCK_NB_m13		( FLOCK_LOCK_m13 | FLOCK_READ_m13 | FLOCK_NON_BLOCKING_m13 )
#define FLOCK_WRITE_LOCK_NB_m13		( FLOCK_LOCK_m13 | FLOCK_WRITE_m13 | FLOCK_NON_BLOCKING_m13 )
#define FLOCK_READ_UNLOCK_m13		( FLOCK_UNLOCK_m13 | FLOCK_READ_m13 )
#define FLOCK_WRITE_UNLOCK_m13		( FLOCK_UNLOCK_m13 | FLOCK_WRITE_m13 )

// global default locking modes
// NOTE: individual files can be locked by setting FLOCK_LOCK_m13 bit regardless global mode
#define FLOCK_MODE_NONE_m13		((si1) 0) // do not automatically lock any files (locking still possible)
#define FLOCK_MODE_MED_m13		((si1) 1) // automatically lock MED files
#define FLOCK_MODE_ALL_m13		((si1) 2) // automatically lock all files

// FILE_m13 standard streams
#define stdin_m13	((FILE_m13 *) stdin)
#define stdout_m13	((FILE_m13 *) stdout)
#define stderr_m13	((FILE_m13 *) stderr)

// Typedefs
typedef struct {
	ui4	tag; // == FILE_TAG_m13 (marker for FILE_m13 vs FILE)
	ui2	flags;
	ui2	perms; // system file permissions (lower 12 bits of "st_mode" element of stat structure)
	si1	path[PATH_BYTES_m13]; // not necessarily unique to physical file (links/aliases)
	union {
		ui8	fid; // unique to physical file
		struct {
			ui4	device_id;
			union {
				ui4	inode_id;
				ui4	path_crc; // failsafe (inode_id == 0)
			};
		};
	};
	FILE	*fp; // system FILE pointer
	struct FLOCK_ENTRY_m13	*lock; // cached lock list entry (set by flock_m13(); NULL until first locking operation, cleared on close)
	si4	fd; // system file descriptor (not necessarily unique to physical file (dup/dup2)
	si1	pad_bytes[4]; // future use (bytes will be there anyway)
	si8	len; // current file length (-1 indicates not set)
	si8	pos; // file pointer position (relative to start)  (-1 indicates not set)
	si8	acc; // uutc of last file access (open, read, or write functions, if FILE_FLAGS_TIME_m13 bit set)
	struct DGST_STREAM_m13	*dgst; // streaming canonical digest state (DGST module; NULL unless FILE_FLAGS_DIGEST_m13 set)
} FILE_m13;

// Prototypes
FILE_m13 	*FILE_from_std_m13(FILE *std_fp, const si1 *path);
ui8		FILE_id_m13(void *fp, const si1 *path);
FILE_m13	*FILE_init_m13(void *fp, ...); // varargs(fp == stream): const si1 *path
tern		FILE_is_std_m13(void *fp);
tern		FILE_locking_m13(void *fp, tern heed); // turn locking on or off for a file (heed or ignore locks on fp)
tern		FILE_same_m13(void *fp1, void *fp2);
tern		FILE_show_m13(FILE_m13 *fp);
FILE 		*FILE_std_m13(void *fp);
FILE 		*FILE_to_std_m13(void *fp, si1 *path);
void		FILE_update_m13(void *fp);



//**********************************************************************************//
// MARK: Processes (PROC)
//**********************************************************************************//

// Thread Management Constants
#define PROC_JOBS_PER_CORE_DEFAULT_m13		0x7FFFFFFF // sentinel: engages session-adaptive tuning - starts at PROC_JOBS_PER_CORE_BASE_m13,
							// steered by measured core utilization (see PROC_jobs_distribute_m13());
							// pass an explicit count to fix jobs per core (no tuning)
#define PROC_JOBS_PER_CORE_BASE_m13		2 // adaptive tuning starting point - benchmarked (M3 Max, decode-dominant jobs with
							// interleaved I/O stalls): 2 runs ~2x faster than 1 (cores pick up decode during
							// other jobs' I/O); 3 no faster & increases the transient memory footprint;
							// change only with due consideration (& benchmarks)
#define PROC_JOBS_PER_CORE_MAX_m13		4 // ceiling for session-adaptive tuning
#define PROC_DEFAULT_PRIORITY_m13 		0x7FFFFFFF
#define PROC_MIN_PRIORITY_m13			0x7FFFFFFE
#define PROC_LOW_PRIORITY_m13			0x7FFFFFFD
#define PROC_MEDIUM_PRIORITY_m13		0x7FFFFFFC
#define PROC_HIGH_PRIORITY_m13			0x7FFFFFFB
#define PROC_MAX_PRIORITY_m13			0x7FFFFFFA
#define PROC_UNDEFINED_PRIORITY_m13		0x7FFFFFF9

// PROC_THREAD_INFO_m13 status values
#define PROC_THREAD_WAITING_m13			((ui4) 0)
#define PROC_THREAD_RUNNING_m13			((ui4) 1 << 0)
#define PROC_THREAD_SUCCEEDED_m13		((ui4) 1 << 1)
#define PROC_THREAD_FAILED_m13			((ui4) 1 << 2)
#define PROC_THREAD_SKIPPED_m13			((ui4) 1 << 3)
#define PROC_THREAD_FINISHED_m13		( PROC_THREAD_SUCCEEDED_m13 | PROC_THREAD_FAILED_m13 | PROC_THREAD_SKIPPED_m13 )

#define THREAD_NAME_BYTES_m13		64


typedef ui8	pid_t_m13; // big enough for all OSs, none use signed values
			   // (pid_t_m13 is used for both process and thread IDs throughout the library)

typedef void 	(*sig_handler_t_m13)(si4); // signal handler function pointer

#if defined MACOS_m13 || defined LINUX_m13
	#ifdef MACOS_m13
	typedef	struct {
		si4	cpu_array[64]; // selected cpu numbers (from zero); cast cpu_array to thread_policy_t
		ui4	cpu_count; // number of cpus in cpu_array; cast to mach_msg_type_number_t
	}				cpu_set_t_m13; // max 64 logical cores (in this implementation, no actual limit)
	#endif // MACOS_m13
	#ifdef LINUX_m13
	typedef	cpu_set_t		cpu_set_t_m13; // opaque type (unclear if there is a logical core count limit)
	#endif // LINUX_m13

// pthreads
	typedef	pthread_t		pthread_t_m13; // opaque type (appears to be large structure: 4-8 KB)
	typedef pthread_attr_t		pthread_attr_t_m13;
	typedef void *			pthread_rval_m13;
	typedef pthread_rval_m13 	(*pthread_fn_m13)(void *);
	typedef	pthread_mutex_t		pthread_mutex_t_m13;
	typedef	pthread_mutexattr_t	pthread_mutexattr_t_m13;

// semaphores
	typedef sem_t			sem_t_m13;
#endif // MACOS_m13 || LINUX_m13

#ifdef WINDOWS_m13
	typedef	ui8			cpu_set_t_m13; // max 64 logical cores (defined as DWORD_PTR in Windows, but not used as a pointer; used as ui8)

// pthreads
	typedef	HANDLE			pthread_t_m13; // == void * (opaque type)
	typedef void *			pthread_attr_t_m13;
	typedef ui4 			pthread_rval_m13;
	typedef pthread_rval_m13 	(*pthread_fn_m13)(void *);
	typedef	SRWLOCK			pthread_mutex_t_m13; // user-mode lock, no syscall on uncontended paths (a kernel Mutex object costs a syscall per op)
						// zero-initialized == SRWLOCK_INIT == valid unlocked lock (statically initialized & calloc'd mutexes work without init, as with PTHREAD_MUTEX_INITIALIZER)
						// not recursive (matches POSIX default mutexes); no abandoned-lock rescue (POSIX has none either); must be released by the acquiring thread
	typedef	SECURITY_ATTRIBUTES	pthread_mutexattr_t_m13; // accepted for interface parity; ignored (SRWLOCKs take no attributes)

// semaphores
	typedef	HANDLE			sem_t_m13;
#endif // WINDOWS_m13

// inverse semaphores (isems)

// function as semaphores that block when count > 0
// function interfaces resemble, but are not the same as sempahore function interfaces
// (implemented with mutex because atomic alone can't guarantee no change between accesses in isem functions)
// (implemented with atomic because mutex alone can't guarantee global value updated between thread accesses)

// isems also have optional ownership
// if an isem is owned by another thread, accesses that would increase the count will block
// (or fail in try functions) until ownership is released or transferred to the calling thread

// example usage: flock reader count => write request blocks in isem until reader count == 0

// the two special OWNER VALUES, accepted wherever an owner is passed - isem_init_m13()'s owner argument &
// isem_chown_m13()'s tid. Both name a value rather than an action, so they read correctly in both places
// ("initialize with no owner", "change owner to no owner"). A verb form would not: there is no
// "initialize with unown".
#define ISEM_SELF_m13		((pid_t_m13) 0xFFFFFFFFFFFFFFFF) // the calling thread
#define ISEM_NO_OWNER_m13	((pid_t_m13) 0) // unowned: releases ownership when passed to isem_chown_m13()
#define ISEM_NAME_BYTES_m13	30 // short ascii names (size chosen for alignment with subsequent two terns)

typedef struct {
	_Atomic ui4		count;
	ui4			period; // checking period, in nanoseconds (max ~4.3 secs); 0 indicates no wait in try functions, default (10 us) in non-try functions
	_Atomic pid_t_m13	owner; // thread id of owning thread; zero indicates no ownership
	pthread_mutex_t_m13	mutex;
	si1 			name[ISEM_NAME_BYTES_m13];
	_Atomic tern		initialized;
	tern			free_on_destroy;
} isem_t_m13;

// Process Structures

typedef struct {
	const si1		*name; // typically the name of the job function (if heap allocated, caller responsible for freeing)
	pthread_fn_m13		function; // pointer to the function to be threaded [ prototype: pthread_rval_m13 function(void *) ]
	void			*function_arg; // pointer to data function() expects as input
	const si1		*affinity_str; // if not empty, a cpu set will be created; if cpu_set_p != NULL, the cpu set will be written at that address, if NULL it will remain so (not used in MacOS)
	cpu_set_t_m13		*cpu_set_p; // if not NULL & affinity_str is empty, it will be used (not used in MacOS)
	si4			priority; // if not set, system default priority will be used
	_Atomic ui4		status; // contract enforced by PROC_job_init_m13(): RUNNING set before function runs, finish status guaranteed after return (function may also set these itself)
	pid_t_m13		_id; // thread id [ set by PROC_job_init_m13() ]
	pid_t_m13		_pid; // parent thread id [ set by PROC_launch_job_m13() ]
	ui4			parent_behavior; // spawner's behavior code at launch (snapshot) [ set by PROC_job_launch_m13(); pushed as thread's base behavior by PROC_job_init_m13() ]
	tern			threaded; // TRUE_m13 to run as a thread, FALSE_m13 to run in current thread
	tern			detached; // if TRUE_m13, thread exits when finished, not joinable
	tern			skip; // if TRUE_m13, don't run this job (e.g. because data missing)
} PROC_JOB_m13;

// thread trampoline for pthread_create_m13(): carries the spawner's behavior (snapshot at spawn) into the new thread
typedef struct {
	pthread_fn_m13	fn;
	void		*arg;
	ui4		behavior;
} G_THREAD_TRAMPOLINE_m13;

// Prototypes
// Note: medlib versions of standard pthread functions are prototyped with other standard function versions
tern			PROC_adjust_open_file_limit_m13(si4 new_limit, tern verbose_flag);
tern			PROC_change_affinity_m13(pthread_t_m13 *thread_p, pthread_attr_t_m13 *attributes, cpu_set_t_m13 *cpu_set_p);
sf8			PROC_cpu_time_m13(void); // process cpu time (user + system, all threads), in seconds
tern			PROC_default_threading_m13(void *lh); // lh is an LH_m13 *
#define PROC_ENCODE_SET_BYTES_m13	((si4) 4096) // worst-case encode-set string (alternating singles on very-many-core parts)
tern			PROC_encode_cpu_set_m13(const si1 *reserved_str, si1 *encode_str, si4 *n_workers);  // encode set = logicals whose PHYSICAL core hosts no reserved logical (topology-derived; hybrid-safe)
cpu_set_t_m13		*PROC_generate_cpu_set_m13(const si1 *affinity_str, cpu_set_t_m13 *cpu_set_p);
pid_t_m13		PROC_id_for_thread_m13(pthread_t_m13 *thread_p);
tern			PROC_increase_process_priority_m13(tern verbose_flag, si4 sudo_prompt_flag, ...); // varargs (sudo_prompt_flag == TRUE_m13): const si1 *exec_name, sf8 timeout_secs
tern			PROC_jobs_distribute_m13(PROC_JOB_m13 *jobs, si4 n_jobs, si4 reserved_cores, si4 jobs_per_core, tern thread_jobs, tern wait_jobs);
pthread_rval_m13	PROC_job_init_m13(void *arg);
tern			PROC_job_launch_m13(PROC_JOB_m13 *job);
tern			PROC_jobs_wait_m13(PROC_JOB_m13 *jobs, si4 n_jobs);
tern			PROC_show_thread_affinity_m13(pthread_t_m13 *thread_p);
void			PROC_show_thread_list_m13(void);
pthread_t_m13		*PROC_thread_for_id_m13(pid_t_m13 _id);
void			PROC_thread_list_add_m13(pid_t_m13 _id, pid_t_m13 _pid, pthread_t_m13 *thread_p, const si1 *name);
void			PROC_thread_list_remove_m13(pid_t_m13 _id);
const si1		*PROC_thread_name_m13(pid_t_m13 _id);
pid_t_m13		PROC_thread_parent_id_m13(pid_t_m13 _id);



//**********************************************************************************//
// MARK: Parallel Functions (PAR)
//**********************************************************************************//

// The PAR functions run other library functions in threads, without requiring the caller to know
// anything about threading. Each PAR_INFO_m13 is a handle ("future") for one running function:
// launch, poll status (or wait), then collect the return value & error info.
// Primary use case: true parallelism for single-threaded host languages (e.g. the Python wrapper),
// such as concurrent reads of multiple sessions.
//
// The target function is passed by pointer, its arguments described by a signature string & captured
// at launch, so any library function can be run in parallel without PAR-specific code, given:
//
// 1) the target function must return a pointer; NULL return is interpreted as failure
// 2) signature characters: 'p' == pointer, 'i' == integer, 'd' == sf8
//    '|' == start of variadic arguments (matches the "..." of variadic target prototypes)
// 3) integer arguments should be cast to si8 at the PAR_launch_m13() call site
//    (like printf() "%ld" arguments); pointer & sf8 arguments are captured as passed
// 4) sf8 arguments are currently supported only in the variadic shapes enumerated in
//    PAR_call_m13() (e.g. DM_get_matrix_m13()); add shapes there as needed
// 5) on job failure, error info set by the job's thread (or a descendant) is captured into
//    the handle & the causal error is cleared (handled) => check the handle, not G_error_code_m13()
//    (launch-time errors are set on the caller's thread as usual => caller handles)
//
// example (concurrent reads of two sessions):
//	par_a = PAR_launch_m13(NULL, (void *) G_open_session_m13, "pppiipp", NULL, slice_a, list_a, (si8) len_a, (si8) lh_flags, password, NULL);
//	par_b = PAR_launch_m13(NULL, (void *) G_open_session_m13, "pppiipp", NULL, slice_b, list_b, (si8) len_b, (si8) lh_flags, password, NULL);
//	PAR_wait_m13(par_a, NULL);
//	PAR_wait_m13(par_b, NULL);
//	sess_a = (SESS_m13 *) par_a->r_val;  // NULL on failure (error info in handle)
//	sess_b = (SESS_m13 *) par_b->r_val;
//
// large fan-outs: capture each call with PAR_prep_m13() (same arguments as PAR_launch_m13()),
// then PAR_distribute_m13() launches the set throttled to the available cores
// (as PROC_jobs_distribute_m13() does for PROC jobs)
// (core counts from HW_PARAMS_m13; all cores are used by default - benchmarks show heterogeneous CPUs
// (e.g. Apple silicon) run integer-heavy job sets fastest across ALL cores; to keep efficiency cores
// free for system responsiveness pass reserved_cores == efficiency_cores; on hyperthreaded CPUs
// compute-bound jobs may run best throttled to physical cores: reserved_cores == logical - physical;
// jobs_per_core: pass PROC_JOBS_PER_CORE_DEFAULT_m13 for session-adaptive tuning (recommended; starts
// at PROC_JOBS_PER_CORE_BASE_m13 & converges on the measured optimum over repeated calls))

// Constants
#define PAR_MAX_ARGS_m13		16 // maximum total arguments
#define PAR_MAX_FIXED_ARGS_m13		8 // maximum fixed arguments of a variadic target
#define PAR_MAX_TAIL_ARGS_m13		8 // maximum variadic (post '|') arguments
#define PAR_SIG_BYTES_m13		( PAR_MAX_ARGS_m13 + 2 ) // signature characters, '|', terminal zero
#define PAR_AFFINITY_BYTES_m13		16
#define PAR_MSG_BYTES_m13		( (PATH_BYTES_m13 << 1) + 128 ) // == E_MAX_MSG_LEN_m13 (defined below)
#define PAR_DEFAULTS_m13		"defaults"

// Status Macros (status values defined with PROC constants)
#define PAR_RUNNING_m13(par_info)	( ((par_info)->job.status == PROC_THREAD_RUNNING_m13) ? TRUE_m13 : FALSE_m13 )
#define PAR_FINISHED_m13(par_info)	( ((par_info)->job.status & PROC_THREAD_FINISHED_m13) ? TRUE_m13 : FALSE_m13 )
#define PAR_SUCCEEDED_m13(par_info)	( ((par_info)->job.status == PROC_THREAD_SUCCEEDED_m13) ? TRUE_m13 : FALSE_m13 )
#define PAR_FAILED_m13(par_info)	( ((par_info)->job.status == PROC_THREAD_FAILED_m13) ? TRUE_m13 : FALSE_m13 )

// Structures
typedef union {
	void	*p; // pointer arguments ('p')
	si8	i; // integer arguments ('i')
	sf8	d; // sf8 arguments ('d')
} PAR_ARG_m13;

typedef struct {
	PROC_JOB_m13	job; // thread control (function, priority, affinity, threading, status)
	void		*fn; // target function [ must return a pointer; NULL return interpreted as failure ]
	void		*r_val; // target function return value [ valid when PAR_SUCCEEDED_m13() ]
	const si1	*error_function; // function that set error (static string; error info captured at job finish)
	PAR_ARG_m13	args[PAR_MAX_ARGS_m13]; // captured target function arguments (unused cells zero)
	si4		n_args; // total captured arguments
	si4		n_fixed; // arguments before '|' (== n_args if target not variadic)
	si4		error_code; // E_MED_CODES_m13 value at job finish (E_NONE_m13 if no error)
	si4		error_line; // line that set error
	si1		error_message[PAR_MSG_BYTES_m13]; // may originate in a descendant thread (causal error)
	si1		label[THREAD_NAME_BYTES_m13]; // thread name & job description
	si1		affinity[PAR_AFFINITY_BYTES_m13]; // cpu affinity string (empty => any cpu)
	si1		sig[PAR_SIG_BYTES_m13]; // argument signature string
	tern		variadic; // sig contains '|'
	tern		allocated; // allocated by PAR library code => freed by PAR_free_m13()
} PAR_INFO_m13;

// Prototypes
void			*PAR_call_m13(PAR_INFO_m13 *par_info); // executes captured call in calling thread
tern			PAR_distribute_m13(PAR_INFO_m13 **par_infos, si4 n_infos, si4 reserved_cores, si4 jobs_per_core, tern thread_jobs, tern wait_jobs); // launch prepped handles throttled to cores (args as in PROC_jobs_distribute_m13())
tern			PAR_free_m13(PAR_INFO_m13 **par_info_ptr);
PAR_INFO_m13		*PAR_init_m13(PAR_INFO_m13 *par_info, const si1 *label, ...); // varargs(label != PAR_DEFAULTS_m13 or NULL): si4 priority, const si1 *affinity, si4 threaded
PAR_INFO_m13		*PAR_launch_m13(PAR_INFO_m13 *par_info, void *fn, const si1 *sig, ...); // varargs: target function arguments, per sig
PAR_INFO_m13		*PAR_prep_m13(PAR_INFO_m13 *par_info, void *fn, const si1 *sig, ...); // varargs: target function arguments, per sig (capture without launching)
PAR_INFO_m13		*PAR_prep_exec_m13(PAR_INFO_m13 *par_info, void *fn, const si1 *sig, va_list v_args);
tern			PAR_show_info_m13(PAR_INFO_m13 *par_info);
tern			PAR_start_m13(PAR_INFO_m13 *par_info); // launch a prepped handle
pthread_rval_m13	PAR_thread_m13(void *arg);
tern			PAR_wait_m13(PAR_INFO_m13 *par_info, const si1 *interval);
tern			PAR_wait_all_m13(PAR_INFO_m13 **par_infos, si4 n_infos, const si1 *interval);



//**********************************************************************************//
// MARK: Parity (PRTY)
//**********************************************************************************//

// NOTE: names beginning with "parity" are RESERVED - they identify parity files & directories (PRTY_is_parity_m13())
// writers should reject them for sessions, channels, & segments at name creation time (G_valid_name_m13())

// Flags
#define PRTY_GLB_SESS_REC_DATA_m13	((ui4) 1 << 0)
#define PRTY_GLB_SESS_REC_IDX_m13	((ui4) 1 << 1)
#define PRTY_SEG_SESS_REC_DATA_m13	((ui4) 1 << 2)
#define PRTY_SEG_SESS_REC_IDX_m13	((ui4) 1 << 3)
#define PRTY_TS_CHAN_REC_DATA_m13	((ui4) 1 << 4)
#define PRTY_TS_CHAN_REC_IDX_m13	((ui4) 1 << 5)
#define PRTY_TS_SEG_REC_DATA_m13	((ui4) 1 << 6)  // requires segment number (or PRTY_ALL_SEGS_m13)
#define PRTY_TS_SEG_REC_IDX_m13		((ui4) 1 << 7)  // requires segment number (or PRTY_ALL_SEGS_m13)
#define PRTY_TS_SEG_DAT_DATA_m13	((ui4) 1 << 8)  // requires segment number (or PRTY_ALL_SEGS_m13)
#define PRTY_TS_SEG_DAT_IDX_m13		((ui4) 1 << 9)  // requires segment number (or PRTY_ALL_SEGS_m13)
#define PRTY_TS_SEG_META_m13		((ui4) 1 << 10)  // requires segment number (or PRTY_ALL_SEGS_m13)
#define PRTY_VID_CHAN_REC_DATA_m13	((ui4) 1 << 11)
#define PRTY_VID_CHAN_REC_IDX_m13	((ui4) 1 << 12)
#define PRTY_VID_SEG_REC_DATA_m13	((ui4) 1 << 13)  // requires segment number (or PRTY_ALL_SEGS_m13)
#define PRTY_VID_SEG_REC_IDX_m13	((ui4) 1 << 14)  // requires segment number (or PRTY_ALL_SEGS_m13)
#define PRTY_VID_SEG_DAT_DATA_m13	((ui4) 1 << 15)  // requires segment number (or PRTY_ALL_SEGS_m13)
#define PRTY_VID_SEG_DAT_IDX_m13	((ui4) 1 << 16)  // requires segment number (or PRTY_ALL_SEGS_m13)
#define PRTY_VID_SEG_META_m13		((ui4) 1 << 17)  // requires segment number (or PRTY_ALL_SEGS_m13)

#define PRTY_GLB_SESS_RECS_m13		( PRTY_GLB_SESS_REC_DATA_m13 | PRTY_GLB_SESS_REC_IDX_m13 )
#define PRTY_SSR_m13			( PRTY_SEG_SESS_REC_DATA_m13 | PRTY_SEG_SESS_REC_IDX_m13 )
#define PRTY_TS_CHAN_RECS_m13		( PRTY_TS_CHAN_REC_DATA_m13 | PRTY_TS_CHAN_REC_IDX_m13 )
#define PRTY_VID_CHAN_RECS_m13		( PRTY_VID_CHAN_REC_DATA_m13 | PRTY_VID_CHAN_REC_IDX_m13 )
#define PRTY_TS_SEG_RECS_m13		( PRTY_TS_SEG_REC_DATA_m13 | PRTY_TS_SEG_REC_IDX_m13 )
#define PRTY_VID_SEG_RECS_m13		( PRTY_VID_SEG_REC_DATA_m13 | PRTY_VID_SEG_REC_IDX_m13 )
#define PRTY_TS_SEG_DATA_m13		( PRTY_TS_SEG_DAT_DATA_m13 | PRTY_TS_SEG_DAT_IDX_m13 )
#define PRTY_VID_SEG_DATA_m13		( PRTY_VID_SEG_DAT_DATA_m13 | PRTY_VID_SEG_DAT_IDX_m13 )

#define PRTY_SESS_RECS_m13		( PRTY_GLB_SESS_RECS_m13 | PRTY_SSR_m13 )
#define PRTY_CHAN_RECS_m13		( PRTY_TS_CHAN_RECS_m13 | PRTY_VID_CHAN_RECS_m13 )
#define PRTY_SEG_RECS_m13		( PRTY_TS_SEG_RECS_m13 | PRTY_VID_SEG_RECS_m13 )
#define PRTY_SEG_DATA_m13		( PRTY_TS_SEG_DATA_m13 | PRTY_VID_SEG_DATA_m13 )

#define PRTY_TS_CHAN_m13		PRTY_TS_CHAN_RECS_m13
#define PRTY_TS_SEG_m13			( PRTY_TS_SEG_RECS_m13 | PRTY_TS_SEG_DATA_m13 | PRTY_TS_SEG_META_m13 )
#define PRTY_VID_CHAN_m13		PRTY_VID_CHAN_RECS_m13
#define PRTY_VID_SEG_m13		( PRTY_VID_SEG_RECS_m13 | PRTY_VID_SEG_DATA_m13 | PRTY_VID_SEG_META_m13 )

#define PRTY_SESS_m13			( PRTY_SESS_RECS_m13 | PRTY_SSR_m13 )
#define PRTY_CHAN_m13			( PRTY_TS_CHAN_m13 | PRTY_VID_CHAN_m13 )
#define PRTY_SEG_m13			( PRTY_TS_SEG_m13 | PRTY_VID_SEG_m13 )

#define PRTY_ALL_TS_m13			( PRTY_SESS_m13 | PRTY_TS_CHAN_m13 | PRTY_TS_SEG_m13 )
#define PRTY_ALL_VID_m13		( PRTY_SESS_m13 | PRTY_VID_CHAN_m13 | PRTY_VID_SEG_m13 )
#define PRTY_ALL_FILES_m13		( PRTY_ALL_TS_m13 | PRTY_ALL_VID_m13 )
#define PRTY_ALL_SEGS_m13		((si4) -1) // pass as "segment_number" argument to PRTY_write_m13()

// Masks
#define PRTY_TS_MASK_m13		( PRTY_TS_CHAN_m13 | PRTY_TS_SEG_m13 )
#define PRTY_VID_MASK_m13		( PRTY_VID_CHAN_m13 | PRTY_VID_SEG_m13 )

// Parity file array fixed positions
#define PRTY_FILE_CHECK_IDX_m13		0 		  // file to check in first slot
#define PRTY_FILE_DAMAGED_IDX_m13	PRTY_FILE_CHECK_IDX_m13  // damaged file in first slot

// Miscellaneous
#define PCRC_BLOCK_BYTES_DEFAULT_m13	4096 // used in PCRC_DATA_m13 (must be multiple of 4)
#define PCRC_TAG_m13		((ui8) 0x0123456789ABCDEF) // used in PCRC_DATA_m13 (note this is same as CMP_BLOCK_START_UID_m13 - mistake but benign in practice)
// manifest tag: high 56 bits identify the structure, low 8 bits are its STRUCTURE GENERATION - the tag is the only
// thing a reader locates independently, so putting the generation here lets it decide whether it can parse the
// structure at all before trusting any field inside it (version_major/minor cannot validate their own position).
// base mismatch => no manifest (legacy pcrc data); base match + unknown generation => FAIL CLOSED, never guess.
// bump the generation when the binary LAYOUT moves; bump version_major when a field's MEANING changes in place.
#define PRTY_MANIFEST_TAG_BASE_m13	((ui8) 0xFEDCBA9876543200) // high 56 bits (distinct from PCRC_TAG_m13)
#define PRTY_MANIFEST_TAG_GEN_MASK_m13	((ui8) 0xFF)
#define PRTY_MANIFEST_TAG_GEN_m13	((ui8) 1) // current structure generation: PRTY_MEMBER_m13 records
#define PRTY_MANIFEST_TAG_m13		(PRTY_MANIFEST_TAG_BASE_m13 | PRTY_MANIFEST_TAG_GEN_m13)
#define PRTY_MANIFEST_VER_MAJOR_m13	((ui1) 1)
#define PRTY_MANIFEST_VER_MINOR_m13	((ui1) 0)


// Structures
typedef struct {
	si1		path[PATH_BYTES_m13];
	si8		len;
	FILE_m13	*fp;
	tern		finished; // data incorporated into parity
} PRTY_FILE_m13;

typedef struct {
	si8		length;
	si8		offset;
} PRTY_BLOCK_m13; // bad block location returned from PRTY_validate_m13()

typedef struct {
	ui1		*parity;
	ui1		*data;
	si8		mem_block_bytes;
	si1		path[PATH_BYTES_m13]; // path to parity file
	PRTY_FILE_m13	*files;
	si4		n_files;
	si4		n_bad_blocks;
	PRTY_BLOCK_m13	*bad_blocks;
	// verify mode (PRTY_verify_m13): compare the computed parity against the parity file ALREADY on disk
	// instead of writing it, to catch a parity that has drifted out of sync with its members - which
	// rebuilding from scratch can never detect.  NOTE: PRTY_m13 is stack-allocated & not zeroed by its
	// users, so any field added here must be set explicitly by every caller.
	tern		verify;
	si8		mismatch_bytes;		// verify mode: differing bytes (0 == parity is correct)
	si8		first_mismatch_offset;	// verify mode: offset of the first differing byte, -1 if none
} PRTY_m13;

typedef struct {
	ui8		tag; // marker to confirm identity of this structure
	ui8		session_UID; // present in all parity files (same names - in case misplaced)
	ui8		segment_UID; // UID_NO_ENTRY_m13 (zero) in parity data that is session level (same names - in case misplaced)
	ui4		n_blocks; // number of data blocks (& crcs) preceding this structure
	ui4		block_bytes; // data bytes per block (except probably the last), multiple of 4 bytes (defaults to PCRC_BLOCK_BYTES_DEFAULT_m13)
} PCRC_DATA_m13;  // used to add CRCs to files that don't have many (e.g. index files), or any (e.g. video data files)

// PARITY MANIFEST: parity files have no universal headers (their bodies are the xor of their member files), so the manifest carries their identity
// (names alone cannot: all sessions use the same reserved parity names & channel names are commonly reused across unrelated sessions).
// It is parity-specific & is NOT part of the pcrc facility - pcrc is the general appended-block-CRC mechanism for localizing damage, which
// parity files, video files & (optionally) index files all use.  The manifest merely LIVES inside the pcrc-covered region; that is placement, not kinship.
//
// manifest layout, in file order:  [ PRTY_MEMBER_m13 * n_members, ascending by file_UID ] [ PRTY_MANIFEST_m13 ]
//
// written between the parity data & the pcrc crcs, & covered by them, so it is transparent to pre-manifest readers (which locate everything relative to the file end)
// everything is located backward from the tag at the end of the structure, so the manifest is end-anchored like the pcrc data itself
//
// session_UID & segment_UID are NOT duplicated here: they are carried by PCRC_DATA_m13 (below).  Those fields are unreliable in
// legacy files only, & legacy files have no manifest - so the presence of a valid manifest is what certifies them.  channel_UID has
// no PCRC_DATA_m13 field (video parity is paired within a segment, so it needs an associated channel as well), so it lives here.
//
// legacy pcrc data (m12 & early m13) has no manifest: the UID fields of PCRC_DATA_m13 were never set in those files & are unreliable
//
// versioning: tag & version fields are end-anchored & never move; fields preceding them may change with the tag's structure generation
typedef struct {
	ui1	protected_region[2]; // zeros
	ui1	version_major; // PRTY_MANIFEST_VER_MAJOR_m13
	ui1	version_minor; // PRTY_MANIFEST_VER_MINOR_m13
	ui4	n_members; // number of PRTY_MEMBER_m13 records preceding this structure
	ui8	channel_UID; // channel UID if common to all member files (e.g. video data parity), otherwise UID_NO_ENTRY_m13
	ui8	tag; // PRTY_MANIFEST_TAG_m13 (identity + structure generation; located backward from start of pcrc crcs)
} PRTY_MANIFEST_m13; // 24 bytes

// One member file's identity & content stamp AS OF THE PARITY BUILD.  A parity file cannot be trusted for repair unless its members
// still hold the bytes it was built from, & that CANNOT be detected from the parity arithmetic at repair time: with a member missing
// or damaged there is one equation & one unknown, so the missing member absorbs any inconsistency - a stale parity & a damaged member
// are indistinguishable.  The check therefore has to be metadata recorded BEFORE the damage, which is what these fields are.
//
// lib_mod_time & the CRCs are read straight from each member's universal header, so verifying costs one UH read (1 KiB) per member,
// not a pass over the data.  That catches every library-mediated divergence (a record appended with update_parity off, a re-key, a
// version update).  It does NOT catch bytes rotting underneath an unchanged UH - PRTY_validate_m13() & the pcrc block crcs cover that.
typedef struct {
	ui8	file_UID; // member file UID (records sorted ascending on this)
	si8	lib_mod_time; // member's UH lib_mod_time when the parity was built (0 == member unstamped)
	ui4	header_CRC; // member's UH header_CRC when the parity was built
	ui4	body_CRC; // member's UH body_CRC when the parity was built (CRC_NO_ENTRY_m13 == not recorded)
} PRTY_MEMBER_m13; // 24 bytes

// in-memory aggregate of a parity file's full identity: NOT an on-disk layout.  session_UID & segment_UID are read from
// PCRC_DATA_m13; channel_UID, version, & the member manifest from the extension.  returned by PRTY_manifest_m13() &
// PRTY_set_manifest_m13() so callers have one object; the fields are written to / read from disk separately (see above).
typedef struct {
	ui8	session_UID; // from PCRC_DATA_m13
	ui8	channel_UID; // from PRTY_MANIFEST_m13
	ui8	segment_UID; // from PCRC_DATA_m13
	ui1	version_major; // from PRTY_MANIFEST_m13
	ui1	version_minor;
	ui4	n_members;
	PRTY_MEMBER_m13	members[]; // ascending by file_UID (flexible array member)
} PRTY_IDENT_m13;

// Parity File Structure:
// 1) parity data
// 2) pcrc extension (member file UIDs + PRTY_MANIFEST_m13) // identity of parity data (parity files have no universal headers); absent in legacy files
// 3) crc of parity data (& extension) in blocks // used to confirm that parity data is not itself damaged, & if so, to localize the damage, so that it can hopefully still be used & then rebuilt
// 4) PCRC_DATA_m13 structure

// PCRC Prototypes
// pcrc is the GENERAL appended-block-CRC facility: a file's data is covered by per-block CRCs written past
// its end, so damage can be LOCALIZED rather than merely detected.  Parity files use it (they have no
// universal header of their own), video data files use it, and index files may.  It is not a parity concept:
// the former name for PCRC_validate_m13() was PRTY_validate_pcrc_m13, and applied to a video file it read
// wrong, which is why this namespace was split out.
si8	PCRC_block_bytes_m13(FILE_m13 *fp, const si1 *file_path);
tern	PCRC_find_damage_m13(const si1 *file_path, PRTY_BLOCK_m13 **bad_blocks, si4 *n_bad_blocks, ui4 *n_blocks); // WHICH blocks fail; any argument may be NULL
si8	PCRC_offset_m13(FILE_m13 *fp, const si1 *file_path, PCRC_DATA_m13 *pcrc);
tern	PCRC_show_m13(const si1 *file_path);
tern	PCRC_update_m13(void *ptr, si8 n_bytes, si8 offset, void *fp, ...);  // vararg(fp == FILE *): const si1 *path)
tern	PCRC_validate_m13(const si1 *file_path); // verdict only; stops at the first bad block
tern	PCRC_write_m13(const si1 *file_path, ui4 block_bytes);

// PRTY Prototypes
// parity proper: the xor of a group of member files, & the manifest recording which members it was built
// from.  The manifest merely LIVES inside the pcrc-covered region - placement, not kinship.
tern	PRTY_build_m13(PRTY_m13 *parity_ps);
si4	PRTY_file_compare_m13(const void *a, const void *b);
si1	**PRTY_file_list_m13(const si1 *MED_path, si4 *n_files);
tern	PRTY_find_damage_m13(const si1 *file_path, PRTY_BLOCK_m13 **bad_blocks, si4 *n_bad_blocks, ui4 *n_blocks); // WHERE a MED file is damaged; any argument may be NULL
ui4	PRTY_flag_for_path_m13(const si1 *path);
tern	PRTY_is_parity_m13(const si1 *path, tern MED_file);
PRTY_IDENT_m13	*PRTY_manifest_m13(FILE_m13 *fp, const si1 *file_path, si8 *ext_offset);
tern	PRTY_recover_segment_header_fields_m13(const si1 *MED_file, ui8 *segment_uid, si4 *segment_number);
tern	PRTY_repair_file_m13(PRTY_m13 *parity_ps);
tern	PRTY_restore_m13(const si1 *MED_path);
PRTY_IDENT_m13	*PRTY_set_manifest_m13(PCRC_DATA_m13 *pcrc, const si1 *MED_path);
tern	PRTY_truncate_m13(void *fp, si8 new_len, ...);  // vararg(fp == FILE *): const si1 *path  [called by ftruncate_m13() before the file shortens]
tern	PRTY_update_m13(void *ptr, si8 n_bytes, si8 offset, void *fp, ...);  // vararg(fp == FILE *): const si1 *path
tern	PRTY_update_manifest_m13(void *ptr, si8 n_bytes, si8 offset, void *fp, ...);  // vararg(fp == FILE *): const si1 *path  [called by fwrite_m13() when a write carries a member's universal header]
tern	PRTY_validate_m13(const si1 *file_path); // verdict: is this MED file's own content intact?
tern	PRTY_verify_m13(const si1 *sess_path, ui4 flags, si4 segment_number);  // parity ON DISK == xor of its members?
tern	PRTY_write_m13(const si1 *sess_path, ui4 flags, si4 segment_number);  // (re)build parity from scratch



//**********************************************************************************//
// MARK: Runtime Configuration (RC)
//**********************************************************************************//

// Constants
#define RC_NO_ENTRY_m13		-2
#define RC_ERR_m13 		-1
#define RC_NO_OPTION_m13	0
#define RC_STRING_TYPE_m13 	1
#define RC_FLOAT_TYPE_m13 	2
#define RC_INTEGER_TYPE_m13 	3
#define RC_TERNARY_TYPE_m13 	4
#define RC_UNKNOWN_TYPE_m13 	5

#define RC_STRING_CHARS_m13	255  // maximum characters in an rc string value, EXCLUDING the terminal NUL
#define RC_STRING_BYTES_m13	(RC_STRING_CHARS_m13 + 1)  // 256: the buffer size every rc string caller must supply

// A scanf() field width has to sit inside the format string literal, so it cannot be written as arithmetic:
// stringifying (RC_STRING_BYTES_m13 - 1) yields the literal text "256 - 1", not "255". That is why
// RC_STRING_CHARS_m13 is the primitive & RC_STRING_BYTES_m13 derives from it. These two formats are the only
// places the width is spelled, so it cannot drift from the buffer size.
#define RC_STRINGIFY_HELPER_m13(x)	#x
#define RC_STRINGIFY_m13(x)		RC_STRINGIFY_HELPER_m13(x)
#define RC_LINE_FMT_m13			"%" RC_STRINGIFY_m13(RC_STRING_CHARS_m13) "[^\r\n]"   // read to end of line
#define RC_OPTION_FMT_m13		"%" RC_STRINGIFY_m13(RC_STRING_CHARS_m13) "[^,\r\n]"  // read one comma-delimited option

// RC field-table target types (see the RC field table & writer in medlib_m13.c)
#define RC_TGT_TERN_m13		1  // target is a tern in the globals
#define RC_TGT_SI4_m13		2  // target is an si4
#define RC_TGT_SI8_m13		3  // target is an si8; "shift" scales the file value (e.g. GB -> bytes)
#define RC_TGT_VERSION_m13	4  // validated, not stored
#define RC_TGT_CHATTINESS_m13	5  // maps to default_behavior.code suppression bits
#define RC_TGT_CRC_MODE_m13	6  // maps to a CRC mode bit pair
#define RC_TGT_FILE_LOCK_m13	7  // maps to a file lock mode

typedef struct {
	const si1	*name;
	const si1	*notes;		// newline separated; each line is emitted as its own "%% NOTES:" line
	const si1	*type_str;	// as it appears in the file
	const si1	*options_key;	// "OPTIONS ONLY" (closed set) or "OPTIONS" (free value)
	const si1	*options;
	const si1	*dflt;
	si4		rc_type;	// type passed to the RC parser
	si4		target_type;	// RC_TGT_*
	void		*target;	// NULL for the mapped & validated kinds
	si4		shift;		// RC_TGT_SI8_m13: file value << shift == stored value
} RC_FIELD_m13;


// Prototypes
si4	RC_read_field_m13(const si1 *field_name, si1 **buffer, tern update_buffer_ptr, si1 *field_value_str, sf8 *float_val, si8 *int_val, tern *TERN_val);
si4	RC_read_field_2_m13(const si1 *field_name, si1 **buffer, tern update_buffer_ptr, void *val, si4 val_type, ...); // vararg (val_type == RC_UNKNOWN_TYPE_m13): si4 *returned_val_type


//**********************************************************************************//
// MARK: Networking (NET)
//**********************************************************************************//

// Constants
#define NET_MAC_ADDRESS_BYTES_m13		6
#define NET_MAC_ADDRESS_STR_BYTES_m13		(NET_MAC_ADDRESS_BYTES_m13 * 3) // 6 hex bytes plus colons & terminal zero
#define NET_IPV4_ADDRESS_BYTES_m13		4
#define NET_IPV4_ADDRESS_STR_BYTES_m13		(NET_IPV4_ADDRESS_BYTES_m13 * 4) // 4 dec bytes plus periods & terminal zero (16); retained for legacy string buffers

// Family-agnostic address (MED 1.1 library 13): holds IPv4 or IPv6.
// *** On-disk stable: no platform-variable types (no sockaddr_storage), no OS AF_* values. ***
// "family" is a MED library code (NET_ADDR_FAM_IPV4/6_m13), NOT the OS AF_INET/AF_INET6 value (which differs by OS).
// Runtime code maps family <-> AF_* in local variables only (NET_af_from_family_m13 / NET_family_from_af_m13).
#define NET_ADDR_FAM_NONE_m13			((ui1) 0)  // not set
#define NET_ADDR_FAM_IPV4_m13			((ui1) 4)
#define NET_ADDR_FAM_IPV6_m13			((ui1) 6)
#define NET_ADDR_PREFIX_NO_ENTRY_m13		((ui1) 0xFF)
#define NET_ADDR_BYTES_m13			16	// holds IPv4 (in bytes[0..3]) or a full IPv6 address, network byte order
#define NET_ADDR_STR_BYTES_m13			46	// == INET6_ADDRSTRLEN; string is byte-aligned so it needs no rounding (holds any v4/v6 literal + terminal zero)


// Structures
typedef struct {			// 64 bytes, 8-byte aligned, zero padding; self-aligned for direct disk<->memory transfer
	union {				// off 0:  network byte order; the ui8 pair forces 8-alignment, so placing it first eliminates all padding
		ui1	bytes[NET_ADDR_BYTES_m13];
		struct {	// aligned 8-byte copy/compare
			ui8	hi;
			ui8	lo;
		};
	} addr;
	ui1	family;			// off 16: NET_ADDR_FAM_* (library code, not AF_*)
	ui1	prefix_len;		// off 17: CIDR prefix length (bits); replaces IPv4 dotted netmask (valid for v4 & v6); NET_ADDR_PREFIX_NO_ENTRY_m13 = none
	si1	string[NET_ADDR_STR_BYTES_m13];	// off 18..64: presentation form (inet_ntop); dotted-quad for v4, colon-hex for v6
} NET_ADDR_m13;

typedef struct {
	union {
		ui1	MAC_address_bytes[8]; // network byte order
		ui8	MAC_address_num; // network byte order
	};
	NET_ADDR_m13	LAN_address;	// interface (LAN) address; LAN_address.prefix_len carries the subnet prefix
	NET_ADDR_m13	WAN_address;	// public (WAN) address as seen from the internet
	si4		MTU;		// maximum transmission unit
	si1		interface_name[64];
	si1		host_name[256];	// max 253 ascii characters
	si1		MAC_address_string[NET_MAC_ADDRESS_STR_BYTES_m13]; // upper case hex with colons (18 bytes)
	si1		link_speed[16];
	si1		duplex[16];
	tern		active; // interface status
	tern		plugged_in;
} NET_PARAMS_m13;

// Prototypes
tern		NET_addr_set_m13(NET_ADDR_m13 *addr, const si1 *addr_str); // parse a v4 or v6 literal into family/bytes/string; FALSE if not a valid literal
si4		NET_af_from_family_m13(ui1 family); // NET_ADDR_FAM_* -> AF_INET / AF_INET6 (AF_UNSPEC if NONE)
ui1		NET_family_from_af_m13(si4 af); // AF_INET / AF_INET6 -> NET_ADDR_FAM_*
tern		NET_check_internet_connection_m13(void);
tern		NET_domain_to_ip_m13(const si1 *domain_name, si1 *ip);
ui1		NET_v4_prefix_from_mask_m13(const ui1 *mask_bytes); // dotted-quad mask bytes -> CIDR prefix length
tern		NET_v4_mask_str_from_prefix_m13(si1 *mask_str, ui1 prefix_len); // CIDR prefix length -> "255.255.255.0" (v4 only)
NET_PARAMS_m13	*NET_get_active_m13(si1 *iface, NET_PARAMS_m13 *np);
tern		NET_get_adapter_m13(NET_PARAMS_m13 *np, tern copy_global);
tern		NET_get_config_m13(NET_PARAMS_m13 *np, tern copy_global);
NET_PARAMS_m13	*NET_get_default_interface_m13(NET_PARAMS_m13 *np);
NET_PARAMS_m13	*NET_get_duplex_m13(si1 *iface, NET_PARAMS_m13 *np);
tern		NET_get_ethtool_m13(NET_PARAMS_m13 *np, tern copy_global);
NET_PARAMS_m13	*NET_get_host_name_m13(NET_PARAMS_m13 *np);
void		*NET_get_in_addr_m13(struct sockaddr *sa);
NET_PARAMS_m13	*NET_get_lan_ipv4_address_m13(si1 *iface, NET_PARAMS_m13 *np);
NET_PARAMS_m13	*NET_get_link_speed_m13(si1 *iface, NET_PARAMS_m13 *np);
NET_PARAMS_m13	*NET_get_mac_address_m13(si1 *iface, NET_PARAMS_m13 *np);
NET_PARAMS_m13	*NET_get_mtu_m13(si1 *iface, NET_PARAMS_m13 *np);
NET_PARAMS_m13	*NET_get_parameters_m13(si1 *iface, NET_PARAMS_m13 *np);
NET_PARAMS_m13	*NET_get_plugged_in_m13(si1 *iface, NET_PARAMS_m13 *np);
NET_PARAMS_m13	*NET_get_wan_ipv4_address_m13(NET_PARAMS_m13 *np);
si1		*NET_iface_name_for_addr_m13(si1 *iface_name, const si1 *iface_addr);
tern		NET_init_tables_m13(void); // set global NET_PARAMS for default internet interface
tern		NET_reset_parameters_m13(NET_PARAMS_m13 *np);
tern		NET_resolve_arguments_m13(si1 *iface, NET_PARAMS_m13 **params_ptr, tern *free_params);
tern		NET_show_parameters_m13(NET_PARAMS_m13 *np);
tern		NET_trim_address_m13(si1 *addr_str);



//**********************************************************************************//
// MARK: MED Errors
//**********************************************************************************//


// Behaviors Codes
#define RETURN_ON_FAIL_m13		((ui4) 1 << 0) // set error & return; if not set, exit on error
#define IGNORE_ERROR_m13		((ui4) 1 << 1) // don't set error & return (even if RETURN_ON_FAIL_m13 is not set)
#define SUPPRESS_ERROR_OUTPUT_m13	((ui4) 1 << 2)
#define SUPPRESS_WARNING_OUTPUT_m13	((ui4) 1 << 3)
#define SUPPRESS_MESSAGE_OUTPUT_m13	((ui4) 1 << 4)
#define RETRY_ONCE_m13			((ui4) 1 << 5) // some calls, particularly connecting to external servers, have a low benign failure rate

// Behavior constants (can't overlap with codes)
#define CURRENT_BEHAVIOR_STACK_m13	((ui4) 1 << 30)
#define CURRENT_BEHAVIOR_m13		((ui4) 1 << 31)

#define DEFAULT_BEHAVIOR_m13		RETURN_ON_FAIL_m13 // (code) return on fail, observe system errors, do not retry, show all output
#define E_BEHAVIOR_m13			((ui4) 0) // (code) exit on fail, observe system errors, do not retry, show all output
#define E_UNKNOWN_LINE_m13		((si4) -1) // used in signal errors which have no line numbers

// convenience
#define SUPPRESS_OUTPUT_m13		( SUPPRESS_ERROR_OUTPUT_m13 | SUPPRESS_WARNING_OUTPUT_m13 | SUPPRESS_MESSAGE_OUTPUT_m13 )
#define RETURN_QUIETLY_m13		( IGNORE_ERROR_m13 | RETURN_ON_FAIL_m13 | SUPPRESS_OUTPUT_m13 )  // generally used with handleable or acceptable errors

// error codes
#define E_NUM_CODES_m13		21
typedef enum {
	E_NONE_m13,		// 0, no error
	E_GEN_m13,		// 1, general / unspecified error
	E_SIG_m13,		// 2, system signal
	E_ALLOC_m13,		// 3, allocation error
	E_FGEN_m13,		// 4, general file error
	E_FEXIST_m13,		// 5, file exists error
	E_FOPEN_m13,		// 6, file open error
	E_FREAD_m13,		// 7, file read error
	E_FWRITE_m13,		// 8, file write error
	E_FLOCK_m13,		// 9, file lock error
	E_FMED_m13,		// 10, not med file error
	E_FACC_m13,		// 11, file access / password error
	E_CRYP_m13,		// 12, cryptographic error
	E_META_m13,		// 13, metadata error
	E_REC_m13,		// 14, records error
	E_NET_m13,		// 15, network error
	E_CMP_m13,		// 16, compression / computational error
	E_PROC_m13,		// 17, process control error
	E_FILT_m13,		// 18, filter error
	E_DB_m13,		// 19, database error
	E_PRTY_m13		// 20, parity error
} E_MED_CODES_m13;

// error string table
#define	E_MAX_MSG_LEN_m13		((PATH_BYTES_m13 << 1) + 128)  // enough for two paths plus some text
#define	E_TBL_TAG_LEN_m13		8 // Note: tag strings do not conatins the "E_" of the enums, this is added in the printf()
#define	E_TBL_MSG_LEN_m13		56
#define E_STR_TABLE_ENTRIES_m13		E_NUM_CODES_m13
#define E_STR_TABLE_m13 { \
	{ "NONE", "no errors" }, \
	{ "GEN", "general error" }, \
	{ "SIG", "system signal" }, \
	{ "ALLOC", "memory allocation error" }, \
	{ "FGEN", "general file error" }, \
	{ "FEXIST", "file or directory not found" }, \
	{ "FOPEN", "file open error" }, \
	{ "FREAD", "file read error" }, \
	{ "FWRITE", "file write error" }, \
	{ "FLOCK", "file lock error" }, \
	{ "FMED", "not MED file or directory" }, \
	{ "FACC", "access denied" }, \
	{ "CRYPT", "cryptographic error" }, \
	{ "META", "metadata error / not found" }, \
	{ "REC", "record error" }, \
	{ "NET", "network error" }, \
	{ "CMP", "compression / computation error" }, \
	{ "PROC", "process error" }, \
	{ "FILT", "filter error" }, \
	{ "DB", "database error" }, \
	{ "PRTY", "parity error" } \
}

typedef struct {
	const si1	tag[E_TBL_TAG_LEN_m13];
	const si1	msg[E_TBL_MSG_LEN_m13];
} E_STRING_m13;

#define E_CHAIN_THREADS_m13	16 // thread-ancestry snapshot depth (matches FT_SNAP_STACKS_m13)

typedef struct {
	pid_t_m13	_id;
	si1		name[THREAD_NAME_BYTES_m13];
} E_CHAIN_ENTRY_m13;

typedef struct {
	_Atomic si4		code;
	si4			signal;
	si4			line;
	const si1		*function;
	si1			message[E_MAX_MSG_LEN_m13];
	si1			thread_name[THREAD_NAME_BYTES_m13];
	_Atomic pid_t_m13	thread_id;
	si4			n_chain_threads; // thread-ancestry snapshot (causal thread first), captured at error set:
	E_CHAIN_ENTRY_m13	thread_chain[E_CHAIN_THREADS_m13]; // the error carries everything needed to describe its origin, so thread-list entries need not outlive their threads (symmetric with the function stack snapshot)
	pthread_mutex_t_m13	mutex; // prevent access while being modified (does not duplicate role of isem)
	isem_t_m13		isem; // inverse semaphore
} ERROR_m13;

#define G_set_error_m13(code, message, ...)	G_set_error_exec_m13(__FUNCTION__, __LINE__, code, message, ##__VA_ARGS__) // vararg(code == E_SIG_m13): si4 sig_num (followed by optional formatting string values)

// Debugging printf()
// one definition for both builds: fprintf_m13() routes stderr through the mex output gate under MATLAB,
// & the TC_* colour macros are empty there, so this expands to the same text either way
#define eprintf_m13(fmt, ...)		do { fprintf_m13(stderr, "%s%s%s(%d)%s " fmt "\n", TC_RED_m13, __FUNCTION__, TC_BLUE_m13, __LINE__, TC_RESET_m13, ##__VA_ARGS__); fflush(stderr); } while(0)



//**********************************************************************************//
// MARK: MED Macros
//**********************************************************************************//

#define METADATA_CODE_m13(x)			( (((x) == TS_METADATA_TYPE_CODE_m13) || ((x) == VID_METADATA_TYPE_CODE_m13)) ? TRUE_m13 : FALSE_m13 )
#define INDICES_CODE_m13(x)			( (((x) == TS_INDS_TYPE_CODE_m13) || ((x) == VID_INDS_TYPE_CODE_m13) || ((x) == REC_INDS_TYPE_CODE_m13)) ? TRUE_m13 : FALSE_m13 )
#define DATA_CODE_m13(x)			( (((x) == TS_DATA_TYPE_CODE_m13) || ((x) == VID_DATA_TYPE_CODE_m13) || ((x) == REC_DATA_TYPE_CODE_m13)) ? TRUE_m13 : FALSE_m13 )
#define RECORD_CODE_m13(x)			( (((x) == REC_INDS_TYPE_CODE_m13) || ((x) == REC_DATA_TYPE_CODE_m13)) ? TRUE_m13 : FALSE_m13 )
#define CHANNEL_CODE_m13(x)			( (((x) == TS_CHAN_TYPE_CODE_m13) || ((x) == VID_CHAN_TYPE_CODE_m13)) ? TRUE_m13 : FALSE_m13 )
#define SEGMENT_CODE_m13(x)			( (((x) == TS_SEG_TYPE_CODE_m13) || ((x) == VID_SEG_TYPE_CODE_m13)) ? TRUE_m13 : FALSE_m13 )
#define PLURAL_m13(x) 				( ((x) == 1) ? "" : "s" )
#define ABS_m13(x)				( ((x) >= 0) ? (x) : -(x) )  // do not increment/decrement in call to ABS (as x occurs thrice)
#define HEX_STR_BYTES_m13(x, y) 		( ((x) << 1) + (((x) - 1) * (y)) + 1 ) // x numerical bytes with y-byte seperators plus terminal zero
#define BIN_STR_BYTES_m13(x, y) 		( ((x) << 3) + (((x) - 1) * (y)) + 1 )  // x numerical bytes with y-byte seperators plus terminal zero
#define REMOVE_DISCONT_m13(x)			( ((x) >= 0) ? (x) : -(x) )  // do not increment/decrement in call to REMOVE_DISCONTINUITY (as x occurs thrice)
#define APPLY_DISCONT_m13(x)			( ((x) <= 0) ? (x) : -(x) )  // do not increment/decrement in call to APPLY_DISCONTINUITY (as x occurs thrice)
#define MAX_OPEN_FILES_m13(n_chans, n_segs)	( (5 * n_chans * n_segs) + (2 * n_segs) + (2 * n_chans) + 5 ) // Note: final +5 == 2 for session level records plus 3 for standard streams (stdin, stdout, & stderr)
// "S" versions are for slice structures (not pointers)
#define MED_VER_1_0_m13(x)			( (x->MED_version_major == 1 && x->MED_version_minor == 0) ? TRUE_m13 : FALSE_m13 )
#define MED_VER_1_0_S_m13(x)			( (x.MED_version_major == 1 && x.MED_version_minor == 0) ? TRUE_m13 : FALSE_m13 )
#define SLICE_IDX_COUNT_m13(x)			( ((x)->end_idx - (x)->start_idx) + 1 )
#define SLICE_IDX_COUNT_S_m13(x)		( ((x).end_idx - (x).start_idx) + 1 )
#define SLICE_SEG_COUNT_m13(x)			( ((x)->end_seg_num - (x)->start_seg_num) + 1 )
#define SLICE_SEG_COUNT_S_m13(x)		( ((x).end_seg_num - (x).start_seg_num) + 1 )
#define SLICE_DUR_m13(x)			( ((x)->end_time - (x)->start_time) + 1 ) // time in usecs
#define SLICE_DUR_S_m13(x)			( ((x).end_time - (x).start_time) + 1 ) // time in usecs



//**********************************************************************************//
// MARK: Hardware (HW)
//**********************************************************************************//

// Structures
typedef struct {
	si8	integer_multiplications_per_sec; // test mimics RED/PRED in operand length, other tests may yield somewhat different results
	si8	integer_divisions_per_sec; // test mimics RED/PRED in operand length, other tests may yield somewhat different results
	sf8	nsecs_per_integer_multiplication; // test mimics RED/PRED in operand length, other tests may yield somewhat different results
	sf8	nsecs_per_integer_division; // test mimics RED/PRED in operand length, other tests may yield somewhat different results
} HW_PERFORMANCE_SPECS_m13;

#define HW_MAX_LOGICAL_CORES_m13	((si4) 256) // sizes the per-logical topology maps below

typedef struct {
	ui1				endianness;
	si4				physical_cores;
	si4				logical_cores; // written once at G_init_medlib_m13() (main thread, before any worker threads exist) => plain reads are safe
	si4				performance_cores; // heterogeneous CPUs (e.g. Apple silicon): high performance ("P") cores; == physical_cores in uniform CPUs
	si4				efficiency_cores; // heterogeneous CPUs: low power ("E") cores; == 0 in uniform CPUs
	si1				core_types[HW_MAX_LOGICAL_CORES_m13 + 1]; // per-LOGICAL type map, indexed by logical cpu number: 'P' or 'E' (all 'P' on uniform parts). Zero-terminated => prints directly ("PPPPPPPPPPPPPPPPEEEEEEEEEEEEEEEE" on an i9-13900K). Empty string == unknown (topology source unavailable)
	si4				physical_core_ids[HW_MAX_LOGICAL_CORES_m13]; // DENSE physical-core index of each logical cpu (SMT siblings share an index; assignment order = first-logical order); -1 == unknown. THE topology map for affinity work (PROC_encode_cpu_set_m13() consumes it)
	tern				hyperthreading;
	tern				AES_accel; // hardware AES instructions present (x86 AES-NI / ARMv8 AES); UNKNOWN_m13 until detected
	tern				SHA256_accel; // hardware SHA-256 instructions present (x86 SHA extensions / ARMv8 SHA-2); UNKNOWN_m13 until detected
	sf8				minimum_speed;
	sf8				maximum_speed;
	sf8				current_speed;
	si8				current_speed_uutc;
	HW_PERFORMANCE_SPECS_m13	performance_specs;
	ui8				system_memory_size; // system physical RAM (in bytes)
	ui4				system_page_size; // memory page (in bytes)
	ui4				map_granularity; // mapping PLACEMENT granularity (bytes): == page size on Unix; Windows
							 // dwAllocationGranularity (64 KB - MapViewOfFileEx placement & mapping
							 // offsets align to THIS, not the page). Page-aligned ALLOCATION (guarded
							 // tables, mprotect bounds) uses system_page_size - do not conflate.
	ui8				heap_base_address;
	ui8				heap_max_address;
	si1				cpu_manufacturer[64];
	si1				cpu_model[64];
	si1				machine_serial[56]; // maximum serial number length is 50 characters
	ui4				machine_code; // code based on serial number (written once at G_init_medlib_m13(), before any worker threads exist)
} HW_PARAMS_m13;

// Prototypes
ui4	HW_get_block_size_m13(const si1 *volume_path);
tern	HW_get_core_info_m13(void);
tern	HW_get_crypto_accel_m13(void); // AES_accel & SHA256_accel (encryption/hash routines read these to select hardware vs table paths)
tern	HW_get_endianness_m13(void);
tern	HW_get_info_m13(void); // fill whole HW_PARAMS_m13 structure

// D31: live CPU monitor - kernel-direct per-core loads & temperatures (Linux /proc/stat + hwmon
// sysfs; macOS Mach host ticks, temps NAN; Windows not yet). No forked utilities: a fork+exec
// perturbs the cores being measured & drags in sysstat/lm-sensors dependencies. Loads are BUSY
// fractions (non-idle, INCLUDING kernel/softirq time) as deltas between successive reads on the
// same monitor. Open one monitor per consumer (GUI thread, throttle controller, ...).
typedef struct {
	si4	n_logical; // loads[] length (logical cores)
	si4	n_physical; // temps[] length (physical cores)
	sf8	*loads; // [n_logical] busy fraction [0,1] since the previous read
	sf8	load; // machine mean busy fraction
	sf8	*temps; // [n_physical] degrees Celsius; NAN == no sensor
	sf8	temp; // package temperature (or mean of core temps); NAN == unavailable
	sf8	temp_max; // TjMax: the chip's own critical temperature (hwmon temp*_crit, fallback temp*_max), degrees C;
			  // NAN == unknown. Thermal policy should be expressed as a FRACTION of this, not against a
			  // hard-coded band - TjMax is 100 on many desktop parts, 105 on mobile, ~85 on some server parts
	// internals
	ui8	*prev_busy;
	ui8	*prev_total;
	void	*temp_paths; // platform temperature source state
} HW_CPU_MONITOR_m13;

HW_CPU_MONITOR_m13	*HW_cpu_monitor_open_m13(void);
tern	HW_cpu_monitor_read_m13(HW_CPU_MONITOR_m13 *mon);
void	HW_cpu_monitor_close_m13(HW_CPU_MONITOR_m13 **mon);
tern	HW_memory_usage_m13(sf8 *system_used_fraction, ui8 *process_rss_bytes); // stateless; either arg may be NULL; system fraction uses MemAvailable semantics, process figure is RSS
tern	HW_get_machine_code_m13(void);
tern	HW_get_machine_serial_m13(void);
tern	HW_get_performance_specs_m13(tern get_current);
si1	*HW_get_performance_specs_file_m13(si1 *file);
tern	HW_get_performance_specs_from_file_m13(void);
tern	HW_get_memory_info_m13(void);
tern	HW_init_tables_m13(void);
tern	HW_show_info_m13(void);



//**********************************************************************************//
// MARK: General MED
//**********************************************************************************//

// Daylight Change code
typedef union {
	struct {
		si1	code_type; // (DST end / DST Not Observed / DST start) == (-1 / 0 / +1)
		si1	day_of_week; // (No Entry / [Sunday : Saturday]) == (-1 / [0 : 6])
		si1	relative_weekday_of_month; // (No Entry / [First : Fifth] / Last) == (0 / [1 : 5] / 6)
		si1	day_of_month; // (No Entry / [1 : 31]) == (0 / [1 : 31])
		si1	month; // (No Entry / [January : December]) == (-1 / [0 : 11])
		si1	hours_of_day; // [-128 : +127] hours relative to 0:00 (midnight)
		si1	reference_time; // (Local / UTC) == (0 / +1)
		si1	shift_minutes; // [-120 : +120] minutes
	};
	si8 value; // 0 indicates DST is not observed, -1 indicates no entry
} DAYLIGHT_TIME_CHANGE_CODE_m13;

typedef struct {
	si1	country[METADATA_RECORDING_LOCATION_BYTES_m13];
	si1	country_acronym_2_letter[3]; // two-letter acronym; (ISO 3166 ALPHA-2)
	si1	country_acronym_3_letter[4]; // three-letter acronym (ISO-3166 ALPHA-3)
	si1	territory[METADATA_RECORDING_LOCATION_BYTES_m13];
	si1	territory_acronym[TIMEZONE_STRING_BYTES_m13];
	si1	standard_timezone[TIMEZONE_STRING_BYTES_m13];
	si1	standard_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m13];
	si4	standard_UTC_offset; // seconds
	si1	daylight_timezone[TIMEZONE_STRING_BYTES_m13];
	si1	daylight_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m13];
	si8	daylight_time_start_code; // DAYLIGHT_TIME_CHANGE_CODE_m13 - cast to use other fields
	si8	daylight_time_end_code; // DAYLIGHT_TIME_CHANGE_CODE_m13 - cast to use other fields
	si8	daylight_codes_start_date; // onset of rules encoded in daylight codes (in uutc). DTCC_START_DATE_NO_ENTRY (-1) indicates it is the only historical rule for this timezone in the table.
} TIMEZONE_INFO_m13;

typedef struct {
	si1	table_name[METADATA_RECORDING_LOCATION_BYTES_m13];
	si1	alias[METADATA_RECORDING_LOCATION_BYTES_m13];
} TIMEZONE_ALIAS_m13;

typedef struct {
	tern	conditioned;
	si4	n_segs; // == (si4) UNKNOWN_m13 if segment range is unknown, otherwise == number of segments in slice
	si8 	start_time;
	si8 	end_time;
	union { // session-relative (global indexing)
		si8	start_samp_num;
		si8	start_frame_num;
		si8	start_idx; // generic
	};
	union { // session-relative (global indexing)
		si8	end_samp_num;
		si8	end_frame_num;
		si8	end_idx; // generic
	};
	si4 	start_seg_num;
	si4 	end_seg_num;
} SLICE_m13;

typedef struct {
	si8 	start_time;
	si8 	end_time;
	union { // session-relative (global indexing)
		si8	start_samp_num;
		si8	start_frame_num;
		si8	start_idx; // generic
	};
	union { // session-relative (global indexing)
		si8	end_samp_num;
		si8	end_frame_num;
		si8	end_idx; // generic
	};
	si4 	start_seg_num;
	si4 	end_seg_num;
} CONTIGUON_m13;

typedef struct { // times in uutc
	si8	creation;
	si8	access;
	si8	modification;
} FILE_TIMES_m13;

typedef struct { // fields from ipinfo.io
	TIMEZONE_INFO_m13	timezone_info;
	si1			WAN_IPv4_address[IPV4_ADDRESS_BYTES_m13 * 4];
	si1			locality[LOCALITY_BYTES_m13];
	si1			postal_code[POSTAL_CODE_BYTES_m13];
	si1			timezone_description[METADATA_RECORDING_LOCATION_BYTES_m13];
	sf8			latitude;
	sf8			longitude;
} LOCATION_INFO_m13;

typedef struct {
	ui4			source_type; // type code of source channel, or NO_TYPE_CODE_m13 (session level)
	union {
		sf4		sampling_frequency;
		sf4		frame_rate;
		sf4		rate;
	};
	union {
		si8		n_session_samples;
		si8		n_session_frames;
	};
	struct Sgmt_REC_m13	*Sgmt_recs; // defined below == record header + REC_Sgmt_v11_m13 body (session number of segments in length)
} Sgmt_RECS_ENTRY_m13;

typedef struct { // multiple thread access
	pthread_mutex_t_m13			mutex;
	_Atomic(Sgmt_RECS_ENTRY_m13 *)		entries;
	_Atomic si4				size; // total allocated Sgmt_RECORD_ENTRYs
	_Atomic si4				top_idx; // last non-empty Sgmt_RECORD_ENTRY in list
} Sgmt_RECS_LIST_m13;

typedef struct {
	ui8			UID;
	si1			path[PATH_BYTES_m13]; // path including file system session directory name
	si1			fs_name[NAME_BYTES_m13]; // name from file system
	si1			uh_name[NAME_BYTES_m13]; // name from universal_header (if differs from file system, otherwise empty)
	si8			start_time;
	si8			end_time;
	si4			n_segments; // number of segments in the session; ALL are always mapped (per-channel segs arrays are full-length & stable) - the map-subset window (n_mapped/first_mapped) was removed
	Sgmt_RECS_LIST_m13	*Sgmt_recs_list; // list with one entry for each unique sampling frequency and channel type
	si1			index_channel_name[NAME_BYTES_m13]; // contains user specified value if needed, open_session_m13() matches to session channel
	struct CHAN_m13		*index_channel;
} CURRENT_SESSION_m13; // PROC_GLOBS_m13 element

typedef struct {
	sf8			minimum_sampling_frequency;
	sf8			maximum_sampling_frequency;
	sf8			minimum_frame_rate;
	sf8			maximum_frame_rate;
	tern 			sampling_frequencies_vary;
	tern 			frame_rates_vary;
	struct CHAN_m13		*minimum_sampling_frequency_channel;
	struct CHAN_m13		*maximum_sampling_frequency_channel;
	struct CHAN_m13		*minimum_frame_rate_channel;
	struct CHAN_m13		*maximum_frame_rate_channel;
} ACTIVE_CHANNELS_m13; // PROC_GLOBS_m13 element

typedef struct {
	// 3-state lifecycle of the shared, process-global time constants (set once from metadata section 3, then read by every thread):
	//   UNKNOWN_m13 (0)  = not yet attempted           -> a reader should trigger the (one-time) population
	//   FALSE_m13  (-1)  = attempted, but CANNOT be set (section 3 not accessible, e.g. opened with only an L1 password)
	//                      -> MED still functions; times are shown obfuscated/relative (see STR_time_m13()); do NOT retry
	//   TRUE_m13   (1)   = set & valid (populated from section 3, or built directly for a new file)
	// This single field replaces the old, drift-prone (set, RTO_known) pair: whenever the constants are set, the recording
	// time offset is set with them, so a real recording_time_offset is available iff set == TRUE_m13.  (The one historical
	// exception - a machine-location fallback that set the timezone but not a file RTO - was CURRENT_TIME display only.)
	_Atomic tern	set;		// atomic so lock-free readers see UNKNOWN or a final value, never a torn write
	_Atomic tern	populating;	// election claim only: readers CAS FALSE->TRUE to become the single populating thread; losers wait for set != UNKNOWN (see G_decrypt_metadata_m13())
	tern 	observe_DST;
	si8	recording_time_offset;
	si4	standard_UTC_offset;
	si1	standard_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m13];
	si1	standard_timezone_string[TIMEZONE_STRING_BYTES_m13];
	si1	daylight_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m13];
	si1	daylight_timezone_string[TIMEZONE_STRING_BYTES_m13];
	DAYLIGHT_TIME_CHANGE_CODE_m13	daylight_start_code; // si1[8] / si8
	DAYLIGHT_TIME_CHANGE_CODE_m13	daylight_end_code; // si1[8] / si8
	// resolved recording location: G_set_time_constants_m13() matches a full timezone-table row, so the
	// country & territory are known once the timezone is. Retained here so G_init_metadata_m13() can seed
	// metadata section 3 with them (previously they were zeroed & the knowledge was discarded). An app's
	// own value (e.g. from an rc file) still overrides - the rc file is never modified by the library.
	si1	country[METADATA_RECORDING_LOCATION_BYTES_m13];
	si1	territory[METADATA_RECORDING_LOCATION_BYTES_m13];
} TIME_CONSTANTS_m13; // PROC_GLOBS_m13 element

typedef struct {
	_Atomic ui4	mmap_block_bytes; // read size for memory mapped files (process data may be on different volumes)
					  // if files are on different volumes, set file-specific mmap_block_bytes in FILE_m13 structure
					  // atomic: lazily cached by FPS_mmap_alloc_m13()/HW_get_block_size_m13() on concurrent segment-read
					  // threads (check-then-set); same value per volume, but the store/load must not race
	// med_rand_w / med_rand_z (seedable PRNG state) moved to true thread-local storage in medlib_m13.c: a shared
	// proc_globs copy raced across threads (proc_globs is shared per session). Each thread now has its own reproducible stream.
	_Atomic tern	threading; // TRUE_m13 == thread processing where appropriate (atomic b/c other threads may access)
	tern 		memory_mapping; // set ONCE at G_proc_globs_init_m13() from the global value, then read-only (never mutated while worker threads run) => no atomic needed
	_Atomic tern	proc_error; // thread-local mechanism for void functions (no return value) to indicate that an error occurred (global error may be set by other threads; atomic b/c other threads may access)
} PROC_GLOB_MISC_m13; // PROC_GLOBS_m13 element

// All MED File Structures begin with a level header structure
typedef struct LH_m13 { // multiple thread access
	union {
		ui4 		type_code;
		struct {
			si1 	type_string[TYPE_BYTES_m13];
			tern	allocated; // allocated on heap, independently (not en bloc)
			tern	names_differ; // file system & universal header name mismatch
		};
	};
	si1					*path; // NULL in proc_globs
	si1					*name; // NULL in proc_globs
	_Atomic(struct LH_m13 *)		parent; // NULL in proc_globs
	_Atomic(struct PROC_GLOBS_m13 *)	proc_globs; // self in proc_globs
	ui8					flags;
	_Atomic si8				access_time; // uutc of last use of this structure by the calling program (updated by open, read, & write functions)
} LH_m13;

// non-standard structure
// required compiler option (gcc, clang): -fms-extensions
// suppress warnings: -Wno-microsoft-anon-tag
#ifdef __cplusplus
typedef struct PROC_GLOBS_m13 { // multiple thread access
	union {
		LH_m13 	header; // in case just want the level header
		struct { // this struct replaces anonymous LH_m13 in C++
			union {
				ui4 		type_code;
				struct {
					si1 	type_string[TYPE_BYTES_m13];
					tern	allocated; // allocated on heap, independently (not en bloc)
					tern	names_differ;
				};
			};
			si1					*path; // NULL in proc_globs
			si1					*name; // NULL in proc_globs
			_Atomic(LH_m13 *)			parent; // NULL in proc_globs
			_Atomic(struct PROC_GLOBS_m13 *)	proc_globs; // self in proc_globs
			ui8					flags;
			_Atomic si8				access_time; // uutc of last use of this structure by the calling program (updated by open, read, & write functions)
		};
	};
 // Password
	PASSWORD_DATA_m13	password_data;
	// establish-once election for password_data: guards the concurrent lazy-init in FPS_read_m13() when
	// G_read_channel_m13()/G_read_segment_m13() are entered directly from multiple threads (no single-threaded
	// G_read_session_m13() phase to establish it first).  These MUST live outside PASSWORD_DATA_m13 - G_process_
	// password_data_m13() memsets that whole struct, which would clobber flags kept inside it.  Same pattern as
	// time_constants.set/populating (atomic => re-init-safe on pg reuse, no mutex lifecycle).
	_Atomic tern		password_established;	// 3-state: UNKNOWN = not yet established; FALSE = established, no decrypt access (this read fails, as before); TRUE = established w/ access. Losers wait for != UNKNOWN, then replicate the winner's pass/fail => never see a half-built pwd
	_Atomic tern		password_electing;	// election claim: CAS FALSE->TRUE picks the single populating thread
 // Current Session
	CURRENT_SESSION_m13	current_session;
 // Active Channels
	ACTIVE_CHANNELS_m13	active_channels;
 // Time Constants
	TIME_CONSTANTS_m13	time_constants;
 // Miscellaneous
	PROC_GLOB_MISC_m13	miscellaneous;
	pid_t_m13		_id; // thread or process id (used if LH_m13 unknown [NULL])
	LH_m13			*child; // hierarchy level immediately below these process globals
	_Atomic si4		ref_count; // (atomic: modified by concurrent threads outside list mutex)
} PROC_GLOBS_m13;
#else // __cplusplus
typedef struct PROC_GLOBS_m13 {
	union {
		LH_m13	header; // in case just want the level header (parent NULL in PROC_GLOBS_m13)
		LH_m13; // anonymous LH_m13
	};
 // Password
	PASSWORD_DATA_m13	password_data;
	// establish-once election for password_data: guards the concurrent lazy-init in FPS_read_m13() when
	// G_read_channel_m13()/G_read_segment_m13() are entered directly from multiple threads (no single-threaded
	// G_read_session_m13() phase to establish it first).  These MUST live outside PASSWORD_DATA_m13 - G_process_
	// password_data_m13() memsets that whole struct, which would clobber flags kept inside it.  Same pattern as
	// time_constants.set/populating (atomic => re-init-safe on pg reuse, no mutex lifecycle).
	_Atomic tern		password_established;	// 3-state: UNKNOWN = not yet established; FALSE = established, no decrypt access (this read fails, as before); TRUE = established w/ access. Losers wait for != UNKNOWN, then replicate the winner's pass/fail => never see a half-built pwd
	_Atomic tern		password_electing;	// election claim: CAS FALSE->TRUE picks the single populating thread
 // Current Session
	CURRENT_SESSION_m13	current_session;
 // Active Channels
	ACTIVE_CHANNELS_m13	active_channels;
 // Time Constants
	TIME_CONSTANTS_m13	time_constants;
 // Miscellaneous
	PROC_GLOB_MISC_m13	miscellaneous;
	pid_t_m13		_id; // thread id (used if LH_m13 unknown [NULL])
	LH_m13			*child; // hierarchy level immediately below these process globals
	_Atomic si4		ref_count; // count of structures & threads currently linked to these process globals (atomic: modified by concurrent threads outside list mutex)
} PROC_GLOBS_m13;
#endif // standard c

// NOTE: placement of LH_m13 in structures allows passing of LH_m13 pointer to functions,
// and based on its content, functions can cast pointer to specific level structures.
// e.g:
// if (lh->type_code == LH_SESS_m13)
//	sess = (SESS_m13 *) lh;

typedef struct {
	pthread_mutex_t_m13			mutex;
	const TIMEZONE_INFO_m13			*timezone_table;
	const TIMEZONE_ALIAS_m13		*country_aliases_table;
	const TIMEZONE_ALIAS_m13		*country_acronym_aliases_table;
	const ui4				**CRC_tables;
	const ui1				*AES_sbox_table;
	const ui1				*AES_rsbox_table;
	const ui1				*AES_rcon_table;
	const ui4				*SHA_h0_table;
	const ui4				*SHA_k_table;
	const sf8				*CMP_normal_CDF_table;
	const CMP_VDS_THRESHOLD_MAP_ENTRY_m13	*CMP_VDS_threshold_map;
	const sf8				*CMP_log_table;  // COMPUTED entropy-estimator LUT (log2(n)); heap, freed like CRC_tables
	NET_PARAMS_m13				NET_params; // parameters for default internet interface
	HW_PARAMS_m13				HW_params;
	const E_STRING_m13			*E_strings_table;
	#ifdef WINDOWS_m13
	HINSTANCE				hNTdll; // handle to ntdll dylib (used by WN_nap_m13(); only loaded if used)
	#endif
} GLOBAL_TABLES_m13;

typedef struct {
	const si1	*function; // function in which behavior was set
	si4		line; // line at which behavior was set
	ui4		code; // behavior code
} BEHAVIOR_m13;

typedef struct { // single thread access
	pid_t_m13		_id; // thread id
	BEHAVIOR_m13		*behaviors; // current behavior at top of stack (last entry)
	si4			size; // total allocated behaviors
	si4			top_idx; // top of behavior stack
} BEHAVIOR_STACK_m13;

// Note: behavior stacks are thread-local (storage belongs to the thread; reclaimed at thread exit by a TSD destructor).
// No global registry exists: nothing reads another thread's behavior stack, & thread-owned storage makes behaviors
// available in every thread at every lifecycle moment (including globals initialization, freeing, & re-initialization).
// (Function stacks retain their global registry - error snapshots read ancestor threads' stacks.)


// Exec Function Defines
#define G_add_behavior_m13(code)		G_add_behavior_exec_m13(__FUNCTION__, __LINE__, code) // call with "G_add_behavior_m13(ui4 behavior)" prototype
#define G_pop_behavior_m13()			G_pop_behavior_exec_m13(__FUNCTION__, __LINE__) // call with "G_pop_behavior_m13(ui4 behavior)" prototype
#define G_push_behavior_m13(code)		G_push_behavior_exec_m13(__FUNCTION__, __LINE__, code) // call with "G_push_behavior_m13(ui4 behavior)" prototype
#define G_remove_behavior_m13(code)		G_remove_behavior_exec_m13(__FUNCTION__, __LINE__, code) // call with "G_remove_behavior_m13(ui4 behavior)" prototype
#define G_behavior_stack_reset_m13(code)	G_behavior_stack_reset_exec_m13(__FUNCTION__, __LINE__, code) // call with "G_behavior_stack_reset_m13(ui4 behavior)" prototype
#define G_push_function_m13() 			G_push_function_exec_m13(__FUNCTION__, __LINE__) // call with "G_push_function_m13(void)" prototype
#define G_pop_function_m13()			G_pop_function_exec_m13(__FUNCTION__, __LINE__) // call with "G_pop_function_m13(void)" prototype


typedef struct {
	const si1	*name;
	si4		entry_line;
	si4		return_line;
} FUNCTION_ENTRY_m13;

typedef struct { // single thread access
	pid_t_m13		_id; // thread id
	FUNCTION_ENTRY_m13	*functions;
	si4			size; // total allocated functions
	si4			top_idx; // top of function stack
	// Note: error display state lives in the error snapshot (see FT_SNAPSHOT_m13 below) -
	// live stacks are never frozen by errors (error may be handled & cleared, execution continues)
} FUNCTION_STACK_m13;

typedef struct { // multiple thread access
	pthread_mutex_t_m13		mutex;
	_Atomic(FUNCTION_STACK_m13 **)	stack_ptrs; // secondary indirection to function stacks
	_Atomic si4			size; // total allocated function_stacks
	_Atomic si4			top_idx; // last non-empty function_stack in list
} FUNCTION_STACK_LIST_m13;

#ifdef FT_DEBUG_m13
// error function stack snapshot: G_set_error_exec_m13() copies the function stacks of the causal thread & its
// ancestors here for later display (G_show_function_stack_m13() / exit_exec_m13()); subsequent pops record the
// error's propagation (return lines) into the snapshot; live stacks are never modified, so function tracking
// continues normally when an error is handled & cleared (e.g. PAR job errors captured into their handles)
#define FT_SNAP_STACKS_m13	16 // maximum thread chain depth
#define FT_SNAP_FRAMES_m13	256 // maximum frames per stack

typedef struct {
	pid_t_m13		_id; // thread id at snapshot
	si4			n_frames;
	si4			unwind_idx; // error propagation recording position (top frame first); -1 == fully unwound
	FUNCTION_ENTRY_m13	frames[FT_SNAP_FRAMES_m13];
} FT_SNAP_STACK_m13;

typedef struct {
	_Atomic tern		active;
	si4			n_stacks;
	FT_SNAP_STACK_m13	stacks[FT_SNAP_STACKS_m13]; // causal thread first, ancestors follow
} FT_SNAPSHOT_m13;
#endif  // FT_DEBUG_m13

typedef struct { // multiple thread access
	pthread_mutex_t_m13		mutex;
	_Atomic(PROC_GLOBS_m13 **)	proc_globs_ptrs; // secondary indirection to process globals
	_Atomic si4			size; // total allocated proc_globs
	_Atomic si4			top_idx; // last non-empty function_stack in list
} PROC_GLOBS_LIST_m13;

typedef struct FLOCK_ENTRY_m13 { // multiple thread access (tagged: forward referenced by FILE_m13)
	_Atomic ui8		file_id; // from FILE_id_m13()
	isem_t_m13		read_cnt; // ownership or number of processes currently reading file
	_Atomic ui4		open_cnt; // number of processes that have file open
	_Atomic ui4		write_depth; // reentrant write lock depth (auto-locking writes within explicit write-locked spans; modified only by owning thread)
	pthread_mutex_t_m13	os_mutex; // serializes os lock state transitions (multiprocess locking)
	si4			os_fd; // dedicated descriptor holding the os lock (duplicated => user closes cannot drop it); -1 == none
	_Atomic si1		os_lock_state; // this process's os lock: FLOCK_OS_NONE/SHARED/EXCLUSIVE_m13
	ui1			pad_bytes[3]; // future use (bytes will be there anyway)
} FLOCK_ENTRY_m13;

typedef struct { // multiple thread access
	pthread_mutex_t_m13		mutex;
	_Atomic(FLOCK_ENTRY_m13	**)	lock_ptrs; // secondary indirection to FLOCK_ENTRY_m13 *
	_Atomic si4			size; // total allocated locks
	_Atomic si4			top_idx; // last non-empty lock in list
} FLOCK_LIST_m13;

#define FLOCK_HELD_SLOTS_m13	16

typedef struct { // single thread access (thread-local read-lock bookkeeping)
	FLOCK_ENTRY_m13	*entry; // NULL == empty slot
	ui8		file_id; // validates entry (slot is stale if entry was reused for another file)
	si4		count; // read locks held on entry by this thread
} FLOCK_HELD_READS_m13;

typedef struct { // multiple thread access
	_Atomic(void *)		address;
	_Atomic ui8		requested_bytes;
	_Atomic ui8		actual_bytes; // actual bytes allocated => may be more than were requested
	const si1		*alloc_function;
	const si1		*free_function;
	si4			alloc_line;
	si4			free_line;
	pid_t_m13		alloc_thread_id;
	si1			alloc_thread_name[THREAD_NAME_BYTES_m13];
	pid_t_m13		free_thread_id;
	si1			free_thread_name[THREAD_NAME_BYTES_m13];
} AT_ENTRY_m13;

typedef struct { // multiple thread access
	pthread_mutex_t_m13		mutex;
	_Atomic(AT_ENTRY_m13 *)		entries; // no secondary indirection
	_Atomic si8			size; // total allocated entries
	_Atomic si4			top_idx; // last non-empty entry in list
} AT_LIST_m13; // global

typedef struct { // multiple thread access
	_Atomic pid_t_m13	_id; // thread id
	_Atomic pid_t_m13	_pid; // parent thread id
	pthread_t_m13 		thread; // pthread_t_m13
	si1			name[THREAD_NAME_BYTES_m13];
} THREAD_ENTRY_m13;

typedef struct { // multiple thread access
	pthread_mutex_t_m13		mutex;
	_Atomic(THREAD_ENTRY_m13 *)	entries; // no secondary indirection
	_Atomic si8			size; // total allocated entries
	_Atomic si4			top_idx; // last non-empty entry in list
} THREAD_LIST_m13;

typedef struct {
	si1		path[PATH_BYTES_m13];
	si1		name[NAME_BYTES_m13];
	ui1		version_major;
	ui1		version_minor;
	FILE_TIMES_m13	file_times;
} APP_INFO_m13;

typedef struct {
	pthread_mutex_t_m13	mutex;
	_Atomic ui1		value;
} TEST_BYTE_m13; // global byte to check pointer for readability (global ensures volatility, mutex prevents concurrent usage)

typedef struct {
	si1				cwd[PATH_BYTES_m13]; // current working directory (periodically auto-cleared)
	si1				temp_dir[PATH_BYTES_m13]; // system temp directory (periodically auto-cleared)
	si1				temp_file[PATH_BYTES_m13]; // full path to temp file (i.e. incudes temp_dir)
									// not thread safe => use G_unique_temp_file_m13() in threaded applications
	ui4				file_creation_umask;
	pid_t_m13			main_id;  // process thread id (not necessarily same as process id)
	pthread_mutex_t_m13		update_mutex;
	tern				threading; // global default, used to set process globals default
	tern				memory_mapping; // global default, used to set process globals default
	si1				file_lock_mode; // enable global file locking
	tern				file_lock_multiprocess; // extend locks across concurrent library instances (os level locks; see Files section)
	const si1			*file_lock_timeout; // blocking timeout (as string)  [nap_m13() form]
	ui4				CRC_mode;
	tern				access_times; // record times of each structure & file access
	tern				write_sorted_records; // if records unsorted, re-write after sorting
	tern				write_corrected_headers; // if files closed without header update, n_entries may be zero (if TRUE_m13, write corrected header when encountered)
	tern				update_file_system_names; // if session or channel file system name differ from higher level names, rename lower level files & directories
	tern				update_header_names; // if session or channel file system name differs from universal header, update affected universal headers (requires update_file_system_names to be TRUE_m13)
	tern				update_MED_version; // if file MED version is not current, update affected files
	si1				update_L3_password[MAX_PASSWORD_STRING_BYTES_m13]; // level-3 password for the schema 0->1 re-key that a MED-version update performs on an ENCRYPTED session (MED 1.1 == schema 1); the app (via dhnlib for DHN files) sets this before opening with update_MED_version; recovers L1/L2 & re-keys all files; cleared after the update. Empty => unencrypted-only update (bail on an encrypted session)
	tern				update_parity; // update parity on write, if exists (e.g. updating header names or MED version; best turned off & manually batched on data conversion or acquisition)
	tern				write_CSigs; // write CSig (attestation) records at custody events (close, conversion, transfer, archive, repair, rekey; see DGST module & REC_write_CSig_m13())
	si8				CSig_reread_max_bytes; // dirty/unusable digest stream fallback policy: files at or below this size are re-read (DGST_file_m13()), larger files skip with a warning
	tern				session_key_cache; // cache derived session keys in the OS key store: repeated short-process session opens skip the KDF; a supplied password validates a hit via the stored fingerprint (see SESSION_KEY_CACHE_FP_BYTES_m13); no-password opens honor the timeout
	si4				session_key_cache_timeout; // cached session key lifetime, seconds (hard-capped at GLOBALS_SESSION_KEY_CACHE_TIMEOUT_MAX_m13 == 2 h)
	tern				increase_priority; // increase process priority if PROC_increase_priority_m13() is called
	tern				background_processing; // run distributed jobs (PROC_jobs_distribute_m13()/PAR_distribute_m13()) at low priority (default NO == full speed; overrides per-job priorities when YES)
	_Atomic si4			distribute_calls; // concurrently active job distributions (PROC & PAR): adaptive jobs_per_core trusts utilization measurements only when a distribution runs alone
	ui1				new_password_required_classes; // OPT-IN creation-time composition rule (bitmask of PW_CLASS_*_m13); 0 == none (default). Length policy is unconditional; this is extra. Reads never consult it.
	ui1				new_password_min_classes; // OPT-IN: require at least N of the 4 categories present (0 == off). The generally-deployed standard (Active Directory: 3 of 4). Independent of, & AND-ed with, new_password_required_classes.
	TEST_BYTE_m13			test_byte;
	_Atomic tern			suspend_stacks;
} GLOBAL_MISC_m13;

typedef struct CMP_BUFFER_DEPOT_m13 CMP_BUFFER_DEPOT_m13;  // forward decl (full def in the CMP section); GLOBALS holds only a pointer

typedef struct {
	pthread_mutex_t_m13		mutex;
// Application Info
	APP_INFO_m13			*app_info;
// Tables
	GLOBAL_TABLES_m13		*tables;
// Behavior Stacks (thread local)
// Function Stacks (thread local)
	FUNCTION_STACK_LIST_m13		*function_stack_list;
// Process Globals (thread local)
	PROC_GLOBS_LIST_m13		*proc_globs_list;
// File Locking (global)
	FLOCK_LIST_m13			*file_lock_list;
// Allocation Tracking (global)
	THREAD_LIST_m13			*thread_list;
// Allocation Tracking (global)
	AT_LIST_m13			*AT_list;
// CMP Buffer Depot (global checkout/return pool of locked, page-aligned scratch bundles)
	CMP_BUFFER_DEPOT_m13		*CMP_buffer_depot;
// Record Filters (global default)
	si4 				*record_filters; // signed, "NULL terminated" array version of MED record type codes to include or exclude when reading records.
						  // The terminal entry is NO_TYPE_CODE_m13 (== zero). NULL or no filter codes includes all records (== no filters).
						  // filter modes: match positive: include
						  //		 match negative: exclude
						  //		 no match:
						  //			all filters positive: exclude
						  //			else: include
						  // Note: as type codes are composed of ascii bytes values (< 0x80), it is always possible to make them negative without promotion.
// Behavior
	BEHAVIOR_m13			default_behavior;
// Error
	ERROR_m13			error; // causal error
// Miscellaneous
	GLOBAL_MISC_m13			miscellaneous;
} GLOBALS_m13;

// Universal Header Structure
typedef struct {
 // start robust mode region
	ui4		header_CRC; // CRC of the universal header after this field
	ui4		body_CRC; // CRC of the entire file after the universal header
	union { // "segment_end_time" used in code when that is it's meaning, for clarity
			si8	file_end_time;
			si8	segment_end_time;
	};
	si8		n_entries;
	ui4		maximum_entry_size;
 // end robust mode region
	si4		segment_number;
	union {
		ui4		type_code;
		struct {
			si1	type_string[TYPE_BYTES_m13];
			ui1	MED_version_major;
			ui1	MED_version_minor;
			ui1	byte_order_code;
		};
	};
	si8		session_start_time;
	union { // "segment_start_time" used in code when that is it's meaning, for clarity
			si8	file_start_time;
			si8	segment_start_time;
	};
	si1		session_name[NAME_BYTES_m13]; // utf8[63], base name only, no extension
	si1		channel_name[NAME_BYTES_m13]; // utf8[63], base name only, no extension
	ui1		supplementary_protected_region[UH_SUPPLEMENTARY_PROTECTED_REGION_BYTES_m13];
	ui1		anchor_escrow_field[ANCHOR_ESCROW_FIELD_BYTES_m13]; // top-level password string escrow (see anchor_type; final 48 bytes of the MED 1.0 supplementary protected region)
	ui8		session_UID; // session UID of originating data set
	ui8		channel_UID; // channel UID of originating data set
	ui8		segment_UID; // segment UID of originating data set
	ui8		file_UID; // unique to current file
	ui8		provenance_UID; // file UID of originating file
	ui1		level_1_password_validation_field[PASSWORD_VALIDATION_FIELD_BYTES_m13];
	ui1		level_2_password_validation_field[PASSWORD_VALIDATION_FIELD_BYTES_m13];
	ui1		level_3_password_validation_field[PASSWORD_VALIDATION_FIELD_BYTES_m13];
	ui4		video_data_file_number; // MED 1.1 and above
	tern		live; // file currently being generated, MED 1.1 and above
	tern		ordered; // MED 1.1 and above
	si1		metadata_section_2_encryption;		// metadata section 2 (see the encryption-byte block in the offset defines)
	si1		metadata_section_3_encryption;		// metadata section 3
	si1		time_series_data_encryption;		// time series data
	si1		video_data_encryption;			// video data
	si1		maximum_record_encryption_level;	// RECORDS (index & data) files ONLY: highest level among the file's records (0 => none, known); ENCRYPTION_LEVEL_NO_ENTRY_m13 (-128) == unknown in every non-records UH (records self-report - read a records file's header for this)
	si1		encryption_reserved;			// reserved for a future encryptable region
	ui1		crypto_schema; // password validation & key derivation schema (taken from protected region: zeros in older files == CRYPTO_SCHEMA_LEGACY_m13, which is read-only)
	ui1		kdf_exponent[UH_KDF_EXPONENT_LEVELS_m13]; // KDF iterations == 2^n, PER LEVEL: [0]=L1 [1]=L2 [2]=L3 [3]=reserved L4 (schema 1+; unused under legacy). Independent per level so a machine level (e.g. the DHN L3) can use a low exponent without a weak human L1/L2 dragging it up.
	ui1		anchor_type; // recovery anchor form (see ANCHOR_TYPE_*_m13; taken from protected region: zeros in older files == no anchor)
	ui1		anchor_key_ID; // which anchor public key sealed this session (anchor type 2; taken from protected region)
	ui1		level_1_escrow_field[ESCROW_FIELD_BYTES_m13]; // AES-256(escrow key from level 2 chain material, level 1 password bytes); taken from protected region
	ui1		reserved_level_4_escrow_field[ESCROW_FIELD_BYTES_m13]; // EXPLICITLY RESERVED for a future level 4: zero until then
	ui1		reserved_level_4_validation_field[PASSWORD_VALIDATION_FIELD_BYTES_m13]; // EXPLICITLY RESERVED for a future level 4: zero until then
	ui1		kdf_salt_extension[CRYPTO_KDF_SALT_EXTENSION_BYTES_m13]; // KDF salt = session UID then this (128 bits total); random per session, written at password generation
	ui1		protected_region[UH_PROTECTED_REGION_BYTES_m13]; // remainder of the former discretionary region (reclaimed 2026-07-18): the universal header is entirely machine space
	si8		lib_mod_time; // uutc: fs modification time the library stamps at its last write. LAST UH field (8-byte aligned at offset 1016) so kdf_exponent[] can grow without misaligning it. zeros in older files == unstamped
} UH_m13;

// universal header layout (compile-time; see LAYOUT_*_m13 note)
LAYOUT_SIZE_m13(UH_m13, UH_BYTES_m13);
LAYOUT_FIELD_m13(UH_m13, header_CRC, UH_HEADER_CRC_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, body_CRC, UH_BODY_CRC_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, file_end_time, UH_FILE_END_TIME_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, n_entries, UH_NUMBER_OF_ENTRIES_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, maximum_entry_size, UH_MAXIMUM_ENTRY_SIZE_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, segment_number, UH_SEGMENT_NUMBER_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, type_string, UH_TYPE_STR_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, type_code, UH_TYPE_CODE_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, MED_version_major, UH_MED_VERSION_MAJOR_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, MED_version_minor, UH_MED_VERSION_MINOR_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, byte_order_code, UH_BYTE_ORDER_CODE_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, session_start_time, UH_SESSION_START_TIME_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, file_start_time, UH_FILE_START_TIME_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, session_name, UH_SESSION_NAME_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, channel_name, UH_CHANNEL_NAME_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, supplementary_protected_region, UH_SUPPLEMENTARY_PROTECTED_REGION_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, anchor_escrow_field, UH_ANCHOR_ESCROW_FIELD_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, session_UID, UH_SESSION_UID_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, channel_UID, UH_CHANNEL_UID_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, segment_UID, UH_SEGMENT_UID_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, file_UID, UH_FILE_UID_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, provenance_UID, UH_PROVENANCE_UID_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, level_1_password_validation_field, UH_LEVEL_1_PASSWORD_VALIDATION_FIELD_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, level_2_password_validation_field, UH_LEVEL_2_PASSWORD_VALIDATION_FIELD_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, level_3_password_validation_field, UH_LEVEL_3_PASSWORD_VALIDATION_FIELD_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, video_data_file_number, UH_VIDEO_DATA_FILE_NUMBER_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, live, UH_LIVE_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, ordered, UH_ORDERED_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, metadata_section_2_encryption, UH_METADATA_SECTION_2_ENCRYPTION_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, metadata_section_3_encryption, UH_METADATA_SECTION_3_ENCRYPTION_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, time_series_data_encryption, UH_TIME_SERIES_DATA_ENCRYPTION_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, video_data_encryption, UH_VIDEO_DATA_ENCRYPTION_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, maximum_record_encryption_level, UH_MAXIMUM_RECORD_ENCRYPTION_LEVEL_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, encryption_reserved, UH_ENCRYPTION_RESERVED_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, crypto_schema, UH_CRYPTO_SCHEMA_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, kdf_exponent, UH_KDF_EXPONENT_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, anchor_type, UH_ANCHOR_TYPE_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, anchor_key_ID, UH_ANCHOR_KEY_ID_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, level_1_escrow_field, UH_LEVEL_1_ESCROW_FIELD_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, reserved_level_4_escrow_field, UH_RESERVED_LEVEL_4_ESCROW_FIELD_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, reserved_level_4_validation_field, UH_RESERVED_LEVEL_4_VALIDATION_FIELD_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, kdf_salt_extension, UH_KDF_SALT_EXTENSION_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, protected_region, UH_PROTECTED_REGION_OFFSET_m13);
LAYOUT_FIELD_m13(UH_m13, lib_mod_time, UH_LIB_MOD_TIME_OFFSET_m13);

// Metadata Structures
typedef struct {
	si1	level_1_password_hint[PASSWORD_HINT_BYTES_m13];
	si1	level_2_password_hint[PASSWORD_HINT_BYTES_m13];
	si1	anonymized_subject_ID[METADATA_ANONYMIZED_SUBJECT_ID_BYTES_m13]; // utf8[63], MED 1.1 & above (moved from universal header)
	ui1	protected_region[METADATA_SECTION_1_PROTECTED_REGION_BYTES_m13];
	ui1	discretionary_region[METADATA_SECTION_1_DISCRETIONARY_REGION_BYTES_m13];
} METADATA_SECTION_1_m13;

typedef struct {
 // channel type independent fields
	si1	session_description[METADATA_SESSION_DESCRIPTION_BYTES_m13]; // utf8[511]
	si1	channel_description[METADATA_CHANNEL_DESCRIPTION_BYTES_m13]; // utf8[255]
	si1	segment_description[METADATA_SEGMENT_DESCRIPTION_BYTES_m13]; // utf8[255]
	si1	equipment_description[METADATA_EQUIPMENT_DESCRIPTION_BYTES_m13]; // utf8[510]
	si4	acquisition_channel_number;
 // channel type specific fields
	si1	reference_description[TS_METADATA_REFERENCE_DESCRIPTION_BYTES_m13]; // utf8[255]
	sf8	sampling_frequency;
	sf8	low_frequency_filter_setting;
	sf8	high_frequency_filter_setting;
	sf8	line_filter_frequency_setting;	// line filtering applied OUTSIDE the library (hardware or upstream software), any mechanism - notch,
						// comb, template (renamed from notch_filter_frequency_setting: "notch" implied one mechanism).
						// Library-applied line filtering is per-block truth: CMP_BF_LINE_NOISE_FILTERED_m13
	sf8	power_line_frequency;		// mains fundamental in the recording environment (harmonics implied; renamed from AC_line_frequency)
	sf8	amplitude_units_conversion_factor;
	si1	amplitude_units_description[TS_METADATA_AMPLITUDE_UNITS_DESCRIPTION_BYTES_m13]; // utf8[31]
	sf8	time_base_units_conversion_factor;
	si1	time_base_units_description[TS_METADATA_TIME_BASE_UNITS_DESCRIPTION_BYTES_m13]; // utf8[31]
	si8	session_start_sample_number; // session, not segment relative
	si8	number_of_samples;
	si8	number_of_blocks;
	si8	maximum_block_bytes;
	ui4	maximum_block_samples;
	ui4	maximum_block_keysample_bytes;
	sf8	maximum_block_duration;
	si8	number_of_discontinuities;
	si8	maximum_contiguous_blocks;
	si8	maximum_contiguous_block_bytes;
	si8	maximum_contiguous_samples;
	ui1	protected_region[TS_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m13];
	ui1	discretionary_region[TS_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m13];
} TS_METADATA_SECTION_2_m13;

typedef struct {
 // type-independent fields
	si1	session_description[METADATA_SESSION_DESCRIPTION_BYTES_m13];	  // utf8[511]
	si1	channel_description[METADATA_CHANNEL_DESCRIPTION_BYTES_m13];	  // utf8[511]
	si1	segment_description[METADATA_SEGMENT_DESCRIPTION_BYTES_m13];	  // utf8[511]
	si1	equipment_description[METADATA_EQUIPMENT_DESCRIPTION_BYTES_m13];  // utf8[510]
	si4	acquisition_channel_number;
 // type-specific fields
	sf8	time_base_units_conversion_factor;
	si1	time_base_units_description[VID_METADATA_TIME_BASE_UNITS_DESCRIPTION_BYTES_m13]; // utf8[31]
	si8	session_start_frame_number; // session, not segment relative
	si8	number_of_frames;
	sf8	frame_rate;
	si8	number_of_clips;
	si8	maximum_clip_bytes;
	ui4	maximum_clip_frames;
	si4	number_of_video_files;
	sf8	maximum_clip_duration;
	si8	number_of_discontinuities;
	si8	maximum_contiguous_clips;
	si8	maximum_contiguous_clip_bytes;
	si8	maximum_contiguous_frames;
	ui4	horizontal_pixels;
	ui4	vertical_pixels;
	si1	video_format[VID_METADATA_VIDEO_FORMAT_BYTES_m13];   // utf8[31] (human-readable container/codec tag; machine truth in codec_config)
	ui1	display_transform; // VID_METADATA_DISPLAY_TRANSFORM_* (EXIF Orientation semantics; 0 == no entry, display as stored)
	ui1	video_flags;  // VID_METADATA_VIDEO_FLAGS_*
	ui2	codec_config_bytes;  // used bytes of codec_config (0 == no entry)
	ui1	codec_config[VID_METADATA_CODEC_CONFIG_MAX_BYTES_m13];  // container codec private data ("extradata"), verbatim; constant within a segment
	ui1	protected_region[VID_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m13];
	ui1	discretionary_region[VID_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m13];
} VID_METADATA_SECTION_2_m13;

// All metadata section substructures are the same sizes
typedef union {
	ui1				section_2[METADATA_SECTION_2_BYTES_m13];
	TS_METADATA_SECTION_2_m13	time_series_section_2;
	VID_METADATA_SECTION_2_m13	video_section_2;
} METADATA_SECTION_2_m13;

typedef struct {
	si8	recording_time_offset;
	DAYLIGHT_TIME_CHANGE_CODE_m13	daylight_time_start_code; // si1[8] / si8
	DAYLIGHT_TIME_CHANGE_CODE_m13	daylight_time_end_code; // si1[8] / si8
	si1	standard_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m13]; // ascii[8]
	si1	standard_timezone_string[TIMEZONE_STRING_BYTES_m13]; // ascii[31]
	si1	daylight_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m13]; // ascii[8]
	si1	daylight_timezone_string[TIMEZONE_STRING_BYTES_m13]; // ascii[31]
	si1	subject_name_1[METADATA_SUBJECT_NAME_BYTES_m13]; // utf8[31]
	si1	subject_name_2[METADATA_SUBJECT_NAME_BYTES_m13]; // utf8[31]
	si1	subject_name_3[METADATA_SUBJECT_NAME_BYTES_m13]; // utf8[31]
	si1	subject_ID[METADATA_SUBJECT_ID_BYTES_m13]; // utf8[31]
	si1	recording_country[METADATA_RECORDING_LOCATION_BYTES_m13]; // utf8[63]
	si1	recording_territory[METADATA_RECORDING_LOCATION_BYTES_m13]; // utf8[63]
	si1	recording_locality[METADATA_RECORDING_LOCATION_BYTES_m13]; // utf8[63]
	si1	recording_institution[METADATA_RECORDING_LOCATION_BYTES_m13]; // utf8[63]
	si1	geotag_format[METADATA_GEOTAG_FORMAT_BYTES_m13]; // ascii[31]
	si1	geotag_data[METADATA_GEOTAG_DATA_BYTES_m13]; // ascii[1023]
	si4	standard_UTC_offset;
	ui1	protected_region[METADATA_SECTION_3_PROTECTED_REGION_BYTES_m13];
	ui1	discretionary_region[METADATA_SECTION_3_DISCRETIONARY_REGION_BYTES_m13];
} METADATA_SECTION_3_m13;

// metadata section layouts (compile-time; see LAYOUT_*_m13 note)
// (the offset defines are FILE-level: subtract the section's own file offset for the in-struct position)
LAYOUT_SIZE_m13(METADATA_SECTION_1_m13, METADATA_SECTION_1_BYTES_m13);
LAYOUT_SIZE_m13(TS_METADATA_SECTION_2_m13, METADATA_SECTION_2_BYTES_m13);
LAYOUT_SIZE_m13(VID_METADATA_SECTION_2_m13, METADATA_SECTION_2_BYTES_m13);
LAYOUT_SIZE_m13(METADATA_SECTION_2_m13, METADATA_SECTION_2_BYTES_m13);
LAYOUT_SIZE_m13(METADATA_SECTION_3_m13, METADATA_SECTION_3_BYTES_m13);
LAYOUT_FIELD_m13(METADATA_SECTION_1_m13, level_1_password_hint, METADATA_LEVEL_1_PASSWORD_HINT_OFFSET_m13 - METADATA_SECTION_1_OFFSET_m13);
LAYOUT_FIELD_m13(METADATA_SECTION_1_m13, level_2_password_hint, METADATA_LEVEL_2_PASSWORD_HINT_OFFSET_m13 - METADATA_SECTION_1_OFFSET_m13);
LAYOUT_FIELD_m13(METADATA_SECTION_1_m13, anonymized_subject_ID, METADATA_ANONYMIZED_SUBJECT_ID_OFFSET_m13 - METADATA_SECTION_1_OFFSET_m13);
LAYOUT_FIELD_m13(METADATA_SECTION_1_m13, protected_region, METADATA_SECTION_1_PROTECTED_REGION_OFFSET_m13 - METADATA_SECTION_1_OFFSET_m13);
LAYOUT_FIELD_m13(METADATA_SECTION_1_m13, discretionary_region, METADATA_SECTION_1_DISCRETIONARY_REGION_OFFSET_m13 - METADATA_SECTION_1_OFFSET_m13);
LAYOUT_FIELD_m13(TS_METADATA_SECTION_2_m13, session_description, METADATA_SESSION_DESCRIPTION_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(TS_METADATA_SECTION_2_m13, channel_description, METADATA_CHANNEL_DESCRIPTION_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(TS_METADATA_SECTION_2_m13, segment_description, METADATA_SEGMENT_DESCRIPTION_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(TS_METADATA_SECTION_2_m13, equipment_description, METADATA_EQUIPMENT_DESCRIPTION_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(TS_METADATA_SECTION_2_m13, acquisition_channel_number, METADATA_ACQUISITION_CHANNEL_NUMBER_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(TS_METADATA_SECTION_2_m13, reference_description, TS_METADATA_REFERENCE_DESCRIPTION_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(TS_METADATA_SECTION_2_m13, sampling_frequency, TS_METADATA_SAMPLING_FREQUENCY_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(TS_METADATA_SECTION_2_m13, session_start_sample_number, TS_METADATA_SESSION_START_SAMPLE_NUMBER_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(TS_METADATA_SECTION_2_m13, number_of_samples, TS_METADATA_NUMBER_OF_SAMPLES_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(TS_METADATA_SECTION_2_m13, maximum_block_samples, TS_METADATA_MAXIMUM_BLOCK_SAMPLES_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(TS_METADATA_SECTION_2_m13, protected_region, TS_METADATA_SECTION_2_PROTECTED_REGION_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(TS_METADATA_SECTION_2_m13, discretionary_region, TS_METADATA_SECTION_2_DISCRETIONARY_REGION_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(VID_METADATA_SECTION_2_m13, time_base_units_conversion_factor, VID_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(VID_METADATA_SECTION_2_m13, session_start_frame_number, VID_METADATA_SESSION_START_FRAME_NUMBER_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(VID_METADATA_SECTION_2_m13, number_of_frames, VID_METADATA_NUMBER_OF_FRAMES_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(VID_METADATA_SECTION_2_m13, frame_rate, VID_METADATA_FRAME_RATE_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(VID_METADATA_SECTION_2_m13, number_of_clips, VID_METADATA_NUMBER_OF_CLIPS_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(VID_METADATA_SECTION_2_m13, maximum_clip_bytes, VID_METADATA_MAXIMUM_CLIP_BYTES_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(VID_METADATA_SECTION_2_m13, maximum_clip_frames, VID_METADATA_MAXIMUM_CLIP_FRAMES_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(VID_METADATA_SECTION_2_m13, number_of_video_files, VID_METADATA_NUMBER_OF_VIDEO_FILES_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(VID_METADATA_SECTION_2_m13, maximum_clip_duration, VID_METADATA_MAXIMUM_CLIP_DURATION_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(VID_METADATA_SECTION_2_m13, number_of_discontinuities, VID_METADATA_NUMBER_OF_DISCONTINUITIES_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(VID_METADATA_SECTION_2_m13, maximum_contiguous_clips, VID_METADATA_MAXIMUM_CONTIGUOUS_CLIPS_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(VID_METADATA_SECTION_2_m13, maximum_contiguous_frames, VID_METADATA_MAXIMUM_CONTIGUOUS_FRAMES_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(VID_METADATA_SECTION_2_m13, horizontal_pixels, VID_METADATA_HORIZONTAL_PIXELS_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(VID_METADATA_SECTION_2_m13, vertical_pixels, VID_METADATA_VERTICAL_PIXELS_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(VID_METADATA_SECTION_2_m13, video_format, VID_METADATA_VIDEO_FORMAT_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(VID_METADATA_SECTION_2_m13, display_transform, VID_METADATA_DISPLAY_TRANSFORM_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(VID_METADATA_SECTION_2_m13, video_flags, VID_METADATA_VIDEO_FLAGS_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(VID_METADATA_SECTION_2_m13, codec_config_bytes, VID_METADATA_CODEC_CONFIG_BYTES_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(VID_METADATA_SECTION_2_m13, codec_config, VID_METADATA_CODEC_CONFIG_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(VID_METADATA_SECTION_2_m13, protected_region, VID_METADATA_SECTION_2_PROTECTED_REGION_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(VID_METADATA_SECTION_2_m13, discretionary_region, VID_METADATA_SECTION_2_DISCRETIONARY_REGION_OFFSET_m13 - METADATA_SECTION_2_OFFSET_m13);
LAYOUT_FIELD_m13(METADATA_SECTION_3_m13, recording_time_offset, METADATA_RECORDING_TIME_OFFSET_OFFSET_m13 - METADATA_SECTION_3_OFFSET_m13);
LAYOUT_FIELD_m13(METADATA_SECTION_3_m13, subject_name_1, METADATA_SUBJECT_NAME_1_OFFSET_m13 - METADATA_SECTION_3_OFFSET_m13);
LAYOUT_FIELD_m13(METADATA_SECTION_3_m13, protected_region, METADATA_SECTION_3_PROTECTED_REGION_OFFSET_m13 - METADATA_SECTION_3_OFFSET_m13);
LAYOUT_FIELD_m13(METADATA_SECTION_3_m13, discretionary_region, METADATA_SECTION_3_DISCRETIONARY_REGION_OFFSET_m13 - METADATA_SECTION_3_OFFSET_m13);

#ifdef __cplusplus // c++ does not accept anonymous structures
typedef struct {
	METADATA_SECTION_1_m13		section_1;
	union {
		ui1				section_2[METADATA_SECTION_2_BYTES_m13];
		TS_METADATA_SECTION_2_m13	time_series_section_2;
		VID_METADATA_SECTION_2_m13	video_section_2;
	};
	METADATA_SECTION_3_m13		section_3;
} METADATA_m13;
#else // __cplusplus
typedef struct {
	METADATA_SECTION_1_m13		section_1;
	METADATA_SECTION_2_m13;
	METADATA_SECTION_3_m13		section_3;
} METADATA_m13;
#endif // standard C

// Record Structures
typedef struct REC_HDR_m13 { // struct name for medrec_m13.h interdependency
	ui4		record_CRC;
	ui4		total_record_bytes; // header + body bytes
	si8		start_time;
	union {
		ui4		type_code;
		struct {
			si1	type_string[TYPE_BYTES_m13];
			ui1	version_major;
			ui1	version_minor; // minor version == 0
			si1	encryption_level;
		};
	};
} REC_HDR_m13;

// record header layout (compile-time; see LAYOUT_*_m13 note)
LAYOUT_SIZE_m13(REC_HDR_m13, REC_HDR_BYTES_m13);
LAYOUT_FIELD_m13(REC_HDR_m13, record_CRC, REC_HDR_CRC_OFFSET_m13);
LAYOUT_FIELD_m13(REC_HDR_m13, total_record_bytes, REC_HDR_TOTAL_RECORD_BYTES_OFFSET_m13);
LAYOUT_FIELD_m13(REC_HDR_m13, start_time, REC_HDR_START_TIME_OFFSET_m13);
LAYOUT_FIELD_m13(REC_HDR_m13, type_string, REC_HDR_TYPE_STR_OFFSET_m13);
LAYOUT_FIELD_m13(REC_HDR_m13, type_code, REC_HDR_TYPE_CODE_OFFSET_m13);
LAYOUT_FIELD_m13(REC_HDR_m13, version_major, REC_HDR_VERSION_MAJOR_OFFSET_m13);
LAYOUT_FIELD_m13(REC_HDR_m13, version_minor, REC_HDR_VERSION_MINOR_OFFSET_m13);
LAYOUT_FIELD_m13(REC_HDR_m13, encryption_level, REC_HDR_ENCRYPTION_LEVEL_OFFSET_m13);

typedef struct {
	si8		file_offset; // never negative: the record indices are not used to indicate discontinuities
	si8		start_time;
	union {
		ui4		type_code;
		struct {
			si1	type_string[TYPE_BYTES_m13];
			ui1	version_major;
			ui1	version_minor;
			si1	encryption_level;
		};
	};
} REC_IDX_m13;

// Time Series Indices Structures
typedef struct {
	si8	file_offset; // negative values indicate discontinuity
	si8	start_time;
	si8	start_samp_num;
} TS_IDX_m13;

// Video Indices Structures
typedef struct {
	si8 	file_offset; // negative values indicate discontinuity
	si8	start_time;
	ui4	start_frame_num;
	ui4	vid_file_num;
} VID_IDX_m13;

typedef struct {
	si8	file_offset; // negative values indicate discontinuity (in time series & video indices)
	si8	start_time;
	ui1	pad[8];
} GEN_IDX_m13;

// All index structures are the same size, and have the same first two fields (hence GEN_IDX_m13)
typedef struct {
	union {
		REC_IDX_m13	record_index;
		TS_IDX_m13	time_series_index;
		VID_IDX_m13	video_index;
		GEN_IDX_m13	generic_index;
	};
} INDEX_m13;

// index family layout (compile-time; see LAYOUT_*_m13 note)
LAYOUT_SIZE_m13(REC_IDX_m13, INDEX_BYTES_m13);
LAYOUT_SIZE_m13(TS_IDX_m13, INDEX_BYTES_m13);
LAYOUT_SIZE_m13(VID_IDX_m13, INDEX_BYTES_m13);
LAYOUT_SIZE_m13(GEN_IDX_m13, INDEX_BYTES_m13);
LAYOUT_SIZE_m13(INDEX_m13, INDEX_BYTES_m13);
LAYOUT_FIELD_m13(REC_IDX_m13, file_offset, REC_IDX_FILE_OFFSET_OFFSET_m13);
LAYOUT_FIELD_m13(REC_IDX_m13, start_time, REC_IDX_START_TIME_OFFSET_m13);
LAYOUT_FIELD_m13(REC_IDX_m13, type_string, REC_IDX_TYPE_STR_OFFSET_m13);
LAYOUT_FIELD_m13(REC_IDX_m13, type_code, REC_IDX_TYPE_CODE_OFFSET_m13);
LAYOUT_FIELD_m13(REC_IDX_m13, version_major, REC_IDX_VERSION_MAJOR_OFFSET_m13);
LAYOUT_FIELD_m13(REC_IDX_m13, version_minor, REC_IDX_VERSION_MINOR_OFFSET_m13);
LAYOUT_FIELD_m13(REC_IDX_m13, encryption_level, REC_IDX_ENCRYPTION_LEVEL_OFFSET_m13);
LAYOUT_FIELD_m13(TS_IDX_m13, file_offset, TS_IDX_FILE_OFFSET_OFFSET_m13);
LAYOUT_FIELD_m13(TS_IDX_m13, start_time, TS_IDX_START_TIME_OFFSET_m13);
LAYOUT_FIELD_m13(TS_IDX_m13, start_samp_num, TS_IDX_START_SAMPLE_NUMBER_OFFSET_m13);
LAYOUT_FIELD_m13(VID_IDX_m13, file_offset, VID_IDX_FILE_OFFSET_OFFSET_m13);
LAYOUT_FIELD_m13(VID_IDX_m13, start_time, VID_IDX_START_TIME_OFFSET_m13);
LAYOUT_FIELD_m13(VID_IDX_m13, start_frame_num, VID_IDX_START_FRAME_OFFSET_m13);
LAYOUT_FIELD_m13(VID_IDX_m13, vid_file_num, VID_IDX_VIDEO_FILE_NUMBER_OFFSET_m13);



//**********************************************************************************//
// MARK: File Processing Struct (FPS)
//**********************************************************************************//

// Constants
#define FPS_ITEMS_NO_ENTRY_m13			((si8) 0) // passed as n_items
#define FPS_BYTES_NO_ENTRY_m13			((si8) 0) // passed as n_bytes
#define FPS_AUTOBYTES_m13			((si8) 0) // passed as n_bytes (use level header flags to determine FPS_UH_ONLY_m13 or FPS_FULL_FILE_m13) => just makes intention clearer than zero
#define FPS_NO_ALLOC_m13			((si8) -1) // used as n_bytes in FPS_open_m13() & FPS_init_m13() only
#define FPS_FULL_FILE_m13			((si8) 0x7FFFFFFFFFFFFFF9) // passed as offset or n_bytes (value positive so not treated as discontinuity if offset)
#define FPS_UH_ONLY_m13				((si8) 0x7FFFFFFFFFFFFFFA) // passed as offset or n_bytes (value positive so not treated as discontinuity if offset)
#define FPS_UH_OFFSET_m13			((si8) 0x7FFFFFFFFFFFFFFB) // passed as offset (value positive so not treated as discontinuity)
#define FPS_APPEND_m13				((si8) 0x7FFFFFFFFFFFFFFC) // passed as offset (value positive so not treated as discontinuity)
#define FPS_REL_START_m13			((si8) 0x7FFFFFFFFFFFFFFD) // passed as offset with rel_bytes vararg (value positive so not treated as discontinuity)
#define FPS_REL_CURR_m13			((si8) 0x7FFFFFFFFFFFFFFE) // passed as offset with rel_bytes vararg (value positive so not treated as discontinuity)
#define FPS_REL_END_m13				((si8) 0x7FFFFFFFFFFFFFFF) // passed as offset with rel_bytes vararg (value positive so not treated as discontinuity)
#define FPS_REL_OFFSET_m13(x)			( ((x) >= FPS_REL_START_m13) ? TRUE_m13 : FALSE_m13 )

#define FPS_FILE_LENGTH_UNKNOWN_m13		-1
#define FPS_PROTOTYPE_TYPE_CODE_m13		TS_METADATA_TYPE_CODE_m13 // any metadata type would do
#define FPS_FD_CLOSED_m13			FILE_FD_CLOSED_m13
#define FPS_FD_NO_ENTRY_m13			FILE_FD_NO_ENTRY_m13
#define FPS_FD_EPHEMERAL_m13			FILE_FD_EPHEMERAL_m13

// Directives Flags
// Open Mode Flags: "append" implies "write";
#define FPS_DF_NO_FLAGS_m13			((ui8) NO_FLAGS_m13)
#define FPS_DF_READ_MODE_m13			((ui8) 1 << 0)
#define FPS_DF_WRITE_MODE_m13			((ui8) 1 << 1)
#define FPS_DF_APPEND_MODE_m13			(((ui8) 1 << 2) | FPS_DF_WRITE_MODE_m13)
#define FPS_DF_CREATE_MODE_m13			((ui8) 1 << 3)
#define FPS_DF_TRUNCATE_MODE_m13		((ui8) 1 << 4)
// End Open Mode Flags
#define FPS_DF_CLOSE_AFTER_OP_m13		((ui8) 1 << 5) // close after operation (read / write)
#define FPS_DF_FLUSH_AFTER_WRITE_m13		((ui8) 1 << 6)
#define FPS_DF_UPDATE_UH_m13			((ui8) 1 << 7) // update universal header with write
#define FPS_DF_LEAVE_DECRYPTED_m13		((ui8) 1 << 8) // if data would be encrypted before write, leave data in memory unencrypted
#define FPS_DF_FREE_CPS_m13			((ui8) 1 << 9)
#define FPS_DF_MMAP_m13				((ui8) 1 << 10)

// Open Mode Flag Groups
#define FPS_NO_OPEN_MODE_m13			((ui8) 0)
#define FPS_R_OPEN_MODE_m13			FPS_DF_READ_MODE_m13
#define FPS_RP_OPEN_MODE_m13			( FPS_DF_READ_MODE_m13 | FPS_DF_WRITE_MODE_m13 )
#define FPS_W_OPEN_MODE_m13			( FPS_DF_WRITE_MODE_m13 | FPS_DF_CREATE_MODE_m13 | FPS_DF_TRUNCATE_MODE_m13 ) // open for writing, truncate if exists, create if does not
#define FPS_WP_OPEN_MODE_m13			( FPS_DF_WRITE_MODE_m13 | FPS_DF_READ_MODE_m13 | FPS_DF_CREATE_MODE_m13 | FPS_DF_TRUNCATE_MODE_m13) // open for reading & random writing, truncate if exists, create if does not
#define FPS_WN_OPEN_MODE_m13			( FPS_DF_WRITE_MODE_m13  | FPS_DF_CREATE_MODE_m13 )// open for random writing, create if does not exist
#define FPS_WNP_OPEN_MODE_m13			( FPS_DF_WRITE_MODE_m13 | FPS_DF_READ_MODE_m13  | FPS_DF_CREATE_MODE_m13 ) // open for reading & random writing, create if does not exist
#define FPS_A_OPEN_MODE_m13			( FPS_DF_APPEND_MODE_m13  | FPS_DF_CREATE_MODE_m13 ) // open for appending, create if does not exist
#define FPS_AP_OPEN_MODE_m13			( FPS_DF_APPEND_MODE_m13 | FPS_DF_READ_MODE_m13 | FPS_DF_CREATE_MODE_m13 ) // open for reading & appending, create if does not exist
#define FPS_AC_OPEN_MODE_m13			( FPS_DF_APPEND_MODE_m13 | FPS_DF_CREATE_MODE_m13 | FPS_DF_TRUNCATE_MODE_m13) // open for appending, truncate if exists, create if does not
#define FPS_ACP_OPEN_MODE_m13			( FPS_DF_APPEND_MODE_m13 | FPS_DF_READ_MODE_m13 | FPS_DF_CREATE_MODE_m13 | FPS_DF_TRUNCATE_MODE_m13 ) // open for reading & appending, truncate if exists, create if does not

#define FPS_OPEN_MODE_MASK_m13			( FPS_DF_READ_MODE_m13 | FPS_DF_WRITE_MODE_m13 | FPS_DF_APPEND_MODE_m13 | FPS_DF_TRUNCATE_MODE_m13 )

// Open Mode Strings
// ("c" is for "clobber"; "n" is for "no clobber")
// ("clobber" == "truncate" => can't use 't' because Windows uses it for "text")
#define FPS_R_OPEN_STR_m13			"r" // open for reading, fail if does not exist
#define FPS_RP_OPEN_STR_m13			"r+" // open for reading & random writing, fail if does not exist
#define FPS_W_OPEN_STR_m13			"w" // open for random writing, truncate if exists, create if does not
#define FPS_WP_OPEN_STR_m13			"w+" // open for reading & random writing, truncate if exists, create if does not
#define FPS_WN_OPEN_STR_m13			"wn" // open for random writing, create if does not exist  [truncate removed]
#define FPS_WNP_OPEN_STR_m13			"wn+" // open for reading & random writing, create if does not exist  [truncate removed]
#define FPS_A_OPEN_STR_m13			"a" // open for appending, create if does not exist
#define FPS_AP_OPEN_STR_m13			"a+" // open for reading & appending, create if does not exist
#define FPS_AC_OPEN_STR_m13			"ac" // open for appending, truncate if exists, create if does not  [truncate added]
#define FPS_ACP_OPEN_STR_m13			"ac+" // open for reading & appending, truncate if exists, create if does not  [truncate added]

#define FPS_READ_OPEN_STR_DEFAULT_m13		FPS_R_OPEN_STR_m13
#define FPS_WRITE_OPEN_STR_DEFAULT_m13		FPS_WN_OPEN_STR_m13
#define FPS_OPEN_STR_DEFAULT_m13		FPS_WNP_OPEN_STR_m13 // most flexible

// Directive Defaults
#define FPS_DIRECS_CLOSE_AFTER_OPERATION_DEFAULT_m13		FALSE_m13
#define FPS_DIRECS_FLUSH_AFTER_WRITE_DEFAULT_m13		TRUE_m13
#define FPS_DIRECS_UPDATE_UH_DEFAULT_m13			FALSE_m13
#define FPS_DIRECS_LEAVE_DECRYPTED_DEFAULT_m13			FALSE_m13
#define FPS_DIRECS_FREE_CMP_PROCESSING_STRUCT_DEFAULT_m13	TRUE_m13
#define FPS_DIRECS_MEMORY_MAP_DEFAULT_m13		 	FALSE_m13
#define FPS_DIRECS_READ_OPEN_MODE_DEFAULT_m13			FPS_R_OPEN_MODE_m13
#define FPS_DIRECS_READ_OPEN_STRING_DEFAULT_m13			FPS_R_OPEN_STRING_m13
#define FPS_DIRECS_WRITE_OPEN_MODE_DEFAULT_m13			FPS_WN_OPEN_MODE_m13
#define FPS_DIRECS_WRITE_OPEN_STRING_DEFAULT_m13		FPS_WN_OPEN_STRING_m13
#define FPS_DIRECS_OPEN_MODE_DEFAULT_m13			FPS_DIRECS_READ_OPEN_MODE_DEFAULT_m13  // default to read open (safest)


// Structures
typedef struct {
	ui8	flags;
} FPS_DIRECS_m13; // implemented as structure for future directives that may not work well as flags

// Parameters contain "mechanics" of FPS (mostly used internally by library functions)
typedef struct {
	si1			mode_str[6]; // open mode string (Windows systems: 'b' added by fopen_m13(), but not included in this string)
	tern			header_read; // universal header has been read in
	tern			full_file_read; // full file has been read in
	tern			recs_ordered; // record files only, runtime: order established for this open (scan, handshake, or sort - see G_read_records_m13()); NOT_SET until established
	si8			raw_data_bytes; // bytes in raw data array
	ui1			*raw_data; // universal header followed by data (in standard read - just region requested, in full file & mem map - matches media)
	struct CPS_m13		*cps; // CMP processing struct, for time series data FPSs
	FILE_m13		*fp; // FILE_m13 pointer (which contains standard FILE pointer); fp is initialized to point to "local_f" (below), but is manipulable
	FILE_m13		local_f; // all FPS manipulation functions use "fp" rather than "local_f" allowing the FPS to point to a different FILE_m13 if needed
 // memory mapping
	ui8			*mmap_block_bitmap; // each bit represents block_bytes bytes; NULL if not memory mapping
	ui4			mmap_block_bytes; // read size for memory mapped files (size data may be on different volumes, or even files within the same volume)
	ui4			mmap_n_blocks; // file system block in file == number of bits in bitmap
	ui4			mmap_blocks_read; // when this == mmap_n_blocks, mmap directive flag is unset & full_file_read parameter is set to TRUE_m13
	ui1			pad[4]; // future use (compiler would pad regardless)
} FPS_PARAMS_m13;

#ifdef __cplusplus
typedef struct {
	union {
		LH_m13			header; // in case just want the level header (type == GENERIC_TYPE_CODE_m13 => use universal header to get specific type)
		struct { // this struct replaces anonymous LH_m13 for C++
			union {
				ui4 		type_code;
				struct {
					si1 	type_string[TYPE_BYTES_m13];
					tern	allocated; // allocated on heap, independently (not en bloc)
						tern	names_differ; // file system & universal header name mismatch
				};
			};
			si1					*path; // NULL in proc_globs
			si1					*name; // NULL in proc_globs
			_Atomic(LH_m13 *)			parent; // NULL in proc_globs
			_Atomic(struct PROC_GLOBS_m13 *)	proc_globs; // self in proc_globs
			ui8					flags;
			_Atomic si8				access_time; // uutc of last use of this structure by the calling program (updated by open, read, & write functions)
		};
	};
	si1				local_name[MAX_NAME_BYTES_m13]; // full path from root including extension
	UH_m13				*uh; // universal header - points to base of raw_data array (even in video data files)
	FPS_DIRECS_m13	 		direcs;
	FPS_PARAMS_m13	 		params;
	union {			  	// the MED file types (set to point to current data (read or write)
		METADATA_m13		*metadata;
		REC_IDX_m13		*rec_inds;
		ui1			*rec_data;
		TS_IDX_m13		*ts_inds;
		ui1			*ts_data; // compressed data (not modified), CPS block header is modifiable pointer within this array
		VID_IDX_m13		*vid_inds;
		void			*vid_data;
		ui1			*data_ptrs; // generic name for all of the above (dissociable from raw data array / universal header, if needed)
	};
	si8				n_items; // items in most recent read/write, not necessarily the whole file
} FPS_m13;
#else // __cplusplus
typedef struct {
	union {
		LH_m13			header; // in case just want the level header (type == GENERIC_TYPE_CODE_m13 => use universal header to get specific type)
		LH_m13; 		// anonymous LH_m13
	};
	si1				local_name[MAX_NAME_BYTES_m13]; // base name of level (pointed to by level_header name)
	UH_m13				*uh; // universal header - points to base of raw_data array (even in video data files)
	FPS_DIRECS_m13	 		direcs;
	FPS_PARAMS_m13	 		params;
	union {				// the MED file types (set to point to current data (read or write)
		METADATA_m13		*metadata;
		REC_IDX_m13		*rec_inds;
		ui1			*rec_data;
		TS_IDX_m13		*ts_inds;
		ui1			*ts_data; // compressed data (not modified), CPS block header is modifiable pointer within this array
		VID_IDX_m13		*vid_inds;
		ui1			*vid_data;
		ui1			*data_ptrs; // generic name for all of the above (dissociable from raw data array / universal header, if needed)
	};
	si8				n_items; // items in most recent read/write, not necessarily the whole file
} FPS_m13;
#endif // standard C

// Prototypes
si8		FPS_bytes_for_items_m13(FPS_m13 *fps, si8 *n_items, si8 offset);
FPS_m13		*FPS_clone_m13(FPS_m13 *proto_fps, const si1 *path, si8 n_bytes, si8 copy_bytes, void *parent);
tern		FPS_close_m13(FPS_m13 *fps);
si4		FPS_compare_times_m13(const void *a, const void *b);
si8		FPS_flen_m13(FPS_m13 *fps, const si1 *path);
tern		FPS_free_m13(void *ptr);
si8		FPS_header_offset_m13(FPS_m13 *fps, const si1 *path);
FPS_m13		*FPS_init_m13(FPS_m13 *fps, const si1 *path, const si1 *mode_str, si8 n_bytes, void *parent);
FPS_DIRECS_m13  *FPS_init_direcs_m13(FPS_DIRECS_m13 *direcs);
FPS_PARAMS_m13  *FPS_init_params_m13(FPS_PARAMS_m13 *params);
tern		FPS_is_open_m13(FPS_m13 *fps);
si8		FPS_items_for_bytes_m13(FPS_m13 *fps, si8 *n_bytes);
tern		FPS_mmap_m13(FPS_m13 *fps, tern set);
si8		FPS_mmap_read_m13(FPS_m13 *fps, si8 n_bytes);
FPS_m13		*FPS_open_m13(const si1 *path, const si1 *mode_str, si8 n_bytes, void *parent, ...); // vararg(mode_str empty): ui8 fd_flags
FPS_m13 	*FPS_read_m13(FPS_m13 *fps, si8 offset, si8 n_bytes, si8 n_items, ...); // varargs(fps invalid): const si1 *path, const si1 *mode, const si1 *password, void *parent, ui8 lh_flags
											// varargs(offset == FPS_REL_START/CURR/END): si8 rel_bytes
tern		FPS_realloc_m13(FPS_m13 *fps, si8 n_bytes);
tern		FPS_reopen_m13(FPS_m13 *fps, const si1 *mode, ...); // vararg(mode_str empty): ui8 fd_flags
si8		FPS_resolve_offset_m13(FPS_m13 *fps, si8 offset, ...);  // varargs(offset == FPS_REL_START/CURR/END): si8 rel_bytes
si8		FPS_seek_m13(FPS_m13 *fps, si8 offset, ...); // varargs(offset == FPS_REL_START/CURR/END): si8 rel_bytes
si8		FPS_set_direcs_from_lh_flags_m13(FPS_m13 *fps, ui8 lh_flags);
ui8		FPS_set_open_flags_m13(FPS_m13 *fps, const si1 *mode_str);
si1		*FPS_set_open_string_m13(FPS_m13 *fps, ui8 flags);
tern		FPS_set_pointers_m13(FPS_m13 *fps, si8 offset);
tern		FPS_show_m13(FPS_m13 *fps);
tern		FPS_show_direcs_m13(FPS_m13 *fps);
tern		FPS_show_params_m13(FPS_m13 *fps);
tern		FPS_sort_m13(FPS_m13 **fps_array, si4 n_fps);
tern		FPS_update_maximum_entry_size_m13(FPS_m13 *fps, si8 n_bytes, si8 n_items, si8 offset);
tern		FPS_write_m13(FPS_m13 *fps, si8 offset, si8 n_bytes, si8 n_items, ...); // varargs(offset == FPS_REL_START/CURR/END): si8 rel_bytes



//**********************************************************************************//
// MARK: MED Structures
//**********************************************************************************//

typedef union {
	si1	str[8];
	ui4	code;
} STR_CODE_m13;

#ifdef __cplusplus
typedef struct {
	union {
		REC_HDR_m13	header; // in case just want the record header
		struct { // this replaces anonymous REC_HDR_m13 for C++
			ui4		record_CRC;
			ui4		total_record_bytes; // header + body bytes
			union { // anonymous union
				si8	time;
				si8	end_time;
			};
			union { // anonymous union
				ui4		type_code;
				struct {
					si1	type_string[TYPE_BYTES_m13];
					ui1	version_major;
					ui1	version_minor;
					si1	encryption_level;
				};
			};
		};
	};
	union {
		REC_Sgmt_v11_m13	body; // in case just want the record body
		struct { // this replaces anonymous REC_Sgmt_v11_m13 for C++
			si8		end_time;
			union {
				si8	start_sample_number;
				si8	start_frame_number;
			};
			union {
				si8	end_sample_number;
				si8	end_frame_number;
			};
			si4		segment_number;
			union {
				sf4	samp_freq;
				sf4	frame_rate;
			};
		};
	};
} Sgmt_REC_m13;
#else // __cplusplus
typedef struct {
	union {
		REC_HDR_m13	header; // in case just want the record header
		REC_HDR_m13; // anonymous REC_HDR_m13
	};
	union {
		REC_Sgmt_v11_m13	body; // in case just want the record body
		REC_Sgmt_v11_m13; // anonymous REC_Sgmt_v11_m13
	};
} Sgmt_REC_m13;
#endif // standard C
// NOTE: construction of Sgmt_REC_m13 in this way allows direct reading of Sgmt
// record headers & bodies into this structure (excluding the segment description)

#ifdef __cplusplus
typedef struct {
	union {
		LH_m13		header; // in case just want the level header
		struct { // this struct replaces anonymous LH_m13 for C++
			union {
				ui4 		type_code;
				struct {
					si1 	type_string[TYPE_BYTES_m13];
					tern	allocated; // allocated on heap, independently (not en bloc)
					tern	names_differ;
				};
			};
			si1					*path; // NULL in proc_globs
			si1					*name; // NULL in proc_globs
			_Atomic(LH_m13 *)			parent; // NULL in proc_globs
			_Atomic(struct PROC_GLOBS_m13 *)	proc_globs; // self in proc_globs
			ui8					flags;
			_Atomic si8				access_time; // uutc of last use of this structure by the calling program (updated by open, read, & write functions)
		};
	};
	FPS_m13			*metadata_fps; // also used as prototype
	union {
		FPS_m13		*ts_data_fps;
		FPS_m13		*vid_data_fps;
	};
	union {
		FPS_m13		*ts_inds_fps;
		FPS_m13		*vid_inds_fps;
	};
	FPS_m13			*rec_data_fps;
	FPS_m13			*rec_inds_fps;
	si8			n_contigua;
	CONTIGUON_m13		*contigua;
	si1			local_path[PATH_BYTES_m13]; // full path to segment directory (including segment directory itself)
	si1			fs_name[SEG_NAME_BYTES_m13]; // stored here, no segment_name field in universal header
	si1			uh_name[SEG_NAME_BYTES_m13]; // stored here, no segment_name field in universal header
	SLICE_m13		slice;
} SEG_m13;
#else // __cplusplus
typedef struct {
	union {
		LH_m13		header; // in case just want the level header
		LH_m13; // anonymous LH_m13
	};
	FPS_m13			*metadata_fps; // also used as prototype
	union {
		FPS_m13		*ts_data_fps;
		FPS_m13		*vid_data_fps;
	};
	union {
		FPS_m13		*ts_inds_fps;
		FPS_m13		*vid_inds_fps;
	};
	FPS_m13			*rec_data_fps;
	FPS_m13			*rec_inds_fps;
	si8			n_contigua;
	CONTIGUON_m13		*contigua;
	si1			local_path[PATH_BYTES_m13]; // full path to segment directory (including segment directory itself)
	si1			fs_name[SEG_NAME_BYTES_m13]; // stored here, no segment_name field in universal header
	si1			uh_name[SEG_NAME_BYTES_m13]; // stored here, no segment_name field in universal header
	SLICE_m13		slice;
} SEG_m13;
#endif // standard C

#ifdef __cplusplus
typedef struct CHAN_m13 {
	union {
		LH_m13		header; // in case just want the level header
		struct { // this struct replaces anonymous LH_m13 for C++
			union {
				ui4 		type_code;
				struct {
					si1 	type_string[TYPE_BYTES_m13];
					tern	allocated; // allocated on heap, independently (not en bloc)
					tern	names_differ;
				};
			};
			si1					*path; //  points to local_path
			si1					*name; // points to local_fs_name or local_uh_name
			_Atomic(LH_m13 *)			parent;
			_Atomic(struct PROC_GLOBS_m13 *)	proc_globs;
			ui8					flags;
			_Atomic si8				access_time; // uutc of last use of this structure by the calling program (updated by open, read, & write functions)
		};
	};
	FPS_m13			*metadata_fps; // used as prototype or ephemeral file, does not correspond to stored data
	FPS_m13			*rec_data_fps;
	FPS_m13			*rec_inds_fps;
	SEG_m13			**segs;
	Sgmt_REC_m13		*Sgmt_recs;
	si8			n_contigua;
	CONTIGUON_m13		*contigua;
	si1			local_path[PATH_BYTES_m13]; // full path to channel directory (including channel directory itself)
	si1			fs_name[NAME_BYTES_m13]; // name from file system
	si1			uh_name[NAME_BYTES_m13]; // name from universal header (if differs from file system name)
	SLICE_m13		slice;
} CHAN_m13;
#else // __cplusplus
typedef struct CHAN_m13 {
	union {
		LH_m13		header; // in case just want the level header
		LH_m13; // anonymous LH_m13
	};
	FPS_m13			*metadata_fps; // used as prototype or ephemeral file, does not correspond to stored data
	FPS_m13			*rec_data_fps;
	FPS_m13			*rec_inds_fps;
	SEG_m13			**segs;
	Sgmt_REC_m13		*Sgmt_recs;
	si8			n_contigua;
	CONTIGUON_m13		*contigua;
	si1			local_path[PATH_BYTES_m13]; // full path to channel directory (including channel directory itself)
	si1			fs_name[NAME_BYTES_m13]; // name from file system
	si1			uh_name[NAME_BYTES_m13]; // name from universal header (if differs from file system name, otherwise empty)
	SLICE_m13		slice;
} CHAN_m13;
#endif // standard C

#ifdef __cplusplus
typedef struct {
	union {
		LH_m13	header; // in case just want the level header
		struct { // this struct replaces anonymous LH_m13 in C++
			union {
				ui4 		type_code;
				struct {
					si1 	type_string[TYPE_BYTES_m13];
					tern	allocated; // allocated on heap, independently (not en bloc)
					tern	names_differ;
				};
			};
			si1					*path; // NULL in proc_globs
			si1					*name; // NULL in proc_globs
			_Atomic(LH_m13 *)			parent; // NULL in proc_globs
			_Atomic(struct PROC_GLOBS_m13 *)	proc_globs; // self in proc_globs
			ui8					flags;
			_Atomic si8				access_time; // uutc of last use of this structure by the calling program (updated by open, read, & write functions)
		};
	};
	FPS_m13		**rec_data_fps;
	FPS_m13		**rec_inds_fps;
	Sgmt_REC_m13	*Sgmt_recs;
	si1		local_path[PATH_BYTES_m13]; // full path to segmented session records directory (including directory itself)
	SLICE_m13	slice;
} SSR_m13;
#else // __cplusplus
typedef struct {
	union {
		LH_m13	header; // in case just want the level header
		LH_m13; // anonymous LH_m13
	};
	FPS_m13		**rec_data_fps;
	FPS_m13		**rec_inds_fps;
	Sgmt_REC_m13	*Sgmt_recs;
	si1		local_path[PATH_BYTES_m13]; // full path to channel directory (including channel directory itself)
	si1		fs_name[NAME_BYTES_m13]; // name from file system
	si1		uh_name[NAME_BYTES_m13]; // name from universal header (if differs from file system name)
	SLICE_m13	slice;
} SSR_m13;
#endif // standard C

#ifdef __cplusplus
typedef struct {
	union {
		LH_m13		header; // in case just want the level header
		struct { // this struct replaces anonymous LH_m13 in C++
			union {
				ui4 		type_code;
				struct {
					si1 	type_string[TYPE_BYTES_m13];
					tern	allocated; // allocated on heap, independently (not en bloc)
					tern	names_differ;
				};
			};
			si1					*path; // NULL in proc_globs
			si1					*name; // NULL in proc_globs
			_Atomic(LH_m13 *)			parent; // NULL in proc_globs
			_Atomic(struct PROC_GLOBS_m13 *)	proc_globs; // self in proc_globs
			ui8					flags;
			_Atomic si8				access_time; // uutc of last use of this structure by the calling program (updated by open, read, & write functions)
		};
	};
	FPS_m13			*ts_metadata_fps; // used as prototype or ephemeral file, does not correspond to stored data
	FPS_m13			*vid_metadata_fps; // used as prototype or ephemeral file, does not correspond to stored data
	si4			n_ts_chans;
	CHAN_m13		**ts_chans;
	si4			n_vid_chans;
	CHAN_m13		**vid_chans;
	FPS_m13			*rec_data_fps;
	FPS_m13			*rec_inds_fps;
	SSR_m13			*ssr;
	Sgmt_REC_m13		*Sgmt_recs;
	si8			n_contigua;
	CONTIGUON_m13		*contigua;
	SLICE_m13		slice;
} SESS_m13;
#else // __cplusplus
typedef struct {
	union {
		LH_m13		header; // in case just want the level header
		LH_m13; // anonymous LH_m13
	};
	FPS_m13			*ts_metadata_fps; // used as prototype or ephemeral file, does not correspond to stored data
	FPS_m13			*vid_metadata_fps; // used as prototype or ephemeral file, does not correspond to stored data
	si4			n_ts_chans;
	CHAN_m13		**ts_chans;
	si4			n_vid_chans;
	CHAN_m13		**vid_chans;
	FPS_m13			*rec_data_fps;
	FPS_m13			*rec_inds_fps;
	SSR_m13			*ssr;
	Sgmt_REC_m13		*Sgmt_recs;
	si8			n_contigua;
	CONTIGUON_m13		*contigua;
	SLICE_m13		slice;
} SESS_m13;
#endif // standard C

// Miscellaneous structures that depend on above
typedef struct {
	si1			MED_dir[PATH_BYTES_m13];
	ui8			flags;
	LH_m13			*MED_struct; // SESS_m13, SSR_m13, CHAN_m13, or SEG_m13 pointer (used to pass & return)
	LH_m13			*parent; // SESS_m13 or CHAN_m13 pointer
	SLICE_m13		*slice;
	const si1		*password;
} READ_MED_THREAD_INFO_m13;

typedef struct {
	CHAN_m13	*chan;
	si4		acq_num;
} ACQ_NUM_SORT_m13;

// schema 0 -> 1 re-key context, held by G_update_MED_version_m13() & passed to G_update_MED_type_m13().
// uh = the established schema-1 password data to stamp; legacy_pwd/schema1_pwd = the two key contexts swapped into
// the process-global pg->password_data around decrypt (legacy) / re-encrypt (schema-1). NULL context => no re-key.
typedef struct {
	UH_m13			*uh;
	PASSWORD_DATA_m13	legacy_pwd;
	PASSWORD_DATA_m13	schema1_pwd;
} REKEY_CTX_m13;



//**********************************************************************************//
// MARK: GENERAL MED Functions (G)
//**********************************************************************************//

// Prototypes
CHAN_m13		*G_active_channel_m13(SESS_m13 *sess, si1 channel_type);
void			G_add_behavior_exec_m13(const si1 *function, const si4 line, ui4 code);
ui4 			G_add_level_extension_m13(si1 *directory_name);
tern			G_all_zeros_m13(ui1 *bytes, si4 field_length);
CHAN_m13		*G_alloc_channel_m13(CHAN_m13 *chan, FPS_m13 *proto_fps, const si1 *path, LH_m13 *parent, si4 n_segs, tern chan_recs, tern seg_recs);
SEG_m13			*G_alloc_segment_m13(SEG_m13 *seg, FPS_m13 *proto_fps, const si1 *path, LH_m13 *parent, si4 seg_num, tern seg_recs);
SESS_m13		*G_alloc_session_m13(FPS_m13 *proto_fps, const si1 *path, si4 n_ts_chans, si4 n_vid_chans, si4 n_segs, si1 **ts_chan_names, si1 **vid_chan_names, tern sess_recs, tern seg_sess_recs, tern chan_recs, tern seg_recs);
void 			G_apply_recording_time_offset_m13(si8 *time, si8 recording_time_offset);
si1 			*G_base_name_m13(void *level_header, const si1 *path, si1 *base_name);
BEHAVIOR_STACK_m13	*G_behavior_stack_m13(void);
void			G_behavior_stack_reset_exec_m13(const si1 *function, si4 line, ui4 code);
si1			*G_behavior_string_m13(ui4 behavior_code, si1 *behavior_string);
si8			G_build_contigua_m13(void *level_header);
Sgmt_REC_m13		*G_build_Sgmt_records_m13(void *level_header, si4 search_mode, ui4 *source_type);
tern 			G_calculate_indices_CRCs_m13(FPS_m13 *fps);
tern			G_calculate_metadata_CRC_m13(FPS_m13 *fps);
tern			G_calculate_record_data_CRCs_m13(FPS_m13 *fps);
tern			G_calculate_time_series_data_CRCs_m13(FPS_m13 *fps);
tern			G_calculate_video_data_CRCs_m13(FPS_m13 *fps);
CHAN_m13		*G_change_index_chan_m13(SESS_m13 *sess, CHAN_m13 *chan, const si1 *chan_name, si1 chan_type);
ui4			G_channel_type_from_path_m13(const si1 *path);
tern			G_check_char_type_m13(void);
tern			G_check_file_list_m13(si1 **file_list, si4 n_files);
tern			G_check_file_system_m13(const si1 *file_system_path, si4 is_cloud, ...); // varargs (is_cloud == TRUE_m13): const si1 *cloud_directory, const si1 *cloud_service_name, const si1 *cloud_utilities_directory
tern 			G_check_password_m13(const si1 *password);
tern 			G_check_new_password_m13(const si1 *password, ui1 level, tern lib_generated); // creation-time policy (level-aware min length [L3 human = 12] + weak warning + OPT-IN class rule); lib_generated => exempt; NOT used when reading or for re-keyed legacy passwords
tern			G_password_char_ok_m13(si4 c); // shared "acceptable password byte" predicate (rejects C0 control codes & DEL; allows UTF-8): used by entry AND creation so CLI/GUI agree
si1			*G_password_requirements_m13(si1 *string); // ready-to-display sentence of the CURRENT new-password policy (length + opt-in composition rule) for a GUI/terminal to show during creation; NULL => allocate (caller frees)
ui1			G_password_classes_m13(const si1 *password); // bitmask of PW_CLASS_*_m13 present in the password (GUI can mirror the composition rule)
si4			G_estimate_password_bits_m13(const si1 *password); // conservative entropy estimate in bits (dictionary/keyboard-walk/date/repeat aware); creation-time only
ui1			G_clamp_kdf_exponent_m13(si4 exponent, tern lib_generated_password); // enforces the non-overridable floors & ceiling; the single point of policy
ui1			G_suggest_kdf_exponent_m13(const si1 *password, tern lib_generated_password); // constant-attack-cost suggestion: clamp(target - haircut(estimate))
sf8			G_kdf_open_time_estimate_m13(ui1 exponent, sf8 *slow_machine_secs); // seconds per password per session open ON THIS MACHINE (calibrated, not tabulated)
tern			G_password_info_m13(const si1 *path, const si1 *password, PASSWORD_INFO_m13 *info); // read one header (+ §1 hints); password NULL = info only (no KDF); else validate & set info->access_level. No open, no side effects, no record scan.
void			G_clear_error_m13(void);
tern			G_clear_terminal_m13(void);
si4			G_compare_acq_nums_m13(const void *a, const void *b);
si4 			G_compare_record_index_times_m13(const void *a, const void *b);
tern			G_condition_password_m13(const si1 *password, si1 *password_bytes, ui1 crypto_schema); // schema >= 1: raw UTF-8 bytes, REJECT if > 16 bytes; legacy: terminal-byte reduction (read-only compatibility)
tern			G_condition_slice_m13(void *level_header, SLICE_m13 *slice);
tern			G_condition_timezone_info_m13(TIMEZONE_INFO_m13 *tz_info);
tern			G_correct_universal_header_m13(FPS_m13 *fps);
ui4			G_current_behavior_m13(void); // returns behavior code
BEHAVIOR_m13		*G_current_behavior_entry_m13(void); // returns pointer to BEHAVIOR_m13 struct (useful for debugging)
si8			G_current_uutc_m13(void);
si4			G_days_in_month_m13(si4 month, si4 year);
tern			G_AES_crypt_m13(UH_m13 *uh, PASSWORD_DATA_m13 *pwd, si1 level, ui1 *data, si8 len, tern encrypt);
tern 			G_decrypt_metadata_m13(FPS_m13 *fps);
tern			G_encryption_decrypted_state_m13(si1 level);  // TRUE only for the decrypted transients (-LEVEL_1/-LEVEL_2); ENCRYPTION_NO_ENTRY_m13 (-128) is a legitimate sentinel & is NOT one
tern			G_validate_encryption_map_m13(UH_m13 *uh, ui4 type_code);  // a NEGATIVE encryption level for THIS file's own region is an in-memory transient: on disk it means the file was written mid-decryption & cannot be trusted (irrelevant regions are ignored - real sessions carry stale values there)
tern			G_validate_metadata_m13(FPS_m13 *fps);  // crude, format-constrained sanity of metadata section 2, before its numbers reach an allocator or a divisor
tern 			G_decrypt_records_m13(FPS_m13 *fps, ...); // varargs (fps == NULL): REC_HDR_m13 *rh, si8 n_records (used to decrypt Sgmt_records arrays)
tern 			G_decrypt_time_series_m13(FPS_m13 *fps);
tern 			G_decrypt_video_m13(FPS_m13 *fps);
void			G_delete_behavior_stack_m13(void);
void			G_delete_function_stack_m13(void);
si4			G_DST_offset_m13(si8 uutc);
tern			G_encrypt_metadata_m13(FPS_m13 *fps);
tern			G_encrypt_records_m13(FPS_m13 *fps);
tern 			G_encrypt_time_series_m13(FPS_m13 *fps);
tern 			G_encrypt_video_m13(FPS_m13 *fps);
tern			G_enter_password_m13(si1 *password, const si1 *prompt, tern confirm_no_entry, sf8 timeout_secs, tern create_password); // interactive TTY entry; accepts UTF-8 (up to PASSWORD_BYTES bytes, whole characters); create_password confirms
#define G_enter_ascii_password_m13	G_enter_password_m13  // compat alias: entry is no longer ASCII-only (accepts UTF-8)
void			G_error_clear_m13(void);
si4			G_error_code_m13(void);
void 			G_error_message_m13(const si1 *fmt, ...) FMT_ATTR_m13(1, 2);
si1 			G_exists_m13(const si1 *path);
si8			G_file_length_m13(void *fp, const si1 *path);  // fp: FILE_m13 * or FILE * (as the rest of the f*_m13 family)
si1			**G_file_list_m13(si1 **file_list, si4 *n_files, const si1 *enclosing_directory, const si1 *name, const si1 *extension, ui4 flags);
FILE_TIMES_m13		*G_file_times_m13(FILE_m13 *fp, const si1 *path, FILE_TIMES_m13 *ft, tern set_time);
tern			G_fill_empty_password_bytes_m13(si1 *password_bytes);
CONTIGUON_m13		*G_find_discontinuities_m13(void *level_header, si8 *n_contigua);
si8			G_find_index_m13(SEG_m13 *seg, si8 target, ui4 mode);
si1			*G_find_metadata_file_m13(const si1 *path, si1 *md_path);
si8			G_find_record_index_m13(FPS_m13 *rec_inds_fps, si8 target_time, ui4 mode, si8 low_idx);
si1			*G_find_timezone_acronym_m13(si1 *timezone_acronym, si4 standard_UTC_offset, si4 DST_offset);
si4			G_first_open_segment_m13(void *level_header);  // index of first open (non-NULL) segment on the reference channel; (si4) FALSE_m13 if none
si8			G_flen_m13(FILE_m13 *fp, const si1 *path);
//si8 			G_frame_number_for_uutc_m13(void *level_header, si8 target_uutc, ui4 mode, ...); // varargs (level_header == NULL): si8 ref_frame_number, si8 ref_uutc, sf8 frame_rate
tern			G_free_channel_m13(void *ptr);
void			G_free_global_tables_m13(void);
void			G_free_globals_m13(tern cleanup_for_exit);
tern			G_free_segment_m13(void *ptr);
tern			G_free_session_m13(void *ptr);
tern			G_free_ssr_m13(void *ptr);
tern			G_full_path_m13(const si1 *path, si1 *full_path);
FUNCTION_STACK_m13	*G_function_stack_m13(pid_t_m13 _id);
si1			**G_generate_numbered_names_m13(si1 **names, const si1 *prefix, si4 n_names);
tern			G_generate_password_data_m13(FPS_m13 *fps, const si1 *L1_pw, const si1 *L2_pw, const si1 *L3_pw, const si1 *L1_pw_hint, const si1 *L2_pw_hint, ui1 lib_generated_mask, tern check_policy); // lib_generated_mask: per-level bit (bit0=L1, bit1=L2, bit2=L3) set when that level's pw came from G_generate_password_m13 (=> lower per-level KDF floor). check_policy: run G_check_new_password (FALSE for a re-key of legacy passwords, which are accepted as-is)
tern			G_generate_password_m13(si1 *password, si4 n_chars, ui1 *suggested_kdf_exponent); // machine password: keyboard-ASCII, CSPRNG; n_chars<=0 => PASSWORD_BYTES; returns a LOW suggested KDF exponent (entropy is in the password)
si8			G_generate_recording_time_offset_m13(void *level_header, si8 recording_start_time_uutc); // level resolves the pg whose constants are set (writer-side, 2026-08-10); NULL => current thread
si1			*G_generate_segment_name_m13(si1 *segment_name, FPS_m13 *fps);
ui8			G_generate_UID_m13(ui8 *uid);
void		*G_guarded_table_alloc_m13(size_t bytes);  // page-aligned, PROT_NONE guard pages either side; for computed read-forever tables
tern		G_guarded_table_seal_m13(void *table, size_t bytes);  // PROT_READ after filling: wild writes fault at the store
tern		G_guarded_table_free_m13(void *table, size_t bytes);  // restores RW & frees; (ptr, bytes) must match the alloc
si8			G_header_offset_m13(FILE_m13 *fp, const si1 *path);
tern			G_include_record_m13(ui4 type_code, si4 *record_filters);
si8			G_index_for_time_m13(void *level_header, si8 target_time, ui4 mode, ...);  // varargs(lh == NULL): si8 ref_index, si8 ref_time, sf8 rate
tern			G_init_global_tables_m13(tern init_all_tables);
tern			G_init_globals_m13(tern init_all_tables, const si1 *app_path, ...); // varargs(app_path): ui4 version_major, ui4 version_minor
tern			G_init_medlib_m13(tern init_all_tables, const si1 *app_path, ...); // varargs(app_path): ui4 version_major, ui4 version_minor;
tern			G_init_metadata_m13(FPS_m13 *fps, tern init_for_update);
tern			G_init_metadata_struct_m13(METADATA_m13 *md, ui4 type_code, tern init_for_update, PROC_GLOBS_m13 *pg); // bare-struct initializer (sections 2 & 3; section 1 untouched); NULL pg => current thread
SLICE_m13		*G_init_slice_m13(SLICE_m13 *slice);
tern			G_init_timezone_tables_m13(void);
tern			G_init_universal_header_m13(FPS_m13 *fps, ui4 type_code, tern generate_file_UID, tern originating_file);
tern			G_is_level_header_m13(void *ptr);
tern			G_is_video_data_m13(const si1 *path);
ui4			G_level_m13(const si1 *full_file_name, ui4 *input_type_code);
tern			G_local_timezone_m13(TIMEZONE_INFO_m13 *timezone_info); // fill from the machine's OWN timezone configuration (no network, no shell): acronyms/names + standard UTC offset for G_set_time_constants_m13()'s cascade; FALSE => could not determine (callers fall back to G_location_info_m13())
tern			G_location_info_m13(LOCATION_INFO_m13 *loc_info, const si1 *ip_str, const si1 *ipinfo_token, tern set_timezone_globals, tern prompt);
tern			G_MED_file_m13(ui4 type_code);
ui4			G_MED_path_components_m13(const si1 *path, si1 *MED_dir, si1* MED_name); // passed path must exist (accepts a file within a MED directory)
ui4			G_MED_path_parse_m13(const si1 *path, si1 *MED_dir, si1 *MED_name); // as G_MED_path_components_m13(), but path need not exist (use when allocating a new MED tree)
ui4			G_MED_type_code_from_string_m13(const si1 *string);
const si1		*G_MED_type_string_from_code_m13(ui4 code);
tern			G_merge_metadata_m13(FPS_m13 *md_fps_1, FPS_m13 *md_fps_2, FPS_m13 *merged_md_fps);
tern			G_merge_universal_headers_m13(FPS_m13 *fps_1, FPS_m13 *fps_2, FPS_m13 *merged_fps);
void 			G_message_m13(const si1 *fmt, ...);
#ifdef MATLAB_m13
tern			G_mex_interp_thread_m13(void);  // TRUE iff current thread is MATLAB's interpreter thread (the only thread mex API calls are legal on)
#endif
CHAN_m13		*G_open_channel_m13(CHAN_m13 *chan, SLICE_m13 *slice, const si1 *chan_path, void *parent, ui8 flags, const si1 *password);
pthread_rval_m13	G_open_channel_thread_m13(void *ptr);
tern			G_open_records_m13(void *level_header, ...);  // varagrgs(level == ssr): si4 seg_num
SSR_m13			*G_open_seg_sess_recs_m13(SESS_m13 *sess);
SEG_m13			*G_open_segment_m13(SEG_m13 *seg, SLICE_m13 *slice, const si1 *seg_path, void *parent, ui8 flags, const si1 *password);
pthread_rval_m13	G_open_segment_thread_m13(void *ptr);
SESS_m13		*G_open_session_m13(SESS_m13 *sess, SLICE_m13 *slice, void *file_list, si4 list_len, ui8 flags, const si1 *password, const si1 *index_channel_name);
si8			G_pad_m13(ui1 *buffer, si8 content_len, ui4 alignment);
ui4			G_page_size_m13(void); // system memory page size: HW_params' known value; direct query fallback (lock-free: usable from allocation primitives)
ui4			G_map_granularity_m13(void); // mapping-placement granularity: page size on Unix, allocation granularity (64 KB) on Windows; same lock-free contract
// NOTE: applications needing the process-wide tables (HW_params, NET_params, ...) declare
// "extern GLOBALS_m13 *globals_m13;" & read globals_m13->tables-> directly (external linkage in
// non-MATLAB builds; under MATLAB_m13 the mex sees the statics, so no accessor is needed there either)
tern			G_path_parts_m13(const si1 *file_name, si1 *path, si1 *name, si1 *extension);
void			G_pop_behavior_exec_m13(const si1 *function, const si4 line);
void			G_pop_function_exec_m13(const si1 *function, const si4 line);
tern			G_proc_error_get_m13(void *level_header);
void			G_proc_error_set_m13(void *level_header, tern state);
PROC_GLOBS_m13		*G_proc_globs_m13(void *level_header); // level_header NULL: resolve by thread id / ancestry
									// NOTE: a thread with multiple open sessions has multiple proc_globs with the same _id =>
									// NULL resolves to the first found; pass a level header for deterministic resolution
void			G_proc_globs_delete_m13(void *level_header);
PROC_GLOBS_m13		*G_proc_globs_find_m13(void *level_header); // as G_proc_globs_m13(), but no creation & no linkage (returns NULL if not found)
tern			G_proc_globs_init_m13(PROC_GLOBS_m13 *pg);
PROC_GLOBS_m13		*G_proc_globs_new_m13(void *level_header);
tern			G_process_password_data_m13(FPS_m13 *fps, const si1 *unspecified_pw);
tern			G_propagate_flags_m13(void *level_header, ui8 new_flags);
void			G_push_behavior_exec_m13(const si1 *function, const si4 line, ui4 code);
void			G_push_function_exec_m13(const si1 *function, const si4 line);
tern			G_rates_vary_m13(SESS_m13 *sess);
CHAN_m13		*G_read_channel_m13(CHAN_m13 *chan, SLICE_m13 *slice, ...); // varargs(chan == NULL): const si1 *chan_path, void *parent, ui8 lh_flags, const si1 *password, const si1 *index_channel_name
pthread_rval_m13	G_read_channel_thread_m13(void *ptr);
si4			G_read_cs_file_m13(const si1 *cs_file_name, si4 n_available_channels, si4 **map, si4 **reverse_map, si1 ***names, sf8 **decimation_frequencies, ui4 **block_samples, si1 ***descriptions);
LH_m13			*G_read_data_m13(void *level_header, SLICE_m13 *slice, ...); // varargs(lh == NULL): const si1 *file_list, si4 list_len, ui8 lh_flags, const si1 *password, const si1 *index_channel_name
tern			G_read_medlibrc_m13(const si1 *path, tern sequential); // path NULL == the user file (~/.medlibrc; written with defaults if unusable); non-NULL == an application's own RC, applied on top & never written by the library. sequential TRUE == fast forward walk (fields in emitted order); FALSE == search from start (a partial file, any order)
si8			G_read_records_m13(void *level_header, SLICE_m13 *slice, ...); // varargs(level->type_code == LH_SSR_m13): si4 seg_num
SEG_m13			*G_read_segment_m13(SEG_m13 *seg, SLICE_m13 *slice, ...); // varargs(seg == NULL): const si1 *seg_path, void *parent, ui8 lh_flags, const si1 *password
pthread_rval_m13	G_read_segment_thread_m13(void *ptr);
SESS_m13		*G_read_session_m13(SESS_m13 *sess, SLICE_m13 *slice, ...); // varargs(sess == NULL): void *file_list, si4 list_len, ui8 lh_flags, const si1 *password, const si1 *index_channel_name
si8			G_read_time_series_data_m13(SEG_m13 *seg, SLICE_m13 *slice);
UH_m13			*G_read_universal_header_m13(const si1 *path, UH_m13 *uh);
tern			G_recover_passwords_m13(const si1 *L3_password, UH_m13* universal_header, si1 *L1_out, si1 *L2_out); // L1_out/L2_out (legacy schema only): if non-NULL, RETURN the recovered password STRINGS (>= MAX_PASSWORD_STRING_BYTES each; empty if that level absent) & stay SILENT (re-key/programmatic use); both NULL => print recovered values (recovery tool)
si1			*G_recover_password_string_m13(si1 *password_string, ui1 target_level, const si1 *higher_level_pw, UH_m13 *universal_header); // escrow recovery: returns the TYPED password string of target_level; read-only, validated before return
si1			*G_unseal_password_string_m13(si1 *password_string, const ui1 *anchor_private_key, UH_m13 *universal_header); // anchor (type 2) recovery of the top-level password string; anchor-holder side, requires only the universal header
tern			G_random_bytes_m13(ui1 *buffer, si4 n_bytes); // SYSTEM ENTROPY (CSPRNG): for cryptographic material only (rand64_m13() is a seeded PRNG - fine for UIDs, fatal for keys)
void			G_remove_behavior_exec_m13(const si1 *function, const si4 line, ui4 code);
void			G_remove_recording_time_offset_m13(si8 *time, si8 recording_time_offset);
tern			G_reset_metadata_for_update_m13(FPS_m13 *fps);
si4			G_search_mode_m13(SLICE_m13 *slice);
si4			G_search_Sgmt_records_m13(Sgmt_REC_m13 *Sgmt_records, si4 n_segs, SLICE_m13 *slice, ui4 search_mode);
void			G_secure_erase_m13(void *buffer, size_t n_bytes); // volatile zeroing of key material (plain memset before return/free is legally elided by optimizers)
si4			G_segment_for_index_m13(void *level_header, si8 target_index);
si4			G_segment_for_path_m13(const si1 *path);
si4			G_segment_for_time_m13(void *level_header, si8 target_time);
si4			G_segment_range_m13(void *level_header, SLICE_m13 *slice);
ui4			*G_segment_video_start_frames_m13(FPS_m13 *vid_inds_fps, ui4 *n_video_files);
tern			G_sendgrid_email_m13(const si1 *sendgrid_key, const si1 *to_email, const si1 *cc_email, const si1 *to_name, const si1 *subject, const si1 *content, const si1 *from_email, const si1 *from_name, const si1 *reply_to_email, const si1 *reply_to_name);
tern			G_session_directory_m13(FPS_m13 *fps);
si1			*G_session_path_for_path_m13(const si1 *path, si1 *sess_path);
si8			G_session_samples_m13(void *level_header, sf8 rate);
tern			G_set_anchor_public_key_m13(const ui1 *public_key, ui1 key_ID); // configure the type 2 recovery anchor for subsequent session creation (NULL clears; process-wide, set once at startup)
tern			G_set_encryption_map_m13(UH_m13 *uh, si1 metadata_section_2, si1 metadata_section_3, si1 time_series_data, si1 video_data, si1 maximum_record_encryption_level); // stamp the session-wide encryption levels into a UH at creation (call on every UH); pass ENCRYPTION_LEVEL_NO_ENTRY_m13 for maximum_record_encryption_level unless the creator knows it (0 = no passwords / all readable, or a fixed policy)
void			G_set_error_exec_m13(const si1 *function, si4 line, si4 code, const si1 *message, ...); // vararg(code == E_SIG_m13): si4 sig_num (followed by optional formatting string values)
tern			G_set_session_globals_m13(void *level_header, const si1 *MED_path, const si1 *password);
void			G_set_signal_traps_m13(tern set);
tern			G_set_time_constants_m13(void *level_header, TIMEZONE_INFO_m13 *timezone_info, si8 session_start_time, tern prompt); // level resolves the pg whose constants are set; NULL => current thread (the single-session case; multiple per-pg constant sets are new in m13)
Sgmt_REC_m13		*G_Sgmt_records_m13(void *level_header, si4 search_mode);
ui4			G_Sgmt_records_source_m13(void *level_header, Sgmt_REC_m13 *Sgmt_recs);
tern			G_show_behavior_m13(ui4 behavior_code);
tern			G_show_contigua_m13(void *level_header);
tern			G_show_daylight_change_code_m13(DAYLIGHT_TIME_CHANGE_CODE_m13 *code, const si1 *prefix);
tern			G_show_error_m13(void);
tern			G_show_file_times_m13(FILE_TIMES_m13 *ft);
si4			G_show_function_stack_m13(pid_t_m13 _id);
void			G_show_globals_m13(void);
tern			G_show_level_header_m13(void *level_header);
tern			G_show_level_header_flags_m13(ui8 flags);
tern			G_show_location_info_m13(LOCATION_INFO_m13 *li);
void			G_show_lock_m13(FLOCK_ENTRY_m13 *lock);
tern			G_show_metadata_m13(FPS_m13 *fps, METADATA_m13 *md, ui4 type_code);
tern			G_show_password_data_m13(PASSWORD_DATA_m13 *pwd, si1 pw_level);
tern			G_show_password_hints_m13(PASSWORD_DATA_m13 *pwd, si1 pw_level);
tern			G_show_proc_globs_m13(void *level_header);
tern			G_show_records_m13(FPS_m13 *rec_data_fps, si4 *record_filters);
tern			G_show_Sgmt_records_m13(void *level_header, Sgmt_REC_m13 *Sgmt);
tern 			G_show_slice_m13(SLICE_m13 *slice);
tern			G_show_timezone_info_m13(const TIMEZONE_INFO_m13 *timezone_entry, tern show_DST_detail);
tern			G_show_universal_header_m13(FPS_m13 *fps, UH_m13 *uh);
void			G_signal_trap_m13(si4 sig_num);
tern			G_sort_channels_by_acq_num_m13(SESS_m13 *sess);
tern			G_rec_inds_ordered_m13(REC_IDX_m13 *inds, si8 n_recs); // backward abort-early order scan (TIME_NO_ENTRY counts as unordered)
tern			G_sort_records_m13(FPS_m13 *rec_inds_fps, FPS_m13 *rec_data_fps);
tern			G_swap_names_m13(void *level_header);
// returns TRUE_m13 if an operator responded (including an empty entry accepting the default),
// UNKNOWN_m13 if input was exhausted & the default was applied with nobody present, FALSE_m13 on
// error or a required entry with no input.  Prompts guarding irreversible actions must test for
// UNKNOWN_m13 & take the conservative branch rather than trusting the default.
tern			G_terminal_entry_m13(const si1 *prompt, si1 type, void *buffer, void *default_input, tern required, tern validate);
tern			G_terminal_password_bytes_m13(const si1 *password, si1 *password_bytes);
tern			G_ternary_entry_m13(const si1 *entry);
tern			G_textbelt_text_m13(const si1 *phone_number, const si1 *content, const si1 *textbelt_key);
void			G_thread_exit_m13(void);
si8			G_time_for_index_m13(void *level_header, si8 target_index, ui4 mode, ...);  // varargs(level_header == NULL): si8 ref_index, si8 ref_time, sf8 rate
void			G_update_access_time_m13(void *level_header);
tern			G_update_channel_name_m13(CHAN_m13 *chan);
tern			G_update_channel_name_header_m13(const si1 *path, const si1 *fs_name);
tern			G_update_MED_type_m13(void *level_header, const si1 *path, REKEY_CTX_m13 *rk); // used by G_update_MED_version_m13(); level_header = any lh of the session being updated; rk = re-key context (NULL = no re-key)
tern			G_update_MED_version_m13(FPS_m13 *fps);
tern			G_update_session_name_m13(FPS_m13 *fps);
tern			G_update_session_name_header_m13(const si1 *fs_path, const si1 *fs_name, const si1 *uh_name); // used by G_update_session_name_m13()
tern			G_valid_file_code_m13(ui4 type_code);
tern			G_valid_level_code_m13(ui4 type_code);
tern			G_valid_name_m13(const si1 *name); // session/channel/segment base names; rejects the reserved "parity" prefix (for writers, at name creation time)
tern			G_valid_tern_m13(tern *val);
tern			G_validate_record_data_CRCs_m13(FPS_m13 *fps);
tern			G_validate_time_series_data_CRCs_m13(FPS_m13 *fps);
tern			G_validate_video_data_CRCs_m13(FPS_m13 *fps);
void			G_warning_message_m13(const si1 *fmt, ...) FMT_ATTR_m13(1, 2);
void			G_write_medlibrc_m13(const si1 *path);



//**********************************************************************************//
// MARK: Windows Specific Functions (WN)
//**********************************************************************************//

#ifdef WINDOWS_m13

// function typedefs for NTdll dylib()

typedef struct _IO_STATUS_BLOCK {
	union {
		NTSTATUS Status;
		PVOID Pointer;
	};
	ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef enum _FILE_INFORMATION_CLASS {
	FileDirectoryInformation = 1,
	FileFullDirectoryInformation = 2,
	FileBothDirectoryInformation = 3,
	FileBasicInformation = 4,  // supported by WN_query_file_information_m13()
	FileStandardInformation = 5,
	FileInternalInformation = 6,
	FileEaInformation = 7,
	FileAccessInformation = 8,  // supported by WN_query_file_information_m13()
	FileNameInformation = 9,
	FileRenameInformation = 10,
	FileLinkInformation = 11,
	FileNamesInformation = 12,
	FileDispositionInformation = 13,
	FilePositionInformation = 14,
	FileFullEaInformation = 15,
	FileModeInformation = 16,
	FileAlignmentInformation = 17,
	FileAllInformation = 18,
	FileAllocationInformation = 19,
	FileEndOfFileInformation = 20,
	FileAlternateNameInformation = 21,
	FileStreamInformation = 22,
	FilePipeInformation = 23,
	FilePipeLocalInformation = 24,
	FilePipeRemoteInformation = 25,
	FileMailslotQueryInformation = 26,
	FileMailslotSetInformation = 27,
	FileCompressionInformation = 28,
	FileObjectIdInformation = 29,
	FileCompletionInformation = 30,
	FileMoveClusterInformation = 31,
	FileQuotaInformation = 32,
	FileReparsePointInformation = 33,
	FileNetworkOpenInformation = 34,
	FileAttributeTagInformation = 35,
	FileTrackingInformation = 36,
	FileIdBothDirectoryInformation = 37,
	FileIdFullDirectoryInformation = 38,
	FileValidDataLengthInformation = 39,
	FileShortNameInformation = 40,
	FileIoCompletionNotificationInformation = 41,
	FileIoStatusBlockRangeInformation = 42,
	FileIoPriorityHintInformation = 43,
	FileSfioReserveInformation = 44,
	FileSfioVolumeInformation = 45,
	FileHardLinkInformation = 46,
	FileProcessIdsUsingFileInformation = 47,
	FileNormalizedNameInformation = 48,
	FileNetworkPhysicalNameInformation = 49,
	FileIdGlobalTxDirectoryInformation = 50,
	FileIsRemoteDeviceInformation = 51,
	FileUnusedInformation = 52,
	FileNumaNodeInformation = 53,
	FileStandardLinkInformation = 54,
	FileRemoteProtocolInformation = 55,
	FileRenameInformationBypassAccessCheck = 56,
	FileLinkInformationBypassAccessCheck = 57,
	FileVolumeNameInformation = 58,
	FileIdInformation = 59,
	FileIdExtdDirectoryInformation = 60,
	FileReplaceCompletionInformation = 61,
	FileHardLinkFullIdInformation = 62,
	FileIdExtdBothDirectoryInformation = 63,
	FileDispositionInformationEx = 64,
	FileRenameInformationEx = 65,
	FileRenameInformationExBypassAccessCheck = 66,
	FileDesiredStorageClassInformation = 67,
	FileStatInformation = 68,
	FileMemoryPartitionInformation = 69,
	FileStatLxInformation = 70,
	FileCaseSensitiveInformation = 71,
	FileLinkInformationEx = 72,
	FileLinkInformationExBypassAccessCheck = 73,
	FileStorageReserveIdInformation = 74,
	FileCaseSensitiveInformationForceAccessCheck = 75,
	FileKnownFolderInformation = 76,
	FileStatBasicInformation = 77,
	FileId64ExtdDirectoryInformation = 78,
	FileId64ExtdBothDirectoryInformation = 79,
	FileIdAllExtdDirectoryInformation = 80,
	FileIdAllExtdBothDirectoryInformation = 81,
	FileStreamReservationInformation,
	FileMupProviderInfo,
	FileMaximumInformation
} FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;

typedef struct _FILE_BASIC_INFORMATION {
	LARGE_INTEGER	CreationTime;
	LARGE_INTEGER	LastAccessTime;
	LARGE_INTEGER	LastWriteTime;
	LARGE_INTEGER	ChangeTime;
	ULONG		FileAttributes;
} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;

typedef DWORD	ACCESS_MASK;

typedef struct _FILE_ACCESS_INFORMATION {
  ACCESS_MASK	AccessFlags;
} FILE_ACCESS_INFORMATION, *PFILE_ACCESS_INFORMATION;

// function typedef for WN_query_information_file_m13()
typedef HRESULT (CALLBACK* NTQUERYINFOFILETYPE)(HANDLE, IO_STATUS_BLOCK *, PVOID, ULONG, FILE_INFORMATION_CLASS);

// function typedefs for WN_sleep_m13()
typedef HRESULT (CALLBACK* ZWSETTIMERRESTYPE)(ULONG, BOOLEAN, ULONG *);
typedef HRESULT (CALLBACK* NTDELAYEXECTYPE)(BOOLEAN, LARGE_INTEGER *);

// Prototypes
tern		WN_cleanup_m13(void);
tern		WN_clear_m13(void);
si8		WN_date_to_uutc_m13(sf8 date);
si8		WN_filetime_to_uutc_m13(ui1 *win_filetime); // for conversion of windows file time to uutc on any platform
tern		WN_init_terminal_m13(void);
si4 		WN_ls_1d_to_buf_m13(const si1 **dir_strs, si4 n_dirs, tern full_path, si1 **buffer);
si4		WN_ls_1d_to_tmp_m13(const si1 **dir_strs, si4 n_dirs, tern full_path, si1 *temp_file);
void		WN_nap_m13(ui8 ns);  // total nanoseconds (no timespec: UCRT tv_nsec is 4 bytes)
void		*WN_query_information_file_m13(void *fp, si4 info_class, void *fi);
tern		WN_reset_terminal_m13(void);
tern		WN_socket_startup_m13(void);
si4		WN_system_m13(const si1 *command);
si8		WN_time_to_uutc_m13(FILETIME win_time);
sf8		WN_uutc_to_date_m13(si8 uutc);
FILETIME	WN_uutc_to_win_time_m13(si8 uutc);
tern		WN_windify_file_paths_m13(si1 *target, const si1 *source);
si1		*WN_windify_format_string_m13(const si1 *fmt);
#endif // WINDOWS_m13



//**********************************************************************************//
// MARK: MED Alignment Checking (ALCK)
//**********************************************************************************//

// Prototypes



//**********************************************************************************//
// MARK: Allocation Tracking (AT)
//**********************************************************************************//

#ifdef AT_DEBUG_m13

// NOTE: The AT system keeps track all allocated & freed memory blocks, so the list can grow continuously & may appear like a a very slow memory leak
// Previously freed memory blocks are not replaced in the list so in the case of an attempted double free, it can inform where the block was previously freed.

// Prototypes
ui8	AT_actual_size_m13(void *address);
tern	AT_add_entry_m13(const si1 *function, si4 line, void *address, size_t requested_bytes);
tern	AT_freeable_m13(void *address);
tern	AT_remove_entry_m13(const si1 *function, si4 line, void *address);
ui8	AT_requested_size_m13(void *address);
void	AT_show_entries_m13(void);
	void	AT_show_entry_m13(void *address);
tern	AT_update_entry_m13(const si1 *function, si4 line, void *orig_address, void *new_address, size_t requested_bytes);

// AT replacement functions for alloc standard functions (these do not need to be called directly)
void	*AT_aligned_alloc_m13(const si1 *function, si4 line, si8 alignment, si8 n_bytes);
void	AT_aligned_free_m13(const si1 *function, si4 line, void *ptr);
void	*AT_calloc_m13(const si1 *function, si4 line, size_t n_members, si8 el_size);
void	**AT_calloc_2D_m13(const si1 *function, si4 line, size_t dim1, size_t dim2, si8 el_size);
void	AT_free_m13(const si1 *function, si4 line, void *ptr);
void	AT_free_all_m13(const si1 *function, si4 line);
void	AT_free_2D_m13(const si1 *function, si4 line, void **ptr, size_t dim1);
void	*AT_malloc_m13(const si1 *function, si4 line, si8 n_bytes);
void	**AT_malloc_2D_m13(const si1 *function, si4 line, size_t dim1, si8 dim2_bytes);
void	*AT_realloc_m13(const si1 *function, si4 line, void *ptr, si8 n_bytes);
void	**AT_realloc_2D_m13(const si1 *function, si4 line, void **ptr, size_t curr_dim1, size_t new_dim2, size_t curr_dim2_bytes, si8 new_dim2_bytes);
void	*AT_recalloc_m13(const si1 *function, si4 line, void *ptr, size_t curr_members, size_t new_members, si8 el_size);
void	**AT_recalloc_2D_m13(const si1 *function, si4 line, void **ptr, size_t curr_dim1, size_t new_dim1, size_t curr_dim2, size_t new_dim2, si8 el_size);

// preprocessor directives to replace standard alloc functions with AT versions
#define aligned_alloc_m13(a, b)			AT_aligned_alloc_m13(__FUNCTION__, __LINE__, a, b)
#define aligned_free_m13(a)			AT_aligned_free_m13(__FUNCTION__, __LINE__, a)
#define calloc_m13(a, b)			AT_calloc_m13(__FUNCTION__, __LINE__, a, b)
#define calloc_2D_m13(a, b, c)			AT_calloc_2D_m13(__FUNCTION__, __LINE__, a, b, c)
#define free_m13(a)				AT_free_m13(__FUNCTION__, __LINE__, a)
#define free_all_m13()				AT_free_all_m13(__FUNCTION__, __LINE__)
#define free_2D_m13(a, b)			AT_free_2D_m13(__FUNCTION__, __LINE__, a, b)
#define malloc_m13(a)				AT_malloc_m13(__FUNCTION__, __LINE__, a)
#define malloc_2D_m13(a, b)			AT_malloc_2D_m13(__FUNCTION__, __LINE__, a, b)
#define realloc_m13(a, b)			AT_realloc_m13(__FUNCTION__, __LINE__, a, b)
#define realloc_2D_m13(a, b, c, d, e)		AT_realloc_2D_m13(__FUNCTION__, __LINE__, a, b, c, d, e)
#define recalloc_m13(a, b, c, d)		AT_recalloc_m13(__FUNCTION__, __LINE__, a, b, c, d)
#define recalloc_2D_m13(a, b, c, d, e, f)	AT_recalloc_2D_m13(__FUNCTION__, __LINE__, a, b, c, d, e, f)

#endif // AT_DEBUG_m13



//**********************************************************************************//
// MARK: Strings (STR)
//**********************************************************************************//

// Prototypes
// NOTE: string args declared as void pointers are standard strings or pstrs, freely mixed (disambiguated by tag)
// pstr inputs: lengths & character counts read from structure (no scan); pstr outputs: grown as required (cannot overflow), counts maintained
// output functions returning void * return the output arg as passed (or an allocated standard string if the output arg was passed NULL)
void		*STR_bin_m13(void *str, void *num_ptr, size_t num_bytes, const void *byte_separator, tern numeric_order);
const si1	*STR_bool_m13(ui8 val, tern colored);
wchar_t		*STR_char2wchar_m13(wchar_t *target, const void *source);
ui4		STR_check_spaces_m13(const void *string);
si4		STR_compare_m13(const void *a, const void *b); // qsort() comparator: elements are standard string or pstr pointers, freely mixed
tern		STR_contains_formatting_m13(const void *string, void *plain_string);
tern		STR_contains_regex_m13(const void *string);
void 		*STR_duration_m13(void *dur_str, si8 int_usecs, tern abbreviated, tern two_level);
tern		STR_escape_chars_m13(void *string, si1 target_char, si8 buffer_len); // buffer_len ignored for pstrs (msize authoritative)
void		*STR_fixed_width_int_m13(void *string, si4 string_bytes, si8 number);
void		*STR_hex_m13(void *str, void *num_ptr, si8 num_bytes, const void *byte_separator, tern numeric_order);
tern		STR_is_empty_m13(const void *string); // O(1) for pstrs
si1		*STR_match_end_m13(const void *pattern, const void *buffer); // returns position within buffer string
si1		*STR_match_end_bin_m13(const void *pattern, const si1 *buffer, si8 buf_len); // buffer is raw bytes (may contain zeros), not a string
si1		*STR_match_line_end_m13(const void *pattern, const void *buffer); // returns position within buffer string
si1		*STR_match_line_start_m13(const void *pattern, const void *buffer); // returns position within buffer string
si1		*STR_match_start_m13(const void *pattern, const void *buffer); // returns position within buffer string
si1		*STR_match_start_bin_m13(const void *pattern, const si1 *buffer, si8 buf_len); // buffer is raw bytes (may contain zeros), not a string
void 		*STR_re_escape_m13(const void *str, void *esc_str);
tern 		STR_replace_char_m13(si1 c, si1 new_c, void *buffer);
void		*STR_replace_pattern_m13(const void *pattern, const void *new_pattern, void *buffer, void *new_buffer);
void		*STR_size_m13(void *size_str, si8 n_bytes, tern base_two);
tern		STR_sort_m13(void *string_array, si8 n_strings); // elements are standard string or pstr pointers, freely mixed
tern		STR_strip_character_m13(void *string, si1 character);
const si1	*STR_tern_m13(tern val, tern colored);
void		*STR_time_m13(void *level_header, si8 uutc_time, void *time_str, tern fixed_width, tern relative_days, si4 colored_text, ...);  // varargs(colored_text == TRUE_m13): si1 *date_color, si1 *time_color e.g. TC_BLUE_m12, TC_GREEN_m13; Note: time_str buffer sould be of length TIME_STRING_BYTES_m13
tern		STR_to_lower_m13(void *string);
tern		STR_to_title_m13(void *string);
tern		STR_to_upper_m13(void *string);
tern		STR_unescape_chars_m13(void *string, si1 target_char);
void		*STR_wchar2char_m13(void *target, const wchar_t *source);



//**********************************************************************************//
// MARK: Compression & Computation (CMP)
//**********************************************************************************//

// CMP: Miscellaneous Constants
#define CMP_SAMPLE_VALUE_NO_ENTRY_m13		NAN_SI4_m13
#define CMP_SPLINE_TAIL_LEN_m13			6
#define CMP_SPLINE_UPSAMPLE_SF_RATIO_m13	((sf8) 3.0)
#define CMP_MAK_PAD_SAMPLES_m13			3
#define CMP_MAK_INPUT_BUFFERS_m13		8
#define CMP_MAK_IN_Y_BUF			0
#define CMP_MAK_IN_X_BUF			1
#define CMP_MAK_OUTPUT_BUFFERS_m13		4
#define CMP_MAK_OUT_Y_BUF			0
#define CMP_MAK_OUT_X_BUF			1
#define CMP_VDS_INPUT_BUFFERS_m13		(CMP_MAK_INPUT_BUFFERS_m13 + 6)  // +1 template[8], +2 dirty-interval scratch[9,10], +3 defer scratch[11,12,13]
#define CMP_VDS_DIRTY_LO_BUF_m13		9  // VDS refine: dirty-interval lo bounds (local re-interpolation)
#define CMP_VDS_DIRTY_HI_BUF_m13		10 // VDS refine: dirty-interval hi bounds
#define CMP_VDS_CAND_BUF_m13			11 // VDS defer refine: candidate list (also reused as raw interval list for merge)
#define CMP_VDS_DEFER_BUF_m13			12 // VDS defer refine: deferred-candidate sample positions (carried forward)
#define CMP_VDS_ACCEPT_BUF_m13			13 // VDS defer refine: accepted candidate anchor-indices (blast-exclusion scan)
#define CMP_VDS_DEFER_DENSITY_SHIFT_m13		6  // use defer refine when post-spurious anchors <= (block_samps >> 6) (~1.5%); denser => local (defer's extra rounds not worth ~3% fewer anchors)
#define CMP_VDS_BLAST_COUPLING_m13		3  // Akima coupling radius: inserting near anchor i perturbs the fit for anchors within +-3
#define CMP_VDS_OUTPUT_BUFFERS_m13		CMP_MAK_OUTPUT_BUFFERS_m13
#define CMP_VDS_LOWPASS_ORDER_m13		6
#define CMP_VDS_MINIMUM_SAMPLES_m13		10
#define CMP_SRRED_BOTTOM_SCALE_m13		((sf8) 0.005) // minimum search scale (scale search starts here)
#define CMP_SRRED_TOP_SCALE_m13			((sf8) 0.995) // maximum search scale (usually exits well before here)
#define CMP_SRRED_SCALE_STEP_m13		((sf8) 0.0001) // scale search increment
#define CMP_SRRED_MINIMUM_SAMPLES_m13		((ui4) 10) // minimum block samples to measure SRRED parameters (fewer => block falls through to MBE). Currently equals CMP_VDS_MINIMUM_SAMPLES_m13, but defined separately so SRRED & VDS can diverge.
#define CMP_SRRED_RANK_SCALE_m13		((sf8) 0.2) // fixed scale at which SRRED ranks derivative levels (the derivative level that is best AT the scale SRRED will use, not at unscaled scale 1.0)
// SRRED scale-search parameters. These are DEFAULTS for the per-CPS settable fields params.SRRED_scale_window /
// _scale_refresh / _scale_bailout_mult (initialized to these in CMP_init_CPS_parameters). Override the fields directly,
// or apply a preset "set" below. Larger window / smaller refresh / smaller bailout => more thorough scan (better ratio,
// slower); the thorough end is estimator-bound, so it is exactly what the log table (CMP_log_table) accelerates.
#define CMP_SRRED_SCALE_WINDOW_DEFAULT_m13		((si4) 100) // windowed-tracker half-width in scale steps (0 => full anchor scan every block). Edge-expands if the min lands on a boundary; empirically within +/-0.1% of the full-scan optimum.
#define CMP_SRRED_SCALE_REFRESH_DEFAULT_m13		((si8) 128) // blocks between forced full-range anchor scans (catches drift/jumps the local window misses)
#define CMP_SRRED_OVF_HIST_MAX_SPAN_m13			((si8) 1 << 20) // widest overflow value range the scan's per-block value histogram will cover (4 MB of ui4 bins); wider-range blocks fall back to the raw overflow walk
#define CMP_SRRED_SCALE_BAILOUT_MULT_DEFAULT_m13	((sf8) 2.5) // anchor-scan early-exit: keep scanning until scale exceeds this x the best-so-far, then bail (0.0 => no bailout, true full scan). 2.5: ~22-25x faster than full scan for <=0.02% size cost, with margin above the loss cliff near 1.5-2.0
#define CMP_SRRED_SCALE_BAILOUT_SPAN_m13		((sf8) 0.05) // minimum ABSOLUTE span past the best-so-far before bailing: exit = max(mult x scale, scale + span). A multiplicative window alone is very narrow at small scales (2.5 x 0.01 spans just 0.025), so a shallow shelf of local minima at low scale can arm the bailout & exit before the true optimum is ever reached (measured: 0.8% on real data). 0.05 = 2x the largest shelf-to-optimum-basin bridge observed across the 8 development datasets (0.027)
// preset sets (assign all three fields together):
#define CMP_SRRED_SCALE_WINDOW_MAX_COMPRESSION_m13		((si4) 0)   // window 0 + bailout 0.0 => an exhaustive full scale scan every block. Estimator-bound, so the log table makes it affordable; spends compute for a bit more ratio.
#define CMP_SRRED_SCALE_REFRESH_MAX_COMPRESSION_m13		((si8) 1)
#define CMP_SRRED_SCALE_BAILOUT_MULT_MAX_COMPRESSION_m13	((sf8) 0.0)
#define CMP_SRRED_SCALE_WINDOW_MAX_SPEED_m13			((si4) 30)  // narrow tracking window + infrequent re-anchoring => fastest; keeps margin above the ~20-step fidelity cliff on volatile data
#define CMP_SRRED_SCALE_REFRESH_MAX_SPEED_m13			((si8) 512)
#define CMP_SRRED_SCALE_BAILOUT_MULT_MAX_SPEED_m13		((sf8) 2.5)
#define CMP_SRRED_SCRAP_BUFFERS_m13		2
#define CMP_RED_TO_PRED_m13			TRUE_m13 // for CMP_swap_RED_PRED_m13()
#define CMP_PRED_TO_RED_m13			FALSE_m13 // for CMP_swap_RED_PRED_m13()
#define CMP_RED_PRED_SWAP_m13			UNKNOWM_m13 // for CMP_swap_RED_PRED_m13()

// CMP: Block Fixed Header Offset Constants
#define CMP_BLOCK_FIXED_HDR_BYTES_m13				56 // fixed region only
#define CMP_BLOCK_START_UID_m13					((ui8) 0x0123456789ABCDEF) // ui8 (decimal 81,985,529,216,486,895); (note this is same as PCRC_TAG_m13 - benign mistake)
#define CMP_BLOCK_START_UID_OFFSET_m13				0
#define CMP_BLOCK_CRC_OFFSET_m13				8 // ui4
#define CMP_BLOCK_CRC_NO_ENTRY_m13				CRC_NO_ENTRY_m13
#define CMP_BLOCK_BLOCK_FLAGS_OFFSET_m13			12 // ui4
#define CMP_BLOCK_BLOCK_FLAGS_NO_ENTRY_m13			0
#define CMP_BLOCK_CRC_START_OFFSET_m13				CMP_BLOCK_BLOCK_FLAGS_OFFSET_m13
#define CMP_BLOCK_START_TIME_OFFSET_m13				16 // si8
#define CMP_BLOCK_START_TIME_NO_ENTRY_m13			TIME_NO_ENTRY_m13
#define CMP_BLOCK_ACQUISITION_CHANNEL_NUMBER_OFFSET_m13		24 // si4
#define CMP_BLOCK_ACQUISITION_CHANNEL_NUMBER_NO_ENTRY_m13	-1
#define CMP_BLOCK_TOTAL_BLOCK_BYTES_OFFSET_m13			28 // ui4
#define CMP_BLOCK_TOTAL_BLOCK_BYTES_NO_ENTRY_m13		0
// CMP Block Encryption Start
#define CMP_BLOCK_NUMBER_OF_SAMPLES_OFFSET_m13			32 // ui4
// BLOCK ENCRYPTION MAP (reworked 2026-08-05). TWO DISJOINT encrypted regions, with a 4-byte hole:
//   [  0 ..  36)  CLEAR - UID, block_CRC, block_flags, start_time, acquisition_channel_number,
//                         total_block_bytes, number_of_samples
//   [ 36 ..  52)  AES   - exactly ONE block: number_of_records, record_region_bytes, parameter_flags &
//                         the four region-size fields
//   [ 52 ..  56)  CLEAR - total_header_bytes. The decryptor sizes the span below from it, so it cannot
//                         itself be encrypted. This is the hole, & it is the whole reason for the split.
//   [ 56 ..   N)  AES   - records / parameter / protected / discretionary / model regions + compressed
//                         data. N is per algorithm - see CMP_crypt_block_m13().
//
// Until 2026-08-05 encryption simply began at CMP_BLOCK_NUMBER_OF_SAMPLES_OFFSET_m13 (32), which put
// total_header_bytes INSIDE the region it describes: decrypt read it as ciphertext, got a huge value,
// clamped to the whole payload & "decrypted" plaintext into garbage. Every encrypted RED/PRED block was
// silently corrupt (measured: 24,550 of 40,000 samples wrong, no error raised). MBE escaped only because
// its rule keys off block_flags, which was already in the clear.
//
// number_of_samples is left in the clear DELIBERATELY: the time series index publishes it already (the
// deltas between consecutive start_samp_num entries of an unencrypted .tidx), so encrypting it bought
// nothing - and leaving it out is what makes the encrypted header region exactly one AES block, with no
// ciphertext stealing in a short region.
//
// Safe to change because no encrypted MED data exists in the wild (Matt, 2026-08-05). No FIELD MOVES, so
// every existing unencrypted file is untouched & no updater pass is needed - which is what decided it: the
// real corpus is multi-week, hundreds-of-channel datasets whose every block would otherwise be rewritten
// (& the block CRC covers offset 12 onward, so moving a field invalidates every block CRC too).
#define CMP_BLOCK_ENCRYPTION_HDR_START_OFFSET_m13		CMP_BLOCK_NUMBER_OF_RECORDS_OFFSET_m13 // 36
#define CMP_BLOCK_ENCRYPTION_HDR_BYTES_m13			(CMP_BLOCK_TOTAL_HEADER_BYTES_OFFSET_m13 - CMP_BLOCK_ENCRYPTION_HDR_START_OFFSET_m13) // 16 == ENCRYPTION_BLOCK_BYTES_m13: exactly one AES block
#define CMP_BLOCK_ENCRYPTION_START_OFFSET_m13			CMP_BLOCK_RECORDS_REGION_OFFSET_m13 // 56
// round a byte count up to a whole number of AES blocks: SRRED & VDS encrypt in two stages, & both stages
// must land on block boundaries or ciphertext stealing would apply at a different point in each direction
#define CMP_AES_ROUND_UP_m13(bytes)				((((si8) (bytes)) + (ENCRYPTION_BLOCK_BYTES_m13 - 1)) & ~((si8) (ENCRYPTION_BLOCK_BYTES_m13 - 1)))
#define CMP_BLOCK_NUMBER_OF_SAMPLES_NO_ENTRY_m13		0xFFFFFFFF
#define CMP_BLOCK_NUMBER_OF_RECORDS_OFFSET_m13			36 // ui2
#define CMP_BLOCK_NUMBER_OF_RECORDS_NO_ENTRY_m13		0xFFFF
#define CMP_BLOCK_RECORD_REGION_BYTES_OFFSET_m13		38 // ui2
#define CMP_BLOCK_RECORD_REGION_BYTES_NO_ENTRY_m13		0xFFFF
#define CMP_BLOCK_PARAMETER_FLAGS_OFFSET_m13			40 // ui4
#define CMP_BLOCK_PARAMETER_FLAGS_NO_ENTRY_m13			0
#define CMP_BLOCK_PARAMETER_REGION_BYTES_OFFSET_m13		44 // ui2
#define CMP_BLOCK_PARAMETER_REGION_BYTES_NO_ENTRY_m13		0xFFFF
#define CMP_BLOCK_PROTECTED_REGION_BYTES_OFFSET_m13		46 // ui2
#define CMP_BLOCK_PROTECTED_REGION_BYTES_NO_ENTRY_m13		0xFFFF
#define CMP_BLOCK_DISCRETIONARY_REGION_BYTES_OFFSET_m13		48 // ui2
#define CMP_BLOCK_DISCRETIONARY_REGION_BYTES_NO_ENTRY_m13	0xFFFF
#define CMP_BLOCK_MODEL_REGION_BYTES_OFFSET_m13			50 // ui2
#define CMP_BLOCK_MODEL_REGION_BYTES_NO_ENTRY_m13		0xFFFF
#define CMP_BLOCK_TOTAL_HEADER_BYTES_OFFSET_m13			52 // ui4
#define CMP_BLOCK_TOTAL_HEADER_BYTES_NO_ENTRY_m13		0xFFFF
#define CMP_BLOCK_RECORDS_REGION_OFFSET_m13			56

// CMP: Record Header Offset Constants
#define CMP_REC_HDR_BYTES_m13				8
#define CMP_REC_HDR_TYPE_CODE_OFFSET_m13		0 // ui4
#define CMP_REC_HDR_TYPE_CODE_NO_ENTRY_m13		0
#define CMP_REC_HDR_VERSION_MAJOR_OFFSET_m13		4 // ui1
#define CMP_REC_HDR_VERSION_MAJOR_NO_ENTRY_m13		0xFF
#define CMP_REC_HDR_VERSION_MINOR_OFFSET_m13		5 // ui1
#define CMP_REC_HDR_VERSION_MINOR_NO_ENTRY_m13		0xFF
#define CMP_REC_HDR_TOTAL_BYTES_OFFSET_m13		6 // ui2
#define CMP_REC_HDR_TOTAL_BYTES_NO_ENTRY_m13		0xFFFF // Note maximum CMP record size is 65k - smaller than MED record

// CMP: RED (Range Encoded Derivatives) Model Offset Constants
#define CMP_RED_MODEL_NUMBER_OF_KEYSAMPLE_BYTES_OFFSET_m13 	0 // ui4
#define CMP_RED_MODEL_DERIVATIVE_LEVEL_OFFSET_m13		4 // ui1
#define CMP_RED_MODEL_PAD_OFFSET_m13				5 // ui1[3]
#define CMP_RED_MODEL_NUMBER_OF_STATISTICS_BINS_OFFSET_m13	8  // ui2
#define CMP_RED_MODEL_FLAGS_OFFSET_m13				10 // ui2
#define CMP_RED_MODEL_FIXED_HDR_BYTES_m13			12
// RED Model Flags
#define CMP_RED_FLAGS_NO_ZERO_COUNTS_m13			((ui2) 1 << 0) // bit 0
#define CMP_RED_FLAGS_POSITIVE_DERIVATIVES_m13			((ui2) 1 << 1) // bit 1
#define CMP_RED_FLAGS_2_BYTE_OVERFLOWS_m13				((ui2) 1 << 2) // bit 2
#define CMP_RED_FLAGS_3_BYTE_OVERFLOWS_m13				((ui2) 1 << 3) // bit 3
#define CMP_RED_FLAGS_OVERFLOW_BYTES_MASK_m13				( CMP_RED_FLAGS_2_BYTE_OVERFLOWS_m13 | CMP_RED_FLAGS_3_BYTE_OVERFLOWS_m13 )

// CMP: PRED (Predictive RED) Model Offset Constants
#define CMP_PRED_MODEL_NUMBER_OF_KEYSAMPLE_BYTES_OFFSET_m13 		0 // ui4
#define CMP_PRED_MODEL_DERIVATIVE_LEVEL_OFFSET_m13 			4 // ui1
#define CMP_PRED_MODEL_PAD_OFFSET_m13					5 // ui1[3]
#define CMP_PRED_MODEL_NUMBERS_OF_STATISTICS_BINS_OFFSET_m13		8 // ui2[3]
#define CMP_PRED_MODEL_NUMBER_OF_NIL_STATISTICS_BINS_OFFSET_m13		CMP_PRED_MODEL_NUMBERS_OF_STATISTICS_BINS_OFFSET_m13 // ui2
#define CMP_PRED_MODEL_NUMBER_OF_POS_STATISTICS_BINS_OFFSET_m13		10 // ui2
#define CMP_PRED_MODEL_NUMBER_OF_NEG_STATISTICS_BINS_OFFSET_m13		12 // ui2
#define CMP_PRED_MODEL_FLAGS_OFFSET_m13					14 // ui2
#define CMP_PRED_MODEL_FIXED_HDR_BYTES_m13				16
// PRED Model Flags
#define CMP_PRED_FLAGS_NO_ZERO_COUNTS_m13				((ui2) 1 << 0) // bit 0
#define CMP_PRED_FLAGS_BIT_1_m13					((ui2) 1 << 1) // bit 1 Note: this is used for positive derivatives in RED, left empty here to keep bits same
#define CMP_PRED_FLAGS_2_BYTE_OVERFLOWS_m13					((ui2) 1 << 2) // bit 2
#define CMP_PRED_FLAGS_3_BYTE_OVERFLOWS_m13					((ui2) 1 << 3) // bit 3
#define CMP_PRED_FLAGS_OVERFLOW_BYTES_MASK_m13				( CMP_PRED_FLAGS_2_BYTE_OVERFLOWS_m13 | CMP_PRED_FLAGS_3_BYTE_OVERFLOWS_m13 )

// CMP: SRRED (Scaled + Residuals RED) Model Offset Constants
#define CMP_SRRED_MODEL_SCALED_BLOCK_SCALE_OFFSET_m13			0 // sf4  (note this is independent of block header gradient parameter)
#define CMP_SRRED_MODEL_SCALED_BLOCK_TOTAL_BYTES_OFFSET_m13		4 // ui4
#define CMP_SRRED_MODEL_SCALED_BLOCK_MODEL_BYTES_OFFSET_m13		8 // ui2
#define CMP_SRRED_MODEL_RESIDUALS_BLOCK_MODEL_BYTES_OFFSET_m13 		10 // ui2
#define CMP_SRRED_MODEL_DERIVATIVE_LEVEL_OFFSET_m13 			12 // ui1
#define CMP_SRRED_MODEL_PAD_OFFSET_m13					13 // ui1
#define CMP_SRRED_MODEL_FLAGS_OFFSET_m13				14 // ui2
#define CMP_SRRED_MODEL_FIXED_HDR_BYTES_m13				16
// SRRED Model Flags
#define CMP_SRRED_FLAGS_SCALED_PRED_m13				((ui4) 1 << 0) // bit 0
#define CMP_SRRED_FLAGS_SCALED_MBE_m13				((ui4) 1 << 1) // bit 1
#define CMP_SRRED_FLAGS_RESIDUALS_RED_m13			((ui4) 1 << 2) // bit 2
#define CMP_SRRED_FLAGS_RESIDUALS_MBE_m13			((ui4) 1 << 3) // bit 3
#define CMP_SRRED_FLAGS_SCALED_ALGORITHMS_MASK_m13			( CMP_SRRED_FLAGS_SCALED_PRED_m13 | CMP_SRRED_FLAGS_SCALED_MBE_m13 )
#define CMP_SRRED_FLAGS_RESIDUALS_ALGORITHMS_MASK_m13			( CMP_SRRED_FLAGS_RESIDUALS_RED_m13 | CMP_SRRED_FLAGS_RESIDUALS_MBE_m13 )
#define CMP_SRRED_FLAGS_ALGORITHMS_MASK_m13				( CMP_SRRED_FLAGS_SCALED_ALGORITHMS_MASK_m13 | CMP_SRRED_FLAGS_RESIDUALS_ALGORITHMS_MASK_m13 )

// CMP: SSE (Symbol Sequence Encoding) Model Offset Constants
#define CMP_SSE_MODEL_NUMBER_OF_KEYSAMPLE_BYTES_OFFSET_m13	0 // ui4
#define CMP_SSE_MODEL_DERIVATIVE_LEVEL_OFFSET_m13		4 // ui1
#define CMP_SSE_MODEL_PAD_BYTES_OFFSET_m13			5 // ui1[3]
#define CMP_SSE_MODEL_NUMBER_OF_STATISTICS_BINS_OFFSET_m13	8 // ui2
#define CMP_SSE_MODEL_FLAGS_OFFSET_m13				10 // ui2
#define CMP_SSE_MODEL_FIXED_HDR_BYTES_m13			12
// SSE Model Flags
#define CMP_SSE_FLAGS_BIT_0_m13						((ui2) 1 << 0) // bit 0 Note: this is left empty to keep bits same as RED & PRED
#define CMP_SSE_FLAGS_BIT_1_m13						((ui2) 1 << 1) // bit 1 Note: this is left empty to keep bits same as RED & PRED
#define CMP_SSE_2_BYTE_OVERFLOWS_MASK_m13				((ui2) 1 << 2)		// bit 2
#define CMP_SSE_3_BYTE_OVERFLOWS_MASK_m13				((ui2) 1 << 3)		// bit 3
#define CMP_SSE_OVERFLOW_BYTES_MASK_m13					(CMP_SSE_2_BYTE_OVERFLOWS_MASK_m13 | CMP_SSE_3_BYTE_OVERFLOWS_MASK_m13)

// CMP: MBE (Minimal Bit Encoding) Model Offset Constants
#define CMP_MBE_MODEL_MINIMUM_VALUE_OFFSET_m13			0 // si4
#define CMP_MBE_MODEL_BITS_PER_SAMPLE_OFFSET_m13		4 // ui1
#define CMP_MBE_MODEL_DERIVATIVE_LEVEL_OFFSET_m13 		5 // ui1
#define CMP_MBE_MODEL_FLAGS_OFFSET_m13				6 // ui2
#define CMP_MBE_MODEL_FIXED_HDR_BYTES_m13			8
// MBE Model Flags
// RETIRED 2026-08-02: this handoff moved to CPS_PARAMS_m13.MBE_preprocessed. It was read out of the block
// buffer, which is realloc'd & never zeroed, so a stale bit could send MBE down the "already preprocessed"
// path with garbage lmin/derivative_level/bits_per_sample. Macro kept only for CMP_show_block_m13().
#define CMP_MBE_FLAGS_PREPROCESSED_MASK_m13		((ui2) 1 << 0) // bit 0 - no longer written

// CMP: VDS (Variably Decimated Stream) Model Offset Constants
#define CMP_VDS_MODEL_NUMBER_OF_VDS_SAMPLES_OFFSET_m13		0 // ui4
#define CMP_VDS_MODEL_AMPLITUDE_BLOCK_TOTAL_BYTES_OFFSET_m13	4 // ui4
#define CMP_VDS_MODEL_AMPLITUDE_BLOCK_MODEL_BYTES_OFFSET_m13	8 // ui2
#define CMP_VDS_MODEL_TIME_BLOCK_MODEL_BYTES_OFFSET_m13 	10 // ui2
#define CMP_VDS_MODEL_FLAGS_OFFSET_m13				12 // ui4 (more options for VDS)
#define CMP_VDS_MODEL_FIXED_HDR_BYTES_m13			16
// VDS Model Flags
#define CMP_VDS_FLAGS_AMPLITUDE_RED1_m13	((ui4) 1 << 0)  // bit 0
#define CMP_VDS_FLAGS_AMPLITUDE_PRED1_m13	((ui4) 1 << 1)  // bit 1
#define CMP_VDS_FLAGS_AMPLITUDE_MBE_m13		((ui4) 1 << 2)  // bit 2
#define CMP_VDS_FLAGS_AMPLITUDE_RED2_m13	((ui4) 1 << 3)  // bit 3
#define CMP_VDS_FLAGS_AMPLITUDE_PRED2_m13	((ui4) 1 << 4)  // bit 4
#define CMP_VDS_FLAGS_TIME_RED1_m13		((ui4) 1 << 5)  // bit 5
#define CMP_VDS_FLAGS_TIME_PRED1_m13		((ui4) 1 << 6)  // bit 6
#define CMP_VDS_FLAGS_TIME_MBE_m13		((ui4) 1 << 7)  // bit 7
#define CMP_VDS_FLAGS_TIME_RED2_m13		((ui4) 1 << 8)  // bit 8
#define CMP_VDS_FLAGS_TIME_PRED2_m13		((ui4) 1 << 9)  // bit 9
#define CMP_VDS_AMPLITUDE_ALGORITHMS_MASK_m13	( CMP_VDS_FLAGS_AMPLITUDE_RED1_m13 | CMP_VDS_FLAGS_AMPLITUDE_PRED1_m13 | CMP_VDS_FLAGS_AMPLITUDE_MBE_m13 \
						| CMP_VDS_FLAGS_AMPLITUDE_RED2_m13 | CMP_VDS_FLAGS_AMPLITUDE_PRED2_m13 )
#define CMP_VDS_TIME_ALGORITHMS_MASK_m13	( CMP_VDS_FLAGS_TIME_RED1_m13 | CMP_VDS_FLAGS_TIME_PRED1_m13 | CMP_VDS_FLAGS_TIME_MBE_m13 \
						| CMP_VDS_FLAGS_TIME_RED2_m13 | CMP_VDS_FLAGS_TIME_PRED2_m13 )
#define CMP_VDS_ALGORITHMS_MASK_m13		( CMP_VDS_AMPLITUDE_ALGORITHMS_m13 | CMP_VDS_TIME_ALGORITHMS_m13 )

// CMP Block Flag Masks
#define CMP_BF_BLOCK_FLAG_BITS_m13		32
#define CMP_BF_DISCONTINUITY_m13		((ui4) 1 << 0)  // bit 0
#define CMP_BF_ENCRYPTED_m13			((ui4) 1 << 1)  // bit 1 (block is currently encrypted - get level from universal header)
#define CMP_BF_LINE_NOISE_FILTERED_m13		((ui4) 1 << 2)  // bit 2 (line noise filter was applied to this block's samples before encoding;
							// the line-noise score byte, if present, records what was REMOVED, not what remains -
							// filtering may be turned on & off during a recording, so this is per-block truth)
#define CMP_BF_RED1_ENCODING_m13		((ui4) 1 << 8)  // bit 8
#define CMP_BF_PRED1_ENCODING_m13		((ui4) 1 << 9)  // bit 9
#define CMP_BF_MBE_ENCODING_m13			((ui4) 1 << 10)  // bit 10
#define CMP_BF_VDS_ENCODING_m13			((ui4) 1 << 11)  // bit 11
#define CMP_BF_RED2_ENCODING_m13		((ui4) 1 << 12)  // bit 12 (fastest lossless)
#define CMP_BF_PRED2_ENCODING_m13		((ui4) 1 << 13)  // bit 13 (fast lossless; better compression than RED)
#define CMP_BF_SRRED_ENCODING_m13		((ui4) 1 << 14)  // bit 14 (slower lossless; better compression than PRED)
#define CMP_BF_SSE_ENCODING_m13			((ui4) 1 << 15)  // bit 15 (fastest compression; compression ratio highly data-dependent)

#define CMP_BF_ALGORITHMS_MASK_m13		( CMP_BF_RED1_ENCODING_m13 | CMP_BF_PRED1_ENCODING_m13 | CMP_BF_MBE_ENCODING_m13 \
						| CMP_BF_VDS_ENCODING_m13 | CMP_BF_RED2_ENCODING_m13 | CMP_BF_PRED2_ENCODING_m13 \
						| CMP_BF_SRRED_ENCODING_m13 | CMP_BF_SSE_ENCODING_m13 )
// CMP Parameter Map Indices
#define CMP_PF_INTERCEPT_IDX_m13		((ui4) 0) // parameter flags bit 0
#define CMP_PF_GRADIENT_IDX_m13			((ui4) 1) // parameter flags bit 1
// bits 2 & 3 were AMPLITUDE_SCALE / FREQUENCY_SCALE, deleted 2026-08-07 with the block-scaling machinery
// (dev code, never exercised, absent from every file in the wild - SRRED & VDS replace it, and a general
// factor lives in the TS metadata "amplitude_units_conversion_factor"). Noise scores moved down into bit 2:
// they have never been written either (the encoder site was a stub), so nothing in the wild carries the old
// position.
#define CMP_PF_NOISE_SCORES_IDX_m13		((ui4) 2) // parameter flags bit 2
// bits 3-15: PROTECTED - reserved for future library parameters. Bits 16-31 are DISCRETIONARY: an
// application sets them in params.user_parameter_flags & owns their meaning (see CMP_generate_parameter_map_m13()).
#define CMP_PF_FIRST_DISCRETIONARY_IDX_m13	((ui4) 16)

// NOISE SCORES - four independent one-byte scores packed into the single ui4 of parameter bit 2.
// ALL FOUR share one scale & one direction: 0 == clean, CMP_NS_MAX_m13 (254) == maximally degraded,
// CMP_NS_NO_ENTRY_m13 (0xFF) == not computed. A uniform polarity makes max() across the four "worst problem
// in this block" & the sum a usable aggregate, without knowing which score fired. 0xFF is the ONLY indication
// of absence - there is deliberately no separate presence mask, which would be a second source of truth.
// ⚠️ Every score is measured on the RAW block, before detrending or any other transform: they describe the
// RECORDING, not the codec's intermediate state. Anything derived from a codec decision (the derivative
// level, for instance, which the FIND search picks per block) would make the same data score differently
// under different compression directives.
#define CMP_NS_LINE_NOISE_IDX_m13	((ui4) 0) // byte 0: proportion of the high band phase-locked to the mains
#define CMP_NS_ENTROPY_DEFICIT_IDX_m13		((ui4) 1) // byte 1: entropy DEFICIT - information loss (dropout, stuck bits, over-quantization)
#define CMP_NS_NON_NORMALITY_IDX_m13	((ui4) 2) // byte 2: Kolmogorov-Smirnov D
#define CMP_NS_LLP_IDX_m13		((ui4) 3) // byte 3: local linear prediction residual vs white noise
#define CMP_NS_BYTES_m13		4
#define CMP_NS_NO_ENTRY_m13		((ui1) 0xFF)
#define CMP_NS_MAX_m13			((ui1) 254)
#define CMP_NS_ENTROPY_BINS_m13		256 // histogram resolution for the entropy score (see CMP_entropy_deficit_m13())
// selector mask for CMP_noise_scores_m13(): which scores to compute. The three library-computed scores share
// their passes over the data, so asking for all three costs barely more than asking for one.
#define CMP_NS_ENTROPY_DEFICIT_SEL_m13		((ui4) 1 << CMP_NS_ENTROPY_DEFICIT_IDX_m13)
#define CMP_NS_NON_NORMALITY_SEL_m13	((ui4) 1 << CMP_NS_NON_NORMALITY_IDX_m13)
#define CMP_NS_LLP_SEL_m13		((ui4) 1 << CMP_NS_LLP_IDX_m13)
#define CMP_NS_ALL_SEL_m13		( CMP_NS_ENTROPY_DEFICIT_SEL_m13 | CMP_NS_NON_NORMALITY_SEL_m13 | CMP_NS_LLP_SEL_m13 )
#define CMP_NS_SCORE_BYTE_m13(x)	( (ui1) ((((x) >= (sf8) 1.0) ? (sf8) 1.0 : (((x) <= (sf8) 0.0) ? (sf8) 0.0 : (x))) * (sf8) CMP_NS_MAX_m13 + (sf8) 0.5) )

// CMP Parameter Flag Masks
#define CMP_PF_PARAMETER_FLAG_BITS_m13		32
#define CMP_PF_INTERCEPT_m13			((ui4) 1 << CMP_PF_INTERCEPT_IDX_m13) // bit 0
#define CMP_PF_GRADIENT_m13			((ui4) 1 << CMP_PF_GRADIENT_IDX_m13) // bit 1
#define CMP_PF_NOISE_SCORES_m13			((ui4) 1 << CMP_PF_NOISE_SCORES_IDX_m13) // bit 2

// PARAMETER MAP. The block's parameter region holds ONLY the parameters whose bits are set - reserving all 32
// slots would waste 120+ bytes on a block that uses one or two - so block_parameter_map[] translates a
// parameter's bit INDEX into its offset within that densely packed region.
// Entries for parameters ABSENT from this block hold CMP_PF_MAP_NO_ENTRY_m13. Before 2026-08-07 they were
// simply not written, so they kept whatever the PREVIOUS block left there: an absent parameter read back a
// stale but entirely plausible offset into a DIFFERENT parameter, silently. (Demonstrated: a block carrying
// only noise scores still had map[INTERCEPT] == 0 from an earlier detrended block, so an intercept read
// returned the noise-score word reinterpreted as si4.) With the sentinel the MAP alone answers "is this
// parameter here?" - the caller no longer has to consult parameter_flags first and remember to.
#define CMP_PF_MAP_NO_ENTRY_m13			((ui4) 0xFFFFFFFF)

// Access a block parameter. Parameters are 4 bytes each & block_parameters is a ui4 *, so the map offset
// advances in 4-byte units; the type cast is centralized HERE rather than re-derived at every call site
// (where "(sf4 *) params + offset" only works because sizeof(sf4) == sizeof(ui4)).
#define CMP_BLOCK_PARAM_PRESENT_m13(cps, idx)	( (cps)->params.block_parameter_map[idx] != CMP_PF_MAP_NO_ENTRY_m13 )
#define CMP_BLOCK_PARAM_m13(cps, idx, type)	( *(type *) ((cps)->block_parameters + (cps)->params.block_parameter_map[idx]) )

// Compression Modes
#define CMP_COMPRESSION_MODE_NO_ENTRY_m13	((ui1) 0)
#define CMP_DECOMPRESSION_MODE_m13		((ui1) 1)
#define CMP_COMPRESSION_MODE_m13		((ui1) 2)

// Lossy Compression Modes

// Compression Algorithms (use CMP block flag codes)
#define CMP_RED1_COMPRESSION_m13	CMP_BF_RED1_ENCODING_m13
#define CMP_RED2_COMPRESSION_m13	CMP_BF_RED2_ENCODING_m13
#define CMP_RED_COMPRESSION_m13		CMP_RED2_COMPRESSION_m13 // use RED v2 as default RED
#define CMP_PRED1_COMPRESSION_m13	CMP_BF_PRED1_ENCODING_m13
#define CMP_PRED2_COMPRESSION_m13	CMP_BF_PRED2_ENCODING_m13
#define CMP_PRED_COMPRESSION_m13	CMP_PRED2_COMPRESSION_m13 // use PRED v2 as default PRED
#define CMP_SRRED_COMPRESSION_m13	CMP_BF_SRRED_ENCODING_m13
#define CMP_SSE_COMPRESSION_m13		CMP_BF_SSE_ENCODING_m13
#define CMP_MBE_COMPRESSION_m13		CMP_BF_MBE_ENCODING_m13
#define CMP_VDS_COMPRESSION_m13		CMP_BF_VDS_ENCODING_m13

// Directives Flags
// Note: data encryption is set & read via universal_header->encryption_1 field (seems like a directive, but then things have to be set in both places)
#define CPS_DF_COMPRESSION_MODE_m13			((ui8) 1 << 0) // unset == decompressio, set == compression
#define CPS_DF_RED1_ALGORITHM_m13			((ui8) 1 << 1)
#define CPS_DF_RED2_ALGORITHM_m13			((ui8) 1 << 2)
#define CPS_DF_RED_ALGORITHM_m13			CPS_DF_RED2_ALGORITHM_m13 // default RED
#define CPS_DF_PRED1_ALGORITHM_m13			((ui8) 1 << 3)
#define CPS_DF_PRED2_ALGORITHM_m13			((ui8) 1 << 4)
#define CPS_DF_PRED_ALGORITHM_m13			CPS_DF_PRED2_ALGORITHM_m13 // default PRED
#define CPS_DF_SRRED_ALGORITHM_m13			((ui8) 1 << 5)
#define CPS_DF_SSE_ALGORITHM_m13			((ui8) 1 << 6)
#define CPS_DF_MBE_ALGORITHM_m13			((ui8) 1 << 7)
#define CPS_DF_VDS_ALGORITHM_m13			((ui8) 1 << 8)

// BUFFER OWNERSHIP (bits 9 & 10). Unset (the memset default) == the library owns the buffer: it allocates,
// grows & frees it. Set == the CALLER owns it, for a deep user doing something non-standard with the memory -
// a MATLAB mxArray, an mmap'd region, pinned or pooled memory - where free_m13() would be undefined behavior
// rather than merely wasteful. The library then:
//	(1) never frees it, (2) never moves it (the caller's pointer stays valid for the life of the CPS),
//	(3) STILL bounds-checks it, & (4) fails loudly - G_set_error_m13() & NULL/FALSE_m13 - if a request
//	    does not fit, rather than proceeding into a buffer it cannot grow.
// (3) & (4) are why ownership is a DIRECTIVE & not a sentinel in the size field: allocated_decompressed_samples
// / allocated_compressed_bytes must stay TRUE, or the library gives up the very check that makes non-ownership
// safe. The caller supplies the pointer & the true size in the CPS_PARAMS_m13 it passes to
// CMP_allocate_CPS_m13(), which copies that struct wholesale; allocate VALIDATES both & errors if either is
// missing. Two bits, not one: a MEX layer may own the decompressed array while the library still owns the
// compressed buffer. Only DATA buffers can be caller-owned - SCRATCH buffers are library-owned by lifetime
// (see CMP_fit_CPS_scratch_m13()).
// Replaced CMP_SELF_MANAGED_MEMORY_m13 (deleted 2026-08-07), which was documented to "prevent automatic
// re-allocation" but never reached the size field it was tested against, so the guard could not fire & the
// buffer was freed anyway. It had never been exercised.
#define CPS_DF_CALLER_OWNS_DECOMPRESSED_m13		((ui8) 1 << 9)
#define CPS_DF_CALLER_OWNS_COMPRESSED_m13		((ui8) 1 << 10)


#define CPS_DF_CPS_POINTER_RESET_m13			((ui8) 1 << 12)
#define CPS_DF_CPS_CACHING_m13				((ui8) 1 << 13)
#define CPS_DF_FALL_THROUGH_TO_BEST_ENCODING_m13	((ui8) 1 << 14)
#define CPS_DF_RESET_DISCONTINUITY_m13			((ui8) 1 << 15)
#define CPS_DF_NO_ZERO_COUNTS_m13			((ui8) 1 << 17)
#define CPS_DF_SET_OVERFLOW_BYTES_m13			((ui8) 1 << 18) // user sets value in parameters
#define CPS_DF_FIND_OVERFLOW_BYTES_m13			((ui8) 1 << 19) // determine overflow bytes on a block by block basis
// CPS_DF_POSITIVE_DERIVATIVES_m13: caller PROMISES the stream's derivatives are strictly positive, selecting RED's
// positive-derivative model (range 1..255 with no sign bit, keysample flag 0x00). The win is the OVERFLOW CEILING,
// not entropy: it doubles from 127 to 255, and an in-range value costs ONE symbol where an overflow costs the flag
// plus 2-4 raw bytes. That matters for VDS times, whose gaps are often large & which are monotonic BY CONSTRUCTION.
// 2026-08-02: the encoders used to INFER this from "minimum_difference_value > 0" instead. That was wrong twice
// over - it made a rarely-taken (so undertested) path fire on ordinary sample data, where a single zero difference
// anywhere in a block silently turned it off again; and the size estimators model -127..127 unconditionally, so
// they scored every 128..255 value as an overflow & badly mis-priced exactly the blocks that took the path.
// It is now selected only by this directive. WARNING: it is a promise, not a hint - the overflow width drops a
// sign bit, so setting it on a stream that can go negative or flat is a correctness hazard, not just a size loss.
// ⚠️ RETIRED 2026-08-06 - NO ENCODER CONSULTS THIS; setting it is inert. Kept as a RESERVED bit (files in the
// wild may carry the corresponding HEADER flag, so the decoders still honour that - see
// CMP_RED_FLAGS_POSITIVE_DERIVATIVES_m13). It promised all-positive derivatives, which moved the 255-value
// coding window from [-127,127] to [1,255]. Retired because it stopped paying: the values it rescues are
// 0.01-0.02% of a real time stream, and in the lossy regime where they are common, 91-100% of VDS time
// sub-blocks have already fallen through to MBE, which does not consult the window at all. Its last caller
// was the VDS time sub-stream; MEASURED cost of dropping it there: 8-40 bytes per ~100 KB.
#define CPS_DF_POSITIVE_DERIVATIVES_m13 		((ui8) 1 << 20)
#define CPS_DF_SET_DERIVATIVE_LEVEL_m13			((ui8) 1 << 21)	 // user sets level in parameters
#define CPS_DF_FIND_DERIVATIVE_LEVEL_m13		((ui8) 1 << 22)
#define CPS_DF_CONVERT_TO_NATIVE_UNITS_m13		((ui8) 1 << 23)

// NOISE SCORES - one directive per score, in the same order as the bytes they produce, so the directives can
// express exactly what the format can: any subset (each absent byte reads CMP_NS_NO_ENTRY_m13). Set any of
// them & the block carries the noise-score parameter. CPS_DF_ALL_NOISE_SCORES_m13 is a convenience mask, not
// a separate mechanism.
// ⚠️ SELECTION IS ABOUT INTENT, NOT SPEED, for entropy & normality: they SHARE both passes over the data, so
// asking for one costs the same as asking for both - MEASURED at 10,000 samples, entropy alone 30 us,
// normality alone 29 us, the pair together 30 us. Only two selections actually save time: LLP alone needs
// just the first pass (11 us), and omitting LINE NOISE avoids a filter that costs ~100x all the others.
// ⚠️ LINE NOISE is computed by the library ONLY IF the caller has not already supplied it: a deposited value
// in params.noise_scores[CMP_NS_LINE_NOISE_IDX_m13] WINS. An application already running
// FILT_line_noise_m13() gets the TRUE score for a few percent of that run & should deposit it; when no value
// was deposited the library computes the FAST ESTIMATE (FILT_line_noise_estimate_m13(), ~45x cheaper than
// the full machinery), which needs params.sampling_frequency & params.line_frequency.
#define CPS_DF_LINE_NOISE_SCORE_m13		((ui8) 1 << 24)
// The application sets/clears this as it switches true line-noise filtering (FILT_line_noise_m13()) on & off
// during recording: set => this block's samples are the FILTERED data, the encoder stamps
// CMP_BF_LINE_NOISE_FILTERED_m13 into the block header, & the deposited line-noise byte reads "what was
// REMOVED" (recomputing on filtered data is ~0 by construction - scorer & filter share the template
// estimator - so the pre-filter score is deposited instead: it records how much the data was manipulated).
#define CPS_DF_LINE_NOISE_FILTERED_m13		((ui8) 1 << 16)
#define CPS_DF_ENTROPY_DEFICIT_SCORE_m13	((ui8) 1 << 25)
#define CPS_DF_NON_NORMALITY_SCORE_m13		((ui8) 1 << 26)
#define CPS_DF_LLP_SCORE_m13			((ui8) 1 << 27)
#define CPS_DF_ALL_NOISE_SCORES_m13		( CPS_DF_LINE_NOISE_SCORE_m13 | CPS_DF_ENTROPY_DEFICIT_SCORE_m13 | \
						CPS_DF_NON_NORMALITY_SCORE_m13 | CPS_DF_LLP_SCORE_m13 )

// directives flags (lossy)
#define CPS_DF_DETREND_DATA_m13				((ui8) 1 << 32)

// masks
#define CPS_DF_ALGORITHM_MASK_m13			( CPS_DF_RED1_ALGORITHM_m13 | CPS_DF_PRED1_ALGORITHM_m13 | CPS_DF_RED2_ALGORITHM_m13 | \
							CPS_DF_PRED2_ALGORITHM_m13 | CPS_DF_VDS_ALGORITHM_m13 | CPS_DF_MBE_ALGORITHM_m13 | \
							CPS_DF_SRRED_ALGORITHM_m13 | CPS_DF_SSE_ALGORITHM_m13 )

// directive defaults
#define CPS_DIRECTIVES_COMPRESSION_MODE_DEFAULT_m13			FALSE_m13 // TRUE_m13 == compression, FALSE_m13 == decompression
#define CPS_DIRECTIVES_RED_ALGORITHM_DEFAULT_m13			FALSE_m13 // algorithm defaults are mutually exclusive (one, & only one, must be true)
#define CPS_DIRECTIVES_PRED_ALGORITHM_DEFAULT_m13			TRUE_m13 // algorithm defaults are mutually exclusive (one, & only one, must be true)
#define CPS_DIRECTIVES_SRRED_ALGORITHM_DEFAULT_m13			FALSE_m13 // algorithm defaults are mutually exclusive (one, & only one, must be true)
#define CPS_DIRECTIVES_SSE_ALGORITHM_DEFAULT_m13			FALSE_m13 // algorithm defaults are mutually exclusive (one, & only one, must be true)
#define CPS_DIRECTIVES_VDS_ALGORITHM_DEFAULT_m13			FALSE_m13 // algorithm defaults are mutually exclusive (one, & only one, must be true)
#define CPS_DIRECTIVES_MBE_ALGORITHM_DEFAULT_m13			FALSE_m13 // algorithm defaults are mutually exclusive (one, & only one, must be true)
#define CPS_DIRECTIVES_LEVEL_1_ENCRYPTION_DEFAULT_m13			FALSE_m13 // encryption defaults are mutually exclusive (one, & only one, can be true, but neither must be)
#define CPS_DIRECTIVES_LEVEL_2_ENCRYPTION_DEFAULT_m13			FALSE_m13 // encryption defaults are mutually exclusive (one, & only one, can be true, but neither must be)
#define CPS_DIRECTIVES_CPS_POINTER_RESET_DEFAULT_m13			TRUE_m13
#define CPS_DIRECTIVES_CPS_CACHING_DEFAULT_m13				TRUE_m13
#define CPS_DIRECTIVES_FALL_THROUGH_TO_BEST_ENCODING_DEFAULT_m13	TRUE_m13
#define CPS_DIRECTIVES_RESET_DISCONTINUITY_DEFAULT_m13			TRUE_m13
#define CPS_DIRECTIVES_NO_ZERO_COUNTS_DEFAULT_m13			FALSE_m13
#define CPS_DIRECTIVES_SET_OVERFLOW_BYTES_DEFAULT_m13			FALSE_m13 // user sets value in parameters
#define CPS_DIRECTIVES_FIND_OVERFLOW_BYTES_DEFAULT_m13			TRUE_m13 // determine overflow bytes on a block by block basis
#define CPS_DIRECTIVES_SET_DERIVATIVE_LEVEL_DEFAULT_m13			FALSE_m13 // user sets level in parameters
#define CPS_DIRECTIVES_FIND_DERIVATIVE_LEVEL_DEFAULT_m13		TRUE_m13 // was FALSE while the search was in development; on by default 2026-08-02 (with neither set, CMP_differentiate_m13() just uses level 1)
#define CPS_DIRECTIVES_CONVERT_TO_NATIVE_UNITS_DEFAULT_m13		TRUE_m13
// directive defaults (lossy)
#define CPS_DIRECTIVES_DETREND_DATA_DEFAULT_m13				FALSE_m13
#define CPS_DIRECTIVES_ALGORITHM_DEFAULT_m13				CMP_PRED_COMPRESSION_m13
#define CPS_DIRECTIVES_ENCRYPTION_LEVEL_DEFAULT_m13			NO_ENCRYPTION_m13

// parameters defaults
#define CPS_PARAMS_NUMBER_OF_BLOCK_PARAMS_DEFAULT_m13		0
#define CPS_PARAMS_MINIMUM_SAMPLE_VALUE_DEFAULT_m13		CMP_SAMPLE_VALUE_NO_ENTRY_m13
#define CPS_PARAMS_MAXIMUM_SAMPLE_VALUE_DEFAULT_m13		CMP_SAMPLE_VALUE_NO_ENTRY_m13
#define CPS_PARAMS_DISCONTINUITY_DEFAULT_m13			UNKNOWN_m13
#define CPS_PARAMS_DERIVATIVE_LEVEL_DEFAULT_m13			((ui1) 1)
#define CPS_PARAMS_OVERFLOW_BYTES_DEFAULT_m13			4
// (SRRED scale-search defaults & preset sets live with the SRRED scale macros above: CMP_SRRED_SCALE_*_{DEFAULT,MAX_COMPRESSION,MAX_SPEED}_m13)
// parameters defaults (lossy)
#define CPS_PARAMS_VDS_GOAL_ATTEMPTS_DEFAULT_m13		20
#define CPS_PARAMS_VDS_THRESHOLD_DEFAULT_m13			((sf8) 5.0) // generally an integer, but any float value is fine. Range 0.0 to 10.0; default == 5.0 (0.0 == lossless compression)
// parameters defaults (variable region)
#define CMP_USER_NUMBER_OF_RECORDS_DEFAULT_m13			((ui2) 0)
#define CMP_USER_RECORD_REGION_BYTES_DEFAULT_m13		((ui2) 0)
#define CMP_USER_PARAMETER_FLAGS_DEFAULT_m13			((ui4) 0)
#define CMP_PROTECTED_REGION_BYTES_DEFAULT_m13			((ui2) 0)
#define CMP_USER_DISCRETIONARY_REGION_BYTES_DEFAULT_m13		((ui2) 0)

// RED/PRED Codec Constants
#define CMP_SI1_KEYSAMPLE_FLAG_m13 		((si1) 0x80) // -128 as si1
#define CMP_UI1_KEYSAMPLE_FLAG_m13 		((ui1) 0x80) // +128 as ui1
#define CMP_POS_DERIV_KEYSAMPLE_FLAG_m13	((ui1) 0x00) // no zero differences expected in positive derivative model
#define CMP_RED_TOTAL_COUNTS_m13 		((ui4) 0x10000) // 2^16
#define CMP_RED_MAXIMUM_RANGE_m13 		((ui8) 0x1000000000000) // 2^48
#define CMP_RED_RANGE_MASK_m13			((ui8) 0xFFFFFFFFFFFF) // 2^48 - 1
#define CMP_RED_MAX_STATS_BINS_m13 		256
#define CMP_RED_CODER_OVERHEAD_m13		((sf8) 1.0015) // CMP_RED_estimate_bytes_m13 multiplies its entropy (coded-data) term by this to recover the range coder's real output above the entropy bound (count-quantization to 65535 + per-stream flush); the exact 3-byte/symbol model term is NOT scaled. Measured RED-body actual/estimate at the encoder's own decision point over 8 sets x all SRRED per-block scales (2890 streams): pooled-median 1.0013, per-set 1.0008-1.0020 (fy 1.013 - small streams where the fixed flush dominates); 1.0015 is central & a hair pessimistic (safely favors exact MBE in ties). Flows to every consumer consistently: RED x1, SRRED x2, PRED x3. Being ~multiplicative it leaves within-algorithm argmins (scale, derivative level) put & sharpens only the vs-MBE(exact) diversion.
// Multiplier CMP_max_compressed_bytes_m13() applies to the keysample stream (CMP_MAX_KEYSAMPLE_BYTES_m13, the hard
// input bound) to bound the range coder's output when there is NO MBE diversion to fall back on, i.e. when
// CPS_DF_FALL_THROUGH_TO_BEST_ENCODING_m13 is clear. Derived, not guessed, from the coder's own structure:
//   - the coder's model IS the block's own histogram, so the IDEAL output is <= 1 byte per keysample byte;
//   - a FULL dump emits 6 bytes but discharges only (6 - log256(range)), wasting up to 2 bytes, and a dump needs
//     4 bytes of accumulated cost, so the worst case is 1.5x the ideal. (It requires the interval to straddle a
//     2^40 boundary at essentially every dump - ~2^-24 each in practice, which is why the MEASURED overhead is
//     ~0.15% - but nothing prevents it.)
//   - the two >>16 floors can cost a further bit per symbol, a further 1.125x. 1.5 x 1.125 = 1.6875, so 1.7 is
//     the airtight figure & is what is used: this branch is the rare one (a caller forcing pure RED/PRED for
//     testing), so buying the PROOF outright is worth the memory, and it is what lets the range-encode loop run
//     with no per-dump bounds check.
// For scale: 1.7 x 5 = 8.5 bytes/sample, against a worst MEASURED 4.83 on adversarial input
// (dev/claude_tests/cmp_bound_test.c) - 76% headroom over anything observable.
#define CMP_RED_CODER_WORST_CASE_MULT_m13	((sf8) 1.7)

// Cushion added to the MBE-sized branch of CMP_max_compressed_bytes_m13(), covering the amount by which a RED/PRED
// stream that WINS the biased comparison can still exceed the MBE block the buffer was sized for. Fitted, not
// guessed: the matched (estimate, actual) pair was captured at the encoder's own decision point over 3087 blocks -
// real neural data AND adversarial data engineered to sit on the RED-vs-MBE crossover - at block sizes from 1k to
// 100k samples. The tightest envelope holding every point is
//         actual <= estimate + 168 bytes
// with a multiplicative term of EXACTLY ZERO across a 150x range of block sizes. That is structural, not luck: the
// range coder's flush, the model-header rounding & the 8-byte pad are FIXED-size effects, so the residual does not
// scale with the block. A proportional cushion would be pure waste - block_samps/4 would be 75 KB per block at the
// 300k-sample blocks that occur in real recordings, against a real requirement of a few hundred bytes.
// 1024 is ~6x the worst residual ever observed. Figure: ~/Desktop/SRRED/estimator_ceiling.png.
// NB this is an EMPIRICAL ceiling. The range coder's pathological worst case (see
// CMP_RED_CODER_WORST_CASE_MULT_m13) sits far above it & is NOT bounded by this fit - which is why the
// FALL_THROUGH-clear branch is sized from the derivation instead.
#define CMP_ESTIMATE_CUSHION_BYTES_m13		((si8) 1024)

// Ceiling on the derivative level CMP_differentiate_m13()'s FIND search will climb to. Level 6 is the highest ever
// observed on real or development data; each level costs a full raw si4 anchor in the model region, and leaving it
// uncapped (the field is a ui1) would make the compressed-block bound carry 255 x 4 anchor bytes instead of 32.
// A caller using SET_DERIVATIVE_LEVEL is not clamped - it is a caller promise - but the sizing function charges
// the greater of this cap and the caller's goal level.
#define CMP_MAX_DERIVATIVE_LEVEL_m13		((ui1) 8)

// CMP_RED2/PRED2_encode_m13() multiply their estimated total by this before comparing against the EXACT MBE total,
// so RED/PRED must beat MBE by more than this margin to run at all.
// ⚠️ This is a SPEED knob, not a safety one. It used to be load-bearing for buffer safety; it no longer is - the
// compressed buffer now carries CMP_ESTIMATE_CUSHION_BYTES_m13, which bounds the estimator's error on its own, so
// RED/PRED could safely run whenever the UNBIASED estimate wins. What the margin still buys:
//   1) SPEED. MBE encodes & decodes far more cheaply than the range coder, so a near-tie should go to MBE anyway.
//   2) COST IS NOISE. Measured over x/y/sx/fx x RED2/PRED2/SRRED: 1.005 costs +0.0013%, 1.01 costs +0.006%,
//      1.02 costs +0.024%, 1.05 costs +0.35%.
//   3) It keeps the MEASURED regime the OPERATING regime. The cushion was fitted from blocks encoded with this
//      margin in force, so blocks with est in [mbe/1.01, mbe] never ran RED & are not in that sample. Lowering
//      this to 1.0 starts encoding exactly that band - re-measure the cushion first if you do.
#define CMP_MBE_BIAS_m13			((sf8) 1.01)
#define CMP_PRED_CATS_m13 			3
#define CMP_PRED_NIL_m13 			0
#define CMP_PRED_POS_m13 			1
#define CMP_PRED_NEG_m13			2

// decoders that walk packed data in whole 8-byte words (e.g. MBE) can read one word past the last data byte;
// buffers that hold compressed block data are padded by this amount so the final word read stays in bounds
#define CMP_DECODE_SLACK_BYTES_m13		8

// Macros
#define CMP_MAX_KEYSAMPLE_BYTES_m13(block_samps)		( block_samps * 5 ) // full si4 plus 1 keysample flag byte per sample
// ⚠️ SUPERSEDED 2026-08-02 by CMP_max_compressed_bytes_m13(). DO NOT USE IN NEW CODE, and prefer passing zero for
// compressed_data_bytes so CMP_allocate_CPS_m13() sizes the buffer itself. This macro bounds ONLY a full-range MBE
// block (fixed header + MBE model header + 4 bytes/sample + 8-byte pad) with no variable region, so it silently
// under-bounds: any caller using block records / detrend / amplitude or frequency scale (all of which add variable
// region bytes), RED or PRED with CPS_DF_FALL_THROUGH_TO_BEST_ENCODING_m13 CLEAR (pure RED measured at 4.8
// bytes/sample), and SRRED or VDS, which pack TWO sub-blocks into one block (measured at ~2x this). Kept only so
// existing application code still compiles while it migrates.
#define CMP_MAX_COMPRESSED_BYTES_m13(block_samps, n_blocks)	( ((block_samps * 4) + CMP_BLOCK_FIXED_HDR_BYTES_m13 + CMP_MBE_MODEL_FIXED_HDR_BYTES_m13 + 7) * n_blocks )
#define CMP_PRED_CAT_m13(x)					( (x) ? (((x) & 0x80) ? CMP_PRED_NEG_m13 : CMP_PRED_POS_m13) : CMP_PRED_NIL_m13 )
#define CMP_IS_DETRENDED_m13(bh_ptr)				( (bh_ptr->parameter_flags & CMP_PF_INTERCEPT_m13) && (bh_ptr->parameter_flags & CMP_PF_GRADIENT_m13) )
#define CMP_VARIABLE_REGION_BYTES_v1_m13(bh_ptr)		( (ui4) (bh_ptr)->record_region_bytes + (ui4) (bh_ptr)->parameter_region_bytes + \
								(ui4) (bh_ptr)->protected_region_bytes + (ui4) (bh_ptr)->discretionary_region_bytes )
#define CMP_VARIABLE_REGION_BYTES_v2_m13(bh_ptr)		( (ui4) (bh_ptr)->total_header_bytes - ((ui4) CMP_BLOCK_FIXED_HDR_BYTES_m13 + \
								( (ui4) (bh_ptr)->model_region_bytes) )

// Update CPS Pointer Flags
#define CMP_UPDATE_ORIGINAL_PTR_m13		((ui1) 1 << 0)
#define CMP_RESET_ORIGINAL_PTR_m13 		((ui1) 1 << 1)
#define CMP_UPDATE_BLOCK_HDR_PTR_m13		((ui1) 1 << 2)
#define CMP_RESET_BLOCK_HDR_PTR_m13		((ui1) 1 << 3)
#define CMP_UPDATE_DECOMPRESSED_PTR_m13		((ui1) 1 << 4)
#define CMP_RESET_DECOMPRESSED_PTR_m13		((ui1) 1 << 5)

// Binterpolate() center mode codes
#define CMP_CENT_MODE_NONE_m13		0 // extrema only
#define CMP_CENT_MODE_MIDPOINT_m13	1 // best performance if extrema needed: (min + max) / 2
#define CMP_CENT_MODE_MEAN_m13		2 // best performance if extrema not needed
#define CMP_CENT_MODE_MEDIAN_m13	3 // best measure of central tendency
#define CMP_CENT_MODE_FASTEST_m13	4 // CMP_CENT_MODE_MIDPOINT_m13 if extrema requested, CMP_CENT_MODE_MEAN_m13 if not
#define CMP_CENT_MODE_DEFAULT_m13	CMP_CENT_MODE_MEAN_m13

// Normal cumulative distribution function values from -3 to +3 standard deviations in 0.1 sigma steps
#define CMP_NORMAL_CDF_TABLE_ENTRIES_m13 61

// CMP_KS_score_m13() null-expectation correction: D_null(N) = COEF/sqrt(N) + FLOOR, subtracted so clean data
// reads ~0 at any block size. Fitted over N = 128 .. 262144 (400 gaussian realizations each); residuals stay
// under 0.0005 (< 1/8 byte). The 1/sqrt(N) term is finite-sample error. ⚠️ The constant term is empirical &
// unexplained - it is NOT truncation residue (widening the table to +/-8 sigma leaves the floor unchanged,
// measured). Re-fit with dev/claude_tests/ks_score_test.c if the bin width ever changes.
#define CMP_KS_NULL_COEF_m13		((sf8) 0.529152)
#define CMP_KS_NULL_FLOOR_m13		((sf8) 0.000354)
#define CMP_LOG_TABLE_ENTRIES_m13	(1 << 16)  // CMP_log_table[n] = log2(n) for n in [0, 65536); entropy-estimator LUT (counts >= this fall back to a live log2())
#define CMP_NORMAL_CDF_TABLE_m13 {	0.00134989803163010, 0.00186581330038404, 0.00255513033042794, 0.00346697380304067, \
					0.00466118802371875, 0.00620966532577614, 0.00819753592459614, 0.01072411002167580, \
					0.01390344751349860, 0.01786442056281660, 0.02275013194817920, 0.02871655981600180, \
					0.03593031911292580, 0.04456546275854310, 0.05479929169955800, 0.06680720126885810, \
					0.08075665923377110, 0.09680048458561040, 0.11506967022170800, 0.13566606094638300, \
					0.15865525393145700, 0.18406012534676000, 0.21185539858339700, 0.24196365222307300, \
					0.27425311775007400, 0.30853753872598700, 0.34457825838967600, 0.38208857781104700, \
					0.42074029056089700, 0.46017216272297100, 0.50000000000000000, 0.53982783727702900, \
					0.57925970943910300, 0.61791142218895300, 0.65542174161032400, 0.69146246127401300, \
					0.72574688224992600, 0.75803634777692700, 0.78814460141660300, 0.81593987465324100, \
					0.84134474606854300, 0.86433393905361700, 0.88493032977829200, 0.90319951541439000, \
					0.91924334076622900, 0.93319279873114200, 0.94520070830044200, 0.95543453724145700, \
					0.96406968088707400, 0.97128344018399800, 0.97724986805182100, 0.98213557943718300, \
					0.98609655248650100, 0.98927588997832400, 0.99180246407540400, 0.99379033467422400, \
					0.99533881197628100, 0.99653302619695900, 0.99744486966957200, 0.99813418669961600, \
					0.99865010196837000 }


// VDS user threshold (0-10) -> "m", the substitution threshold as a MULTIPLE OF THE BLOCK BASELINE.
//   thresh = max(m(u) * baseline, CMP_VDS_THRESHOLD_FLOOR_m13),  baseline = median |y - running median|
// Multiplicative (not additive) so the decision is invariant to gain AND to sampling rate: measured lossiness
// held at 0.96 +/- 11% over a 16x range of sf (30 kHz / 7.5 kHz / 1.875 kHz) on the same physiology.
// Anchors are measured, not chosen (EEG 255 Hz + units 30 kHz, both vs their own lossless PRED):
//   m <= 1  : output EXCEEDS lossless (97.7% / 104.4%) & needs 93% anchors -> useless, falls through to PRED
//   m = 4   : 39.8% / 48.7% of lossless -> the knee, and 1.7% of spike amplitude on unit waveforms  => u = 5
//   m = 8   : 30.5% / 46.0% -> saturation; 8 -> 16 buys <4% size for double the error  => u = 10
// so m doubles every 5 threshold units, anchored at m(5) = 4.
#define CMP_VDS_THRESHOLD_BASE_m13		((sf8) 2.0)  // m(0); u == 0 never reaches here (redirects to PRED)
#define CMP_VDS_THRESHOLD_DOUBLING_m13		((sf8) 5.0)  // threshold units per doubling of m
#define CMP_VDS_THRESHOLD_FLOOR_m13		((sf8) 1.0)  // 1 LSB: the data cannot express a smaller difference, so a
					// threshold below this resolves nothing & merely forces every +/-1 dither sample to be stored
					// verbatim. Also covers baseline == 0 exactly (flat block), where m * baseline would be 0.
					// In COUNTS, not physical units - resolvability is a property of the quantization, and counts
					// are what keeps the decision gain-invariant.

// OBSOLETE (2026-08-01): the interpolated user->algorithm threshold table, and its LFP / no-filt split, are no
// longer used - CMP_VDS_get_theshold_m13() now computes m(u) in closed form (above). Retained only so the
// globals table entry & CMP_init_tables_m13() need not change in the same pass; safe to delete outright.
#define CMP_VDS_THRESHOLD_MAP_TABLE_ENTRIES_m13	101
#define CMP_VDS_THRESHOLD_MAP_TABLE_m13 { \
	{ 0.0, 0.653145437887747, 0.087080239615529 }, \
	{ 0.1, 0.666524244368605, 0.096431833334447 }, \
	{ 0.2, 0.679901382612239, 0.105786886279366 }, \
	{ 0.3, 0.693195278745327, 0.115136328803986 }, \
	{ 0.4, 0.706328010262686, 0.124471091262003 }, \
	{ 0.5, 0.719312446055280, 0.133782104007116 }, \
	{ 0.6, 0.732256199033084, 0.143060297393023 }, \
	{ 0.7, 0.745271324448842, 0.152296601773421 }, \
	{ 0.8, 0.758469877555299, 0.161481947502008 }, \
	{ 0.9, 0.771963913605199, 0.170607264932484 }, \
	{ 1.0, 0.785865487851287, 0.179663484418544 }, \
	{ 1.1, 0.800286655103895, 0.188641536313887 }, \
	{ 1.2, 0.815276097313215, 0.197532350972212 }, \
	{ 1.3, 0.830643391297273, 0.206329712723042 }, \
	{ 1.4, 0.846141732247177, 0.215054845489400 }, \
	{ 1.5, 0.861524315354039, 0.223744284804766 }, \
	{ 1.6, 0.876544335808968, 0.232434732668414 }, \
	{ 1.7, 0.890954988803074, 0.241162891079617 }, \
	{ 1.8, 0.904515693506558, 0.249965462037652 }, \
	{ 1.9, 0.917339694399690, 0.258879147541791 }, \
	{ 2.0, 0.930077675257187, 0.267940649591310 }, \
	{ 2.1, 0.943425290156307, 0.277186670185483 }, \
	{ 2.2, 0.958078193174307, 0.286653911323584 }, \
	{ 2.3, 0.974732038388447, 0.296379075004888 }, \
	{ 2.4, 0.994082479875984, 0.306398209551974 }, \
	{ 2.5, 1.016674233046698, 0.316738820477894 }, \
	{ 2.6, 1.041615782368465, 0.327422417795757 }, \
	{ 2.7, 1.067220754080376, 0.338470387411485 }, \
	{ 2.8, 1.091794360492593, 0.349904115230999 }, \
	{ 2.9, 1.113758598041713, 0.361744987160223 }, \
	{ 3.0, 1.133017949669186, 0.374014389105076 }, \
	{ 3.1, 1.150496180583432, 0.386733706971483 }, \
	{ 3.2, 1.167137104226452, 0.399924326665363 }, \
	{ 3.3, 1.183884534040251, 0.413618810138173 }, \
	{ 3.4, 1.201682114424834, 0.427895185364072 }, \
	{ 3.5, 1.221180179543847, 0.442843040517614 }, \
	{ 3.6, 1.242133433412453, 0.458551963775380 }, \
	{ 3.7, 1.264126230660231, 0.475111543313956 }, \
	{ 3.8, 1.286742925916765, 0.492611367309924 }, \
	{ 3.9, 1.309573914909860, 0.511146276456767 }, \
	{ 4.0, 1.332419792627307, 0.530891512927402 }, \
	{ 4.1, 1.355340749569296, 0.552085150181815 }, \
	{ 4.2, 1.378413034757496, 0.574966933045358 }, \
	{ 4.3, 1.401712891490464, 0.599776606343383 }, \
	{ 4.4, 1.425298489977415, 0.626806804776781 }, \
	{ 4.5, 1.449169878829132, 0.656567276545111 }, \
	{ 4.6, 1.473315445926537, 0.689623472979960 }, \
	{ 4.7, 1.497723579150550, 0.726578911803450 }, \
	{ 4.8, 1.522488258560118, 0.768420064481964 }, \
	{ 4.9, 1.548262083298817, 0.816356801068373 }, \
	{ 5.0, 1.575881693662255, 0.871725955480487 }, \
	{ 5.1, 1.606179052450206, 0.936062669220404 }, \
	{ 5.2, 1.638914040756128, 1.010951239878917 }, \
	{ 5.3, 1.671426199703439, 1.097583980703378 }, \
	{ 5.4, 1.700723162658563, 1.195933599766891 }, \
	{ 5.5, 1.725174105101550, 1.305244713299358 }, \
	{ 5.6, 1.748170849533368, 1.423680806598649 }, \
	{ 5.7, 1.774263115611458, 1.548589345494656 }, \
	{ 5.8, 1.807974531435571, 1.678111066838628 }, \
	{ 5.9, 1.849031541061601, 1.810472754539325 }, \
	{ 6.0, 1.886797330616806, 1.944453521649560 }, \
	{ 6.1, 1.910022474285093, 2.079203902778041 }, \
	{ 6.2, 1.922265774682071, 2.214633042233715 }, \
	{ 6.3, 1.940778181878725, 2.350143024521086 }, \
	{ 6.4, 1.983325396122841, 2.485226121348756 }, \
	{ 6.5, 2.061401451135639, 2.619918574839387 }, \
	{ 6.6, 2.133662260846322, 2.754123018668519 }, \
	{ 6.7, 2.176369286579269, 2.887938280170844 }, \
	{ 6.8, 2.208654721196260, 3.021880758743963 }, \
	{ 6.9, 2.244989253035562, 3.156354273762239 }, \
	{ 7.0, 2.288044683671034, 3.290783349148508 }, \
	{ 7.1, 2.338500365190698, 3.425483693188855 }, \
	{ 7.2, 2.390947777679918, 3.560967005267627 }, \
	{ 7.3, 2.438785285059024, 3.697362380003563 }, \
	{ 7.4, 2.488873238561683, 3.834765208006531 }, \
	{ 7.5, 2.551586167259061, 3.973524655912109 }, \
	{ 7.6, 2.621198805891288, 4.113956660573023 }, \
	{ 7.7, 2.687949002958154, 4.256298709504494 }, \
	{ 7.8, 2.749458372117692, 4.400925898369564 }, \
	{ 7.9, 2.819492290144386, 4.548231341439871 }, \
	{ 8.0, 2.895978757474240, 4.698585849519076 }, \
	{ 8.1, 2.955556103053909, 4.852443976714582 }, \
	{ 8.2, 3.069362319165638, 5.010398618334849 }, \
	{ 8.3, 3.168267272628890, 5.172880252796954 }, \
	{ 8.4, 3.259217694239259, 5.340703709726352 }, \
	{ 8.5, 3.372393284309650, 5.514642980430471 }, \
	{ 8.6, 3.480506090966784, 5.695814408509766 }, \
	{ 8.7, 3.613539318894531, 5.885218071139942 }, \
	{ 8.8, 3.746867732981435, 6.084360685571760 }, \
	{ 8.9, 3.892198202949155, 6.294816359498106 }, \
	{ 9.0, 4.047865691316694, 6.518719318957218 }, \
	{ 9.1, 4.238233461465871, 6.758827410761595 }, \
	{ 9.2, 4.427441777303221, 7.018317189344236 }, \
	{ 9.3, 4.652784008515709, 7.301166878082886 }, \
	{ 9.4, 4.900609519620827, 7.612468057618706 }, \
	{ 9.5, 5.207111696945288, 7.960975856395534 }, \
	{ 9.6, 5.573339028134005, 8.358863805875041 }, \
	{ 9.7, 6.028522124546244, 8.820648191204029 }, \
	{ 9.8, 6.700281313487421, 9.364866314353193 }, \
	{ 9.9, 7.649180239136531, 10.019987941003953 }, \
	{ 10.0, 9.326074985746468, 10.846993908980265 } \
}


// Typedefs & Structures
typedef struct { // requires 4-byte alignment
	ui4	n_keysample_bytes;
	ui1	derivative_level;
	ui1	pad[3];
	ui2	n_statistics_bins;
	ui2	flags;
} CMP_RED_MODEL_FIXED_HDR_m13;

typedef struct { // requires 4-byte alignment
	ui4	n_keysample_bytes;
	ui1	derivative_level;
	ui1	pad[3];
	union {
		ui2	n_statistics_bins[3];
		struct {
			ui2	n_nil_statistics_bins;
			ui2	n_pos_statistics_bins;
			ui2	n_neg_statistics_bins;
		};
	};
	ui2	flags;
} CMP_PRED_MODEL_FIXED_HDR_m13;

typedef struct { // requires 4-byte alignment
	sf4	scale;
	ui4	scaled_block_total_bytes;
	ui2	scaled_block_model_bytes;
	ui2	residuals_block_model_bytes;
	ui1	derivative_level;
	ui1	pad;
	ui2	flags;
} CMP_SRRED_MODEL_FIXED_HDR_m13;

typedef struct { // requires 4-byte alignment
	si4	minimum_value; // of highest derivative
	ui1	bits_per_sample;
	ui1	derivative_level;
	ui2	flags;
} CMP_MBE_MODEL_FIXED_HDR_m13;

typedef struct {  // requires 4-byte alignment
	ui4	n_keysample_bytes;
	ui1	derivative_level;
	ui1	pad[3];
	ui2	n_statistics_bins;
	ui2	flags;
} CMP_SSE_MODEL_FIXED_HDR_m13;

typedef struct { // requires 4-byte alignment
	ui4	n_VDS_samples;
	ui4	amplitude_block_total_bytes;
	ui2	amplitude_block_model_bytes;
	ui2	time_block_model_bytes;
	ui4	flags; // potentially more options for VDS, so 32 bits
} CMP_VDS_MODEL_FIXED_HDR_m13;

typedef struct {
	ui4	type_code; // note this is not null terminated and so cannot be treated as a string as in REC_HDR_m13 structure
	ui1	version_major;
	ui1	version_minor;
	ui2	total_bytes; // note maximum record size is 65535 - smaller than in REC_HDR_m13 structure
} CMP_REC_HDR_m13;

// CMP_FIXED_BH_m13 declared above

// CMP header layouts (compile-time; see LAYOUT_*_m13 note; CMP_FIXED_BH_m13 is declared before the
// LAYOUT macros, so its assertions live here with the other CMP structures)
LAYOUT_SIZE_m13(CMP_FIXED_BH_m13, CMP_BLOCK_FIXED_HDR_BYTES_m13);
LAYOUT_FIELD_m13(CMP_FIXED_BH_m13, block_start_UID, CMP_BLOCK_START_UID_OFFSET_m13);
LAYOUT_FIELD_m13(CMP_FIXED_BH_m13, block_CRC, CMP_BLOCK_CRC_OFFSET_m13);
LAYOUT_FIELD_m13(CMP_FIXED_BH_m13, block_flags, CMP_BLOCK_BLOCK_FLAGS_OFFSET_m13);
LAYOUT_FIELD_m13(CMP_FIXED_BH_m13, start_time, CMP_BLOCK_START_TIME_OFFSET_m13);
LAYOUT_FIELD_m13(CMP_FIXED_BH_m13, acquisition_channel_number, CMP_BLOCK_ACQUISITION_CHANNEL_NUMBER_OFFSET_m13);
LAYOUT_FIELD_m13(CMP_FIXED_BH_m13, total_block_bytes, CMP_BLOCK_TOTAL_BLOCK_BYTES_OFFSET_m13);
LAYOUT_FIELD_m13(CMP_FIXED_BH_m13, number_of_samples, CMP_BLOCK_NUMBER_OF_SAMPLES_OFFSET_m13);
LAYOUT_FIELD_m13(CMP_FIXED_BH_m13, number_of_records, CMP_BLOCK_NUMBER_OF_RECORDS_OFFSET_m13);
LAYOUT_FIELD_m13(CMP_FIXED_BH_m13, record_region_bytes, CMP_BLOCK_RECORD_REGION_BYTES_OFFSET_m13);
LAYOUT_FIELD_m13(CMP_FIXED_BH_m13, parameter_flags, CMP_BLOCK_PARAMETER_FLAGS_OFFSET_m13);
LAYOUT_FIELD_m13(CMP_FIXED_BH_m13, parameter_region_bytes, CMP_BLOCK_PARAMETER_REGION_BYTES_OFFSET_m13);
LAYOUT_FIELD_m13(CMP_FIXED_BH_m13, protected_region_bytes, CMP_BLOCK_PROTECTED_REGION_BYTES_OFFSET_m13);
LAYOUT_FIELD_m13(CMP_FIXED_BH_m13, discretionary_region_bytes, CMP_BLOCK_DISCRETIONARY_REGION_BYTES_OFFSET_m13);
LAYOUT_FIELD_m13(CMP_FIXED_BH_m13, model_region_bytes, CMP_BLOCK_MODEL_REGION_BYTES_OFFSET_m13);
LAYOUT_FIELD_m13(CMP_FIXED_BH_m13, total_header_bytes, CMP_BLOCK_TOTAL_HEADER_BYTES_OFFSET_m13);
LAYOUT_SIZE_m13(CMP_REC_HDR_m13, CMP_REC_HDR_BYTES_m13);
LAYOUT_FIELD_m13(CMP_REC_HDR_m13, type_code, CMP_REC_HDR_TYPE_CODE_OFFSET_m13);
LAYOUT_FIELD_m13(CMP_REC_HDR_m13, version_major, CMP_REC_HDR_VERSION_MAJOR_OFFSET_m13);
LAYOUT_FIELD_m13(CMP_REC_HDR_m13, version_minor, CMP_REC_HDR_VERSION_MINOR_OFFSET_m13);
LAYOUT_FIELD_m13(CMP_REC_HDR_m13, total_bytes, CMP_REC_HDR_TOTAL_BYTES_OFFSET_m13);

typedef struct {
	ui4 count;
	union {
		si1	value;
		ui1	pos_value;
	};
} CMP_STATISTICS_BIN_m13;

typedef struct {
	si8	n_buffers;
	si8 	n_elements;
	si8	element_size;
	void	**buffer;
 // used internally
	ui8		total_allocated_bytes;
	tern	locked;
	tern	page_aligned;  // TRUE => allocated via aligned_alloc_m13() (lock_memory path) => free with aligned_free_m13()
	tern	allocated;  // TRUE => the struct itself was heap-allocated by CMP_allocate_buffers_m13() (so free it);
			    // stays unset (== 0, not TRUE) for a caller-owned stack/member struct. Records freeable
			    // status at creation - like LH_m13/FILE_m13 'allocated' - so the free path need not guess.
} CMP_BUFFERS_m13;

// Generic checkout/return depot of CMP_BUFFERS bundles (locked & page-aligned). A thread checks out a bundle
// of a given shape, uses it, returns it; no other thread can use it while checked out. Lets stateless
// per-block scratch (VDS, quantfilt, filtfilt, ...) be pooled to the concurrency level instead of held
// per-consumer. See CMP_checkout_buffers_m13() / CMP_return_buffers_m13().
typedef struct {
	tern			in_use;
	CMP_BUFFERS_m13		*buffers;
} CMP_BUFFER_DEPOT_ENTRY_m13;

struct CMP_BUFFER_DEPOT_m13 {  // tag matches the forward decl before GLOBALS_m13
	CMP_BUFFER_DEPOT_ENTRY_m13	*entries;
	si8				count;		// entries created
	si8				allocated;	// capacity of the entries array
	pthread_mutex_t_m13		mutex;
};

typedef struct {
	si8	cache_offset;
	ui4	block_samples;
	si4	block_number;
	tern	data_read;
} CMP_CACHE_BLOCK_INFO_m13;

typedef struct NODE_STRUCT_m13 {
	si4	val;
	ui4	count;
	struct NODE_STRUCT_m13	*prev, *next;
} CMP_NODE_m13;

// directives determine behavior of CPS; parameters for directives that require them are in the CPS_PARAMS_m13 structure
typedef struct {
	ui8	flags;
} CPS_DIRECS_m13; // structure for future directives that may not work well as flags

// parameters contain "mechanics" of CPS
typedef struct {
	// cache parameters
	CMP_CACHE_BLOCK_INFO_m13	*cached_blocks;
	si4				cached_block_list_len;
	si4				cached_block_cnt;
	si4				*cache;
	
	// memory parameters
	si8	allocated_block_samples;
	si8	allocated_keysample_bytes;
	si8	allocated_compressed_bytes; // == time series data fps: (raw_data_bytes - UH_BYTES_m13)
	si8	allocated_decompressed_samples;
	
	// compression parameters
	ui1	goal_derivative_level; // used with set_derivative_level directive
	ui1	derivative_level; // goal/actual pairs because not always possible
	ui1	goal_overflow_bytes; // used with set_overflow_bytes directive
	tern	MBE_preprocessed; // RED/PRED -> MBE fallback handoff: the caller has already computed lmin,
			// derivative_level & bits_per_sample for CMP_MBE_encode_m13(), which consumes & clears this.
			// Lives in the CPS, NOT in the block buffer: the block buffer is never zeroed, so a header
			// bit there is a PRE-EXISTING value the encoder must not trust (2026-08-02).
	ui1	overflow_bytes; // goal/actual pairs because not always possible
	
	// block parameters
	tern	discontinuity; // set if block is first after a discontinuity, passed in compression, returned in decompression
	ui4	block_start_index; // block relative
	ui4	block_end_index; // block relative
	si4	n_block_parameters;
	ui4	block_parameter_map[CMP_PF_PARAMETER_FLAG_BITS_m13];
	si4	minimum_sample_value; // found on compression, stored for general use (and MBE, if used)
	si4	maximum_sample_value; // found on compression, stored for general use (and MBE, if used)
	si4	minimum_difference_value;
	si4	maximum_difference_value;
	ui2	user_number_of_records; // set by user
	ui2	user_record_region_bytes; // set by user to reserve bytes for records in block header
	ui4	user_parameter_flags; // user bits to be set in parameter flags of block header (library flags will be set automatically)
	ui2	protected_region_bytes; // not currently used, set to zero (allows for future expansion)
	ui2	user_discretionary_region_bytes; // set by user to reserve bytes for discretionary region in header
	ui4	variable_region_bytes; // value calculated and set by library based on parameters & directives
	ui4	n_derivative_bytes; // values in derivative or difference buffer
	
	// lossless compression parameters
	sf4	SRRED_scale;
	si4	SRRED_scale_window; // scale-search windowed-tracker half-width in scale steps (0 => full anchor scan every block). Larger => broader/more-thorough per-block search. Default CMP_SRRED_SCALE_WINDOW_DEFAULT_m13; user-settable (see preset sets CMP_SRRED_SCALE_*_{DEFAULT,MAX_COMPRESSION,MAX_SPEED}_m13).
	si8	SRRED_scale_refresh; // blocks between forced full-range anchor scans. Smaller => more frequent full scans. Default CMP_SRRED_SCALE_REFRESH_DEFAULT_m13.
	sf8	SRRED_scale_bailout_mult; // anchor-scan adaptive-bailout multiple (0.0 => no bailout, true full scan). Default CMP_SRRED_SCALE_BAILOUT_MULT_DEFAULT_m13.
	si8	SRRED_overflow_samples; // number of samples in the overflow buffer
	sf8	SRRED_scale_center; // scale-search windowed tracker: previous block's optimal scale (< 0.0 => no anchor yet, do a full anchor scan; == 0.0 => NO-SCALE state: last full scan concluded nothing beats unscaled - skip the ladder & redirect to RED2 until the refresh re-checks); tracks the optimum block-to-block so most blocks scan only a small window (see CMP_SRRED_find_parameters_m13)
	si8	SRRED_scale_refresh_ctr; // blocks since the last full anchor scan; forces a periodic re-anchor (params.SRRED_scale_refresh) to catch drift the window missed
	CMP_BUFFERS_m13	*SRRED_scan_buffers; // overflow-VALUE histogram for the scale scan (built once per block by CMP_SRRED_find_parameters_m13(); each scale step then iterates distinct values, not raw samples - the overflow re-processing was the scan's dominant cost on wide data). Freed by CMP_free_CPS_m13()
	si4	SRRED_ovf_hist_base; // value of histogram bin 0 (the block's minimum overflow value)
	si4	SRRED_ovf_hist_span; // number of bins; 0 => histogram absent (estimator walks the raw overflow array)
	tern	SRRED_sub_encode; // TRUE only while CMP_SRRED_encode_m13() runs a sub-stream through a sub-encoder. Gates MBE raw mode in CMP_MBE_estimate_bytes_m13(): raw keeps input_buffer, which holds the OUTER block's samples, never the sub-stream => raw would silently encode the wrong data. Whole-block MBE (incl. the scale-search seed & every standalone encoder) keeps raw pricing.
	si8	n_stats_entries; // number of bins in the counts array (also used in find derivative level)

	// lossy compression parameters
	// (goal_ratio / actual_ratio / goal_tolerance / minimum_normality were deleted 2026-08-07 with the
	// goal-seeking loop they drove - iterate the scale until a target compression / mean residual ratio was
	// hit, gated on a normality score. SRRED & VDS replaced that mechanism.)
	ui1	noise_scores[CMP_NS_BYTES_m13]; // staging for the packed block parameter. Initialized to CMP_NS_NO_ENTRY_m13
					// by CMP_init_params_m13(). The library fills bytes 1-3 per block; byte 0 (line noise) is
					// EXPENSIVE standalone & essentially free only when FILT_line_noise_m13() is already running,
					// so an application that filters deposits it here before each CMP_encode_m13() call. It is
					// consumed & reset to NO_ENTRY every block, so a stale value cannot carry into the next one.
	si4	VDS_goal_attempts; // maximum VDS anchor-refinement rounds before the block is redirected to PRED
	sf8	VDS_LFP_high_fc; // lowpass filter cutoff for VDS encoding (0.0 for no filter)
	sf8	VDS_threshold; // generally an integer, but float values are fine. Range 0.0 to 10.0; default == 5.0 (0.0 indicates lossless compression)
	sf8	sampling_frequency; // channel sampling frequency. Used by VDS to preserve units during LFP filtering (not
			    // used if no filtering is specified) & by the forced line-noise score. Renamed from
			    // VDS_sampling_frequency 2026-08-07: it is not VDS-specific, & a second field for the same
			    // quantity would be one more thing to keep in sync.
	sf8	line_frequency; // mains frequency for the forced line-noise score (TS metadata section 2 carries it as
			// power_line_frequency). RATE_NO_ENTRY_m13 => no forced score.
	
	// compression arrays
	si1			*keysample_buffer; // passed in both compression & decompression
	// ⚠️ derivative_buffer & next_derivative_buffer are RETASKABLE VIEWS, NOT ownership. Codecs take only the
	// CPS, so params.derivative_buffer IS a codec's input channel & pointing it elsewhere is USING that
	// interface: CMP_differentiate_m13()'s search swaps the pair on every winning level (so a losing candidate
	// costs nothing to abandon - the prior derivative is never recalculated), & SRRED aims it at
	// residuals_buffer for its residual sub-encode (the two sub-encodes are sequential, so the derivative array
	// is not live then). Ownership therefore lives in derivative_bufs[], which NOTHING retasks, & only
	// CMP_free_CPS_m13() / CMP_realloc_CPS_m13() touch it.
	// ⭐⭐ Welding the free() handle onto a field DESIGNED to move is what double-freed this pair (2026-08-14):
	// ownership correctness became a property of every exit path of the search, & two of them did not restore.
	// Same class as "never put ownership in the size field" - here, never put ROLE in the OWNERSHIP pointer.
	si4			*derivative_bufs[2]; // OWNS the pair. Deliberately two separate allocations, NOT one carved
					// bundle: adjacency would turn an overrun into a silent write into the neighbour
					// instead of a redzone hit (Matt: an overrun reveals a defect - do not mask it)
	si4			*derivative_buffer; // VIEW: current derivative stream (also the codec input channel). Never freed through this field
	si4			*next_derivative_buffer; // VIEW: the search's candidate scratch. Never freed through this field
	si4			*overflows_buffer; // used in SRRED: separated overflows for the count-domain scale search (compression)
	si4			*residuals_buffer; // used in SRRED: residual stream (both modes - decode reads it). Distinct from the two above: SRRED needs derivatives, residuals & overflows live simultaneously, so these cannot share storage
	si4			*detrended_buffer; // used if needed in compression, size of decompressed block
	CMP_BUFFERS_m13		*scrap_buffers; // multipurpose, CALLER-OWNED scratch (e.g. DHN_Acq). The library neither
					// allocates nor reads it, but CMP_free_CPS_m13() DOES free it when non-NULL - a
					// deliberate convenience, so caller scratch dies with the CPS it is hung on.
					// ⚠️ Hence the invariant: NULL unless a caller set it. See CMP_init_params_m13().
	CMP_BUFFERS_m13		*noise_score_buffers; // sf8 y/fy for the forced line-noise score (library owned, lazily sized)
	CMP_BUFFERS_m13		*line_noise_buffers; // FILT_line_noise_m13()'s own scratch, kept across blocks so it is not
					// reallocated per block
	CMP_BUFFERS_m13		*VDS_input_buffers;
	CMP_BUFFERS_m13		*VDS_output_buffers;
	struct FILTPS_m13	**filtps;
	si4			n_filtps;
	ui1			*model_region;
	void			*count; // used by RED/PRED encode & decode (ui4 * or ui4 **)
	void			*sorted_count; // used by RED/PRED encode & decode (CMP_STATISTICS_BIN_m13 * or CMP_STATISTICS_BIN_m13 **)
	void			*cumulative_count; // used by RED/PRED encode & decode (ui8 * or ui8 **)
	void			*minimum_range; // used by RED/PRED encode & decode (ui8 * or ui8 **)
	void			*symbol_map; // used by RED/PRED encode & decode (ui1 * or ui1 **)
	
	// pointer copies for switching between RED & PRED (Note: memory must be allocated for PRED initially)
	void			*PRED_base_count;
	void			*PRED_base_sorted_count;
	void			*PRED_base_symbol_map;
	void			*PRED_base_cumulative_count;
	void			*PRED_base_minimum_range;
} CPS_PARAMS_m13;

typedef struct CPS_m13 {
	CPS_DIRECS_m13			direcs;
	CPS_PARAMS_m13			params;
	si4				*input_buffer; // pointer that is updated depending on processing options (e.g. points to detrended data, scaled daata, etc.)
	ui1				*compressed_data; // points to base of FPS time_series_data array, NOT an allocated pointer => do not free; should not be updated
	CMP_FIXED_BH_m13		*block_header; // == FPS time_series_data; points to beginning of current block within compressed_data array, updatable
	si4				*decompressed_data; // returned in decompression or if lossy data requested, used in some compression modes, should not be updated
	si4				*decompressed_ptr; // points to beginning of current block within decompressed_data array, updatable
	si4				*original_data; // passed in compression, should not be updated
	si4				*original_ptr; // points to beginning of current block within original_data array, updatable
	ui1				*block_records; // pointer beginning of records region of block header
	ui4				*block_parameters; // pointer beginning of parameter region of block header
	ui1				*discretionary_region;
} CPS_m13;

// Function Prototypes
CMP_BUFFERS_m13	*CMP_allocate_buffers_m13(CMP_BUFFERS_m13 *buffers, si8 n_buffers, si8 n_elements, si8 element_size, tern zero_data, tern lock_memory);
CMP_BUFFERS_m13	*CMP_checkout_buffers_m13(si8 n_buffers, si8 n_elements, si8 element_size);  // depot: get a locked/aligned bundle of this exact shape
tern		CMP_return_buffers_m13(CMP_BUFFERS_m13 *buffers);  // depot: release a checked-out bundle
void		CMP_free_buffer_depot_m13(void);  // depot: free all pooled bundles (teardown)
si8	CMP_max_compressed_bytes_m13(CPS_m13 *cps, si8 block_samps, si8 n_blocks);
CPS_m13	*CMP_allocate_CPS_m13(FPS_m13 *fps, ui4 mode, si8 data_samples, si8 compressed_data_bytes, si8 keysample_bytes, ui4 block_samples, CPS_DIRECS_m13 *direcs, CPS_PARAMS_m13 *parameters);
tern	CMP_binterpolate_sf8_m13(sf8 *in_data, si8 in_len, sf8 *out_data, si8 out_len, ui4 center_mode, tern extrema, sf8 *minima, sf8 *maxima);
tern	CMP_byte_to_hex_m13(ui1 byte, si1 *hex);
sf8	CMP_calculate_mean_residual_ratio_m13(si4 *original_data, si4 *lossy_data, ui4 n_samps);
tern	CMP_calculate_noise_scores_m13(CPS_m13 *cps, si4 *data, si8 len);  // fills params.noise_scores & writes the packed ui4 block parameter
tern	CMP_calculate_statistics_m13(REC_Stat_v10_m13 *stats_ptr, si4 *data, si8 len, CMP_NODE_m13 *nodes);
sf8	CMP_RED_estimate_bytes_m13(ui4 *cnts, si8 n_bins);  // estimated RED size of a histogram (entropy/8 + 3 bytes/nonzero symbol)
sf8	CMP_PRED_estimate_bytes_m13(ui4 **cat_cnts);  // estimated PRED size (sum of per-category RED estimates)
si8	CMP_MBE_estimate_bytes_m13(CPS_m13 *cps, tern *use_raw, si4 *bits_per_sample);  // EXACT total MBE block size (8-byte padded; n_samps/derivative-level/header all from cps); outputs use_raw & bits_per_sample
si4	CMP_compare_sf8_m13(const void *a, const void * b);
si4	CMP_compare_si4_m13(const void *a, const void * b);
si4	CMP_compare_si8_m13(const void *a, const void * b);
tern	CMP_decode_m13(FPS_m13 *fps);
tern	CMP_crypt_block_m13(UH_m13 *uh, PASSWORD_DATA_m13 *pwd, si1 enc_level, CMP_FIXED_BH_m13 *bh, tern encrypt); // SOLE owner of the block encryption span, both directions (see the block encryption map above)
tern	CMP_decrypt_m13(FPS_m13 *fps); // single block decrypt (see also decrypt_time_series_data_m13)
tern	CMP_detrend_m13(si4 *input_buffer, si4 *output_buffer, si8 len, CPS_m13 *cps);
tern	CMP_detrend_sf8_m13(sf8 *input_buffer, sf8 *output_buffer, si8 len);
sf8	CMP_entropy_deficit_m13(si4 *data, si8 len);  // entropy DEFICIT in [0,1]: 0 == fills its range, 1 == no information (thin wrapper over CMP_noise_scores_m13())
ui1	CMP_differentiate_m13(CPS_m13 *cps);
ui1	CMP_dispersion_m13(CPS_m13 *cps, si4 *deriv_p, ui1 n_derivs);
tern	CMP_encode_m13(FPS_m13 *fps, si8 start_time, si4 acquisition_channel_number, ui4 n_samples);
tern	CMP_encrypt_m13(FPS_m13 *fps); // single block encrypt (see also encrypt_time_series_data_m13)
si8	*CMP_find_crits_m13(sf8 *data, si8 data_len, si8 *n_crits, si8 *crit_xs);
tern	CMP_find_crits_2_m13(sf8 *data, si8 data_len, si8 *n_peaks, si8 *peak_xs, si8 *n_troughs, si8 *trough_xs);
tern	CMP_find_extrema_m13(si4 *input_buffer, si8 len, si4 *min, si4 *max, CPS_m13 *cps);
tern	CMP_free_buffers_m13(CMP_BUFFERS_m13 **buffers_ptr);
tern	CMP_free_CPS_cache_m13(CPS_m13 *cps);
tern	CMP_free_CPS_m13(CPS_m13 *cps, tern free_structure);
sf8	CMP_gamma_cdf_m13(sf8 x, sf8 k, sf8 theta, sf8 offset);
sf8	CMP_gamma_cf_m13(sf8 a, sf8 x, sf8 *g_ln);
sf8	CMP_gamma_inv_cdf_m13(sf8 p, sf8 k, sf8 theta, sf8 offset);
sf8	CMP_gamma_inv_p_m13(sf8 p, sf8 a);
sf8	CMP_gamma_ln_m13(sf8 xx);
sf8	CMP_gamma_p_m13(sf8 a, sf8 x);
sf8	CMP_gamma_ser_m13(sf8 a, sf8 x, sf8 *g_ln);
tern	CMP_generate_parameter_map_m13(CPS_m13 *cps);
void	CMP_get_counts_m13(CPS_m13 *cps, tern overflows);
ui1	CMP_get_overflow_bytes_m13(CPS_m13 *cps, ui4 mode, ui4 algorithm);
tern	CMP_get_variable_region_m13(CPS_m13 *cps);
tern	CMP_hex_to_int_m13(si1 *hex_str, void *hex_val, si4 val_bytes);
CPS_DIRECS_m13	*CMP_init_direcs_m13(CPS_DIRECS_m13 *direcs, ui1 compression_mode);
CPS_PARAMS_m13	*CMP_init_params_m13(CPS_PARAMS_m13 *params);
tern	CMP_init_tables_m13(void);
tern	CMP_integrate_m13(CPS_m13 *cps);
tern	CMP_lad_reg_2_sf8_m13(sf8 *x_input_buffer, sf8 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
sf8	CMP_KS_score_m13(si4 *data, ui4 n_samps);  // Kolmogorov-Smirnov D: 0 == normal, 1 == maximally non-normal (a DEGRADATION measure; byte = round(254 * D))
tern	CMP_lad_reg_2_si4_m13(si4 *x_input_buffer, si4 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
tern	CMP_lad_reg_sf8_m13(sf8 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
tern	CMP_lad_reg_si4_m13(si4 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
sf8	*CMP_lin_interp_2_sf8_m13(si8 *in_x, sf8 *in_y, si8 in_len, sf8 *out_y, si8 *out_len);
sf8	*CMP_lin_interp_sf8_m13(sf8 *in_data, si8 in_len, sf8 *out_data, si8 out_len);
si4	*CMP_lin_interp_si4_m13(si4 *in_data, si8 in_len, si4 *out_data, si8 out_len);
tern	CMP_lin_reg_2_sf8_m13(sf8 *x_input_buffer, sf8 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
tern	CMP_lin_reg_2_si4_m13(si4 *x_input_buffer, si4 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
tern	CMP_lin_reg_sf8_m13(sf8 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
tern	CMP_lin_reg_si4_m13(si4 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
sf8	CMP_LLP_score_m13(si4 *data, si8 len);  // local linear prediction residual / sqrt(3); 0 == perfectly predictable, 1 == white noise or worse
tern	CMP_noise_scores_m13(si4 *data, si8 len, sf8 *scores, ui4 which);  // ONE pass-sharing implementation of the library-computed scores; scores[] is sf8[CMP_NS_BYTES_m13], each in [0,1], only the selected entries written
tern	CMP_lock_buffers_m13(CMP_BUFFERS_m13 *buffers);
tern	CMP_MBE_decode_m13(CPS_m13 *cps);
tern	CMP_MBE_encode_m13(CPS_m13 *cps);
sf8	*CMP_mak_interp_sf8_m13(CMP_BUFFERS_m13 *in_bufs, si8 in_len, CMP_BUFFERS_m13 *out_bufs, si8 out_len);
sf8	CMP_p2z_m13(sf8 p);
tern	CMP_PRED1_decode_m13(CPS_m13 *cps);
tern	CMP_PRED2_decode_m13(CPS_m13 *cps);
tern	CMP_PRED1_encode_m13(CPS_m13 *cps);
tern	CMP_PRED2_encode_m13(CPS_m13 *cps);
sf8	CMP_quantval_m13(sf8 *data, si8 len, sf8 quantile, tern preserve_input, sf8 *buf);
CPS_m13	*CMP_realloc_CPS_m13(FPS_m13 *fps, ui4 compression_mode, si8 data_samples, ui4 block_samples);
tern	CMP_rectify_m13(si4 *input_buffer, si4 *output_buffer, si8 len);
tern	CMP_RED1_decode_m13(CPS_m13 *cps);
tern	CMP_RED2_decode_m13(CPS_m13 *cps);
tern	CMP_RED1_encode_m13(CPS_m13 *cps);
tern	CMP_RED2_encode_m13(CPS_m13 *cps);
si8	CMP_range_encode_m13(si4 *derivatives, si8 n_samps, ui1 deriv_level);
tern	CMP_retrend_si4_m13(si4 *in_y, si4 *out_y, si8 len, sf8 m, sf8 b);
tern	CMP_retrend_2_sf8_m13(sf8 *in_x, sf8 *in_y, sf8 *out_y, si8 len, sf8 m, sf8 b);
si2	CMP_round_si2_m13(sf8 val);
si4	CMP_round_si4_m13(sf8 val);
tern	CMP_set_variable_region_m13(CPS_m13 *cps);
tern	CMP_sf8_to_si2_m13(sf8 *sf8_arr, si2 *si2_arr, si8 len, tern round);
tern	CMP_sf8_to_sf4_m13(sf8 *sf8_arr, sf4 *sf4_arr, si8 len, tern round);
tern	CMP_sf8_to_si4_m13(sf8 *sf8_arr, si4 *si4_arr, si8 len, tern round);
tern	CMP_sf8_to_si4_and_scale_m13(sf8 *sf8_arr, si4 *si4_arr, si8 len, sf8 scale);
tern	CMP_show_block_header_m13(void *level_header, CMP_FIXED_BH_m13 *bh);
tern	CMP_show_block_model_m13(CPS_m13 *cps, tern recursed_call);
tern	CMP_si4_to_sf8_m13(si4 *si4_arr, sf8 *sf8_arr, si8 len);
sf8	*CMP_spline_interp_sf8_m13(sf8 *in_data, si8 in_len, sf8 *out_data, si8 out_len, CMP_BUFFERS_m13 *spline_bufs);
si4	*CMP_spline_interp_si4_m13(si4 *in_data, si8 in_len, si4 *out_data, si8 out_len, CMP_BUFFERS_m13 *spline_bufs);
sf8	CMP_splope_m13(sf8 *xa, sf8 *ya, sf8 *d2y, sf8 x, si8 lo_pt, si8 hi_pt);
tern	CMP_SRRED_decode_m13(CPS_m13 *cps);
tern	CMP_SRRED_encode_m13(CPS_m13 *cps);
tern	CMP_SRRED_find_parameters_m13(CPS_m13 *cps);
sf8	CMP_SRRED_estimate_bytes_m13(CPS_m13 *cps, sf8 scale);
tern    CMP_SSE_decode_m13(CPS_m13 *cps);
tern    CMP_SSE_encode_m13(CPS_m13 *cps);
tern	CMP_swap_RED_PRED_m13(CPS_m13 *cps, tern RED_to_PRED);
sf8	CMP_trace_amplitude_m13(sf8 *y, sf8 *buffer, si8 len, tern detrend);
si8	CMP_ts_sort_m13(si4 *x, si8 len, CMP_NODE_m13 *nodes, CMP_NODE_m13 *head, CMP_NODE_m13 *tail, si4 return_sorted_ts, ...);
tern	CMP_unlock_buffers_m13(CMP_BUFFERS_m13 *buffers);
CMP_FIXED_BH_m13	*CMP_update_CPS_pointers_m13(FPS_m13 *fps, ui1 flags);
tern	CMP_VDS_decode_m13(CPS_m13 *cps);
tern	CMP_VDS_encode_m13(CPS_m13 *cps);
tern	CMP_VDS_generate_template_m13(CPS_m13 *cps, si8 data_len);
sf8	CMP_VDS_get_theshold_m13(CPS_m13 *cps);
sf8	CMP_z2p_m13(sf8 z);
tern	CMP_zero_buffers_m13(CMP_BUFFERS_m13 *buffers);



//**********************************************************************************//
// MARK: Cyclic Redundancy Check (CRC)
//**********************************************************************************//

// ATTRIBUTION
//
// Basic CRC-32 manipulation routines
// by Mark Adler (madler@alumni.caltech.edu)
// placed in the public domain 29 Apr 2015
//
// "This library provides general CRC calculation & validation functions and an
// operation to combine the CRCs of two sequences of bytes into a single CRC.
// The routines in this libary only work with the particular CRC-32 polynomial
// provided here."
//
// Minor modifications for compatibility with the MED Library.
// The CRC routines are customized for little endian machines.

// Typedefs
typedef ui4	crc4;

// Constants
#define CRC_BYTES_m13		4
#define CRC_TABLES_m13		8
#define CRC_TABLE_ENTRIES_m13	256
#define CRC_POLYNOMIAL_m13	((crc4) 0xEDB88320) // note library CRC routines are customized to this polynomial, it cannot be changed arbitrarily
#define CRC_START_VALUE_m13	((crc4) 0x0)
#define CRC_NO_ENTRY_m13	CRC_START_VALUE_m13

// CRC Modes
#define CRC_IGNORE_m13		((ui4) 0) // ignore CRCs
#define CRC_VALIDATE_m13	((ui4) 1 << 0) // validate on input
#define CRC_CALCULATE_m13	((ui4) 1 << 1) // calculate on output
#define CRC_MODES_ALL_m13	( CRC_VALIDATE_m13 | CRC_CALCULATE_m13 ) // validate on input & calculate on output

// Macros
#define CRC_SWAP4_m13(x)	( (((x) >> 24) & 0x000000ff) + (((x) >> 8) & 0x0000ff00) + (((x) & 0x0000ff00) << 8) + (((x) & 0x000000ff) << 24) )

// Function Prototypes
crc4	CRC_calculate_m13(const ui1 *block_ptr, si8 block_bytes);
crc4	CRC_combine_m13(crc4 block_1_crc, crc4 block_2_crc, si8 block_2_bytes);
tern	CRC_init_tables_m13(void);
void	CRC_matrix_square_m13(crc4 *square, const crc4 *mat);
crc4	CRC_matrix_times_m13(const crc4 *mat, crc4 vec);
crc4	CRC_remove_m13(crc4 crc, const ui1 *tail_ptr, si8 tail_bytes);  // inverse of appending: CRC(prefix || tail) + tail -> CRC(prefix)
crc4	CRC_update_m13(const ui1 *block_ptr, si8 block_bytes, crc4 current_crc);
tern	CRC_validate_m13(const ui1 *block_ptr, si8 block_bytes, crc4 crc_to_validate);



//**********************************************************************************//
// MARK: Unicode Transformation Format: 8-bit (UTF8)
//**********************************************************************************//

// Prototypes
tern		UTF8_1_byte_char_m13(const si1 *c);
tern		UTF8_2_byte_char_m13(const si1 *c);
tern		UTF8_3_byte_char_m13(const si1 *c);
tern		UTF8_4_byte_char_m13(const si1 *c);
si4		UTF8_char_bytes_m13(const si1 *c);
size_t		UTF8_strchar_m13(const si1 *s);
size_t		UTF8_strlen_m13(const si1 *s);
tern		UTF8_valid_char_m13(const si1 *c);
tern		UTF8_valid_str_m13(const si1 *s);



//**********************************************************************************//
// MARK: American Encryption Standard: 128 bit (AES)
//**********************************************************************************//

// ATRIBUTION
//
// Advanced Encryption Standard implementation in C.
// By Niyaz PK
// E-mail: niyazlife@gmail.com
// Downloaded from Website: http://www.hoozi.com
//
// "This is the source code for encryption using the latest AES algorithm.
// The AES algorithm is also called Rijndael algorithm. The AES algorithm is
// recommended for non-classified use by the National Institute of Standards
// and Technology (NIST), USA."
//
// For the complete description of the algorithm, see:
// http://www.csrc.nist.gov/publications/fips/fips197/fips-197.pdf
//
// The code in this libaray is set for AES 128-bit encryption / decryption only.
//
// Minor modifications were made for compatibility with the MED Library.

#define AES_KEY_BYTES_m13		PASSWORD_BYTES_m13
#define AES_EXPANDED_KEY_BYTES_m13	ENCRYPTION_KEY_BYTES_m13 // AES-128 == ((AES_NR + 1) * AES_NK * AES_NB)

#define AES_NR_m13		10 // The number of rounds in AES Cipher
#define AES_NK_m13		4 // The number of 32 bit words in the key (128 bits)
#define AES_NB_m13		4 // The number of columns comprising a state in AES
#define AES_NS_m13		( AES_NK_m13 * AES_NB_m13 )  // the number of bytes in the state matrix
#define AES_NR_256_m13		14 // The number of rounds in AES-256 Cipher
#define AES_NK_256_m13		8 // The number of 32 bit words in an AES-256 key (256 bits)
#define AES_KEY_BYTES_256_m13	( AES_NK_256_m13 * 4 ) // 32 (binary key, e.g. KDF-derived: null bytes are valid & not conditioned away)
#define AES_EXPANDED_KEY_BYTES_256_m13	( (AES_NR_256_m13 + 1) * AES_NB_m13 * 4 ) // 240
#define AES_XTIME_m13(x)	( (x << 1) ^ (((x >> 7) & 1) * 0x1b) ) // AES_XTIME is a macro that finds the product of 0x02 and the argument to AES_XTIME modulo 0x1b
#define AES_MULTIPLY_m13(x, y)	( ((y & 1) * x) ^ ((y >> 1 & 1) * AES_XTIME_m13(x)) ^ ((y >> 2 & 1) * AES_XTIME_m13(AES_XTIME_m13(x))) ^ \
				((y >> 3 & 1) * AES_XTIME_m13(AES_XTIME_m13(AES_XTIME_m13(x)))) ^ ((y >> 4 & 1) * \
				AES_XTIME_m13(AES_XTIME_m13(AES_XTIME_m13(AES_XTIME_m13(x))))) ) // Multiply is a macro used to multiply numbers in the field GF(2^8)

#define AES_SBOX_ENTRIES_m13	256
#define AES_SBOX_m13 {		0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76, \
				0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0, \
				0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15, \
				0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75, \
				0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84, \
				0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf, \
				0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8, \
				0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2, \
				0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73, \
				0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb, \
				0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79, \
				0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08, \
				0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a, \
				0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e, \
				0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf, \
				0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 }

#define AES_RSBOX_ENTRIES_m13	256
#define AES_RSBOX_m13 {		0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb, \
				0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb, \
				0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e, \
				0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25, \
				0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92, \
				0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84, \
				0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06, \
				0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b, \
				0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73, \
				0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e, \
				0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b, \
				0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4, \
				0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f, \
				0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef, \
				0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61, \
				0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d };

#define AES_RCON_ENTRIES_m13	255
#define AES_RCON_m13 {		0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, \
				0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, \
				0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, \
				0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, \
				0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, \
				0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, \
				0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, \
				0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, \
				0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, \
				0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, \
				0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, \
				0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, \
				0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, \
				0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, \
				0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, \
				0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb };

// Function Prototypes
void	AES_add_round_key_m13(si4 round, ui1 state[][4], ui1 *round_key);
void	AES_cipher_m13(ui1 *in, ui1 *out, ui1 state[][4], ui1 *round_key);
void	AES_cipher_nr_m13(ui1 *in, ui1 *out, ui1 state[][4], ui1 *round_key, si4 nr);
void	AES_ctr_blocks_m13(ui1 *data, si8 len, si4 phase, const ui1 *init_ctr_block, ui1 *round_key_256);  // low-level CTR engine: explicit initial counter block, NIST SP 800-38A semantics
void	AES_ctr_crypt_256_m13(ui1 *data, si8 offset, si8 len, ui8 nonce, const ui1 *key, ui1 *expanded_key);  // CTR mode (encrypt == decrypt); offset = BYTE offset within covered region (sets counter phase => random access); forward (encryption) schedule only, from AES_key_expansion_256_m13()
void	AES_decrypt_m13(ui1 *data, si8 len, const si1 *password, ui1 *inv_expanded_key); // inv_expanded_key from AES_inv_key_expansion_m13() (NOT the encryption schedule)
void	AES_decrypt_256_m13(ui1 *data, si8 len, const ui1 *key, ui1 *inv_expanded_key); // inv_expanded_key from AES_inv_key_expansion_256_m13() (NOT the encryption schedule)
void	AES_encrypt_m13(ui1 *data, si8 len, const si1 *password, ui1 *expanded_key);
void	AES_encrypt_256_m13(ui1 *data, si8 len, const ui1 *key, ui1 *expanded_key);
tern	AES_init_tables_m13(void);
void	AES_inv_cipher_m13(ui1 *in, ui1 *out, ui1 state[][4], ui1 *round_key);
void	AES_inv_cipher_nr_m13(ui1 *in, ui1 *out, ui1 state[][4], ui1 *round_key, si4 nr);
ui1	*AES_inv_key_expansion_m13(ui1 *inv_expanded_key, const si1 *key);
ui1	*AES_inv_key_expansion_256_m13(ui1 *inv_expanded_key, const ui1 *key);
ui1	*AES_inv_key_schedule_m13(ui1 *inv_expanded_key, const ui1 *expanded_key, si4 nr); // encryption schedule -> equivalent-inverse-cipher schedule (in place OK)
void	AES_inv_mix_columns_m13(ui1 state[][4]);
void	AES_inv_shift_rows_m13(ui1 state[][4]);
void	AES_inv_sub_bytes_m13(ui1 state[][4]);
ui1	*AES_key_expansion_m13(ui1 *round_key, const si1 *key);
ui1	*AES_key_expansion_256_m13(ui1 *round_key, const ui1 *key);
void	AES_mix_columns_m13(ui1 state[][4]);
void	AES_partial_decrypt_m13(si4 n_bytes, ui1 *data, ui1 *round_key); // (round_key == NULL) => keyless decryption
void	AES_partial_encrypt_m13(si4 n_bytes, ui1 *data, ui1 *round_key); // (round_key == NULL) => keyless encryption
void	AES_shift_rows_m13(ui1 state[][4]);
void	AES_sub_bytes_m13(ui1 state[][4]);



//***********************************************************************//
// MARK: Secure Hash Algorithm: 256 bit (SHA)
//***********************************************************************//

// ATTRIBUTION:
//
// Author:	Brad Conte (brad@bradconte.com)
// Disclaimer:	This code is presented "as is" without any guarantees.
// Details:	Implementation of the SHA-256 hashing algorithm.
//		Algorithm specification can be found here:
//	 	http://csrc.nist.gov/publications/fips/fips180-2/fips180-2withchangenotice.pdf
//		This implementation uses little endian byte order.
//
// Code:	https://github.com/B-Con/crypto-algorithms/blob/master/sha256.c
//
// Only SHA-256 functions are included in the MED library.
// The version below contains minor modifications for compatibility with the MED Library.


// Constants
#define SHA_HASH_BYTES_m13	32 // 256 bit
#define SHA_LOW_BYTE_MASK_m13	((ui4) 0x000000FF)

#define	SHA_H0_ENTRIES_m13	8
#define	SHA_H0_m13 {		0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 }

#define	SHA_K_ENTRIES_m13	64
#define	SHA_K_m13 {		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, \
 				0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, \
 				0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, \
 				0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, \
 				0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, \
 				0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, \
 				0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3, \
				0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2 }

// Macros
#define SHA_ROTLEFT_m13(a,b)	( ((a) << (b)) | ((a) >> (32-(b))) )
#define SHA_ROTRIGHT_m13(a,b)	( ((a) >> (b)) | ((a) << (32-(b))) )
#define SHA_CH_m13(x,y,z)	( ((x) & (y)) ^ (~(x) & (z)) )
#define SHA_MAJ_m13(x,y,z)	( ((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)) )
#define SHA_EP0_m13(x)		( SHA_ROTRIGHT_m13(x,2) ^ SHA_ROTRIGHT_m13(x,13) ^ SHA_ROTRIGHT_m13(x,22) )
#define SHA_EP1_m13(x)		( SHA_ROTRIGHT_m13(x,6) ^ SHA_ROTRIGHT_m13(x,11) ^ SHA_ROTRIGHT_m13(x,25) )
#define SHA_SIG0_m13(x)		( SHA_ROTRIGHT_m13(x,7) ^ SHA_ROTRIGHT_m13(x,18) ^ ((x) >> 3) )
#define SHA_SIG1_m13(x)		( SHA_ROTRIGHT_m13(x,17) ^ SHA_ROTRIGHT_m13(x,19) ^ ((x) >> 10) )

// Typedefs & Structures
typedef struct {
	ui1	data[64];
	ui4	state[8];
	ui8	bitlen;
	ui4	datalen;
} SHA_CTX_m13;

// Function Prototypes
void	SHA_finalize_m13(SHA_CTX_m13 *ctx, ui1 *hash);
ui1	*SHA_hash_m13(const ui1 *data, si8 len, ui1 *hash);
ui1	*SHA_hmac_m13(const ui1 *key, si4 key_bytes, const ui1 *data, si8 data_bytes, ui1 *mac);
void	SHA_init_m13(SHA_CTX_m13 *ctx);
tern	SHA_init_tables_m13(void);
ui1	*SHA_pbkdf2_m13(const ui1 *pw, si4 pw_bytes, const ui1 *salt, si4 salt_bytes, ui4 iterations, ui1 *dk);
ui1	*SHA_pbkdf2_resume_m13(const ui1 *pw, si4 pw_bytes, const ui1 *salt, si4 salt_bytes, ui4 from_iter, ui4 to_iter, ui1 *u, ui1 *dk); // resumable PBKDF2: advance (u,dk) state from_iter->to_iter; dk snapshot at any to_iter == PBKDF2(that count). from_iter 0 initializes.
void	SHA_transform_m13(SHA_CTX_m13 *ctx, const ui1 *data);
void	SHA_update_m13(SHA_CTX_m13 *ctx, const ui1 *data, si8 len);



//***********************************************************************//
// MARK: Session Key Cache (SKC)
//***********************************************************************//

// Prototypes (backend selection & GLOBALS_SESSION_KEY_CACHE_* constants are defined near the
// top of this file - the SKC_BACKEND_m13 value must exist before any code is parsed)
tern	SKC_evict_m13(const ui1 *cache_id);
tern	SKC_get_m13(const ui1 *cache_id, ui1 *master, ui1 *fp_out);		// TRUE = hit (master filled, unexpired; fp_out (may be NULL) gets the stored password fingerprint); FALSE = miss/expired/unavailable
void	SKC_id_m13(UH_m13 *uh, ui1 *cache_id);					// cache_id = SHA-256(KDF salt || kdf_exponent)[0:SESSION_KEY_CACHE_ID_BYTES_m13]
tern	SKC_put_m13(const ui1 *cache_id, const ui1 *master, const ui1 *fp, si4 timeout_sec);	// store/refresh (resets expiry); fp = password fingerprint (NULL => zeros); timeout clamped to GLOBALS_SESSION_KEY_CACHE_TIMEOUT_MAX_m13


//***********************************************************************//
// MARK: X25519 Elliptic Curve Diffie-Hellman (XEC)
//***********************************************************************//

// X25519 scalar multiplication on Curve25519 per RFC 7748, used ONLY for the recovery-anchor sealed box
// (anchor type 2): a session writer seals the top-level password string to the anchor holder's public key.
// Sealing requires no secret & no network: an ephemeral keypair is generated, the shared secret is derived
// via Diffie-Hellman, keyed through HMAC (CRYPTO_ANCHOR_STRING_m13), & the password block is AES-256
// enciphered. Only the anchor private key - which never exists on the writing system - can reverse it.
//
// The implementation is the portable reference formulation (64-bit limb arithmetic, constant-time
// conditional swaps, no compiler extensions - Windows compatible). Performance is irrelevant here:
// sealing happens once per session creation, unsealing once per recovery event.
//
// Private keys are 32 random bytes from the SYSTEM ENTROPY SOURCE (G_random_bytes_m13(), NOT rand64_m13():
// a predictable ephemeral key would let anyone reconstruct the shared secret & open the box).

// Constants
#define XEC_KEY_BYTES_m13		32 // public key, private key, & shared secret are all 32 bytes
#define XEC_SEALED_BOX_BYTES_m13	(XEC_KEY_BYTES_m13 + ENCRYPTION_BLOCK_BYTES_m13) // ephemeral public key (32) + AES-256 block (16) == ANCHOR_ESCROW_FIELD_BYTES_m13

// Function Prototypes
tern	XEC_keypair_m13(ui1 *public_key, ui1 *private_key); // fresh keypair from system entropy (private key clamped per RFC 7748)
void	XEC_scalarmult_m13(ui1 *out, const ui1 *scalar, const ui1 *point); // X25519(scalar, point); out == 32 bytes
void	XEC_scalarmult_base_m13(ui1 *public_key, const ui1 *private_key); // public key from private key (base point 9)
tern	XEC_seal_m13(ui1 *sealed_box, const ui1 *plaintext_block, const ui1 *recipient_public_key); // 16-byte block -> 48-byte box; ephemeral private key generated & discarded internally
tern	XEC_unseal_m13(ui1 *plaintext_block, const ui1 *sealed_box, const ui1 *recipient_private_key); // 48-byte box -> 16-byte block (fails on low-order/all-zero shared secret)



//**********************************************************************************//
// MARK: DGST: Canonical File Digests
//**********************************************************************************//

// The canonical MED digest covers: data region first, universal header last, pcrc regions excluded
// (derived data, rewritten by parity maintenance - excluded like parity coverage). Data-then-UH order
// lets a writer absorb body bytes as they are written (fwrite_m13() fusion via FILE_FLAGS_DIGEST_m13)
// & the settled universal header once at close - no re-read of arbitrarily large files.
// Video data files keep their UH at the file END (canonical == physical order).
//
// A digest operation yields DGST_RESULT_m13:
//	body   - SHA-256 of the data region alone: survives legitimate UH transformations
//	         (rename, de-identification, updater) & identifies the data across them
//	full   - SHA-256 of data region then UH: the canonical digest (UH tamper evidence - times, UIDs)
//	resume - the pre-UH SHA state (serialized): a changed UH can be absorbed onto it to re-derive
//	         full WITHOUT re-reading the body (constant time at any file size);
//	         self-consistency: finalizing the resume state alone MUST reproduce body (DGST_resume_valid_m13())
// Files smaller than one universal header are digested whole, in file order (degenerate: body == full).
// External verification of a standard file without pcrcs:
//	body:	tail -c +1025 file | shasum -a 256
//	full:	cat <(tail -c +1025 file) <(head -c 1024 file) | shasum -a 256
// Consumed by the CSig record type (REC_build_CSig_body_m13(), medrec_m13)

// Constants
#define DGST_BYTES_m13		SHA_HASH_BYTES_m13 // 32
#define DGST_CHUNK_BYTES_m13	((si8) 1 << 22) // 4 MiB streaming-read chunk (constant memory at any file size)

// resume state serialization (little endian, explicit layout - NOT a raw SHA_CTX_m13 copy):
#define DGST_RESUME_STATE_OFFSET_m13	0	// ui4[8] (SHA chaining state)
#define DGST_RESUME_BITLEN_OFFSET_m13	32	// ui8 (bits absorbed)
#define DGST_RESUME_DATALEN_OFFSET_m13	40	// ui4 (valid bytes in partial block)
#define DGST_RESUME_DATA_OFFSET_m13	44	// ui1[64] (buffered partial block; bytes past datalen zeroed for determinism)
#define DGST_RESUME_BYTES_m13		108

// Typedefs & Structures
typedef struct {
	ui1	body[DGST_BYTES_m13]; // data region alone
	ui1	full[DGST_BYTES_m13]; // data region then UH (canonical)
	ui1	resume[DGST_RESUME_BYTES_m13]; // pre-UH SHA state
} DGST_RESULT_m13;

typedef struct DGST_STREAM_m13 {
	SHA_CTX_m13	ctx; // canonical-order streaming context (data-region bytes)
	ui1		hold[UH_BYTES_m13]; // video only: trailing holdback so the pre-UH state is capturable (UH arrives in stream order)
	si8		high_water; // data-region bytes received so far (video: absorbed + held)
	si4		hold_bytes; // video only: valid bytes in hold
	tern		dirty; // stream invalidated (rewrite below high water, or gapped write) => finalize returns UNKNOWN_m13
	tern		video; // UH at file end: physical == canonical order
} DGST_STREAM_m13;

// Function Prototypes
tern	DGST_begin_m13(FILE_m13 *fp); // attach stream to a MED file open for writing (existing data-region bytes caught up by read; forces LEN & POS flags)
tern	DGST_file_m13(const si1 *path, DGST_RESULT_m13 *result); // canonical digests by streamed read
tern	DGST_finalize_m13(FILE_m13 *fp, DGST_RESULT_m13 *result); // absorb settled UH, emit digests, detach; UNKNOWN_m13 == stream unusable (fall back to DGST_file_m13()), FALSE_m13 == error
void	DGST_free_m13(FILE_m13 *fp); // detach & free stream state without producing a digest (also called by fclose_m13())
tern	DGST_full_from_resume_m13(const ui1 *resume, const ui1 *uh_bytes, ui1 *full_digest); // absorb a (new) UH onto a stored resume state => full digest, no body re-read
tern	DGST_resume_valid_m13(const ui1 *resume, const ui1 *body_digest); // finalizing the resume state alone must reproduce body
void	DGST_update_m13(FILE_m13 *fp, const void *ptr, si8 n_bytes, si8 offset); // fwrite_m13() hook (internal use)



//**********************************************************************************//
// MARK: VID: Video Container Keyframe Walk
//**********************************************************************************//

// Enumerates the keyframes (sync samples) of a video container from the container's own tables,
// yielding what MED video indices need: byte offset, frame number, & time offset of every frame a
// decoder can start from. Consulted ONCE, when indices are WRITTEN (conversion) - at read time the
// MED index replaces the container's seek machinery entirely (see the video index specification).
// Live acquisition does not parse containers: the encoder knows each keyframe as it is emitted &
// calls VID_keyframe_to_index_m13() directly - the same converter path, minus the walk.
// Backends: AVI (idx1 index; e.g. Natus/XLTek exports) & ISO-BMFF (mp4/mov family: stts/stss/stsc/
// stco/co64/stsz sample tables; moov at head or tail; also extracts out-of-band decoder configuration
// [avcC/hvcC] into codec_config for the video metadata - decode survivability).  Not yet handled in
// BMFF: edit lists (elst - times are media-timeline) & fragmented mp4 (moof; acquisition-preferred
// containers are walked at conversion only, & DHN acquisition writes indices live instead).

// Constants
#define VID_WALK_CONTAINER_UNKNOWN_m13	((ui4) 0)
#define VID_WALK_CONTAINER_AVI_m13	((ui4) 1)
#define VID_WALK_CONTAINER_BMFF_m13	((ui4) 2)

// AVI (RIFF) internals
#define VID_AVI_FOURCC_m13(a, b, c, d)	( (ui4) (a) | ((ui4) (b) << 8) | ((ui4) (c) << 16) | ((ui4) (d) << 24) )
#define VID_AVI_IDX1_KEYFRAME_m13	((ui4) 0x00000010) // AVIIF_KEYFRAME

// a run of MORE than this many consecutive dropped frames is a discontinuity (a single dropped frame is absorbed
// by the constant frame rate, like a single missing time series sample - matches DHN_Acq's >1 sample threshold)
#define VID_DISCONTINUITY_FRAME_THRESHOLD_m13	((si8) 1)

// Typedefs & Structures
typedef struct {
	si8	file_offset; // byte offset of the keyframe's coded data within the video file
	si8	frame_num; // zero-based PRESENT-frame number within the FILE (dropped-frame placeholders excluded, like time series sample numbers; segment-relative numbering is the caller's: add the file's start frame)
	sf8	time_offset; // seconds from file start (computed from the container frame ordinal, so placeholders correctly advance the clock)
	tern	discontinuity; // a dropped-frame run > VID_DISCONTINUITY_FRAME_THRESHOLD_m13 preceded this keyframe within the file (the FILE's first keyframe is never flagged - a file-start entry, not a discontinuity; cross-file gaps are the caller's to detect)
} VID_KEYFRAME_m13;

typedef struct {
	ui4			container; // VID_WALK_CONTAINER_*
	ui4			video_stream; // video stream index within the container
	si1			codec[8]; // fourcc as string (e.g. "XVID")
	ui4			horizontal_pixels;
	ui4			vertical_pixels;
	sf8			frame_rate; // frames per second (from container header)
	si8			n_frames; // PRESENT (readable) video frames in the file - excludes dropped-frame placeholders (container totals like AVI dwTotalFrames include them & are container-internal)
	si8			n_dropped_frames; // dropped-frame placeholder chunks (zero-size): counted for timing, not numbered as frames
	tern			has_audio; // audio stream(s) present (they travel in the container; MED indexes video only)
	ui4			codec_config_bytes; // out-of-band decoder configuration (BMFF avcC/hvcC payload, VERBATIM) - zero when config is in-band (AVI) or exceeds the maximum (in-band parameter sets then mandatory)
	ui1			codec_config[VID_METADATA_CODEC_CONFIG_MAX_BYTES_m13]; // -> video metadata section 2 codec_config (decode survivability: tail-indexed container config is lost on damage; MED-stored config + index + parity = reconstructible from MED alone)
	si8			n_keyframes;
	VID_KEYFRAME_m13	*keyframes; // ordered by frame number; freed by VID_walk_free_m13()
} VID_WALK_m13;

// Prototypes
tern		VID_keyframe_to_index_m13(const VID_KEYFRAME_m13 *kf, si8 file_start_time_uutc, si8 file_start_frame_num, ui4 vid_file_num, VID_IDX_m13 *vi); // keyframe facts -> a video index entry (converters loop the walk; live acquisition calls per emitted keyframe)
VID_WALK_m13	*VID_walk_m13(const si1 *path); // walk a finished container's tables; NULL on error
void		VID_walk_free_m13(VID_WALK_m13 **walk);



//**********************************************************************************//
// MARK: Filter (FILT)
//**********************************************************************************//

// ATTRIBUTION
//
// Some of the filter code was adapted from Matlab functions (MathWorks, Inc).
// www.mathworks.com
//
// The c code herein was written entirely from scratch.


// Constants
#define FILT_LOWPASS_TYPE_m13				1
#define FILT_BANDPASS_TYPE_m13				2
#define FILT_HIGHPASS_TYPE_m13				3
#define FILT_BANDSTOP_TYPE_m13				4
#define FILT_TYPE_DEFAULT_m13				FILT_LOWPASS_TYPE_m13
#define FILT_ORDER_DEFAULT_m13				5
#define FILT_PAD_SAMPLES_PER_POLE_m13			3 // minimum == 3
#define FILT_MAX_ORDER_m13				10
#define FILT_FILTFILT_TOLERANCE_DEFAULT_m13		((sf8) 0.5 / (sf8) 2147483648.0) // streaming filtfilt: relative anticausal truncation < 0.5 LSB at full-scale si4 (the natural boundary - all data returns to si4); pass a looser tolerance to trade guarantee for latency
#define FILT_BAD_FILTER_m13				-1
#define FILT_BAD_DATA_m13				-2
#define FILT_EPS_SF8_m13				((sf8) 2.22045e-16)
#define FILT_RADIX_m13					((sf8) 2.0)
#define FILT_LINE_NOISE_HARMONICS_DEFAULT_m13		10 // template-smoothing cutoff = this * line_freq. Set ABOVE the real 5th-harmonic max (300 Hz @60) on purpose: the soft order-4 filtfilt knee would attenuate the 5th if the cutoff sat on it, and the per-cycle median already rejects everything not phase-locked to the mains, so a generous cutoff can't distort signal (nothing above the 5th to remove anyway)
#define FILT_LINE_NOISE_MIN_SAMPS_PER_CYCLE_m13		40 // FILT_line_noise: below this many samples per line cycle, upsample by ceil(this / samps_per_cycle) for template resolution (& to lift the fs/5 smoothing cap above the harmonics). The phase-locked reshape tolerates non-integer upsampled samps/cycle, so U is sized purely for resolution, NOT to make samps/cycle integer (which would force a needlessly large factor)
#define FILT_LINE_NOISE_UPSAMPLE_CUTOFF_FRACTION_m13	((sf8) 0.95) // FILT_line_noise upsampler: anti-image lowpass cutoff as a fraction of the ORIGINAL Nyquist. As high as practical (kills the zero-stuff images while preserving the most harmonics); a lower cutoff would needlessly attenuate near-Nyquist harmonics
// FILT_line_noise_m13() score NULL FLOOR. The score is amp(mains template)/amp(band), and the template is a
// per-cycle MEDIAN over cycles_per_template cycles - so on data with no mains at all the template still holds
// the median's own sampling residual, and the score reads well above zero. MEASURED on clean synthetic data:
// the floor is this coefficient / sqrt(cycles_per_template), flat to within 3% across cycles 15..480 (a 32x
// range), and - importantly - INDEPENDENT of spectral slope: white 0.1455, pink 0.1445, brown 0.1495 at 60
// cycles. The slow band is split off before templating, so what reaches the median averages down like
// uncorrelated data whatever the original spectrum. Hence no 1/f model & no spectral estimate is needed here.
// It matters: at the default 60 cycles the floor is byte 37 of 254, and at 15 cycles it is byte 75.
// Re-fit with dev/claude_tests/lnf_floor_test.c if the template or band-split logic changes.
#define FILT_LINE_NOISE_NULL_COEF_m13			((sf8) 1.136)
// FILT_line_noise_m13() minimum cycles for a template & score. The information unit of the method is CYCLES,
// not samples or seconds: a 1 s block holds ~60 mains cycles at EVERY sampling frequency. Template fidelity
// degrades as 1/sqrt(cycles) (the null floor above, validated over cycles 15..480), so short blocks lose
// PRECISION, not validity - a degraded score is better than no score. Below this count the floor exceeds
// ~0.3 of full scale & the score is not meaningful. When a block holds fewer cycles than the requested
// cycles_per_template, the window is clamped to what the block holds (static template: it stops tracking
// within-block amplitude drift, which is generally much slower than 1 Hz - the cost of doing business).
#define FILT_LINE_NOISE_MIN_CYCLES_m13			15
// FILT_line_noise_estimate_m13() constants. The estimate is a differential cycle-lag autocorrelation on a
// one-pole high-passed copy: mains + harmonics are jointly periodic at the fundamental's period, so they
// contribute their full power at a whole-cycle lag; a quarter cycle away the smooth background autocorr is
// nearly unchanged while only the (small) even harmonics leak - the difference isolates mains power.
// COMB_FACTOR converts that difference to the mains power fraction for a 1/k harmonic amplitude profile
// (1 + (p2 - p4 + ...)/total; profile-dependent, the method's main systematic). NULL_FLOOR is the residual
// sampling noise of the autocorrelation, removed with the same rescale shape as the true score's null floor.
// MAX_CYCLE_MULT: the lag is round(k * samps_per_cycle) for the k <= this minimizing fractional-lag error
// (1 kHz: k=3 -> lag 50 EXACTLY), which controls harmonic phase error without interpolation.
// NULL_FLOOR/GAIN/GAMMA form the CALIBRATION MAP to the true score's scale:
//   score = min(1, GAIN * (max(0, raw - NULL_FLOOR) / (1 - NULL_FLOOR))^GAMMA)
// The relation is strongly convex & that is NOT band leakage (a 2-pole high-pass changed the fit little):
// it is the true score's own convention - the median-template amplitude ratio saturates quickly once mains
// dominates - so the curvature genuinely belongs in this map. FITTED ON REAL DATA
// (dev/claude_tests/lnf_est_calib.c): 900 blocks of clean iEEG + alpha-scaled REAL mains (amplitude-varying,
// real harmonic profile; line_noise_comparison.medd), grid least squares. RMS vs true score 20 of 254 (47
// unmapped); out-of-sample per-channel MEDIAN error <= 6 bytes including the notch-filtered channel
// (different harmonic profile). The corpus is real physiological background: on synthetic white-ish
// backgrounds the mapped estimate reads HIGH vs a synthetic truth - deployment targets real recordings.
// Re-fit with that harness if the estimator or the true score changes.
// ~45x cheaper than the true score (22 vs 990 us at 1 kHz/4 s).
#define FILT_LINE_NOISE_EST_COMB_FACTOR_m13		((sf8) 1.128)
#define FILT_LINE_NOISE_EST_NULL_FLOOR_m13		((sf8) 0.150)
#define FILT_LINE_NOISE_EST_GAIN_m13			((sf8) 1.600)
#define FILT_LINE_NOISE_EST_GAMMA_m13			((sf8) 0.640)
#define FILT_LINE_NOISE_EST_MAX_CYCLE_MULT_m13		8
#define FILT_ANTIALIAS_FREQ_DIVISOR_DEFAULT_m13		((sf8) 3.5);
#define FILT_UNIT_THRESHOLD_DEFAULT_m13			CPS_PARAMS_VDS_UNIT_THRESHOLD_DEFAULT_m13
#define FILT_NFF_BUFFERS_m13				4
// FILT_VDS_TEMPLATE_MIN_PS_m13 / FILT_VDS_TEMPLATE_LFP_PS_m13 (CPS filtps indices) are GONE: the VDS template
// no longer filters. See CMP_VDS_generate_template_m13() for what replaced them & why.
// FILT_VDS_MIN_SAMPS_PER_CYCLE_m13 is GONE (2026-08-01). It was the minimal-antialias cutoff, then briefly a
// cap on VDS_LFP_high_fc. Both roles are gone: there is no antialias filter, and the span floor below is a
// strictly tighter constraint than any samples-per-cycle cap, so the cap never changed an outcome - it only
// decided whether a (misleading) warning printed. One number governs now.
#define	FILT_VDS_MEDIAN_SPAN_MIN_m13			((si8) 12) // minimum running-median window, in SAMPLES (scale-independent).
					// A median only rejects a transient if its window is substantially longer than the transient; at 5
					// samples a 2-3 sample excursion fills half the window & DRAGS the median onto it, which then hides
					// that excursion from the threshold test. Measured (255 Hz EEG, u=5, span floor swept):
					//   floor  5 -> 77.7% of lossless, max err 455   <- median dragged by transients
					//   floor  8 -> 51.9%,             max err 341
					//   floor 12 -> 48.5%,             max err 335   <- knee, both axes
					//   floor 20 -> 45.5%,             max err 345
					//   floor 32 -> 43.1%,             max err 352
					// Note this is a ROBUSTNESS floor & is deliberately NOT the same number as the sf/5 cutoff cap above:
					// one bounds the highest meaningful frequency, the other the shortest usable window.
#define	FILT_VDS_LFP_FC_DEFAULT_m13			((sf8) 500.0) // VDS: median-window cutoff used when NO LFP cutoff is
					// specified (VDS_LFP_high_fc == 0) - "500 Hz is enough frequency resolution", as the original
					// code assumed. It is a DEFAULT ONLY, not a floor: a caller-supplied cutoff is respected as
					// given, however low, and is constrained only by the sf/FILT_VDS_MIN_SAMPS_PER_CYCLE_m13 cap
					// below, which is a representability limit rather than a policy.
// FILT_VDS_MEDIAN_DECIM_WINDOW_m13 is GONE (2026-08-01). It gated a decimate-and-interpolate approximation of the
// running median. Measured: FILT_quantfilt_m13() costs 8.7 ns/sample at span 5, 18.3 at span 50, 41.1 at span 300
// - a 20x span increase (5 -> 100) costs only 2.6x - so with the 500 Hz floor above bounding span, the exact median
// is always affordable. The approximation saved ~2% of encode time at span 50 while shifting the BASELINE (and so
// the threshold) by up to 11%, which stopped being acceptable once the median became the template itself.

// Quantfilt Tail Options
#define FILT_TRUNCATE_m13			1
#define FILT_EXTRAPOLATE_m13			2
#define FILT_ZEROPAD_m13			3
#define FILT_DEFAULT_TAIL_OPTION_CODE_m13	FILT_TRUNCATE_m13

// Macros
#define FILT_ABS_m13(x)				( (x) >= ((sf8) 0.0) ? (x) : (-x) )
#define FILT_SIGN_m13(x, y)			( (y) >= ((sf8) 0.0) ? FILT_ABS_m13(x) : -FILT_ABS_m13(x) ) // y = abs(x)
#define FILT_POLES_m13(order, cutoffs)		( order * cutoffs )
#define FILT_FILT_PAD_SAMPLES_m13(poles)	( poles * FILT_PAD_SAMPLES_PER_POLE_m13 * 2 )
#define FILT_OFFSET_ORIG_DATA_m13(filtps)	( filtps->filt_data + (filtps->n_poles * FILT_PAD_SAMPLES_PER_POLE_m13) )

// Typedefs & Structs
typedef struct FILTPS_m13 {
	ui4	behavior;
	si4	order;
	si4	n_poles; // n_poles == order * n_cutoffs
	si4	type;
	sf8	sampling_frequency;
	si8	data_length;
	sf8	cutoffs[2];
	sf8	*numerators; // entries == n_poles + 1
	sf8	*denominators; // entries == n_poles + 1
	sf8	*initial_conditions; // entries == n_poles
	sf8	*orig_data;
	sf8	*filt_data;
	sf8	*buffer;
} FILTPS_m13;

typedef struct {
	sf8	real;
	sf8	imag;
} FILT_COMPLEX_m13;

typedef struct FILT_NODE_STRUCT {
	sf8				val;
	struct FILT_NODE_STRUCT		*prev, *next;
} FILT_NODE_m13;

typedef struct {
	// pure sliding-window machinery - NO caller buffers: the quantfilt functions are general & know
	// nothing about where samples come from or where outputs go (channel structures, matrix rows,
	// files, ...); the caller owns all data buffers
	// single-thread use by caller contract (no locking); NOT memcpy-relocatable (the list points into
	// the by-value sentinels below - reference a QUANTFILT_DATA_m13 by pointer, never move it by copy)
	si8		span, oldest_idx;
	sf8		quantile, low_val_q, high_val_q; // steady-state interpolation weights
	FILT_NODE_m13	*nodes; // span + 1 entries (the + 1 is the empty spare that receives each incoming sample)
	FILT_NODE_m13	head, tail; // list sentinels (-DBL_MAX / DBL_MAX)
	FILT_NODE_m13	*low_q_node; // node at the low quantile position (maintained incrementally)
	FILT_NODE_m13	*oldest_node; // next node to evict (rotates through the circular node buffer)
	FILT_NODE_m13	*new_node, *prev_new_node; // empty spare & last insertion (insertion search seed)
	sf8		prev_new_val;
	tern		allocated; // structure allocated by FILT_quantfilt_head_m13() => freed by FILT_quantfilt_free_m13()
} QUANTFILT_DATA_m13;


// Prototypes
tern	FILT_balance_m13(sf8 **a, si4 poles);
si4	FILT_butter_m13(FILTPS_m13 *filtps);
void	FILT_complex_div_m13(FILT_COMPLEX_m13 *a, FILT_COMPLEX_m13 *b, FILT_COMPLEX_m13 *quotient);
void	FILT_complex_exp_m13(FILT_COMPLEX_m13 *exponent, FILT_COMPLEX_m13 *ans);
void	FILT_complex_mult_m13(FILT_COMPLEX_m13 *a, FILT_COMPLEX_m13 *b, FILT_COMPLEX_m13 *product);
tern	FILT_elmhes_m13(sf8 **a, si4 poles);
si4	FILT_filtfilt_m13(FILTPS_m13 *filtps);
tern	FILT_free_CPS_m13(CPS_m13 *cps, tern free_orig_data, tern free_filt_data, tern free_buffer);
tern	FILT_free_m13(FILTPS_m13 **filtps_ptr, tern free_orig_data, tern free_filt_data, tern free_buffer);
FILTPS_m13 *FILT_init_m13(si4 order, si4 type, sf8 samp_freq, si8 data_len, tern alloc_orig_data, tern alloc_filt_data, tern alloc_buffer, ui4 behavior_on_fail, sf8 cutoff_1, ...);
tern	FILT_generate_initial_conditions_m13(FILTPS_m13 *filtps);
tern	FILT_hqr_m13(sf8 **a, si4 poles, FILT_COMPLEX_m13 *eigs);
tern	FILT_invert_matrix_m13(sf8 **a, sf8 **inv_a, si4 order);
sf8	FILT_line_noise_m13(sf8 *y, sf8 *fy, si8 len, sf8 samp_freq, sf8 line_freq, si8 cycles_per_template, tern calculate_score, tern fast_mode, CMP_BUFFERS_m13 *lnf_buffers);
sf8	FILT_line_noise_estimate_m13(si4 *data, si8 len, sf8 samp_freq, sf8 line_freq, CMP_BUFFERS_m13 *est_buffers);
void	FILT_mat_mult_m13(void *a, void *b, void *product, si4 outer_dim1, si4 inner_dim, si4 outer_dim2);
sf8	*FILT_moving_average_m13(sf8 *x, sf8 *ax, si8 len, si8 span, si1 tail_option_code);
sf8	*FILT_noise_floor_m13(sf8 *data, sf8 *filt_data, si8 data_len, sf8 rel_thresh, sf8 abs_thresh, CMP_BUFFERS_m13 *nff_buffers);
sf8	*FILT_quantfilt_m13(sf8 *x, sf8 *qx, si8 len, sf8 quantile, si8 span, si4 tail_option_code, ...); // varargs(tail_option_code negative): FILT_NODE_m13 *nodes (span + 1 entries, uninitialized OK; not freed)
									// Note: tail_option_code must be si4 (not si1): va_start() on a parameter that undergoes default argument promotion is undefined behavior
// Real-time sliding quantile (split form of FILT_quantfilt_m13(), edge semantics == FILT_TRUNCATE_m13):
// head builds the first window (start of recording, or after any discontinuity) from exactly "span"
// caller-accumulated samples & emits the (span + 1) / 2 left-edge outputs (centered, expanding windows);
// mid is then called once per sample - one in, one out, the output lagging span / 2 samples (the centered
// quantile's group delay); tail flushes the span / 2 right-edge outputs (shrinking windows, final value ==
// the last raw sample) at end of recording or before a discontinuity, & leaves qd reusable via head
// head + (mid * (len - span)) + tail produces BIT-IDENTICAL output to the full function with TRUNCATE
QUANTFILT_DATA_m13	*FILT_quantfilt_head_m13(QUANTFILT_DATA_m13 *qd, const sf8 *x, sf8 *qx, si8 span, sf8 quantile); // consumes x[0..span); writes (span + 1) / 2 outputs to qx; qd allocated if NULL; returns qd primed for mid (NULL on failure)
sf8	FILT_quantfilt_mid_m13(QUANTFILT_DATA_m13 *qd, sf8 new_val); // one sample in, one (centered, span / 2 delayed) quantile out
si8	FILT_quantfilt_tail_m13(QUANTFILT_DATA_m13 *qd, sf8 *qx); // writes the span / 2 right-edge outputs to qx; returns the count; qd then reusable via head
void	FILT_quantfilt_free_m13(QUANTFILT_DATA_m13 **qd_ptr); // frees nodes & (if library-allocated) the structure; NULLs the caller's pointer

// Real-time zero-phase filtering (split form of FILT_filtfilt_m13()). Zero-phase IIR is ANTICAUSAL -
// every output formally depends on the entire future - so exact streaming is impossible; instead the
// anticausal (backward-pass) response is truncated at L samples, where L is measured from the filter's
// own impulse response at head time so the truncated tail is below "tolerance" (relative attenuation).
// Outputs therefore lag L samples & match the offline function to within tolerance (sub-integer-LSB at
// the default) - & the final outputs (from tail) match the offline function EXACTLY (same edge handling).
// The sharper / lower-frequency the filter, the larger L: that is the causal price of zero phase, not an
// implementation artifact. Latency & cost are set at head time:
//   chunk == 1: strict one-in/one-out after priming (per-sample backward sweep, O(poles * L) / sample)
//   chunk == 0 (default => L): burst emission every L samples (amortized ~2x a plain forward filter)
// The FILTPS (coefficients, initial conditions - build with FILT_init_m13()) is REFERENCED, not owned.
// Single-thread use by caller contract; not memcpy-relocatable while active (heap ring).
typedef struct {
	FILTPS_m13	*filtps; // caller-built & caller-owned (FILT_init_m13()); only num/den/zi/n_poles read
	si8		L; // anticausal truncation length (measured from the impulse response at head time)
	si8		chunk; // emission chunk (see above)
	sf8		tolerance; // relative attenuation used to size L
	sf8		zc[FILT_MAX_ORDER_m13 * 2]; // forward filter state (direct form II transposed)
	sf8		*fwd_ring; // L + chunk forward-filtered samples (circular)
	si8		ring_len, w_idx, fill; // ring geometry: write index & current fill
	sf8		raw_tail[(FILT_MAX_ORDER_m13 * 2 * FILT_PAD_SAMPLES_PER_POLE_m13) + 1]; // last pad_len + 1 RAW samples (circular; for tail's reflection pad)
	si8		raw_idx, raw_fill;
	tern		allocated; // structure allocated by FILT_filtfilt_head_m13() => freed by FILT_filtfilt_free_m13()
} FILTFILT_DATA_m13;

FILTFILT_DATA_m13	*FILT_filtfilt_head_m13(FILTFILT_DATA_m13 *fd, FILTPS_m13 *filtps, const sf8 *x, sf8 *qx, si8 n, sf8 tolerance, si8 chunk, si8 *n_out); // consumes x[0..n) (n >= pad_len + 1 == (n_poles * 3) + 1); builds the offline left edge (reflection pad + zi), sizes L, emits any ready outputs to qx (*n_out); fd allocated if NULL; NULL on failure
si8	FILT_filtfilt_mid_m13(FILTFILT_DATA_m13 *fd, sf8 new_val, sf8 *qx); // one sample in; writes 0 or "chunk" outputs (L samples delayed) to qx; returns the count
si8	FILT_filtfilt_tail_m13(FILTFILT_DATA_m13 *fd, sf8 *qx); // flushes all pending outputs with the offline right-edge handling (EXACT match); returns the count; fd then reusable via head
void	FILT_filtfilt_free_m13(FILTFILT_DATA_m13 **fd_ptr); // frees the ring & (if library-allocated) the structure; NULLs the caller's pointer
si4	FILT_sf8_sort_m13(const void *n1, const void *n2);
tern	FILT_show_processing_struct_m13(FILTPS_m13 *filt_ps);
tern	FILT_unsymmeig_m13(sf8 **a, si4 poles, FILT_COMPLEX_m13 *eigs);



//**********************************************************************************//
// MARK: Data Matrix (DM)
//**********************************************************************************//

//	Extent Mode (EXTMD) Flags:
//
//	DM_EXTMD_ABSOLUTE_LIMITS_m13: all samples that exist between slice is start & end (specified by either time or sample number) are returned
//		Time specified, no padding requested:
//			read session by time
//		Time specified, padding requested:
//			read session by time, discontinuous regions padded
//		Samples specified, no padding requested:
//			read session by sample numbers
//		Samples specified, padding requested:
//			NOTE: There is potential for huge discontinuities being padded in output matrix, so samples are treated as relative.
//			NOTE: If discontinuity does ocurr in the specified slice, a warning message is displayed that relative output was produced.
//			start_time = time of start sample number
//			duration = (end_sample_number - start_sample_number) / ref_chan_samp_freq;
//			end_time = start_time + duration
//			read session by time, discontinuous regions padded
//
//	DM_EXTMD_RELATIVE_LIMITS_m13: slice start is treated as absolute, slice end is treated as indication of desired slice extents as follows:
//		Time specified, no padding requested:
//			duration = (end_time - start_time)
//			start_sample_number = sample number of start time
//			end_sample_number = start_sample_number + (ref_chan_samp_freq * duration)
//			read session by sample numbers
//		Time specified, padding requested:
//			read session by time, discontinuous regions padded
//		Samples specified, no padding requested:
//			read session by sample numbers
//		Samples specified, padding requested:
//			start_time = time of start sample number
//			duration = (end_sample_number - start_sample_number) / ref_chan_samp_freq;
//			end_time = start_time + duration
//			read session by time, discontinuous regions padded
//
//	Matrix output size can be specified by sample count or resultant sampling frequency:
//	DM_EXTMD_SAMP_COUNT_m13: output sample_count specified when passing matrix
//	DM_EXTMD_SAMP_FREQ_m13: output sampling frequency specified when passing matrix ( == valid_samples / valid_time in slice - not affected by discontinuities)
//
//	These two modes are driven by to general use cases:
//	1) want matrix of this specific size (e.g. viewer window where sample count == pixel columns)
//	2) want all channels at the specified sampling frequency (e.g algorithm looking across channels)


// Flag Definitions:
#define DM_NO_FLAGS_m13				((ui8) NO_FLAGS_m13)
#define DM_TYPE_SI2_m13				((ui8) 1 << 1)
#define DM_TYPE_SI4_m13				((ui8) 1 << 2)
#define DM_TYPE_SF4_m13				((ui8) 1 << 3)
#define DM_TYPE_SF8_m13				((ui8) 1 << 4)
#define DM_TYPE_MASK_m13			( DM_TYPE_SI2_m13 | DM_TYPE_SI4_m13 | DM_TYPE_SF4_m13 | DM_TYPE_SF8_m13 )
#define DM_2D_INDEXING_m13			((ui8) 1 << 7)  // include array of pointers so that matrix[x][y] indexing is possible (expensive with large major dinensions)
#define DM_FMT_SAMPLE_MAJOR_m13			((ui8) 1 << 8)
#define DM_FMT_CHANNEL_MAJOR_m13		((ui8) 1 << 9)
#define DM_FMT_MASK_m13				( DM_FMT_SAMPLE_MAJOR_m13 | DM_FMT_CHANNEL_MAJOR_m13 )
#define DM_EXTMD_SAMP_COUNT_m13			((ui8) 1 << 12)
#define DM_EXTMD_SAMP_FREQ_m13			((ui8) 1 << 13)
#define DM_EXTMD_MASK_m13			( DM_EXTMD_SAMP_COUNT_m13 | DM_EXTMD_SAMP_FREQ_m13 )
#define DM_EXTMD_ABSOLUTE_LIMITS_m13		((ui8) 1 << 14)
#define DM_EXTMD_RELATIVE_LIMITS_m13		((ui8) 1 << 15)
#define DM_EXTMD_LIMIT_MASK_m13			( DM_EXTMD_ABSOLUTE_LIMITS_m13 | DM_EXTMD_RELATIVE_LIMITS_m13 )
#define DM_SCALE_m13				((ui8) 1 << 18)  // output multiplied by this number (unless 0 or 1)
#define DM_FILT_LOWPASS_m13			((ui8) 1 << 20)  // low cutoff == vararg 1
#define DM_FILT_HIGHPASS_m13			((ui8) 1 << 21)  // high cutoff == vararg 1
#define DM_FILT_BANDPASS_m13			((ui8) 1 << 22)  // low cutoff == vararg 1, high cutoff == vararg 2
#define DM_FILT_BANDSTOP_m13			((ui8) 1 << 23)  // low cutoff == vararg 1, high cutoff == vararg 2
#define DM_FILT_ANTIALIAS_m13			((ui8) 1 << 24)  // lowpass with high cutoff computed, no varargs
#define DM_FILT_CUTOFFS_MASK_m13		( DM_FILT_LOWPASS_m13 | DM_FILT_HIGHPASS_m13 | DM_FILT_BANDPASS_m13 | DM_FILT_BANDSTOP_m13 )
#define DM_FILT_MASK_m13			( DM_FILT_CUTOFFS_MASK_m13 | DM_FILT_ANTIALIAS_m13 )
#define DM_INTRP_LINEAR_m13			((ui8) 1 << 28)
#define DM_INTRP_MAKIMA_m13			((ui8) 1 << 29)
#define DM_INTRP_SPLINE_m13			((ui8) 1 << 30)
#define DM_INTRP_UP_MAKIMA_DN_LINEAR_m13	((ui8) 1 << 31)
#define DM_INTRP_MAKIMA_UPSAMPLE_SF_RATIO_m13	((sf8) 1.5)
#define DM_INTRP_UP_SPLINE_DN_LINEAR_m13	((ui8) 1 << 32)  // if sampling frequency ratio >= DM_INTRP_SPLINE_UPSAMPLE_SF_RATIO_m13
#define DM_INTRP_SPLINE_UPSAMPLE_SF_RATIO_m13	CMP_SPLINE_UPSAMPLE_SF_RATIO_m13 // require (out_sf / in_sf) be >= this before spline upsampling, if lower use linear (prevents unnatural spline turns)
#define DM_INTRP_BINTRP_MDPT_m13		((ui8) 1 << 33)  // binterpolate with midpoint center mode
#define DM_INTRP_BINTRP_MEAN_m13		((ui8) 1 << 34)  // binterpolate with mean center mode
#define DM_INTRP_BINTRP_MEDN_m13		((ui8) 1 << 35)  // binterpolate with median center mode
#define DM_INTRP_BINTRP_FAST_m13		((ui8) 1 << 36)  // binterpolate with fast center mode
#define DM_INTRP_BINTRP_MASK_d1			( DM_INTRP_BINTRP_MDPT_m13 | DM_INTRP_BINTRP_MEAN_m13 | DM_INTRP_BINTRP_MEDN_m13 | DM_INTRP_BINTRP_FAST_m13 )
#define DM_INTRP_MASK_m13			( DM_INTRP_LINEAR_m13 | DM_INTRP_MAKIMA_m13 | DM_INTRP_SPLINE_m13 | DM_INTRP_UP_MAKIMA_DN_LINEAR_m13 | \
						DM_INTRP_UP_SPLINE_DN_LINEAR_m13 | DM_INTRP_BINTRP_MASK_d1 )
#define DM_TRACE_RANGES_m13			((ui8) 1 << 40)  // return bin minima & maxima (equal in size, type, & format to data matrix)
#define DM_TRACE_EXTREMA_m13			((ui8) 1 << 41)  // return minima & maxima values also (minimum & maximum per channel, same type as data matrix)
#define DM_DETREND_m13				((ui8) 1 << 42)  // detrend traces (and trace range matrices if DM_TRACE_RANGES_m13 is set)
#define DM_DSCNT_CONTIG_m13			((ui8) 1 << 48)  // return contigua
#define DM_DSCNT_NAN_m13			((ui8) 1 << 49)  // fill absent samples with NaNs (locations specified in returned arrays)
#define DM_DSCNT_ZERO_m13			((ui8) 1 << 50)  // fill absent samples with zeros (locations specified in returned arrays)
#define DM_PAD_MASK_m13				( DM_DSCNT_NAN_m13 | DM_DSCNT_ZERO_m13 )
#define DM_DSCNT_MASK_m13			( DM_DSCNT_CONTIG_m13 | DM_PAD_MASK_m13 )

// Non-flag defines
#define DM_MAXIMUM_INPUT_FREQUENCY_m13		((sf8) -3.0) // value chosen to distinguish from FREQUENCY_NO_ENTRY_m13 (-1.0) & RATE_VARIABLE_m13 (-2.0)
#define DM_MAXIMUM_INPUT_COUNT_m13		((si8) -3) // value chosen to parallel DM_MAXIMUM_INPUT_FREQUENCY_m13 & not conflict with NUMBER_OF_SAMPLES_NO_ENTRY_m13 (-1)


// Note: if arrays are allocted as 2D arrays, array[0] is beginning of one dimensional array containing (channel_count * sample_count) values of specfified type
typedef struct {
	si8		channel_count;  // defines dimension of allocated matrix: updated based on active channels
	si8		sample_count;  // defines dimension of allocated matrix: if extent mode (EXTMD) == DM_EXTMD_SAMP_FREQ_m13, resultant sample count filled in
	sf8		sampling_frequency; // defines dimension of allocated matrix: if extent mode (EXTMD) == DM_EXTMD_SAMP_COUNT_m13, resultant sampling frequenc filled in
	sf8		scale_factor;  // optionally passed (can be passed in varargs), always returned
	sf8		filter_low_fc;  // optionally passed (can be passed in varargs), always returned
	sf8		filter_high_fc;  // optionally passed (can be passed in varargs), always returned
	void		*data;	  // alloced / realloced as needed (cast to type * or type **, if DM_2D_INDEXING_m13 is set)
	void		*range_minima;  // alloced / realloced as needed, present if DM_TRACE_RANGES_m13 bit is set, otherwise NULL (cast to type * or type **, if DM_2D_INDEXING_m13 is set)
	void		*range_maxima;  // alloced / realloced as needed, present if DM_TRACE_RANGES_m13 bit is set, otherwise NULL (cast to type * or type **, if DM_2D_INDEXING_m13 is set)
	void		*trace_minima;  // alloced / realloced as needed, present if DM_TRACE_EXTREMA_m13 bit is set, otherwise NULL (cast to type *)
	void		*trace_maxima;  // alloced / realloced as needed, present if DM_TRACE_EXTREMA_m13 bit is set, otherwise NULL (cast to type *)
	si4		n_contigua;
	CONTIGUON_m13	*contigua;  // sample indexes in matrix frame
 // internal processing elements
	si8		valid_sample_count;  // used with padding options
	ui8		flags;
	si8		maj_dim;
	si8		min_dim;
	si8		el_size;
	si8		data_bytes;
	si8		n_proc_bufs;
	CMP_BUFFERS_m13	**in_bufs;
	CMP_BUFFERS_m13	**out_bufs;
	CMP_BUFFERS_m13	**mak_in_bufs;
	CMP_BUFFERS_m13	**mak_out_bufs;
	CMP_BUFFERS_m13	**spline_bufs;
	FILTPS_m13	**filt_ps;  // cached per-channel filters; reused across calls while design params (order/type/freq/cutoffs) are unchanged, rebuilt when they change
} DATA_MATRIX_m13;

typedef struct {
	DATA_MATRIX_m13	*dm;
	CHAN_m13	*chan;
	si8		chan_idx;
} DM_CHANNEL_THREAD_INFO_m13;


// Prototypes
pthread_rval_m13	DM_channel_thread_m13(void *ptr);
tern			DM_free_matrix_m13(DATA_MATRIX_m13 **matrix);
DATA_MATRIX_m13 	*DM_get_matrix_m13(DATA_MATRIX_m13 *matrix, SESS_m13 *sess, SLICE_m13 *slice, si4 varargs, ...); // can't use tern to flag varargs (undefined behavior)
// DM_get_matrix_m13() varargs: si8 sample_count, sf8 sampling_frequency, ui8 flags, sf8 scale, sf8 fc1, sf8 fc2
//
// IMPORTANT: pass correct types for varargs - compiler cannot promote / convert to proper type because it doesn't know what they should be
//
// varargs DM_FILT_LOWPASS_m13 set: fc1 == high_cutoff
// varargs DM_FILT_HIGHPASS_m13 set: fc1 == low_cutoff
// varargs DM_FILT_BANDPASS_m13 set: fc1 == low_cutoff, fc2 == high_cutoff
// varargs DM_FILT_BANDSTOP_m13 set: fc1 == low_cutoff, fc2 == high_cutoff
tern			DM_show_flags_m13(ui8 flags);
DATA_MATRIX_m13		*DM_transpose_m13(DATA_MATRIX_m13 **in_matrix, DATA_MATRIX_m13 **out_matrix); // if *in_matrix == *out_matrix, done in place; if *out_matrix == NULL, allocated and returned
tern			DM_transpose_in_place_m13(DATA_MATRIX_m13 *matrix, void *base);
tern			DM_transpose_out_of_place_m13(DATA_MATRIX_m13 *in_matrix, DATA_MATRIX_m13 *out_matrix, void *in_base, void *out_base); // used by DM_transpose_m13(), assumes array allocation is taken care of, so use independently with care



//**********************************************************************************//
// MARK: Transmission (TR)
//**********************************************************************************//

// Transmission Header Types
// • Type numbers 0-63 reserved for generic transmission types
// • Type numbers 64-255 used for application specific transmission types

#define TR_TYPE_GENERIC_MIN_m13					0
#define TR_TYPE_GENERIC_MAX_m13					63
#define TR_TYPE_APPLICATION_MIN_m13				64
#define TR_TYPE_APPLICATION_MAX_m13				255

// Generic Transmission Header (TH) Types
#define TR_TYPE_NO_ENTRY_m13					((ui1) 0)
#define TR_TYPE_KEEP_ALIVE_m13					((ui1) 1) // discarded if received by recv_transmission(), and waits for next transmission
#define TR_TYPE_ACK_OK_m13					((ui1) 2)
#define TR_TYPE_ACK_RETRANSMIT_m13				((ui1) 3)
#define TR_TYPE_MESSAGE_m13					((ui1) 4)
#define TR_TYPE_OPERATION_SUCCEEDED_m13				((ui1) 5)
#define TR_TYPE_OPERATION_SUCCEEDED_WITH_MESSAGE_m13		((ui1) 6)
#define TR_TYPE_OPERATION_FAILED_m13				((ui1) 7)
#define TR_TYPE_OPERATION_FAILED_WITH_WARNING_MESSAGE_m13	((ui1) 8)
#define TR_TYPE_OPERATION_FAILED_WITH_ERROR_MESSAGE_m13		((ui1) 9)

// Header Message Type Aliases (shorter :)
#define TR_ERROR_TYPE_m13	TR_TYPE_OPERATION_FAILED_WITH_ERROR_MESSAGE_m13
#define TR_WARNING_TYPE_m13	TR_TYPE_OPERATION_FAILED_WITH_WARNING_MESSAGE_m13
#define TR_SUCCESS_TYPE_m13	TR_TYPE_OPERATION_SUCCEEDED_WITH_MESSAGE_m13
#define TR_MESSAGE_TYPE_m13	TR_TYPE_MESSAGE_m13

// Transmission Error Codes (not enum so can make si8s)
#define TR_E_NONE_m13			((si8) E_NONE_m13) // zero (0)
#define TR_E_GEN_m13			((si8) FALSE_m13) // unknown or unspecified error
#define TR_E_SOCK_FAILED_m13		((si8) -2)
#define TR_E_SOCK_OPEN_m13		((si8) -3)
#define TR_E_SOCK_CLOSED_m13		((si8) -4)
#define TR_E_SOCK_TIMED_OUT_m13		((si8) -5)
#define TR_E_DATA_m13			((si8) -6)
#define TR_E_ID_m13			((si8) -7)
#define TR_E_TRANS_FAILED_m13		((si8) -8)
#define TR_E_CRC_m13			((si8) -9)
#define TR_E_ACK_m13			((si8) -10)

// Transmission Error Strings
#define	TR_E_NONE_STR_m13		"no error"
#define	TR_E_GEN_STR_m13		"unspecified transmission error"
#define TR_E_SOCK_FAILED_STR_m13	"socket failed"
#define TR_E_SOCK_OPEN_STR_m13		"could not open socket"
#define TR_E_SOCK_CLOSED_STR_m13	"socket closed"
#define TR_E_SOCK_TIMED_OUT_STR_m13	"socket timed out"
#define TR_E_DATA_STR_m13		"no data available"
#define TR_E_ID_STR_m13			"transmission ID mismatch"
#define TR_E_TRANS_FAILED_STR_m13	"transmission failed"
#define TR_E_CRC_STR_m13		"checksum mismatch"
#define TR_E_ACK_STR_m13		"no acknowlegment"

// Transmission Error Tags
#define	TR_E_NONE_TAG_m13		"TR_E_NONE"
#define	TR_E_GEN_TAG_m13		"TR_E_GEN"
#define TR_E_SOCK_FAILED_TAG_m13	"TR_E_SOCK_FAILED"
#define TR_E_SOCK_OPEN_TAG_m13		"TR_E_SOCK_OPEN"
#define TR_E_SOCK_CLOSED_TAG_m13	"TR_E_SOCK_CLOSED"
#define TR_E_SOCK_TIMED_OUT_TAG_m13	"TR_E_SOCK_TIMED_OUT"
#define TR_E_DATA_TAG_m13		"TR_E_DATA"
#define TR_E_ID_TAG_m13			"TR_E_ID"
#define TR_E_TRANS_FAILED_TAG_m13	"TR_E_TRANS_FAILED"
#define TR_E_CRC_TAG_m13		"TR_E_CRC"
#define TR_E_ACK_TAG_m13		"TR_E_ACK"

// Transmission Flags
#define TR_FLAGS_DEFAULT_m13		((ui2) 0)
#define TR_FLAGS_BIG_ENDIAN_m13		((ui2) 1) // Bit 0 (LITTLE_ENDIAN == 0, BIG_ENDIAN == 1)
#define TR_FLAGS_UDP_m13		((ui2) 1 << 1) // Bit 1 (TCP == 0, UDP == 1)
#define TR_FLAGS_ENCRYPT_m13		((ui2) 1 << 2) // Bit 2 (body only - header is not encrypted)
#define TR_FLAGS_INCLUDE_KEY_m13	((ui2) 1 << 3) // Bit 3 (expanded encryption key included in data - less secure than bilateral prescience of key)
#define TR_FLAGS_CLOSE_m13		((ui2) 1 << 4) // Bit 4 (close socket after send/recv)
#define TR_FLAGS_ACKNOWLEDGE_m13	((ui2) 1 << 5) // Bit 5 (acknowledge receipt with OK or retransmit)
#define TR_FLAGS_CRC_m13		((ui2) 1 << 6) // Bit 6 (calculate/check transmission CRC - last 4 bytes of transmission)
#define TR_FLAGS_NO_DESTRUCT_m13	((ui2) 1 << 7) // Bit 7 (set if local memory should not be altered - applies to encrpyted transmissions & transmissions that exceed TR_MTU_BYTES_m13)
#define TR_FLAGS_TO_FILE_m13		((ui2) 1 << 8) // Bit 8 (set if received data should go to a file rather than buffer - pseudo FTP)

// TR Defaults
#define TR_VERSION_DEFAULT_m13		((ui1) 1)
#define TR_TYPE_DEFAULT_m13		TR_TYPE_NO_ENTRY_m13
#define TR_ID_CODE_NO_ENTRY_m13		0 // ui4
#define TR_ID_CODE_DEFAULT		TR_ID_CODE_NO_ENTRY_m13

// Transmission Message
#define TR_MESSAGE_HDR_BYTES_m13			16
#define TR_MESSAGE_HDR_TIME_OFFSET_m13			0 // si8
#define TR_MESSAGE_HDR_NO_ENTRY_m13			TIME_NO_ENTRY_m13
#define TR_MESSAGE_HDR_MESSAGE_BYTES_OFFSET_m13		8 // si8
#define TR_MESSAGE_HDR_MESSAGE_BYTES_NO_ENTRY_m13	0

// Transmission Header (TH) Format Constants
#define TR_HDR_BYTES_m13				((si8) 32)
#define TR_CRC_OFFSET_m13				0		  // ui4
#define TR_PACKET_BYTES_OFFSET_m13			4		  // ui2
#define TR_FLAGS_OFFSET_m13				6	  // ui2
#define TR_ID_STRING_OFFSET_m13				8	  // ascii[4]
#define TR_ID_CODE_OFFSET_m13				TR_ID_STRING_OFFSET_m13  // ui4
// TR_ID_CODE_NO_ENTRY_m13 defined above
#define TR_TYPE_OFFSET_m13				13	  // ui1
#define TR_SUBTYPE_OFFSET_m13				14	  // ui1
#define TR_VERSION_OFFSET_m13				15	  // ui1
#define TR_VERSION_NO_ENTRY_m13				0
#define TR_TRANSMISSION_BYTES_OFFSET_m13		16		  // ui8
#define TR_TRANSMISSION_BYTES_NO_ENTRY_m13		0
#define TR_OFFSET_OFFSET_m13				24		  // ui8

// Transmission Info Modes [set by TR_send_transmission_m13() & TR_recv_transmission_m13(), used by TR_close_transmission_m13()]
// indicates whether last transmission was a send or receive
#define TR_MODE_NONE_m13		0
#define TR_MODE_SEND_m13		1
#define TR_MODE_RECV_m13		2
#define TR_MODE_FORCE_CLOSE_m13		3 // set to force close a (TCP) socket

// Miscellaneous
#define TR_INET_MSS_BYTES_m13		1376	// highest multiple of 16, that stays below internet standard frame size (1500) minus [32 (TR header) + 40 (TCP/IP header)
						// + some extra (possible intermediary protocols like GRE, IPsec, PPPoE, or SNAP that may be in the route)]
#define TR_LO_MSS_BYTES_m13		65456 // highest multiple of 16, that stays below backplane (loopback) standard frame size (65535) minus [32 (TR header) + 40 (TCP/IP header)])
#define TR_PORT_STRLEN_m13		8
#define TR_TIMEOUT_NEVER_m13		((sf4) 0.0)
#define TR_PORT_ANY_m13			0 // system assigned port
#define TR_IFACE_ANY_m13		((void *) 0) // all interfaces
#define TR_IFACE_DFLT_m13		"" // default internet interface
#define TR_RETRANSMIT_ATTEMPTS_m13	3


// Typedefs
typedef struct {
	ui4	crc;
	ui2	packet_bytes; // bytes in this packet (including header)
	ui2	flags;
	union {
		struct {
			si1	ID_string[TYPE_BYTES_m13]; // transmission ID is typically application specific
			ui1	type; // transmission type (general [0-63] or transmission ID specific [64-255])
			ui1	subtype; // rarely used
			ui1	version; // transmission header version
		};
		struct {
			ui4	ID_code; // transmission ID is typically application specific
			ui4	combined_check; // use to to check [terminal zero of ID_string, type, subtype, version] as a ui4
		};
	};
	si8	transmission_bytes; // full size of tramsmitted data in bytes (*** does not include header ***)
	si8	offset; // offset (in bytes) of packet data into full data (*** does not include header ***)
} TR_HDR_m13;

// transmission header layout (compile-time; see LAYOUT_*_m13 note)
LAYOUT_SIZE_m13(TR_HDR_m13, TR_HDR_BYTES_m13);
LAYOUT_FIELD_m13(TR_HDR_m13, crc, 0);
LAYOUT_FIELD_m13(TR_HDR_m13, packet_bytes, 4);
LAYOUT_FIELD_m13(TR_HDR_m13, flags, 6);
LAYOUT_FIELD_m13(TR_HDR_m13, ID_string, 8);
LAYOUT_FIELD_m13(TR_HDR_m13, ID_code, 8);
LAYOUT_FIELD_m13(TR_HDR_m13, type, 13);
LAYOUT_FIELD_m13(TR_HDR_m13, subtype, 14);
LAYOUT_FIELD_m13(TR_HDR_m13, version, 15);
LAYOUT_FIELD_m13(TR_HDR_m13, combined_check, 12);
LAYOUT_FIELD_m13(TR_HDR_m13, transmission_bytes, 16);
LAYOUT_FIELD_m13(TR_HDR_m13, offset, 24);

typedef struct {
	union {
		ui1		*buffer; // used internally, first portion is the transmission header
		TR_HDR_m13	*header;
	};
	si8	buffer_bytes; // bytes available for data (actual allocation also includes room for header)
	ui1	*data; // buffer + TR_HDR_BYTES_m13
	si1	*password; // for encryption (NOT freed by TR_free_transmission_info_m13)
	ui1	*expanded_key; // for encryption
	ui1	*inv_expanded_key; // for decryption (equivalent-inverse-cipher schedule, rebuilt whenever expanded_key changes; allocated & freed with expanded_key per expanded_key_allocated)
	tern	expanded_key_allocated; // determines whether to free expanded key
	ui1	mode; // TR_MODE_SEND_m13, TR_MODE_RECV_m13, TR_MODE_NONE_m13 (needed to properly close TCP sockets)
	si4	sock_fd;
	si4	sock_family; // AF_INET or AF_INET6 of the current socket (runtime only, not persisted); 0 (AF_UNSPEC) == none yet
	si1	dest_addr[INET6_ADDRSTRLEN]; // INET6_ADDRSTRLEN == 46 (this can be an IP address string or or a domain name [< 46 characters])
	ui2	dest_port;
	si1	iface_addr[INET6_ADDRSTRLEN]; // zero-length string for any default internet interface
	ui2	iface_port;
	sf4	timeout; // seconds
	ui2	mss; // maximum segment size (max bytes of data per packet [*** does not include header ***]) (typically multiple of 16, must be at least multiple of 8 for library)
} TR_INFO_m13;

typedef struct {
	si8	time;  // uutc
	si8	message_bytes; // includes text, & pad bytes, NOT header bytes
} TR_MESSAGE_HDR_m13; // text follows structure, padded with zeros to 16 byte alignment


// Function Prototypes
TR_INFO_m13	*TR_alloc_trans_info_m13(si8 buffer_bytes, ui4 ID_code, ui1 header_flags, sf4 timeout, const si1 *password);
tern		TR_bind_m13(TR_INFO_m13 *trans_info, si1 *iface_addr, ui2 iface_port);
tern		TR_build_message_m13(TR_MESSAGE_HDR_m13 *msg, const si1 *message_text);
tern		TR_close_transmission_m13(TR_INFO_m13 *trans_info);
tern		TR_connect_m13(TR_INFO_m13 *trans_info, const si1 *dest_addr, ui2 dest_port);
tern		TR_connect_to_server_m13(TR_INFO_m13 *trans_info, const si1 *dest_addr, ui2 dest_port);
tern		TR_create_socket_m13(TR_INFO_m13 *trans_info); // uses trans_info->sock_family (defaults to AF_INET if unset)
tern		TR_ensure_socket_family_m13(TR_INFO_m13 *trans_info, si4 af); // (re)create the socket if it is missing or of the wrong family
tern		TR_make_sockaddr_m13(void *sockaddr_storage_ptr, ui4 *sa_len, si4 af, const si1 *addr_str, ui2 port); // fill a sockaddr_in/in6 for either family (NULL/empty addr_str => wildcard)
tern		TR_free_transmission_info_m13(TR_INFO_m13 **trans_info_ptr);
tern		TR_realloc_trans_info_m13(TR_INFO_m13 *trans_info, si8 buffer_bytes, TR_HDR_m13 **caller_header);
si8		TR_recv_transmission_m13(TR_INFO_m13 *trans_info, TR_HDR_m13 **caller_header); // receive may reallocate, pass caller header to have function set local variable, otherwise pass NULL, can do manually
tern		TR_send_message_m13(TR_INFO_m13 *trans_info, ui1 type, tern encrypt, const si1 *fmt, ...);
si8		TR_send_transmission_m13(TR_INFO_m13 *trans_info);
tern		TR_set_socket_blocking_m13(TR_INFO_m13 *trans_info, tern set);
tern		TR_set_socket_broadcast_m13(TR_INFO_m13 *trans_info, tern set);
tern		TR_set_socket_reuse_address_m13(TR_INFO_m13 *trans_info, tern set);
tern		TR_set_socket_reuse_port_m13(TR_INFO_m13 *trans_info, tern set);
tern		TR_set_socket_timeout_m13(TR_INFO_m13 *trans_info);
tern		TR_show_message_m13(TR_HDR_m13 *header);
tern		TR_show_transmission_m13(TR_INFO_m13 *trans_info);
si1		*TR_strerror_m13(si4 err_num);



//**********************************************************************************//
// MARK: Time Zone (TZ)
//**********************************************************************************//

// Notes:
//
// Western Sahara:
// DST is on most of the year and off during Ramadan, whose dates change annually in a way that is not accomodated by this schema.
// As Ramadan only lasts a month, and can occur at virtually any time of year, this table treats it as if it's Daylight Time
// is it's Standard Time, and it does not observe DST. So, the displayed time will be off by +1 hours during Ramadan each year.
//
// If it were to have a proper entry, it would look something like:
// { "WESTERN SAHARA", "EH", "ESH", "", "", "WESTERN EUROPEAN TIME", "WET", 0, "WESTERN EUROPEAN DAYLIGHT TIME", "WEDT", 0x3c0002041f00ff01, 0xc40003031300ffff, -1 }
//
// But it is represented here as:
// { "WESTERN SAHARA", "EH", "ESH", "", "", "WESTERN EUROPEAN DAYLIGHT TIME", "WEDT", 3600, "", "", 0x0, 0x0, -1 }

#define TZ_TABLE_ENTRIES_m13 399
#define TZ_TABLE_m13 { \
	{ "AFGHANISTAN", "AF", "AFG", "", "", "AFGHANISTAN TIME", "AFT", 16200, "", "", 0x0, 0x0, -1 }, \
	{ "AKROTIRI", "", "", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00010200060001, 0xC4000209000600FF, -1 }, \
	{ "ALAND ISLANDS", "AX", "ALA", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF, -1 }, \
	{ "ALBANIA", "AL", "ALB", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "ALGERIA", "DZ", "DZA", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "", "", 0x0, 0x0, -1 }, \
	{ "AMERICAN SAMOA", "US", "ASM", "", "", "SAMOA STANDARD TIME", "SST", -39600, "", "", 0x0, 0x0, -1 }, \
	{ "ANDORRA", "AD", "AND", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "ANGOLA", "AO", "AGO", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0, -1 }, \
	{ "ANGUILLA", "AI", "AIA", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "ANTIGUA", "AG", "ATG", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "ARGENTINA", "AR", "ARG", "", "", "ARGENTINA TIME", "ART", -10800, "", "", 0x0, 0x0, -1 }, \
	{ "ARMENIA", "AM", "ARM", "", "", "ARMENIA TIME", "AMT", 14400, "", "", 0x0, 0x0, -1 }, \
	{ "ARUBA", "AW", "ABW", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "ASCENSION", "SH", "SHN", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "AUSTRALIA", "AU", "AUS", "WESTERN AUSTRALIA", "WA", "AUSTRALIAN WESTERN STANDARD TIME", "AWST", 28800, "", "", 0x0, 0x0, -1 }, \
	{ "AUSTRALIA", "AU", "AUS", "WESTERN AUSTRALIA", "WA", "AUSTRALIAN CENTRAL WESTERN STANDARD TIME", "ACWST", 31500, "", "", 0x0, 0x0, -1 }, \
	{ "AUSTRALIA", "AU", "AUS", "SOUTH AUSTRALIA", "SA", "AUSTRALIAN CENTRAL STANDARD TIME", "ACST", 34200, "AUSTRALIAN CENTRAL DAYLIGHT TIME", "ACDT", 0x3C00020900010001, 0xC4000303000100FF, -1 }, \
	{ "AUSTRALIA", "AU", "AUS", "NORTHERN TERRITORY", "NT", "AUSTRALIAN CENTRAL STANDARD TIME", "ACST", 34200, "", "", 0x0, 0x0, -1 }, \
	{ "AUSTRALIA", "AU", "AUS", "AUSTRALIAN CAPITAL TERRITORY", "ACT", "AUSTRALIAN EASTERN STANDARD TIME", "AEST", 36000, "AUSTRALIAN EASTERN DAYLIGHT TIME", "AEDT", 0x3C00020900010001, 0xC4000303000100FF, -1 }, \
	{ "AUSTRALIA", "AU", "AUS", "TASMANIA", "TAS", "AUSTRALIAN EASTERN STANDARD TIME", "AEST", 36000, "AUSTRALIAN EASTERN DAYLIGHT TIME", "AEDT", 0x3C00020900010001, 0xC4000303000100FF, -1 }, \
	{ "AUSTRALIA", "AU", "AUS", "VICTORIA", "VIC", "AUSTRALIAN EASTERN STANDARD TIME", "AEST", 36000, "AUSTRALIAN EASTERN DAYLIGHT TIME", "AEDT", 0x3C00020900010001, 0xC4000303000100FF, -1 }, \
	{ "AUSTRALIA", "AU", "AUS", "NEW SOUTH WALES", "NSW", "AUSTRALIAN EASTERN STANDARD TIME", "AEST", 36000, "AUSTRALIAN EASTERN DAYLIGHT TIME", "AEDT", 0x3C00020900010001, 0xC4000303000100FF, -1 }, \
	{ "AUSTRALIA", "AU", "AUS", "QUEENSLAND", "QLD", "AUSTRALIAN EASTERN STANDARD TIME", "AEST", 36000, "", "", 0x0, 0x0, -1 }, \
	{ "AUSTRALIA", "AU", "AUS", "LORD HOWE ISLAND", "", "LORD HOWE STANDARD TIME", "LHST", 37800, "LORD HOWE DAYLIGHT TIME (+30 MIN)", "LHDT", 0x1E00020900010001, 0xE2000203000100FF, -1 }, \
	{ "AUSTRIA", "AT", "AUT", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "AZERBAIJAN", "AZ", "AZE", "", "", "AZERBAIJAN TIME", "AZT", 14400, "", "", 0x0, 0x0, -1 }, \
	{ "BAHAMAS", "BS", "BHS", "", "", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "BAHRAIN", "BH", "BHR", "", "", "ARABIAN STANDARD TIME", "AST", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "BANGLADESH", "BD", "BGD", "", "", "BANGLADESH STANDARD TIME", "BST", 21600, "", "", 0x0, 0x0, -1 }, \
	{ "BARBADOS", "BB", "BRB", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "BARBUDA", "AG", "ATG", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "BELARUS", "BY", "BLR", "", "", "MOSCOW STANDARD TIME", "MSK", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "BELGIUM", "BE", "BEL", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "BELIZE", "BZ", "BLZ", "", "", "CENTRAL STANDARD TIME", "CST", -21600, "", "", 0x0, 0x0, -1 }, \
	{ "BENIN", "BJ", "BEN", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0, -1 }, \
	{ "BERMUDA", "BM", "BMU", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "ATLANTIC DAYLIGHT TIME", "ADT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "BHUTAN", "BT", "BTN", "", "", "BHUTAN TIME", "BTT", 21600, "", "", 0x0, 0x0, -1 }, \
	{ "BOLIVIA", "BO", "BOL", "", "", "BOLIVIA TIME", "BOT", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "BONAIRE", "BQ", "BES", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "BOSNIA", "BA", "BIH", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "BOTSWANA", "BW", "BWA", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "BOUVET ISLAND", "BV", "BVT", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "BRAZIL", "BR", "BRA", "", "", "ACRE TIME", "ACT", -18000, "", "", 0x0, 0x0, -1 }, \
	{ "BRAZIL", "BR", "BRA", "", "", "AMAZON TIME", "AMT", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "BRAZIL", "BR", "BRA", "", "", "BRASILIA TIME", "BRT", -10800, "", "", 0x0, 0x0, -1 }, \
	{ "BRAZIL", "BR", "BRA", "", "", "FERNANDO DE NORONHA TIME", "FNT", -7200, "", "", 0x0, 0x0, -1 }, \
	{ "BRITISH VIRGIN ISLANDS", "VG", "VGB", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "BRUNEI", "BN", "BRN", "", "", "BRUNEI TIME", "BNT", 28800, "", "", 0x0, 0x0, -1 }, \
	{ "BULGARIA", "BG", "BGR", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF, -1 }, \
	{ "BURKINA FASO", "BF", "BFA", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "BURUNDI", "BI", "BDI", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "CAMBODIA", "KH", "KHM", "", "", "INDOCHINA TIME", "ICT", 25200, "", "", 0x0, 0x0, -1 }, \
	{ "CAMEROON", "CM", "CMR", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0, -1 }, \
	{ "CANADA", "CA", "CAN", "NEWFOUNDLAND", "NL", "NEWFOUNDLAND STANDARD TIME", "NST", -12600, "NEWFOUNDLAND DAYLIGHT TIME", "NDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "CANADA", "CA", "CAN", "LABRADOR", "NL", "ATLANTIC STANDARD TIME", "AST", -14400, "ATLANTIC DAYLIGHT TIME", "ADT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "CANADA", "CA", "CAN", "NEW BRUNSWICK", "NB", "ATLANTIC STANDARD TIME", "AST", -14400, "ATLANTIC DAYLIGHT TIME", "ADT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "CANADA", "CA", "CAN", "NOVA SCOTIA", "NS", "ATLANTIC STANDARD TIME", "AST", -14400, "ATLANTIC DAYLIGHT TIME", "ADT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "CANADA", "CA", "CAN", "PRINCE EDWARD ISLAND", "PE", "ATLANTIC STANDARD TIME", "AST", -14400, "ATLANTIC DAYLIGHT TIME", "ADT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "CANADA", "CA", "CAN", "ONTARIO", "ON", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "CANADA", "CA", "CAN", "QUEBEC", "QC", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "CANADA", "CA", "CAN", "MANITOBA", "MB", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "CANADA", "CA", "CAN", "SASKATCHEWAN", "SK", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "CANADA", "CA", "CAN", "NUNAVUT", "NU", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "CANADA", "CA", "CAN", "ALBERTA", "AB", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "CANADA", "CA", "CAN", "NORTHWEST TERRITORIES ", "NT", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "CANADA", "CA", "CAN", "BRITISH COLUMBIA", "BC", "PACIFIC STANDARD TIME", "PST", -28800, "PACIFIC DAYLIGHT TIME", "PDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "CANADA", "CA", "CAN", "YUKON", "YT", "PACIFIC STANDARD TIME", "PST", -28800, "PACIFIC DAYLIGHT TIME", "PDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "CAPE VERDE", "CV", "CPV", "", "", "CAPE VERDE TIME", "CVT", -3600, "", "", 0x0, 0x0, -1 }, \
	{ "CAYMAN ISLANDS", "KY", "CYM", "", "", "EASTERN STANDARD TIME", "EST", -18000, "", "", 0x0, 0x0, -1 }, \
	{ "CENTRAL AFRICAN REPUBLIC", "CF", "CAF", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0, -1 }, \
	{ "CHAD", "TD", "TCD", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0, -1 }, \
	{ "CHILE", "CL", "CHL", "", "", "CHILE STANDARD TIME", "CLT", -14400, "CHILE SUMMER TIME", "CLST", 0x3C00000800010001, 0xC4000003000100FF, -1 }, \
	{ "CHILE", "CL", "CHL", "EASTER ISLAND", "", "EASTER ISLAND STANDARD TIME", "EAST", -21600, "EASTER ISLAND SUMMER TIME", "EASST", 0x3C00000800010001, 0xC4000003000100FF, -1 }, \
	{ "CHINA", "CN", "CHN", "", "", "CHINA STANDARD TIME", "CST", 28800, "", "", 0x0, 0x0, -1 }, \
	{ "CHRISTMAS ISLAND (AU)", "CX", "CXR", "", "", "CHRISTMAS ISLAND TIME", "CXT", 25200, "", "", 0x0, 0x0, -1 }, \
	{ "COCOS ISLANDS (AU)", "CC", "CCK", "", "", "COCOS ISLAND TIME", "CCT", 23400, "", "", 0x0, 0x0, -1 }, \
	{ "COLOMBIA", "CO", "COL", "", "", "COLOMBIA TIME", "COT", -18000, "", "", 0x0, 0x0, -1 }, \
	{ "COMOROS", "KM", "COM", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "COOK ISLANDS", "CK", "COK", "", "", "COOK ISLAND TIME", "CKT", -36000, "", "", 0x0, 0x0, -1 }, \
	{ "COSTA RICA", "CR", "CRI", "", "", "CENTRAL STANDARD TIME", "CST", -21600, "", "", 0x0, 0x0, -1 }, \
	{ "CROATIA", "HR", "HRV", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600F, -1 }, \
	{ "CUBA", "CU", "CUB", "", "", "CUBA STANDARD TIME", "CST", -18000, "CUBA DAYLIGHT TIME", "CDT", 0x3C00000200020001, 0xC400010A000100FF, -1 }, \
	{ "CURACAO", "CW", "CUW", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "CYPRUS", "CY", "CYP", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF, -1 }, \
	{ "CZECH REPUBLIC", "CZ", "CZE", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "DEMOCRATIC REPUBLIC OF THE CONGO", "CD", "COD", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0, -1 }, \
	{ "DEMOCRATIC REPUBLIC OF THE CONGO", "CD", "COD", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "DENMARK", "DK", "DNK", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "DHEKELIA", "", "", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00010200060001, 0xC4000209000600FF, -1 }, \
	{ "DJIBOUTI", "DJ", "DJI", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "DOMINICA", "DM", "DMA", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "DOMINICAN REPUBLIC", "DO", "DOM", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "EAST TIMOR", "TL", "TLS", "", "", "EAST TIMOR TIME", "TLT", 32400, "", "", 0x0, 0x0, -1 }, \
	{ "ECUADOR", "EC", "ECU", "", "", "ECUADOR TIME", "ECT", -18000, "", "", 0x0, 0x0, -1 }, \
	{ "EGYPT", "EG", "EGY", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "EL SALVADOR", "SV", "SLV", "", "", "CENTRAL STANDARD TIME", "CST", -21600, "", "", 0x0, 0x0, -1 }, \
	{ "EQUATORIAL GUINEA", "GQ", "GNQ", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0, -1 }, \
	{ "ERITREA", "ER", "ERI", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "ESTONIA", "EE", "EST", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF, -1 }, \
	{ "ESWATINI", "SZ", "SWZ", "", "", "SOUTH AFRICA STANDARD TIME", "SAST", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "ETHIOPIA", "ET", "ETH", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "FALKLAND ISLANDS", "FK", "FLK", "", "", "FALKLAND ISLANDS SUMMER TIME", "FKST", -10800, "", "", 0x0, 0x0, -1 }, \
	{ "FAROE ISLANDS", "FO", "FRO", "", "", "WESTERN EUROPEAN TIME", "WET", 0, "WESTERN EUROPEAN DAYLIGHT TIME", "WEDT", 0x3C00010200060001, 0xC4000209000600FF, -1 }, \
	{ "FIJI", "FJ", "FJI", "", "", "FIJI TIME", "FJT", 43200, "FIJI DAYLIGHT TIME", "FJDT", 0x3C00020A00010001, 0xC4000300000300FF, -1 }, \
	{ "FINLAND", "FI", "FIN", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF, -1 }, \
	{ "FRANCE", "FR", "FRA", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "FRENCH GUIANA", "GF", "GUF", "", "", "FRENCH GUIANA TIME", "GFT", -10800, "", "", 0x0, 0x0, -1 }, \
	{ "FRENCH POLYNESIA", "PF", "PYF", "TAHITI", "", "TAHITI TIME", "TAHT", -36000, "", "", 0x0, 0x0, -1 }, \
	{ "FRENCH POLYNESIA", "PF", "PYF", "MARQUESAS", "", "MARQUESAS TIME", "MART", -34200, "", "", 0x0, 0x0, -1 }, \
	{ "FRENCH POLYNESIA", "PF", "PYF", "GAMBIER", "", "GAMBIER TIME", "GAMT", -32400, "", "", 0x0, 0x0, -1 }, \
	{ "FRENCH SOUTHERN TERRITORIES", "TF", "ATF", "", "", "FRENCH SOUTHERN AND ANTARCTIC TIME", "TFT", 18000, "", "", 0x0, 0x0, -1 }, \
	{ "FUTUNA", "WF", "WLF", "", "", "WALLIS AND FUTUNA TIME", "WFT", 43200, "", "", 0x0, 0x0, -1 }, \
	{ "GABON", "GA", "GAB", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0, -1 }, \
	{ "GAMBIA", "GM", "GMB", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "GEORGIA", "GE", "GEO", "", "", "GEORGIA STANDARD TIME", "GET", 14400, "", "", 0x0, 0x0, -1 }, \
	{ "GEORGIA", "GE", "GEO", "", "", "MOSCOW STANDARD TIME", "MSK", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "GERMANY", "DE", "DEU", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "GHANA", "GH", "GHA", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "GIBRALTAR", "GI", "GIB", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "GREECE", "GR", "GRC", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF, -1 }, \
	{ "GREENLAND", "GL", "GRL", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "GREENLAND", "GL", "GRL", "", "", "EASTERN GREENLAND TIME", "EGT", -3600, "EASTERN GREENLAND SUMMER TIME", "EGST", 0x3C00FE0200060001, 0xC400FF09000600FF, -1 }, \
	{ "GREENLAND", "GL", "GRL", "", "", "WESTERN GREENLAND TIME", "WGT", -10800, "WESTERN GREENLAND SUMMER TIME", "WGST", 0x3C00FE0200060001, 0xC400FF09000600FF, -1 }, \
	{ "GREENLAND", "GL", "GRL", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "ATLANTIC DAYLIGHT TIME", "ADT", 0x3C00FE0200060001, 0xC400FF09000600FF, -1 }, \
	{ "GRENADA", "GD", "GRD", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "GRENADINES", "VC", "VCT", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "GUADELOUPE", "GP", "GLP", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "GUAM", "GU", "GUM", "", "", "CHAMORRO STANDARD TIME", "CHST", 36000, "", "", 0x0, 0x0, -1 }, \
	{ "GUATEMALA", "GT", "GTM", "", "", "FRENCH GUIANA TIME", "GFT", -10800, "", "", 0x0, 0x0, -1 }, \
	{ "GUERNSEY", "GG", "GGY", "", "", "GREENWICH MEAN TIME", "GMT", 0, "BRITISH SUMMER TIME", "BST", 0x3C00010200060001, 0xC4000209000600FF, -1 }, \
	{ "GUINEA", "GN", "GIN", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "GUINEA-BISSAU", "GW", "GNB", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "GUYANA", "GY", "GUY", "", "", "GUYANA TIME", "GYT", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "HAITI", "HT", "HTI", "", "", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "HEARD ISLANDS", "HM", "HMD", "", "", "FRENCH SOUTHERN AND ANTARCTIC TIME", "TFT", 18000, "", "", 0x0, 0x0, -1 }, \
	{ "HERZEGOVINA", "BA", "BIH", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "HOLY SEE", "VA", "VAT", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "HONDURAS", "HN", "HND", "", "", "CENTRAL STANDARD TIME", "CST", -21600, "", "", 0x0, 0x0, -1 }, \
	{ "HONG KONG", "HK", "HKG", "", "", "HONG KONG TIME", "HKT", 28800, "", "", 0x0, 0x0, -1 }, \
	{ "HUNGARY", "HU", "HUN", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "ICELAND", "IS", "ISL", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "INDIA", "IN", "IND", "", "", "INDIA TIME ZONE", "IST", 19800, "", "", 0x0, 0x0, -1 }, \
	{ "INDONESIA", "ID", "IDN", "", "", "WESTERN INDONESIAN TIME", "WIB", 25200, "", "", 0x0, 0x0, -1 }, \
	{ "INDONESIA", "ID", "IDN", "", "", "CENTRAL INDONESIAN TIME", "WITA", 28800, "", "", 0x0, 0x0, -1 }, \
	{ "INDONESIA", "ID", "IDN", "", "", "EASTERN INDONESIAN TIME", "WIT", 32400, "", "", 0x0, 0x0, -1 }, \
	{ "IRAN", "IR", "IRN", "", "", "IRAN STANDARD TIME", "IRST", 12600, "IRAN DAYLIGHT TIME", "IRDT", 0x3C0000021600FF01, 0xC40000081600FFFF, -1 }, \
	{ "IRAQ", "IQ", "IRQ", "", "", "ARABIA STANDARD TIME", "AST", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "IRELAND", "IE", "IRL", "", "", "GREENWICH MEAN TIME", "GMT", 0, "IRISH STANDARD TIME", "IST", 0x3C00010200060001, 0xC4000209000600FF, -1 }, \
	{ "ISLE OF MAN", "IM", "IMN", "", "", "GREENWICH MEAN TIME", "GMT", 0, "BRITISH SUMMER TIME", "BST", 0x3C00010200060001, 0xC4000209000600FF, -1 }, \
	{ "ISRAEL", "IL", "ISR", "", "", "ISRAEL STANDARD TIME", "IST", 7200, "ISRAEL DAYLIGHT TIME", "IDT", 0x3C00D20200060001, 0xC4000209000600FF, -1 }, \
	{ "ITALY", "IT", "ITA", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "IVORY COAST", "CI", "CIV", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "JAMAICA", "JM", "JAM", "", "", "EASTERN STANDARD TIME", "EST", -18000, "", "", 0x0, 0x0, -1 }, \
	{ "JAN MAYEN", "SJ", "SJM", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "JAPAN", "JP", "JPN", "", "", "JAPAN STANDARD TIME", "JST", 32400, "", "", 0x0, 0x0, -1 }, \
	{ "JERSEY", "JE", "JEY", "", "", "GREENWICH MEAN TIME", "GMT", 0, "BRITISH SUMMER TIME", "BST", 0x3C00010200060001, 0xC4000209000600FF, -1 }, \
	{ "JORDAN", "JO", "JOR", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00000200060501, 0xC4000109000605FF, -1 }, \
	{ "KAZAKHSTAN", "KZ", "KAZ", "", "", "ORAL TIME", "ORAT", 18000, "", "", 0x0, 0x0, -1 }, \
	{ "KAZAKHSTAN", "KZ", "KAZ", "", "", "ALMA-ATA TIME", "ALMT", 21600, "", "", 0x0, 0x0, -1 }, \
	{ "KEELING ISLANDS", "CC", "CCK", "", "", "COCOS ISLANDS TIME", "CCT", 23400, "", "", 0x0, 0x0, -1 }, \
	{ "KENYA", "KE", "KEN", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "KIRIBATI", "KI", "KIR", "", "", "GILBERT ISLAND TIME", "GILT", 43200, "", "", 0x0, 0x0, -1 }, \
	{ "KIRIBATI", "KI", "KIR", "", "", "PHOENIX ISLAND TIME", "PHOT", 46800, "", "", 0x0, 0x0, -1 }, \
	{ "KIRIBATI", "KI", "KIR", "", "", "LINE ISLANDS TIME", "LINT", 50400, "", "", 0x0, 0x0, -1 }, \
	{ "KOSOVO", "XK", "XKX", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "KUWAIT", "KW", "KWT", "", "", "ARABIA STANDARD TIME", "AST", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "KYRGYZSTAN", "KG", "KGZ", "", "", "KYRGYZSTAN TIME", "KGT", 21600, "", "", 0x0, 0x0, -1 }, \
	{ "LAOS", "LA", "LAO", "", "", "INDOCHINA TIME", "ICT", 25200, "", "", 0x0, 0x0, -1 }, \
	{ "LATVIA", "LV", "LVA", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF, -1 }, \
	{ "LEBANON", "LB", "LBN", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00000200060001, 0xC4000009000600FF, -1 }, \
	{ "LESOTHO", "LS", "LSO", "", "", "SOUTH AFRICA STANDARD TIME", "SAST", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "LIBERIA", "LR", "LBR", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "LIBYA", "LY", "LBY", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "LIECHTENSTEIN", "LI", "LIE", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "LITHUANIA", "LT", "LTU", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF, -1 }, \
	{ "LUXEMBOURG", "LU", "LUX", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "MACAU", "MO", "MAC", "", "", "CHINA STANDARD TIME", "CST", 28800, "", "", 0x0, 0x0, -1 }, \
	{ "MADAGASCAR", "MG", "MDG", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "MALAWI", "MW", "MWI", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "MALAYSIA", "MY", "MYS", "", "", "MALAYSIA TIME", "MYT", 28800, "", "", 0x0, 0x0, -1 }, \
	{ "MALDIVES", "MV", "MDV", "", "", "MALDIVES TIME", "MVT", 18000, "", "", 0x0, 0x0, -1 }, \
	{ "MALI", "ML", "MLI", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "MALTA", "MT", "MLT", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "MARSHALL ISLANDS", "MH", "MHL", "", "", "MARSHALL ISLANDS TIME", "MHT", 43200, "", "", 0x0, 0x0, -1 }, \
	{ "MARTINIQUE", "MQ", "MTQ", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "MAURITANIA", "MR", "MRT", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "MAURITIUS", "MU", "MUS", "", "", "MAURITIUS TIME", "MUT", 14400, "", "", 0x0, 0x0, -1 }, \
	{ "MAYOTTE", "YT", "MYT", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "MCDONALD ISLANDS", "US", "USA", "", "", "ALASKA STANDARD TIME", "AKST", 32400, "ALASKA DAYLIGHT TIME", "AKDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "MCDONALD ISLANDS", "HM", "HMD", "", "", "FRENCH SOUTHERN AND ANTARCTIC TIME", "TFT", 18000, "", "", 0x0, 0x0, -1 }, \
	{ "MEXICO", "MX", "MEX", "", "", "EASTERN STANDARD TIME", "EST", -18000, "", "", 0x0, 0x0, -1 }, \
	{ "MEXICO", "MX", "MEX", "", "", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020300010001, 0xC4000209000600FF, -1 }, \
	{ "MEXICO", "MX", "MEX", "", "", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020300010001, 0xC4000209000600FF, -1 }, \
	{ "MEXICO", "MX", "MEX", "", "", "PACIFIC STANDARD TIME", "PST", -28800, "PACIFIC DAYLIGHT TIME", "PDT", 0x3C00020300010001, 0xC4000209000600FF, -1 }, \
	{ "MICRONESIA", "FM", "FSM", "", "", "CHUUK TIME", "CHUT", 36000, "", "", 0x0, 0x0, -1 }, \
	{ "MICRONESIA", "FM", "FSM", "", "", "POHNPEI STANDARD TIME", "PONT", 39600, "", "", 0x0, 0x0, -1 }, \
	{ "MICRONESIA", "FM", "FSM", "", "", "KOSRAE TIME", "KOST", 39600, "", "", 0x0, 0x0, -1 }, \
	{ "MIDWAY", "UM", "UMI", "", "", "SAMOA STANDARD TIME", "SST", -39600, "", "", 0x0, 0x0, -1 }, \
	{ "MIQUELON", "PM", "SPM", "", "", "PIERRE & MIQUELON STANDARD TIME", "PMST", -10800, "PIERRE & MIQUELON DAYLIGHT TIME", "PMDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "MOLDOVA", "MD", "MDA", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "MONACO", "MC", "MCO", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "MONGOLIA", "MN", "MNG", "", "", "HOVD TIME", "HOVT", 25200, "", "", 0x0, 0x0, -1 }, \
	{ "MONGOLIA", "MN", "MNG", "", "", "ULAANBAATAR TIME", "ULAT", 28800, "", "", 0x0, 0x0, -1 }, \
	{ "MONGOLIA", "MN", "MNG", "", "", "CHOIBALSAN TIME", "CHOT", 28800, "", "", 0x0, 0x0, -1 }, \
	{ "MONTENEGRO", "ME", "MNE", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "MONTSERRAT", "MS", "MSR", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "MOROCCO", "MA", "MAR", "", "", "WESTERN EUROPEAN TIME", "WET", 0, "", "", 0x0, 0x0, -1 }, \
	{ "MOZAMBIQUE", "MZ", "MOZ", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "MYANMAR", "MM", "MMR", "", "", "MYANMAR TIME", "MMT", 23400, "", "", 0x0, 0x0, -1 }, \
	{ "NAMIBIA", "NA", "NAM", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "NAURU", "NR", "NRU", "", "", "NAURU TIME", "NRT", 43200, "", "", 0x0, 0x0, -1 }, \
	{ "NEPAL", "NP", "NPL", "", "", "NEPAL TIME", "NPT", 20700, "", "", 0x0, 0x0, -1 }, \
	{ "NETHERLANDS", "NL", "NLD", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "NETHERLANDS ANTILLES", "AN", "ANT", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "NEVIS", "KN", "KNA", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "NEW CALEDONIA", "NC", "NCL", "", "", "NEW CALEDONIA TIME", "NCT", 39600, "", "", 0x0, 0x0, -1 }, \
	{ "NEW ZEALAND", "NZ", "NZL", "", "", "NEW ZEALAND STANDARD TIME", "NZST", 43200, "NEW ZEALAND DAYLIGHT TIME", "NZDT", 0x3C00020800060001, 0xC4000303000100FF, -1 }, \
	{ "NEW ZEALAND", "NZ", "NZL", "CHATHAM ISLAND", "", "CHATHAM ISLAND STANDARD TIME", "CHAST", 45900, "CHATHAM ISLAND DAYLIGHT TIME", "CHADT", 0x3C00020800060001, 0xC4000303000100FF, -1 }, \
	{ "NICARAGUA", "NI", "NIC", "", "", "CENTRAL STANDARD TIME", "CST", -21600, "", "", 0x0, 0x0, -1 }, \
	{ "NIGER", "NE", "NER", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0, -1 }, \
	{ "NIGERIA", "NG", "NGA", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0, -1 }, \
	{ "NIUE", "NU", "NIU", "", "", "NIUE TIME", "NUT", -39600, "", "", 0x0, 0x0, -1 }, \
	{ "NORFOLK ISLAND", "NF", "NFK", "", "", "NORFOLK TIME", "NFT", 39600, "NORFOLK DAYLIGHT TIME", "NFDT", 0x3C00020900010001, 0xC4000303000100FF, -1 }, \
	{ "NORTH KOREA", "KP", "PRK", "", "", "KOREA STANDARD TIME", "KST", 32400, "", "", 0x0, 0x0, -1 }, \
	{ "NORTH MACEDONIA", "MK", "MKD", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "NORTHERN MARIANA ISLANDS", "MP", "MNP", "", "", "CHAMORRO STANDARD TIME", "CHST", 36000, "", "", 0x0, 0x0, -1 }, \
	{ "NORWAY", "NO", "NOR", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "OMAN", "OM", "OMN", "", "", "GULF STANDARD TIME", "GST", 14400, "", "", 0x0, 0x0, -1 }, \
	{ "PAKISTAN", "PK", "PAK", "", "", "PAKISTAN STANDARD TIME", "PKT", 18000, "", "", 0x0, 0x0, -1 }, \
	{ "PALAU", "PW", "PLW", "", "", "PALAU TIME", "PWT", 32400, "", "", 0x0, 0x0, -1 }, \
	{ "PALESTINE", "PS", "PSE", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00D20200060001, 0xC4000209000600FF, -1 }, \
	{ "PANAMA", "PA", "PAN", "", "", "EASTERN STANDARD TIME", "EST", -18000, "", "", 0x0, 0x0, -1 }, \
	{ "PAPUA NEW GUINEA", "PG", "PNG", "", "", "PAPUA NEW GUINEA TIME", "PGT", 36000, "", "", 0x0, 0x0, -1 }, \
	{ "PAPUA NEW GUINEA", "PG", "PNG", "BOUGAINVILLE", "", "BOUGAINVILLE STANDARD TIME", "BST", 39600, "", "", 0x0, 0x0, -1 }, \
	{ "PARAGUAY", "PY", "PRY", "", "", "PARAGUAY TIME", "PYT", -14400, "PARAGUAY SUMMER TIME", "PYST", 0x3C00000900010001, 0xC4000002000400FF, -1 }, \
	{ "PERU", "PE", "PER", "", "", "PERU TIME", "PET", -18000, "", "", 0x0, 0x0, -1 }, \
	{ "PHILIPPINES", "PH", "PHL", "", "", "PHILIPPINE TIME", "PHT", 28800, "", "", 0x0, 0x0, -1 }, \
	{ "PITCAIRN ISLANDS", "PN", "PCN", "", "", "PITCAIRN STANDARD TIME", "PST", 28800, "", "", 0x0, 0x0, -1 }, \
	{ "POLAND", "PL", "POL", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "PORTUGAL", "PT", "PRT", "", "", "WESTERN EUROPEAN TIME", "WET", 0, "WESTERN EUROPEAN DAYLIGHT TIME", "WEDT", 0x3C00010200060001, 0xC4000209000600FF, -1 }, \
	{ "PORTUGAL", "PT", "PRT", "", "", "AZORES TIME", "AZOT", 3600, "AZORES SUMMER TIME", "AZOST", 0x3C00000200060001, 0xC4000109000600FF, -1 }, \
	{ "PRINCIPE", "ST", "STP", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "PUERTO RICO", "PR", "PRI", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "QATAR", "QA", "QAT", "", "", "ARABIA STANDARD TIME", "AST", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "REPUBLIC OF THE CONGO", "CG", "COG", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0, -1 }, \
	{ "REUNION", "RE", "REU", "", "", "REUNION TIME", "RET", 14400, "", "", 0x0, 0x0, -1 }, \
	{ "ROMANIA", "RO", "ROU", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF, -1 }, \
	{ "RUSSIA", "RU", "RUS", "KALININGRAD", "", "EASTERN EUROPEAN TIME", "EET", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "RUSSIA", "RU", "RUS", "MOSCOW", "", "MOSCOW STANDARD TIME", "MSK", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "RUSSIA", "RU", "RUS", "SAMARA", "", "SAMARA TIME", "SAMT", 14400, "", "", 0x0, 0x0, -1 }, \
	{ "RUSSIA", "RU", "RUS", "YEKATERINBURG", "", "YEKATERINBURG TIME", "YEKT", 18000, "", "", 0x0, 0x0, -1 }, \
	{ "RUSSIA", "RU", "RUS", "OMSK", "", "OMSK STANDARD TIME", "OMST", 21600, "", "", 0x0, 0x0, -1 }, \
	{ "RUSSIA", "RU", "RUS", "KRASNOYARSK", "", "KRASNOYARSK TIME", "KRAT", 25200, "", "", 0x0, 0x0, -1 }, \
	{ "RUSSIA", "RU", "RUS", "NOVOSIBIRSK", "", "NOVOSIBIRSK TIME", "NOVT", 25200, "", "", 0x0, 0x0, -1 }, \
	{ "RUSSIA", "RU", "RUS", "IRKUTSK", "", "IRKUTSK TIME", "IRKT", 28800, "", "", 0x0, 0x0, -1 }, \
	{ "RUSSIA", "RU", "RUS", "YAKUTSK", "", "YAKUTSK TIME", "YAKT", 32400, "", "", 0x0, 0x0, -1 }, \
	{ "RUSSIA", "RU", "RUS", "VLADIVOSTOK", "", "VLADIVOSTOK TIME", "VLAT", 36000, "", "", 0x0, 0x0, -1 }, \
	{ "RUSSIA", "RU", "RUS", "MAGADAN", "", "MAGADAN TIME", "MAGT", 39600, "", "", 0x0, 0x0, -1 }, \
	{ "RUSSIA", "RU", "RUS", "SAKHALIN", "", "SAKHALIN TIME", "SAKT", 39600, "", "", 0x0, 0x0, -1 }, \
	{ "RUSSIA", "RU", "RUS", "SREDNEKOLYMSK", "", "SREDNEKOLYMSK TIME", "SRED", 39600, "", "", 0x0, 0x0, -1 }, \
	{ "RUSSIA", "RU", "RUS", "ANADYR", "", "ANADYR TIME", "ANAT", 43200, "", "", 0x0, 0x0, -1 }, \
	{ "RUSSIA", "RU", "RUS", "KAMCHATKA", "", "KAMCHATKA TIME", "PETT", 43200, "", "", 0x0, 0x0, -1 }, \
	{ "RWANDA", "RW", "RWA", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "SABA", "BQ", "BES", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "SAINT BARTHELEMY", "BL", "BLM", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "SAINT HELENA", "SH", "SHN", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "SAINT KITTS", "KN", "KNA", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "SAINT LUCIA", "LC", "LCA", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "SAINT MARTIN", "MF", "MAF", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "SAINT PIERRE", "PM", "SPM", "", "", "PIERRE & MIQUELON STANDARD TIME", "PMST", -10800, "PIERRE & MIQUELON DAYLIGHT TIME", "PMDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "SAINT VINCENT", "VC", "VCT", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "SAMOA", "WS", "WSM", "", "", "WEST SAMOA TIME", "WST", 46800, "WEST SAMOA TIME", "WST", 0x3C00030800060001, 0xC4000403000100FF, -1 }, \
	{ "SAN MARINO", "SM", "SMR", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "SAO TOME", "ST", "STP", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "SAUDI ARABIA", "SA", "SAU", "", "", "ARABIA STANDARD TIME", "AST", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "SENEGAL", "SN", "SEN", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "SERBIA", "RS", "SRB", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "SEYCHELLES", "SC", "SYC", "", "", "SEYCHELLES TIME", "SCT", 14400, "", "", 0x0, 0x0, -1 }, \
	{ "SIERRA LEONE", "SL", "SLE", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "SINGAPORE", "SG", "SGP", "", "", "SINGAPORE TIME", "SGT", 28800, "", "", 0x0, 0x0, -1 }, \
	{ "SINT EUSTATIUS", "BQ", "BES", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "SINT MAARTEN", "SX", "SXM", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "SLOVAKIA", "SK", "SVK", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "SLOVENIA", "SI", "SVN", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "SOLOMON ISLANDS", "SB", "SLB", "", "", "SOLOMON ISLANDS TIME", "SBT", 39600, "", "", 0x0, 0x0, -1 }, \
	{ "SOMALIA", "SO", "SOM", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "SOUTH AFRICA", "ZA", "ZAF", "", "", "SOUTH AFRICA STANDARD TIME", "SAST", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "SOUTH AFRICA", "ZA", "ZAF", "MARION ISLAND", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "SOUTH GEORGIA ISLAND", "GS", "SGS", "", "", "SOUTH GEORGIA TIME", "GST", -7200, "", "", 0x0, 0x0, -1 }, \
	{ "SOUTH KOREA", "KR", "KOR", "", "", "KOREA STANDARD TIME", "KST", 32400, "", "", 0x0, 0x0, -1 }, \
	{ "SOUTH SANDWICH ISLANDS", "GS", "SGS", "", "", "SOUTH GEORGIA TIME", "GST", -7200, "", "", 0x0, 0x0, -1 }, \
	{ "SOUTH SUDAN", "SS", "SSD", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "SPAIN", "ES", "ESP", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "SPAIN", "ES", "ESP", "", "", "WESTERN EUROPEAN TIME", "WET", 0, "WESTERN EUROPEAN DAYLIGHT TIME", "WEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "SRI LANKA", "LK", "LKA", "", "", "INDIA STANDARD TIME", "IST", 19800, "", "", 0x0, 0x0, -1 }, \
	{ "SUDAN", "SD", "SDN", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "SURINAME", "SR", "SUR", "", "", "SURINAME TIME", "SRT", -10800, "", "", 0x0, 0x0, -1 }, \
	{ "SVALBARD", "SJ", "SJM", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "SWAZILAND", "SZ", "SWZ", "", "", "SOUTH AFRICA STANDARD TIME", "SAST", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "SWEDEN", "SE", "SWE", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "SWITZERLAND", "CH", "CHE", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "SYRIA", "SY", "SYR", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00000200060501, 0xC4000009000605FF, -1 }, \
	{ "TAIWAN", "TW", "TWN", "", "", "CHINA STANDARD TIME", "CST", 28800, "", "", 0x0, 0x0, -1 }, \
	{ "TAJIKISTAN", "TJ", "TJK", "", "", "TAJIKISTAN TIME", "TJT", 18000, "", "", 0x0, 0x0, -1 }, \
	{ "TANZANIA", "TZ", "TZA", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "THAILAND", "TH", "THA", "", "", "INDOCHINA TIME", "ICT", 25200, "", "", 0x0, 0x0, -1 }, \
	{ "TOBAGO", "TT", "TTO", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "TOGO", "TG", "TGO", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "TOKELAU", "TK", "TKL", "", "", "TOKELAU TIME", "TKT", 46800, "", "", 0x0, 0x0, -1 }, \
	{ "TONGA", "TO", "TON", "", "", "TONGA TIME", "TOT", 46800, "", "", 0x0, 0x0, -1 }, \
	{ "TRINIDAD", "TT", "TTO", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "TRISTAN DA CUNHA", "SH", "SHN", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "TUNISIA", "TN", "TUN", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "", "", 0x0, 0x0, -1 }, \
	{ "TURKEY", "TR", "TUR", "", "", "TURKEY TIME", "TRT", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "TURKMENISTAN", "TM", "TKM", "", "", "TURKMENISTAN TIME", "TMT", 18000, "", "", 0x0, 0x0, -1 }, \
	{ "TURKS AND CAICOS", "TC", "TCA", "", "", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "TUVALU", "TV", "TUV", "", "", "TUVALU TIME", "TVT", 43200, "", "", 0x0, 0x0, -1 }, \
	{ "UGANDA", "UG", "UGA", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "UKRAINE", "UA", "UKR", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF, -1 }, \
	{ "UKRAINE", "UA", "UKR", "", "", "MOSCOW STANDARD TIME", "MSK", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "UNITED ARAB EMIRATES", "AE", "ARE", "", "", "GULF STANDARD TIME", "GST", 14400, "", "", 0x0, 0x0, -1 }, \
	{ "UNITED KINGDOM", "GB", "GBR", "", "", "GREENWICH MEAN TIME", "GMT", 0, "BRITISH SUMMER TIME", "BST", 0x3C00010200060001, 0xC4000209000600FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "ALABAMA", "AL", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "ALASKA", "AK", "ALASKA STANDARD TIME", "AKST", -32400, "ALASKA DAYLIGHT TIME", "AKDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "ALASKA", "AK", "HAWAII-ALEUTIAN STANDARD TIME", "HST", -36000, "HAWAII-ALEUTIAN DAYLIGHT TIME", "HDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "ARIZONA", "AZ", "MOUNTAIN STANDARD TIME", "MST", -25200, "", "", 0x0, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "ARKANSAS", "AR", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "CALIFORNIA", "CA", "PACIFIC STANDARD TIME", "PST", -28800, "PACIFIC DAYLIGHT TIME", "PDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "COLORADO", "CO", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "CONNECTICUT", "CT", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "DELAWARE", "DE", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "DISTRICT OF COLUMBIA", "DC", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "FLORIDA", "FL", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "FLORIDA", "FL", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "GEORGIA", "GA", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "HAWAII", "HI", "HAWAII-ALEUTIAN STANDARD TIME", "HST", -36000, "", "", 0x0, 0x0, -1 }, \
	{ "UNITED STATES", "US", "USA", "IDAHO", "ID", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "IDAHO", "ID", "PACIFIC STANDARD TIME", "PST", -28800, "PACIFIC DAYLIGHT TIME", "PDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "ILLINOIS", "IL", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "INDIANA", "IN", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "INDIANA", "IN", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "IOWA", "IA", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "KANSAS", "KS", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "KANSAS", "KS", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "KENTUCKY", "KY", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "KENTUCKY", "KY", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "LOUISIANA", "LA", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "MAINE", "ME", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "MARYLAND", "MD", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "MASSACHUSETTS", "MA", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "MICHIGAN", "MI", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "MICHIGAN", "MI", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "MINNESOTA", "MN", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "MISSISSIPPI", "MS", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "MISSOURI", "MO", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "MONTANA", "MT", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "NEBRASKA", "NE", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "NEBRASKA", "NE", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "NEVADA", "NV", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "NEVADA", "NV", "PACIFIC STANDARD TIME", "PST", -28800, "PACIFIC DAYLIGHT TIME", "PDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "NEW HAMPSHIRE", "NH", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "NEW JERSEY", "NJ", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "NEW MEXICO", "NM", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "NEW YORK", "NY", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "NORTH CAROLINA", "NC", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "NORTH DAKOTA", "ND", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "NORTH DAKOTA", "ND", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "OHIO", "OH", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "OKLAHOMA", "OK", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "OREGON", "OR", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "OREGON", "OR", "PACIFIC STANDARD TIME", "PST", -28800, "PACIFIC DAYLIGHT TIME", "PDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "PENNSYLVANIA", "PA", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "RHODE ISLAND", "RI", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "SOUTH CAROLINA", "SC", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "SOUTH DAKOTA", "SD", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "SOUTH DAKOTA", "SD", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "TENNESSEE", "TN", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "TENNESSEE", "TN", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "TEXAS", "TX", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "TEXAS", "TX", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "UTAH", "UT", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "VERMONT", "VT", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "VIRGINIA", "VA", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "WASHINGTON", "WA", "PACIFIC STANDARD TIME", "PST", -28800, "PACIFIC DAYLIGHT TIME", "PDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "WEST VIRGINIA", "WV", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "WISCONSIN", "WI", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "WYOMING", "WY", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES VIRGIN ISLANDS", "VI", "VIR", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "URUGUAY", "UY", "URY", "", "", "URUGUAY TIME", "UYT", -10800, "", "", 0x0, 0x0, -1 }, \
	{ "UZBEKISTAN", "UZ", "UZB", "", "", "UZBEKISTAN TIME", "UZT", 18000, "", "", 0x0, 0x0, -1 }, \
	{ "VANUATU", "VU", "VUT", "", "", "VANUATU TIME", "VUT", 39600, "", "", 0x0, 0x0, -1 }, \
	{ "VATICAN CITY", "VA", "VAT", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "VENEZUELA", "VE", "VEN", "", "", "VENEZUELAN STANDARD TIME", "VET", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "VIETNAM", "VN", "VNM", "", "", "INDOCHINA TIME", "ICT", 25200, "", "", 0x0, 0x0, -1 }, \
	{ "WALLIS", "WF", "WLF", "", "", "WALLIS AND FUTUNA TIME", "WFT", 43200, "", "", 0x0, 0x0, -1 }, \
	{ "WESTERN SAHARA", "EH", "ESH", "", "", "WESTERN EUROPEAN DAYLIGHT TIME", "WEDT", 3600, "", "", 0x0, 0x0, -1 }, \
	{ "YEMEN", "YE", "YEM", "", "", "ARABIA STANDARD TIME", "AST", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "ZAMBIA", "ZM", "ZMB", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "ZIMBABWE", "ZW", "ZWE", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0, -1 } \
}

#define TZ_COUNTRY_ALIASES_ENTRIES_m13 16
#define TZ_COUNTRY_ALIASES_TABLE_m13 { \
	{ "CHINA", "PEOPLE'S REPUBIC OF CHINA" }, \
	{ "CHINA", "PEOPLES REPUBIC OF CHINA" }, \
	{ "CZECH REPUBLIC", "CZECHIA" }, \
	{ "RUSSIA", "RUSSIAN FEDERATION" }, \
	{ "TURKS AND CAICOS", "TURKS & CAICOS" }, \
	{ "TURKS AND CAICOS", "TURKS" }, \
	{ "TURKS AND CAICOS", "CAICOS" }, \
	{ "UNITED KINGDOM", "UNITED KINGDOM OF GREAT BRITAIN & NORTHERN IRELAND" }, \
	{ "UNITED KINGDOM", "UNITED KINGDOM OF GREAT BRITAIN AND NORTHERN IRELAND" }, \
	{ "UNITED KINGDOM", "BRITAIN" }, \
	{ "UNITED KINGDOM", "ENGLAND" }, \
	{ "UNITED KINGDOM", "GREAT BRITAIN" }, \
	{ "UNITED KINGDOM", "NORTHERN IRELAND" }, \
	{ "UNITED KINGDOM", "SCOTLAND" }, \
	{ "UNITED KINGDOM", "WALES" }, \
	{ "UNITED STATES", "UNITED STATES OF AMERICA" } \
}

#define TZ_COUNTRY_ACRONYM_ALIASES_ENTRIES_m13 1
#define TZ_COUNTRY_ACRONYM_ALIASES_TABLE_m13 { \
	{ "GB", "UK" } \
}



//**********************************************************************************//
// MARK: Database (DB)
//**********************************************************************************//

// Currently only PostGres databases are supported.

#ifdef DATABASE_m13

// Defines
#define DB_EXPECTED_ROWS_NO_ENTRY_m13	((si4) -1)

// Prototypes
tern		DB_check_result_m13(PGresult *result);
PGresult	*DB_execute_command_m13(PGconn *conn, const si1 *command, si4 *rows, si4 expected_rows);

#endif // DATABASE_m13



//***********************************************************************//
// MARK: MED Versions of Standard Posix Functions
//***********************************************************************//

// the calling interfaces to these function are generally those defined by Posix
// in some cases additional functionality are added via special input values or varargs
// in some cases behavior is more robust than standard functions
// in a few cases return values are modified to be more useful
// see function definition comments for specific changes
// several non-standard functions that are closely related to standard functions (or imho should be standard functions), are also included here
// note FILE * types have been replaced with void *, where appropriate, to allow use of either FILE * or FILE_m13 * types without casts or compiler warnings

#if defined MACOS_m13 || defined LINUX_m13
	typedef	struct stat		struct_stat_m13;
#endif // MACOS_m13 || LINUX_m13
#ifdef WINDOWS_m13
	typedef	struct _stat64		struct_stat_m13;
#endif // WINDOWS_m13


si4		asprintf_m13(si1 **target, const si1 *fmt, ...) FMT_ATTR_m13(2, 3);
size_t		calloc_size_m13(void *address, size_t el_size); // memory allocation size in number of elements
tern		cp_m13(const si1 *arg1, const si1 *arg2, ...);  // copy (Unix cp parallel): "cp_m13(src, tgt)" or, recursively, "cp_m13(\"-R\", src, tgt)" - the option LEADS, as in the shell (a trailing option would need a NULL sentinel & therefore a wrapper macro, since a vararg may only be read once known to have been passed)
si4		errno_m13(void);  // returns platform independent value of errno
void		errno_reset_m13(void); // zero errno before calling functions that may set it
void		exit_exec_m13(const si1 *function, const si4 line, si4 status);  // called with exit_m13(si4 status) prototype (preproceesor adds function & line)
si4		fclose_m13(void *fp);  // pass address of FILE_m13 * to null local pointer
si4		fileno_m13(void *fp);
tern		fisopen_m13(void *fp); // returns whether file is open
si8		flen_m13(void *fp); // returns length of file
si4		flock_m13(void *fp, si4 operation, ...); // varargs(FLOCK_TIMEOUT_m13 bit set): const si1 *nap_str (string to pass to nap_m13())
							 // varargs(fp == FILE *): const si1 *file_path, const si1 *nap_str (must pass something for nap_str, but can be NULL)
FILE_m13	*fopen_m13(const si1 *path, const si1 *mode, ...); // varargs(mode empty): const si1 *mode, FILE_m13 *m13_fp, si4 flags, ui2 (as si4) permissions
si4		fprintf_m13(void *fp, const si1 *fmt, ...) FMT_ATTR_m13(2, 3);
si4		fputc_m13(si4 c, void *fp);
size_t		fread_m13(void *ptr, si8 el_size, size_t n_elements, void *fp, ...); // (el_size negative): non_blocking read
tern		freeable_m13(void *address); // returns whether address is freeable without segfaulting
void		*freopen_m13(const si1 *path, const si1 *mode, void *fp);
si4		fscanf_m13(void *fp, const si1 *fmt, ...);
si4		fseek_m13(void *fp, si8 offset, si4 whence);
si4		fstat_m13(si4 fd, struct_stat_m13 *sb);
si4		fstat_fp_m13(void *fp, struct_stat_m13 *sb); // fstat() called with file pointer
si8		ftell_m13(void *fp);
si4		ftruncate_m13(void *fp, off_t len);
size_t		fwrite_m13(void *ptr, si8 el_size, size_t n_elements, void *fp, ...);   // (el_size negative): non-blocking write; varargs(fp == NULL): FILE *fp (as void *), si1 *path
si1		*getcwd_m13(si1 *buf, size_t size);
pid_t_m13	getpid_m13(void);
pid_t_m13	gettid_m13(void);
// inverse semaphore functions [semaphores (with optional ownership) that unlock when count == 0]
void		isem_chown_m13(isem_t_m13 *isem, pid_t_m13 tid); // ownership not required  [special tid values: ISEM_SELF_m13, ISEM_NO_OWNER_m13]
void		isem_dec_m13(isem_t_m13 *sem); // ownership not required
void		isem_destroy_m13(isem_t_m13 *isem); // ownership not required
ui4		isem_getcnt_m13(isem_t_m13 *isem); // ownership not required
void		isem_inc_m13(isem_t_m13 *sem); // ownership or no owner required, or block
isem_t_m13	*isem_init_m13(isem_t_m13 *isem, ui4 init_val, const si1 *nap_str, const si1 *name, pid_t_m13 owner, tern free_on_destroy);
void		isem_own_m13(isem_t_m13 *isem); // ownership or no owner required, or block
void		isem_setcnt_m13(isem_t_m13 *isem, ui4 count); // ownership or no owner required, or block
void		isem_show_m13(isem_t_m13 *isem); // ownership not required
tern		isem_tryinc_m13(isem_t_m13 *sem); // ownershipp or no owner required, or fail
tern		isem_tryown_m13(isem_t_m13 *isem); // no ownership required, or fail
tern		isem_trysetcnt_m13(isem_t_m13 *isem, ui4 count); // ownership or no owner required, or fail
tern		isem_trywait_m13(isem_t_m13 *sem); // ownership or no owner required, or fail
tern		isem_trywait_noinc_m13(isem_t_m13 *isem); // ownership or no owner required, or fail
void		isem_wait_m13(isem_t_m13 *sem); // ownership or no owner required, or block
void		isem_wait_noinc_m13(isem_t_m13 *isem); // ownership or no owner required, or block
size_t		malloc_size_m13(void *address);
tern		md_m13(const si1 *dir); // synonym for mkdir()
void		*memalign_m13(void *addr, si4 alignment);  // (alignment == -1): page align
void		*memset_m13(void *ptr, si4 val, si8 n_members, ...);  // vargargs(n_members < 0): const void *el_val (val == el_size)
tern		mkdir_m13(const si1 *dir); // make directory
void		*memcpy_m13(void *target, const void *source, size_t n_bytes);
void		*memmove_m13(void *target, const void *source, size_t n_bytes);  // uses memcpy() if regions do not overlap
tern		mlock_m13(void *addr, si8 len);  // (len < 0): len = -len, lock regardless of page alignment
si4		mprotect_m13(void *address, size_t len, si4 protection);
si1		mreadable_m13(void *addr, size_t len, tern full_range);  // checks if memory is readable, & returns alignment
tern		munlock_m13(void *addr, size_t len);
si1		mwritable_m13(void *addr, size_t len, tern full_range);  // checks if memory is writable, & returns alignment
tern		mv_m13(const si1 *path, const si1 *new_path);  // move, rename
void		nanosleep_m13(struct timespec *tv);
void		nap_m13(const si1 *nap_str);  // sleep with duration set by string
struct timespec	*nap_timespec_m13(const si1 *nap_str, struct timespec *nap);
si4		printf_m13(const si1 *fmt, ...) FMT_ATTR_m13(1, 2);
si4		pthread_create_m13(pthread_t_m13 *thread, pthread_attr_t_m13 *attributes, pthread_fn_m13 start_routine, void *arg); // new thread inherits spawner's current behavior (snapshot) as its base stack entry
si4		pthread_equal_m13(pthread_t_m13 thread_1, pthread_t_m13 thread_2);  // returns exactly 1 if same thread, 0 if not (POSIX guarantees only non-zero; narrowed so all platforms agree)
void		pthread_exit_m13(void *ptr);
si1		*pthread_getname_m13(pthread_t_m13 thread, si1 *thread_name, size_t name_len);
si1		*pthread_getname_id_m13(pid_t_m13 _id, si1 *thread_name, size_t name_len);  // get thread name by thread id
si4		pthread_join_m13(pthread_t_m13 thread, void **value_ptr);
si4		pthread_kill_m13(pthread_t_m13 thread, si4 signal);
si4		pthread_mutex_destroy_m13(pthread_mutex_t_m13 *mutex_p);
si4		pthread_mutex_init_m13(pthread_mutex_t_m13 *mutex_p, pthread_mutexattr_t_m13 *attr_p);
si4		pthread_mutex_lock_m13(pthread_mutex_t_m13 *mutex_p);
si4		pthread_mutex_trylock_m13(pthread_mutex_t_m13 *mutex_p);
si4		pthread_mutex_unlock_m13(pthread_mutex_t_m13 *mutex_p);
pthread_t_m13	pthread_self_m13(void);
si4		putc_m13(si4 c, void *fp);
si4		putch_m13(si4 c);
si4		putchar_m13(si4 c);
ui4		rand32_m13(void); // 32-bit random number using system generator
ui4		rand32_med_m13(void); // 32-bit random number using medlib generator (replicable sequences across platforms)
ui4		rand32_med_wz_m13(volatile ui4 *w, volatile ui4 *z); // faster, less convenient, version of rand32_med_m13()
ui8		rand64_m13(void); // 64-bit random number using system random number generator
ui8		rand64_med_m13(void); // 64-bit random number using medlib generator (replicable sequences across platforms)
ui8		rand64_med_wz_m13(volatile ui4 *w, volatile ui4 *z); // faster, less convenient, version of rand64_med_m13()
si4		random_m13(void); // 31-bit random number using system generator (posix standard)
tern		rm_m13(const si1 *arg1, ...);  // remove (Unix rm/rmdir parallel): "rm_m13(path)" or, recursively, "rm_m13(\"-R\", path)" - the option LEADS, as in the shell (see cp_m13()); a non-empty directory is only removed with "-R"
si4		scanf_m13(const si1 *fmt, ...);
si4		sem_init_m13(sem_t_m13 *sem, si4 shared, ui4 init_val);
sem_t_m13	*sem_open_m13(const si1 *name, si4 o_flags, ...);  // (MacOS only) varargs(o_flags & O_CREAT): mode_t mode (as ui4), ui4 init_val
si4		sem_post_m13(sem_t_m13 *sem);
si4		sem_trywait_m13(sem_t_m13 *sem);
si4		sem_wait_m13(sem_t_m13 *sem);
si4		snprintf_m13(si1 *target, si4 target_field_bytes, const si1 *fmt, ...) FMT_ATTR_m13(3, 4);
si4		sprintf_m13(si1 *target, const si1 *fmt, ...) FMT_ATTR_m13(2, 3);
void		srand_med_m13(ui4 seed); // seed medlib random number generator
void		srand_med_wz_m13(ui4 seed, volatile ui4 *w, volatile ui4 *z); // faster, less convenient, version of srand_med_m13()
void		srandom_m13(ui4 seed); // seed system random number generator
si4		sscanf_m13(si1 *target, const si1 *fmt, ...);
si4		stat_m13(const si1 *path, struct_stat_m13 *sb);
si8		strcat_m13(void *target, const void *source); // args are standard strings or pstrs (disambiguated by tag); pstr targets grown as required
size_t		strchar_m13(const void *string); // strlen() that returns number of characters in string regardless of multibyte characters; O(1) for pstrs
si4		strcmp_m13(const void *string_1, const void *string_2); // args are standard strings or pstrs (disambiguated by tag)
si8		strcpy_m13(void *target, const void *source); // args are standard strings or pstrs (disambiguated by tag); pstr targets grown as required
size_t		strlen_m13(const void *source); // arg is standard string or pstr (disambiguated by tag); O(1) for pstrs
si8		strncat_m13(void *target, const void *source, size_t n_chars); // args are standard strings or pstrs (disambiguated by tag); pstr targets grown as required
si4		strncmp_m13(const void *string_1, const void *string_2, size_t n_chars); // args are standard strings or pstrs (disambiguated by tag)
si8		strncpy_m13(void *target, const void *source, size_t n_chars); // args are standard strings or pstrs (disambiguated by tag); pstr targets grown as required
si4		system_m13(const si1 *command, ...); // varargs(command = NULL): const si1 *command, tern (as si4) null_std_streams, ui4 behavior;
si4		system_pipe_m13(si1 **buffer_ptr, si8 buf_len, const si1 *command, ui4 flags, ...); // varargs(SP_BEHAVIOR_PASSED_m13 flag set): ui4 behavior)
												    // varargs(SP_SEPARATE_STREAMS_m13 flag set): si1 **e_buffer_ptr, si8 e_buf_len
												    // note: if both passed, behavior is first argument
void		tempclean_m13(void);
FILE_m13	*tempfile_m13(void);
si1		*tempnam_m13(si1 *path);
tern		touch_m13(const si1 *path);
si4		truncate_m13(const si1 *path, off_t len);
si4		vasprintf_m13(si1 **target, const si1 *fmt, va_list args);
si4		vfprintf_m13(void *fp, const si1 *fmt, va_list args);
si4		vprintf_m13(const si1 *fmt, va_list args);
si4		vsnprintf_m13(si1 *target, si4 target_field_bytes, const si1 *fmt, va_list args);
si4		vsprintf_m13(si1 *target, const si1 *fmt, va_list args);

// standard (& related) functions with AT_DEBUG_m13 versions
#ifndef AT_DEBUG_m13 // use these protoypes in all cases, defines will convert if needed
void	*aligned_alloc_m13(si8 alignment, si8 n_bytes);  // (alignment == -1): alignment = system page size; (n_bytes < 0): n_bytes = -n_bytes, all pages touched before return
void	aligned_free_m13(void *ptr);  // must use this in Windows if pointer obtained from aligned_alloc_m13()
void	*calloc_m13(size_t n_members, si8 el_size); // (el_size negative): level headers flag
void	**calloc_2D_m13(size_t dim1, size_t dim2, si8 el_size); // (el_size negative): level header flag
void	free_m13(void *ptr);
void	free_2D_m13(void **ptr, size_t dim1);
void	*malloc_m13(si8 n_bytes); // (n_bytes negative): level header flag
void	**malloc_2D_m13(size_t dim1, si8 dim2_bytes); // (dim2_bytes negative): level headers flag
void	*realloc_m13(void *ptr, si8 n_bytes); // (n_bytes negative): level header flag
void	**realloc_2D_m13(void **ptr, size_t curr_dim1, size_t new_dim1, size_t curr_dim2_bytes, si8 new_dim2_bytes); // (new_dim2_bytes negative): level headers flag
void	*recalloc_m13(void *ptr, size_t curr_members, size_t new_members, si8 el_size); // (el_size negative): level header flag
void	**recalloc_2D_m13(void **ptr, size_t curr_dim1, size_t new_dim1, size_t curr_dim2, size_t new_dim2, si8 el_size); // (el_size negative): level header flag
#endif // AT_DEBUG_m13

#define exit_m13(arg)			exit_exec_m13(__FUNCTION__, __LINE__, arg) // call with "G_pop_function_m13(void)" prototype

// "loop"s below deal with terminal semicolon (optimized out on compile)
#ifdef FT_DEBUG_m13
	#define return_m13(rval)		do { G_pop_function_m13(); return(rval); } while(0)
	#define return_void_m13			do { G_pop_function_m13(); return; } while(0)
	#define thread_return_m13(rval)		do { G_pop_function_m13(); G_thread_exit_m13(); return(rval); } while(0) // clears behavior stack & function stack, if it exists
	#define thread_return_null_m13		do { G_pop_function_m13(); G_thread_exit_m13(); return((pthread_rval_m13) 0); } while(0) // clears behavior stack & function stack, if it exists
#else
	#define return_m13(rval)		return(rval)
	#define return_void_m13			return
	#define thread_return_m13(rval)		do { G_thread_exit_m13(); return(rval); } while(0) // clears behavior stack & function stack, if it exists
	#define thread_return_null_m13		do { G_thread_exit_m13(); return((pthread_rval_m13) 0); } while(0) // clears behavior stack & function stack, if it exists
#endif // FT_DEBUG_m13



//**********************************************************************************//
// MARK: Password Tables
//**********************************************************************************//

// GENERATED by dev/gen_pw_tables.py -- edit the generator, not this block.

// breach-corpus passwords, most common first
#define PW_PASSWORDS_ENTRIES_m13	106
#define PW_PASSWORDS_m13 { \
	"123456","password","123456789","12345678","12345","qwerty","1234567","111111","1234567890","123123","abc123","1234", \
	"password1","iloveyou","000000","qwerty123","zaq12wsx","dragon","sunshine","princess","letmein","654321","monkey", \
	"27653","1qaz2wsx","123321","qwertyuiop","superman","asdfghjkl","trustno1","shadow","master","hello","freedom", \
	"whatever","qazwsx","michael","football","baseball","welcome","jesus","ninja","mustang","password123","admin", \
	"login","root","guest","test","user","secret","pass","access","flower","hottie","loveme","zaq1zaq1","hello123", \
	"charlie","donald","batman","soccer","harley","ranger","buster","thomas","robert","jordan","hunter","michelle", \
	"daniel","starwars","klaster","112233","george","computer","jessica","pepper","1111","zxcvbn","555555","11111111", \
	"131313","777777","pass1234","maggie","159753","aaaaaa","ginger","joshua","cheese","amanda","summer","love","ashley", \
	"nicole","chelsea","biteme","matthew","yankees","987654321","dallas","austin","thunder","taylor","matrix" }

// common English words, roughly frequency ordered
#define PW_WORDS_ENTRIES_m13	2539
#define PW_WORDS_m13 { \
	"the","and","that","have","for","not","with","you","this","but","his","from","they","say","her","she","will","one", \
	"all","would","there","their","what","out","about","who","get","which","when","make","can","like","time","just", \
	"him","know","take","people","into","year","your","good","some","could","them","see","other","than","then","now", \
	"look","only","come","its","over","think","also","back","after","use","two","how","our","work","first","well","way", \
	"even","new","want","because","any","these","give","day","most","man","find","here","thing","tell","very","still", \
	"should","through","much","where","before","too","same","right","down","life","around","another","old","great","big", \
	"high","different","small","large","next","early","young","important","few","public","bad","able","need","feel", \
	"three","state","never","become","between","really","something","family","own","leave","put","while","mean","keep", \
	"student","seem","however","talk","turn","start","might","show","hear","play","run","move","live","believe","hold", \
	"bring","happen","write","provide","sit","stand","lose","pay","meet","include","continue","set","learn","change", \
	"lead","understand","watch","follow","stop","create","speak","read","allow","add","spend","grow","open","walk","win", \
	"offer","remember","love","consider","appear","buy","wait","serve","die","send","expect","build","stay","fall","cut", \
	"reach","kill","remain","suggest","raise","pass","sell","require","report","decide","pull","house","home","water", \
	"world","school","room","mother","area","money","story","fact","month","lot","study","book","eye","job","word", \
	"business","issue","side","kind","head","far","black","long","little","happy","friend","baby","summer","winter", \
	"spring","autumn","january","february","march","april","may","june","july","august","september","october","november", \
	"december","monday","tuesday","wednesday","thursday","friday","saturday","sunday","music","movie","phone","computer", \
	"internet","email","password","login","account","online","game","player","team","sport","soccer","football", \
	"baseball","basketball","tennis","hockey","golf","swim","bike","car","truck","train","plane","boat","apartment", \
	"garden","kitchen","bedroom","office","college","university","teacher","doctor","nurse","hospital","patient", \
	"medicine","health","brain","heart","blood","bone","muscle","nerve","seizure","neurology","cardiology","surgery", \
	"clinic","research","science","data","analysis","result","method","system","network","server","database","file", \
	"folder","document","project","company","market","customer","service","product","price","dollar","bank","credit", \
	"card","cash","payment","order","shipping","delivery","red","blue","green","yellow","orange","purple","white", \
	"brown","gray","pink","silver","gold","north","south","east","west","city","town","country","america","canada", \
	"mexico","england","france","germany","spain","italy","china","japan","india","brazil","russia","australia","africa", \
	"europe","asia","dog","cat","bird","fish","horse","cow","pig","sheep","chicken","duck","rabbit","mouse","bear", \
	"lion","tiger","elephant","apple","banana","grape","lemon","peach","cherry","berry","melon","tomato","potato", \
	"carrot","onion","bread","cheese","milk","coffee","juice","beer","wine","pizza","burger","salad","soup","rice", \
	"pasta","correct","battery","staple","wrong","true","false","yes","maybe","always","often","above","below","under", \
	"inside","outside","near","away","front","top","bottom","left","middle","center","begin","end","finish","return", \
	"arrive","enter","exit","close","lock","unlock","push","carry","drop","lift","throw","catch","kick","hit","break", \
	"fix","destroy","repair","clean","dirty","wash","dry","cook","eat","drink","sleep","wake","dream","rest","teach", \
	"draw","paint","sing","dance","listen","shout","whisper","laugh","cry","smile","frown","angry","sad","glad","afraid", \
	"brave","strong","weak","fast","slow","quick","easy","hard","simple","difficult","heavy","light","dark","bright", \
	"loud","quiet","warm","cold","hot","cool","wet","fresh","ancient","modern","late","soon","later","during","until", \
	"since","sometimes","rarely","usually","seldom","daily","weekly","monthly","yearly","today","tomorrow","yesterday", \
	"morning","afternoon","evening","night","noon","midnight","hour","minute","second","week","decade","century", \
	"season","weather","rain","snow","wind","storm","cloud","sun","moon","star","sky","earth","ground","land","sea", \
	"ocean","river","lake","mountain","hill","valley","forest","tree","flower","grass","leaf","root","branch","seed", \
	"fruit","vegetable","meat","egg","salt","sugar","spice","oil","butter","flour","dough","cake","pie","cookie","candy", \
	"chocolate","table","chair","desk","bed","sofa","lamp","door","window","wall","floor","ceiling","roof","stair", \
	"gate","fence","yard","garage","street","road","path","bridge","tunnel","building","tower","castle","church", \
	"temple","museum","library","store","shop","hotel","restaurant","cafe","theater","cinema","park","beach","island", \
	"desert","jungle","farm","field","barn","paper","pen","pencil","page","chapter","poem","song","note","sound","voice", \
	"noise","silence","number","letter","sentence","line","list","chart","graph","picture","photo","image","video", \
	"film","color","shape","circle","square","triangle","round","flat","sharp","smooth","rough","thick","thin","wide", \
	"narrow","deep","shallow","short","tall","low","tiny","huge","giant","parent","father","son","daughter","brother", \
	"sister","uncle","aunt","cousin","nephew","niece","grandfather","grandmother","husband","wife","child","neighbor", \
	"stranger","person","group","crowd","king","queen","prince","princess","knight","soldier","captain","general", \
	"president","leader","boss","worker","farmer","driver","pilot","sailor","police","fire","chef","waiter","clerk", \
	"lawyer","judge","scientist","engineer","artist","writer","face","ear","nose","mouth","tooth","tongue","lip","chin", \
	"neck","shoulder","arm","elbow","hand","finger","thumb","chest","stomach","leg","knee","foot","toe","skin","hair", \
	"nail","lung","liver","kidney","lucky","funny","silly","crazy","smart","clever","wise","foolish","nice","rude", \
	"polite","honest","question","answer","problem","solution","reason","cause","effect","difference","example","power", \
	"energy","force","speed","heat","wave","space","matter","atom","cell","gene","metal","wood","stone","glass", \
	"plastic","cloth","leather","rubber","steel","iron","copper","across","along","beside","third","fourth","fifth", \
	"last","previous","final","single","double","triple","half","quarter","zero","four","five","six","seven","eight", \
	"nine","ten","eleven","twelve","twenty","thirty","forty","fifty","sixty","seventy","eighty","ninety","hundred", \
	"thousand","million","billion","violet","golden","dragon","eagle","wolf","shark","falcon","phoenix","cobra","viper", \
	"hawk","raven","fox","panther","secure","safety","private","secret","hidden","locked","key","code","cipher","crypt", \
	"guard","sample","record","signal","channel","filter","value","point","range","limit","level","scale","unit", \
	"hippocampus","amygdala","cortex","cortical","thalamus","cerebellum","cerebral","temporal","frontal","parietal", \
	"occipital","lobe","gyrus","sulcus","neuron","neuronal","synapse","axon","dendrite","myelin","glia","astrocyte", \
	"epilepsy","epileptic","ictal","interictal","postictal","aura","convulsion","spike","complex","spindle","rhythm", \
	"alpha","beta","gamma","delta","theta","oscillation","frequency","amplitude","voltage","microvolt","electrode", \
	"contact","montage","bipolar","referential","reference","impedance","artifact","notch","sampling","recording", \
	"acquisition","monitor","telemetry","ambulatory","implant","depth","grid","strip","stereo","eeg","ecog","seeg","emg", \
	"ecg","ekg","eog","mri","ctscan","pet","spect","fmri","imaging","scan","slice","volume","subject","session", \
	"segment","annotation","event","marker","onset","offset","duration","latency","diagnosis","prognosis","treatment", \
	"therapy","medication","dose","focus","lateralization","localization","resection","ablation","stimulation", \
	"neurostimulator","pacemaker","clinical","protocol","consent","deidentified","anonymous","encrypted","archive", \
	"backup","rem","nrem","stage","staging","hypnogram","arousal","apnea","snore","respiration","oxygen","saturation", \
	"rate","pulse","pressure","temperature","vital","sign","alarm","threshold","baseline","neurologist","epileptologist", \
	"technologist","technician","physician","resident","fellow","ward","admission","discharge","summary","ability", \
	"absence","accept","accident","accurate","achieve","acid","act","active","actual","addition","address","admire", \
	"admit","adult","advance","advice","affect","afford","again","against","age","agency","agent","agree","ahead","aid", \
	"aim","air","aircraft","album","alcohol","alive","almost","alone","already","although","amazing","among","amount", \
	"analyze","anger","animal","announce","annual","anxiety","anybody","anyone","anything","anyway","anywhere","apart", \
	"apologize","appeal","apply","appoint","approach","approve","architect","argue","arise","army","arrange","arrest", \
	"art","article","ashamed","aside","ask","asleep","aspect","assist","assume","attach","attack","attempt","attend", \
	"attention","attitude","attract","audience","author","available","average","avoid","awake","award","aware","awful", \
	"background","backward","bacteria","badly","bag","balance","ball","band","bar","barrier","base","basic","basis", \
	"basket","bath","battle","bean","beard","beat","beautiful","beauty","behalf","behave","behind","belief","bell", \
	"belong","belt","bench","bend","beneath","benefit","best","bet","better","beyond","bicycle","bill","bind","birth", \
	"birthday","bit","bite","bitter","blade","blame","blank","blanket","bleed","bless","blind","block","blow","board", \
	"body","boil","bomb","boot","border","boring","born","borrow","both","bother","bottle","bounce","bound","bowl","box", \
	"boy","brand","breakfast","breast","breath","breathe","breed","brick","brief","broad","broken","brush","bubble", \
	"bucket","budget","bullet","bunch","burden","burn","burst","bury","bus","bush","busy","button","cabin","cable", \
	"calculate","calendar","call","calm","camera","camp","campaign","canal","cancel","cancer","candidate","candle", \
	"capable","capital","capture","carbon","care","career","careful","cart","case","cast","casual","category","cattle", \
	"caution","cave","cease","celebrate","cement","ceremony","certain","chain","challenge","chamber","champion","chance", \
	"character","charge","charity","charm","chase","cheap","cheat","check","cheek","cheer","chemical","chief", \
	"childhood","chill","choice","choose","chop","circuit","circumstance","citizen","civil","claim","class","classic", \
	"clear","click","client","climate","climb","clock","club","coach","coal","coast","coat","coin","collapse","collar", \
	"collect","colony","column","combine","comfort","command","comment","commit","common","communicate","community", \
	"compare","compete","complain","complete","concept","concern","conclude","concrete","condition","conduct","confirm", \
	"conflict","confuse","connect","consist","constant","construct","consult","consume","contain","content","contest", \
	"context","contract","contrast","contribute","control","convince","copy","core","corn","corner","cost","cotton", \
	"couch","cough","council","count","county","couple","courage","course","court","cover","crack","craft","crash", \
	"cream","creature","crew","crime","crisis","critic","crop","cross","crown","crucial","cruel","crush","crystal", \
	"culture","cup","cure","curious","current","curtain","curve","custom","cycle","damage","danger","dare","date","dawn", \
	"dead","deal","dear","death","debate","debt","deck","declare","decline","decorate","decrease","deer","defeat", \
	"defend","define","degree","delay","deliver","demand","democracy","demonstrate","deny","depart","depend","deposit", \
	"derive","describe","deserve","design","desire","despite","detail","detect","determine","develop","device","devote", \
	"diamond","diary","dictionary","diet","differ","dig","dinner","direct","dirt","disagree","disappear","disaster", \
	"discipline","discover","discuss","disease","dish","dismiss","display","distance","distinct","distribute","district", \
	"disturb","divide","divorce","domestic","dominate","donate","dot","doubt","dozen","draft","drag","drama","dress", \
	"drive","drug","drum","due","dull","dump","dust","duty","each","eager","earn","ease","economy","edge","edit", \
	"educate","effort","either","elderly","elect","electric","elegant","element","elevator","else","emerge","emergency", \
	"emotion","emphasis","employ","empty","enable","encourage","enemy","engage","engine","enhance","enjoy","enormous", \
	"enough","entire","entry","envelope","environment","equal","equip","era","error","escape","especially","essay", \
	"essential","establish","estate","estimate","ever","every","evidence","evil","exact","examine","exceed","excellent", \
	"except","exchange","excite","exclude","excuse","execute","exercise","exhibit","exist","expand","expense", \
	"experience","experiment","expert","explain","explore","export","expose","express","extend","extent","external", \
	"extra","extreme","fabric","factor","factory","fade","fail","faint","fair","faith","fame","familiar","famous","fan", \
	"fancy","fantasy","fashion","fat","fate","fault","favor","fear","feather","feature","federal","fee","feed","female", \
	"festival","fetch","fever","fiber","fiction","fight","figure","fill","finance","fine","firm","fit","flag","flame", \
	"flash","flavor","flee","flesh","flight","float","flood","flow","fluid","fly","fold","food","fool","foreign", \
	"forget","forgive","fork","form","formal","former","fortune","forward","found","frame","free","freeze","frequent", \
	"frighten","fuel","full","fun","function","fund","funeral","fur","furniture","further","future","gain","gallery", \
	"gang","gap","garbage","gas","gather","gear","gender","generate","gentle","genuine","gesture","ghost","gift","girl", \
	"global","glove","glue","goal","goat","god","goods","govern","grab","grade","grain","grand","grant","grave","greet", \
	"grey","grief","grin","grip","grocery","guess","guest","guide","guilty","gun","guy","habit","hall","handle","hang", \
	"harbor","harm","hat","hate","heal","heaven","heel","height","hell","hello","help","hence","hero","hesitate","hide", \
	"hint","hip","hire","history","hobby","hole","holiday","hollow","holy","honey","honor","hook","hope","horizon", \
	"horn","horror","host","household","housing","human","humble","humor","hunger","hunt","hurry","hurt","ice","idea", \
	"ideal","identify","idle","ignore","ill","illegal","imagine","immediate","impact","imply","import","impose", \
	"impress","improve","impulse","income","increase","indeed","independent","index","indicate","individual","indoor", \
	"industry","infant","infect","inflation","influence","inform","initial","injure","inner","innocent","input", \
	"inquiry","insect","insight","insist","inspire","install","instance","instead","institute","instruct","instrument", \
	"insurance","intend","intense","interest","interior","internal","international","interpret","interview","introduce", \
	"invent","invest","investigate","invite","involve","item","jacket","jail","jam","jar","jaw","jazz","jealous","jeans", \
	"jet","jewel","join","joint","joke","journal","journey","joy","jump","junior","jury","justice","justify","keen", \
	"kid","kiss","knife","knock","knowledge","lab","label","labor","lack","ladder","lady","language","lap","launch", \
	"law","lawn","lay","layer","lazy","league","lean","leap","least","lecture","legal","legend","lend","length","lens", \
	"less","lesson","let","liberal","license","lie","limb","link","liquid","literature","load","loan","local","locate", \
	"log","logic","lonely","loop","loose","lord","loss","loyal","luck","lunch","luxury","machine","mad","magazine", \
	"magic","mail","main","maintain","major","male","mall","manage","manner","manual","many","map","margin","mark", \
	"marriage","mask","mass","master","match","material","math","mature","maximum","mayor","meal","measure","media", \
	"medical","medium","melt","member","memory","mental","mention","menu","mercy","mere","merit","message","mild","mile", \
	"military","mind","mine","minimum","minister","minor","mirror","miss","mission","mistake","mix","mobile","mode", \
	"model","moderate","modest","modify","moment","monkey","mood","moral","more","mortgage","motion","motor","mud", \
	"multiple","murder","must","mutual","mystery","naked","name","nation","native","natural","nature","navy","neat", \
	"necessary","negative","neglect","negotiate","neither","nest","net","neutral","news","nobody","nod","none","nor", \
	"normal","nothing","notice","notion","novel","nowhere","nuclear","nut","object","observe","obtain","obvious", \
	"occasion","occupy","occur","odd","off","offend","officer","official","okay","olive","omit","once","onto","operate", \
	"opinion","opponent","opportunity","oppose","option","ordinary","organ","organize","origin","otherwise","ought", \
	"outcome","outdoor","outer","output","oven","overall","overcome","owe","owner","pace","pack","package","pain","pair", \
	"palace","pale","palm","pan","panel","panic","parade","part","participate","particular","partner","party","passage", \
	"passenger","passion","past","pattern","pause","peace","peak","penalty","pepper","per","percent","perfect","perform", \
	"perhaps","period","permanent","permit","personal","persuade","phase","phrase","physical","piano","pick","piece", \
	"pile","pill","pin","pipe","pitch","pity","place","plain","plan","plant","plate","platform","pleasant","please", \
	"pleasure","plenty","plot","plug","plus","pocket","poet","poison","pole","policy","polish","political","poll","pool", \
	"poor","pop","popular","population","port","portion","portrait","pose","position","positive","possess","possible", \
	"post","pot","potential","pound","pour","poverty","powder","practical","practice","praise","pray","precise", \
	"predict","prefer","pregnant","premium","prepare","present","preserve","press","pretend","pretty","prevent","pride", \
	"priest","primary","prime","principle","print","prior","prison","prize","probably","proceed","process","produce", \
	"profession","profile","profit","program","progress","promise","promote","prompt","proof","proper","property", \
	"proportion","propose","protect","proud","prove","publish","pump","punch","punish","purchase","pure","purpose", \
	"pursue","puzzle","qualify","quality","quantity","quit","quite","quote","race","radio","rail","rank","rapid","rare", \
	"rather","ratio","raw","react","ready","real","realize","recall","receive","recent","recipe","recognize","recommend", \
	"recover","reduce","refer","reflect","reform","refuse","regard","region","regret","regular","reject","relate", \
	"relax","release","relevant","relief","religion","rely","remark","remind","remote","remove","rent","repeat", \
	"replace","reply","represent","request","rescue","reserve","resist","resolve","resource","respect","respond", \
	"restore","retain","retire","reveal","revenue","reverse","review","revolution","reward","rich","rid","ride","ridge", \
	"rifle","ring","riot","rise","risk","roast","rob","rock","role","roll","rope","rose","route","routine","row","royal", \
	"rub","ruin","rule","rural","rush","sacred","safe","sail","salary","sale","sand","satisfy","sauce","save","scare", \
	"scatter","scene","schedule","scheme","score","scratch","scream","screen","script","seal","search","seat","section", \
	"sector","seek","seize","select","self","senior","sense","separate","series","serious","settle","several","severe", \
	"sew","shade","shadow","shake","shall","shame","share","shave","shed","sheet","shelf","shell","shelter","shield", \
	"shift","shine","ship","shirt","shock","shoe","shoot","shore","shot","shower","shrink","shut","sick","sight","silk", \
	"similar","sin","sink","sir","site","situation","size","skill","skirt","slave","slide","slight","slip","smell", \
	"smoke","snake","soap","social","society","sock","soft","soil","solid","solve","sorry","sort","soul","source", \
	"spare","special","species","specific","speech","spell","sphere","spin","spirit","split","spoil","spot","spread", \
	"spy","squeeze","stable","staff","stake","standard","stare","station","steady","steal","steam","steep","step", \
	"stick","stiff","stir","stock","straight","strange","strategy","stream","strength","stress","stretch","strike", \
	"string","stroke","structure","struggle","studio","stuff","stupid","style","submit","substance","succeed","success", \
	"such","sudden","suffer","suit","super","supply","support","suppose","sure","surface","surprise","surround","survey", \
	"survive","suspect","sustain","swallow","swear","sweat","sweep","sweet","swing","switch","sword","symbol","sympathy", \
	"tail","tale","talent","tank","tape","target","task","taste","tax","tea","tear","technical","technique","technology", \
	"teeth","telephone","temporary","tempt","tend","tension","tent","term","terrible","territory","terror","test","text", \
	"thank","theme","theory","thirst","thorough","those","though","thought","thread","threat","throat","thunder","thus", \
	"ticket","tide","tie","tight","tip","tire","title","together","toilet","tone","tonight","tool","topic","total", \
	"touch","tough","tour","toward","toy","trace","track","trade","tradition","traffic","transfer","transform", \
	"translate","transport","trap","travel","treat","trend","trial","tribe","trick","trip","troop","trouble","trust", \
	"truth","try","tube","tune","twice","twin","twist","type","typical","ugly","ultimate","umbrella","unable","undergo", \
	"unfair","uniform","union","unique","unite","universe","unknown","unless","unlike","unusual","upon","upper","upset", \
	"urban","urge","urgent","useful","usual","utility","vacation","valid","van","variety","various","vast","vehicle", \
	"venture","version","vertical","vessel","veteran","via","victim","victory","view","village","violate","violence", \
	"virtue","virus","visible","vision","visit","visual","vote","voyage","wage","wander","war","warn","waste","wealth", \
	"weapon","wear","web","wedding","weigh","welcome","welfare","wheat","wheel","whether","whole","whom","whose","why", \
	"widow","width","wild","wing","wipe","wire","wish","witness","woman","wonder","wool","worry","worse","worship", \
	"worth","wound","wrap","yet","yield","youth","zone" }

// common given names & surnames
#define PW_NAMES_ENTRIES_m13	169
#define PW_NAMES_m13 { \
	"james","robert","john","michael","david","william","richard","joseph","thomas","charles","christopher","daniel", \
	"matthew","anthony","mark","donald","steven","paul","andrew","joshua","kenneth","kevin","brian","george","timothy", \
	"ronald","jason","edward","jeffrey","ryan","jacob","gary","nicholas","eric","jonathan","stephen","larry","justin", \
	"scott","brandon","benjamin","samuel","gregory","frank","alexander","raymond","patrick","jack","dennis","jerry", \
	"tyler","aaron","jose","adam","nathan","henry","zachary","douglas","peter","kyle","noah","ethan","jeremy","walter", \
	"christian","keith","roger","terry","mary","patricia","jennifer","linda","elizabeth","barbara","susan","jessica", \
	"sarah","karen","nancy","lisa","margaret","betty","sandra","ashley","dorothy","kimberly","emily","donna","michelle", \
	"carol","amanda","melissa","deborah","stephanie","rebecca","laura","sharon","cynthia","kathleen","amy","shirley", \
	"angela","helen","anna","brenda","pamela","nicole","samantha","katherine","christine","debra","rachel","carolyn", \
	"janet","catherine","maria","heather","diane","ruth","julie","olivia","smith","johnson","williams","brown","jones", \
	"garcia","miller","davis","rodriguez","martinez","hernandez","lopez","gonzalez","wilson","anderson","taylor","moore", \
	"jackson","martin","lee","perez","thompson","white","harris","sanchez","clark","ramirez","lewis","robinson","walker", \
	"young","allen","king","wright","torres","nguyen","hill","flores","green","adams","nelson","baker","hall","rivera", \
	"campbell","mitchell","carter","roberts" }



//**********************************************************************************//
//*********************************** MED Records **********************************//


//**********************************************************************************//
// MARK: Write API (WRT)
//**********************************************************************************//

// Write API (merged from the medwrt development module 2026-08-11; started 2026-08-09, Matt & Claude). A general-purpose, shallow-user write API for
// MED sessions - the target callers are Python/MATLAB bindings & simple C programs. Everything needed
// already exists in medlib; this module packages the MANUAL apparatus (DHN_Acq's MED_setup() + block
// processor were the reference) so a caller cannot produce a subtly broken session by forgetting
// bookkeeping (index entries, section-2 running aggregates, UH settling at close).
//
// ⭐ THE 90% PATH (Matt, 2026-08-09: "user just has arrays of samples by channel at some sampling rate &
// just wants them compressed into MED & nothing fancy - that will be 90% of uses"):
//
//     WRT_session_m13("/data/my_sess", samples, n_chans, n_samps, 30000.0, NULL, NULL, NULL);
//
// Arrays in, MED session out. NULLs mean: channel names generated via G_generate_numbered_names_m13()
// (the library's existing facility - D16 reuse), all-default options
// (PRED2 + fall-through, 10 s blocks, one segment, no encryption, no filtering), all-default metadata
// with times as PURE oUTC from zero - no wall time claimed (RESOLVED, Matt 2026-08-10: the pure-oUTC
// case must be EASY & the library handles it here - claiming the conversion moment as recording time
// would be false; zero-based oUTC claims nothing). Sampling frequency is the ONE datum beyond the
// arrays the library cannot invent, so it is a
// direct argument. Everything fancier is a filled field away.
//
// TWO TIERS above the existing deep API:
//   Tier 1 - one-shot: an in-memory channels x samples array + the two structs below -> session on disk.
//            (The likely actual request for Python/MATLAB users.)
//   Tier 2 - streaming handle: create / add_channel / write_samples (ANY length - the library blocks &
//            buffers) / mark_discontinuity / write_record / close. For converters, acquisition & big data.
//
// STRUCT PHILOSOPHY (Matt, 2026-08-09; revised 2026-08-10, D19): two separate structures -
//   METADATA_m13           - WHAT this recording is: THE LIBRARY'S OWN metadata structure, used as a
//                            template (no parallel "write metadata" struct - it was a costume over
//                            real section 1/2/3 fields). Per-channel overrides at add_channel.
//   WRT_OPTIONS_m13  - HOW to write it: ONLY what has no MED-field counterpart (codec, filters,
//                            passwords, blocking, segmenting, durability, time mode). An option's
//                            CONSEQUENCE may land in real metadata - e.g. decimation_frequency, once
//                            encoded, simply IS tmd2->sampling_frequency.
// Both have _defaults_ initializers; a zero-filled struct from a binding means "all defaults". The
// shallow path is: session name, sampling_frequency, maybe passwords - everything else defaults.
//
// OPEN DESIGN DECISIONS (numbered for annotation):
//   D1. RESOLVED (Matt, 2026-08-09): metadata & options are PER-CHANNEL - some channels decimated
//       (different fs), some filtered, different codecs. The session-level structs passed to create are
//       the DEFAULTS TEMPLATE; add_channel takes optional per-channel structs merged FIELD-WISE: any
//       field left at its NO_ENTRY/empty default inherits the session value, any field set wins. (So a
//       channel overriding only fs specifies only fs.) The handle stores one merged pair per channel.
//   D2. RESOLVED (Matt, 2026-08-09). Three realities, one model:
//       - OFFLINE (the common case): continuous flow from session_start_time by sample count.
//       - ONLINE: the caller passes start_time on write_samples calls - its best estimate of actual
//         wall time for those samples - & the library derives block start times from the most recent
//         caller estimate plus sample flow since. (start_time = TIME_NO_ENTRY_m13 => continuous flow.)
//       - NO IDEA OF TRUE TIME (expected to be COMMON): times used as PURE oUTC - callers may simply
//         start their times at zero. metadata.time_mode says which world we are in.
//       THE LIBRARY SETS UP THE RECORDING TIME OFFSET (RTO) FOR THE CALLER in every mode: uutc mode
//       generates the RTO per the library's privacy conventions (true time recoverable by those with
//       access); oUTC mode records that no true wall time is claimed & times pass through as offsets.
//       No shallow user should ever compute an RTO.
//   D3. RESOLVED (Matt, 2026-08-09): segmenting is a MODE - options.segmenting_mode:
//       NONE: whole session is one segment (default).
//       AUTO: duration-based rollover (options.auto_segment_hours).
//       MANUAL: the writing code calls WRT_new_segment_m13() whenever it wants.
//       (Midnight-alignment & the rest of DHN_Acq's scheduling stay app-side.)
//   D10. Filtering internals (Matt, 2026-08-09): do NOT model this module's filtering on DHN_Acq's
//       current per-block filtfilt + pad/replication apparatus - DHN_Acq is migrating its filtering to
//       the HEAD/MID/TAIL streaming mechanism. medwrt's antialias & LNF should target that mechanism
//       (filter state carried across blocks; head = warm-up, mid = steady state, tail = flush at
//       discontinuities/close), so the two converge instead of forking.
//   D4. Records (Note etc.) in v1: prototyped below; body formats limited to Note initially.
//   D5. Threading: v1 serial; options.threaded reserved for a library-internal per-channel encode pool.
//   D6. RESOLVED (Matt, 2026-08-10): the engine ALWAYS writes each block as it completes (& settles at
//       close) - one well-defined behavior, no library buffering mechanism. Durability management
//       beyond that (fsync policy, batching) is a DISTINCT concern & stays app-side; the OS page cache
//       already buffers for throughput. (The write_through option was removed.)
//   D8. Decimation (Matt, 2026-08-09: "this is going to be wanted"): modeled on DHN_Acq - per-channel
//       decimation_frequency, antialias filtfilt before resample, fractional-step LINEAR-INTERPOLATION
//       resampler with cross-block phase continuity (next-output-index phase carried across blocks;
//       boundary samples interpolated from the previous block's last value; block start time adjusted by
//       the fractional offset so timestamps stay honest; integer-step fast path = pure picking;
//       discontinuities reset the phase). Lives as an internal of this module first; candidate for
//       promotion into medlib proper so DHN_Acq itself can eventually call it (dedupe).
//   D11. RESOLVED (Matt, 2026-08-10): ONE ENGINE, TWO SKINS. The streaming handle IS the engine - it
//       owns every bookkeeping invariant (indices, aggregates, UH settling, Sgmt records, rollover,
//       RTO, discontinuities, filtering, decimation) - & the one-shot is a thin veneer over it. DEEP
//       APPS (DHN_Acq, converters) use the SAME engine: no duplication, bugs fixed once. What the
//       engine adds for them: (a) THREADING CONTRACT - see D5; (b) ESCAPE HATCHES - accessors to the
//       per-channel CPS/FPS for knobs the options struct does not cover, under the rule that the
//       library still owns the bookkeeping invariants regardless of what the caller touches. Hatch
//       shape RESOLVED (Matt, 2026-08-10): RAW ACCESS - "strong open source philosophy", no
//       public/private ceremony; accessors exist for binding convenience, not as a boundary; (c) a POST-BLOCK
//       CALLBACK (chan, start time, samples, bytes, algorithm, noise-score bytes, discontinuity) for
//       display taps / stats / alerts (DHN_Acq's line-noise alert = byte 0 of the scores).
//   D5. RESOLVED (2026-08-10, follows from D11): a threading CONTRACT now, a pool later. Per-channel
//       operations are independent - write_samples(chan) may be called concurrently from different
//       threads for DIFFERENT channels (each channel owns its CPS/staging/FPSes); the same channel is
//       caller-serialized; session-level operations (segment roll, records, close) are internally
//       protected; rollover fixes the roll TIME once under the session lock & each channel rolls
//       lazily as its writes cross it. options.threaded stays reserved for an optional INTERNAL pool
//       for single-threaded bindings - later, not v1.
//   D4. RESOLVED (2026-08-10, follows from D11): records are the GENERIC path from v1 -
//       write_record(type_code, time, body) through the medrec machinery; converters carry arbitrary
//       source annotation types, so a Note-only special case would just be re-widened.
//   D12. REVISED (Matt, 2026-08-10): MECHANISM in the engine, spreading POLICY in the app. The engine
//       gives every channel TWO block-size fields (the DHN_Acq initial-vs-target pattern):
//         initial_block_samples - the target for the first block after any SESSION-WIDE discontinuity
//                                 (session start included: same machinery, Matt's first-sample
//                                 principle) - so phase offsets re-establish after gaps for free;
//         target/block samples  - the steady-state size (options.block_duration/block_samples).
//       EQUAL BY DEFAULT (no stagger; correct for offline). An app wanting spread completions (e.g.
//       DHN_Acq keeping encode CPU even) computes its own policy & sets per-channel initials - the
//       group-proportional fraction it uses today is offered as an optional convenience helper
//       (WRT_spread_block_phases_m13()), not baked in. Pure mechanism, policy-free.
//   D13. RESOLVED (Matt, 2026-08-10; unifies the float-input question with the buffer design): the
//       channel staging buffers are "MAGIC" RING BUFFERS OF sf8 (VM double-mapped: the ring is always
//       linear, no wrap copies) - the filters want sf8 anyway, & head/mid/tail filtfilt & line-noise
//       machinery ALREADY EXIST in medlib (so D8 decimation has NO missing dependency). Conversion to
//       si4 happens ONLY AT ENCODE, quantized by amplitude_units_conversion_factor (stored_int *
//       factor = physical units). Callers may write si4 OR sf8 (write_samples / write_samples_sf8);
//       si4 input converts up on entry. Tier 1 accepts sf8 arrays & AUTO-RANGES the conversion factor
//       (whole array in hand); the streaming engine requires the factor up front for sf8 input
//       (default 1.0 = round to integer units).
//   D14. acquisition_channel_number (Matt, 2026-08-10): per-channel metadata - a PHYSICAL reality,
//       useful for archaeology & matching clinical recordings. When the caller does not set it, the
//       library AUTO-SETS to the add order (1-based) - documented as ORDINAL, corresponding to nothing
//       physical unless the caller says so. [Recommendation pending Matt's nod: add-order default; if
//       synthetic-vs-real ever needs to be distinguishable in-file, a discretionary-region marker is
//       the place - not a special number scheme.]
//   D15. SCOPE decisions recorded (Matt, 2026-08-10): VIDEO channels are a LOT - stubbed
//       (WRT_add_vid_channel_m13() exists & fails cleanly; add at the end if still a good idea).
//       APPEND/RESUME into an existing session: deliberately out (WORM philosophy; exists_behavior =
//       fail/overwrite/rename only). Tier-1 one-shot is UNIFORM rate & length by design - anything
//       else is the engine. FLUSH: WRT_flush_m13() exists - library handles closing/flushing normally;
//       an explicit flush is the deep-user case & they should know what they are doing (partials
//       written as short blocks WITHOUT a discontinuity mark).
//   D16. REVISED (Matt, 2026-08-10): THE HANDLE'S INTERNALS *ARE* THE MED HIERARCHY. A write channel
//       CONTAINS the real library structures (its metadata FPS with the full METADATA_m13; the session
//       handle owns a real SESS_m13 with real CHAN_m13/SEG_m13/FPS children) - not shadows of them.
//       Consequences: encryption is the EXISTING machinery on the struct it was built for; Sgmt
//       records come from the existing builders walking the same hierarchy; READ & WRITE paths share
//       one set of structures (close-time verification re-walks what a reader will walk); deep apps
//       navigate the hierarchy they already know - the escape hatch is just the hierarchy itself.
//       The curated WRT_METADATA/OPTIONS structs are INPUT CONVENIENCES (zero-fillable,
//       binding-friendly) APPLIED ONTO the real structures at create/add-channel - appliers, not
//       storage; the hierarchy is the single source of truth. ⭐AND: the library's existing write-side
//       code is CLUMSY WITH INCONSISTENT INTERFACES & is UP FOR REVISION (Matt) - implementation
//       posture is "fix medlib where medlib is the right fix", not "wrap the warts". (This module is
//       separate from medlib only for navigation convenience during development; it becomes regular
//       public library functions.)
//   D25. VIDEO ADOPTION (Matt, 2026-08-11): WRT_video_m13() takes a chronological list of native
//       video files (caller's order is the contract; optional per-file start times - NULL derives
//       contiguously from the walk durations) & makes them MED citizens in the current segment:
//       renamed to <seg>_n%04d.<NATIVE EXT> (players still play them), optional AES-256-CTR
//       encryption (video_data_encryption_level; nonce = file_UID; container bytes only - identity,
//       parity & repair need no keys), MED universal header appended as a FOOTER (identity for
//       parity/restore), PCRC after it (self-checking immediately; the parity build refreshes it),
//       .vidx built per keyframe via the container walk (VID_walk_m13), video metadata settled at
//       close from the walk facts, & video parity classes added to the segment-closure masks.
//       A file the walker cannot parse is adopted OPAQUELY (renamed + footer + pcrc, no index
//       entries) with a warning - the bytes are preserved either way. v1: video with segmenting
//       NONE (multi-segment video rides the same slot-reuse machinery later).
//   D24. MID-RECORDING FILTER TOGGLES (Matt, 2026-08-10): LNF may be switched on/off during a
//       recording (the 2026-08-09 block-flag semantics were built for this) - & the ANTIALIAS filter
//       needs a toggle too: some labs use the sharpness of stimulation artifacts as a trigger, so
//       they want antialiasing OFF during stimulation stretches of a decimated recording (aliasing
//       accepted - their data, their call). Both toggles share ONE boundary procedure: full chain
//       drain (every stage's tail) + the current partial block emitted SHORT - so no block ever
//       mixes filtered & unfiltered samples & the per-block flag stays truthful - then the stage
//       flips & head-restarts. The decimator's phase carry SURVIVES the boundary: indices stay
//       locked to time through any number of toggles. Toggling a stage on that was off at start is
//       supported (machinery configured up front where cheap [AA filtps whenever decimating], or
//       lazily [LNF scratch]); a chainless channel engages the chain at first toggle-on. D12 stagger
//       re-arms at the boundary (same lockstep-refill argument as discontinuities).
//   D23. FILTER CHAIN & MAGIC RINGS (Matt, 2026-08-10, "the fun part"): filtered data flows as sf8
//       through a VM double-mapped ("magic") ring - any window <= ring length is a LINEAR view, so
//       filter code never sees the wrap. One ring per filtered channel; STAGES are cursors on it:
//       each stage owns finished_idx (ABSOLUTE sample index - the decimator's anti-slip discipline;
//       ring position = idx % len) & processes up to the previous stage's pointer minus its own
//       lookahead lag. The LAST stage's pointer gates block emission. Stage order is configuration
//       (antialias -> LNF per Matt's sketch); a rate-changing stage (decimation) is TERMINAL: it
//       drains the ring into the existing si4 staging, leaving the emit machinery untouched.
//       ⭐The si4-direct-to-compression path is PRESERVED (Matt: "probably a real path for some") -
//       the chain engages only when a filter stage is enabled. SIZING: ingest chunks through free
//       space, so the ring needn't fit a whole call: default = 2 x (sum of stage lags + input
//       samples per block), page-rounded; options override for tuning.
//   D22. HUMAN-READABLE START TIME (Matt, 2026-08-10: "Perhaps we should allow a human readable
//       start time field or fields?"): options.session_start_string, "YYYY-MM-DD HH:MM:SS[.ffffff]"
//       (space or 'T' separator), interpreted as LOCAL time at the recording site & translated to
//       uutc against the RESOLVED time constants (the NRD2MED/CSC2MED pattern: timegm() minus
//       standard_UTC_offset). Parsed at create AFTER timezone resolution, so create's step 1 is now
//       constants-then-parse-then-RTO. Used only when session_start_time was not set explicitly.
//       WRT_parse_time_string_m13() is the public helper (level-resolved constants).
//   D21. IMPLEMENTATION CORRECTIONS to D19 (2026-08-10, from the m13 structs themselves): password
//       HINTS & the anonymized subject ID are REAL METADATA SECTION-1 FIELDS in MED 1.1 (anonymized
//       ID moved there from the UH) -> they live in the METADATA template, & the hint fields leave
//       options. Encryption LEVELS however are NOT metadata fields in m13 (that was m12 section 1):
//       they are UNIVERSAL-HEADER fields stamped by G_set_encryption_map_m13() on every UH at
//       creation -> all four levels (section 2, section 3, data, records) are OPTIONS. Also per Matt:
//       "Geotag Data" is a STANDARD rc field (DHN_Acq will adopt it), & G_location_info_m13()
//       (ipinfo.io via curl, 5 s timeout, clean FALSE offline - air-gap safe because optional)
//       implements WRT_acquire_geotag_m13() for real.
//   D20. RESOLVED (Matt, 2026-08-10): TIME/LOCATION CONTEXT IS STEP ZERO of a write - "I usually set
//       up the time/location context first when starting a write." Sequence (the DHN_Acq pattern,
//       m13-ized): (a) claim the WRITER'S pg FIRST - G_set_time_constants_m13() writes into the
//       current thread's pg (G_proc_globs_m13(NULL)), so the D7 pg step must precede it; this is the
//       "new" usage of G_proc_globs_new_m13() (m12 had one global set; m13 constants are per-pg).
//       (b) fill TIMEZONE_INFO_m13 from the template's section-3 country/territory/timezone-acronym
//       fields & call G_set_time_constants_m13(&tz, start_time, FALSE) - ⭐prompt MUST be FALSE here
//       (the ambiguity prompt is scanf: would hang a mex/binding); the library already collapses
//       multiple table rows with identical constants without prompting, so a REAL ambiguity becomes a
//       clean error telling the caller which section-3 field to add. Passing a nonzero start_time
//       also generates the RTO (G_generate_recording_time_offset_m13); pass 0 when the RTO is not
//       wanted (OUTC mode / apply_recording_time_offset FALSE - follow lib conventions there).
//       (c) the payoff: the matched table row's resolved country/territory/UTC offsets/DST codes are
//       retained in pg->time_constants, & G_init_metadata_m13() SEEDS SECTION 3 from them (calculated
//       values written into metadata); the caller's explicit template values still win. All existing
//       machinery - medwrt orchestrates, adds nothing.
//   D19. RESOLVED (Matt, 2026-08-10): "WRT_METADATA looks like it should just be a library
//       METADATA structure" - CORRECT & REMOVED. Callers, the RC/CS loaders (D18) & the binding
//       structures fill a METADATA_m13 template directly; WRT_metadata_defaults_m13()
//       initializes one to library defaults/NO_ENTRYs; per-channel overrides are templates too,
//       merged field-wise against the sections' NO_ENTRY conventions (D1). Field relocations:
//       encryption LEVELS are real section-1 fields -> set in the template (negative level = encrypt
//       on write, the existing convention); PASSWORDS are not fields -> stay in options; time_mode /
//       session_start_time are write-time decisions, not MED fields -> move to options; the lat/lon
//       convenience fields go with the costume - WRT_format_geotag_m13() writes straight into the
//       template's section-3 geotag fields (bindings may sugar this). Options keeps only what has no
//       MED-field counterpart; where an option has a metadata CONSEQUENCE the engine records it
//       (decimation_frequency -> recorded section-2 sampling_frequency; template states INPUT rate).
//   D17. RC REVIEW (Matt, 2026-08-10: "the RC is a summary of decisions that have to be made in
//       writing but don't end up as explicit fields in the data"). ABSORBED into the surface:
//       segment_description (+ per-segment update via WRT_new_segment_m13's optional description),
//       time-base units factor/description, apply_recording_time_offset (UUTC mode may store TRUE
//       times - RTO application is a choice, not an inevitability), record_encryption_level (data &
//       section levels alone were not enough; per-record-TYPE levels - DHN_Acq has four - via the
//       hierarchy per D16), the FOUR record-structure toggles matching G_alloc_session_m13's
//       signature (session records / segmented session records / channel records / segment records),
//       & parity (NONE / ALL FILES / ALL DATA FILES). Convention kept: VDS_LFP_high_fc == NO_ENTRY =>
//       falls back to the high-frequency-filter setting (the RC's "USE HIGH FREQUENCY FILTER SETTING").
//       DELIBERATELY APP-SIDE (acquisition/deployment concerns): channel-spec files, data directories
//       & cloud archiving pipeline, packet buffers, alert thresholds & remote notifications (the
//       post-block callback carries the facts), watchdog, all Neuralynx networking & port-record
//       specifics (generic write_record covers the record type), auto-segment TIME-OF-DAY anchoring
//       (MANUAL segmenting covers it; interval AUTO is in).
//   D18. CONFIG LOADERS (Matt, 2026-08-10): the C structs are the SINGLE DEFINITION - the MATLAB/
//       Python binding structures mirror them field-for-field (string enums translated at the
//       binding; ⭐RULE: no field name is shared between the metadata & options structs, so flat
//       name-value/kwargs auto-route). In mex & in our own apps the fields COME FROM RC & CS FILES,
//       so medwrt provides the loaders (reusing RC_read_field_m13 & the DHN_Acq RC field VOCABULARY
//       for compatibility): WRT_config_read_m13() fills both structs from an RC file (absent
//       fields keep defaults; unknown fields ignored - app-specific ones live in the same file);
//       WRT_cs_read_m13() fills channel names + per-channel override structs from a CS file
//       (unspecified entries = NO_ENTRY = inherit via the D1 merge). A mex/binding may thus accept
//       an rc_file/cs_file argument in place of - or merged under - explicit structures.
//   D9. RESOLVED (Matt, 2026-08-09): discontinuity handling is a MODE - options.discontinuity_mode:
//       EXPLICIT: only WRT_mark_discontinuity_m13() creates gaps; an online start_time beyond tolerance
//                 is an ERROR (catches caller time bugs). The offline default.
//       IMPLICIT: an online start_time beyond tolerance AUTO-MARKS a discontinuity (dropout streams
//                 just work); within tolerance it only refines block timing (jitter absorption).
//       Either way the explicit call always works, & the library's own implicit marks (session & segment
//       first blocks) apply in both modes.
//   D7. Writer-side proc-globs: WRT_create_session_m13() calls G_proc_globs_new_m13() itself when the
//       calling thread's pg is already bound to a live session - the "second write session in one
//       thread" hazard is resolved inside the API where no shallow user ever sees it.
//
// LINKED INTO libmed_m13 (mklibs.sh) but implementation is STUBBED until the design settles - every
// entry point exists & compiles; unimplemented ones set an error & fail cleanly.



//**********************************************************************************//
// MARK: WRT Structures
//**********************************************************************************//

// WHAT this recording is -> the library's own METADATA_m13, used as a TEMPLATE (D19): section 1
// (encryption levels; negative = encrypt on write), section 2 (sampling_frequency [REQUIRED - the rate
// of the samples the caller provides], filter settings, power_line_frequency, units, descriptions,
// acquisition_channel_number [NO_ENTRY => auto-set to add order, D14]...), section 3 (subject,
// recording location, geotag, UTC offsets / timezone). WRT_metadata_defaults_m13() initializes a
// template; per-channel overrides are templates too, merged field-wise against the sections' NO_ENTRY
// conventions (D1). Geotag: RECOMMENDED FORMAT ISO 6709 (international standard since 1983, current ed.
// 2022): compact ASCII, sign-explicit, optional altitude, explicit CRS, e.g.
// "+40.68941-074.04478+0025.0CRSWGS_84/"; datum WGS 84 (what GPS emits; what GeoJSON/RFC 7946
// mandates). MED's geotag_format field lets other formats coexist later, so this locks nothing in.
// WRT_format_geotag_m13() fills the template's geotag fields from lat/lon/alt.

// HOW to write -> CPS directives/parameters, filters, security, structure & behavior
typedef struct {
	// compression
	ui8	codec;				// a CPS_DF_*_ALGORITHM_m13 flag; default CPS_DF_PRED2_ALGORITHM_m13
	tern	fall_through_to_best;		// default TRUE
	sf8	VDS_threshold;			// codec-specific knobs; NO_ENTRY when unused
	sf8	VDS_LFP_high_fc;		// NO_ENTRY => falls back to high_frequency_filter_setting (D17, RC convention)
	// time (D2/D19: write-time decisions, not MED fields)
	si1	time_mode;			// WRT_TIME_* below (D2)
	si8	session_start_time;		// uutc (or CURRENT_TIME_m13) in UUTC mode; first-sample offset in OUTC mode (typically 0)
	si1	session_start_string[64];	// D22: human-readable alternative, "YYYY-MM-DD HH:MM:SS[.ffffff]", LOCAL time at the
						// recording site (translated against the resolved timezone); used when session_start_time is unset
	// security (D21): passwords are never stored anywhere; HINTS & the anonymized subject ID are real
	// section-1 fields -> METADATA template. Encryption LEVELS are UH fields in m13 (stamped on every
	// UH by G_set_encryption_map_m13() at creation) -> all four are options
	si1	level_1_password[MAX_PASSWORD_STRING_BYTES_m13];
	si1	level_2_password[MAX_PASSWORD_STRING_BYTES_m13];
	si1	level_3_password[MAX_PASSWORD_STRING_BYTES_m13];
	si1	section_2_encryption_level;
	si1	section_3_encryption_level;
	si1	data_encryption_level;
	si1	video_data_encryption_level;	// D25: native container bytes, AES-256-CTR (footer & pcrc excluded); DHN default preference is 2 (video is subject-identifying)
	si4	encode_workers;		// D26: worker-pool size for distributor mode (0 = caller-thread encode; WRT_packets_m13 required when > 0)
	si1	record_encryption_level;	// D17: all records; per-record-TYPE levels via the hierarchy (D16)
	tern	apply_recording_time_offset;	// D17: UUTC mode may store TRUE times (FALSE); default TRUE (privacy offset)
	// library-applied processing (2026-08-09 semantics: filtered bit + deposited/estimated byte 0)
	tern	line_noise_filter;		// template LNF on the way in; sets CPS_DF_LINE_NOISE_FILTERED_m13
	si1	worker_affinity[64];		// D31d pool worker cpu set, e.g. "4-15"; EMPTY = all cores set EXPLICITLY
						// (pthread_create inherits the spawning thread's mask - the pool spawns on the
						// first WRT_packets caller, which DHN_Acq pins to "2-3": 12 workers on 2 cores)
	ui8	noise_scores;			// mask of CPS_DF_*_SCORE_m13 directives; 0 = none
	// decimation (D8; per-channel by nature - THE per-channel motivator). The METADATA template's
	// section-2 sampling_frequency is the rate of the samples the CALLER PROVIDES; when decimation_frequency is set the library antialias-
	// filters (unless antialias_filter == FALSE_m13; auto-disabled for full-rate VDS, which antialiases
	// intrinsically - but still REQUIRED when decimating), then resamples by fractional-step linear
	// interpolation with cross-block phase continuity (the DHN_Acq decimate() algorithm), & the RECORDED
	// section-2 sampling_frequency is the OUTPUT rate. block_duration is in OUTPUT time.
	sf8	decimation_frequency;		// output rate; NO_ENTRY = no decimation
	// amplitude quantum (2026-08-14): the CALLER's samples may carry resolution finer than one unit
	// of the recorded amplitude units - acquisition hardware commonly delivers counts whose LSBs sit
	// below the units the file declares (Neuralynx: 1 uV at bit 6, so 24-bit counts hold 18 valid
	// bits). The chain must filter at the NATIVE resolution & round to the coarser grid AFTERWARD:
	// rounding first discards what the filter needs, & not rounding at all pays full entropy for
	// detail below the noise floor (measured: +6.0 bits/sample on 40 kHz neural data, 5.55 -> 11.18).
	// This is the terminal quantizer m12's apps applied themselves before CMP_encode; the library
	// owns the seam now. Value = caller units per recorded unit (Neuralynx: 64.0). NOT a library
	// constant - no width, shift or hardware is assumed here; 1.0 (the default) is a no-op.
	sf8	amplitude_quantum;		// caller sample units per recorded unit; 1.0 = no requantization
	tern	antialias_filter;		// default UNKNOWN = automatic (on when decimating, VDS rule above)
	ui4	initial_block_samples;		// D12: first-block target after any session-wide discontinuity (incl. start); 0 = equal to the steady-state block size (no stagger). Per-channel via D1 merge; apps set spreading policy themselves (or use WRT_spread_block_phases_m13())
	// discontinuities (D9)
	si1	discontinuity_mode;		// WRT_DISCONT_* below
	sf8	discontinuity_tolerance;	// in sample periods; online start_time deviation beyond this = gap (default 1.5)
	// structure & behavior
	sf8	block_duration;			// seconds; XOR with block_samples (other = NO_ENTRY)
	ui4	block_samples;
	si1	segmenting_mode;		// WRT_SEG_* below (D3)
	sf8	auto_segment_hours;		// AUTO mode rollover interval
	// record structure & file-set toggles (D17; match G_alloc_session_m13's signature)
	tern	session_records;		// session-level record files
	tern	seg_session_records;		// segmented session records (SSR, .recd)
	tern	channel_records;		// channel-level record files
	tern	segment_records;		// segment-level record files
	si1	parity;				// WRT_PARITY_* below (D17)
	si1	exists_behavior;		// WRT_EXISTS_* below
	tern	threaded;			// reserved (D5); v1 ignores
} WRT_OPTIONS_m13;

// time_mode (D2)
#define WRT_TIME_UUTC_m13		((si1) 0) // true wall time known: session_start_time is uutc (or CURRENT_TIME_m13); library generates the RTO & converts that ONE time to oUTC
#define WRT_TIME_OUTC_m13		((si1) 1) // no true wall time claimed: session_start_time is a pure offset (zero is typical); no RTO generated
// NOTE (both modes): ALL ongoing time arguments (WRT_packets/WRT_samples start_time, WRT_mark_discontinuity, record
// start times, session_sf8 file starts) are oUTC - the library uses oUTC universally; RTO is added back only for
// display (when apply_RTO is unset, RTO is zero & oUTC == full uutc)

// segmenting_mode (D3)
#define WRT_SEG_NONE_m13		((si1) 0) // whole session is one segment (default)
#define WRT_SEG_AUTO_m13		((si1) 1) // duration-based rollover (auto_segment_hours)
#define WRT_SEG_MANUAL_m13	((si1) 2) // caller segments via WRT_new_segment_m13()

// discontinuity_mode (D9)
#define WRT_DISCONT_EXPLICIT_m13	((si1) 0) // only WRT_mark_discontinuity_m13() creates gaps; out-of-tolerance online start_time = ERROR (default)
#define WRT_DISCONT_IMPLICIT_m13	((si1) 1) // out-of-tolerance online start_time AUTO-MARKS a discontinuity

// parity (D17)
#define WRT_PARITY_NONE_m13	((si1) 0)
#define WRT_PARITY_ALL_m13	((si1) 1) // all files
#define WRT_PARITY_DATA_m13	((si1) 2) // data files only

// exists_behavior
#define WRT_EXISTS_FAIL_m13	((si1) 0) // default: refuse to touch an existing session
#define WRT_EXISTS_OVERWRITE_m13	((si1) 1)
#define WRT_EXISTS_RENAME_m13	((si1) 2) // append numeric suffix to the new session

// medlib-internal WRT defines - all expand only inside medlib_m13.c (the types & tables they
// reference are .c-local by design - D16); defined here per the header-only defines rule

// per-channel pool_state (D30)
#define WRT_POOL_IDLE_m13	((ui1) 0)
#define WRT_POOL_QUEUED_m13	((ui1) 1)
#define WRT_POOL_RUNNING_m13	((ui1) 2)

// adopted-video encryption io (D25)
#define WRT_VID_ENCRYPT_CHUNK_BYTES_m13	((si8) 1 << 22)  // 4 MiB encryption io chunks (the NAT2MED constant)

// internal per-channel write state accessor
#define WRT_INT_m13(ws)	((WRT_INT_m13 *) (ws)->chan_state_int)

// RC field table (D18; the table itself lives in medlib_m13.c)
#define WRT_RC_MD_m13		((si1) 0)  // row targets the METADATA_m13 template
#define WRT_RC_OPT_m13		((si1) 1)  // row targets WRT_OPTIONS_m13
#define WRT_RC_NO_SENT_m13	((si8) 0x8000000000000000)  // "this integer row has no NO-ENTRY value"
#define WRT_RC_N_FIELDS_m13	((si4) (sizeof(WRT_rc_fields_m13) / sizeof(WRT_rc_fields_m13[0])))
// row builders (bytes from sizeof => cannot drift from the structs)
#define WRT_MDSTR_m13(nm, fld, nts) \
	{.name = nm, .type = "string", .default_str = "NO ENTRY", .notes = nts, .dest = WRT_RC_MD_m13, .kind = WRT_RC_KSTR_m13, \
	.offset = offsetof(METADATA_m13, fld), .bytes = (si4) sizeof(((METADATA_m13 *) 0)->fld), .no_entry_f = (sf8) NAN, .no_entry_i = WRT_RC_NO_SENT_m13}
#define WRT_MDF8_m13(nm, fld, dflt, noent, nts) \
	{.name = nm, .type = "float", .default_str = dflt, .notes = nts, .dest = WRT_RC_MD_m13, .kind = WRT_RC_KSF8_m13, \
	.offset = offsetof(METADATA_m13, fld), .scale = (sf8) 1.0, .no_entry_f = noent, .no_entry_i = WRT_RC_NO_SENT_m13}
#define WRT_OPTSTR_m13(nm, fld, nts) \
	{.name = nm, .type = "string", .default_str = "NO ENTRY", .notes = nts, .dest = WRT_RC_OPT_m13, .kind = WRT_RC_KSTR_m13, \
	.offset = offsetof(WRT_OPTIONS_m13, fld), .bytes = (si4) sizeof(((WRT_OPTIONS_m13 *) 0)->fld), .no_entry_f = (sf8) NAN, .no_entry_i = WRT_RC_NO_SENT_m13}
#define WRT_OPTF8_m13(nm, fld, dflt, noent, scl, nts) \
	{.name = nm, .type = "float", .default_str = dflt, .notes = nts, .dest = WRT_RC_OPT_m13, .kind = WRT_RC_KSF8_m13, \
	.offset = offsetof(WRT_OPTIONS_m13, fld), .scale = scl, .no_entry_f = noent, .no_entry_i = WRT_RC_NO_SENT_m13}
#define WRT_OPTINT_m13(knd, nm, fld, dflt, noent, nts) \
	{.name = nm, .type = "integer", .default_str = dflt, .notes = nts, .dest = WRT_RC_OPT_m13, .kind = knd, \
	.offset = offsetof(WRT_OPTIONS_m13, fld), .no_entry_f = (sf8) NAN, .no_entry_i = noent}
#define WRT_OPTTERN_m13(nm, fld, dflt, nts) \
	{.name = nm, .type = "ternary", .default_str = dflt, .notes = nts, .opts = WRT_yes_no_opts_m13, .n_opts = 2, \
	.dest = WRT_RC_OPT_m13, .kind = WRT_RC_KTERN_m13, .offset = offsetof(WRT_OPTIONS_m13, fld), .no_entry_f = (sf8) NAN, .no_entry_i = WRT_RC_NO_SENT_m13}
#define WRT_OPTMAP_m13(knd, nm, fld, dflt, oarr, varr, nts) \
	{.name = nm, .type = "string", .default_str = dflt, .notes = nts, .opts = oarr, .opt_vals = varr, .only = TRUE_m13, \
	.n_opts = (si4) (sizeof(oarr) / sizeof(oarr[0])), .dest = WRT_RC_OPT_m13, .kind = knd, \
	.offset = offsetof(WRT_OPTIONS_m13, fld), .no_entry_f = (sf8) NAN, .no_entry_i = WRT_RC_NO_SENT_m13}

// Tier 2 handle. D16: the internals ARE the MED hierarchy - sess is a real SESS_m13 whose channels/
// segments/FPSes are the same structures the read path uses; per-channel write state (sf8 magic ring,
// initial/target block sizes, aggregates cursor) attaches alongside the real CHAN_m13. The curated
// structs below are the applied INPUT, retained only as the defaults template for late add_channel
// merges. Open by philosophy - deep apps navigate it like any open session.
typedef struct {
	SESS_m13		*sess;		// the real hierarchy (single source of truth)
	METADATA_m13		metadata;	// session defaults TEMPLATE (D19: the real library struct; appliers, not storage - D16)
	WRT_OPTIONS_m13	options;
	si4			n_chans;
	si4			n_vid_chans;	// D25
	tern			writing_started; // add_channel refused after first write_samples
	pthread_mutex_t_m13	start_mutex;	// first-write latch: per-channel writer THREADS may race the
						// G_alloc_session_m13() trigger (Matt: channels encode in threads).
						// Steady-state writes touch only their own channel's state - no lock.
						// mark_discontinuity / flush / close are CONTROL-THREAD operations:
						// they touch every channel; caller quiesces the writers first.
	si1			path[PATH_BYTES_m13]; // resolved session directory (RENAME exists-behavior may alter the requested path)
	PROC_GLOBS_m13		*pg;		// this write session's proc globals (claimed at create - D20a)
	FPS_m13			*proto_fps;	// prototype metadata FPS (template applied); G_alloc_session_m13() inherits from it at first write (D14: channel count is known then)
	void			*chan_state_int;// internal per-channel write state (public struct once the surface settles - D16)
} WRT_SESS_m13;


//**********************************************************************************//
// MARK: WRT Prototypes
//**********************************************************************************//

// geolocation
tern	WRT_format_geotag_m13(sf8 latitude, sf8 longitude, sf8 altitude_meters, si1 *format_out, si1 *data_out);
	// formats an ISO 6709 point string (datum WGS 84) into data_out (>= METADATA_GEOTAG_DATA_BYTES_m13)
	// & sets format_out (>= METADATA_GEOTAG_FORMAT_BYTES_m13) to "ISO 6709". altitude_meters NAN => omitted.
tern	WRT_acquire_geotag_m13(si1 *format_out, si1 *data_out);
	// STUB: automatic geolocation (OS location services / IP geolocation) -> WRT_format_geotag_m13().
	// Must remain OPTIONAL & fail cleanly with no network: DHN deployments are AIR-GAPPED by requirement.

// config loaders (D18): fill the structs from RC / CS files (the DHN_Acq field vocabulary)
tern	WRT_config_read_m13(const si1 *rc_path, METADATA_m13 *md, WRT_OPTIONS_m13 *opt);
tern	WRT_config_write_m13(const si1 *rc_path, METADATA_m13 *md, WRT_OPTIONS_m13 *opt); // template generator: full standard-field rc with current values (NULL structs => "DEFAULT"); doubles as the vocabulary documentation
si4	WRT_cs_read_m13(const si1 *cs_path, si1 ***chan_names, METADATA_m13 **md_overrides, WRT_OPTIONS_m13 **opt_overrides);
	// returns n_chans; allocates the three arrays (caller frees); overrides are D1-merge-ready

// magic ring buffer (D23): VM double-mapped - element i & element (i + len) address the SAME memory,
// so any window of <= len elements starting anywhere is linear. Element-size agnostic (Matt, D24
// symmetry: si4 output rings for EVERY channel - the ring IS the block accumulator & the encoder
// reads linear windows from it zero-copy; sf8 rings carry the filter chain).
typedef struct {
	void	*data;		// double-mapped base (2 * len elements addressable)
	si8	len;		// ring length in ELEMENTS (byte size is page-multiple)
	si8	el_bytes;	// element size
	void	*_map_hdl;	// platform mapping handle (internal)
} WRT_RING_m13;

tern	WRT_ring_alloc_m13(WRT_RING_m13 *ring, si8 min_elements, si8 el_bytes);	// rounds up to page multiple
void	WRT_ring_free_m13(WRT_RING_m13 *ring);

// time-string translation (D22): "YYYY-MM-DD HH:MM:SS[.ffffff]" (space or 'T'), LOCAL time at the
// recording site -> uutc, against the LEVEL's resolved time constants; TIME_NO_ENTRY_m13 on failure
si8	WRT_parse_time_string_m13(void *level_header, const si1 *time_str);

// defaults (zero-filled structs from bindings are equivalent to calling these)
void	WRT_metadata_defaults_m13(METADATA_m13 *md);	// initialize a template to library defaults/NO_ENTRYs (D19)
void	WRT_options_defaults_m13(WRT_OPTIONS_m13 *opt);

// Tier 1 - one-shot: samples[chan][samp] (or NULL-terminated set of channel arrays), all channels same
// length & rate in v1; channel names from chan_names (NULL => "ch_0001" style)
tern	WRT_session_m13(const si1 *sess_path, si4 **samples, si4 n_chans, si8 n_samps, sf8 sampling_frequency,
			      si1 **chan_names, METADATA_m13 *md, WRT_OPTIONS_m13 *opt);
			      // chan_names/md/opt may ALL be NULL (the 90% path above); sampling_frequency
			      // is authoritative for the session (md may refine everything else)
tern	WRT_session_sf8_m13(const si1 *sess_path, sf8 **samples, si4 n_chans, si8 n_samps, sf8 sampling_frequency,
			      si1 **chan_names, METADATA_m13 *md, WRT_OPTIONS_m13 *opt);
			      // the Python/MATLAB-native one-shot (D13): physical-units arrays; the library
			      // AUTO-RANGES amplitude_units_conversion_factor unless md supplies one

// Tier 2 - streaming
WRT_SESS_m13	*WRT_create_session_m13(const si1 *sess_path, METADATA_m13 *md, WRT_OPTIONS_m13 *opt);
si4	WRT_add_ts_channel_m13(WRT_SESS_m13 *ws, const si1 *chan_name,
			       METADATA_m13 *md_override, WRT_OPTIONS_m13 *opt_override);  // FIELD-WISE merge over session defaults; NULL => inherit all (D1)
// D27: per-channel telemetry - the throttle controller's observation surface (& the app's buffer
// gauge / startup capacity prediction). SNAPSHOT semantics: single racy read of live cursors -
// values are instantaneously consistent enough for control decisions, never for sample accounting.
typedef struct {
	si8	pending_in;		// chain input samples ingested but not yet serviced (0 for direct)
	si8	pending_out;		// output samples accumulated but not yet encoded
	sf8	in_ring_fill;		// chain ring occupancy [0,1] (0 for direct)
	sf8	out_ring_fill;		// output ring occupancy [0,1]
	sf8	encode_lag_secs;	// data time waiting to be encoded ((pending_in/in_fs) + (pending_out/fs))
	sf8	visit_cost_ewma_us;	// worker visit cost, exponentially weighted mean (pool mode; 0 in caller mode)
	sf8	visit_cost_peak_us;	// decaying peak (throttle budgets against bursts, not means - VDS cost is correlated)
	si8	visits;			// total worker visits
	si4	events_pending;		// unprocessed boundary events
	sf8	line_noise_score;	// most recent encoded block's line-noise score [0,1] (0 clean); NAN before the first scored block (or scoring off)
} WRT_TELEM_m13;

tern	WRT_set_codec_m13(WRT_SESS_m13 *ws, si4 chan_idx, ui4 codec);  // D28: mid-recording codec switch at a clean block boundary (the throttle VDS->PRED2 rung; CHANNEL_NUMBER_ALL_CHANNELS_m13 = all)
tern	WRT_set_encode_workers_m13(WRT_SESS_m13 *ws, si4 n_workers);  // D31b: live pool resize (CPU-reclamation actuator; grow = new PROC jobs, shrink = index-based exits at visit boundaries; ceiling = logical cores; pre-start = config only)
si8	WRT_bytes_written_m13(WRT_SESS_m13 *ws, si8 *segment_bytes);  // D32: bytes on disk (session return; current segment via arg) - session size, bits/sample & time-to-full readouts; snapshot semantics
tern	WRT_set_VDS_goal_attempts_m13(WRT_SESS_m13 *ws, si4 chan_idx, si4 attempts);  // D28: cap VDS refinement rounds live (the cheap half-step before a full PRED2 switch; 0 = library default)
tern	WRT_telemetry_m13(WRT_SESS_m13 *ws, si4 chan_idx, WRT_TELEM_m13 *t);
tern	WRT_packets_m13(WRT_SESS_m13 *ws, void *packets, si8 n_packets, si8 packet_bytes, si8 samples_offset, si4 *chan_map, si8 start_time);  // INTERLEAVED entry (acquisition/DAT): packet = all channels' si4 values at one sample time; stride packet_bytes, values at samples_offset; chan_map[w] = packet slot for channel w (NULL = identity, negative = skip); start_time as WRT_samples_m13
tern	WRT_samples_m13(WRT_SESS_m13 *ws, si4 chan_idx, si4 *samps, si8 n_samps, si8 start_time);  // any length; start_time = caller's wall-time estimate for samps[0] (online), TIME_NO_ENTRY_m13 = continuous flow (offline; D2)
tern	WRT_samples_sf8_m13(WRT_SESS_m13 *ws, si4 chan_idx, sf8 *samps, si8 n_samps, si8 start_time);  // physical-units input (D13); quantized by amplitude_units_conversion_factor at encode
tern	WRT_flush_m13(WRT_SESS_m13 *ws);  // D15: deep-user flush - partials written as SHORT blocks, NO discontinuity mark
si4	WRT_add_vid_channel_m13(WRT_SESS_m13 *ws, const si1 *chan_name);  // D25: register a video channel (before writing starts); returns video channel index
tern	WRT_video_m13(WRT_SESS_m13 *ws, si4 vid_chan_idx, si1 **file_paths, si4 n_files, si8 *file_start_times);  // D25: adopt native video files (chronological) into the current segment; times NULL => contiguous
tern	WRT_set_line_noise_filter_m13(WRT_SESS_m13 *ws, si4 chan_idx, tern on);  // D24; chan_idx or CHANNEL_NUMBER_ALL_CHANNELS_m13
tern	WRT_set_antialias_filter_m13(WRT_SESS_m13 *ws, si4 chan_idx, tern on);   // D24; decimating channels only (AA exists for decimation)
tern	WRT_mark_discontinuity_m13(WRT_SESS_m13 *ws, si8 new_start_time);  // applies to all channels (D2)
tern	WRT_new_segment_m13(WRT_SESS_m13 *ws, const si1 *segment_description);  // MANUAL segmenting (D3): close current segment, open the next; NULL description = keep current (D17)
struct REC_WRITE_m13;  // defined in medrec_m13.h (included below - the record machinery lives there)
tern	WRT_record_m13(WRT_SESS_m13 *ws, struct REC_WRITE_m13 *req);  // D4: session-level record via the generic medrec writer (REC_write_m13); pool mode drains first  // D4: Note only in v1
tern	WRT_close_session_m13(WRT_SESS_m13 *ws);  // flush, settle UHs, write final metadata, free

// post-block callback (D11c): invoked after each block is encoded & written; read-only facts for
// display taps, stats & alerts (e.g. line-noise alert = noise_scores[0]). Called on the writing thread.
typedef struct {
	si4	chan_idx;
	si8	start_time;			// block start (uutc/oUTC per time_mode)
	ui4	n_samples;
	ui4	total_block_bytes;
	ui4	algorithm;			// CMP_BF_*_ENCODING_m13 flag actually used
	ui1	noise_scores[4];		// 0xFF = not computed
	tern	discontinuity;
} WRT_BLOCK_INFO_m13;
typedef void	(*MED_write_block_callback_m13)(WRT_SESS_m13 *ws, const WRT_BLOCK_INFO_m13 *info, void *user_data);
tern	WRT_set_block_callback_m13(WRT_SESS_m13 *ws, MED_write_block_callback_m13 fn, void *user_data);

// optional convenience (D12): the DHN_Acq spreading policy - within each group of channels sharing a
// steady-state block size, set channel k's initial_block_samples to round(full * k/N) (input domain,
// decimation-aware). Call after add_channel calls, before the first write. Purely optional: any app may
// set per-channel initials itself instead.
tern	WRT_spread_block_phases_m13(WRT_SESS_m13 *ws);




//**********************************************************************************//

#include "medrec_m13.h"


#endif // MEDLIB_IN_m13

