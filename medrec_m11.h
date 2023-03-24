
//**********************************************************************************//
//****************************  MED 1.0 C Library Records  *************************//
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


#ifndef MEDREC_IN_m11
#define MEDREC_IN_m11


//*************************************************************************************//
//***************************   General Record Constants   ****************************//
//*************************************************************************************//

// Constants
// LARGEST_RECORD_BYTES_m11 needs to be kept up to date, and can fail with arbitrary size records
// If it exceeds METADATA_FILE_BYTES_m11 then check_all_alignments() should be updated.
#define LARGEST_RECORD_BYTES_m11	METADATA_FILE_BYTES_m11  // 16 kB
#define RECORD_BODY_ALIGNMENT_m11       16
#define NO_RECORD_NUMBER_m11            -1

// Prototypes
void		show_record_m11(FILE_PROCESSING_STRUCT_m11 *fps, RECORD_HEADER_m11 *record_header, si8 record_number);  // pass NO_RECORD_NUMBER_m11 (for record_number) to suppress record number display
TERN_m11        check_record_structure_alignments_m11(ui1 *bytes);


//*************************************************************************************//
//********************************   Record Types   ***********************************//
//*************************************************************************************//

// Use CamelCase for 4-letter type string (case sensitive types)
// Translate to type code one byte at a time with ascii table
// (note that bytes, and therefore letters, are reversed in little-endian versions)
// #defines, structure names, and function names should follow the format in examples below
// Record versions are specified in defines and structure names as v<version major><version minor>
// e.g. version 1.0 is denoted by v10
// Record structures require 8-byte alignment. Pad structures where necessary.


//*************************************************************************************//
//******************************   Term: Terminal Record   ****************************//
//*************************************************************************************//

// Constants
#define REC_Term_TYPE_STRING_m11        "Term"                  // ascii[4]
#define REC_Term_TYPE_CODE_m11          (ui4) 0x6d726554        // ui4 (little endian)
// #define REC_Tern_TYPE_CODE_m11       (ui4) 0x5465726d        // ui4 (big endian)

// If there are any records, there is also a terminal record index
// in record indices files
// with the following header values, and no body
// File Offset = record data file length
// Start Time = segment end μUTC + 1
// Type String = REC_Term_TYPE_STRING_m11
// Type Code = REC_Term_TYPE_CODE_m11
// Format Version Major = 0xFF
// Format Version Minor = 0xFF
// Encryption Level = 0

//*************************************************************************************//
//*******************************   Sgmt: Segment Record   ****************************//
//*************************************************************************************//

// A segment record is entered at the Session and or Channel Level for each new segment
// The encryption level for these records is typically set to the same as for metadata section 2

// Constants
#define REC_Sgmt_TYPE_STRING_m11        "Sgmt"                  // ascii[4]
#define REC_Sgmt_TYPE_CODE_m11          (ui4) 0x746D6753        // ui4 (little endian)
// #define REC_Sgmt_TYPE_CODE_m11       (ui4) 0x53676D74        // ui4 (big endian)

// Version 1.0
#define REC_Sgmt_v10_BYTES_m11                                          48
#define REC_Sgmt_v10_END_TIME_OFFSET_m11                                0							// si8
#define REC_Sgmt_v10_START_SAMPLE_NUMBER_OFFSET_m11			8							// si8
#define REC_Sgmt_v10_START_SAMPLE_NUMBER_NO_ENTRY_m11			SAMPLE_NUMBER_NO_ENTRY_m11
#define REC_Sgmt_v10_START_FRAME_NUMBER_OFFSET_m11			REC_Sgmt_v10_START_SAMPLE_NUMBER_OFFSET_m11		// si8
#define REC_Sgmt_v10_START_FRAME_NUMBER_NO_ENTRY_m11			FRAME_NUMBER_NO_ENTRY_m11
#define REC_Sgmt_v10_END_SAMPLE_NUMBER_OFFSET_m11			16							// si8
#define REC_Sgmt_v10_END_SAMPLE_NUMBER_NO_ENTRY_m11			SAMPLE_NUMBER_NO_ENTRY_m11
#define REC_Sgmt_v10_END_FRAME_NUMBER_OFFSET_m11			REC_Sgmt_v10_END_SAMPLE_NUMBER_OFFSET_m11		// si8
#define REC_Sgmt_v10_END_FRAME_NUMBER_NO_ENTRY_m11			FRAME_NUMBER_NO_ENTRY_m11
#define REC_Sgmt_v10_SEGMENT_UID_OFFSET_m11                             24							// ui8
#define REC_Sgmt_v10_SEGMENT_NUMBER_OFFSET_m11                          32							// si4
#define REC_Sgmt_v10_SEGMENT_NUMBER_NO_ENTRY_m11                        SEGMENT_NUMBER_NO_ENTRY_m11
#define REC_Sgmt_v10_ACQUISITION_CHANNEL_NUMBER_OFFSET_m11              36							// si4
#define REC_Sgmt_v10_ACQUISITION_CHANNEL_NUMBER_NO_ENTRY_m11            CHANNEL_NUMBER_NO_ENTRY_m11
#define REC_Sgmt_v10_ACQUISITION_CHANNEL_NUMBER_ALL_CHANNELS_m11        CHANNEL_NUMBER_ALL_CHANNELS_m11
#define REC_Sgmt_v10_SAMPLING_FREQUENCY_OFFSET_m11                      40							// sf8
#define REC_Sgmt_v10_SAMPLING_FREQUENCY_NO_ENTRY_m11                    FREQUENCY_NO_ENTRY_m11
#define REC_Sgmt_v10_SAMPLING_FREQUENCY_VARIABLE_m11		        FREQUENCY_VARIABLE_m11
#define REC_Sgmt_v10_FRAME_RATE_OFFSET_m11                      	REC_Sgmt_v10_SAMPLING_FREQUENCY_OFFSET_m11		// sf8
#define REC_Sgmt_v10_FRAME_RATE_NO_ENTRY_m11                    	FREQUENCY_NO_ENTRY_m11
#define REC_Sgmt_v10_FRAME_RATE_VARIABLE_m11		        	FREQUENCY_VARIABLE_m11
#define REC_Sgmt_v10_SEGMENT_DESCRIPTION_OFFSET_m11                     REC_Sgmt_v10_BYTES_m11					// si1 (arbitrary length, padded to 16-byte alignment for encryption)

// Structures
// REC_Sgmt_v10_m11 defined in medlib_m11.h due to codependency

// Prototypes
void            show_rec_Sgmt_type_m11(RECORD_HEADER_m11 *record_header);
TERN_m11        check_rec_Sgmt_type_alignment_m11(ui1 *bytes);


//*************************************************************************************//
//*****************************   Stat: Statistics Record   ***************************//
//*************************************************************************************//

// Constants
#define REC_Stat_TYPE_STRING_m11        "Stat"                  // ascii[4]
#define REC_Stat_TYPE_CODE_m11          (ui4) 0x74617453        // ui4 (little endian)
// #define REC_Sgmt_TYPE_CODE_m11       (ui4) 0x53746174        // ui4 (big endian)

// Version 1.0
#define REC_Stat_v10_BYTES_m11			32
#define REC_Stat_v10_MINIMUM_OFFSET_m11		0                       // si4
#define REC_Stat_v10_MINIMUM_NO_ENTRY_m11	NAN_SI4_m11
#define REC_Stat_v10_MAXIMUM_OFFSET_m11		4                       // si4
#define REC_Stat_v10_MAXIMUM_NO_ENTRY_m11	NAN_SI4_m11
#define REC_Stat_v10_MEAN_OFFSET_m11		8                       // si4
#define REC_Stat_v10_MEAN_NO_ENTRY_m11		NAN_SI4_m11
#define REC_Stat_v10_MEDIAN_OFFSET_m11		12                      // si4
#define REC_Stat_v10_MEDIAN_NO_ENTRY_m11	NAN_SI4_m11
#define REC_Stat_v10_MODE_OFFSET_m11		16			// si4
#define REC_Stat_v10_MODE_NO_ENTRY_m11		NAN_SI4_m11
#define REC_Stat_v10_VARIANCE_OFFSET_m11	20                      // sf4
#define REC_Stat_v10_VARIANCE_NO_ENTRY_m11	((sf4) nan(NULL))       // NOTE this value must be tested with isnan(), or an equivalent function, rather than ==
#define REC_Stat_v10_SKEWNESS_OFFSET_m11	24                      // sf4
#define REC_Stat_v10_SKEWNESS_NO_ENTRY_m11	((sf4) nan(NULL))       // NOTE this value must be tested with isnan(), or an equivalent function, rather than ==
#define REC_Stat_v10_KURTOSIS_OFFSET_m11	28                      // sf4
#define REC_Stat_v10_KURTOSIS_NO_ENTRY_m11	((sf4) nan(NULL))       // NOTE this value must be tested with isnan(), or an equivalent function, rather than ==

// Structures
// REC_Stat_v10_m11 defined in medlib_m11.h due to codependency

// Prototypes
void            show_rec_Stat_type_m11(RECORD_HEADER_m11 *record_header);
TERN_m11        check_rec_Stat_type_alignment_m11(ui1 *bytes);



//*************************************************************************************//
//******************************   EDFA: EDF Annotation   *****************************//
//*************************************************************************************//
// Constants
#define REC_EDFA_TYPE_STRING_m11        "EDFA"			// ascii[4]
#define REC_EDFA_TYPE_CODE_m11		(ui4) 0x41464445	// ui4 (little endian)
// #define REC_EDFA_TYPE_CODE_m11    	(ui4) 0x45444641	// ui4 (big endian)

// Version 1.0
#define REC_EDFA_v10_BYTES_m11			8
#define REC_EDFA_v10_DURATION_OFFSET_m11        0			// si8
#define REC_EDFA_v10_ANNOTATION_OFFSET_m11      REC_EDFA_v10_BYTES_m11	// si1

// Structures
typedef struct {
	si8	duration;  // microseconds (single element structure not really necessary, but done for consistency).
} REC_EDFA_v10_m11;
// Annotation follows structure - aribitrary length array of si1s padded to 16 byte alignment (for structure + string)

// Prototypes
void	        show_rec_EDFA_type_m11(RECORD_HEADER_m11 *record_header);
TERN_m11        check_rec_EDFA_type_alignment_m11(ui1 *bytes);



//*************************************************************************************//
//********************************   Note: Note Record   ******************************//
//*************************************************************************************//

// Constants
#define REC_Note_TYPE_STRING_m11        "Note"			// ascii[4]
#define REC_Note_TYPE_CODE_m11		(ui4) 0x65746f4e	// ui4 (little endian)
// #define REC_Note_TYPE_CODE_m11       (ui4) 0x4e6f7465	// ui4

// Version 1.0
#define REC_Note_v10_TEXT_OFFSET_m11    0

// Structures
// (none)
// Annotation follows header - aribitrary length array of si1s padded to 16 byte alignment


// Prototypes
void	        show_rec_Note_type_m11(RECORD_HEADER_m11 *record_header);
TERN_m11	check_rec_Note_type_alignment_m11(ui1 *bytes);



//*************************************************************************************//
//*******************************   Seiz: Seizure Record   ****************************//
//*************************************************************************************//

// Constants
#define REC_Seiz_TYPE_STRING_m11        "Seiz"                  // ascii[4]
#define REC_Seiz_TYPE_CODE_m11          (ui4) 0x7a696553        // ui4 (little endian)
// #define REC_Seiz_TYPE_CODE_m11       (ui4) 0x5365697a        // ui4 (big endian)

// Version 1.0
// REC_Seiz_v10_m11 offsets below apply to base address of record
#define REC_Seiz_v10_BYTES_m11			                        1296
#define REC_Seiz_v10_LATEST_OFFSET_TIME_OFFSET_m11	                0	// si8
#define REC_Seiz_v10_NUMBER_OF_CHANNELS_OFFSET_m11	                8	// si4
#define REC_Seiz_v10_ONSET_CODE_OFFSET_m11		                12	// si4
#define REC_Seiz_v10_MARKER_NAME_1_OFFSET_m11	                        16	// utf8[31]
#define REC_Seiz_v10_MARKER_NAME_BYTES_m11		                128
#define REC_Seiz_v10_MARKER_NAME_2_OFFSET_m11	                        144	// utf8[31]
#define REC_Seiz_v10_ANNOTATION_OFFSET_m11		                272	// utf8[255]
#define REC_Seiz_v10_ANNOTATION_BYTES_m11		                1024
#define REC_Seiz_v10_CHANNELS_OFFSET_m11		                REC_Seiz_v10_BYTES_m11
// REC_Seiz_v10_CHANNEL_m11 offsets below apply to base address of channel
#define REC_Seiz_v10_CHANNEL_BYTES_m11		                        280
#define REC_Seiz_v10_CHANNEL_NAME_OFFSET_m11		                0	// utf8[63]
#define REC_Seiz_v10_CHANNEL_ONSET_TIME_OFFSET_m11                      256	// si8
#define REC_Seiz_v10_CHANNEL_OFFSET_TIME_OFFSET_m11                     264	// si8
#define REC_Seiz_v10_CHANNEL_SEGMENT_NUMBER_OFFSET_m11                  272	// si4
#define REC_Seiz_v10_CHANNEL_SEGMENT_NUMBER_NO_ENTRY_m11                SEGMENT_NUMBER_NO_ENTRY_m11
#define REC_Seiz_v10_CHANNEL_PAD_OFFSET_m11                       	276	// ui1[4]
#define REC_Seiz_v10_CHANNEL_PAD_BYTES_m11                              4
// Onset Codes
#define REC_Seiz_v10_ONSET_NO_ENTRY_m11		-1
#define REC_Seiz_v10_ONSET_UNKNOWN_m11		0
#define REC_Seiz_v10_ONSET_FOCAL_m11            1
#define REC_Seiz_v10_ONSET_GENERALIZED_m11      2
#define REC_Seiz_v10_ONSET_PROPAGATED_m11       3
#define REC_Seiz_v10_ONSET_MIXED_m11            4

// Structures
typedef struct {
        // earliest_onset_time is in the record header
        si8	latest_offset_time;                                     // uutc
        si4	number_of_channels;
        si4	onset_code;
        si1	marker_name_1[REC_Seiz_v10_MARKER_NAME_BYTES_m11];	// utf8[31]
        si1	marker_name_2[REC_Seiz_v10_MARKER_NAME_BYTES_m11];	// utf8[31]
        si1	annotation[REC_Seiz_v10_ANNOTATION_BYTES_m11];		// utf8[255]
} REC_Seiz_v10_m11;

typedef struct {
 	si1	name[BASE_FILE_NAME_BYTES_m11]; // channel name, no extension
       	si8	onset_time;                     // uutc
        si8	offset_time;                    // uutc
        si4     segment_number;
        ui1     pad[REC_Seiz_v10_CHANNEL_PAD_BYTES_m11];	// for 8 byte alignment (couldn't think of a use for them)
} REC_Seiz_v10_CHANNEL_m11;

// Prototypes
void	        show_rec_Seiz_type_m11(RECORD_HEADER_m11 *record_header);
TERN_m11	check_rec_Seiz_type_alignment_m11(ui1 *bytes);



//*************************************************************************************//
//*****************************   SyLg: System Log Record   ***************************//
//*************************************************************************************//

// Constants
#define REC_SyLg_TYPE_STRING_m11        "SyLg"			// ascii[4]
#define REC_SyLg_TYPE_CODE_m11	        (ui4) 0x674c7953	// ui4 (little endian)
// #define REC_SyLg_TYPE_CODE_m11       (ui4) 0x53794c67	// ui4 (big endian)

// Version 1.0
#define REC_SyLg_v10_TEXT_OFFSET_m11    0

// Structures
// (none)
// Log text follows header - aribitrary length array of si1s padded to 16 byte alignment

// Prototype
void	        show_rec_SyLg_type_m11(RECORD_HEADER_m11 *record_header);
TERN_m11	check_rec_SyLg_type_alignment_m11(ui1 *bytes);



//*************************************************************************************//
//***************************   NlxP: NeuraLynx Port Record   *************************//
//*************************************************************************************//

// Constants
#define REC_NlxP_TYPE_STRING_m11             "NlxP"                  // ascii[4]
#define REC_NlxP_TYPE_CODE_m11               (ui4) 0x50786C4E        // ui4 (little endian)
// #define REC_NlxP_TYPE_CODE_m11            (ui4) 0x4E6C7850        // ui4 (big endian)

// Trigger Modes
#define REC_NlxP_v10_NO_TRIGGER_m11		((ui1) 0)
#define REC_NlxP_v10_ANY_BIT_CHANGE_m11		((ui1) 1)
#define REC_NlxP_v10_HIGH_BIT_SET_m11		((ui1) 2)
#define REC_NlxP_v10_UNKNOWN_TRIGGER_MODE_m11	((ui1) 0xFF)

// Version 1.0
#define REC_NlxP_v10_BYTES_m11                          16
#define REC_NlxP_v10_RAW_PORT_VALUE_OFFSET_m11          0	// ui4
#define REC_NlxP_v10_VALUE_OFFSET_m11                   4	// ui4
#define REC_NlxP_v10_SUBPORT_OFFSET_m11                 8	// ui1
#define REC_NlxP_v10_NUMBER_OF_SUBPORTS_OFFSET_m11      9	// ui1
#define REC_NlxP_v10_TRIGGER_MODE_OFFSET_m11            10	// ui1
#define REC_NlxP_v10_PAD_OFFSET_m11               	11	// ui1
#define REC_NlxP_v10_PAD_BYTES_m11                      5

// Structures
typedef struct {
        ui4     raw_port_value;         // full port value (no bits masked out)
        ui4     value;                  // subport value only (high bits masked out in HIGH BIT SET trigger mode)
        ui1     subport;                // [1 to <number_of_subports>]  (subport that triggered this record)
        ui1     number_of_subports;     // [1, 2, or 4]  (one 32-bit, two 16-bit, or four 8-bit)
        ui1     trigger_mode;           // REC_NlxP_v10_NO_TRIGGER_m11, REC_NlxP_v10_ANY_BIT_CHANGE_m11, or REC_NlxP_v10_HIGH_BIT_SET_m11
        ui1     pad[REC_NlxP_v10_PAD_BYTES_m11];
} REC_NlxP_v10_m11;

// Prototypes
void            show_rec_NlxP_type_m11(RECORD_HEADER_m11 *record_header);
TERN_m11        check_rec_NlxP_type_alignment_m11(ui1 *bytes);


//*************************************************************************************//
//***********************   Curs: Cadwell EMG Cursor Annotation   *********************//
//*************************************************************************************//

// Constants
#define REC_Curs_TYPE_STRING_m11             "Curs"                  // ascii[4]
#define REC_Curs_TYPE_CODE_m11               (ui4) 0x73727543        // ui4 (little endian)
// #define REC_Curs_TYPE_CODE_m11            (ui4) 0x43757273        // ui4 (big endian)

// Version 1.0
#define REC_Curs_v10_BYTES_m11                          160
#define REC_Curs_v10_ID_NUMBER_OFFSET_m11          	0	// si8
#define REC_Curs_v10_LATENCY_OFFSET_m11			8	// si8
#define REC_Curs_v10_VALUE_OFFSET_m11			16	// sf8
#define REC_Curs_v10_NAME_OFFSET_m11			24	// si1[136]
#define REC_Curs_v10_NAME_BYTES_m11         		136

// Structures
typedef struct {
    si8 id_number;
    si8 latency;
    sf8 value;
    si1 name[REC_Curs_v10_NAME_BYTES_m11];
} REC_Curs_v10_m11;

// Prototypes
void            show_rec_Curs_type_m11(RECORD_HEADER_m11 *record_header);
TERN_m11        check_rec_Curs_type_alignment_m11(ui1 *bytes);



//*************************************************************************************//
//****************************   Epoc: Sleep Stage Record   ***************************//
//*************************************************************************************//

// Constants
#define REC_Epoc_TYPE_STRING_m11             "Epoc"                  // ascii[4]
#define REC_Epoc_TYPE_CODE_m11               (ui4) 0x636F7045        // ui4 (little endian)
// #define REC_Epoc_TYPE_CODE_m11            (ui4) 0x45706F63        // ui4 (big endian)

// Version 1.0
#define REC_Epoc_v10_BYTES_m11                          176
#define REC_Epoc_v10_ID_NUMBER_OFFSET_m11          	0	// si8
#define REC_Epoc_v10_END_TIME_OFFSET_m11		8	// si8
#define REC_Epoc_v10_EPOCH_TYPE_OFFSET_m11		16	// si1[32]
#define REC_Epoc_v10_EPOCH_TYPE_BYTES_m11          	32
#define REC_Epoc_v10_TEXT_OFFSET_m11			48	// si1[128]
#define REC_Epoc_v10_TEXT_BYTES_m11          		128

// Structures
typedef struct {
    si8 id_number;
    si8 end_time;
    si1 epoch_type[REC_Epoc_v10_EPOCH_TYPE_BYTES_m11];
    si1 text[REC_Epoc_v10_TEXT_BYTES_m11];
} REC_Epoc_v10_m11;

// Prototypes
void            show_rec_Epoc_type_m11(RECORD_HEADER_m11 *record_header);
TERN_m11        check_rec_Epoc_type_alignment_m11(ui1 *bytes);



//*************************************************************************************//
//**************************   ESti: Electrical Stimulation   *************************//
//*************************************************************************************//

// Constants
#define REC_ESti_TYPE_STRING_m11             "ESti"                  // ascii[4]
#define REC_ESti_TYPE_CODE_m11               (ui4) 0x69745345        // ui4 (little endian)
// #define REC_ESti_TYPE_CODE_m11            (ui4) 0x45537469        // ui4 (big endian)

// Version 1.0
#define REC_ESti_v10_BYTES_m11          		416
#define REC_ESti_v10_AMPLITUDE_OFFSET_m11          	0	// sf8
#define REC_ESti_v10_FREQUENCY_OFFSET_m11  		8	// sf8
#define REC_ESti_v10_PULSE_WIDTH_OFFSET_m11  		16	// si8
#define REC_ESti_v10_AMP_UNIT_CODE_OFFSET_m11		24	// si4
#define REC_ESti_v10_MODE_CODE_OFFSET_m11		28	// si4
#define REC_ESti_v10_WAVEFORM_OFFSET_m11		32	// utf8[31]
#define REC_ESti_v10_WAVEFORM_BYTES_m11			128
#define REC_ESti_v10_ANODE_OFFSET_m11			160	// utf8[31]
#define REC_ESti_v10_ANODE_BYTES_m11			128
#define REC_ESti_v10_CATHODE_OFFSET_m11			288	// utf8[31]
#define REC_ESti_v10_CATHODE_BYTES_m11			128

// Unit codes
#define REC_ESti_v10_AMP_UNIT_NO_ENTRY_m11	-1
#define REC_ESti_v10_AMP_UNIT_UNKNOWN_m11	0
#define REC_ESti_v10_AMP_UNIT_MA_m11		1
#define REC_ESti_v10_AMP_UNIT_V_m11		2

// Mode codes
#define REC_ESti_v10_MODE_NO_ENTRY_m11		-1
#define REC_ESti_v10_MODE_UNKNOWN_m11		0
#define REC_ESti_v10_MODE_CURRENT_m11		1
#define REC_ESti_v10_MODE_VOLTAGE_m11		2

// Structures
typedef struct {
	sf8	amplitude;
	sf8	frequency;
	si8	pulse_width;
	si4	amp_unit_code;
	si4	mode_code;
	si1	waveform[REC_ESti_v10_WAVEFORM_BYTES_m11];
	si1	anode[REC_ESti_v10_ANODE_BYTES_m11];
	si1	cathode[REC_ESti_v10_CATHODE_BYTES_m11];
} REC_ESti_v10_m11;

// Prototypes
void            show_rec_ESti_type_m11(RECORD_HEADER_m11 *record_header);
TERN_m11        check_rec_ESti_type_alignment_m11(ui1 *bytes);



//*************************************************************************************//
//***************************   CSti: Cognitive Stimulation   *************************//
//*************************************************************************************//

// Constants
#define REC_CSti_TYPE_STRING_m11             "CSti"                  // ascii[4]
#define REC_CSti_TYPE_CODE_m11               (ui4) 0x69745343        // ui4 (little endian)
// #define REC_CSti_TYPE_CODE_m11            (ui4) 0x43537469        // ui4 (big endian)

// Version 1.0
#define REC_CSti_v10_BYTES_m11          		208
#define REC_CSti_v10_STIMULUS_DURATION_OFFSET_m11	0	// si8
#define REC_CSti_v10_TASK_TYPE_OFFSET_m11  		8	// utf8[15]
#define REC_CSti_v10_TASK_TYPE_BYTES_m11		64
#define REC_CSti_v10_STIMULUS_TYPE_OFFSET_m11		72	// utf8[15]
#define REC_CSti_v10_STIMULUS_TYPE_BYTES_m11		64
#define REC_CSti_v10_PATIENT_RESPONSE_OFFSET_m11	136	// utf8[15]
#define REC_CSti_v10_PATIENT_RESPONSE_BYTES_m11		64
#define REC_CSti_v10_PAD_OFFSET_m11			200	// ui1[8]
#define REC_CSti_v10_PAD_BYTES_m11			8

// Structures
typedef struct {
	si8	stimulus_duration;
	si1	task_type[REC_CSti_v10_TASK_TYPE_BYTES_m11];
	si1	stimulus_type[REC_CSti_v10_STIMULUS_TYPE_BYTES_m11];
	si1	patient_response[REC_CSti_v10_PATIENT_RESPONSE_BYTES_m11];
	ui1	pad[REC_CSti_v10_PAD_BYTES_m11];
} REC_CSti_v10_m11;

// Prototypes
void            show_rec_CSti_type_m11(RECORD_HEADER_m11 *record_header);
TERN_m11        check_rec_CSti_type_alignment_m11(ui1 *bytes);



#endif // MEDREC_IN_m11

