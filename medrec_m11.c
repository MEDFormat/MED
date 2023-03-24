
//**********************************************************************************//
//***************************  MED 1.0.1 C Library Records  ************************//
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



#include "medlib_m11.h"


//*************************************************************************************//
//**********************************   show_record()   ********************************//
//*************************************************************************************//

void	show_record_m11(FILE_PROCESSING_STRUCT_m11 *fps, RECORD_HEADER_m11 *record_header, si8 record_number)
{
	ui4                     type_code;
	si1	                time_str[TIME_STRING_BYTES_m11], hex_str[HEX_STRING_BYTES_m11(CRC_BYTES_m11)];

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// decrypt record body if necessary
	if (record_header->encryption_level > NO_ENCRYPTION_m11)
		decrypt_record_data_m11(fps, record_header, 1);
		    
	// display record header fields
	if (record_number != NO_RECORD_NUMBER_m11)
		printf_m11("Record Number: %ld\n", record_number);
	printf_m11("---------------- Record Header - START ----------------\n");
	if (record_header->record_CRC == RECORD_HEADER_RECORD_CRC_NO_ENTRY_m11) {
		printf_m11("Record CRC: no entry\n");
	} else {
		generate_hex_string_m11((ui1 *) &record_header->record_CRC, CRC_BYTES_m11, hex_str);
		printf_m11("Record CRC: %s\n", hex_str);
	}
	type_code = record_header->type_code;
	if (type_code) {
		generate_hex_string_m11((ui1 *) record_header->type_string, CRC_BYTES_m11, hex_str);
		printf_m11("Record Type String: %s (%s)\n", record_header->type_string, hex_str);
	} else {
		printf_m11("Record Type String: no entry\n");
	}
	if (record_header->version_major == RECORD_HEADER_VERSION_MAJOR_NO_ENTRY_m11 || record_header->version_minor == RECORD_HEADER_VERSION_MINOR_NO_ENTRY_m11) {
		if (record_header->version_major == RECORD_HEADER_VERSION_MAJOR_NO_ENTRY_m11)
			printf_m11("Record Version Major: no entry\n");
		else
			printf_m11("Record Version Major: %u\n", record_header->version_major);
		if (record_header->version_minor == RECORD_HEADER_VERSION_MINOR_NO_ENTRY_m11)
			printf_m11("Record Version Minor: no entry\n");
		else
			printf_m11("Record Version Minor: %u\n", record_header->version_minor);
	} else {
		printf_m11("Record Version: %hu.%hu\n", record_header->version_major, record_header->version_minor);
	}
	printf_m11("Record Encryption Level: %hd ", record_header->encryption_level);
	if (record_header->encryption_level == NO_ENCRYPTION_m11)
		printf_m11("(none)\n");
	else if (record_header->encryption_level == LEVEL_1_ENCRYPTION_m11)
		printf_m11("(level 1, currently encrypted)\n");
	else if (record_header->encryption_level == LEVEL_2_ENCRYPTION_m11)
		printf_m11("(level 2, currently encrypted)\n");
	else if (record_header->encryption_level == LEVEL_1_ENCRYPTION_DECRYPTED_m11)
		printf_m11("(level 1, currently decrypted)\n");
	else if (record_header->encryption_level == LEVEL_2_ENCRYPTION_DECRYPTED_m11)
		printf_m11("(level 2, currently decrypted)\n");
	else
		printf_m11("(unrecognized code)\n");
	if (record_header->total_record_bytes == RECORD_HEADER_TOTAL_RECORD_BYTES_NO_ENTRY_m11)
		printf_m11("Record Total Record Bytes: no entry\n");
	else
		printf_m11("Record Total Record Bytes: %u\n", record_header->total_record_bytes);

	if (record_header->start_time == RECORD_HEADER_START_TIME_NO_ENTRY_m11)
		printf_m11("Record Start Time: no entry\n");
	else {
		time_string_m11(record_header->start_time, time_str, TRUE_m11, FALSE_m11, FALSE_m11);
		printf_m11("Record Start Time: %ld (oUTC), %s\n", record_header->start_time, time_str);
	}
	printf_m11("----------------- Record Header - END -----------------\n");

	// record body
	printf_m11("----------------- Record Body - START -----------------\n");
	if (record_header->encryption_level > NO_ENCRYPTION_m11) {
		printf_m11("No access to this record\n");
		printf_m11("------------------ Record Body - END ------------------\n\n");
		return;
	}

	// pass the display off to custom functions - new records types should be added here (maintain alphabetical order of record types)
	switch (type_code) {
	case REC_Sgmt_TYPE_CODE_m11:
		show_rec_Sgmt_type_m11(record_header);
		break;
	case REC_Stat_TYPE_CODE_m11:
		show_rec_Stat_type_m11(record_header);
		break;
	case REC_Note_TYPE_CODE_m11:
		show_rec_Note_type_m11(record_header);
		break;
	case REC_EDFA_TYPE_CODE_m11:
		show_rec_EDFA_type_m11(record_header);
		break;
	case REC_Seiz_TYPE_CODE_m11:
		show_rec_Seiz_type_m11(record_header);
		break;
	case REC_SyLg_TYPE_CODE_m11:
		show_rec_SyLg_type_m11(record_header);
		break;
	case REC_NlxP_TYPE_CODE_m11:
		show_rec_NlxP_type_m11(record_header);
		break;
	case REC_Curs_TYPE_CODE_m11:
		show_rec_Curs_type_m11(record_header);
		break;
	case REC_Epoc_TYPE_CODE_m11:
		show_rec_Epoc_type_m11(record_header);
		break;
	case REC_ESti_TYPE_CODE_m11:
		show_rec_ESti_type_m11(record_header);
		break;
	case REC_CSti_TYPE_CODE_m11:
		show_rec_CSti_type_m11(record_header);
		break;
	default:
		warning_message_m11("%s(): 0x%x is an unrecognized record type code\n", __FUNCTION__, type_code);
		break;
	}
	printf_m11("------------------ Record Body - END ------------------\n\n");

	return;
}


//*************************************************************************************//
//*********************   check_record_structure_alignments()   ***********************//
//*************************************************************************************//

TERN_m11	check_record_structure_alignments_m11(ui1 *bytes)
{
	extern GLOBALS_m11	*globals_m11;
	TERN_m11		return_value, free_flag = FALSE_m11;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// see if already checked
	if (globals_m11->all_record_structures_aligned != UNKNOWN_m11)
		return(globals_m11->all_record_structures_aligned);

	return_value = TRUE_m11;
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(LARGEST_RECORD_BYTES_m11);
		free_flag = TRUE_m11;
	}

	// check all structures - add new functions here
	if ((check_rec_Sgmt_type_alignment_m11(bytes)) == FALSE_m11)
		return_value = FALSE_m11;
	if ((check_rec_Stat_type_alignment_m11(bytes)) == FALSE_m11)
		return_value = FALSE_m11;
	if ((check_rec_Note_type_alignment_m11(bytes)) == FALSE_m11)
		return_value = FALSE_m11;
	if ((check_rec_EDFA_type_alignment_m11(bytes)) == FALSE_m11)
		return_value = FALSE_m11;
	if ((check_rec_Seiz_type_alignment_m11(bytes)) == FALSE_m11)
		return_value = FALSE_m11;
	if ((check_rec_SyLg_type_alignment_m11(bytes)) == FALSE_m11)
		return_value = FALSE_m11;
	if ((check_rec_NlxP_type_alignment_m11(bytes)) == FALSE_m11)
		return_value = FALSE_m11;
	if ((check_rec_Curs_type_alignment_m11(bytes)) == FALSE_m11)
		return_value = FALSE_m11;
	if ((check_rec_Epoc_type_alignment_m11(bytes)) == FALSE_m11)
		return_value = FALSE_m11;
	if ((check_rec_ESti_type_alignment_m11(bytes)) == FALSE_m11)
		return_value = FALSE_m11;
	if ((check_rec_CSti_type_alignment_m11(bytes)) == FALSE_m11)
		return_value = FALSE_m11;

	if (free_flag == TRUE_m11)
		free((void *) bytes);

	if (return_value == TRUE_m11) {
		globals_m11->all_record_structures_aligned = TRUE_m11;
		if (globals_m11->verbose == TRUE_m11)
			printf_m11("%s(): All Record structures are aligned\n", __FUNCTION__);
	}
	else {
		globals_m11->all_record_structures_aligned = FALSE_m11;
		error_message_m11("%s(): One or more Record structures are NOT aligned\n", __FUNCTION__);
	}

	return(return_value);
}


//*************************************************************************************//
//*******************************   Sgmt: Segment Record   ****************************//
//*************************************************************************************//

void    show_rec_Sgmt_type_m11(RECORD_HEADER_m11 *record_header)
{
	extern GLOBALS_m11	*globals_m11;
	REC_Sgmt_v10_m11	*Sgmt;
	si1                     time_str[TIME_STRING_BYTES_m11], hex_str[HEX_STRING_BYTES_m11(8)], *segment_description;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		Sgmt = (REC_Sgmt_v10_m11 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m11);

		time_string_m11(Sgmt->end_time, time_str, TRUE_m11, FALSE_m11, FALSE_m11);
		printf_m11("End Time: %ld (oUTC), %s\n", Sgmt->end_time, time_str);
		if (Sgmt->start_sample_number == REC_Sgmt_v10_START_SAMPLE_NUMBER_NO_ENTRY_m11)
			printf_m11("Start Sample Number: no entry\n");
		else
			printf_m11("Start Sample Number: %ld\n", Sgmt->start_sample_number);
		if (Sgmt->end_sample_number == REC_Sgmt_v10_END_SAMPLE_NUMBER_NO_ENTRY_m11)
			printf_m11("End Sample Number: no entry\n");
		else
			printf_m11("End Sample Number: %ld\n", Sgmt->end_sample_number);
		generate_hex_string_m11((ui1*)&Sgmt->segment_UID, 8, hex_str);
		printf_m11("Segment UID: %s\n", hex_str);
		if (Sgmt->segment_number == REC_Sgmt_v10_SEGMENT_NUMBER_NO_ENTRY_m11)
			printf_m11("Segment Number: no entry\n");
		else
			printf_m11("Segment Number: %d\n", Sgmt->segment_number);

		if (Sgmt->acquisition_channel_number == REC_Sgmt_v10_ACQUISITION_CHANNEL_NUMBER_ALL_CHANNELS_m11)
			printf_m11("Acquisition Channel Number: all channels\n");
		else if (Sgmt->acquisition_channel_number == REC_Sgmt_v10_ACQUISITION_CHANNEL_NUMBER_NO_ENTRY_m11)
			printf_m11("Acquisition Channel Number: no entry\n");
		else
			printf_m11("Acquisition Channel Number: %d\n", Sgmt->acquisition_channel_number);

		if (Sgmt->sampling_frequency == REC_Sgmt_v10_SAMPLING_FREQUENCY_NO_ENTRY_m11)
			printf_m11("Sampling Frequency: no entry\n");
		else if (Sgmt->sampling_frequency == REC_Sgmt_v10_SAMPLING_FREQUENCY_VARIABLE_m11)
			printf_m11("Sampling Frequency: variable\n");
		else
			printf_m11("Sampling Frequency: %lf\n", Sgmt->sampling_frequency);

		if (record_header->total_record_bytes > (RECORD_HEADER_BYTES_m11 + REC_Sgmt_v10_BYTES_m11)) {
			segment_description = (si1 *) Sgmt + REC_Sgmt_v10_SEGMENT_DESCRIPTION_OFFSET_m11;
			if (*segment_description)
				UTF8_printf_m11("Segment Description: %s\n", segment_description);
			else
				printf_m11("Segment Description: no entry\n");
		} else {
			printf_m11("Segment Description: no entry\n");
		}
	}
	// Unrecognized record version
	else {
		error_message_m11("%s(): Unrecognized Sgmt Record version (%hhd.%hhd)\n", __FUNCTION__, record_header->version_major, record_header->version_minor);
	}

	return;
}


TERN_m11     check_rec_Sgmt_type_alignment_m11(ui1 *bytes)
{
	extern GLOBALS_m11	*globals_m11;
	REC_Sgmt_v10_m11	*Sgmt;
	TERN_m11                free_flag = FALSE_m11;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// check overall size
	if (sizeof(REC_Sgmt_v10_m11) != REC_Sgmt_v10_BYTES_m11)
		goto REC_Sgmt_v10_NOT_ALIGNED_m11;

	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(REC_Sgmt_v10_BYTES_m11);
		free_flag = TRUE_m11;
	}
	Sgmt = (REC_Sgmt_v10_m11 *) bytes;
	if (&Sgmt->end_time != (si8*)(bytes + REC_Sgmt_v10_END_TIME_OFFSET_m11))
		goto REC_Sgmt_v10_NOT_ALIGNED_m11;
	if (&Sgmt->start_sample_number != (si8 *) (bytes + REC_Sgmt_v10_START_SAMPLE_NUMBER_OFFSET_m11))
		goto REC_Sgmt_v10_NOT_ALIGNED_m11;
	if (&Sgmt->start_frame_number != (si8 *) (bytes + REC_Sgmt_v10_START_FRAME_NUMBER_OFFSET_m11))
		goto REC_Sgmt_v10_NOT_ALIGNED_m11;
	if (&Sgmt->end_sample_number != (si8 *) (bytes + REC_Sgmt_v10_END_SAMPLE_NUMBER_OFFSET_m11))
		goto REC_Sgmt_v10_NOT_ALIGNED_m11;
	if (&Sgmt->end_frame_number != (si8 *) (bytes + REC_Sgmt_v10_END_FRAME_NUMBER_OFFSET_m11))
		goto REC_Sgmt_v10_NOT_ALIGNED_m11;
	if (&Sgmt->segment_UID != (ui8 *) (bytes + REC_Sgmt_v10_SEGMENT_UID_OFFSET_m11))
		goto REC_Sgmt_v10_NOT_ALIGNED_m11;
	if (&Sgmt->segment_number != (si4 *) (bytes + REC_Sgmt_v10_SEGMENT_NUMBER_OFFSET_m11))
		goto REC_Sgmt_v10_NOT_ALIGNED_m11;
	if (&Sgmt->acquisition_channel_number != (si4 *) (bytes + REC_Sgmt_v10_ACQUISITION_CHANNEL_NUMBER_OFFSET_m11))
		goto REC_Sgmt_v10_NOT_ALIGNED_m11;
	if (&Sgmt->sampling_frequency != (sf8 *) (bytes + REC_Sgmt_v10_SAMPLING_FREQUENCY_OFFSET_m11))
		goto REC_Sgmt_v10_NOT_ALIGNED_m11;

	// aligned
	if (free_flag == TRUE_m11)
		free((void *) bytes);

	if (globals_m11->verbose == TRUE_m11)
		printf_m11("%s(): REC_Sgmt_v10_m11 structure is aligned\n", __FUNCTION__);

	return(TRUE_m11);

	// not aligned
REC_Sgmt_v10_NOT_ALIGNED_m11:

	if (free_flag == TRUE_m11)
		free((void *) bytes);

	error_message_m11("%s(): REC_Sgmt_v10_m11 structure is NOT aligned", __FUNCTION__);

	return(FALSE_m11);

}


//*************************************************************************************//
//*******************************   Stat: Segment Record   ****************************//
//*************************************************************************************//

void    show_rec_Stat_type_m11(RECORD_HEADER_m11 *record_header)
{
	REC_Stat_v10_m11	*Stat;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		Stat = (REC_Stat_v10_m11 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m11);
		if (Stat->minimum == REC_Stat_v10_MINIMUM_NO_ENTRY_m11)
			printf_m11("Minimum: no entry\n");
		else
			printf_m11("Minimum: %d\n", Stat->minimum);
		if (Stat->maximum == REC_Stat_v10_MAXIMUM_NO_ENTRY_m11)
			printf_m11("Maximum: no entry\n");
		else
			printf_m11("Maximum: %d\n", Stat->maximum);
		if (Stat->mean == REC_Stat_v10_MEAN_NO_ENTRY_m11)
			printf_m11("Mean: no entry\n");
		else
			printf_m11("Mean: %d\n", Stat->mean);
		if (Stat->median == REC_Stat_v10_MEDIAN_NO_ENTRY_m11)
			printf_m11("Median: no entry\n");
		else
			printf_m11("Median: %d\n", Stat->median);
		if (Stat->mode == REC_Stat_v10_MODE_NO_ENTRY_m11)
			printf_m11("Mode: no entry\n");
		else
			printf_m11("Mode: %d\n", Stat->mode);
		if (isnan(Stat->variance))
			printf_m11("Variance: no entry\n");
		else
			printf_m11("Variance: %f\n", Stat->variance);
		if (isnan(Stat->skewness))
			printf_m11("Skewness: no entry\n");
		else
			printf_m11("Skewness: %f\n", Stat->skewness);
		if (isnan(Stat->kurtosis))
			printf_m11("Kurtosis: no entry\n");
		else
			printf_m11("Kurtosis: %f\n", Stat->kurtosis);
	}
	// Unrecognized record version
	else {
		error_message_m11("%s(): Unrecognized Stat Record version (%hhd.%hhd)\n", __FUNCTION__, record_header->version_major, record_header->version_minor);
	}

	return;

}


TERN_m11     check_rec_Stat_type_alignment_m11(ui1 *bytes)
{
	extern GLOBALS_m11	*globals_m11;
	REC_Stat_v10_m11	*Stat;
	TERN_m11                free_flag = FALSE_m11;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// check overall size
	if (sizeof(REC_Stat_v10_m11) != REC_Stat_v10_BYTES_m11)
		goto REC_Stat_v10_NOT_ALIGNED_m11;

	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(REC_Stat_v10_BYTES_m11);
		free_flag = TRUE_m11;
	}
	Stat = (REC_Stat_v10_m11 *) bytes;
	if (&Stat->minimum != (si4 *) (bytes + REC_Stat_v10_MINIMUM_OFFSET_m11))
		goto REC_Stat_v10_NOT_ALIGNED_m11;
	if (&Stat->maximum != (si4 *) (bytes + REC_Stat_v10_MAXIMUM_OFFSET_m11))
		goto REC_Stat_v10_NOT_ALIGNED_m11;
	if (&Stat->mean != (si4 *) (bytes + REC_Stat_v10_MEAN_OFFSET_m11))
		goto REC_Stat_v10_NOT_ALIGNED_m11;
	if (&Stat->median != (si4 *) (bytes + REC_Stat_v10_MEDIAN_OFFSET_m11))
		goto REC_Stat_v10_NOT_ALIGNED_m11;
	if (&Stat->mode != (si4 *) (bytes + REC_Stat_v10_MODE_OFFSET_m11))
		goto REC_Stat_v10_NOT_ALIGNED_m11;
	if (&Stat->variance != (sf4 *) (bytes + REC_Stat_v10_VARIANCE_OFFSET_m11))
		goto REC_Stat_v10_NOT_ALIGNED_m11;
	if (&Stat->skewness != (sf4 *) (bytes + REC_Stat_v10_SKEWNESS_OFFSET_m11))
		goto REC_Stat_v10_NOT_ALIGNED_m11;
	if (&Stat->kurtosis != (sf4 *) (bytes + REC_Stat_v10_KURTOSIS_OFFSET_m11))
		goto REC_Stat_v10_NOT_ALIGNED_m11;

	// aligned
	if (free_flag == TRUE_m11)
		free((void *) bytes);

	if (globals_m11->verbose == TRUE_m11)
		printf_m11("%s(): REC_Stat_v10_m11 structure is aligned\n", __FUNCTION__);

	return(TRUE_m11);

	// not aligned
REC_Stat_v10_NOT_ALIGNED_m11:

	if (free_flag == TRUE_m11)
		free((void *) bytes);

	error_message_m11("%s(): REC_Stat_v10_m11 structure is NOT aligned\n", __FUNCTION__);

	return(FALSE_m11);
}


//*************************************************************************************//
//********************************   Note: Note Record   ******************************//
//*************************************************************************************//

void	show_rec_Note_type_m11(RECORD_HEADER_m11 *record_header)
{
	si1	*note_text;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		if (record_header->total_record_bytes > RECORD_HEADER_BYTES_m11) {
			note_text = (si1 *) record_header + RECORD_HEADER_BYTES_m11;
			if (*note_text)
				UTF8_printf_m11("Note Text: %s\n", note_text);
			else
				printf_m11("Note Text: no entry\n");
		} else {
			printf_m11("Note Text: no entry\n");
		}
	}
	// Unrecognized record version
	else {
		error_message_m11("%s(): Unrecognized Note Record version (%hhd.%hhd)\n", __FUNCTION__, record_header->version_major, record_header->version_minor);
	}

	return;
}


TERN_m11        check_rec_Note_type_alignment_m11(ui1 *bytes)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// no structures to check
	return(TRUE_m11);
}


//*************************************************************************************//
//******************   EDFA: European Data Format Annotation Record   *****************//
//*************************************************************************************//

void	show_rec_EDFA_type_m11(RECORD_HEADER_m11 *record_header)
{
	REC_EDFA_v10_m11	*edfa;
	si1			*annotation;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		edfa = (REC_EDFA_v10_m11 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m11);
		printf_m11("Duration %ld microseconds\n", edfa->duration);
		annotation = (si1 *) edfa + REC_EDFA_v10_ANNOTATION_OFFSET_m11;
		if (*annotation)
			UTF8_printf_m11("Annotation: %s\n", annotation);
		else
			printf_m11("Annotation: no entry\n");
	}
	// Unrecognized record version
	else {
		error_message_m11("%s(): Unrecognized EDFA Record version (%hhd.%hhd)\n", __FUNCTION__, record_header->version_major, record_header->version_minor);
	}

	return;
}


TERN_m11	check_rec_EDFA_type_alignment_m11(ui1 *bytes)
{
	extern GLOBALS_m11	*globals_m11;
	REC_EDFA_v10_m11	*edfa;
	TERN_m11		free_flag = FALSE_m11;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// check overall size
	if (sizeof(REC_EDFA_v10_m11) != REC_EDFA_v10_BYTES_m11)
		goto REC_EDFA_v10_NOT_ALIGNED_m11;

	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(REC_EDFA_v10_BYTES_m11);
		free_flag = TRUE_m11;
	}
	edfa = (REC_EDFA_v10_m11 *) bytes;
	if (&edfa->duration != (si8 *) (bytes + REC_EDFA_v10_DURATION_OFFSET_m11))
		goto REC_EDFA_v10_NOT_ALIGNED_m11;

	// aligned
	if (free_flag == TRUE_m11)
		free((void *) bytes);

	if (globals_m11->verbose == TRUE_m11)
		printf_m11("%s(): REC_EDFA_v10_m11 structure is aligned\n", __FUNCTION__);

	return(TRUE_m11);

	// not aligned
REC_EDFA_v10_NOT_ALIGNED_m11:

	if (free_flag == TRUE_m11)
		free((void *) bytes);

	error_message_m11("%s(): REC_EDFA_v10_m11 structure is NOT aligned\n", __FUNCTION__);

	return(FALSE_m11);
}



//*************************************************************************************//
//*******************************   Seiz: Seizure Record   ****************************//
//*************************************************************************************//

void	show_rec_Seiz_type_m11(RECORD_HEADER_m11 *record_header)
{
	extern GLOBALS_m11		*globals_m11;
	si4			        i;
	TERN_m11                        mn1 = FALSE_m11, mn2 = FALSE_m11;
	REC_Seiz_v10_m11		*Seiz;
	REC_Seiz_v10_CHANNEL_m11	*chans;
	si1			        time_str[TIME_STRING_BYTES_m11];

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		Seiz = (REC_Seiz_v10_m11 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m11);
		time_string_m11(Seiz->latest_offset_time, time_str, TRUE_m11, FALSE_m11, FALSE_m11);
		printf_m11("Latest Offset Time: %ld (oUTC), %ld (µUTC), %s\n", Seiz->latest_offset_time, Seiz->latest_offset_time + globals_m11->recording_time_offset, time_str);
		printf_m11("Number of Channels: %d\n", Seiz->number_of_channels);
		printf_m11("Onset Code: %d ", Seiz->onset_code);
		switch (Seiz->onset_code) {
		case REC_Seiz_v10_ONSET_NO_ENTRY_m11:
			printf_m11("(no entry)\n");
			break;
		case REC_Seiz_v10_ONSET_UNKNOWN_m11:
			printf_m11("(unknown)\n");
			break;
		case REC_Seiz_v10_ONSET_FOCAL_m11:
			printf_m11("(focal)\n");
			break;
		case REC_Seiz_v10_ONSET_GENERALIZED_m11:
			printf_m11("(generalized)\n");
			break;
		case REC_Seiz_v10_ONSET_PROPAGATED_m11:
			printf_m11("(propagated)\n");
			break;
		case REC_Seiz_v10_ONSET_MIXED_m11:
			printf_m11("(mixed)\n");
			break;
		default:
			warning_message_m11("%s(): %d is an unrecognized Seiz onset code", __FUNCTION__, Seiz->onset_code);
			break;
		}
		if (strlen(Seiz->marker_name_1))
			mn1 = TRUE_m11;
		if (strlen(Seiz->marker_name_2))
			mn2 = TRUE_m11;
		if (mn1 == TRUE_m11 && mn2 == TRUE_m11)
			UTF8_printf_m11("Marker Names: %s %s\n", Seiz->marker_name_1, Seiz->marker_name_2);
		else if (mn1 == TRUE_m11)
			UTF8_printf_m11("Marker Name 1: %s\nMarker Name 2: no entry\n", Seiz->marker_name_1);
		else if (mn2 == TRUE_m11)
			UTF8_printf_m11("Marker Name 1: no_entry\nMarker Name 2: %s\n", Seiz->marker_name_2);
		else
			printf_m11("Marker Names: no_entry\n");
		if (strlen(Seiz->annotation))
			UTF8_printf_m11("Annotation: %s\n", Seiz->annotation);
		else
			printf_m11("Annotation: no entry\n");
		chans = (REC_Seiz_v10_CHANNEL_m11 *) ((ui1 *) Seiz + REC_Seiz_v10_CHANNELS_OFFSET_m11);
		for (i = 0; i < Seiz->number_of_channels; ++i) {
			if (strlen(chans[i].name))
				UTF8_printf_m11("Channel Name: %s\n", chans[i].name);
			else
				printf_m11("Channel Name: no entry\n");
			time_string_m11(chans[i].onset_time, time_str, TRUE_m11, FALSE_m11, FALSE_m11);
			printf_m11("\tOnset Time: %ld (oUTC), %ld (µUTC), %s\n", chans[i].onset_time, chans[i].onset_time + globals_m11->recording_time_offset, time_str);
			time_string_m11(chans[i].offset_time, time_str, TRUE_m11, FALSE_m11, FALSE_m11);
			printf_m11("\tOffset Time: %ld (oUTC), %ld (µUTC), %s\n", chans[i].offset_time, chans[i].offset_time + globals_m11->recording_time_offset, time_str);
			if (chans[i].segment_number == REC_Seiz_v10_CHANNEL_SEGMENT_NUMBER_NO_ENTRY_m11)
				printf_m11("Segment Number: no entry\n");
			else
				printf_m11("Segment Number: %d\n", chans[i].segment_number);
		}
	}
	// Unrecognized record version
	else {
		error_message_m11("%s(): Unrecognized Seiz Record version (%hhd.%hhd)\n", __FUNCTION__, record_header->version_major, record_header->version_minor);
	}

	return;
}


TERN_m11	check_rec_Seiz_type_alignment_m11(ui1 *bytes)
{
	extern GLOBALS_m11		*globals_m11;
	REC_Seiz_v10_m11		*Seiz;
	REC_Seiz_v10_CHANNEL_m11	*chan;
	TERN_m11			free_flag = FALSE_m11;
	ui1				*chan_bytes;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// check overall sizes
	if (sizeof(REC_Seiz_v10_m11) != REC_Seiz_v10_BYTES_m11)
		goto REC_Seiz_v10_NOT_ALIGNED_m11;
	if (sizeof(REC_Seiz_v10_CHANNEL_m11) != REC_Seiz_v10_CHANNEL_BYTES_m11)
		goto REC_Seiz_v10_NOT_ALIGNED_m11;

	// check fields - base structure
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(REC_Seiz_v10_BYTES_m11 + REC_Seiz_v10_CHANNEL_BYTES_m11);
		free_flag = TRUE_m11;
	}
	Seiz = (REC_Seiz_v10_m11 *) bytes;
	if (&Seiz->latest_offset_time != (si8 *) (bytes + REC_Seiz_v10_LATEST_OFFSET_TIME_OFFSET_m11))
		goto REC_Seiz_v10_NOT_ALIGNED_m11;
	if (&Seiz->number_of_channels != (si4 *) (bytes + REC_Seiz_v10_NUMBER_OF_CHANNELS_OFFSET_m11))
		goto REC_Seiz_v10_NOT_ALIGNED_m11;
	if (&Seiz->onset_code != (si4 *) (bytes + REC_Seiz_v10_ONSET_CODE_OFFSET_m11))
		goto REC_Seiz_v10_NOT_ALIGNED_m11;
	if (Seiz->marker_name_1 != (si1 *) (bytes + REC_Seiz_v10_MARKER_NAME_1_OFFSET_m11))
		goto REC_Seiz_v10_NOT_ALIGNED_m11;
	if (Seiz->marker_name_2 != (si1 *) (bytes + REC_Seiz_v10_MARKER_NAME_2_OFFSET_m11))
		goto REC_Seiz_v10_NOT_ALIGNED_m11;
	if (Seiz->annotation != (si1 *) (bytes + REC_Seiz_v10_ANNOTATION_OFFSET_m11))
		goto REC_Seiz_v10_NOT_ALIGNED_m11;
	// check fields - channel structures
	chan_bytes = bytes + REC_Seiz_v10_CHANNELS_OFFSET_m11;
	chan = (REC_Seiz_v10_CHANNEL_m11 *) chan_bytes;
	if (chan->name != (si1 *) (chan_bytes + REC_Seiz_v10_CHANNEL_NAME_OFFSET_m11))
		goto REC_Seiz_v10_NOT_ALIGNED_m11;
	if (&chan->onset_time != (si8 *) (chan_bytes + REC_Seiz_v10_CHANNEL_ONSET_TIME_OFFSET_m11))
		goto REC_Seiz_v10_NOT_ALIGNED_m11;
	if (&chan->offset_time != (si8 *) (chan_bytes + REC_Seiz_v10_CHANNEL_OFFSET_TIME_OFFSET_m11))
		goto REC_Seiz_v10_NOT_ALIGNED_m11;
	if (&chan->segment_number != (si4 *) (chan_bytes + REC_Seiz_v10_CHANNEL_SEGMENT_NUMBER_OFFSET_m11))
		goto REC_Seiz_v10_NOT_ALIGNED_m11;
	if (chan->pad != (ui1 *) (chan_bytes + REC_Seiz_v10_CHANNEL_PAD_OFFSET_m11))
		goto REC_Seiz_v10_NOT_ALIGNED_m11;

	// aligned
	if (free_flag == TRUE_m11)
		free((void *) bytes);

	if (globals_m11->verbose == TRUE_m11)
		printf_m11("%s(): REC_Seiz_v10_m11 structure is aligned\n", __FUNCTION__);

	return(TRUE_m11);

	// not aligned
REC_Seiz_v10_NOT_ALIGNED_m11:

	if (free_flag == TRUE_m11)
		free((void *) bytes);

	error_message_m11("%s(): REC_Seiz_v10_m11 structure is NOT aligned\n", __FUNCTION__);

	return(FALSE_m11);
}


//*************************************************************************************//
//*****************************   SyLg: System Log Record   ***************************//
//*************************************************************************************//

void	show_rec_SyLg_type_m11(RECORD_HEADER_m11 *record_header)
{
	si1	*log_entry;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		log_entry = (si1 *) record_header + RECORD_HEADER_BYTES_m11;
		if (*log_entry)
			UTF8_printf_m11("System Log entry:\n%s\n", log_entry);
		else
			printf_m11("System Log entry: no entry\n");
	}
	// Unrecognized record version
	else {
		error_message_m11("%s(): Unrecognized SyLg Record version (%hhd.%hhd)\n", __FUNCTION__, record_header->version_major, record_header->version_minor);
	}

	return;
}


TERN_m11	check_rec_SyLg_type_alignment_m11(ui1 *bytes)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// no structures to check
	return(TRUE_m11);
}



//*************************************************************************************//
//*********************   NlxP: NeuraLynx Parallel Port Record   **********************//
//*************************************************************************************//

void    show_rec_NlxP_type_m11(RECORD_HEADER_m11 *record_header)
{
	si1                     hex_str[HEX_STRING_BYTES_m11(4)];
	REC_NlxP_v10_m11	*nlxp;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		nlxp = (REC_NlxP_v10_m11 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m11);
		printf_m11("Value: %u\n", nlxp->value);
		printf_m11("Subport: %hhu\n", nlxp->subport);
		printf_m11("Number of Subports: %hhu\n", nlxp->number_of_subports);
		printf_m11("Trigger Mode: ");
		switch (nlxp->trigger_mode) {
		case REC_NlxP_v10_NO_TRIGGER_m11:
			printf_m11("NO TRIGGER\n");
			break;
		case REC_NlxP_v10_ANY_BIT_CHANGE_m11:
			printf_m11("ANY BIT CHANGE\n");
			break;
		case REC_NlxP_v10_HIGH_BIT_SET_m11:
			printf_m11("HIGH BIT SET\n");
			break;
		default:
			warning_message_m11("%s(): Unrecognized trigger mode (%hhu)", __FUNCTION__, nlxp->trigger_mode);
			break;
		}
		printf_m11("Raw Port Value: %u  (unsigned dec)\n", nlxp->raw_port_value);
		generate_hex_string_m11((ui1 *) &nlxp->raw_port_value, 4, hex_str);
		printf_m11("Raw Port Bytes: %s  (hex)\n", hex_str);
	}
	// Unrecognized record version
	else {
		error_message_m11("%s(): Unrecognized NlxP Record version (%hhd.%hhd)\n", __FUNCTION__, record_header->version_major, record_header->version_minor);
	}

	return;
}


TERN_m11     check_rec_NlxP_type_alignment_m11(ui1 *bytes)
{
	extern GLOBALS_m11	*globals_m11;
	REC_NlxP_v10_m11	*nlxp;
	TERN_m11                free_flag = FALSE_m11;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// check overall size
	if (sizeof(REC_NlxP_v10_m11) != REC_NlxP_v10_BYTES_m11)
		goto REC_NlxP_v10_NOT_ALIGNED_m11;

	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(REC_NlxP_v10_BYTES_m11);
		free_flag = TRUE_m11;
	}

	nlxp = (REC_NlxP_v10_m11 *) bytes;
	if (&nlxp->raw_port_value != (ui4 *) (bytes + REC_NlxP_v10_RAW_PORT_VALUE_OFFSET_m11))
		goto REC_NlxP_v10_NOT_ALIGNED_m11;
	if (&nlxp->value != (ui4 *) (bytes + REC_NlxP_v10_VALUE_OFFSET_m11))
		goto REC_NlxP_v10_NOT_ALIGNED_m11;
	if (&nlxp->subport != (ui1 *) (bytes + REC_NlxP_v10_SUBPORT_OFFSET_m11))
		goto REC_NlxP_v10_NOT_ALIGNED_m11;
	if (&nlxp->number_of_subports != (ui1 *) (bytes + REC_NlxP_v10_NUMBER_OF_SUBPORTS_OFFSET_m11))
		goto REC_NlxP_v10_NOT_ALIGNED_m11;
	if (&nlxp->trigger_mode != (ui1 *) (bytes + REC_NlxP_v10_TRIGGER_MODE_OFFSET_m11))
		goto REC_NlxP_v10_NOT_ALIGNED_m11;
	if (nlxp->pad != (ui1 *) (bytes + REC_NlxP_v10_PAD_OFFSET_m11))
		goto REC_NlxP_v10_NOT_ALIGNED_m11;

	// aligned
	if (free_flag == TRUE_m11)
		free((void *) bytes);

	if (globals_m11->verbose == TRUE_m11)
		printf_m11("%s(): REC_NlxP_v10_m11 structure is aligned\n", __FUNCTION__);

	return(TRUE_m11);

	// not aligned
REC_NlxP_v10_NOT_ALIGNED_m11:

	if (free_flag == TRUE_m11)
		free((void *) bytes);

	error_message_m11("%s(): REC_NlxP_v10_m11 structure is NOT aligned\n", __FUNCTION__);

	return(FALSE_m11);

}


//*************************************************************************************//
//***********************   Curs: Cadwell EMG Cursor Annotation   *********************//
//*************************************************************************************//

void    show_rec_Curs_type_m11(RECORD_HEADER_m11 *record_header)
{
	REC_Curs_v10_m11	*curs;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		curs = (REC_Curs_v10_m11 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m11);
		printf_m11("ID Number: %ld\n", curs->id_number);
		printf_m11("Latency: %ld\n", curs->latency);
		printf_m11("Value: %lf\n", curs->value);
		UTF8_printf_m11("Name: %s\n", curs->name);
	}
	// Unrecognized record version
	else {
		error_message_m11("%s(): Unrecognized Curs Record version (%hhd.%hhd)\n", __FUNCTION__, record_header->version_major, record_header->version_minor);
	}

	return;
}


TERN_m11     check_rec_Curs_type_alignment_m11(ui1 *bytes)
{
	extern GLOBALS_m11	*globals_m11;
	REC_Curs_v10_m11	*curs;
	TERN_m11                free_flag = FALSE_m11;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// check overall size
	if (sizeof(REC_Curs_v10_m11) != REC_Curs_v10_BYTES_m11)
		goto REC_Curs_v10_NOT_ALIGNED_m11;

	// check fields
	if (bytes == NULL) {
		bytes = (ui1*) malloc(REC_Curs_v10_BYTES_m11);
		free_flag = TRUE_m11;
	}

	curs = (REC_Curs_v10_m11 *) bytes;
	if (&curs->id_number != (si8 *) (bytes + REC_Curs_v10_ID_NUMBER_OFFSET_m11))
		goto REC_Curs_v10_NOT_ALIGNED_m11;
	if (&curs->latency != (si8 *) (bytes + REC_Curs_v10_LATENCY_OFFSET_m11))
		goto REC_Curs_v10_NOT_ALIGNED_m11;
	if (&curs->value != (sf8 *) (bytes + REC_Curs_v10_VALUE_OFFSET_m11))
		goto REC_Curs_v10_NOT_ALIGNED_m11;
	if (curs->name != (si1 *) (bytes + REC_Curs_v10_NAME_OFFSET_m11))
		goto REC_Curs_v10_NOT_ALIGNED_m11;

	// aligned
	if (free_flag == TRUE_m11)
		free((void *) bytes);

	if (globals_m11->verbose == TRUE_m11)
		printf_m11("%s(): REC_Curs_v10_m11 structure is aligned\n", __FUNCTION__);

	return(TRUE_m11);

	// not aligned
REC_Curs_v10_NOT_ALIGNED_m11:

	if (free_flag == TRUE_m11)
		free((void *) bytes);

	error_message_m11("%s(): REC_Curs_v10_m11 structure is NOT aligned\n", __FUNCTION__);

	return(FALSE_m11);

}


//*************************************************************************************//
//****************************   Epoc: Sleep Stage Record   ***************************//
//*************************************************************************************//

void    show_rec_Epoc_type_m11(RECORD_HEADER_m11 *record_header)
{
	REC_Epoc_v10_m11	*epoc;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		epoc = (REC_Epoc_v10_m11 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m11);
		printf_m11("ID Number: %ld\n", epoc->id_number);
		printf_m11("End Time: %ld\n", epoc->end_time);
		UTF8_printf_m11("Epoch Type: %s\n", epoc->epoch_type);
		UTF8_printf_m11("Text: %s\n", epoc->text);
	}
	// Unrecognized record version
	else {
		error_message_m11("%s(): Unrecognized Curs Record version (%hhd.%hhd)\n", __FUNCTION__, record_header->version_major, record_header->version_minor);
	}

	return;
}


TERN_m11     check_rec_Epoc_type_alignment_m11(ui1 *bytes)
{
	extern GLOBALS_m11	*globals_m11;
	REC_Epoc_v10_m11	*epoc;
	TERN_m11                free_flag = FALSE_m11;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// check overall size
	if (sizeof(REC_Epoc_v10_m11) != REC_Epoc_v10_BYTES_m11)
		goto REC_Epoc_v10_NOT_ALIGNED_m11;

	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(REC_Epoc_v10_BYTES_m11);
		free_flag = TRUE_m11;
	}

	epoc = (REC_Epoc_v10_m11 *) bytes;
	if (&epoc->id_number != (si8 *) (bytes + REC_Epoc_v10_ID_NUMBER_OFFSET_m11))
		goto REC_Epoc_v10_NOT_ALIGNED_m11;
	if (&epoc->end_time != (si8 *) (bytes + REC_Epoc_v10_END_TIME_OFFSET_m11))
		goto REC_Epoc_v10_NOT_ALIGNED_m11;
	if (epoc->epoch_type != (si1 *) (bytes + REC_Epoc_v10_EPOCH_TYPE_OFFSET_m11))
		goto REC_Epoc_v10_NOT_ALIGNED_m11;
	if (epoc->text != (si1 *) (bytes + REC_Epoc_v10_TEXT_OFFSET_m11))
		goto REC_Epoc_v10_NOT_ALIGNED_m11;

	// aligned
	if (free_flag == TRUE_m11)
		free((void *) bytes);

	if (globals_m11->verbose == TRUE_m11)
		printf_m11("%s(): REC_Epoc_v10_m11 structure is aligned\n", __FUNCTION__);

	return(TRUE_m11);

	// not aligned
REC_Epoc_v10_NOT_ALIGNED_m11:

	if (free_flag == TRUE_m11)
		free((void *) bytes);

	error_message_m11("%s(): REC_Epoc_v10_m11 structure is NOT aligned\n", __FUNCTION__);

	return(FALSE_m11);

}


//*************************************************************************************//
//**************************   ESti: Electrical Stimulation   *************************//
//*************************************************************************************//

void    show_rec_ESti_type_m11(RECORD_HEADER_m11 *record_header)
{
	REC_ESti_v10_m11	*esti;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		esti = (REC_ESti_v10_m11 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m11);
		printf_m11("Amplitude: %lf ", esti->amplitude);
		switch (esti->amp_unit_code) {
			case REC_ESti_v10_AMP_UNIT_MA_m11:
				printf_m11("mA\n");
				break;
			case REC_ESti_v10_AMP_UNIT_V_m11:
				printf_m11("V\n");
				break;
			case REC_ESti_v10_AMP_UNIT_NO_ENTRY_m11:
				printf_m11("(units no entry)\n");
				break;
			case REC_ESti_v10_AMP_UNIT_UNKNOWN_m11:
				printf_m11("(units unknown)\n");
				break;
			default:
				printf_m11("(unrecognized units code: %d)\n", esti->amp_unit_code);
				break;
		}
		printf_m11("Frequency: %lf (Hz)\n", esti->frequency);
		printf_m11("Pulse Width: %ld (µS)\n", esti->pulse_width);
		printf_m11("Mode: ");
		switch (esti->mode_code) {
			case REC_ESti_v10_MODE_CURRENT_m11:
				printf_m11("constant current\n");
				break;
			case REC_ESti_v10_MODE_VOLTAGE_m11:
				printf_m11("constant voltage\n");
				break;
			case REC_ESti_v10_MODE_NO_ENTRY_m11:
				printf_m11("no entry\n");
				break;
			case REC_ESti_v10_MODE_UNKNOWN_m11:
				printf_m11("unknown\n");
				break;
			default:
				printf_m11("unrecognized mode code (%d)\n", esti->mode_code);
				break;
		}

		UTF8_printf_m11("Waveform: %s\n", esti->waveform);
		UTF8_printf_m11("Anode: %s\n", esti->anode);
		UTF8_printf_m11("Cathode: %s\n", esti->cathode);
	}
	// Unrecognized record version
	else {
		error_message_m11("%s(): Unrecognized ESti Record version (%hhd.%hhd)\n", __FUNCTION__, record_header->version_major, record_header->version_minor);
	}

	return;
}


TERN_m11     check_rec_ESti_type_alignment_m11(ui1 *bytes)
{
	extern GLOBALS_m11	*globals_m11;
	REC_ESti_v10_m11	*esti;
	TERN_m11                free_flag = FALSE_m11;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// check overall size
	if (sizeof(REC_ESti_v10_m11) != REC_ESti_v10_BYTES_m11)
		goto REC_ESti_v10_NOT_ALIGNED_m11;

	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(REC_ESti_v10_BYTES_m11);
		free_flag = TRUE_m11;
	}

	esti = (REC_ESti_v10_m11 *) bytes;
	if (&esti->amplitude != (sf8 *) (bytes + REC_ESti_v10_AMPLITUDE_OFFSET_m11))
		goto REC_ESti_v10_NOT_ALIGNED_m11;
	if (&esti->frequency != (sf8 *) (bytes + REC_ESti_v10_FREQUENCY_OFFSET_m11))
		goto REC_ESti_v10_NOT_ALIGNED_m11;
	if (&esti->pulse_width != (si8 *) (bytes + REC_ESti_v10_PULSE_WIDTH_OFFSET_m11))
		goto REC_ESti_v10_NOT_ALIGNED_m11;
	if (&esti->amp_unit_code != (si4 *) (bytes + REC_ESti_v10_AMP_UNIT_CODE_OFFSET_m11))
		goto REC_ESti_v10_NOT_ALIGNED_m11;
	if (&esti->mode_code != (si4 *) (bytes + REC_ESti_v10_MODE_CODE_OFFSET_m11))
		goto REC_ESti_v10_NOT_ALIGNED_m11;
	if (esti->waveform != (si1 *) (bytes + REC_ESti_v10_WAVEFORM_OFFSET_m11))
		goto REC_ESti_v10_NOT_ALIGNED_m11;
	if (esti->anode != (si1 *) (bytes + REC_ESti_v10_ANODE_OFFSET_m11))
		goto REC_ESti_v10_NOT_ALIGNED_m11;
	if (esti->cathode != (si1 *) (bytes + REC_ESti_v10_CATHODE_OFFSET_m11))
		goto REC_ESti_v10_NOT_ALIGNED_m11;

	// aligned
	if (free_flag == TRUE_m11)
		free((void *) bytes);

	if (globals_m11->verbose == TRUE_m11)
		printf_m11("%s(): REC_ESti_v10_m11 structure is aligned\n", __FUNCTION__);

	return(TRUE_m11);

	// not aligned
REC_ESti_v10_NOT_ALIGNED_m11:

	if (free_flag == TRUE_m11)
		free((void *) bytes);

	error_message_m11("%s(): REC_ESti_v10_m11 structure is NOT aligned\n", __FUNCTION__);

	return(FALSE_m11);

}


//*************************************************************************************//
//**************************   CSti: Cognitive Stimulation   **************************//
//*************************************************************************************//

void    show_rec_CSti_type_m11(RECORD_HEADER_m11 *record_header)
{
	REC_CSti_v10_m11	*csti;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		csti = (REC_CSti_v10_m11 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m11);
		printf_m11("Stimulus Duration: %ld (usecs)\n", csti->stimulus_duration);
		UTF8_printf_m11("Task Type: %s\n", csti->task_type);
		UTF8_printf_m11("Stimulus Type: %s\n", csti->stimulus_type);
		UTF8_printf_m11("Patient Response: %s\n", csti->patient_response);
	}
	// Unrecognized record version
	else {
		error_message_m11("%s(): Unrecognized CSti Record version (%hhd.%hhd)\n", __FUNCTION__, record_header->version_major, record_header->version_minor);
	}

	return;
}


TERN_m11     check_rec_CSti_type_alignment_m11(ui1 *bytes)
{
	extern GLOBALS_m11	*globals_m11;
	REC_CSti_v10_m11	*csti;
	TERN_m11                free_flag = FALSE_m11;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// check overall size
	if (sizeof(REC_CSti_v10_m11) != REC_CSti_v10_BYTES_m11)
		goto REC_CSti_v10_NOT_ALIGNED_m11;

	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(REC_CSti_v10_BYTES_m11);
		free_flag = TRUE_m11;
	}

	csti = (REC_CSti_v10_m11 *) bytes;
	if (&csti->stimulus_duration != (si8 *) (bytes + REC_CSti_v10_STIMULUS_DURATION_OFFSET_m11))
		goto REC_CSti_v10_NOT_ALIGNED_m11;
	if (csti->task_type != (si1 *) (bytes + REC_CSti_v10_TASK_TYPE_OFFSET_m11))
		goto REC_CSti_v10_NOT_ALIGNED_m11;
	if (csti->stimulus_type != (si1 *) (bytes + REC_CSti_v10_STIMULUS_TYPE_OFFSET_m11))
		goto REC_CSti_v10_NOT_ALIGNED_m11;
	if (csti->patient_response != (si1 *) (bytes + REC_CSti_v10_PATIENT_RESPONSE_OFFSET_m11))
		goto REC_CSti_v10_NOT_ALIGNED_m11;

	// aligned
	if (free_flag == TRUE_m11)
		free((void *) bytes);

	if (globals_m11->verbose == TRUE_m11)
		printf_m11("%s(): REC_CSti_v10_m11 structure is aligned\n", __FUNCTION__);

	return(TRUE_m11);

	// not aligned
REC_CSti_v10_NOT_ALIGNED_m11:

	if (free_flag == TRUE_m11)
		free((void *) bytes);

	error_message_m11("%s(): REC_CSti_v10_m11 structure is NOT aligned\n", __FUNCTION__);

	return(FALSE_m11);

}


//*************************************************************************************//
//********************************   New Record Type   ********************************//
//*************************************************************************************//



