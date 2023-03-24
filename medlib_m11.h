
#ifndef MEDLIB_IN_m11
#define MEDLIB_IN_m11

//**********************************************************************************//
//*******************************  MED 1.0.1 C Library  ****************************//
//**********************************************************************************//


// Multiscale Electrophysiology Data (MED) Format Software Library, Version 1.0.1
// Written by Matt Stead


// LICENSE & COPYRIGHT:

// MED library source code (medlib) is copyrighted by Dark Horse Neuro Inc, 2021 (Matt Stead & Casey Stengel)

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
// Dan Crepeau, Jan Cimbálnik, Jon Lange, and Jon Halford for their contributions
// in design, coding, testing, implementation, and adoption.

// The encryption / decryption algorithm is the 128-bit AES standard ( http://www.csrc.nist.gov/publications/fips/fips197/fips-197.pdf ).
// AES routines (128 bit only) are included in the library, with attribution, for convenience.

// The hash algorithm is the SHA-256 standard ( http://csrc.nist.gov/publications/fips/fips180-4/fips-180-4.pdf ).
// Basic SHA-256 routines are included in the library, with attribution, for convenience.

// Strings are encoded in the Universal Character Set standard, ISO/IEC 10646:2012 otherwise known as UTF-8.
// ( http://standards.iso.org/ittf/PubliclyAvailableStandards/c056921_ISO_IEC_10646_2012.zip )
// Basic UTF-8 manipulation routines are included in the library, with attribution, for convenience.

// Error detection is implemented with 32-bit cyclic redundancy checksums (CRCs).
// Basic CRC-32 manipulation routines are included in the library, with attribution, for convenience.


// USAGE:

// The library is optimized for 64-bit operating systems on 64-bit processors with 64-bit words and addressing.
// However, it can be used with in 32-bit contexts without modification at a performance cost.

// The library is written with tab width = indent width = 8 spaces and a monospaced font.
// Tabs are tabs characters, not spaces.
// Set your editor preferences to these for intended alignment.

// The library contains some non-standard structures:
// 	required compiler option (gcc, clang): -fms-extensions
// 	suppress warnings with: -Wno-microsoft-anon-tag


// VERSIONING:

// All functions, constants, macros, and data types defined in the library are tagged
// with the suffix "_mFL" (for "MED major format 'F', library 'L'").

// MED_FORMAT_VERSION_MAJOR is restricted to single digits 1 through 9
// MED_FORMAT_VERSION_MINOR is restricted to 0 through 254, minor version resets to zero with new major format version
// MED_LIBRARY_VERSION is restricted to 1 through 254, library version resets to one with new major format version

// MED_FULL_FORMAT_NAME == "<MED_VERSION_MAJOR_m11>.<MED_VERSION_MINOR_m11>"
// MED_FULL_LIBRARY_NAME == "<MED_FULL_FORMAT_NAME_m11>.<MED_LIBRARY_VERSION_m11>"
// MED_LIBRARY_TAG == "<MED_VERSION_MAJOR_m11>.<MED_LIBRARY_VERSION_m11>"

// Examples:
// "_m11" indicates "MED format major version 1, library version 1"
// "_m21" indicates "MED format major version 2, library version 1" (for MED 2)
// "_m213" indicates "MED format major version 2, library version 13" (for MED 2)

// All library versions associated with a particular major format version are guaranteed to work on MED files of that major version.
// Minor format versions may add fields to the format in protected regions, but no preexisting fields will be removed or moved.
// Only library versions released on or after a minor version will make use of new fields, and only if the minor version of the files contains them.
// Backward compatibility will be maintained between major versions if practical.


//**********************************************************************************//
//*********************************  Library Includes  *****************************//
//**********************************************************************************//

#include "targets_m11.h"

#ifdef WINDOWS_m11
// the following is necessary to include <winsock2.h> (or can define WIN32_LEAN_AND_MEAN, but excludes a lot of stuff)
// winsock2.h has to be included before windows.h, but requires WIN32 to be defined, which is usually defined by windows.h
// NEED_WIN_SOCKETS_m11 is defined in "targets.h"
	#ifdef NEED_WIN_SOCKETS_m11
		#ifndef WIN32
			#define WIN32
		#endif
		#include <winsock2.h>
		#pragma comment(lib, "ws2_32.lib") // link with Ws2_32.lib
	#endif
	#include <windows.h>
	#include <io.h>
	#include <direct.h>
	#include <fileapi.h>
	#include <share.h>
	#include <memoryapi.h>
	#include <synchapi.h>
	#define _USE_MATH_DEFINES  // Needed for standard math constants. Must be defined before math.h included.
#endif
#if defined MACOS_m11 || defined LINUX_m11
	#include <unistd.h>
	#include <dirent.h>
	#include <sys/time.h>
	#include <sys/resource.h>
	#include <stdatomic.h>
	#include <sys/mman.h>
	#include <pthread.h>
#endif
#if defined LINUX_m11 || defined WINDOWS_m11
	#include <malloc.h>
#endif
#ifdef MACOS_m11
	#include <malloc/malloc.h>
#endif
#include <stdlib.h>
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
#include <errno.h>
#ifdef MATLAB_m11
	#include "mex.h"
	#include "matrix.h"
#endif



//**********************************************************************************//
//******************************  Elemental Typedefs  ******************************//
//**********************************************************************************//

#ifndef SIZE_TYPES_IN_m11
#define SIZE_TYPES_IN_m11

#include <stdint.h>

typedef uint8_t		ui1;
typedef char		si1;  // Note: the "char" type is not guaranteed to be a signed one-byte integer.  If it is not, library will exit during initialization
typedef uint16_t	ui2;
typedef int16_t		si2;
typedef uint32_t	ui4;
typedef int32_t		si4;
typedef uint64_t	ui8;
typedef int64_t		si8;
typedef float		sf4;
typedef double		sf8;
typedef long double	sf16;

#endif  // SIZE_TYPES_IN_m11

// MED Library Ternary Boolean Schema
typedef si1				TERN_m11;
#define TRUE_m11			1
#define UNKNOWN_m11			0
#define FALSE_m11			-1

// Reserved si4 Sample Values
#define NAN_SI4_m11			((si4) 0x80000000)
#define NEG_INF_SI4_m11           	((si4) 0x80000001)
#define POS_INF_SI4_m11			((si4) 0x7FFFFFFF)
#define MAX_SAMP_VAL_SI4_m11        	((si4) 0x7FFFFFFE)
#define MIN_SAMP_VAL_SI4_m11		((si4) 0x80000002)
// Reserved si2 Sample Values
#define NAN_SI2_m11			((si2) 0x8000)
#define NEG_INF_SI2_m11           	((si2) 0x8001)
#define POS_INF_SI2_m11			((si2) 0x7FFF)
#define MAX_SAMP_VAL_SI2_m11        	((si2) 0x7FFE)
#define MIN_SAMP_VAL_SI2_m11		((si2) 0x8002)



//**********************************************************************************//
//****************  Record Structures Integral to the MED Library  *****************//
//**************  (prototypes & constants declared in medrec_m11.h)  ***************//
//**********************************************************************************//


//*************************************************************************************//
//*******************************   Sgmt: Segment Record   ****************************//
//*************************************************************************************//

// A segment record is entered at the Session and or Channel Level for each new segment
// The encryption level for these records is typically set to the same as for metadata section 2

// Structures
typedef struct {
	si8     end_time;
	union {
		si8     start_sample_number;	// session-relative (global indexing)
		si8     start_frame_number;	// session-relative (global indexing)
	};
	union {
		si8     end_sample_number;	// session-relative (global indexing)
		si8     end_frame_number;	// session-relative (global indexing)
	};
	ui8     segment_UID;
	si4     segment_number;
	si4     acquisition_channel_number;  // REC_Sgmt_v10_ACQUISITION_CHANNEL_NUMBER_ALL_CHANNELS_m11 in session level records
	union {
		sf8     sampling_frequency;  // channel sampling frequency (REC_Sgmt_v10_SAMPLING_FREQUENCY_VARIABLE_m11 in session level records, if sampling frequencies vary across time series channels)
		sf8     frame_rate;  	     // channel frame rate (REC_Sgmt_v10_FRAME_RATE_VARIABLE_m11 in session level records, if frame rates vary across video channels)
	};
} REC_Sgmt_v10_m11;
// Description follows sampling_frequency / frame_rate in structure.
// The description is an aribitrary length array of si1s padded to 16 byte alignment (total of structure + string).


//*************************************************************************************//
//*****************************   Stat: Statistics Record   ***************************//
//*************************************************************************************//

typedef struct {
	si4     minimum;
	si4     maximum;
	si4     mean;
	si4     median;
	si4     mode;
	sf4     variance;
	sf4     skewness;
	sf4     kurtosis;
} REC_Stat_v10_m11;


//*************************************************************************************//
//************   CMP_BLOCK_FIXED_HEADER Structure required in MED section  ************//
//*************************************************************************************//

typedef struct {
	// CMP block header fixed region start
	ui8     block_start_UID;
	ui4     block_CRC;
	ui4     block_flags;
	si8     start_time;
	si4     acquisition_channel_number;
	ui4     total_block_bytes;
	// CMP block encryption start
	ui4     number_of_samples;
	ui2     number_of_records;
	ui2     record_region_bytes;
	ui4     parameter_flags;
	ui2     parameter_region_bytes;
	ui2     protected_region_bytes;
	ui2     discretionary_region_bytes;
	ui2     model_region_bytes;
	ui4     total_header_bytes;
	// CMP block header variable region start
} CMP_BLOCK_FIXED_HEADER_m11;


//**********************************************************************************//
//**********************************  ENCRYPTION  **********************************//
//**********************************************************************************//

// Encryption & Password Constants
#define ENCRYPTION_LEVEL_NO_ENTRY_m11		-128
#define NO_ENCRYPTION_m11			0
#define LEVEL_0_ENCRYPTION_m11			NO_ENCRYPTION_m11
#define LEVEL_1_ENCRYPTION_m11			1
#define LEVEL_2_ENCRYPTION_m11			2
#define LEVEL_0_ACCESS_m11			LEVEL_0_ENCRYPTION_m11
#define LEVEL_1_ACCESS_m11			LEVEL_1_ENCRYPTION_m11
#define LEVEL_2_ACCESS_m11			LEVEL_2_ENCRYPTION_m11
#define LEVEL_1_ENCRYPTION_DECRYPTED_m11        -LEVEL_1_ENCRYPTION_m11
#define LEVEL_2_ENCRYPTION_DECRYPTED_m11        -LEVEL_2_ENCRYPTION_m11
#define ENCRYPTION_BLOCK_BYTES_m11		16      // AES-128
#define ENCRYPTION_KEY_BYTES_m11                176     // AES-128   = ((AES_NR + 1) * AES_NK * AES_NB)
#define PASSWORD_BYTES_m11			ENCRYPTION_BLOCK_BYTES_m11
#define MAX_PASSWORD_CHARACTERS_m11		PASSWORD_BYTES_m11
#define MAX_ASCII_PASSWORD_STRING_BYTES_m11	(MAX_PASSWORD_CHARACTERS_m11 + 1)  // 1 byte per character in ascii plus terminal zero
#define MAX_UTF8_PASSWORD_BYTES_m11		(MAX_PASSWORD_CHARACTERS_m11 * 4)  // up to 4 bytes per character in UTF-8
#define MAX_PASSWORD_STRING_BYTES_m11		(MAX_UTF8_PASSWORD_BYTES_m11 + 1)  // 1 byte for null-termination
#define PASSWORD_VALIDATION_FIELD_BYTES_m11     PASSWORD_BYTES_m11
#define PASSWORD_HINT_BYTES_m11                 256

// Password Data Structure
typedef struct {
	ui1	level_1_encryption_key[ENCRYPTION_KEY_BYTES_m11];
	ui1	level_2_encryption_key[ENCRYPTION_KEY_BYTES_m11];
	si1     level_1_password_hint[PASSWORD_HINT_BYTES_m11];
	si1     level_2_password_hint[PASSWORD_HINT_BYTES_m11];
	ui1	access_level;
	ui1	processed;  // 0 or 1 (not ternary)
} PASSWORD_DATA_m11;



//**********************************************************************************//
//********************************  MED Constants  *********************************//
//**********************************************************************************//

// Versioning Constants
#define MED_FORMAT_VERSION_MAJOR_m11		1  // restricted to single digits 1 through 9
#define MED_FORMAT_VERSION_MINOR_m11		0  // restricted to 0 through 254, minor version resets to zero with new major format version
#define MED_LIBRARY_VERSION_m11                 1  // restricted to 1 through 254, library version resets to one with new major format version
#define MED_VERSION_NO_ENTRY_m11                0xFF
#define MED_FULL_FORMAT_NAME_m11             	"\"" ## MED_VERSION_MAJOR_m11 ## "." ## MED_VERSION_MINOR_m11 ## "\""
#define MED_FULL_LIBRARY_NAME_m11             	"\"" ## MED_FULL_FORMAT_NAME_m11 ## "." ## MED_LIBRARY_VERSION_m11 ## "\""
#define MED_LIBRARY_TAG_m11			"\"_m" ## MED_VERSION_MAJOR_m11 ## MED_LIBRARY_VERSION_m11 ## "\""

// Miscellaneous Constants
#define BASE_FILE_NAME_BYTES_m11                256        // utf8[63]
#define SEGMENT_BASE_FILE_NAME_BYTES_m11        (BASE_FILE_NAME_BYTES_m11 + 8)
#define VIDEO_DATA_BASE_FILE_NAME_BYTES_m11     (SEGMENT_BASE_FILE_NAME_BYTES_m11 + 8)
#define FULL_FILE_NAME_BYTES_m11                1024        // utf8[255]
#define INDEX_BYTES_m11				24
#define BIG_ENDIAN_m11                          0
#define LITTLE_ENDIAN_m11                       1
#define TYPE_BYTES_m11                          5
#define TYPE_STRLEN_m11                         4
#define UID_BYTES_m11                           8
#define UID_NO_ENTRY_m11                        0
#define PAD_BYTE_VALUE_m11                      0x7e        // ascii tilde ("~") as si1
#define FILE_NUMBERING_DIGITS_m11               4
#define FREQUENCY_NO_ENTRY_m11                  -1.0
#define FRAME_RATE_NO_ENTRY_m11                 FREQUENCY_NO_ENTRY_m11
#define FREQUENCY_VARIABLE_m11			-2.0
#define FRAME_RATE_VARIABLE_m11			FREQUENCY_VARIABLE_m11
#define UNKNOWN_NUMBER_OF_ENTRIES_m11           -1
#define SEGMENT_NUMBER_NO_ENTRY_m11             -1
#define FIRST_OPEN_SEGMENT_m11			-2
#define CHANNEL_NUMBER_NO_ENTRY_m11             -1
#define CHANNEL_NUMBER_ALL_CHANNELS_m11         -2
#define FILE_EXISTS_ERROR_m11                   1
#define DOES_NOT_EXIST_m11                      2
#define FILE_EXISTS_m11                         4
#define DIR_EXISTS_m11                          8
#define SIZE_STRING_BYTES_m11                   32
#define UNKNOWN_SEARCH_m11                      0
#define TIME_SEARCH_m11                         1
#define SAMPLE_SEARCH_m11			2
#define FRAME_SEARCH_m11			SAMPLE_SEARCH_m11
#define NO_OVERFLOWS_m11			4  // e.g. in find_index_m11(), restrict returned index to valid segment values
#define IPV4_ADDRESS_BYTES_m11			4
#define POSTAL_CODE_BYTES_m11			16
#define LOCALITY_BYTES_m11			64  	//  ascii[63]
#if defined MACOS_m11 || defined LINUX_m11
	#define NULL_DEVICE_m11			"/dev/null"
#endif
#ifdef WINDOWS_m11
	#define PRINTF_BUF_LEN_m11		1024
	#define NULL_DEVICE_m11			"NUL"
#endif
#define SAMPLE_NUMBER_EPS_m11			((sf8) 0.001)
#define FRAME_NUMBER_EPS_m11			((sf8) 0.01)


// Error Handling Constants
#define USE_GLOBAL_BEHAVIOR_m11         ((ui4) 0)
#define RESTORE_BEHAVIOR_m11            ((ui4) 1)
#define EXIT_ON_FAIL_m11                ((ui4) 2)  // exit program
#define RETURN_ON_FAIL_m11              ((ui4) 4)  // return from function: if function is main(), behavior is up to programmer
// if neither EXIT_ON_FAIL_m11 nor RETURN_ON_FAIL_m11 are set, function will continue (i.e. could be CONTINUE_ON_FAIL_m11, if this were defined)
#define SUPPRESS_ERROR_OUTPUT_m11       ((ui4) 8)
#define SUPPRESS_WARNING_OUTPUT_m11     ((ui4) 16)
#define SUPPRESS_MESSAGE_OUTPUT_m11     ((ui4) 32)
#define SUPPRESS_OUTPUT_m11         	(SUPPRESS_ERROR_OUTPUT_m11 | SUPPRESS_WARNING_OUTPUT_m11 | SUPPRESS_MESSAGE_OUTPUT_m11)
#define RETRY_ONCE_m11                  ((ui4) 64)

// Target Value Constants (ui4)
#define NO_INDEX_m11			-1  // assigned to signed values (si4 or si8)
#define FIND_DEFAULT_MODE_m11        	0
#define FIND_START_m11          	(1 << 0)
#define FIND_END_m11            	(1 << 1)
#define FIND_CENTER_m11			(1 << 2)
#define FIND_PREVIOUS_m11        	(1 << 3)
#define FIND_CURRENT_m11        	(1 << 4)
#define FIND_NEXT_m11           	(1 << 5)
#define FIND_CLOSEST_m11        	(1 << 6)
#define FIND_LAST_BEFORE_m11		(1 << 7)
#define FIND_FIRST_ON_OR_AFTER_m11	(1 << 8)
#define FIND_LAST_ON_OR_BEFORE_m11	(1 << 9)
#define FIND_FIRST_AFTER_m11		(1 << 10)
#define FIND_ABSOLUTE_m11       	(1 << 30)  // session relative sample numbering
#define FIND_RELATIVE_m11       	(1 << 31)  // segment relative sample numbering

// Text Color Constant Strings
#ifdef MATLAB_m11  // Matlab doesn't do text coloring this way (can be done with CPRINTF())
	#define TC_BLACK_m11            ""
	#define TC_RED_m11              ""
	#define TC_GREEN_m11            ""
	#define TC_YELLOW_m11           ""
	#define TC_BLUE_m11             ""
	#define TC_MAGENTA_m11          ""
	#define TC_CYAN_m11             ""
	#define TC_LIGHT_GRAY_m11	""
	#define TC_DARK_GRAY_m11	""
	#define TC_LIGHT_RED_m11	""
	#define TC_LIGHT_GREEN_m11	""
	#define TC_LIGHT_YELLOW_m11	""
	#define TC_LIGHT_BLUE_m11	""
	#define TC_LIGHT_MAGENTA_m11	""
	#define TC_LIGHT_CYAN_m11	""
	#define TC_WHITE_m11		""
	#define TC_BRIGHT_BLACK_m11	""
	#define TC_BRIGHT_RED_m11	""
	#define TC_BRIGHT_GREEN_m11	""
	#define TC_BRIGHT_YELLOW_m11	""
	#define TC_BRIGHT_BLUE_m11	""
	#define TC_BRIGHT_MAGENTA_m11	""
	#define TC_BRIGHT_CYAN_m11	""
	#define TC_BRIGHT_WHITE_m11	""
	#define TC_RESET_m11		""
	// non-color constants
	#define TC_BOLD_m11		""
	#define TC_BOLD_RESET_m11	""
	#define TC_UNDERLINE_m11	""
	#define TC_UNDERLINE_RESET_m11	""
#else
	#define TC_BLACK_m11		"\033[30m"
	#define TC_RED_m11		"\033[31m"
	#define TC_GREEN_m11		"\033[32m"
	#define TC_YELLOW_m11		"\033[33m"
	#define TC_BLUE_m11		"\033[34m"
	#define TC_MAGENTA_m11		"\033[35m"
	#define TC_CYAN_m11		"\033[36m"
	#define TC_LIGHT_GRAY_m11	"\033[37m"
	#define TC_DARK_GRAY_m11	"\033[90m"
	#define TC_LIGHT_RED_m11	"\033[91m"
	#define TC_LIGHT_GREEN_m11	"\033[92m"
	#define TC_LIGHT_YELLOW_m11	"\033[93m"
	#define TC_LIGHT_BLUE_m11	"\033[94m"
	#define TC_LIGHT_MAGENTA_m11	"\033[95m"
	#define TC_LIGHT_CYAN_m11	"\033[96m"
	#define TC_WHITE_m11		"\033[97m"
	#define TC_BRIGHT_BLACK_m11	"\033[30;1m"
	#define TC_BRIGHT_RED_m11	"\033[31;1m"
	#define TC_BRIGHT_GREEN_m11	"\033[32;1m"
	#define TC_BRIGHT_YELLOW_m11	"\033[33;1m"
	#define TC_BRIGHT_BLUE_m11	"\033[34;1m"
	#define TC_BRIGHT_MAGENTA_m11	"\033[35;1m"
	#define TC_BRIGHT_CYAN_m11	"\033[36;1m"
	#define TC_BRIGHT_WHITE_m11	"\033[37;1m"
	#define TC_RESET_m11		"\033[0m"
	// non-color constants
	#define TC_BOLD_m11		"\033[1m"
	#define TC_BOLD_RESET_m11	"\033[21m"
	#define TC_UNDERLINE_m11	"\033[4m"
	#define TC_UNDERLINE_RESET_m11	"\033[24m"
#endif

// Time Related Constants
#define TIMEZONE_ACRONYM_BYTES_m11                      8       // ascii[7]
#define TIMEZONE_STRING_BYTES_m11                       64      // ascii[63]
#define MAXIMUM_STANDARD_UTC_OFFSET_m11                 ((si4) 86400)
#define MINIMUM_STANDARD_UTC_OFFSET_m11                 ((si4) -86400)
#define STANDARD_UTC_OFFSET_NO_ENTRY_m11                ((si4) 0x7FFFFFFF)
#define MAXIMUM_DST_OFFSET_m11                          7200
#define MINIMUM_DST_OFFSET_m11                          0
#define DST_OFFSET_NO_ENTRY_m11                         -1
#define TIME_STRING_BYTES_m11                           128
#define NUMBER_OF_SAMPLES_NO_ENTRY_m11			-1
#define NUMBER_OF_FRAMES_NO_ENTRY_m11			NUMBER_OF_SAMPLES_NO_ENTRY_m11
#define EMPTY_SLICE_m11					-1
#define SAMPLE_NUMBER_NO_ENTRY_m11                      ((si8) 0x8000000000000000)
#define FRAME_NUMBER_NO_ENTRY_m11                       SAMPLE_NUMBER_NO_ENTRY_m11
#define BEGINNING_OF_SAMPLE_NUMBERS_m11                 ((si8) 0x000000000)
#define END_OF_SAMPLE_NUMBERS_m11                       ((si8) 0x7FFFFFFFFFFFFFFF)
#define UUTC_NO_ENTRY_m11                               ((si8) 0x8000000000000000)
#define UUTC_EARLIEST_TIME_m11                          ((si8) 0x000000000)  // 00:00:00.000000 Thursday, 1 Jan 1970, UTC
#define UUTC_LATEST_TIME_m11                            ((si8) 0x7FFFFFFFFFFFFFFF)  // 04:00:54.775808 Sunday, 10 Jan 29424, UTC
#define BEGINNING_OF_TIME_m11                           UUTC_EARLIEST_TIME_m11
#define END_OF_TIME_m11                                 UUTC_LATEST_TIME_m11
#define CURRENT_TIME_m11				((si8) 0xFFFFFFFFFFFFFFFF)  // used with time_string_m11() & generate_recording_time_offset_m11()
#define TWENTY_FOURS_HOURS_m11				((si8) 86400000000)
#define Y2K_m11                                         ((si8) 0x00035D013B37E000)  // 00:00:00.000000 Saturday, 1 Jan 2000, UTC  (946684800000000 decimal)
#define WIN_TICKS_PER_USEC_m11				((si8) 10)
#define WIN_USECS_TO_EPOCH_m11				((si8) 11644473600000000)

// Time Change Code Constants
#define DTCC_VALUE_NOT_OBSERVED_m11                     0
#define DTCC_VALUE_NO_ENTRY_m11                         -1
#define DTCC_VALUE_DEFAULT_m11                          DTCC_VALUE_NO_ENTRY_m11
#define DTCC_DST_END_CODE                               -1
#define DTCC_DST_START_CODE                             1
#define DTCC_DST_NOT_OBSERVED_CODE                      0
#define DTCC_DAY_OF_WEEK_NO_ENTRY                       0
#define DTCC_FIRST_RELATIVE_WEEKDAY_OF_MONTH            1
#define DTCC_LAST_RELATIVE_WEEKDAY_OF_MONTH             6
#define DTCC_RELATIVE_WEEKDAY_OF_MONTH_NO_ENTRY         0
#define DTCC_DAY_OF_MONTH_NO_ENTRY                      0
#define DTCC_MONTH_NO_ENTRY                             -1
#define DTCC_HOURS_OF_DAY_NO_ENTRY                      -128
#define DTCC_LOCAL_REFERENCE_TIME                       0
#define DTCC_UTC_REFERENCE_TIME                         1
#define DTCC_REFERENCE_TIME_NO_ENTRY                    -1
#define DTCC_SHIFT_MINUTES_TIME_NO_ENTRY                -128
#define DTCC_START_DATE_NO_ENTRY			-1  // NO_ENTRY indicates it is the only historical rule for this timezone in the table

// Global Defaults
#define GLOBALS_VERBOSE_DEFAULT_m11                             FALSE_m11
#define GLOBALS_BEHAVIOR_ON_FAIL_DEFAULT_m11		        EXIT_ON_FAIL_m11
#define GLOBALS_CRC_MODE_DEFAULT_m11			        CRC_CALCULATE_ON_OUTPUT_m11
#define GLOBALS_BEHAVIOR_STACK_SIZE_INCREMENT_m11		256
#define GLOBALS_REFERENCE_CHANNEL_INDEX_NO_ENTRY_m11		-1
#define GLOBALS_MMAP_BLOCK_BYTES_NO_ENTRY_m11			-1
#define GLOBALS_MMAP_BLOCK_BYTES_DEFAULT_m11			4096  // 4 KiB
#define GLOBALS_AT_LIST_SIZE_INCREMENT_m11			8096

// Global Time Defaults
#define GLOBALS_OBSERVE_DST_DEFAULT_m11				FALSE_m11
#define GLOBALS_RTO_KNOWN_DEFAULT_m11				UNKNOWN_m11
#define GLOBALS_SESSION_START_TIME_DEFAULT_m11			UUTC_NO_ENTRY_m11
#define GLOBALS_SESSION_END_TIME_DEFAULT_m11			UUTC_NO_ENTRY_m11
#define GLOBALS_RECORDING_TIME_OFFSET_DEFAULT_m11               0
#define GLOBALS_RECORDING_TIME_OFFSET_NO_ENTRY_m11              0
#define GLOBALS_STANDARD_UTC_OFFSET_DEFAULT_m11                 0
#define GLOBALS_STANDARD_UTC_OFFSET_NO_ENTRY_m11		STANDARD_UTC_OFFSET_NO_ENTRY_m11
#define GLOBALS_STANDARD_TIMEZONE_ACRONYM_DEFAULT_m11	        "oUTC"
#define GLOBALS_STANDARD_TIMEZONE_STRING_DEFAULT_m11            "offset Coordinated Universal Time"
#define GLOBALS_DAYLIGHT_TIMEZONE_ACRONYM_DEFAULT_m11           ""
#define GLOBALS_DAYLIGHT_TIMEZONE_STRING_DEFAULT_m11            ""

// File Type Constants
#define NO_TYPE_CODE_m11                                        (ui4) 0x0
#define UNKNOWN_TYPE_CODE_m11                                   NO_TYPE_CODE_m11
#define NO_FILE_TYPE_STRING_m11				        ""			// ascii[4]
#define NO_FILE_TYPE_CODE_m11				        NO_TYPE_CODE_m11	// ui4 (big & little endian)
#define ALL_TYPES_STRING_m11		        		"allt"			// ascii[4]
#define ALL_TYPES_CODE_m11                                      (ui4) 0x746C6C61	// ui4 (little endian)
// #define ALL_TYPES_CODE_m11					(ui4) 0x616C6C74	// ui4 (big endian)
#define SESSION_DIRECTORY_TYPE_STRING_m11		        "medd"			// ascii[4]
#define SESSION_DIRECTORY_TYPE_CODE_m11                         (ui4) 0x6464656D	// ui4 (little endian)
// #define SESSION_DIRECTORY_TYPE_CODE_m11                      (ui4) 0x6D656464        // ui4 (big endian)
#define TIME_SERIES_CHANNEL_DIRECTORY_TYPE_STRING_m11           "ticd"                  // ascii[4]
#define TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m11             (ui4) 0x64636974        // ui4 (little endian)
// #define TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m11          (ui4) 0x74696364        // ui4 (big endian)
#define TIME_SERIES_SEGMENT_DIRECTORY_TYPE_STRING_m11           "tisd"                  // ascii[4]
#define TIME_SERIES_SEGMENT_DIRECTORY_TYPE_CODE_m11             (ui4) 0x64736974        // ui4 (little endian)
// #define TIME_SERIES_SEGMENT_DIRECTORY_TYPE_CODE_m11          (ui4) 0x74697364        // ui4 (big endian)
#define TIME_SERIES_METADATA_FILE_TYPE_STRING_m11               "tmet"                  // ascii[4]
#define TIME_SERIES_METADATA_FILE_TYPE_CODE_m11                 (ui4) 0x74656D74        // ui4 (little endian)
// #define TIME_SERIES_METADATA_FILE_TYPE_CODE_m11              (ui4) 0x746D6574        // ui4 (big endian)
#define TIME_SERIES_DATA_FILE_TYPE_STRING_m11                   "tdat"                  // ascii[4]
#define TIME_SERIES_DATA_FILE_TYPE_CODE_m11                     (ui4) 0x74616474        // ui4 (little endian)
// #define TIME_SERIES_DATA_FILE_TYPE_CODE_m11                  (ui4) 0x74646174        // ui4 (big endian)
#define TIME_SERIES_INDICES_FILE_TYPE_STRING_m11                "tidx"                  // ascii[4]
#define TIME_SERIES_INDICES_FILE_TYPE_CODE_m11                  (ui4) 0x78646974        // ui4 (little endian)
// #define TIME_SERIES_INDICES_FILE_TYPE_CODE_m11               (ui4) 0x74696478        // ui4 (big endian)
#define VIDEO_SEGMENT_DIRECTORY_TYPE_STRING_m11                 "visd"                  // ascii[4]
#define VIDEO_SEGMENT_DIRECTORY_TYPE_CODE_m11                   (ui4) 0x64736976        // ui4 (little endian)
// #define VIDEO_SEGMENT_DIRECTORY_TYPE_CODE_m11                (ui4) 0x76697364        // ui4 (big endian)
#define VIDEO_CHANNEL_DIRECTORY_TYPE_STRING_m11                 "vicd"                  // ascii[4]
#define VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m11                   (ui4) 0x64636976        // ui4 (little endian)
// #define VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m11                (ui4) 0x76696364        // ui4 (big endian)
#define VIDEO_METADATA_FILE_TYPE_STRING_m11                     "vmet"                  // ascii[4]
#define VIDEO_METADATA_FILE_TYPE_CODE_m11                       (ui4) 0x74656D76        // ui4 (little endian)
// #define VIDEO_METADATA_FILE_TYPE_CODE_m11                    (ui4) 0x766D6574        // ui4 (big endian)
#define VIDEO_INDICES_FILE_TYPE_STRING_m11                      "vidx"                  // ascii[4]
#define VIDEO_INDICES_FILE_TYPE_CODE_m11                        (ui4) 0x78646976        // ui4 (little endian)
// #define VIDEO_INDICES_FILE_TYPE_CODE_m11                     (ui4) 0x76696478        // ui4 (big endian)
#define RECORD_DIRECTORY_TYPE_STRING_m11                        "recd"                        // ascii[4]
#define RECORD_DIRECTORY_TYPE_CODE_m11                          (ui4) 0x64636572        // ui4 (little endian)
// #define RECORD_DIRECTORY_TYPE_CODE_m11                       (ui4) 0x72656364        // ui4 (big endian)
#define RECORD_DATA_FILE_TYPE_STRING_m11                        "rdat"			// ascii[4]
#define RECORD_DATA_FILE_TYPE_CODE_m11                          (ui4) 0x74616472	// ui4 (little endian)
// #define RECORD_DATA_FILE_TYPE_CODE_m11                       (ui4) 0x72646174	// ui4 (big endian)
#define RECORD_INDICES_FILE_TYPE_STRING_m11                     "ridx"			// ascii[4]
#define RECORD_INDICES_FILE_TYPE_CODE_m11                       (ui4) 0x78646972	// ui4 (little endian)
// #define RECORD_INDICES_FILE_TYPE_CODE_m11                    (ui4) 0x72696478	// ui4 (big endian)

// Channel Types
#define UNKNOWN_CHANNEL_TYPE_m11	NO_FILE_TYPE_CODE_m11
#define TIME_SERIES_CHANNEL_TYPE_m11	TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m11
#define VIDEO_CHANNEL_TYPE_m11		VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m11

// Reference Channel Types (used in change_reference_channel_m11())
#define DEFAULT_CHANNEL_m11			0
#define DEFAULT_TIME_SERIES_CHANNEL_m11		1
#define DEFAULT_VIDEO_CHANNEL_m11		2
#define HIGHEST_RATE_TIME_SERIES_CHANNEL_m11	3
#define LOWEST_RATE_TIME_SERIES_CHANNEL_m11	4
#define HIGHEST_RATE_VIDEO_CHANNEL_m11		5
#define LOWEST_RATE_VIDEO_CHANNEL_m11		6

// Generate File List flags
	// Path Parts
#define GFL_PATH_m11             		((ui4) 1)
#define GFL_NAME_m11             		((ui4) 2)
#define GFL_EXTENSION_m11        		((ui4) 4)
#define GFL_FULL_PATH_m11        		(GFL_PATH_m11 | GFL_NAME_m11 | GFL_EXTENSION_m11)
#define GFL_PATH_PARTS_MASK_m11        		GFL_FULL_PATH_m11
	// Other Options
#define GFL_FREE_INPUT_FILE_LIST_m11		((ui4) 16)
#define GFL_INCLUDE_INVISIBLE_FILES_m11		((ui4) 32)

// Spaces Constants
#define NO_SPACES_m11                           0
#define ESCAPED_SPACES_m11                      1
#define UNESCAPED_SPACES_m11                    2
#define ALL_SPACES_m11                          (ESCAPED_SPACES_m11 | UNESCAPED_SPACES_m11)

// File Processing Constants
#define FPS_FILE_LENGTH_UNKNOWN_m11		-1
#define FPS_UNIVERSAL_HEADER_ONLY_m11		-1
#define FPS_FULL_FILE_m11			-2
#define FPS_APPEND_m11				-3
#define FPS_CLOSE_m11				-4
#define FPS_NO_LOCK_TYPE_m11			~(F_RDLCK | F_WRLCK | F_UNLCK)  // from <fcntl.h>
#define FPS_NO_LOCK_MODE_m11			0
#define FPS_READ_LOCK_ON_READ_OPEN_m11		1
#define FPS_WRITE_LOCK_ON_READ_OPEN_m11		2
#define FPS_WRITE_LOCK_ON_WRITE_OPEN_m11        4
#define FPS_WRITE_LOCK_ON_READ_WRITE_OPEN_m11   8
#define FPS_READ_LOCK_ON_READ_m11               16
#define FPS_WRITE_LOCK_ON_WRITE_m11             32
#define FPS_NO_OPEN_MODE_m11		        0
#define FPS_R_OPEN_MODE_m11			1
#define FPS_R_PLUS_OPEN_MODE_m11                2
#define FPS_W_OPEN_MODE_m11                     4
#define FPS_W_PLUS_OPEN_MODE_m11                8
#define FPS_A_OPEN_MODE_m11                     16
#define FPS_A_PLUS_OPEN_MODE_m11                32
#define FPS_GENERIC_READ_OPEN_MODE_m11		(FPS_R_OPEN_MODE_m11 | FPS_R_PLUS_OPEN_MODE_m11 | FPS_W_PLUS_OPEN_MODE_m11 | FPS_A_PLUS_OPEN_MODE_m11)
#define FPS_GENERIC_WRITE_OPEN_MODE_m11		(FPS_R_PLUS_OPEN_MODE_m11 | FPS_W_OPEN_MODE_m11 | FPS_W_PLUS_OPEN_MODE_m11 | FPS_A_OPEN_MODE_m11 | FPS_A_PLUS_OPEN_MODE_m11)
#define FPS_PROTOTYPE_FILE_TYPE_CODE_m11        TIME_SERIES_METADATA_FILE_TYPE_CODE_m11  // any metadata type would do
#define FPS_FD_CLOSED_m11                     	-1
#define FPS_FD_NO_ENTRY_m11                     -2
#define FPS_FD_EPHEMERAL_m11                    -3

// File Processing Directives Defaults
#define FPS_DIRECTIVES_CLOSE_FILE_DEFAULT_m11		        TRUE_m11
#define FPS_DIRECTIVES_MEMORY_MAP_DEFAULT_m11		        FALSE_m11
#define FPS_DIRECTIVES_FLUSH_AFTER_WRITE_DEFAULT_m11		TRUE_m11
#define FPS_DIRECTIVES_FREE_CMP_PROCESSING_STRUCT_DEFAULT_m11	TRUE_m11
#define FPS_DIRECTIVES_FREE_PASSWORD_DATA_DEFAULT_m11		FALSE_m11
#define FPS_DIRECTIVES_UPDATE_UNIVERSAL_HEADER_DEFAULT_m11	FALSE_m11
#define FPS_DIRECTIVES_LEAVE_DECRYPTED_DEFAULT_m11		FALSE_m11
#define FPS_DIRECTIVES_LOCK_MODE_DEFAULT_m11		        FPS_NO_LOCK_MODE_m11  // Unix file locking may cause problems with networked file systems
// #define FPS_DIRECTIVES_LOCK_MODE_DEFAULT_m11                     (FPS_READ_LOCK_ON_READ_OPEN_m11 | FPS_WRITE_LOCK_ON_WRITE_OPEN_m11 | FPS_WRITE_LOCK_ON_READ_WRITE_OPEN_m11)
#define FPS_DIRECTIVES_OPEN_MODE_DEFAULT_m11		        FPS_NO_OPEN_MODE_m11

// Universal Header: File Format Constants
#define UNIVERSAL_HEADER_OFFSET_m11					0
#define UNIVERSAL_HEADER_BYTES_m11					1024    // 1 kB
#define UNIVERSAL_HEADER_HEADER_CRC_OFFSET_m11				0       // ui4
#define UNIVERSAL_HEADER_BODY_CRC_OFFSET_m11				4       // ui4
#define UNIVERSAL_HEADER_HEADER_CRC_START_OFFSET_m11			UNIVERSAL_HEADER_BODY_CRC_OFFSET_m11
#define UNIVERSAL_HEADER_BODY_CRC_START_OFFSET_m11			UNIVERSAL_HEADER_BYTES_m11
#define UNIVERSAL_HEADER_FILE_END_TIME_OFFSET_m11			8	// si8
#define UNIVERSAL_HEADER_NUMBER_OF_ENTRIES_OFFSET_m11			16      // si8
#define UNIVERSAL_HEADER_NUMBER_OF_ENTRIES_NO_ENTRY_m11			-1
#define UNIVERSAL_HEADER_MAXIMUM_ENTRY_SIZE_OFFSET_m11			24      // ui4
#define UNIVERSAL_HEADER_MAXIMUM_ENTRY_SIZE_NO_ENTRY_m11		0
#define UNIVERSAL_HEADER_SEGMENT_NUMBER_OFFSET_m11                      28      // si4
#define UNIVERSAL_HEADER_SEGMENT_NUMBER_NO_ENTRY_m11                    SEGMENT_NUMBER_NO_ENTRY_m11
#define UNIVERSAL_HEADER_SEGMENT_LEVEL_CODE_m11				-1
#define UNIVERSAL_HEADER_CHANNEL_LEVEL_CODE_m11				-2
#define UNIVERSAL_HEADER_SESSION_LEVEL_CODE_m11				-3
#define UNIVERSAL_HEADER_TYPE_STRING_OFFSET_m11				32       // ascii[4]
#define UNIVERSAL_HEADER_TYPE_STRING_TERMINAL_ZERO_OFFSET_m11		(UNIVERSAL_HEADER_TYPE_STRING_OFFSET_m11 + 4)  // si1
#define UNIVERSAL_HEADER_TYPE_CODE_OFFSET_m11				UNIVERSAL_HEADER_TYPE_STRING_OFFSET_m11  // ui4
#define UNIVERSAL_HEADER_TYPE_NO_ENTRY_m11				0       // zero as ui4 or zero-length string as ascii[4]
#define UNIVERSAL_HEADER_MED_VERSION_MAJOR_OFFSET_m11			37     // ui1
#define UNIVERSAL_HEADER_MED_VERSION_MAJOR_NO_ENTRY_m11			MED_VERSION_NO_ENTRY_m11
#define UNIVERSAL_HEADER_MED_VERSION_MINOR_OFFSET_m11			38      // ui1
#define UNIVERSAL_HEADER_MED_VERSION_MINOR_NO_ENTRY_m11			MED_VERSION_NO_ENTRY_m11
#define UNIVERSAL_HEADER_BYTE_ORDER_CODE_OFFSET_m11			39      // ui1
#define UNIVERSAL_HEADER_BYTE_ORDER_CODE_NO_ENTRY_m11			0xFF
#define UNIVERSAL_HEADER_SESSION_START_TIME_OFFSET_m11			40      // si8
#define UNIVERSAL_HEADER_FILE_START_TIME_OFFSET_m11			48      // si8
#define UNIVERSAL_HEADER_SESSION_NAME_OFFSET_m11                        56      // utf8[63]
#define UNIVERSAL_HEADER_CHANNEL_NAME_OFFSET_m11                        312     // utf8[63]
#define UNIVERSAL_HEADER_ANONYMIZED_SUBJECT_ID_OFFSET_m11             	568     // utf8[63]
#define UNIVERSAL_HEADER_ANONYMIZED_SUBJECT_ID_BYTES_m11              	BASE_FILE_NAME_BYTES_m11
#define UNIVERSAL_HEADER_SESSION_UID_OFFSET_m11                         824     // ui8
#define UNIVERSAL_HEADER_CHANNEL_UID_OFFSET_m11                         832     // ui8
#define UNIVERSAL_HEADER_SEGMENT_UID_OFFSET_m11                         840     // ui8
#define UNIVERSAL_HEADER_FILE_UID_OFFSET_m11				848     // ui8
#define UNIVERSAL_HEADER_PROVENANCE_UID_OFFSET_m11			856     // ui8
#define UNIVERSAL_HEADER_LEVEL_1_PASSWORD_VALIDATION_FIELD_OFFSET_m11	864     // ui1
#define UNIVERSAL_HEADER_LEVEL_2_PASSWORD_VALIDATION_FIELD_OFFSET_m11	880     // ui1
#define UNIVERSAL_HEADER_LEVEL_3_PASSWORD_VALIDATION_FIELD_OFFSET_m11   896     // ui1
#define UNIVERSAL_HEADER_PROTECTED_REGION_OFFSET_m11			912
#define UNIVERSAL_HEADER_PROTECTED_REGION_BYTES_m11			56
#define UNIVERSAL_HEADER_DISCRETIONARY_REGION_OFFSET_m11		968
#define UNIVERSAL_HEADER_DISCRETIONARY_REGION_BYTES_m11			56

// Metadata: File Format Constants
#define METADATA_BYTES_m11			15360   // 15 kB
#define FPS_PROTOTYPE_BYTES_m11			METADATA_BYTES_m11
#define METADATA_FILE_BYTES_m11			(METADATA_BYTES_m11 + UNIVERSAL_HEADER_BYTES_m11)	// 16 kB
#define METADATA_SECTION_1_OFFSET_m11		1024
#define METADATA_SECTION_1_BYTES_m11		1024	// 1 kB
#define METADATA_SECTION_2_OFFSET_m11		2048
#define METADATA_SECTION_2_BYTES_m11		10240   // 10 kB
#define METADATA_SECTION_3_OFFSET_m11		12288
#define METADATA_SECTION_3_BYTES_m11		4096    // 4 kB

// Metadata: File Format Constants - Section 1 Fields
#define METADATA_LEVEL_1_PASSWORD_HINT_OFFSET_m11		1024	// utf8[63]
#define METADATA_LEVEL_2_PASSWORD_HINT_OFFSET_m11		1280    // utf8[63]
#define METADATA_SECTION_2_ENCRYPTION_LEVEL_OFFSET_m11		1536    // si1
#define METADATA_SECTION_2_ENCRYPTION_LEVEL_DEFAULT_m11		LEVEL_1_ENCRYPTION_m11
#define METADATA_SECTION_3_ENCRYPTION_LEVEL_OFFSET_m11		1537    // si1
#define METADATA_SECTION_3_ENCRYPTION_LEVEL_DEFAULT_m11		LEVEL_2_ENCRYPTION_m11
#define METADATA_TIME_SERIES_DATA_ENCRYPTION_LEVEL_OFFSET_m11	1538
#define METADATA_TIME_SERIES_DATA_ENCRYPTION_LEVEL_DEFAULT_m11	NO_ENCRYPTION_m11
#define METADATA_SECTION_1_PROTECTED_REGION_OFFSET_m11		1539
#define METADATA_SECTION_1_PROTECTED_REGION_BYTES_m11		253
#define METADATA_SECTION_1_DISCRETIONARY_REGION_OFFSET_m11	1792
#define METADATA_SECTION_1_DISCRETIONARY_REGION_BYTES_m11	256

// Metadata: File Format Constants - Section 2 Channel Type Independent Fields
#define METADATA_SESSION_DESCRIPTION_OFFSET_m11                 2048    // utf8[511]
#define METADATA_SESSION_DESCRIPTION_BYTES_m11                  2048
#define METADATA_CHANNEL_DESCRIPTION_OFFSET_m11                 4096    // utf8[255]
#define METADATA_CHANNEL_DESCRIPTION_BYTES_m11                  1024
#define METADATA_SEGMENT_DESCRIPTION_OFFSET_m11                 5120    // utf8[255]
#define METADATA_SEGMENT_DESCRIPTION_BYTES_m11                  1024
#define METADATA_EQUIPMENT_DESCRIPTION_OFFSET_m11               6144    // utf8[510]
#define METADATA_EQUIPMENT_DESCRIPTION_BYTES_m11                2044
#define METADATA_ACQUISITION_CHANNEL_NUMBER_OFFSET_m11          8188    // si4
#define METADATA_ACQUISITION_CHANNEL_NUMBER_NO_ENTRY_m11        CHANNEL_NUMBER_NO_ENTRY_m11

// Metadata: File Format Constants - Time Series Section 2 Fields
#define TIME_SERIES_METADATA_REFERENCE_DESCRIPTION_OFFSET_m11                   8192            // utf8[255]
#define TIME_SERIES_METADATA_REFERENCE_DESCRIPTION_BYTES_m11                    1024
#define TIME_SERIES_METADATA_SAMPLING_FREQUENCY_OFFSET_m11                      9216            // sf8
#define TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m11                             FREQUENCY_NO_ENTRY_m11
#define TIME_SERIES_METADATA_FREQUENCY_VARIABLE_m11				FREQUENCY_VARIABLE_m11
#define TIME_SERIES_METADATA_LOW_FREQUENCY_FILTER_SETTING_OFFSET_m11            9224            // sf8
#define TIME_SERIES_METADATA_HIGH_FREQUENCY_FILTER_SETTING_OFFSET_m11           9232            // sf8
#define TIME_SERIES_METADATA_NOTCH_FILTER_FREQUENCY_SETTING_OFFSET_m11          9240            // sf8
#define TIME_SERIES_METADATA_AC_LINE_FREQUENCY_OFFSET_m11                       9248            // sf8
#define TIME_SERIES_METADATA_AMPLITUDE_UNITS_CONVERSION_FACTOR_OFFSET_m11       9256            // sf8
#define TIME_SERIES_METADATA_AMPLITUDE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m11     0.0
#define TIME_SERIES_METADATA_AMPLITUDE_UNITS_DESCRIPTION_OFFSET_m11             9264            // utf8[31]
#define TIME_SERIES_METADATA_AMPLITUDE_UNITS_DESCRIPTION_BYTES_m11              128
#define TIME_SERIES_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_OFFSET_m11       9392            // sf8
#define TIME_SERIES_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m11     0.0
#define TIME_SERIES_METADATA_TIME_BASE_UNITS_DESCRIPTION_OFFSET_m11             9400            // utf8[31]
#define TIME_SERIES_METADATA_TIME_BASE_UNITS_DESCRIPTION_BYTES_m11              128
#define TIME_SERIES_METADATA_ABSOLUTE_START_SAMPLE_NUMBER_OFFSET_m11            9528
#define TIME_SERIES_METADATA_ABSOLUTE_START_SAMPLE_NUMBER_NO_ENTRY_m11          SAMPLE_NUMBER_NO_ENTRY_m11
#define TIME_SERIES_METADATA_NUMBER_OF_SAMPLES_OFFSET_m11                       9536            // si8
#define TIME_SERIES_METADATA_NUMBER_OF_SAMPLES_NO_ENTRY_m11                     NUMBER_OF_SAMPLES_NO_ENTRY_m11
#define TIME_SERIES_METADATA_NUMBER_OF_BLOCKS_OFFSET_m11                        9544            // si8
#define TIME_SERIES_METADATA_NUMBER_OF_BLOCKS_NO_ENTRY_m11                      -1
#define TIME_SERIES_METADATA_MAXIMUM_BLOCK_BYTES_OFFSET_m11                     9552            // si8
#define TIME_SERIES_METADATA_MAXIMUM_BLOCK_BYTES_NO_ENTRY_m11                   -1
#define TIME_SERIES_METADATA_MAXIMUM_BLOCK_SAMPLES_OFFSET_m11                   9560            // ui4
#define TIME_SERIES_METADATA_MAXIMUM_BLOCK_SAMPLES_NO_ENTRY_m11                 0xFFFFFFFF
#define TIME_SERIES_METADATA_MAXIMUM_BLOCK_KEYSAMPLE_BYTES_OFFSET_m11          	9564            // ui4
#define TIME_SERIES_METADATA_MAXIMUM_BLOCK_KEYSAMPLE_BYTES_NO_ENTRY_m11        	0xFFFFFFFF
#define TIME_SERIES_METADATA_MAXIMUM_BLOCK_DURATION_OFFSET_m11                  9568            // sf8
#define TIME_SERIES_METADATA_MAXIMUM_BLOCK_DURATION_NO_ENTRY_m11                -1.0
#define TIME_SERIES_METADATA_NUMBER_OF_DISCONTINUITIES_OFFSET_m11               9576            // si8
#define TIME_SERIES_METADATA_NUMBER_OF_DISCONTINUITIES_NO_ENTRY_m11             -1
#define TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_BLOCKS_OFFSET_m11               9584            // si8
#define TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_BLOCKS_NO_ENTRY_m11             -1
#define TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_BLOCK_BYTES_OFFSET_m11          9592            // si8
#define TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_BLOCK_BYTES_NO_ENTRY_m11        -1
#define TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_SAMPLES_OFFSET_m11              9600            // si8
#define TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_SAMPLES_NO_ENTRY_m11            -1
#define TIME_SERIES_METADATA_SECTION_2_PROTECTED_REGION_OFFSET_m11              9608
#define TIME_SERIES_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m11               1344
#define TIME_SERIES_METADATA_SECTION_2_DISCRETIONARY_REGION_OFFSET_m11          10952
#define TIME_SERIES_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m11           1336

// Metadata: File Format Constants - Video Section 2 Fields
#define VIDEO_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_OFFSET_m11		8192		// sf8
#define VIDEO_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m11		0.0
#define VIDEO_METADATA_TIME_BASE_UNITS_DESCRIPTION_OFFSET_m11			8200		// utf8[31]
#define VIDEO_METADATA_TIME_BASE_UNITS_DESCRIPTION_BYTES_m11			128
#define VIDEO_METADATA_ABSOLUTE_START_FRAME_NUMBER_OFFSET_m11			8328
#define VIDEO_METADATA_ABSOLUTE_START_FRAME_NUMBER_NO_ENTRY_m11			FRAME_NUMBER_NO_ENTRY_m11
#define VIDEO_METADATA_NUMBER_OF_FRAMES_OFFSET_m11				8336		// si8
#define VIDEO_METADATA_NUMBER_OF_FRAMES_NO_ENTRY_m11				NUMBER_OF_FRAMES_NO_ENTRY_m11
#define VIDEO_METADATA_FRAME_RATE_OFFSET_m11					8344		// sf8
#define VIDEO_METADATA_FRAME_RATE_NO_ENTRY_m11					FRAME_RATE_NO_ENTRY_m11
#define VIDEO_METADATA_FRAME_RATE_VARIABLE_m11					FRAME_RATE_VARIABLE_m11
#define VIDEO_METADATA_NUMBER_OF_CLIPS_OFFSET_m11                        	8352            // si8
#define VIDEO_METADATA_NUMBER_OF_CLIPS_NO_ENTRY_m11                      	-1
#define VIDEO_METADATA_MAXIMUM_CLIP_BYTES_OFFSET_m11				8360            // si8
#define VIDEO_METADATA_MAXIMUM_CLIP_BYTES_NO_ENTRY_m11				-1
#define VIDEO_METADATA_MAXIMUM_CLIP_FRAMES_OFFSET_m11				8368            // ui4
#define VIDEO_METADATA_MAXIMUM_CLIP_FRAMES_NO_ENTRY_m11				0xFFFFFFFF
#define VIDEO_METADATA_NUMBER_OF_VIDEO_FILES_OFFSET_m11                 	8372    	// si4
#define VIDEO_METADATA_NUMBER_OF_VIDEO_FILES_NO_ENTRY_m11               	-1
#define VIDEO_METADATA_MAXIMUM_CLIP_DURATION_OFFSET_m11                  	8376            // sf8
#define VIDEO_METADATA_MAXIMUM_CLIP_DURATION_NO_ENTRY_m11                	-1.0
#define VIDEO_METADATA_NUMBER_OF_DISCONTINUITIES_OFFSET_m11               	8384            // si8
#define VIDEO_METADATA_NUMBER_OF_DISCONTINUITIES_NO_ENTRY_m11             	-1
#define VIDEO_METADATA_MAXIMUM_CONTIGUOUS_CLIPS_OFFSET_m11			8392            // si8
#define VIDEO_METADATA_MAXIMUM_CONTIGUOUS_CLIPS_NO_ENTRY_m11			-1
#define VIDEO_METADATA_MAXIMUM_CONTIGUOUS_CLIP_BYTES_OFFSET_m11          	8400            // si8
#define VIDEO_METADATA_MAXIMUM_CONTIGUOUS_CLIP_BYTES_NO_ENTRY_m11		-1
#define VIDEO_METADATA_MAXIMUM_CONTIGUOUS_FRAMES_OFFSET_m11			8408            // si8
#define VIDEO_METADATA_MAXIMUM_CONTIGUOUS_FRAMES_NO_ENTRY_m11			-1
#define VIDEO_METADATA_HORIZONTAL_PIXELS_OFFSET_m11				8416		// ui4
#define VIDEO_METADATA_HORIZONTAL_PIXELS_NO_ENTRY_m11				0
#define VIDEO_METADATA_VERTICAL_PIXELS_OFFSET_m11				8420		// ui4
#define VIDEO_METADATA_VERTICAL_PIXELS_NO_ENTRY_m11				0
#define VIDEO_METADATA_VIDEO_FORMAT_OFFSET_m11                          	8424		// utf8[63]
#define VIDEO_METADATA_VIDEO_FORMAT_BYTES_m11                           	256
#define VIDEO_METADATA_SECTION_2_PROTECTED_REGION_OFFSET_m11			8680
#define VIDEO_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m11			1808
#define VIDEO_METADATA_SECTION_2_DISCRETIONARY_REGION_OFFSET_m11		10488
#define VIDEO_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m11			1800

// Metadata: File Format Constants - Section 3 Fields
#define METADATA_RECORDING_TIME_OFFSET_OFFSET_m11               12288           // si8
#define METADATA_RECORDING_TIME_OFFSET_NO_ENTRY_m11             GLOBALS_RECORDING_TIME_OFFSET_NO_ENTRY_m11
#define METADATA_DAYLIGHT_TIME_START_CODE_OFFSET_m11            12296           // DAYLIGHT_TIME_CHANGE_CODE_m11 (si1[8])
#define METADATA_DAYLIGHT_TIME_START_CODE_NO_ENTRY_m11          DTCC_VALUE_NO_ENTRY_m11
#define METADATA_DAYLIGHT_TIME_END_CODE_OFFSET_m11              12304           // DAYLIGHT_TIME_CHANGE_CODE_m11 (si1[8])
#define METADATA_DAYLIGHT_TIME_END_CODE_NO_ENTRY_m11            DTCC_VALUE_NO_ENTRY_m11
#define METADATA_STANDARD_TIMEZONE_ACRONYM_OFFSET_m11           12312           // ascii[7]
#define METADATA_STANDARD_TIMEZONE_ACRONYM_BYTES_m11            TIMEZONE_ACRONYM_BYTES_m11
#define METADATA_STANDARD_TIMEZONE_STRING_OFFSET_m11            12320           // ascii[63]
#define METADATA_STANDARD_TIMEZONE_STRING_BYTES_m11             TIMEZONE_STRING_BYTES_m11
#define METADATA_DAYLIGHT_TIMEZONE_ACRONYM_OFFSET_m11           12384           // ascii[7]
#define METADATA_DAYLIGHT_TIMEZONE_ACRONYM_BYTES_m11            TIMEZONE_ACRONYM_BYTES_m11
#define METADATA_DAYLIGHT_TIMEZONE_STRING_OFFSET_m11            12392           // ascii[63]
#define METADATA_DAYLIGHT_TIMEZONE_STRING_BYTES_m11             TIMEZONE_STRING_BYTES_m11
#define METADATA_SUBJECT_NAME_1_OFFSET_m11                      12456           // utf8[31]
#define METADATA_SUBJECT_NAME_BYTES_m11                         128
#define METADATA_SUBJECT_NAME_2_OFFSET_m11                      12584           // utf8[31]
#define METADATA_SUBJECT_NAME_3_OFFSET_m11                      12712           // utf8[31]
#define METADATA_SUBJECT_ID_OFFSET_m11                          12840           // utf8[31]
#define METADATA_SUBJECT_ID_BYTES_m11                           128
#define METADATA_RECORDING_COUNTRY_OFFSET_m11                   12968           // utf8[63]
#define METADATA_RECORDING_TERRITORY_OFFSET_m11                 13224           // utf8[63]
#define METADATA_RECORDING_LOCALITY_OFFSET_m11                  13480           // utf8[63]
#define METADATA_RECORDING_INSTITUTION_OFFSET_m11               13736           // utf8[63]
#define METADATA_RECORDING_LOCATION_BYTES_m11                   256
#define METADATA_GEOTAG_FORMAT_OFFSET_m11                       13992           // ascii[31]
#define METADATA_GEOTAG_FORMAT_BYTES_m11                        32
#define METADATA_GEOTAG_DATA_OFFSET_m11                         14024           // ascii[1023]
#define METADATA_GEOTAG_DATA_BYTES_m11                          1024
#define METADATA_STANDARD_UTC_OFFSET_OFFSET_m11                 15048           // si4
#define METADATA_STANDARD_UTC_OFFSET_NO_ENTRY_m11               STANDARD_UTC_OFFSET_NO_ENTRY_m11
#define METADATA_SECTION_3_PROTECTED_REGION_OFFSET_m11          15052
#define METADATA_SECTION_3_PROTECTED_REGION_BYTES_m11           668
#define METADATA_SECTION_3_DISCRETIONARY_REGION_OFFSET_m11      15720
#define METADATA_SECTION_3_DISCRETIONARY_REGION_BYTES_m11       664

// Records: Header Format Constants
#define RECORD_HEADER_BYTES_m11			                        24
#define RECORD_HEADER_RECORD_CRC_OFFSET_m11		                0                       // ui4
#define RECORD_HEADER_RECORD_CRC_NO_ENTRY_m11	                        CRC_NO_ENTRY_m11
#define RECORD_HEADER_TOTAL_RECORD_BYTES_OFFSET_m11                     4                       // ui4
#define RECORD_HEADER_TOTAL_RECORD_BYTES_NO_ENTRY_m11			0
#define RECORD_HEADER_RECORD_CRC_START_OFFSET_m11			RECORD_HEADER_TOTAL_RECORD_BYTES_OFFSET_m11
#define RECORD_HEADER_START_TIME_OFFSET_m11                             8                       // si8
#define RECORD_HEADER_START_TIME_NO_ENTRY_m11                           UUTC_NO_ENTRY_m11       // si8
#define RECORD_HEADER_TYPE_STRING_OFFSET_m11                            16	                // ascii[4]
#define RECORD_HEADER_TYPE_STRING_TERMINAL_ZERO_OFFSET_m11              (RECORD_HEADER_TYPE_STRING_OFFSET_m11 + 4)	// si1
#define RECORD_HEADER_TYPE_CODE_OFFSET_m11                              RECORD_HEADER_TYPE_STRING_OFFSET_m11		// ui4
#define RECORD_HEADER_TYPE_CODE_NO_ENTRY_m11		                0	                // ui4
#define RECORD_HEADER_VERSION_MAJOR_OFFSET_m11	                        21	                // ui1
#define RECORD_HEADER_VERSION_MAJOR_NO_ENTRY_m11	                0xFF
#define RECORD_HEADER_VERSION_MINOR_OFFSET_m11	                        22	                // ui1
#define RECORD_HEADER_VERSION_MINOR_NO_ENTRY_m11	                0xFF
#define RECORD_HEADER_ENCRYPTION_LEVEL_OFFSET_m11                       23	                // si1
#define RECORD_HEADER_ENCRYPTION_LEVEL_NO_ENTRY_m11                     ENCRYPTION_LEVEL_NO_ENTRY_m11

// Record Index: Format Constants
#define RECORD_INDEX_BYTES_m11                                          INDEX_BYTES_m11
#define RECORD_INDEX_FILE_OFFSET_OFFSET_m11                             0                       // si8
#define RECORD_INDEX_FILE_OFFSET_NO_ENTRY_m11                           -1
#define RECORD_INDEX_START_TIME_OFFSET_m11                              8                       // si8
#define RECORD_INDEX_START_TIME_NO_ENTRY_m11                            UUTC_NO_ENTRY_m11
#define RECORD_INDEX_TYPE_STRING_OFFSET_m11                             16                      // ascii[4]
#define RECORD_INDEX_TYPE_STRING_TERMINAL_ZERO_OFFSET_m11               (RECORD_INDEX_TYPE_STRING_OFFSET_m11 + 4)	// si1
#define RECORD_INDEX_TYPE_CODE_OFFSET_m11                               RECORD_INDEX_TYPE_STRING_OFFSET_m11		// as ui4
#define RECORD_INDEX_TYPE_CODE_NO_ENTRY_m11                             0                       // as ui4
#define RECORD_INDEX_VERSION_MAJOR_OFFSET_m11	                        21                      // ui1
#define RECORD_INDEX_VERSION_MAJOR_NO_ENTRY_m11	                        0xFF
#define RECORD_INDEX_VERSION_MINOR_OFFSET_m11	                        22                      // ui1
#define RECORD_INDEX_VERSION_MINOR_NO_ENTRY_m11	                        0xFF
#define RECORD_INDEX_ENCRYPTION_LEVEL_OFFSET_m11                        23                      // si1
#define RECORD_INDEX_ENCRYPTION_LEVEL_NO_ENTRY_m11                      ENCRYPTION_LEVEL_NO_ENTRY_m11

// Time Series Index: Format Constants
#define TIME_SERIES_INDEX_BYTES_m11                                     INDEX_BYTES_m11
#define TIME_SERIES_INDEX_FILE_OFFSET_OFFSET_m11                        0               // si8
#define TIME_SERIES_INDEX_FILE_OFFSET_NO_ENTRY_m11                      -1
#define TIME_SERIES_INDEX_START_TIME_OFFSET_m11			        8               // si8
#define TIME_SERIES_INDEX_START_TIME_NO_ENTRY_m11                       UUTC_NO_ENTRY_m11
#define TIME_SERIES_INDEX_START_SAMPLE_NUMBER_OFFSET_m11                16              // si8
#define TIME_SERIES_INDEX_START_SAMPLE_NUMBER_NO_ENTRY_m11              -1

// Video Index: Format Constants
#define VIDEO_INDEX_BYTES_m11			                INDEX_BYTES_m11
#define VIDEO_INDEX_FILE_OFFSET_OFFSET_m11                      0                       // si8
#define VIDEO_INDEX_FILE_OFFSET_NO_ENTRY_m11                    -1
#define VIDEO_INDEX_START_TIME_OFFSET_m11                       8                       // si8
#define VIDEO_INDEX_START_TIME_NO_ENTRY_m11                     UUTC_NO_ENTRY_m11
#define VIDEO_INDEX_START_FRAME_OFFSET_m11                      16                      // ui4
#define VIDEO_INDEX_START_FRAME_NO_ENTRY_m11                    0xFFFFFFFF
#define VIDEO_INDEX_VIDEO_FILE_NUMBER_OFFSET_m11                20                      // ui4
#define VIDEO_INDEX_VIDEO_FILE_NUMBER_NO_ENTRY_m11              0
#define VIDEO_INDEX_TERMINAL_VIDEO_FILE_NUMBER_m11              0xFFFFFFFF

// Level Header (LH) Type Codes:
#define LH_SESSION_m11			SESSION_DIRECTORY_TYPE_CODE_m11
#define LH_SEGMENTED_SESS_RECS_m11	RECORD_DIRECTORY_TYPE_CODE_m11   // technically a session level element, but handy for distinguishing session records from segmented sess recs
#define LH_TIME_SERIES_CHANNEL_m11	TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m11
#define LH_VIDEO_CHANNEL_m11		VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m11
#define LH_TIME_SERIES_SEGMENT_m11	TIME_SERIES_SEGMENT_DIRECTORY_TYPE_CODE_m11
#define LH_VIDEO_SEGMENT_m11		VIDEO_SEGMENT_DIRECTORY_TYPE_CODE_m11

// Level Header (LH) Flags Definitions:

// level header flags
// READ == on open: open file & read universal header (applies to data files, index files are always read in full)
//         on read: set FPS pointer to section specified by time slice (decrpyting if necessary)
// READ_FULL == on open: read full file (no memory mapping required, & closing)
// MMAP == allocate memory for full file, but only read on demand, (no re-reading occurs, but potentially memory expensive, good paired with VDS)
// ACTIVE ==  applies only to channels. Mark a CHANNEL as active to return data. Marking a channel as inactive does not free or close it.
// EPHEMERAL DATA == if GENERATE_EPHEMERAL_DATA_m11 is set, ephemeral data is created if it does not exist.
//	If UPDATE_EPHEMERAL_DATA is set, the data is updated whenever the channel or segment open set changes (opening of new elements, not the active status)
//	The UPDATE_EPHEMERAL_DATA bit is set by the lower levels and reset by the higher level once the data has been updated.
//	i.e  read_channel_m11() checks the segment bits (e.g. read_segment_m11() opened a new segment) & if update required, it does the channel level update & clears the segment bit.
//	It then sets it's bit to trigger update at the session level. After updating, the session will clear the channel level bit.

// all levels
#define LH_NO_FLAGS_m11					((ui8) 0)
#define LH_USE_GLOBAL_FLAGS_m11				LH_NO_FLAGS_m11
#define LH_OPEN_m11					((ui8) 1 << 0)	// all level & sublevel flags have been operated on
#define LH_GENERATE_EPHEMERAL_DATA_m11			((ui8) 1 << 1)	// implies all level involvement
#define LH_UPDATE_EPHEMERAL_DATA_m11			((ui8) 1 << 2)	// signal to higher level from lower level (reset by higher level after update)

// session level
#define LH_INCLUDE_TIME_SERIES_CHANNELS_m11		((ui8) 1 << 8)
#define LH_INCLUDE_VIDEO_CHANNELS_m11			((ui8) 1 << 9)
#define LH_MAP_ALL_TIME_SERIES_CHANNELS_m11		((ui8) 1 << 12)
#define LH_MAP_ALL_VIDEO_CHANNELS_m11			((ui8) 1 << 13)

#define LH_READ_SLICE_SESSION_RECORDS_m11		((ui8) 1 << 16)	// read full indices file (close file); open data, read universal header, leave open
#define LH_READ_FULL_SESSION_RECORDS_m11		((ui8) 1 << 17)	// read full indices file & data files, close all files
#define LH_MEM_MAP_SESSION_RECORDS_m11			((ui8) 1 << 18)	// allocate, but don't read full file
// segmented session records level
#define LH_READ_SLICE_SEGMENTED_SESS_RECS_m11		((ui8) 1 << 19)	// read full indices file (close file); open data, read universal header, leave open
#define LH_READ_FULL_SEGMENTED_SESS_RECS_m11		((ui8) 1 << 20)	// read full indices file & data files, close all files
#define LH_MEM_MAP_SEGMENTED_SESS_RECS_m11		((ui8) 1 << 21)	// allocate, but don't read full data file

// channel level
#define LH_CHANNEL_ACTIVE_m11				((ui8) 1 << 32)
#define LH_MAP_ALL_SEGMENTS_m11				((ui8) 1 << 33)
// (active channels only)
#define LH_READ_SLICE_CHANNEL_RECORDS_m11		((ui8) 1 << 40)	// read full indices file (close file); open data, read universal header, leave open
#define LH_READ_FULL_CHANNEL_RECORDS_m11		((ui8) 1 << 41)	// read full indices file & data files, close all files
#define LH_MEM_MAP_CHANNEL_RECORDS_m11			((ui8) 1 << 42)	// allocate, but don't read full file

// segment level (active channels only)
#define LH_READ_SLICE_SEGMENT_DATA_m11			((ui8) 1 << 48)	// read full metadata & indices files, close files; open data, read universal header, leave open
#define LH_READ_FULL_SEGMENT_DATA_m11			((ui8) 1 << 49)	// read full metadata, indices, & data files, close all files
#define LH_MEM_MAP_SEGMENT_DATA_m11			((ui8) 1 << 50)	// allocate, but don't read full file
#define LH_READ_SLICE_SEGMENT_RECORDS_m11		((ui8) 1 << 51)	// read full indices file (close file); open data, read universal header, leave open
#define LH_READ_FULL_SEGMENT_RECORDS_m11		((ui8) 1 << 52)	// read full indices file & data files, close all files
#define LH_MEM_MAP_SEGMENT_RECORDS_m11			((ui8) 1 << 53)	// allocate, but don't read full file
#define LH_READ_SEGMENT_METADATA_m11			((ui8) 1 << 54)	// read segment metadata
#define LH_NO_CPS_PTR_RESET_m11				((ui8) 1 << 60) // caller will update pointers
#define LH_NO_CPS_CACHING_m11				((ui8) 1 << 61) // set cps_caching parameter to FALSE

// flag groups
#define LH_MAP_ALL_CHANNELS_m11       	      (	LH_MAP_ALL_TIME_SERIES_CHANNELS_m11 | LH_MAP_ALL_VIDEO_CHANNELS_m11 )

// reading masks (not to be used as flags: SLICE/FULL mutually exclusive)
#define LH_READ_SESSION_RECORDS_MASK_m11      (	LH_READ_SLICE_SESSION_RECORDS_m11 | LH_READ_FULL_SESSION_RECORDS_m11 )
#define LH_READ_SEGMENTED_SESS_RECS_MASK_m11  (	LH_READ_SLICE_SEGMENTED_SESS_RECS_m11 | LH_READ_FULL_SEGMENTED_SESS_RECS_m11 )
#define LH_READ_CHANNEL_RECORDS_MASK_m11      (	LH_READ_SLICE_CHANNEL_RECORDS_m11 | LH_READ_FULL_CHANNEL_RECORDS_m11 )
#define LH_READ_SEGMENT_RECORDS_MASK_m11      (	LH_READ_SLICE_SEGMENT_RECORDS_m11 | LH_READ_FULL_SEGMENT_RECORDS_m11 )
#define LH_READ_SEGMENT_DATA_MASK_m11         (	LH_READ_SLICE_SEGMENT_DATA_m11 | LH_READ_FULL_SEGMENT_DATA_m11 )
#define LH_ALL_READ_FLAGS_MASK_m11	      ( LH_READ_SESSION_RECORDS_MASK_m11 | LH_READ_SEGMENTED_SESS_RECS_MASK_m11 | \
						LH_READ_CHANNEL_RECORDS_MASK_m11 | LH_READ_SEGMENT_DATA_MASK_m11 | LH_READ_SEGMENT_RECORDS_MASK_m11 )
// memory map flags & masks
#define LH_MEM_MAP_ALL_RECORDS_m11	      ( LH_MEM_MAP_SESSION_RECORDS_m11 | LH_MEM_MAP_SEGMENTED_SESS_RECS_m11 | LH_MEM_MAP_CHANNEL_RECORDS_m11 | LH_MEM_MAP_SEGMENT_RECORDS_m11 )
#define LH_MEM_MAP_ALL_m11	      	      (	LH_MEM_MAP_ALL_RECORDS_m11 | LH_MEM_MAP_SEGMENT_DATA_m11 )
#define LH_ALL_MEM_MAP_FLAGS_MASK_m11		LH_MEM_MAP_ALL_m11

// record reading groups
#define LH_READ_SLICE_ALL_RECORDS_m11	      (	LH_READ_SLICE_SESSION_RECORDS_m11 | LH_READ_SLICE_SEGMENTED_SESS_RECS_m11 | LH_READ_SLICE_CHANNEL_RECORDS_m11 | LH_READ_SLICE_SEGMENT_RECORDS_m11 )
#define LH_READ_FULL_ALL_RECORDS_m11	      (	LH_READ_FULL_SESSION_RECORDS_m11 | LH_READ_FULL_SEGMENTED_SESS_RECS_m11 | LH_READ_FULL_CHANNEL_RECORDS_m11 | LH_READ_FULL_SEGMENT_RECORDS_m11 )

// channel type groups
#define LH_INCLUDE_ALL_CHAN_TYPES_m11	      (	LH_INCLUDE_TIME_SERIES_CHANNELS_m11 | LH_INCLUDE_VIDEO_CHANNELS_m11 )



//**********************************************************************************//
//******************************  MED Error Numbers  *******************************//
//**********************************************************************************//

// error codes
#define	E_NO_ERR_m11			0
#define E_NO_FILE_m11			1
#define E_READ_ERR_m11			2
#define E_WRITE_ERR_m11			3
#define E_NOT_MED_m11			4
#define E_BAD_PASSWORD_m11		5
#define E_NO_METADATA_m11		6
#define	E_NO_INET_m11			7


// error strings
#define	E_NO_ERR_STR_m11		"no errors"
#define	E_NO_FILE_STR_m11		"file not found"
#define	E_READ_ERR_STR_m11		"file read error"
#define	E_WRITE_ERR_STR_m11		"file write error"
#define E_NOT_MED_STR_m11		"not a MED file or directory"
#define E_BAD_PASSWORD_STR_m11		"invalid password"
#define E_NO_METADATA_STR_m11		"metadata file not found"
#define	E_NO_INET_STR_m11		"no internet connection found"



//**********************************************************************************//
//**********************************  MED Macros  **********************************//
//**********************************************************************************//

#define ABS_m11(x)			( ((x) >= 0) ? (x) : -(x) )	// do not increment/decrement in call to ABS (as x occurs thrice)
#define HEX_STRING_BYTES_m11(x)         ( ((x) + 1) * 3 )
#define REMOVE_DISCONTINUITY_m11(x)     ( ((x) >= 0) ? (x) : -(x) )	// do not increment/decrement in call to REMOVE_DISCONTINUITY (as x occurs thrice)
#define APPLY_DISCONTINUITY_m11(x)      ( ((x) <= 0) ? (x) : -(x) )	// do not increment/decrement in call to APPLY_DISCONTINUITY (as x occurs thrice)
#define MAX_OPEN_FILES_m11(number_of_channels, number_of_segments)      ((5 * number_of_channels * number_of_segments) + (2 * number_of_segments) + (2 * number_of_channels) + 5)
									// Note: final +5 == 2 for session level records plus 3 for standard streams (stdin, stdout, & stderr)
// "S" versions are for slice structures (not pointers)
#define TIME_SLICE_SAMPLE_COUNT_m11(slice)	(((slice)->end_sample_number - (slice)->start_sample_number) + 1)
#define TIME_SLICE_SAMPLE_COUNT_S_m11(slice)	(((slice).end_sample_number - (slice).start_sample_number) + 1)
#define TIME_SLICE_SEGMENT_COUNT_m11(slice)	(((slice)->end_segment_number - (slice)->start_segment_number) + 1)
#define TIME_SLICE_SEGMENT_COUNT_S_m11(slice)	(((slice).end_segment_number - (slice).start_segment_number) + 1)
#define TIME_SLICE_DURATION_m11(slice)		(((slice)->end_time - (slice)->start_time) + 1)  // time in usecs
#define TIME_SLICE_DURATION_S_m11(slice)	(((slice).end_time - (slice).start_time) + 1)  // time in usecs



//**********************************************************************************//
//************************************  Mutexes  ***********************************//
//**********************************************************************************//

#if defined MACOS_m11 || defined LINUX_m11
	typedef	pthread_mutex_t		pthread_mutex_t_m11;
	typedef	pthread_mutexattr_t	pthread_mutexattr_t_m11;
#endif

#ifdef WINDOWS_m11
	typedef	HANDLE			pthread_mutex_t_m11;
	typedef	SECURITY_ATTRIBUTES	pthread_mutexattr_t_m11;
#endif

si4		pthread_mutex_destroy_m11(pthread_mutex_t_m11 *mutex);
si4		pthread_mutex_init_m11(pthread_mutex_t_m11 *mutex, pthread_mutexattr_t_m11 *attr);
si4		pthread_mutex_lock_m11(pthread_mutex_t_m11 *mutex);
si4		pthread_mutex_unlock_m11(pthread_mutex_t_m11 *mutex);


//**********************************************************************************//
//**********************************  MED Globals  *********************************//
//**********************************************************************************//

// Daylight Change code
typedef union {
	struct {
		si1     code_type;                      // (DST end / DST Not Observed / DST start) ==  (-1 / 0 / +1)
		si1	day_of_week;                    // (No Entry / [Sunday : Saturday]) ==  (-1 / [0 : 6])
		si1     relative_weekday_of_month;      // (No Entry / [First : Fifth] / Last) ==  (0 / [1 : 5] / 6)
		si1     day_of_month;                   // (No Entry / [1 : 31]) ==  (0 / [1 : 31])
		si1     month;                          // (No Entry / [January : December]) ==  (-1 / [0 : 11])
		si1     hours_of_day;                   // [-128 : +127] hours relative to 0:00 (midnight)
		si1     reference_time;                 // (Local / UTC) ==  (0 / +1)
		si1     shift_minutes;                  // [-120 : +120] minutes
	};
	si8     value;                                  // 0 indicates DST is not observed, -1 indicates no entry
} DAYLIGHT_TIME_CHANGE_CODE_m11;

typedef struct {
	si1	country[METADATA_RECORDING_LOCATION_BYTES_m11];
	si1	country_acronym_2_letter[3]; // two-letter acronym; (ISO 3166 ALPHA-2)
	si1	country_acronym_3_letter[4]; // three-letter acronym (ISO-3166 ALPHA-3)
	si1	territory[METADATA_RECORDING_LOCATION_BYTES_m11];
	si1	territory_acronym[TIMEZONE_STRING_BYTES_m11];
	si1	standard_timezone[TIMEZONE_STRING_BYTES_m11];
	si1	standard_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m11];
	si4	standard_UTC_offset; // seconds
	si1	daylight_timezone[TIMEZONE_STRING_BYTES_m11];
	si1	daylight_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m11];
	si8	daylight_time_start_code;  // DAYLIGHT_TIME_CHANGE_CODE_m11 - cast to use other fields
	si8	daylight_time_end_code;  // DAYLIGHT_TIME_CHANGE_CODE_m11 - cast to use other fields
	si8	daylight_codes_start_date;  // onset of rules encoded in daylight codes (in uutc). DTCC_START_DATE_NO_ENTRY (-1) indicates it is the only historical rule for this timezone in the table.
} TIMEZONE_INFO_m11;

typedef struct {
	si1	table_name[METADATA_RECORDING_LOCATION_BYTES_m11];
	si1	alias[METADATA_RECORDING_LOCATION_BYTES_m11];
} TIMEZONE_ALIAS_m11;

typedef struct {
	TERN_m11	conditioned;
	si4		number_of_segments;  // == UNKNOWN_m11 if segment range is unknown, otherwise == number of segments in slice
	si8     	start_time;
	si8     	end_time;
	union {  // session-relative (global indexing)
		si8	start_sample_number;
		si8	start_frame_number;
	};
	union {  // session-relative (global indexing)
		si8	end_sample_number;
		si8	end_frame_number;
	};
	si4     	start_segment_number;
	si4     	end_segment_number;
} TIME_SLICE_m11;

typedef struct {
	si8     	start_time;
	si8     	end_time;
	union {  // session-relative (global indexing)
		si8	start_sample_number;
		si8	start_frame_number;
	};
	union {  // session-relative (global indexing)
		si8	end_sample_number;
		si8	end_frame_number;
	};
	si4     	start_segment_number;
	si4     	end_segment_number;
} CONTIGUON_m11;

typedef struct {  // times in uutc
	si8	creation;
	si8	access;
	si8	modification;
} FILE_TIMES_m11;

typedef struct {  // fields from ipinfo.io
	TIMEZONE_INFO_m11	timezone_info;
	si1			WAN_IPv4_address[IPV4_ADDRESS_BYTES_m11 * 4];
	si1			locality[LOCALITY_BYTES_m11];
	si1			postal_code[POSTAL_CODE_BYTES_m11];
	si1			timezone_description[METADATA_RECORDING_LOCATION_BYTES_m11];
	sf8			latitude;
	sf8			longitude;
} LOCATION_INFO_m11;

#ifdef AT_DEBUG_m11
typedef struct {
	void 		*address;
	ui8		bytes;  // actual bytes allocated => may be more than were requested
	const si1	*alloc_function;
	const si1	*free_function;
} AT_NODE;
#else
typedef struct {
	void 		*address;
	ui8		bytes;  // actual bytes allocated => may be more than were requested
} AT_NODE;
#endif

typedef struct {
	// Password
	PASSWORD_DATA_m11               password_data;
	// Record Filters
	si4 				*record_filters;	// signed, "NULL terminated" array version of MED record type codes to include or exclude when reading records.
								// The terminal entry is NO_TYPE_CODE_m11 (== zero). NULL or no filter codes includes all records (== no filters).
								// filter modes: match positive: include
								//		 match negative: exclude
								//		 no match:
								//			all filters positive: exclude
								//			else: include
								// Note: as type codes are composed of ascii bytes values (< 0x80), it is always possible to make them negative without promotion.
	// Current Session
	si8				session_UID;
	si1				session_directory[FULL_FILE_NAME_BYTES_m11];	// path including file system session directory name
	si1				*session_name;  				// points to: uh_session_name if known, else fs_session_name if known, else NULL
	si1				uh_session_name[BASE_FILE_NAME_BYTES_m11];	// from MED universal header - original name
	si1				fs_session_name[BASE_FILE_NAME_BYTES_m11];	// from file system - may be renamed by user (e.g. channel subset)
	si8				session_start_time;
	si8				session_end_time;
	union {
		si8			number_of_session_samples;
		si8			number_of_session_frames;
	};
	si4				number_of_session_segments;	// number of segments in the session, regardless of whether they are mapped
	si4				number_of_mapped_segments;	// may be less than number_of_session_segments
	si4				first_mapped_segment_number;
	si1				reference_channel_name[BASE_FILE_NAME_BYTES_m11];	// contains user specified value if needed, open_session_m11() matches to session channel
	struct CHANNEL_m11		*reference_channel;		// note "reference" here refers to reference channel for sample/frame numbers, not the time series recording reference electrode
	// Active Channels (applies to active channel set)
	TERN_m11 			time_series_frequencies_vary;
	sf8				minimum_time_series_frequency;
	sf8				maximum_time_series_frequency;
	struct CHANNEL_m11		*minimum_time_series_frequency_channel;
	struct CHANNEL_m11		*maximum_time_series_frequency_channel;
	TERN_m11 			video_frame_rates_vary;
	sf8				minimum_video_frame_rate;
	sf8				maximum_video_frame_rate;
	struct CHANNEL_m11		*minimum_video_frame_rate_channel;
	struct CHANNEL_m11		*maximum_video_frame_rate_channel;
	// Time Constants
	TERN_m11			time_constants_set;
	TERN_m11			RTO_known;
	TERN_m11                        observe_DST;
	si8                             recording_time_offset;
	si4                             standard_UTC_offset;
	si1                             standard_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m11];
	si1                             standard_timezone_string[TIMEZONE_STRING_BYTES_m11];
	si1                             daylight_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m11];
	si1                             daylight_timezone_string[TIMEZONE_STRING_BYTES_m11];
	DAYLIGHT_TIME_CHANGE_CODE_m11   daylight_time_start_code;  // si1[8] / si8
	DAYLIGHT_TIME_CHANGE_CODE_m11   daylight_time_end_code;  // si1[8] / si8
	TIMEZONE_INFO_m11		*timezone_table;
	TIMEZONE_ALIAS_m11		*country_aliases_table;
	TIMEZONE_ALIAS_m11		*country_acronym_aliases_table;
	pthread_mutex_t_m11		TZ_mutex;
	// Alignment Fields
	TERN_m11                        universal_header_aligned;
	TERN_m11                        metadata_section_1_aligned;
	TERN_m11                        time_series_metadata_section_2_aligned;
	TERN_m11                        video_metadata_section_2_aligned;
	TERN_m11                        metadata_section_3_aligned;
	TERN_m11                        all_metadata_structures_aligned;
	TERN_m11                        time_series_indices_aligned;
	TERN_m11                        video_indices_aligned;
	TERN_m11                        CMP_block_header_aligned;
	TERN_m11			CMP_record_header_aligned;
	TERN_m11                        record_header_aligned;
	TERN_m11                        record_indices_aligned;
	TERN_m11                        all_record_structures_aligned;
	TERN_m11                        all_structures_aligned;
	ui4				**CRC_table;
	ui4                             CRC_mode;
	pthread_mutex_t_m11		CRC_mutex;
	// AES tables
	si4				*AES_sbox_table;
	si4				*AES_rcon_table;
	si4				*AES_rsbox_table;
	pthread_mutex_t_m11		AES_mutex;
	// SHA256 tables
	ui4				*SHA_h0_table;
	ui4				*SHA_k_table;
	pthread_mutex_t_m11		SHA_mutex;
	// UTF8 tables
	ui4				*UTF8_offsets_table;
	si1				*UTF8_trailing_bytes_table;
	pthread_mutex_t_m11		UTF8_mutex;
	// allocation tracking (AT)
	AT_NODE				*AT_nodes;
	si8				AT_node_count;  // total allocated nodes
	si8				AT_used_node_count;  // nodes in use
	pthread_mutex_t_m11		AT_mutex;
	// Errors
	si4				err_code;
	const si1			*err_func;
	si4				err_line;
	// Miscellaneous
	TERN_m11			time_series_data_encryption_level;
	TERN_m11                        verbose;
	ui4                             behavior_on_fail;
	si1				temp_dir[FULL_FILE_NAME_BYTES_m11];  // system temp directory (periodically auto-cleared)
	si1				temp_file[FULL_FILE_NAME_BYTES_m11];  // full path to temp file (i.e. includes temp_dir)
	ui4				*behavior_stack;
	volatile ui4			behavior_stack_entries;
	volatile ui4			behavior_stack_size;
	pthread_mutex_t_m11		behavior_mutex;
	ui8				level_header_flags;
	ui4				mmap_block_bytes;  // read size for memory mapped files
} GLOBALS_m11;


//**********************************************************************************//
//********************************  MED Structures  ********************************//
//**********************************************************************************//


// Universal Header Structure
typedef struct {
	// start robust mode region
	ui4	header_CRC;     // CRC of the universal header after this field
	ui4     body_CRC;       // CRC of the entire file after the universal header
	union {  // "segment_end_time" used in code when that is it's meaning, for clarity
		si8	file_end_time;
		si8	segment_end_time;
	};
	si8	number_of_entries;
	ui4	maximum_entry_size;
	// end robust mode region
	si4     segment_number;
	union {
		struct {
			si1     type_string[TYPE_BYTES_m11];
			ui1     MED_version_major;
			ui1     MED_version_minor;
			ui1     byte_order_code;
		};
		struct {
			ui4     type_code;
			si1	type_string_terminal_zero;  // not used - here for clarity
		};
	};
	si8	session_start_time;
	union {  // "segment_start_time" used in code when that is it's meaning, for clarity
		si8	file_start_time;
		si8	segment_start_time;
	};
	si1	session_name[BASE_FILE_NAME_BYTES_m11]; // utf8[63], base name only, no extension
	si1     channel_name[BASE_FILE_NAME_BYTES_m11]; // utf8[63], base name only, no extension
	si1	anonymized_subject_ID[UNIVERSAL_HEADER_ANONYMIZED_SUBJECT_ID_BYTES_m11]; // utf8[63]
	ui8	session_UID;
	ui8     channel_UID;
	ui8     segment_UID;
	ui8	file_UID;
	ui8	provenance_UID;
	ui1	level_1_password_validation_field[PASSWORD_VALIDATION_FIELD_BYTES_m11];
	ui1     level_2_password_validation_field[PASSWORD_VALIDATION_FIELD_BYTES_m11];
	ui1	level_3_password_validation_field[PASSWORD_VALIDATION_FIELD_BYTES_m11];
	ui1	protected_region[UNIVERSAL_HEADER_PROTECTED_REGION_BYTES_m11];
	ui1	discretionary_region[UNIVERSAL_HEADER_DISCRETIONARY_REGION_BYTES_m11];
} UNIVERSAL_HEADER_m11;

// Metadata Structures
typedef struct {
	si1     level_1_password_hint[PASSWORD_HINT_BYTES_m11];
	si1     level_2_password_hint[PASSWORD_HINT_BYTES_m11];
	si1     section_2_encryption_level;
	si1     section_3_encryption_level;
	si1     time_series_data_encryption_level;
	ui1     protected_region[METADATA_SECTION_1_PROTECTED_REGION_BYTES_m11];
	ui1     discretionary_region[METADATA_SECTION_1_DISCRETIONARY_REGION_BYTES_m11];
} METADATA_SECTION_1_m11;

typedef struct {
	// channel type independent fields
	si1     session_description[METADATA_SESSION_DESCRIPTION_BYTES_m11];            // utf8[511]
	si1     channel_description[METADATA_CHANNEL_DESCRIPTION_BYTES_m11];            // utf8[255]
	si1     segment_description[METADATA_SEGMENT_DESCRIPTION_BYTES_m11];            // utf8[255]
	si1     equipment_description[METADATA_EQUIPMENT_DESCRIPTION_BYTES_m11];        // utf8[510]
	si4     acquisition_channel_number;
	// channel type specific fields
	si1     reference_description[TIME_SERIES_METADATA_REFERENCE_DESCRIPTION_BYTES_m11];        // utf8[255]
	sf8     sampling_frequency;
	sf8     low_frequency_filter_setting;
	sf8     high_frequency_filter_setting;
	sf8     notch_filter_frequency_setting;
	sf8     AC_line_frequency;
	sf8     amplitude_units_conversion_factor;
	si1     amplitude_units_description[TIME_SERIES_METADATA_AMPLITUDE_UNITS_DESCRIPTION_BYTES_m11];  // utf8[31]
	sf8     time_base_units_conversion_factor;
	si1     time_base_units_description[TIME_SERIES_METADATA_TIME_BASE_UNITS_DESCRIPTION_BYTES_m11];  // utf8[31]
	si8     absolute_start_sample_number;
	si8     number_of_samples;
	si8	number_of_blocks;
	si8     maximum_block_bytes;
	ui4     maximum_block_samples;
	ui4     maximum_block_keysample_bytes;
	sf8     maximum_block_duration;
	si8     number_of_discontinuities;
	si8     maximum_contiguous_blocks;
	si8     maximum_contiguous_block_bytes;
	si8     maximum_contiguous_samples;
	ui1     protected_region[TIME_SERIES_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m11];
	ui1     discretionary_region[TIME_SERIES_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m11];
} TIME_SERIES_METADATA_SECTION_2_m11;

typedef struct {
	// type-independent fields
	si1     session_description[METADATA_SESSION_DESCRIPTION_BYTES_m11];			// utf8[511]
	si1     channel_description[METADATA_CHANNEL_DESCRIPTION_BYTES_m11];			// utf8[511]
	si1     segment_description[METADATA_SEGMENT_DESCRIPTION_BYTES_m11];			// utf8[511]
	si1     equipment_description[METADATA_EQUIPMENT_DESCRIPTION_BYTES_m11];        	// utf8[510]
	si4     acquisition_channel_number;
	// type-specific fields
	sf8     time_base_units_conversion_factor;
	si1     time_base_units_description[VIDEO_METADATA_TIME_BASE_UNITS_DESCRIPTION_BYTES_m11];	// utf8[31]
	si8     absolute_start_frame_number;
	si8     number_of_frames;
	sf8     frame_rate;
	si8     number_of_clips;
	si8     maximum_clip_bytes;
	ui4     maximum_clip_frames;
	si4	number_of_video_files;
	sf8     maximum_clip_duration;
	si8     number_of_discontinuities;
	si8	maximum_contiguous_clips;
	si8	maximum_contiguous_clip_bytes;
	si8	maximum_contiguous_frames;
	ui4     horizontal_pixels;
	ui4     vertical_pixels;
	si1     video_format[VIDEO_METADATA_VIDEO_FORMAT_BYTES_m11];                		// utf8[31]
	ui1     protected_region[VIDEO_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m11];
	ui1     discretionary_region[VIDEO_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m11];
} VIDEO_METADATA_SECTION_2_m11;

// All metadata section substructures are the same sizes
typedef union {
		ui1					section_2[METADATA_SECTION_2_BYTES_m11];
		TIME_SERIES_METADATA_SECTION_2_m11	time_series_section_2;
		VIDEO_METADATA_SECTION_2_m11		video_section_2;
} METADATA_SECTION_2_m11;

typedef struct {
	si8     recording_time_offset;
	DAYLIGHT_TIME_CHANGE_CODE_m11   daylight_time_start_code;                       // si1[8] / si8
	DAYLIGHT_TIME_CHANGE_CODE_m11   daylight_time_end_code;                         // si1[8] / si8
	si1     standard_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m11];                  // ascii[8]
	si1     standard_timezone_string[TIMEZONE_STRING_BYTES_m11];                    // ascii[31]
	si1     daylight_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m11];                  // ascii[8]
	si1     daylight_timezone_string[TIMEZONE_STRING_BYTES_m11];                    // ascii[31]
	si1     subject_name_1[METADATA_SUBJECT_NAME_BYTES_m11];                        // utf8[31]
	si1     subject_name_2[METADATA_SUBJECT_NAME_BYTES_m11];                        // utf8[31]
	si1     subject_name_3[METADATA_SUBJECT_NAME_BYTES_m11];                        // utf8[31]
	si1     subject_ID[METADATA_SUBJECT_ID_BYTES_m11];                              // utf8[31]
	si1     recording_country[METADATA_RECORDING_LOCATION_BYTES_m11];               // utf8[63]
	si1     recording_territory[METADATA_RECORDING_LOCATION_BYTES_m11];             // utf8[63]
	si1     recording_locality[METADATA_RECORDING_LOCATION_BYTES_m11];              // utf8[63]
	si1     recording_institution[METADATA_RECORDING_LOCATION_BYTES_m11];           // utf8[63]
	si1     geotag_format[METADATA_GEOTAG_FORMAT_BYTES_m11];                        // ascii[31]
	si1     geotag_data[METADATA_GEOTAG_DATA_BYTES_m11];                            // ascii[1023]
	si4     standard_UTC_offset;
	ui1     protected_region[METADATA_SECTION_3_PROTECTED_REGION_BYTES_m11];
	ui1     discretionary_region[METADATA_SECTION_3_DISCRETIONARY_REGION_BYTES_m11];
} METADATA_SECTION_3_m11;

#ifdef __cplusplus  // c++ does not accept anonymous structures
typedef struct {
	METADATA_SECTION_1_m11		section_1;
	union {
		ui1					section_2[METADATA_SECTION_2_BYTES_m11];
		TIME_SERIES_METADATA_SECTION_2_m11	time_series_section_2;
		VIDEO_METADATA_SECTION_2_m11		video_section_2;
	};
	METADATA_SECTION_3_m11		section_3;
} METADATA_m11;
#else
typedef struct {
	METADATA_SECTION_1_m11		section_1;
	METADATA_SECTION_2_m11;
	METADATA_SECTION_3_m11		section_3;
} METADATA_m11;
#endif

// Record Structures
typedef struct RECORD_HEADER_m11 {  // struct name for medrec_m11.h interdependency
	ui4	record_CRC;
	ui4     total_record_bytes;  // header + body bytes
	si8     start_time;
	union {  // anonymous union
		struct {
			si1     type_string[TYPE_BYTES_m11];
			ui1     version_major;
			ui1     version_minor;
			si1     encryption_level;
		};
		struct {
			ui4     type_code;
			si1	type_string_terminal_zero;  // not used - here for clarity
		};
	};
} RECORD_HEADER_m11;

typedef struct {
	si8	file_offset;  // never negative: the record indices are not used to indicate discontinuities
	si8	start_time;
	union {  // anonymous union
		struct {
			si1     type_string[TYPE_BYTES_m11];
			ui1     version_major;
			ui1     version_minor;
			si1     encryption_level;
		};
		struct {
			ui4     type_code;
			si1	type_string_terminal_zero;  // not used - there for clarity
		};
	};
} RECORD_INDEX_m11;

// Time Series Indices Structures
typedef struct {
	si8	file_offset;  // negative values indicate discontinuity
	si8	start_time;
	si8     start_sample_number;
} TIME_SERIES_INDEX_m11;

// Video Indices Structures
typedef struct {
	si8     file_offset;  // negative values indicate discontinuity
	si8	start_time;
	ui4     start_frame_number;
	ui4     video_file_number;
} VIDEO_INDEX_m11;

typedef struct {
	si8     file_offset;  // negative values indicate discontinuity (in time series & video indices)
	si8	start_time;
	ui1     pad[8];
} GENERIC_INDEX_m11;

// All index structures are the same size, and have the same first two fields (hence GENERIC_INDEX_m11)
typedef struct {
	union {
		RECORD_INDEX_m11	record_index;
		TIME_SERIES_INDEX_m11	time_series_index;
		VIDEO_INDEX_m11		video_index;
		GENERIC_INDEX_m11	generic_index;
	};
} INDEX_m11;

// File Processing Structures
typedef struct {
	TERN_m11        close_file;
	TERN_m11        flush_after_write;
	TERN_m11        update_universal_header;	// when writing
	TERN_m11        leave_decrypted;		// if encrypted during write, return from write function decrypted
	TERN_m11        free_password_data;		// when freeing FPS
	TERN_m11        free_CMP_processing_struct;	// when freeing FPS
	ui4             lock_mode;
	ui4             open_mode;
	TERN_m11	memory_map;  // full file allocated; read regions stored in bitmap; no re-reads; efficient, but memory expensive
} FPS_DIRECTIVES_m11;

// Parameters contain "mechanics" of FPS (mostly used internally by library functions)
typedef struct {
	pthread_mutex_t_m11			mutex;
	si8					last_access_time;	// uutc of last read into or write from this structure to the file system (update by read_file_m11 & write_file_m11)
	TERN_m11				full_file_read;		// full file has been read in / decrypted
	si8					raw_data_bytes;		// bytes in raw data array,
	ui1					*raw_data;		// universal header followed by data (in standard read - just region requested, in full file & mem map - matches media)
	PASSWORD_DATA_m11			*password_data;
	struct CMP_PROCESSING_STRUCT_m11	*cps;			// for time series data FPSs
	// file status
	si4			fd;	// file descriptor
	FILE			*fp;	// file pointer
	si8			fpos;	// current file pointer position (from file start)
	si8			flen;	// file length
	// memory mapping
	ui4			mmap_block_bytes;  // bytes per bit in block bitmap
	ui4			mmap_number_of_blocks;  // file system block in file == number of bits in bitmap
	ui8			*mmap_block_bitmap;  // each bit represents block_bytes bytes;  NULL if not memory mapping
} FPS_PARAMETERS_m11;

typedef struct {  // struct name for CMP functions interdependency
	void					*parent;  // parent structure, NULL if created alone
	si1					full_file_name[FULL_FILE_NAME_BYTES_m11];  // full path from root including extension
	UNIVERSAL_HEADER_m11			*universal_header;  // points to base of raw_data array
	FPS_DIRECTIVES_m11	        	directives;
	FPS_PARAMETERS_m11	        	parameters;
	union {					// the MED file types
						// these are set to point to current data (just read, or to write)
		METADATA_m11			*metadata;
		RECORD_INDEX_m11		*record_indices;  // if filters applied & file mem map or full file, points to filt_rec_data in FPS parameters
		ui1				*record_data;  // if filters applied & file mem map or full file, points to filt_rec_data in FPS parameters
		TIME_SERIES_INDEX_m11		*time_series_indices;
		ui1				*time_series_data;  // compressed data (not modified), CPS block header is modifiable pointer within this array
		VIDEO_INDEX_m11			*video_indices;
		void				*video_data;  // place holder - not yet implemented  (universal_header is NULL as video data is stored in native video format)
		ui1				*data_pointers;  // generic name for all of the above (dissociable from raw data array / universal header, if needed)
	};
	si8					number_of_items;  // items in current read/write, not necessarily the whole file
} FILE_PROCESSING_STRUCT_m11;

// Session, Segmented Session Records, Channel, & Segment Structures
typedef struct {
	union {  // anonymous union
		struct {
			si1     type_string[TYPE_BYTES_m11];
			ui1     pad[3];  // force to 8-byte alignment to avoid alignment issues in potential future uses (in current usage, type_code without string would be sufficient)
		};
		struct {
			ui4     type_code;
			si1	type_string_terminal_zero;  // not used - there for clarity
		};
	};
	void	*parent;  // parent structure, NULL for session or if created alone
	ui8	flags;
	si8	last_access_time;  // uutc of last use of this structure by the calling program (updated by read & open functions)
} LEVEL_HEADER_m11;


//**********************************************************************************//
//****************************  Non-standard Structures  ***************************//
//**********************************************************************************//

// required compiler option (gcc, clang):  -fms-extensions
// suppress warnings:  -Wno-microsoft-anon-tag
#ifdef __cplusplus
typedef struct {
	struct {  // this replaces RECORD_HEADER_m11 for C++
		ui4	record_CRC;
		ui4     total_record_bytes;  // header + body bytes
		si8     start_time;
		union {  // anonymous union
			struct {
				si1     type_string[TYPE_BYTES_m11];
				ui1     version_major;
				ui1     version_minor;
				si1     encryption_level;
			};
			struct {
				ui4     type_code;
				si1	type_string_terminal_zero;  // not used - here for clarity
			};
		};
	};
	struct {  // this replaces REC_Sgmt_v10_m11 for C++
		si8     end_time;
		union {
			si8     start_sample_number;	// session-relative (global indexing)
			si8     start_frame_number;	// session-relative (global indexing)
		};
		union {
			si8     end_sample_number;	// session-relative (global indexing)
			si8     end_frame_number;	// session-relative (global indexing)
		};
		ui8     segment_UID;
		si4     segment_number;
		si4     acquisition_channel_number;  // REC_Sgmt_v10_ACQUISITION_CHANNEL_NUMBER_ALL_CHANNELS_m11 in session level records
		union {
			sf8     sampling_frequency;  // channel sampling frequency (REC_Sgmt_v10_SAMPLING_FREQUENCY_VARIABLE_m11 in session level records, if sampling frequencies vary across time series channels)
			sf8     frame_rate;  	     // channel frame rate (REC_Sgmt_v10_FRAME_RATE_VARIABLE_m11 in session level records, if frame rates vary across video channels)
		};
	};
} Sgmt_RECORD_m11;
#else
typedef struct {
	union {
		RECORD_HEADER_m11	header;  // in case just want the record heade
		RECORD_HEADER_m11;	// anonoymous RECORD_HEADER_m11
	};
	union {
		REC_Sgmt_v10_m11	body;  // in case just want the record body
		REC_Sgmt_v10_m11;	// anonoymous REC_Sgmt_v10_m11
	};
} Sgmt_RECORD_m11;
#endif
// NOTE: construction of Sgmt_RECORD_m11 in this way allows direct reading of Sgmt record headers
// & bodies into this structure (excluding the segment description).

#ifdef __cplusplus
typedef struct {
	struct {  // this struct replaces LEVEL_HEADER_m11 for C++
		union {  // anonymous union
			struct {
				si1     type_string[TYPE_BYTES_m11];
				ui1     pad[3];  // force to 8-byte alignment to avoid alignment issues in potential future uses (in current usage, type_code without string would be sufficient)
			};
			struct {
				ui4     type_code;
				si1	type_string_terminal_zero;  // not used - there for clarity
			};
		};
		void	*parent;  // parent structure, NULL for session or if created alone
		ui8	flags;
		si8	last_access_time;  // uutc of last use of this structure by the calling program (updated by read & open functions)
	};
	FILE_PROCESSING_STRUCT_m11	*metadata_fps;  // also used as prototype
	union {
		FILE_PROCESSING_STRUCT_m11	*time_series_data_fps;
		FILE_PROCESSING_STRUCT_m11	*video_data_fps;
	};
	union {
		FILE_PROCESSING_STRUCT_m11	*time_series_indices_fps;
		FILE_PROCESSING_STRUCT_m11	*video_indices_fps;
	};
	FILE_PROCESSING_STRUCT_m11	*record_data_fps;
	FILE_PROCESSING_STRUCT_m11	*record_indices_fps;
	si1                             path[FULL_FILE_NAME_BYTES_m11]; // full path to segment directory (including segment directory itself)
	si1                             name[SEGMENT_BASE_FILE_NAME_BYTES_m11];  // stored here, no segment_name field in universal header
	TIME_SLICE_m11			time_slice;
	si8				number_of_contigua;
	CONTIGUON_m11			*contigua;
} SEGMENT_m11;
#else
typedef struct {
	union {
		LEVEL_HEADER_m11	header;  // in case just want the level header
		LEVEL_HEADER_m11;	// anonoymous LEVEL_HEADER_m11
	};
	FILE_PROCESSING_STRUCT_m11	*metadata_fps;  // also used as prototype
	union {
		FILE_PROCESSING_STRUCT_m11	*time_series_data_fps;
		FILE_PROCESSING_STRUCT_m11	*video_data_fps;
	};
	union  {
		FILE_PROCESSING_STRUCT_m11	*time_series_indices_fps;
		FILE_PROCESSING_STRUCT_m11	*video_indices_fps;
	};
	FILE_PROCESSING_STRUCT_m11	*record_data_fps;
	FILE_PROCESSING_STRUCT_m11	*record_indices_fps;
	si1                             path[FULL_FILE_NAME_BYTES_m11]; // full path to segment directory (including segment directory itself)
	si1                             name[SEGMENT_BASE_FILE_NAME_BYTES_m11];  // stored here, no segment_name field in universal header
	TIME_SLICE_m11			time_slice;
	si8				number_of_contigua;
	CONTIGUON_m11			*contigua;
} SEGMENT_m11;
#endif

#ifdef __cplusplus
typedef struct CHANNEL_m11 {
	struct {  // this struct replaces LEVEL_HEADER_m11 for C++
		union {  // anonymous union
			struct {
				si1     type_string[TYPE_BYTES_m11];
				ui1     pad[3];  // force to 8-byte alignment to avoid alignment issues in potential future uses (in current usage, type_code without string would be sufficient)
			};
			struct {
				ui4     type_code;
				si1	type_string_terminal_zero;  // not used - there for clarity
			};
		};
		void	*parent;  // parent structure, NULL for session or if created alone
		ui8	flags;
		si8	last_access_time;  // uutc of last use of this structure by the calling program (updated by read & open functions)
	};
	FILE_PROCESSING_STRUCT_m11	*metadata_fps;  // used as prototype or ephemeral file, does not correspond to stored data
	FILE_PROCESSING_STRUCT_m11	*record_data_fps;
	FILE_PROCESSING_STRUCT_m11	*record_indices_fps;
	SEGMENT_m11			**segments;
	Sgmt_RECORD_m11			*Sgmt_records;
	si1			        path[FULL_FILE_NAME_BYTES_m11]; // full path to channel directory (including channel directory itself)
	si1                             name[BASE_FILE_NAME_BYTES_m11];
	TIME_SLICE_m11			time_slice;
	si8				number_of_contigua;
	CONTIGUON_m11			*contigua;
} CHANNEL_m11;
#else
typedef struct CHANNEL_m11 {
	union {
		LEVEL_HEADER_m11	header;  // in case just want the level header
		LEVEL_HEADER_m11;	// anonoymous LEVEL_HEADER_m11
	};
	FILE_PROCESSING_STRUCT_m11	*metadata_fps;  // used as prototype or ephemeral file, does not correspond to stored data
	FILE_PROCESSING_STRUCT_m11	*record_data_fps;
	FILE_PROCESSING_STRUCT_m11	*record_indices_fps;
	SEGMENT_m11			**segments;
	Sgmt_RECORD_m11			*Sgmt_records;
	si1			        path[FULL_FILE_NAME_BYTES_m11]; // full path to channel directory (including channel directory itself)
	si1                             name[BASE_FILE_NAME_BYTES_m11];
	TIME_SLICE_m11			time_slice;
	si8				number_of_contigua;
	CONTIGUON_m11			*contigua;
} CHANNEL_m11;
#endif

#ifdef __cplusplus
typedef struct {
	struct LEVEL_HEADER_m11 {  // this struct replaces LEVEL_HEADER_m11 in C++
		union {  // anonymous union
			struct {
				si1     type_string[TYPE_BYTES_m11];
				ui1     pad[3];  // force to 8-byte alignment to avoid alignment issues in potential future uses (in current usage, type_code without string would be sufficient)
			};
			struct {
				ui4     type_code;
				si1	type_string_terminal_zero;  // not used - there for clarity
			};
		};
		void	*parent;  // parent structure, NULL for session or if created alone
		ui8	flags;
		si8	last_access_time;  // uutc of last use of this structure by the calling program (updated by read & open functions)
	};
	FILE_PROCESSING_STRUCT_m11	**record_data_fps;
	FILE_PROCESSING_STRUCT_m11	**record_indices_fps;
	si1			        path[FULL_FILE_NAME_BYTES_m11];		// full path to segmented session records directory (including directory itself)
	si1                             name[BASE_FILE_NAME_BYTES_m11];		// session name, duplcated in globals
	TIME_SLICE_m11			time_slice;
} SEGMENTED_SESS_RECS_m11;
#else
typedef struct {
	union {
		LEVEL_HEADER_m11	header;  // in case just want the level header
		LEVEL_HEADER_m11;	// anonoymous LEVEL_HEADER_m11
	};
	FILE_PROCESSING_STRUCT_m11	**record_data_fps;
	FILE_PROCESSING_STRUCT_m11	**record_indices_fps;
	si1			        path[FULL_FILE_NAME_BYTES_m11];		// full path to segmented session records directory (including directory itself)
	si1                             name[BASE_FILE_NAME_BYTES_m11];		// session name, duplicated in globals
	TIME_SLICE_m11			time_slice;
} SEGMENTED_SESS_RECS_m11;
#endif

#ifdef __cplusplus
typedef struct {
	struct {  // this struct replaces LEVEL_HEADER_m11 in C++
		union {  // anonymous union
			struct {
				si1     type_string[TYPE_BYTES_m11];
				ui1     pad[3];  // force to 8-byte alignment to avoid alignment issues in potential future uses (in current usage, type_code without string would be sufficient)
			};
			struct {
				ui4     type_code;
				si1	type_string_terminal_zero;  // not used - there for clarity
			};
		};
		void	*parent;  // parent structure, NULL for session or if created alone
		ui8	flags;
		si8	last_access_time;  // uutc of last use of this structure by the calling program (updated by read & open functions)
	};
	FILE_PROCESSING_STRUCT_m11	*time_series_metadata_fps;  // used as prototype or ephemeral file, does not correspond to stored data
	FILE_PROCESSING_STRUCT_m11	*video_metadata_fps;  // used as prototype or ephemeral file, does not correspond to stored data
	Sgmt_RECORD_m11			*Sgmt_records;
	si4			        number_of_time_series_channels;
	CHANNEL_m11			**time_series_channels;
	si4			        number_of_video_channels;
	CHANNEL_m11			**video_channels;
	FILE_PROCESSING_STRUCT_m11	*record_data_fps;
	FILE_PROCESSING_STRUCT_m11	*record_indices_fps;
	SEGMENTED_SESS_RECS_m11		*segmented_sess_recs;
	si1			        path[FULL_FILE_NAME_BYTES_m11];		// full path to session directory (including directory itself)
	si1                             *name;					// points to uh_name (universal header), if known otherwise to fs_name (from file system)
	si1                             uh_name[BASE_FILE_NAME_BYTES_m11];
	si1                             fs_name[BASE_FILE_NAME_BYTES_m11];
	TIME_SLICE_m11			time_slice;
	si8				number_of_contigua;
	CONTIGUON_m11			*contigua;
} SESSION_m11;
#else
typedef struct {
	union {
		LEVEL_HEADER_m11	header;  // in case just want the level header
		LEVEL_HEADER_m11;	// anonoymous LEVEL_HEADER_m11
	};
	FILE_PROCESSING_STRUCT_m11	*time_series_metadata_fps;  // used as prototype or ephemeral file, does not correspond to stored data
	FILE_PROCESSING_STRUCT_m11	*video_metadata_fps;  // used as prototype or ephemeral file, does not correspond to stored data
	Sgmt_RECORD_m11			*Sgmt_records;
	si4			        number_of_time_series_channels;
	CHANNEL_m11			**time_series_channels;
	si4			        number_of_video_channels;
	CHANNEL_m11			**video_channels;
	FILE_PROCESSING_STRUCT_m11	*record_data_fps;
	FILE_PROCESSING_STRUCT_m11	*record_indices_fps;
	SEGMENTED_SESS_RECS_m11		*segmented_sess_recs;
	si1			        path[FULL_FILE_NAME_BYTES_m11];		// full path to session directory (including session directory itself)
	si1                             *name;					// points to uh_name (universal header), if known otherwise to fs_name (from file system)
	si1                             uh_name[BASE_FILE_NAME_BYTES_m11];
	si1                             fs_name[BASE_FILE_NAME_BYTES_m11];	
	TIME_SLICE_m11			time_slice;
	si8				number_of_contigua;
	CONTIGUON_m11			*contigua;
} SESSION_m11;
#endif
// NOTE: placement of LEVEL_HEADER_m11 in SESSION_m11, SEGMENTED_SESS_RECS_m11, CHANNEL_m11, & SEGMENT_m11 structures allows passing of LEVEL_HEADER_m11 pointer to functions,
// and based on its content, functions can cast pointer to specific level structures.
// e.g:	if (level_header->type_code == LH_SESSION_m11)
//		sess = (SESSION_m11 *) level_header;


// Miscellaneous structures that depend on above
typedef struct {
	si4		acq_num;
	CHANNEL_m11	*chan;
} ACQ_NUM_SORT_m11;



//**********************************************************************************//
//****************************  GENERAL MED Functions  *****************************//
//**********************************************************************************//


// Prototypes
TERN_m11	adjust_open_file_limit_m11(si4 new_limit, TERN_m11 verbose_flag);
TERN_m11	all_zeros_m11(ui1 *bytes, si4 field_length);
si1		*behavior_string_m11(ui4 behavior, si1 *behavior_string);
si8		build_contigua_m11(LEVEL_HEADER_m11 *level_header);
Sgmt_RECORD_m11	*build_Sgmt_records_array_m11(FILE_PROCESSING_STRUCT_m11 *ri_fps, FILE_PROCESSING_STRUCT_m11 *rd_fps, CHANNEL_m11 *chan);
si8		bytes_for_items_m11(FILE_PROCESSING_STRUCT_m11 *fps, si8 *number_of_items, si8 read_file_offset);
void    	calculate_indices_CRCs_m11(FILE_PROCESSING_STRUCT_m11 *fps);
void            calculate_metadata_CRC_m11(FILE_PROCESSING_STRUCT_m11 *fps);
void            calculate_record_data_CRCs_m11(FILE_PROCESSING_STRUCT_m11 *fps);
void            calculate_time_series_data_CRCs_m11(FILE_PROCESSING_STRUCT_m11 *fps);
void		change_reference_channel_m11(SESSION_m11 *sess, CHANNEL_m11 *channel, si1 *channel_name, si1 channel_type);
ui4             channel_type_from_path_m11(si1 *path);
wchar_t		*char2wchar_m11(wchar_t *target, si1 *source);
TERN_m11	check_all_alignments_m11(void);
TERN_m11	check_char_type_m11(void);
TERN_m11	check_file_list_m11(si1 **file_list, si4 n_files);
TERN_m11	check_metadata_alignment_m11(ui1 *bytes);
TERN_m11	check_metadata_section_1_alignment_m11(ui1 *bytes);
TERN_m11	check_metadata_section_3_alignment_m11(ui1 *bytes);
TERN_m11        check_password_m11(si1 *password);
TERN_m11	check_record_header_alignment_m11(ui1 *bytes);
TERN_m11	check_record_indices_alignment_m11(ui1 *bytes);
TERN_m11	check_CMP_block_header_alignment_m11(ui1 *bytes);
TERN_m11        check_CMP_record_header_alignment_m11(ui1 *bytes);
TERN_m11	check_time_series_indices_alignment_m11(ui1 *bytes);
TERN_m11	check_time_series_metadata_section_2_alignment_m11(ui1 *bytes);
TERN_m11	check_universal_header_alignment_m11(ui1 *bytes);
TERN_m11	check_video_indices_alignment_m11(ui1 *bytes);
TERN_m11	check_video_metadata_section_2_alignment_m11(ui1 *bytes);
void		clear_terminal_m11(void);
si4		compare_acq_nums_m11(const void *a, const void *b);
void		condition_timezone_info_m11(TIMEZONE_INFO_m11 *tz_info);
void		condition_time_slice_m11(TIME_SLICE_m11 *slice);
si8		current_uutc_m11(void);
si4		days_in_month_m11(si4 month, si4 year);
TERN_m11        decrypt_metadata_m11(FILE_PROCESSING_STRUCT_m11 *fps);
TERN_m11        decrypt_record_data_m11(FILE_PROCESSING_STRUCT_m11 *fps, ...);  // varargs (fps == NULL): RECORD_HEADER_m11 *rh, si8 number_of_records  (used to decrypt Sgmt_records arrays)
TERN_m11        decrypt_time_series_data_m11(FILE_PROCESSING_STRUCT_m11 *fps);
si4             DST_offset_m11(si8 uutc);
void            error_message_m11(si1 *fmt, ...);
void		error_string_m11(void);
void            escape_chars_m11(si1 *string, si1 target_char, si8 buffer_len);
void            extract_path_parts_m11(si1 *full_file_name, si1 *path, si1 *name, si1 *extension);
void            extract_terminal_password_bytes_m11(si1 *password, si1 *password_bytes);
ui4             file_exists_m11(si1 *path);
si8		file_length_m11(FILE *fp, si1 *path);
FILE_TIMES_m11	*file_times_m11(FILE *fp, si1 *path, FILE_TIMES_m11 *ft, TERN_m11 set_time);
CONTIGUON_m11	*find_discontinuities_m11(LEVEL_HEADER_m11 *level_header, si8 *num_contigua);
si8		find_index_m11(SEGMENT_m11 *seg, si8 target, ui4 mode);
si1		*find_timezone_acronym_m11(si1 *timezone_acronym, si4 standard_UTC_offset, si4 DST_offset);
si1		*find_metadata_file_m11(si1 *path, si1 *md_path);
si8		find_record_index_m11(FILE_PROCESSING_STRUCT_m11 *record_indices_fps, si8 target_time, ui4 mode, si8 low_idx);
si8     	frame_number_for_uutc_m11(LEVEL_HEADER_m11 *level_header, si8 target_uutc, ui4 mode, ...);  // varargs: si8 ref_frame_number, si8 ref_uutc, sf8 frame_rate
void            free_channel_m11(CHANNEL_m11* channel, TERN_m11 free_channel_structure);
void            free_globals_m11(TERN_m11 cleanup_for_exit);
void            free_segment_m11(SEGMENT_m11 *segment, TERN_m11 free_segment_structure);
void            free_session_m11(SESSION_m11 *session, TERN_m11 free_session_structure);
void		frequencies_vary_m11(SESSION_m11 *sess);
si1		**generate_file_list_m11(si1 **file_list, si4 *n_files, si1 *enclosing_directory, si1 *name, si1 *extension, ui4 flags);
si1		*generate_hex_string_m11(ui1 *bytes, si4 num_bytes, si1 *string);
ui4             generate_MED_path_components_m11(si1 *path, si1 *MED_dir, si1* MED_name);
si8             generate_recording_time_offset_m11(si8 recording_start_time_uutc);
si1		*generate_segment_name_m11(FILE_PROCESSING_STRUCT_m11 *fps, si1 *segment_name);
ui8             generate_UID_m11(ui8 *uid);
CHANNEL_m11	*get_active_channel_m11(SESSION_m11 *sess, si1 channel_type);
ui1		get_cpu_endianness_m11(void);
ui4		get_level_m11(si1 *full_file_name, ui4 *input_type_code);
LOCATION_INFO_m11	*get_location_info_m11(LOCATION_INFO_m11 *loc_info, TERN_m11 set_timezone_globals, TERN_m11 prompt);
si4		get_search_mode_m11(TIME_SLICE_m11 *slice);
si4		get_segment_index_m11(si4 segment_number);
si4             get_segment_range_m11(LEVEL_HEADER_m11 *level_header, TIME_SLICE_m11 *slice);
ui4		*get_segment_video_start_frames_m11(FILE_PROCESSING_STRUCT_m11 *video_indices_fps, ui4 *number_of_video_files);
si1		*get_session_directory_m11(si1 *session_directory, si1 *MED_file_name, FILE_PROCESSING_STRUCT_m11 *MED_fps);
TERN_m11	include_record_m11(ui4 type_code, si4 *record_filters);
TERN_m11	initialize_globals_m11(void);
TERN_m11	initialize_medlib_m11(TERN_m11 check_structure_alignments, TERN_m11 initialize_all_tables);
TIME_SLICE_m11	*initialize_time_slice_m11(TIME_SLICE_m11 *slice);
TERN_m11	initialize_timezone_tables_m11(void);
void		initialize_universal_header_m11(FILE_PROCESSING_STRUCT_m11 *fps, ui4 type_code, TERN_m11 generate_file_UID, TERN_m11 originating_file);
si8		items_for_bytes_m11(FILE_PROCESSING_STRUCT_m11 *fps, si8 *number_of_bytes);
void		lh_set_directives_m11(si1 *full_file_name, ui8 lh_flags, TERN_m11 *mmap_flag, TERN_m11 *close_flag, si8 *number_of_items);
si1		*MED_type_string_from_code_m11(ui4 code);
ui4             MED_type_code_from_string_m11(si1 *string);
TERN_m11	memory_map_read_m11(FILE_PROCESSING_STRUCT_m11 *fps, si8 file_offset, si8 bytes_to_read, ui4 behavior_on_fail);
TERN_m11        merge_metadata_m11(FILE_PROCESSING_STRUCT_m11 *md_fps_1, FILE_PROCESSING_STRUCT_m11 *md_fps_2, FILE_PROCESSING_STRUCT_m11 *merged_md_fps);
TERN_m11        merge_universal_headers_m11(FILE_PROCESSING_STRUCT_m11 *fps_1, FILE_PROCESSING_STRUCT_m11 *fps_2, FILE_PROCESSING_STRUCT_m11 *merged_fps);
void    	message_m11(si1 *fmt, ...);
void     	nap_m11(si1 *nap_str);
si1		*numerical_fixed_width_string_m11(si1 *string, si4 string_bytes, si4 number);
CHANNEL_m11	*open_channel_m11(CHANNEL_m11 *chan, TIME_SLICE_m11 *slice, si1 *channel_path, ui8 flags, si1 *password);
LEVEL_HEADER_m11	*open_level_m11(LEVEL_HEADER_m11 *lh, TIME_SLICE_m11 *slice, ...);  // varargs (lh == NULL): void *file_list, si4 list_len, ui8 flags, si1 *password
SEGMENT_m11	*open_segment_m11(SEGMENT_m11 *seg, TIME_SLICE_m11 *slice, si1 *segment_path, ui8 flags, si1 *password);
SESSION_m11	*open_session_m11(SESSION_m11 *sess, TIME_SLICE_m11 *slice, void *file_list, si4 list_len, ui8 flags, si1 *password);
si8             pad_m11(ui1 *buffer, si8 content_len, ui4 alignment);
TERN_m11	path_from_root_m11(si1 *path, si1 *root_path);
void            pop_behavior_m11(void);
TERN_m11	process_password_data_m11(FILE_PROCESSING_STRUCT_m11 *fps, si1 *unspecified_pw);
void		propogate_flags_m11(LEVEL_HEADER_m11 *level_header, ui8 new_flags);
void            push_behavior_m11(ui4 behavior);
CHANNEL_m11	*read_channel_m11(CHANNEL_m11 *chan, TIME_SLICE_m11 *slice, ...);  // varargs: si1 *chan_path, ui8 flags, si1 *password
LEVEL_HEADER_m11	*read_data_m11(LEVEL_HEADER_m11 *level_header, TIME_SLICE_m11 *slice, ...);  // varargs (level_header == NULL): si1 *file_list, si4 list_len, ui8 flags, si1 *password
FILE_PROCESSING_STRUCT_m11	*read_file_m11(FILE_PROCESSING_STRUCT_m11 *fps, si1 *full_file_name, si8 file_offset, si8 bytes_to_read, si8 number_of_items, LEVEL_HEADER_m11 *lh, si1 *password, ui4 behavior_on_fail);
SEGMENT_m11	*read_segment_m11(SEGMENT_m11 *seg, TIME_SLICE_m11 *slice, ...);  // varargs: si1 *seg_path, ui8 flags, si1 *password
SESSION_m11	*read_session_m11(SESSION_m11 *sess, TIME_SLICE_m11 *slice, ...);  // varargs: void *file_list, si4 list_len, ui8 flags, si1 *password
si8     	read_record_data_m11(LEVEL_HEADER_m11 *level_header, TIME_SLICE_m11 *slice, ...);  // varargs: si4 seg_num
si8     	read_time_series_data_m11(SEGMENT_m11 *seg, TIME_SLICE_m11 *slice);
TERN_m11	recover_passwords_m11(si1 *L3_password, UNIVERSAL_HEADER_m11* universal_header);
void     	remove_recording_time_offset_m11(si8 *time);
si8		sample_number_for_uutc_m11(LEVEL_HEADER_m11 *level_header, si8 target_uutc, ui4 mode, ...);  // varargs: si8 ref_sample_number, si8 ref_uutc, sf8 sampling_frequency
si4		search_Sgmt_records_m11(Sgmt_RECORD_m11 *Sgmt_records, TIME_SLICE_m11 *slice, ui4 search_mode);
si4		segment_for_frame_number_m11(LEVEL_HEADER_m11 *level_header, si8 target_sample);
si4		segment_for_sample_number_m11(LEVEL_HEADER_m11 *level_header, si8 target_sample);
si4		segment_for_uutc_m11(LEVEL_HEADER_m11 *level_header, si8 target_time);
void		set_error_m11(const si4 err_code, const si1 *function, const si4 line);
TERN_m11	set_global_time_constants_m11(TIMEZONE_INFO_m11 *timezone_info, si8 session_start_time, TERN_m11 prompt);
TERN_m11	set_time_and_password_data_m11(si1 *unspecified_password, si1 *MED_directory, si1 *metadata_section_2_encryption_level, si1 *metadata_section_3_encryption_level);
void		show_behavior_m11(void);
void            show_daylight_change_code_m11(DAYLIGHT_TIME_CHANGE_CODE_m11 *code, si1 *prefix);
void		show_file_times_m11(FILE_TIMES_m11 *ft);
void            show_globals_m11(void);
void		show_level_header_flags_m11(ui8	flags);
void            show_metadata_m11(FILE_PROCESSING_STRUCT_m11 *fps, METADATA_m11 *md, ui4 type_code);
void            show_password_data_m11(PASSWORD_DATA_m11 *pwd);
void		show_password_hints_m11(PASSWORD_DATA_m11 *pwd);
void		show_records_m11(FILE_PROCESSING_STRUCT_m11 *record_data_fps, si4 *record_filters);
void		show_Sgmt_records_array_m11(LEVEL_HEADER_m11 *level_header, Sgmt_RECORD_m11 *Sgmt);
void    	show_time_slice_m11(TIME_SLICE_m11 *slice);
void            show_timezone_info_m11(TIMEZONE_INFO_m11 *timezone_entry, TERN_m11 show_DST_detail);
void            show_universal_header_m11(FILE_PROCESSING_STRUCT_m11 *fps, UNIVERSAL_HEADER_m11 *uh);
TERN_m11	sort_channels_by_acq_num_m11(SESSION_m11 *sess);
si1		*time_string_m11(si8 uutc_time, si1 *time_str, TERN_m11 fixed_width, TERN_m11 relative_days, si4 colored_text, ...);
void            unescape_chars_m11(si1 *string, si1 target_char);
si8		uutc_for_frame_number_m11(LEVEL_HEADER_m11 *level_header, si8 target_frame_number, ui4 mode, ...);  // varargs: si8 ref_frame_number, si8 ref_uutc, sf8 frame_rate
si8		uutc_for_sample_number_m11(LEVEL_HEADER_m11 *level_header, si8 target_sample_number, ui4 mode, ...);  // varargs: si8 ref_smple_number, si8 ref_uutc, sf8 sampling_frequency
TERN_m11        validate_record_data_CRCs_m11(FILE_PROCESSING_STRUCT_m11 *fps);
TERN_m11        validate_time_series_data_CRCs_m11(FILE_PROCESSING_STRUCT_m11 *fps);
void            warning_message_m11(si1 *fmt, ...);
si1		*wchar2char_m11(si1 *target, wchar_t *source);



//**********************************************************************************//
//***************************  Windows-specific Functions  *************************//
//**********************************************************************************//

#ifdef WINDOWS_m11
FILETIME	uutc_to_win_time_m11(si8 uutc);
void		win_cleanup_m11(void);
void		win_clear_m11(void);
si8		win_DATE_to_uutc_m11(sf8 date);
si4		win_ls_1d_to_tmp_m11(si1 **dir_strs, si4 n_dirs, TERN_m11 full_path);
TERN_m11	win_initialize_terminal_m11(void);
TERN_m11	win_reset_terminal_m11(void);
TERN_m11	win_socket_startup_m11(void);
inline si4	win_system_m11(si1 *command);
si8		win_time_to_uutc_m11(FILETIME win_time);
sf8		win_uutc_to_DATE_m11(si8 uutc);
void		windify_file_paths_m11(si1 *target, si1 *source);
si1		*windify_format_string_m11(si1 *fmt);
#endif



//**********************************************************************************//
//*********************  MED AT Functions (allocation tracking)  *******************//
//**********************************************************************************//

#define AT_CURRENTLY_ALLOCATED_m11	((ui4) 1)
#define AT_PREVIOUSLY_FREED_m11		((ui4) 2)
#define AT_ALL_m11			(AT_CURRENTLY_ALLOCATED_m11 | AT_PREVIOUSLY_FREED_m11)

void		AT_add_entry_m11(void *address, const si1 *function);
ui8		AT_alloc_size_m11(void *address);
void		AT_free_all_m11(void);
TERN_m11	AT_freeable_m11(void *address);
void		AT_mutex_off(void);
void		AT_mutex_on(void);
TERN_m11 	AT_remove_entry_m11(void *address, const si1 *function);
void		AT_show_entries_m11(ui4 entry_type);
void		AT_show_entry_m11(void *address);
TERN_m11 	AT_update_entry_m11(void *orig_address, void *new_address, const si1 *function);



//**********************************************************************************//
//*******************************  MED FPS Functions  ******************************//
//**********************************************************************************//

// Prototypes
FILE_PROCESSING_STRUCT_m11	*FPS_allocate_processing_struct_m11(FILE_PROCESSING_STRUCT_m11 *fps, si1 *full_file_name, ui4 type_code, si8 raw_data_bytes, LEVEL_HEADER_m11 *parent, FILE_PROCESSING_STRUCT_m11 *proto_fps, si8 bytes_to_copy);
void		FPS_close_m11(FILE_PROCESSING_STRUCT_m11 *fps);
si4		FPS_compare_start_times_m11(const void *a, const void *b);
void            FPS_free_processing_struct_m11(FILE_PROCESSING_STRUCT_m11 *fps, TERN_m11 free_fps_structure);
FPS_DIRECTIVES_m11	*FPS_initialize_directives_m11(FPS_DIRECTIVES_m11 *directives);
FPS_PARAMETERS_m11	*FPS_initialize_parameters_m11(FPS_PARAMETERS_m11 *parameters);
TERN_m11	FPS_lock_m11(FILE_PROCESSING_STRUCT_m11 *fps, si4 lock_type, const si1 *function, ui4 behavior_on_fail);
si8		FPS_memory_map_read_m11(FILE_PROCESSING_STRUCT_m11 *fps, si8 file_offset, si8 bytes_to_read, const si1 *function, ui4 behavior_on_fail);
void		FPS_mutex_off_m11(FILE_PROCESSING_STRUCT_m11 *fps);
void		FPS_mutex_on_m11(FILE_PROCESSING_STRUCT_m11 *fps);
TERN_m11	FPS_open_m11(FILE_PROCESSING_STRUCT_m11 *fps, const si1 *function, ui4 behavior_on_fail);
si8		FPS_read_m11(FILE_PROCESSING_STRUCT_m11 *fps, si8 file_offset, si8 bytes_to_read, const si1 *function, ui4 behavior_on_fail);
TERN_m11	FPS_reallocate_processing_struct_m11(FILE_PROCESSING_STRUCT_m11 *fps, si8 raw_data_bytes);
void		FPS_seek_m11(FILE_PROCESSING_STRUCT_m11 *fps, si8 file_offset);
void		FPS_set_pointers_m11(FILE_PROCESSING_STRUCT_m11 *fps, si8 file_offset);
void		FPS_show_processing_struct_m11(FILE_PROCESSING_STRUCT_m11 *fps);
void		FPS_sort_m11(FILE_PROCESSING_STRUCT_m11 **fps_array, si4 n_fps);
si4		FPS_unlock_m11(FILE_PROCESSING_STRUCT_m11 *fps, const si1 *function, ui4 behavior_on_fail);



//**********************************************************************************//
//*******************************  MED STR Functions  ******************************//
//**********************************************************************************//

// Prototypes
si4		STR_compare_m11(const void *a, const void *b);
TERN_m11	STR_contains_regex_m11(si1 *string);
si1		*STR_match_end_m11(si1 *pattern, si1 *buffer);
si1		*STR_match_line_end_m11(si1 *pattern, si1 *buffer);
si1		*STR_match_line_start_m11(si1 *pattern, si1 *buffer);
si1		*STR_match_start_m11(si1 *pattern, si1 *buffer);
void    	STR_replace_char_m11(si1 c, si1 new_c, si1 *buffer);
si1		*STR_replace_pattern_m11(si1 *pattern, si1 *new_pattern, si1 *buffer, TERN_m11 free_input_buffer);
void		STR_sort_m11(si1 **string_array, si8 n_strings);
void		STR_strip_character_m11(si1 *s, si1 character);
void		STR_to_lower_m11(si1 *s);
void		STR_to_title_m11(si1 *s);
void		STR_to_upper_m11(si1 *s);


//***********************************************************************//
//*****************  MED VERSIONS OF STANDARD FUNCTIONS  ****************//
//***********************************************************************//


si4		asprintf_m11(si1 **target, si1 *fmt, ...);
void		*calloc_m11(size_t n_members, size_t el_size, const si1 *function, ui4 behavior_on_fail);
void		**calloc_2D_m11(size_t dim1, size_t dim2, size_t el_size, const si1 *function, ui4 behavior_on_fail);
size_t		calloc_size_m11(void *address, size_t element_size);
void		exit_m11(si4 status);
FILE		*fopen_m11(si1 *path, si1 *mode, const si1 *function, ui4 behavior_on_fail);
si4     	fprintf_m11(FILE *stream, si1 *fmt, ...);
si4		fputc_m11(si4 c, FILE *stream);
size_t          fread_m11(void *ptr, size_t el_size, size_t n_members, FILE *stream, si1 *path, const si1 *function, ui4 behavior_on_fail);
void            free_m11(void *ptr, const si1 *function);
void            free_2D_m11(void **ptr, size_t dim1, const si1 *function);
si4     	fscanf_m11(FILE *stream, si1 *fmt, ...);
si4             fseek_m11(FILE *stream, si8 offset, si4 whence, si1 *path, const si1 *function, ui4 behavior_on_fail);
si8            	ftell_m11(FILE *stream, const si1 *function, ui4 behavior_on_fail);
size_t		fwrite_m11(void *ptr, size_t el_size, size_t n_members, FILE *stream, si1 *path, const si1 *function, ui4 behavior_on_fail);
char		*getcwd_m11(char *buf, size_t size);
void		*malloc_m11(size_t n_bytes, const si1 *function, ui4 behavior_on_fail);
void		**malloc_2D_m11(size_t dim1, size_t dim2, size_t el_size, const si1 *function, ui4 behavior_on_fail);
size_t		malloc_size_m11(void *address);
void		memset_m11(void *ptr, const void *pattern, size_t pat_len, size_t n_members);
TERN_m11	mlock_m11(void *addr, size_t len, TERN_m11 zero_data, const si1 *function, ui4 behavior_on_fail);
TERN_m11	munlock_m11(void *addr, size_t len, const si1 *function, ui4 behavior_on_fail);
si4     	printf_m11(si1 *fmt, ...);
si4		putc_m11(si4 c, FILE *stream);
si4		putch_m11(si4 c);
si4		putchar_m11(si4 c);
void		*realloc_m11(void *ptr, size_t n_bytes, const si1 *function, ui4 behavior_on_fail);
void		**realloc_2D_m11(void **curr_ptr, size_t curr_dim1, size_t new_dim1, size_t curr_dim2, size_t new_dim2, size_t el_size, const si1 *function, ui4 behavior_on_fail);
void		*recalloc_m11(void *orig_ptr, size_t curr_bytes, size_t new_bytes, const si1 *function, ui4 behavior_on_fail);
si4     	scanf_m11(si1 *fmt, ...);
si4     	sprintf_m11(si1 *target, si1 *fmt, ...);
si4		snprintf_m11(si1 *target, si4 target_field_bytes, si1 *fmt, ...);
si4     	sscanf_m11(si1 *target, si1 *fmt, ...);
si8		strcat_m11(si1 *target, si1 *source);
si8		strcpy_m11(si1 *target, si1 *source);
si8		strncat_m11(si1 *target, si1 *source, si4 target_field_bytes);
si8		strncpy_m11(si1 *target, si1 *source, si4 target_field_bytes);
si4             system_m11(si1 *command, TERN_m11 null_std_streams, const si1 *function, ui4 behavior_on_fail);
si4		vasprintf_m11(si1 **target, si1 *fmt, va_list args);
si4		vfprintf_m11(FILE *stream, si1 *fmt, va_list args);
si4		vprintf_m11(si1 *fmt, va_list args);
si4		vsnprintf_m11(si1 *target, si4 target_field_bytes, si1 *fmt, va_list args);
si4    		vsprintf_m11(si1 *target, si1 *fmt, va_list args);



//**********************************************************************************//
//************************  CMP (COMPRESSION / COMPUTATION)  ***********************//
//**********************************************************************************//

// CMP: Miscellaneous Constants
#define CMP_SAMPLE_VALUE_NO_ENTRY_m11				NAN_SI4_m11
#define CMP_SPLINE_TAIL_LEN_m11                			6
#define CMP_SPLINE_UPSAMPLE_SF_RATIO_m11			((sf8) 3.0)
#define CMP_MAK_PAD_SAMPLES_m11					3
#define CMP_MAK_INPUT_BUFFERS_m11				8
#define CMP_MAK_IN_Y_BUF					0
#define CMP_MAK_IN_X_BUF					1
#define CMP_MAK_OUTPUT_BUFFERS_m11				4
#define CMP_MAK_OUT_Y_BUF					0
#define CMP_MAK_OUT_X_BUF					1
#define CMP_VDS_INPUT_BUFFERS_m11				(CMP_MAK_INPUT_BUFFERS_m11 + 1)
#define CMP_VDS_OUTPUT_BUFFERS_m11				CMP_MAK_OUTPUT_BUFFERS_m11
#define CMP_VDS_LOWPASS_ORDER_m11				6
#define CMP_VDS_MINIMUM_SAMPLES_m11				10
#define CMP_SELF_MANAGED_MEMORY_m11				-1  // pass CMP_SELF_MANAGED_MEMORY_m11 to CMP_allocate_processing_struct to prevent automatic re-allocation

// CMP: Block Fixed Header Offset Constants
#define CMP_BLOCK_FIXED_HEADER_BYTES_m11                        56                              // fixed region only
#define CMP_BLOCK_START_UID_m11                                 ((ui8) 0x0123456789ABCDEF)      // ui8   (decimal 81,985,529,216,486,895)
#define CMP_BLOCK_START_UID_OFFSET_m11				0
#define CMP_BLOCK_CRC_OFFSET_m11                                8                               // ui4
#define CMP_BLOCK_CRC_NO_ENTRY_m11                              CRC_NO_ENTRY_m11
#define CMP_BLOCK_BLOCK_FLAGS_OFFSET_m11			12                              // ui4
#define CMP_BLOCK_BLOCK_FLAGS_NO_ENTRY_m11			0
#define CMP_BLOCK_CRC_START_OFFSET_m11                          CMP_BLOCK_BLOCK_FLAGS_OFFSET_m11
#define CMP_BLOCK_START_TIME_OFFSET_m11                         16                              // si8
#define CMP_BLOCK_START_TIME_NO_ENTRY_m11                       UUTC_NO_ENTRY_m11
#define CMP_BLOCK_ACQUISITION_CHANNEL_NUMBER_OFFSET_m11         24                              // si4
#define CMP_BLOCK_ACQUISITION_CHANNEL_NUMBER_NO_ENTRY_m11       -1
#define CMP_BLOCK_TOTAL_BLOCK_BYTES_OFFSET_m11			28                              // ui4
#define CMP_BLOCK_TOTAL_BLOCK_BYTES_NO_ENTRY_m11		0
// CMP Block Encryption Start
#define CMP_BLOCK_NUMBER_OF_SAMPLES_OFFSET_m11                  32                              // ui4
#define CMP_BLOCK_ENCRYPTION_START_OFFSET_m11			CMP_BLOCK_NUMBER_OF_SAMPLES_OFFSET_m11
#define CMP_BLOCK_NUMBER_OF_SAMPLES_NO_ENTRY_m11                0xFFFFFFFF
#define CMP_BLOCK_NUMBER_OF_RECORDS_OFFSET_m11                  36                              // ui2
#define CMP_BLOCK_NUMBER_OF_RECORDS_NO_ENTRY_m11                0xFFFF
#define CMP_BLOCK_RECORD_REGION_BYTES_OFFSET_m11		38                              // ui2
#define CMP_BLOCK_RECORD_REGION_BYTES_NO_ENTRY_m11		0xFFFF
#define CMP_BLOCK_PARAMETER_FLAGS_OFFSET_m11			40                              // ui4
#define CMP_BLOCK_PARAMETER_FLAGS_NO_ENTRY_m11			0
#define CMP_BLOCK_PARAMETER_REGION_BYTES_OFFSET_m11		44				// ui2
#define CMP_BLOCK_PARAMETER_REGION_BYTES_NO_ENTRY_m11           0xFFFF
#define CMP_BLOCK_PROTECTED_REGION_BYTES_OFFSET_m11             46                              // ui2
#define CMP_BLOCK_PROTECTED_REGION_BYTES_NO_ENTRY_m11           0xFFFF
#define CMP_BLOCK_DISCRETIONARY_REGION_BYTES_OFFSET_m11         48                              // ui2
#define CMP_BLOCK_DISCRETIONARY_REGION_BYTES_NO_ENTRY_m11       0xFFFF
#define CMP_BLOCK_MODEL_REGION_BYTES_OFFSET_m11                 50                              // ui2
#define CMP_BLOCK_MODEL_REGION_BYTES_NO_ENTRY_m11               0xFFFF
#define CMP_BLOCK_TOTAL_HEADER_BYTES_OFFSET_m11			52                              // ui4
#define CMP_BLOCK_TOTAL_HEADER_BYTES_NO_ENTRY_m11            	0xFFFF
#define CMP_BLOCK_RECORDS_REGION_OFFSET_m11			56

// CMP: Record Header Offset Constants
#define CMP_RECORD_HEADER_BYTES_m11                        	8
#define CMP_RECORD_HEADER_TYPE_CODE_OFFSET_m11                  0				// ui4
#define CMP_RECORD_HEADER_TYPE_CODE_NO_ENTRY_m11		0
#define CMP_RECORD_HEADER_VERSION_MAJOR_OFFSET_m11		4				// ui1
#define CMP_RECORD_HEADER_VERSION_MAJOR_NO_ENTRY_m11		0xFF
#define CMP_RECORD_HEADER_VERSION_MINOR_OFFSET_m11		5				// ui1
#define CMP_RECORD_HEADER_VERSION_MINOR_NO_ENTRY_m11		0xFF
#define CMP_RECORD_HEADER_TOTAL_BYTES_OFFSET_m11		6				// ui2
#define CMP_RECORD_HEADER_TOTAL_BYTES_NO_ENTRY_m11		0xFFFF				// Note maximum CMP record size is 65k - smaller than MED record

// CMP: RED (Range Encoded Derivatives) Model Offset Constants
#define CMP_RED_MODEL_NUMBER_OF_KEYSAMPLE_BYTES_OFFSET_m11             	0                       // ui4
#define CMP_RED_MODEL_DERIVATIVE_LEVEL_OFFSET_m11                  	4                       // ui1
#define CMP_RED_MODEL_PAD_BYTES_OFFSET_m11                  		5                       // ui1[3]
#define CMP_RED_MODEL_NUMBER_OF_STATISTICS_BINS_OFFSET_m11              8                      	// ui2
#define CMP_RED_MODEL_FLAGS_OFFSET_m11                  		10                      // ui2
#define CMP_RED_MODEL_FIXED_HEADER_BYTES_m11                            12
// RED Model Flags
#define CMP_RED_FLAGS_NO_ZERO_COUNTS_MASK_m11				((ui2) 1)       	// bit 0
#define CMP_RED_FLAGS_POSITIVE_DERIVATIVES_MASK_m11			((ui2) 1 << 1)       	// bit 1
#define CMP_RED_2_BYTE_OVERFLOWS_MASK_m11				((ui2) 1 << 2)		// bit 2
#define CMP_RED_3_BYTE_OVERFLOWS_MASK_m11				((ui2) 1 << 3)		// bit 3
#define CMP_RED_OVERFLOW_BYTES_MASK_m11					(CMP_RED_2_BYTE_OVERFLOWS_MASK_m11 | CMP_RED_3_BYTE_OVERFLOWS_MASK_m11)

// CMP: PRED (Predictive RED) Model Offset Constants
#define CMP_PRED_MODEL_NUMBER_OF_KEYSAMPLE_BYTES_OFFSET_m11            	0                       // ui4
#define CMP_PRED_MODEL_DERIVATIVE_LEVEL_OFFSET_m11                 	4                       // ui1
#define CMP_PRED_MODEL_PAD_BYTES_OFFSET_m11                  		5                       // ui1[3]
#define CMP_PRED_MODEL_NUMBERS_OF_STATISTICS_BINS_OFFSET_m11            8			// ui2[3]
#define CMP_PRED_MODEL_NUMBER_OF_NIL_STATISTICS_BINS_OFFSET_m11         CMP_PRED_MODEL_NUMBERS_OF_STATISTICS_BINS_OFFSET_m11  // ui2
#define CMP_PRED_MODEL_NUMBER_OF_POS_STATISTICS_BINS_OFFSET_m11         10                      // ui2
#define CMP_PRED_MODEL_NUMBER_OF_NEG_STATISTICS_BINS_OFFSET_m11         12                      // ui2
#define CMP_PRED_MODEL_FLAGS_OFFSET_m11                  		14                      // ui2
#define CMP_PRED_MODEL_FIXED_HEADER_BYTES_m11                           16
// PRED Model Flags
#define CMP_PRED_FLAGS_NO_ZERO_COUNTS_MASK_m11				((ui2) 1)       	// bit 0
#define CMP_PRED_FLAGS_BIT_1_m11					((ui2) 1 << 1)       	// bit 1  Note: this is used for positive derivatives in RED, left empty here to keep bits same
#define CMP_PRED_2_BYTE_OVERFLOWS_MASK_m11				((ui2) 1 << 2)		// bit 2
#define CMP_PRED_3_BYTE_OVERFLOWS_MASK_m11				((ui2) 1 << 3)		// bit 3
#define CMP_PRED_OVERFLOW_BYTES_MASK_m11				(CMP_PRED_2_BYTE_OVERFLOWS_MASK_m11 | CMP_PRED_3_BYTE_OVERFLOWS_MASK_m11)

// CMP: MBE (Minimal Bit Encoding) Model Offset Constants
#define CMP_MBE_MODEL_MINIMUM_VALUE_OFFSET_m11                   	0			// si4
#define CMP_MBE_MODEL_BITS_PER_SAMPLE_OFFSET_m11                        4                       // ui1
#define CMP_MBE_MODEL_DERIVATIVE_LEVEL_OFFSET_m11                 	5                       // ui1
#define CMP_MBE_MODEL_FLAGS_OFFSET_m11                           	6			// ui2
#define CMP_MBE_MODEL_FIXED_HEADER_BYTES_m11                            8
// MBE Model Flags
#define CMP_MBE_FLAGS_PREPROCESSED_MASK_m11				((ui2) 1)       	// bit 0 - message to MBE_encode()) it will clear it

// CMP: VDS (Vectorized Data Stream) Model Offset Constants
#define CMP_VDS_MODEL_NUMBER_OF_VDS_SAMPLES_OFFSET_m11            	0                       // ui4
#define CMP_VDS_MODEL_AMPLITUDE_BLOCK_TOTAL_BYTES_OFFSET_m11            4                       // ui4
#define CMP_VDS_MODEL_AMPLITUDE_BLOCK_MODEL_BYTES_OFFSET_m11            8                       // ui2
#define CMP_VDS_MODEL_TIME_BLOCK_MODEL_BYTES_OFFSET_m11            	10                      // ui2
#define CMP_VDS_MODEL_FLAGS_OFFSET_m11                  		12                      // ui4  (more options for VDS)
#define CMP_VDS_MODEL_FIXED_HEADER_BYTES_m11                            16
// VDS Model Flags
#define CMP_VDS_FLAGS_AMPLITUDE_RED_MASK_m11				((ui4) 1)       	// bit 0
#define CMP_VDS_FLAGS_AMPLITUDE_PRED_MASK_m11				((ui4) 1 << 1)       	// bit 1
#define CMP_VDS_FLAGS_AMPLITUDE_MBE_MASK_m11				((ui4) 1 << 2)		// bit 2
#define CMP_VDS_FLAGS_TIME_RED_MASK_m11					((ui4) 1 << 5)		// bit 5
#define CMP_VDS_FLAGS_TIME_PRED_MASK_m11				((ui4) 1 << 6)		// bit 6
#define CMP_VDS_FLAGS_TIME_MBE_MASK_m11					((ui4) 1 << 7)		// bit 7
#define CMP_VDS_AMPLITUDE_ALGORITHMS_MASK_m11				(CMP_VDS_FLAGS_AMPLITUDE_RED_MASK_m11 | CMP_VDS_FLAGS_AMPLITUDE_PRED_MASK_m11 | CMP_VDS_FLAGS_AMPLITUDE_MBE_MASK_m11)
#define CMP_VDS_TIME_ALGORITHMS_MASK_m11				(CMP_VDS_FLAGS_TIME_RED_MASK_m11 | CMP_VDS_FLAGS_TIME_PRED_MASK_m11 | CMP_VDS_FLAGS_TIME_MBE_MASK_m11)
#define CMP_VDS_ALGORITHMS_MASK_m11					(CMP_VDS_AMPLITUDE_ALGORITHMS_MASK_m11 | CMP_VDS_TIME_ALGORITHMS_MASK_m11)

// CMP Block Flag Masks
#define CMP_BF_BLOCK_FLAG_BITS_m11			32
#define CMP_BF_DISCONTINUITY_MASK_m11			((ui4) 1)       	// bit 0
#define CMP_BF_LEVEL_1_ENCRYPTION_MASK_m11		((ui4) 1 << 4)		// bit 4
#define CMP_BF_LEVEL_2_ENCRYPTION_MASK_m11		((ui4) 1 << 5)		// bit 5
#define CMP_BF_RED_ENCODING_MASK_m11			((ui4) 1 << 8)		// bit 8
#define CMP_BF_PRED_ENCODING_MASK_m11			((ui4) 1 << 9)		// bit 9
#define CMP_BF_MBE_ENCODING_MASK_m11			((ui4) 1 << 10)		// bit 10
#define CMP_BF_VDS_ENCODING_MASK_m11			((ui4) 1 << 11)		// bit 11

#define CMP_BF_ALGORITHMS_MASK_m11			((ui4) (CMP_BF_RED_ENCODING_MASK_m11 | CMP_BF_PRED_ENCODING_MASK_m11 | \
							CMP_BF_MBE_ENCODING_MASK_m11 | CMP_BF_VDS_ENCODING_MASK_m11))
#define CMP_BF_ENCRYPTION_MASK_m11			((ui4) (CMP_BF_LEVEL_1_ENCRYPTION_MASK_m11 | CMP_BF_LEVEL_2_ENCRYPTION_MASK_m11))

// CMP Parameter Map Indices
#define CMP_PF_INTERCEPT_IDX_m11			((ui4) 0)       // bit 0
#define CMP_PF_GRADIENT_IDX_m11				((ui4) 1)	// bit 1
#define CMP_PF_AMPLITUDE_SCALE_IDX_m11			((ui4) 2)       // bit 2
#define CMP_PF_FREQUENCY_SCALE_IDX_m11			((ui4) 3)       // bit 3
#define CMP_PF_NOISE_SCORES_IDX_m11			((ui4) 4)	// bit 4

// CMP Parameter Flag Masks
#define CMP_PF_PARAMETER_FLAG_BITS_m11			32
#define CMP_PF_INTERCEPT_MASK_m11			((ui4) 1 << CMP_PF_INTERCEPT_IDX_m11)		// bit 0
#define CMP_PF_GRADIENT_MASK_m11			((ui4) 1 << CMP_PF_GRADIENT_IDX_m11)		// bit 1
#define CMP_PF_AMPLITUDE_SCALE_MASK_m11			((ui4) 1 << CMP_PF_AMPLITUDE_SCALE_IDX_m11)	// bit 2
#define CMP_PF_FREQUENCY_SCALE_MASK_m11			((ui4) 1 << CMP_PF_FREQUENCY_SCALE_IDX_m11)	// bit 3
#define CMP_PF_NOISE_SCORES_MASK_m11			((ui4) 1 << CMP_PF_NOISE_SCORES_IDX_m11)	// bit 4

// Compression Modes
#define CMP_COMPRESSION_MODE_NO_ENTRY_m11	((ui1) 0)
#define CMP_DECOMPRESSION_MODE_m11              ((ui1) 1)
#define CMP_COMPRESSION_MODE_m11                ((ui1) 2)

// Lossy Compression Modes
#define CMP_AMPLITUDE_SCALE_MODE_m11		((ui1) 1)
#define CMP_FREQUENCY_SCALE_MODE_m11		((ui1) 2)

// Compression Algorithms
#define CMP_RED_COMPRESSION_m11		CMP_BF_RED_ENCODING_MASK_m11
#define CMP_PRED_COMPRESSION_m11	CMP_BF_PRED_ENCODING_MASK_m11
#define CMP_MBE_COMPRESSION_m11		CMP_BF_MBE_ENCODING_MASK_m11
#define CMP_VDS_COMPRESSION_m11		CMP_BF_VDS_ENCODING_MASK_m11

// CMP Directives Defaults
#define CMP_DIRECTIVES_COMPRESSION_MODE_DEFAULT_m11			CMP_COMPRESSION_MODE_NO_ENTRY_m11
#define CMP_DIRECTIVES_ALGORITHM_DEFAULT_m11				CMP_PRED_COMPRESSION_m11
#define CMP_DIRECTIVES_ENCRYPTION_LEVEL_DEFAULT_m11			NO_ENCRYPTION_m11
#define CMP_DIRECTIVES_CPS_POINTER_RESET_DEFAULT_m11			TRUE_m11
#define CMP_DIRECTIVES_CPS_CACHING_DEFAULT_m11				TRUE_m11
#define CMP_DIRECTIVES_FALL_THROUGH_TO_BEST_ENCODING_DEFAULT_m11	TRUE_m11
#define CMP_DIRECTIVES_RESET_DISCONTINUITY_DEFAULT_m11			TRUE_m11
#define CMP_DIRECTIVES_INCLUDE_NOISE_SCORES_DEFAULT_m11			FALSE_m11
#define CMP_DIRECTIVES_NO_ZERO_COUNTS_DEFAULT_m11			FALSE_m11
#define CMP_DIRECTIVES_SET_OVERFLOW_BYTES_DEFAULT_m11			FALSE_m11  	// user sets value in parameters
#define CMP_DIRECTIVES_FIND_OVERFLOW_BYTES_DEFAULT_m11			TRUE_m11  	// determine overflow bytes on a block by block basis
#define CMP_DIRECTIVES_NO_ZERO_COUNTS_DEFAULT_m11          		FALSE_m11
#define CMP_DIRECTIVES_POSITIVE_DERIVATIVES_DEFAULT_m11    		FALSE_m11
#define CMP_DIRECTIVES_SET_DERIVATIVE_LEVEL_DEFAULT_m11			FALSE_m11	// user sets value in parameters
#define CMP_DIRECTIVES_FIND_DERIVATIVE_LEVEL_DEFAULT_m11		FALSE_m11
#define CMP_DIRECTIVES_CONVERT_TO_NATIVE_UNITS_DEFAULT_m11		TRUE_m11
// lossy compression directives
#define CMP_DIRECTIVES_DETREND_DATA_DEFAULT_m11				FALSE_m11
#define CMP_DIRECTIVES_REQUIRE_NORMALITY_DEFAULT_m11			FALSE_m11
#define CMP_DIRECTIVES_RETURN_LOSSY_DATA_DEFAULT_m11			FALSE_m11
#define CMP_DIRECTIVES_USE_COMPRESSION_RATIO_DEFAULT_m11		FALSE_m11
#define CMP_DIRECTIVES_USE_MEAN_RESIDUAL_RATIO_DEFAULT_m11		TRUE_m11
#define CMP_DIRECTIVES_USE_RELATIVE_RATIO_DEFAULT_m11			FALSE_m11
#define CMP_DIRECTIVES_SET_AMPLITUDE_SCALE_DEFAULT_m11			FALSE_m11	// user sets value in parameters
#define CMP_DIRECTIVES_FIND_AMPLITUDE_SCALE_DEFAULT_m11			FALSE_m11
#define CMP_DIRECTIVES_SET_FREQUENCY_SCALE_DEFAULT_m11			FALSE_m11	// user sets value in parameters
#define CMP_DIRECTIVES_FIND_FREQUENCY_SCALE_DEFAULT_m11			FALSE_m11
#define CMP_DIRECTIVES_VDS_SCALE_BY_BASELINE_DEFAULT_m11		FALSE_m11	// increases compression by ~15%

// CMP Parameters Defaults
#define CMP_PARAMETERS_NUMBER_OF_BLOCK_PARAMETERS_DEFAULT_m11	0
#define CMP_PARAMETERS_MINIMUM_SAMPLE_VALUE_DEFAULT_m11		CMP_SAMPLE_VALUE_NO_ENTRY_m11
#define CMP_PARAMETERS_MAXIMUM_SAMPLE_VALUE_DEFAULT_m11		CMP_SAMPLE_VALUE_NO_ENTRY_m11
#define CMP_PARAMETERS_DISCONTINUITY_DEFAULT_m11		UNKNOWN_m11
#define CMP_PARAMETERS_DERIVATIVE_LEVEL_DEFAULT_m11		((ui1) 1)
#define CMP_PARAMETERS_OVERFLOW_BYTES_DEFAULT_m11          	4
	// lossy compression parameters
#define CMP_PARAMETERS_GOAL_RATIO_DEFAULT_m11			((sf8) 0.05)
#define CMP_PARAMETERS_GOAL_TOLERANCE_DEFAULT_m11		((sf8) 0.005)
#define CMP_PARAMETERS_MAXIMUM_GOAL_ATTEMPTS_DEFAULT_m11	20
#define CMP_PARAMETERS_MINIMUM_NORMALITY_DEFAULT_m11		((ui1) 128)	// ronge 0-254 (low to high)
#define CMP_PARAMETERS_AMPLITUDE_SCALE_DEFAULT_m11		((sf4) 1.0)
#define CMP_PARAMETERS_FREQUENCY_SCALE_DEFAULT_m11		((sf4) 1.0)
#define CMP_PARAMETERS_VDS_THRESHOLD_DEFAULT_m11		((sf8) 5.0)	// generally an integer, but any float value is fine. Range 0.0 to 10.0; default == 5.0  (0.0 == lossless compression)
	// variable region parameters
#define CMP_USER_NUMBER_OF_RECORDS_DEFAULT_m11			((ui2) 0)
#define CMP_USER_RECORD_REGION_BYTES_DEFAULT_m11		((ui2) 0)
#define CMP_USER_PARAMETER_FLAGS_DEFAULT_m11			((ui4) 0)
#define CMP_PROTECTED_REGION_BYTES_DEFAULT_m11			((ui2) 0)
#define CMP_USER_DISCRETIONARY_REGION_BYTES_DEFAULT_m11		((ui2) 0)

// RED/PRED Codec Constants
#define CMP_SI1_KEYSAMPLE_FLAG_m11      	((si1) 0x80)		// -128 as si1
#define CMP_UI1_KEYSAMPLE_FLAG_m11      	((ui1) 0x80)		// +128 as ui1
#define CMP_POS_DERIV_KEYSAMPLE_FLAG_m11	((ui1) 0x00)		// no zero differences expected in positive derivative model
#define CMP_RED_TOTAL_COUNTS_m11        	((ui4) 0x10000)         // 2^16
#define CMP_RED_MAXIMUM_RANGE_m11       	((ui8) 0x1000000000000) // 2^48
#define CMP_RED_RANGE_MASK_m11          	((ui8) 0xFFFFFFFFFFFF)  // 2^48 - 1
#define CMP_RED_MAX_STATS_BINS_m11      	256
#define CMP_PRED_CATS_m11               	3
#define CMP_PRED_NIL_m11                	0
#define CMP_PRED_POS_m11                	1
#define CMP_PRED_NEG_m11			2

// Macros
#define CMP_MAX_KEYSAMPLE_BYTES_m11(block_samps)		(block_samps * 5) // full si4 plus 1 keysample flag byte per sample
#define CMP_MAX_COMPRESSED_BYTES_m11(block_samps, n_blocks)	(((block_samps * 4) + CMP_BLOCK_FIXED_HEADER_BYTES_m11 + 7) * n_blocks)	// (no compression + header + maximum pad bytes) for n_blocks blocks
																	// NOTE: does not take variable region bytes into account and assumes
																	// fallthrough to MBE
#define CMP_PRED_CAT_m11(x)					((x) ? (((x) & 0x80) ? CMP_PRED_NEG_m11 : CMP_PRED_POS_m11) : CMP_PRED_NIL_m11) // do not increment/decrement within call to CMP_PRED_CAT_m11
																		// as "x" is used twice
#define CMP_IS_DETRENDED_m11(block_header_ptr)			((block_header_ptr->parameter_flags & CMP_PF_INTERCEPT_MASK_m11) && (block_header_ptr->parameter_flags & CMP_PF_GRADIENT_MASK_m11))
#define CMP_VARIABLE_REGION_BYTES_v1_m11(block_header_ptr)	((ui4) (block_header_ptr)->record_region_bytes + (ui4) (block_header_ptr)->parameter_region_bytes + \
								(ui4) (block_header_ptr)->protected_region_bytes + (ui4) (block_header_ptr)->discretionary_region_bytes)
#define CMP_VARIABLE_REGION_BYTES_v2_m11(block_header_ptr)	((ui4) (block_header_ptr)->total_header_bytes - ((ui4) CMP_BLOCK_FIXED_HEADER_BYTES_m11 + \
								(ui4) (block_header_ptr)->model_region_bytes))

// Update CPS Pointer Flags
#define CMP_UPDATE_ORIGINAL_PTR_m11             ((ui1) 1)
#define CMP_RESET_ORIGINAL_PTR_m11             	((ui1) 2)
#define CMP_UPDATE_BLOCK_HEADER_PTR_m11         ((ui1) 4)
#define CMP_RESET_BLOCK_HEADER_PTR_m11         	((ui1) 8)
#define CMP_UPDATE_DECOMPRESSED_PTR_m11         ((ui1) 16)
#define CMP_RESET_DECOMPRESSED_PTR_m11         	((ui1) 32)

// Typedefs & Structures
typedef struct {  // requires 4-byte alignment
	ui4	number_of_keysample_bytes;
	ui1	derivative_level;
	ui1	pad[3];
	ui2	number_of_statistics_bins;
	ui2	flags;
} CMP_RED_MODEL_FIXED_HEADER_m11;

typedef struct {  // requires 4-byte alignment
	ui4	number_of_keysample_bytes;
	ui1	derivative_level;
	ui1	pad[3];
	union {
		ui2	numbers_of_statistics_bins[3];
		struct {
			ui2	number_of_nil_statistics_bins;
			ui2	number_of_pos_statistics_bins;
			ui2	number_of_neg_statistics_bins;
		};
	};
	ui2	flags;
} CMP_PRED_MODEL_FIXED_HEADER_m11;

typedef struct {  // requires 4-byte alignment
	si4	minimum_value;  // of highest derivative
	ui1	bits_per_sample;
	ui1	derivative_level;
	ui2	flags;
} CMP_MBE_MODEL_FIXED_HEADER_m11;

typedef struct {  // requires 4-byte alignment
	ui4	number_of_VDS_samples;
	ui4	amplitude_block_total_bytes;
	ui2	amplitude_block_model_bytes;
	ui2	time_block_model_bytes;
	ui4	flags;  // potentially more options for VDS, so 32 bits
} CMP_VDS_MODEL_FIXED_HEADER_m11;

typedef struct {
	ui4	type_code;  // note this is not null terminated and so cannot be treated as a string as in RECORD_HEADER_m11 structure
	ui1	version_major;
	ui1	version_minor;
	ui2	total_bytes;  // note maximum record size is 65535 - smaller than in RECORD_HEADER_m11 structure
} CMP_RECORD_HEADER_m11;

// CMP_BLOCK_FIXED_HEADER_m11 declared above

typedef struct {
	ui4     count;
	union {
		si1     value;
		ui1	pos_value;
	};
} CMP_STATISTICS_BIN_m11;

typedef struct {
	si8	n_buffers;
	si8 	n_elements;
	si8	element_size;
	void	**buffer;
	// used internally
	ui8		total_allocated_bytes;
	TERN_m11	locked;
} CMP_BUFFERS_m11;

typedef struct {
	si8		cache_offset;
	ui4		block_samples;
	si4		block_number;
	TERN_m11	data_read;
} CMP_CACHE_BLOCK_INFO_m11;

typedef struct NODE_STRUCT_m11 {
	si4                     val;
	ui4                     count;
	struct NODE_STRUCT_m11	*prev, *next;
} CMP_NODE_m11;

typedef struct {
	ui4             mode;  // CMP_COMPRESSION_MODE_m11, CMP_DECOMPRESSION_MODE_m11
	ui4             algorithm;  // RED, PRED, MBE, or VDS
	si1             encryption_level;  // encryption level for data blocks: passed in compression
	TERN_m11	cps_pointer_reset;  // FALSE to maunually control cps pointers
	TERN_m11        cps_caching;  // use for largely sequential reads, not random reads or open/close behavior
	TERN_m11        fall_through_to_best_encoding;  // if another encoding would be smaller than RED/PRED, use it for the block
	TERN_m11        reset_discontinuity;  // if discontinuity directive == TRUE_m11, reset to FALSE_m11 after compressing the block
	TERN_m11        include_noise_scores;  // a set of 4 metrics that measure different types of noise (range 0-254: 0 no noise, 254 max noise, 0xFF no entry)
	TERN_m11        no_zero_counts;  // in RED & PRED codecs (when blocks must be encoded with non-block statistics. This is a special case.)
	TERN_m11        set_derivative_level;  // value passed in "goal_derivative_level" parameter
	TERN_m11        find_derivative_level;  // mutually exclusive with "set_derivative_level"
	TERN_m11	set_overflow_bytes;  // value passed in "goal_overflow_bytes" parameter (range 2-4 bytes)
	TERN_m11	find_overflow_bytes;  // mutually exclusive with "set_overflow_bytes"
	TERN_m11	convert_to_native_units;  // use amplitude_units_conversion_factor to to convert to units of amplitude_units_description on read
	// lossy compression directives
	TERN_m11        detrend_data;  // lossless operation, but most useful for lossy compression.
	TERN_m11        require_normality;  // for lossy compression - use lossless if data amplitudes are too oddly distributed.  Pairs with "minimum_normality" parameter.
	TERN_m11        use_compression_ratio;  // used in "find" directives. Mutually exclusive with "use_mean_residual_ratio".
	TERN_m11        use_mean_residual_ratio;  // used in "find" directives. Mutually exclusive with "use_compression_ratio".
	TERN_m11        use_relative_ratio;  // divide goal ratio by the block coefficient of variation in lossy compression routines (more precision in blocks with higher variance)
	TERN_m11        set_amplitude_scale;  // value passed in "amplitude_scale" parameter
	TERN_m11        find_amplitude_scale;  // mutually exclusive with "set_amplitude_scale"
	TERN_m11        set_frequency_scale;  // value passed in "frequency_scale" parameter
	TERN_m11        find_frequency_scale;  // mutually exclusive with "set_frequency_scale"
	TERN_m11	VDS_scale_by_baseline;  // in VDS compression: scale data by baseline amplitude
} CMP_DIRECTIVES_m11;

// Parameters contain "mechanics" of CPS
typedef struct {
	pthread_mutex_t_m11	mutex;
	// cache parameters
	CMP_CACHE_BLOCK_INFO_m11	*cached_blocks;
	si4		cached_block_list_len;
	si4		cached_block_cnt;
	si4		*cache;
	// memory parameters
	si8		allocated_block_samples;
	si8		allocated_keysample_bytes;
	si8		allocated_compressed_bytes;  // == time series data fps: (raw_data_bytes - UNIVERSAL_HEADER_BYTES_m11)
	si8		allocated_decompressed_samples;
	// compression parameters
	ui1		goal_derivative_level;  // used with set_derivative_level directive
	ui1		derivative_level;  // goal/actual pairs because not always possible
	ui1		goal_overflow_bytes;  // used with set_overflow_bytes directive
	ui1		overflow_bytes;  // goal/actual pairs because not always possible
	// block parameters
	TERN_m11	discontinuity;  // set if block is first after a discontinuity, passed in compression, returned in decompression
	ui4		block_start_index;  // block relative
	ui4		block_end_index;  // block relative
	si4		number_of_block_parameters;
	ui4		block_parameter_map[CMP_PF_PARAMETER_FLAG_BITS_m11];
	si4		minimum_sample_value;  // found on compression, stored for general use (and MBE, if used)
	si4		maximum_sample_value;  // found on compression, stored for general use (and MBE, if used)
	si4		minimum_difference_value;
	si4		maximum_difference_value;
	ui2		user_number_of_records;  // set by user
	ui2		user_record_region_bytes;  // set by user to reserve bytes for records in block header
	ui4		user_parameter_flags;  // user bits to be set in parameter flags of block header (library flags will be set automatically)
	ui2		protected_region_bytes;  // not currently used, set to zero (allows for future expansion)
	ui2		user_discretionary_region_bytes;  // set by user to reserve bytes for discretionary region in header
	ui4		variable_region_bytes;  // value calculated and set by library based on parameters & directives
	ui4		number_of_derivative_bytes;  // values in derivative or difference buffer
	// lossy compression parameters
	sf8		goal_ratio;  // either compression ratio or mean residual ratio
	sf8		actual_ratio;  // either compression ratio or mean residual ratio
	sf8		goal_tolerance;  // tolerance for lossy compression mode goal, value of <= 0.0 uses default values, which are returned
	si4		maximum_goal_attempts;  // maximum loops to attain goal compression
	ui1		minimum_normality;  // range 0-254: 0 not normal, 254 perfectly normal, 0xFF no entry
	sf4		amplitude_scale;  // used with set_amplitude_scale directive
	sf4		frequency_scale;  // used with set_frequency_scale directive
	sf8		VDS_LFP_high_fc;  // lowpass filter cutoff for VDS encoding (0.0 for no filter)
	sf8		VDS_threshold;  // generally an integer, but float values work fine. Range 0.0 to 10.0; default == 5.0  (0.0 indicates lossless compression)
	sf8		VDS_sampling_frequency;  // used to preserve units during LFP filtering, if filtering is not specified, this is not used
	// compression arrays
	si1			*keysample_buffer;  // passed in both compression & decompression
	si4			*derivative_buffer;  // used if needed in compression & decompression, size of maximum block differences
	si4			*detrended_buffer;  // used if needed in compression, size of decompressed block
	si4			*scaled_amplitude_buffer;  // used if needed in compression, size of decompressed block
	si4			*scaled_frequency_buffer;  // used if needed in compression, size of decompressed block
	CMP_BUFFERS_m11		*scrap_buffers;  // generic use
	CMP_BUFFERS_m11		*VDS_input_buffers;
	CMP_BUFFERS_m11		*VDS_output_buffers;
	void			**filtps;
	si4			n_filtps;
	ui1			*model_region;
	void			*count;  // used by RED/PRED encode & decode (ui4 * or ui4 **)
	void			*sorted_count;  // used by RED/PRED encode & decode (CMP_STATISTICS_BIN_m11 * or CMP_STATISTICS_BIN_m11 **)
	void			*cumulative_count;  // used by RED/PRED encode & decode (ui8 * or ui8 **)
	void			*minimum_range;  // used by RED/PRED encode & decode (ui8 * or ui8 **)
	void			*symbol_map;  // used by RED/PRED encode & decode (ui1 * or ui1 **)
} CMP_PARAMETERS_m11;

typedef struct CMP_PROCESSING_STRUCT_m11 {
	CMP_DIRECTIVES_m11   		directives;
	CMP_PARAMETERS_m11  		parameters;
	si4				*input_buffer;  // pointer that is updated depending on processing options (e.g. points to detrended data, scaled daata, etc.)
	ui1				*compressed_data;  // points to base of FPS time_series_data array, NOT an allocated pointer => do not free; should not be updated
	CMP_BLOCK_FIXED_HEADER_m11	*block_header; // == FPS time_series_data; points to beginning of current block within compressed_data array, updatable
	si4				*decompressed_data;  // returned in decompression or if lossy data requested, used in some compression modes, should not be updated
	si4				*decompressed_ptr;  // points to beginning of current block within decompressed_data array, updatable
	si4				*original_data;  // passed in compression, should not be updated
	si4				*original_ptr;  // points to beginning of current block within original_data array, updatable
	ui1				*block_records;  // pointer beginning of records region of block header
	ui4				*block_parameters;  // pointer beginning of parameter region of block header
	ui1				*discretionary_region;
} CMP_PROCESSING_STRUCT_m11;


// Function Prototypes
CMP_BUFFERS_m11	*CMP_allocate_buffers_m11(CMP_BUFFERS_m11 *buffers, si8 n_buffers, si8 n_elements, si8 element_size, TERN_m11 zero_data, TERN_m11 lock_memory);
CMP_PROCESSING_STRUCT_m11	*CMP_allocate_processing_struct_m11(FILE_PROCESSING_STRUCT_m11 *fps, ui4 mode, si8 data_samples, si8 compressed_data_bytes, si8 keysample_bytes, ui4 block_samples, CMP_DIRECTIVES_m11 *directives, CMP_PARAMETERS_m11 *parameters);
TERN_m11	CMP_check_CPS_allocation_m11(FILE_PROCESSING_STRUCT_m11 *fps);
void		CMP_cps_mutex_off_m11(CMP_PROCESSING_STRUCT_m11 *cps);
void		CMP_cps_mutex_on_m11(CMP_PROCESSING_STRUCT_m11 *cps);
void    	CMP_decode_m11(FILE_PROCESSING_STRUCT_m11 *fps);
TERN_m11     	CMP_decrypt_m11(FILE_PROCESSING_STRUCT_m11 *fps);  // single block decrypt (see also decrypt_time_series_data_m11)
void    	CMP_free_buffers_m11(CMP_BUFFERS_m11 *buffers, TERN_m11 free_structure);
void    	CMP_free_processing_struct_m11(CMP_PROCESSING_STRUCT_m11 *cps, TERN_m11 free_cps_structure);
void		CMP_generate_parameter_map_m11(CMP_PROCESSING_STRUCT_m11 *cps);
ui1    		CMP_get_overflow_bytes_m11(CMP_PROCESSING_STRUCT_m11 *cps, ui4 mode, ui4 algorithm);
void    	CMP_get_variable_region_m11(CMP_PROCESSING_STRUCT_m11 *cps);
void		CMP_initialize_directives_m11(CMP_DIRECTIVES_m11 *directives, ui1 mode);
void		CMP_initialize_parameters_m11(CMP_PARAMETERS_m11 *parameters);
sf8		*CMP_lin_interp_sf8_m11(sf8 *in_data, si8 in_len, sf8 *out_data, si8 out_len);
si4		*CMP_lin_interp_si4_m11(si4 *in_data, si8 in_len, si4 *out_data, si8 out_len);
void		CMP_lock_buffers_m11(CMP_BUFFERS_m11 *buffers);
void    	CMP_MBE_decode_m11(CMP_PROCESSING_STRUCT_m11 *cps);
sf8     	*CMP_mak_interp_sf8_m11(CMP_BUFFERS_m11 *in_bufs, si8 in_len, CMP_BUFFERS_m11 *out_bufs, si8 out_len);
void    	CMP_offset_time_m11(CMP_BLOCK_FIXED_HEADER_m11 *block_header, si4 action);
void    	CMP_PRED_decode_m11(CMP_PROCESSING_STRUCT_m11 *cps);
CMP_PROCESSING_STRUCT_m11	*CMP_reallocate_processing_struct_m11(FILE_PROCESSING_STRUCT_m11 *fps, ui4 mode, si8 data_samples, ui4 block_samples);
void    	CMP_RED_decode_m11(CMP_PROCESSING_STRUCT_m11 *cps);
void    	CMP_retrend_si4_m11(si4 *in_y, si4 *out_y, si8 len, sf8 m, sf8 b);
void    	CMP_retrend_2_sf8_m11(sf8 *in_x, sf8 *in_y, sf8 *out_y, si8 len, sf8 m, sf8 b);
si2      	CMP_round_si2_m11(sf8 val);
si4     	CMP_round_si4_m11(sf8 val);
void      	CMP_sf8_to_si4_m11(sf8 *sf8_arr, si4 *si4_arr, si8 len);
void      	CMP_sf8_to_si4_and_scale_m11(sf8 *sf8_arr, si4 *si4_arr, si8 len, sf8 scale);
void    	CMP_show_block_header_m11(CMP_BLOCK_FIXED_HEADER_m11 *block_header);
void    	CMP_show_block_model_m11(CMP_PROCESSING_STRUCT_m11 *cps, TERN_m11 recursed_call);
void      	CMP_si4_to_sf8_m11(si4 *si4_arr, sf8 *sf8_arr, si8 len);
sf8		*CMP_spline_interp_sf8_m11(sf8 *in_data, si8 in_len, sf8 *out_data, si8 out_len, CMP_BUFFERS_m11 *spline_bufs);
si4		*CMP_spline_interp_si4_m11(si4 *in_data, si8 in_len, si4 *out_data, si8 out_len, CMP_BUFFERS_m11 *spline_bufs);
void		CMP_unlock_buffers_m11(CMP_BUFFERS_m11 *buffers);
void    	CMP_unscale_amplitude_si4_m11(si4 *input_buffer, si4 *output_buffer, si8 len, sf8 scale_factor);
void    	CMP_unscale_amplitude_sf8_m11(sf8 *input_buffer, sf8 *output_buffer, si8 len, sf8 scale_factor);
void    	CMP_unscale_frequency_si4_m11(si4 *input_buffer, si4 *output_buffer, si8 len, sf8 scale_factor);
CMP_BLOCK_FIXED_HEADER_m11 *CMP_update_CPS_pointers_m11(FILE_PROCESSING_STRUCT_m11 *fps, ui1 flags);
void		CMP_VDS_decode_m11(CMP_PROCESSING_STRUCT_m11 *cps);
void    	CMP_zero_buffers_m11(CMP_BUFFERS_m11 *buffers);



//**********************************************************************************//
//**************************************  CRC  *************************************//
//**********************************************************************************//

// ATTRIBUTION
//
// Basic CRC-32 manipulation routines
// by Mark Adler (madler@alumni.caltech.edu)
// placed in the public domain 29 Apr 2015
//
// "This library provides general CRC calculation & validation functions and an
// operation to combine the CRCs of two sequences of bytes into a single CRC.
// The routines in this library only work with the particular CRC-32 polynomial
// provided here."
//
// Minor modifications for compatibility with the MED Library.
// The CRC routines are customized for little endian machines.


// Constants
#define CRC_BYTES_m11           4
#define CRC_TABLES_m11          8
#define CRC_TABLE_ENTRIES_m11   256
#define CRC_POLYNOMIAL_m11      ((ui4) 0xEDB88320)    // note library CRC routines are customized to this polynomial, it cannot be changed arbitrarily
#define CRC_START_VALUE_m11     ((ui4) 0x0)

// CRC Modes
#define CRC_NO_ENTRY_m11                CRC_START_VALUE_m11
#define CRC_IGNORE_m11                  0
#define CRC_VALIDATE_m11                1
#define CRC_VALIDATE_ON_INPUT_m11       2
#define CRC_VALIDATE_ON_OUTPUT_m11      4
#define CRC_CALCULATE_m11               8
#define CRC_CALCULATE_ON_INPUT_m11      16
#define CRC_CALCULATE_ON_OUTPUT_m11     32

// Macros
#define CRC_SWAP32_m11(q)       ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + (((q) & 0xff00) << 8) + (((q) & 0xff) << 24))

// Function Prototypes
ui4		CRC_calculate_m11(const ui1 *block_ptr, si8 block_bytes);
ui4		CRC_combine_m11(ui4 block_1_crc, ui4 block_2_crc, si8 block_2_bytes);
TERN_m11	CRC_initialize_tables_m11(void);
void		CRC_matrix_square_m11(ui4 *square, const ui4 *mat);
ui4		CRC_matrix_times_m11(const ui4 *mat, ui4 vec);
ui4		CRC_update_m11(const ui1 *block_ptr, si8 block_bytes, ui4 current_crc);
TERN_m11	CRC_validate_m11(const ui1 *block_ptr, si8 block_bytes, ui4 crc_to_validate);



//**********************************************************************************//
//************************************  UTF-8  *************************************//
//**********************************************************************************//

// ATTRIBUTION
//
// Basic UTF-8 manipulation routines
// by Jeff Bezanson
// placed in the public domain Fall 2005
//
// "This code is designed to provide the utilities you need to manipulate
// UTF-8 as an internal string encoding. These functions do not perform the
// error checking normally needed when handling UTF-8 data, so if you happen
// to be from the Unicode Consortium you will want to flay me alive.
// I do this because error checking can be performed at the boundaries (I/O),
// with these routines reserved for higher performance on data known to be
// valid."
//
// downloaded from http://www.cprogramming.com
//
// Minor modifications for compatibility with the MED Library.

// Defines
#define UTF8_BUFFER_SIZE	2048

// Macros
#define UTF8_ISUTF_m11(c)       (((c) & 0xC0) != 0x80) // true if c is the start of a UTF-8 sequence

#define UTF8_OFFSETS_TABLE_ENTRIES_m11	6
#define UTF8_OFFSETS_TABLE_m11        { 0x0UL, 0x00003080UL, 0x000E2080UL, 0x03C82080UL, 0xFA082080UL, 0x82082080UL }

#define UTF8_TRAILING_BYTES_TABLE_ENTRIES_m11	256
#define UTF8_TRAILING_BYTES_TABLE_m11	      {	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, \
						0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, \
						0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, \
						0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, \
						0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, \
						0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, \
						1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, \
						2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5 }

// Function Prototypes
si4	UTF8_char_num_m11(si1 *s, si4 offset);  // byte offset to character number
void	UTF8_dec_m11(si1 *s, si4 *i);  // move to previous character
si4	UTF8_escape_m11(si1 *buf, si4 sz, si1 *src, si4 escape_quotes);  // convert UTF-8 "src" to ASCII with escape sequences.
si4	UTF8_escape_wchar_m11(si1 *buf, si4 sz, ui4 ch);  // given a wide character, convert it to an ASCII escape sequence stored in buf, where buf is "sz" bytes. returns the number of characters output
si4	UTF8_fprintf_m11(FILE *stream, si1 *fmt, ...);  // fprintf() where the format string and arguments may be in UTF-8. You can avoid this function and just use ordinary fprintf() if the current locale is UTF-8.
si4	UTF8_hex_digit_m11(si1 c);  // utility predicates used by the above
void	UTF8_inc_m11(si1 *s, si4 *i);  // move to next character
TERN_m11	UTF8_initialize_tables_m11(void);
si4	UTF8_is_locale_utf8_m11(si1 *locale);  // boolean function returns if locale is UTF-8, 0 otherwise
TERN_m11	UTF8_is_valid_m11(si1 *string);
si1	*UTF8_memchr_m11(si1 *s, ui4 ch, size_t sz, si4 *char_num);  // same as the above, but searches a buffer of a given size instead of a NUL-terminated string.
ui4	UTF8_next_char_m11(si1 *s, si4* i);  // return next character, updating an index variable
si4	UTF8_octal_digit_m11(si1 c);  // utility predicates used by the above
si4	UTF8_offset_m11(si1 *str, si4 char_num);  // character number to byte offset
si4     UTF8_printf_m11(si1 *fmt, ...);  // printf() where the format string and arguments may be in UTF-8. You can avoid this function and just use ordinary printf() if the current locale is UTF-8.
si4	UTF8_read_escape_sequence_m11(si1 *str, ui4 *dest);  // assuming str points to the character after a backslash, read an escape sequence, storing the result in dest and returning the number of input characters processed
si4	UTF8_seqlen_m11(si1 *s);  // returns length of next UTF-8 sequence
si1	*UTF8_strchr_m11(si1 *s, ui4 ch, si4 *char_num);  // return a pointer to the first occurrence of ch in s, or NULL if not found. character index of found character returned in *char_num.
si4	UTF8_strlen_m11(si1 *s);  // count the number of characters in a UTF-8 string
si4	UTF8_to_ucs_m11(ui4 *dest, si4 sz, si1 *src, si4 srcsz);  // convert UTF-8 data to wide character
si4	UTF8_to_utf8_m11(si1 *dest, si4 sz, ui4 *src, si4 srcsz);  // convert wide character to UTF-8 data
si4	UTF8_unescape_m11(si1 *buf, si4 sz, si1 *src);  // convert a string "src" containing escape sequences to UTF-8 if escape_quotes is nonzero, quote characters will be preceded by  backslashes as well.
si4	UTF8_vfprintf_m11(FILE *stream, si1 *fmt, va_list ap);    // called by UTF8_fprintf()
si4	UTF8_vprintf_m11(si1 *fmt, va_list ap);  // called by UTF8_printf()
si4	UTF8_wc_to_utf8_m11(si1 *dest, ui4 ch);  // single character to UTF-8



//**********************************************************************************//
//*************************************  AES-128  **********************************//
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
// THE CODE IN THIS FILE IS SET FOR AES 128-BIT ENCRYPTION / DECRYPTION ONLY
//
// Minor modifications for compatibility with the MED Library.

#define AES_NR_m11	        10	// The number of rounds in AES Cipher
#define AES_NK_m11	        4	// The number of 32 bit words in the key
#define AES_NB_m11	        4	// The number of columns comprising a state in AES. This is a constant in AES.
#define AES_XTIME_m11(x)        ((x << 1) ^ (((x >> 7) & 1) * 0x1b)) // AES_XTIME is a macro that finds the product of {02} and the argument to AES_XTIME modulo {1b}
#define AES_MULTIPLY_m11(x, y)  (((y & 1) * x) ^ ((y >> 1 & 1) * AES_XTIME_m11(x)) ^ ((y >> 2 & 1) * AES_XTIME_m11(AES_XTIME_m11(x))) ^ \
				((y >> 3 & 1) * AES_XTIME_m11(AES_XTIME_m11(AES_XTIME_m11(x)))) ^ ((y >> 4 & 1) * \
				AES_XTIME_m11(AES_XTIME_m11(AES_XTIME_m11(AES_XTIME_m11(x)))))) // Multiplty is a macro used to multiply numbers in the field GF(2^8)

#define AES_SBOX_ENTRIES_m11	256
#define AES_SBOX_m11          {	0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76, \
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

#define AES_RSBOX_ENTRIES_m11	256
#define AES_RSBOX_m11         {	0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb, \
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

#define AES_RCON_ENTRIES_m11	255
#define AES_RCON_m11          {	0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, \
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
void		AES_add_round_key_m11(si4 round, ui1 state[][4], ui1 *round_key);
void		AES_decrypt_m11(ui1 *in, ui1 *out, si1 *password, ui1 *expanded_key);
void		AES_key_expansion_m11(ui1 *round_key, si1 *key);
si4		AES_get_sbox_invert_m11(si4 num);
si4		AES_get_sbox_value_m11(si4 num);
TERN_m11	AES_initialize_tables_m11(void);
void		AES_inv_cipher_m11(ui1 *in, ui1 *out, ui1 state[][4], ui1 *round_key);
void		AES_inv_mix_columns_m11(ui1 state[][4]);
void		AES_inv_shift_rows_m11(ui1 state[][4]);
void		AES_inv_sub_bytes_m11(ui1 state[][4]);



//***********************************************************************//
//**************************  SHA-256 FUNCTIONS  ************************//
//***********************************************************************//

// ATTRIBUTION:
//
// Author:	Brad Conte (brad@bradconte.com)
// Disclaimer:	This code is presented "as is" without any guarantees.
// Details:	Implementation of the SHA-256 hashing algorithm.
//		Algorithm specification can be found here:
//	      	http://csrc.nist.gov/publications/fips/fips180-2/fips180-2withchangenotice.pdf
//		This implementation uses little endian byte order.
//
// Code:	https://github.com/B-Con/crypto-algorithms/blob/master/sha256.c
//
// Only SHA-256 functions are included in the MED library
// The version below contains minor modifications for compatibility with the MED Library.


// Constants
#define SHA_HASH_BYTES_m11	32  // 256 bit
#define SHA_LOW_BYTE_MASK_m11	(ui4) 0x000000FF

#define	SHA_H0_ENTRIES_m11	8
#define	SHA_H0_m11            {	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 }

#define	SHA_K_ENTRIES_m11	64
#define	SHA_K_m11	      {	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, \
       				0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, \
       				0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, \
       				0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, \
       				0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, \
       				0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, \
       				0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3, \
				0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2 }

// Macros
#define SHA_ROTLEFT_m11(a,b) (((a) << (b)) | ((a) >> (32-(b))))
#define SHA_ROTRIGHT_m11(a,b) (((a) >> (b)) | ((a) << (32-(b))))
#define SHA_CH_m11(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define SHA_MAJ_m11(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define SHA_EP0_m11(x) (SHA_ROTRIGHT_m11(x,2) ^ SHA_ROTRIGHT_m11(x,13) ^ SHA_ROTRIGHT_m11(x,22))
#define SHA_EP1_m11(x) (SHA_ROTRIGHT_m11(x,6) ^ SHA_ROTRIGHT_m11(x,11) ^ SHA_ROTRIGHT_m11(x,25))
#define SHA_SIG0_m11(x) (SHA_ROTRIGHT_m11(x,7) ^ SHA_ROTRIGHT_m11(x,18) ^ ((x) >> 3))
#define SHA_SIG1_m11(x) (SHA_ROTRIGHT_m11(x,17) ^ SHA_ROTRIGHT_m11(x,19) ^ ((x) >> 10))

// Typedefs & Structures
typedef struct {
	ui1	data[64];
	ui4	state[8];
	ui8	bitlen;
	ui4	datalen;
} SHA_CTX_m11;

// Function Prototypes
void		SHA_finalize_m11(SHA_CTX_m11 *ctx, ui1 *hash);
ui1    		*SHA_hash_m11(const ui1 *data, si8 len, ui1 *hash);
void		SHA_initialize_m11(SHA_CTX_m11 *ctx);
TERN_m11	SHA_initialize_tables_m11(void);
void		SHA_transform_m11(SHA_CTX_m11 *ctx, const ui1 *data);
void		SHA_update_m11(SHA_CTX_m11 *ctx, const ui1 *data, si8 len);


//**********************************************************************************//
//********************************  Time Zone Data  ********************************//
//**********************************************************************************//

// Notes:
//
// Western Sahara:
// DST is on most of the year and off during Ramadan, whose dates change annually in a way that is not accommodated by this schema.
// As Ramadan only lasts a month, and can occur at virtually any time of year, this table treats it as if it's Daylight Time
// is it's Standard Time, and it does not observe DST.
//
// If it were to have a proper entry, it would look something like:
// { "WESTERN SAHARA", "EH", "ESH", "", "", "WESTERN EUROPEAN TIME", "WET", 0, "WESTERN EUROPEAN DAYLIGHT TIME", "WEDT", 0x3c0002041f00ff01, 0xc40003031300ffff, -1 }
//
// But it is represented here as:
// { "WESTERN SAHARA", "EH", "ESH", "", "", "WESTERN EUROPEAN DAYLIGHT TIME", "WEDT", 3600, "", "", 0x0, 0x0, -1 }

#define TZ_TABLE_ENTRIES_m11      399
#define TZ_TABLE_m11 { \
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
	{ "BENIN", "BJ", "BEN", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0 }, \
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

#define TZ_COUNTRY_ALIASES_ENTRIES_m11      16
#define TZ_COUNTRY_ALIASES_TABLE_m11 { \
	{ "CHINA", "PEOPLE'S REPUBLIC OF CHINA" }, \
	{ "CHINA", "PEOPLES REPUBLIC OF CHINA" }, \
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

#define TZ_COUNTRY_ACRONYM_ALIASES_ENTRIES_m11      1
#define TZ_COUNTRY_ACRONYM_ALIASES_TABLE_m11 { \
	{ "GB", "UK" } \
}


//**********************************************************************************//
//*********************************  MED Records  **********************************//
//**********************************************************************************//

#include "medrec_m11.h"

#endif  // MEDLIB_IN_m11







