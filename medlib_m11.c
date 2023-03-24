
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
// Only library versions released on or afer a minor version will make use of new fields, and only if the minor version of the files contains them.
// Backward compatibility will be maintained between major versions if practical.



#include "medlib_m11.h"
#include "medrec_m11.h"

// Globals
GLOBALS_m11		*globals_m11 = NULL;
pthread_mutex_t_m11	globals_m11_mutex;



//***********************************************************************//
//********************  GENERAL MED LIBRARY FUNCTIONS  ******************//
//***********************************************************************//


TERN_m11	adjust_open_file_limit_m11(si4 new_limit, TERN_m11 verbose_flag)
{
	TERN_m11	ret_val = TRUE_m11;
#if defined MACOS_m11 || defined LINUX_m11
	struct rlimit	resource_limit;
#endif

	// verbose_flag passed because this function is usually called before the MED libraries are initialized
	
	#if defined MACOS_m11 || defined LINUX_m11
	// change resource limits (note: must change before calling any functions that use system resources)
	getrlimit(RLIMIT_NOFILE, &resource_limit);  // get existing limit set
	resource_limit.rlim_cur = (rlim_t) new_limit;  // change open file limit
	if (setrlimit(RLIMIT_NOFILE, &resource_limit) == -1)  // set limit set
		ret_val = FALSE_m11;
	#endif
	
	#ifdef WINDOWS_m11
	if (_setmaxstdio((int) new_limit) == -1)  // change open file limit
		ret_val = FALSE_m11;
	#endif

#ifdef FN_DEBUG_m11  // don't print until resources changed
	#ifdef MATLAB_m11
	mexPrintf("%s()\n", __FUNCTION__);
	#else
	printf("%s()\n", __FUNCTION__);
	#endif
#endif

	if (ret_val == FALSE_m11) {
		if (verbose_flag == TRUE_m11) {
			#ifdef MATLAB_m11
			mexPrintf("%s(): could not adjust process open file limit\n", __FUNCTION__);
			#else
			fprintf(stderr, "%s(): could not adjust process open file limit\n", __FUNCTION__);
			#endif
		}
	}

	return(ret_val);
}


TERN_m11	all_zeros_m11(ui1 *bytes, si4 field_length)
{
	while (field_length--)
		if (*bytes++)
			return(FALSE_m11);
	
	return(TRUE_m11);
}


si1	*behavior_string_m11(ui4 behavior, si1 *behavior_string)
{
	si8	len;
	
	
	if (behavior_string == NULL)  // caller responsible for freeing
		behavior_string = malloc_m11(256, __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	*behavior_string = 0;
	
	if (behavior == USE_GLOBAL_BEHAVIOR_m11) {
		behavior = globals_m11->behavior_on_fail;
		strcat(behavior_string, "USE GLOBAL BEHAVIOR == ");
	}
	
	if (behavior & EXIT_ON_FAIL_m11)
		strcat(behavior_string, "EXIT ON FAIL | ");
	if (behavior & RETURN_ON_FAIL_m11)
		strcat(behavior_string, "RETURN ON FAIL | ");
	if (behavior & SUPPRESS_ERROR_OUTPUT_m11)
		strcat(behavior_string, "SUPPRESS ERROR OUTPUT | ");
	if (behavior & SUPPRESS_WARNING_OUTPUT_m11)
		strcat(behavior_string, "SUPPRESS WARNING OUTPUT | ");
	if (behavior & SUPPRESS_MESSAGE_OUTPUT_m11)
		strcat(behavior_string, "SUPPRESS MESSAGE OUTPUT | ");
	if (behavior & RETRY_ONCE_m11)
		strcat(behavior_string, "RETRY ONCE | ");

	len = strlen(behavior_string);
	if (len)
		behavior_string[len - 3] = 0;
		
	return(behavior_string);
}


si8	build_contigua_m11(LEVEL_HEADER_m11 *level_header)
{
	TERN_m11				force_discont;
	si1					tmp_str[FULL_FILE_NAME_BYTES_m11];
	ui4					type_code;
	si4					n_segs, seg_idx, search_mode;
	si8					i, j, k, *n_contigua, start_idx, end_idx, last_block_samples, last_block_frames;
	si8					last_block_usecs, curr_bytes, new_bytes, absolute_numbering_offset;
	si8					last_segment_end_sample_number, last_segment_end_frame_number, last_segment_end_time, last_segment_number;
	sf8					samp_freq, frame_rate;
	SEGMENT_m11				*seg;
	CHANNEL_m11				*chan;
	SESSION_m11				*sess;
	TIME_SLICE_m11				*slice;
	TIME_SERIES_METADATA_SECTION_2_m11	*tmd2;
	VIDEO_METADATA_SECTION_2_m11		*vmd2;
	CONTIGUON_m11				**contigua, *contiguon;
	TIME_SERIES_INDEX_m11			*tsi;
	VIDEO_INDEX_m11				*vi;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif

	// find contigua in current time slice, and set in level
	
	switch (level_header->type_code) {
		case LH_TIME_SERIES_SEGMENT_m11:
			seg = (SEGMENT_m11 *) level_header;
			slice = &seg->time_slice;
			contigua = &seg->contigua;
			n_contigua = &seg->number_of_contigua;
			chan = NULL;
			type_code = LH_TIME_SERIES_CHANNEL_m11;
			break;
		case LH_VIDEO_SEGMENT_m11:
			seg = (SEGMENT_m11 *) level_header;
			slice = &seg->time_slice;
			contigua = &seg->contigua;
			n_contigua = &seg->number_of_contigua;
			chan = NULL;
			type_code = LH_VIDEO_CHANNEL_m11;
			break;
		case LH_TIME_SERIES_CHANNEL_m11:
			chan = (CHANNEL_m11 *) level_header;
			slice = &chan->time_slice;
			contigua = &chan->contigua;
			n_contigua = &chan->number_of_contigua;
			type_code = LH_TIME_SERIES_CHANNEL_m11;
			break;
		case LH_VIDEO_CHANNEL_m11:
			chan = (CHANNEL_m11 *) level_header;
			slice = &chan->time_slice;
			contigua = &chan->contigua;
			n_contigua = &chan->number_of_contigua;
			type_code = LH_VIDEO_CHANNEL_m11;
			break;
		case LH_SESSION_m11:
			sess = (SESSION_m11 *) level_header;
			slice = &sess->time_slice;
			contigua = &sess->contigua;
			n_contigua = &sess->number_of_contigua;
			chan = globals_m11->reference_channel;
			type_code = chan->type_code;
			break;
		default:
			warning_message_m11("%s(): invalid level type\n", __FUNCTION__);
			return(FALSE_m11);
	}
	
	seg_idx = get_segment_index_m11(slice->start_segment_number);
	if (seg_idx == FALSE_m11)
		return(FALSE_m11);
	n_segs = TIME_SLICE_SEGMENT_COUNT_m11(slice);
	if ((search_mode = get_search_mode_m11(slice)) == FALSE_m11)
		return(FALSE_m11);
	if (*contigua != NULL) {
		free_m11((void *) *contigua, __FUNCTION__);
		*contigua = NULL;
	}
	*n_contigua = 0;
	
	force_discont = TRUE_m11;
	if (type_code == LH_TIME_SERIES_CHANNEL_m11) {
		for (i = 0, j = seg_idx; i < n_segs; ++i, ++j) {
			if (chan != NULL)
				seg = chan->segments[j];
			if (seg == NULL) {  // segment missing
				force_discont = TRUE_m11;
				continue;
			}

			// get metadata
			if (seg->metadata_fps == NULL) {
				sprintf_m11(tmp_str, "%s/%s.%s", seg->path, seg->name, TIME_SERIES_METADATA_FILE_TYPE_STRING_m11);
				if (file_exists_m11(tmp_str) == FILE_EXISTS_m11) {
					seg->metadata_fps = read_file_m11(NULL, tmp_str, 0, 0, FPS_FULL_FILE_m11, (LEVEL_HEADER_m11 *) seg, NULL, USE_GLOBAL_BEHAVIOR_m11);
				} else {
					force_discont = TRUE_m11;
					continue;
				}
			}
			tmd2 = &seg->metadata_fps->metadata->time_series_section_2;
			samp_freq = tmd2->sampling_frequency;
			absolute_numbering_offset = tmd2->absolute_start_sample_number;

			// get indices
			if (seg->time_series_indices_fps == NULL) {
				sprintf_m11(tmp_str, "%s/%s.%s", seg->path, seg->name, TIME_SERIES_INDICES_FILE_TYPE_STRING_m11);
				if (file_exists_m11(tmp_str) == FILE_EXISTS_m11) {
					seg->time_series_indices_fps = read_file_m11(NULL, tmp_str, 0, 0, FPS_FULL_FILE_m11, (LEVEL_HEADER_m11 *) seg, NULL, USE_GLOBAL_BEHAVIOR_m11);
				} else {
					force_discont = TRUE_m11;
					continue;
				}
			}

			// build contigua
			tsi = seg->time_series_indices_fps->time_series_indices;
			if (search_mode == TIME_SEARCH_m11) {
				start_idx = find_index_m11(seg, slice->start_time, TIME_SEARCH_m11 | NO_OVERFLOWS_m11);
				end_idx = find_index_m11(seg, slice->end_time, TIME_SEARCH_m11 | NO_OVERFLOWS_m11);
			} else {
				start_idx = find_index_m11(seg, slice->start_sample_number, SAMPLE_SEARCH_m11 | NO_OVERFLOWS_m11);
				end_idx = find_index_m11(seg, slice->end_sample_number, SAMPLE_SEARCH_m11 | NO_OVERFLOWS_m11);
			}
			
			for (k = start_idx; k <= end_idx; ++k) {
				if (tsi[k].file_offset < 0 || force_discont == TRUE_m11) {
					// close last contiguon
					if (*n_contigua) {
						contiguon = *contigua + *n_contigua - 1;
						contiguon->end_sample_number = (tsi[k].start_sample_number + absolute_numbering_offset) - 1;
						contiguon->end_segment_number = j + 1;
						if (k) {
							contiguon->end_sample_number = (tsi[k].start_sample_number + absolute_numbering_offset) - 1;
							last_block_samples = tsi[k].start_sample_number - tsi[k - 1].start_sample_number;
							last_block_usecs = (si8) round(((sf8) last_block_samples / samp_freq) * (sf8) 1e6);
							contiguon->end_time = (tsi[k - 1].start_time + last_block_usecs) - 1;
						} else {  // discontinuity on segment transition
							contiguon->end_sample_number = last_segment_end_sample_number;
							contiguon->end_time = last_segment_end_time;
							contiguon->end_segment_number = last_segment_number;
						}
					}
					// open new contiguon
					curr_bytes = (size_t) *n_contigua * sizeof(CONTIGUON_m11);
					new_bytes = curr_bytes + sizeof(CONTIGUON_m11);
					*contigua = (CONTIGUON_m11 *) recalloc_m11((void *) *contigua, curr_bytes, new_bytes, __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
					contiguon = *contigua + *n_contigua;
					++(*n_contigua);
					contiguon->start_sample_number = tsi[k].start_sample_number + absolute_numbering_offset;
					contiguon->start_time = tsi[k].start_time;
					contiguon->start_segment_number = j + 1;

					force_discont = FALSE_m11;
				}
			}
			last_segment_end_sample_number = (tsi[k].start_sample_number + absolute_numbering_offset) - 1;
			last_segment_end_time = tsi[k].start_time - 1;
			last_segment_number = j + 1;
		}
		
		// close final contiguon
		contiguon->end_sample_number = (tsi[k].start_sample_number + absolute_numbering_offset) - 1;  // k == next index of last set of indices
		contiguon->end_time = tsi[k].start_time - 1;  // next index start time
		contiguon->end_segment_number = j;

	} else {  // LH_VIDEO_CHANNEL_m11
		for (i = 0, j = seg_idx; i < n_segs; ++i, ++j) {
			if (chan != NULL)
				seg = chan->segments[j];
			if (seg == NULL) {  // segment missing
				force_discont = TRUE_m11;
				continue;
			}
			// get metadata
			if (seg->metadata_fps == NULL) {
				sprintf_m11(tmp_str, "%s/%s.%s", seg->path, seg->name, VIDEO_METADATA_FILE_TYPE_STRING_m11);
				if (file_exists_m11(tmp_str) == FILE_EXISTS_m11) {
					seg->metadata_fps = read_file_m11(NULL, tmp_str, 0, 0, FPS_FULL_FILE_m11, (LEVEL_HEADER_m11 *) seg, NULL, USE_GLOBAL_BEHAVIOR_m11);
				} else {
					force_discont = TRUE_m11;
					continue;
				}
			}
			vmd2 = &seg->metadata_fps->metadata->video_section_2;
			frame_rate = vmd2->frame_rate;
			absolute_numbering_offset = vmd2->absolute_start_frame_number;
			
			// get indices
			if (seg->time_series_indices_fps == NULL) {
				sprintf_m11(tmp_str, "%s/%s.%s", seg->path, seg->name, VIDEO_INDICES_FILE_TYPE_STRING_m11);
				if (file_exists_m11(tmp_str) == FILE_EXISTS_m11) {
					seg->video_indices_fps = read_file_m11(NULL, tmp_str, 0, 0, FPS_FULL_FILE_m11, (LEVEL_HEADER_m11 *) seg, NULL, USE_GLOBAL_BEHAVIOR_m11);
				} else {
					force_discont = TRUE_m11;
					continue;
				}
			}

			// build contigua
			vi = seg->video_indices_fps->video_indices;
			if (search_mode == TIME_SEARCH_m11) {
				start_idx = find_index_m11(seg, slice->start_time, TIME_SEARCH_m11 | NO_OVERFLOWS_m11);
				end_idx = find_index_m11(seg, slice->end_time, TIME_SEARCH_m11 | NO_OVERFLOWS_m11);
			} else {
				start_idx = find_index_m11(seg, slice->start_frame_number, SAMPLE_SEARCH_m11 | NO_OVERFLOWS_m11);
				end_idx = find_index_m11(seg, slice->end_frame_number, SAMPLE_SEARCH_m11 | NO_OVERFLOWS_m11);
			}
			for (k = start_idx; k <= end_idx; ++k) {
				if (vi[k].file_offset < 0 || force_discont == TRUE_m11) {
					// close last contiguon
					if (*n_contigua) {
						contiguon->end_frame_number = ((si8) vi[k].start_frame_number + absolute_numbering_offset) - 1;
						contiguon->end_segment_number = j + 1;
						// end time
						if (k) {
							last_block_frames = vi[k].start_frame_number - vi[k - 1].start_frame_number;
							last_block_usecs = (si8) round(((sf8) last_block_frames / frame_rate) * (sf8) 1e6);
							contiguon->end_time = (vi[k - 1].start_time + last_block_usecs) - 1;
						} else {  // discontinuity on segment transition
							contiguon->end_frame_number = last_segment_end_frame_number;
							contiguon->end_time = last_segment_end_time;
							contiguon->end_segment_number = last_segment_number;
						}
					}
					// open new contiguon
					curr_bytes = (size_t) *n_contigua * sizeof(CONTIGUON_m11);
					new_bytes = curr_bytes + sizeof(CONTIGUON_m11);
					*contigua = (CONTIGUON_m11 *) recalloc_m11((void *) *contigua, curr_bytes, new_bytes, __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
					contiguon = *contigua + *n_contigua;
					++(*n_contigua);
					contiguon->start_frame_number = vi[k].start_frame_number + absolute_numbering_offset;
					contiguon->start_time = vi[k].start_time;
					contiguon->start_segment_number = j + 1;
					
					force_discont = FALSE_m11;
				}
			}
			last_segment_end_frame_number = (vi[k].start_frame_number + absolute_numbering_offset) - 1;
			last_segment_end_time = vi[k].start_time - 1;
			last_segment_number = j + 1;
		}
		
		// close final contiguon
		contiguon->end_frame_number = ((si8) vi[k].start_frame_number + absolute_numbering_offset) - 1;  // k == next index of last set of indices
		contiguon->end_time = vi[k].start_time - 1;  // next index start time
		contiguon->end_segment_number = j;
	}
	
	if (*n_contigua == 0) {
		free_m11((void *) *contigua, __FUNCTION__);
		*contigua = NULL;
		return(FALSE_m11);
	}
	
	// trim contigua ends to slice (sample_number == frame_number)
	if ((*contigua)[0].start_time < slice->start_time)
		(*contigua)[0].start_time = slice->start_time;
	if ((*contigua)[*n_contigua - 1].end_time > slice->end_time)
		(*contigua)[*n_contigua - 1].end_time = slice->end_time;
	if ((*contigua)[0].start_sample_number < slice->start_sample_number)
		(*contigua)[0].start_sample_number = slice->start_sample_number;
	if ((*contigua)[*n_contigua - 1].end_sample_number > slice->end_sample_number)
		(*contigua)[*n_contigua - 1].end_sample_number = slice->end_sample_number;
	
	// set sample/frame numbers to NO ENTRY for variable frequency sessions
	if (level_header->type_code == LH_SESSION_m11) {
		if (globals_m11->time_series_frequencies_vary == TRUE_m11) {
			for (i = 0; i < *n_contigua; ++i)
				(*contigua)[i].start_sample_number = (*contigua)[i].end_sample_number = SAMPLE_NUMBER_NO_ENTRY_m11;  // sample_number == frame_number
		}
	}
	
	return(*n_contigua);
}


Sgmt_RECORD_m11	*build_Sgmt_records_array_m11(FILE_PROCESSING_STRUCT_m11 *ri_fps, FILE_PROCESSING_STRUCT_m11 *rd_fps, CHANNEL_m11 *chan)
{
	TERN_m11			seek_mode;
	si1				**seg_list, *metadata_ext, tmp_str[FULL_FILE_NAME_BYTES_m11], seg_name[SEGMENT_BASE_FILE_NAME_BYTES_m11];
	si4				i, n_segs;
	si8				file_offset, data_len, n_recs, seek_data_size;
	const si8			SEEK_THRESHOLD = 10;  // this factor is a guess, for now
	FILE_PROCESSING_STRUCT_m11	*md_fps;
	RECORD_INDEX_m11		*ri;
	Sgmt_RECORD_m11			*Sgmt_records, *Sgmt_rec;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif

	// use Sgmt records
	if (ri_fps != NULL) {  // assume rd_fps != NULL
		// full record index file already read in
		n_recs = ri_fps->universal_header->number_of_entries;
		if (globals_m11->number_of_session_segments == SEGMENT_NUMBER_NO_ENTRY_m11) {
			ri = ri_fps->record_indices;
			for (n_segs = 0, i = n_recs; i--; ++ri)
				if (ri->type_code == REC_Sgmt_TYPE_CODE_m11)
					++n_segs;
			globals_m11->number_of_session_segments = n_segs;
		} else {
			n_segs = globals_m11->number_of_session_segments;
		}
				
		// allocate Sgmt_records array
		Sgmt_records = (Sgmt_RECORD_m11 *) calloc_m11((size_t) n_segs, sizeof(Sgmt_RECORD_m11), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
		
		// decide if more efficient to read full file, or seek to specific records
		seek_mode = FALSE_m11;
		if (rd_fps->parameters.full_file_read == FALSE_m11) {  // full file not already read in
			data_len = rd_fps->parameters.flen - UNIVERSAL_HEADER_BYTES_m11;
			seek_data_size = (si8) n_segs * sizeof(Sgmt_RECORD_m11);
			if ((data_len / seek_data_size) >= SEEK_THRESHOLD)
				seek_mode = TRUE_m11;
			else
				read_file_m11(rd_fps, NULL, 0, 0, FPS_FULL_FILE_m11, NULL, NULL, USE_GLOBAL_BEHAVIOR_m11); // read in full file
		}
		if (seek_mode == TRUE_m11) {  // ? more efficient to seek (large records files)
			ri = ri_fps->record_indices;
			for (i = 0; i < n_segs; ++ri) {
				if (ri->type_code == REC_Sgmt_TYPE_CODE_m11) {
					read_file_m11(rd_fps, NULL, ri->file_offset, sizeof(Sgmt_RECORD_m11), 1, NULL, NULL, USE_GLOBAL_BEHAVIOR_m11);
					Sgmt_records[i] = *((Sgmt_RECORD_m11 *) rd_fps->record_data);
					Sgmt_records[i++].total_record_bytes = sizeof(Sgmt_RECORD_m11);  // discard description, if any
				}
			}
		} else {  // ? more efficient to read full file (small records files)
			ri = ri_fps->record_indices;
			for (i = 0; i < n_segs; ++ri) {
				if (ri->type_code == REC_Sgmt_TYPE_CODE_m11) {
					file_offset = REMOVE_DISCONTINUITY_m11(ri->file_offset);
					Sgmt_records[i] = *((Sgmt_RECORD_m11 *) (rd_fps->parameters.raw_data + file_offset));
					Sgmt_records[i++].total_record_bytes = sizeof(Sgmt_RECORD_m11);  // discard description, if any
				}
			}
		}
	}
	
	// use metadata files (much less efficient)
	else if (chan != NULL) {  // ri_fps == NULL
		seg_list = generate_file_list_m11(NULL, &n_segs, chan->path, NULL, "?isd", GFL_FULL_PATH_m11);
		globals_m11->number_of_session_segments = n_segs;
		
		// allocate Sgmt_records array
		Sgmt_records = (Sgmt_RECORD_m11 *) calloc_m11((size_t) n_segs, sizeof(Sgmt_RECORD_m11), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);

		switch (chan->type_code) {
			case LH_TIME_SERIES_CHANNEL_m11:
				metadata_ext = TIME_SERIES_METADATA_FILE_TYPE_STRING_m11;
				break;
			case LH_VIDEO_CHANNEL_m11:
				metadata_ext = VIDEO_METADATA_FILE_TYPE_STRING_m11;
				break;
		}
		
		for (i = 0; i < n_segs; ++i) {
			extract_path_parts_m11(seg_list[i], NULL, seg_name, NULL);
			sprintf_m11(tmp_str, "%s/%s.%s", seg_list[i], seg_name, metadata_ext);
			md_fps = read_file_m11(NULL, tmp_str, 0, 0, FPS_FULL_FILE_m11, NULL, NULL, USE_GLOBAL_BEHAVIOR_m11);
			Sgmt_rec = Sgmt_records + i;
			Sgmt_rec->record_CRC = CRC_NO_ENTRY_m11;
			Sgmt_rec->total_record_bytes = RECORD_HEADER_BYTES_m11 + REC_Sgmt_v10_BYTES_m11;  // no description
			Sgmt_rec->start_time = md_fps->universal_header->segment_start_time;
			Sgmt_rec->type_code = REC_Sgmt_TYPE_CODE_m11;
			Sgmt_rec->version_major = 1;
			Sgmt_rec->version_minor = 0;
			Sgmt_rec->encryption_level = NO_ENCRYPTION_m11;
			Sgmt_rec->end_time = md_fps->universal_header->segment_end_time;
			Sgmt_rec->segment_UID = md_fps->universal_header->segment_UID;
			Sgmt_rec->segment_number = md_fps->universal_header->segment_number;
			switch (chan->type_code) {
				case LH_TIME_SERIES_CHANNEL_m11:
					Sgmt_rec->start_sample_number = md_fps->metadata->time_series_section_2.absolute_start_sample_number;
					Sgmt_rec->end_sample_number = Sgmt_rec->start_sample_number + md_fps->metadata->time_series_section_2.number_of_samples - 1;
					Sgmt_rec->acquisition_channel_number = md_fps->metadata->time_series_section_2.acquisition_channel_number;
					Sgmt_rec->sampling_frequency = md_fps->metadata->time_series_section_2.sampling_frequency;
					break;
				case LH_VIDEO_CHANNEL_m11:
					Sgmt_rec->start_frame_number = md_fps->metadata->video_section_2.absolute_start_frame_number;
					Sgmt_rec->end_frame_number = Sgmt_rec->start_frame_number + md_fps->metadata->video_section_2.number_of_frames - 1;
					Sgmt_rec->acquisition_channel_number = md_fps->metadata->video_section_2.acquisition_channel_number;
					Sgmt_rec->frame_rate = md_fps->metadata->video_section_2.frame_rate;
					break;
			}
			FPS_free_processing_struct_m11(md_fps, TRUE_m11);
		}
	}
	
	else {  // ri_fps == NULL && chan == NULL
		error_message_m11("%s(): no records or channel passed\n", __FUNCTION__);
		return(NULL);
	}
			
	// fill in global end fields
	globals_m11->session_end_time = Sgmt_records[n_segs - 1].end_time;
	globals_m11->number_of_session_samples = Sgmt_records[n_segs - 1].end_sample_number + 1;  // frame numbers are unioned
	
	return(Sgmt_records);
}


si8	bytes_for_items_m11(FILE_PROCESSING_STRUCT_m11 *fps, si8 *number_of_items, si8 read_file_offset)
{
	ui4				entry_size;
	si8				i, bytes, max_bytes;
	RECORD_HEADER_m11		*rh;
	CMP_BLOCK_FIXED_HEADER_m11	*bh;
	UNIVERSAL_HEADER_m11		*uh;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// read_file_offset only needed for items with variable size data (TIME_SERIES_DATA_FILE_TYPE_CODE_m11 & RECORD_DATA_FILE_TYPE_CODE_m11)
	// read_file_offset == 0 for writing
	
	bytes = 0;
	uh = fps->universal_header;
	switch (uh->type_code) {
		case TIME_SERIES_INDICES_FILE_TYPE_CODE_m11:
		case VIDEO_INDICES_FILE_TYPE_CODE_m11:
		case RECORD_INDICES_FILE_TYPE_CODE_m11:
			bytes = *number_of_items * INDEX_BYTES_m11;
			uh->maximum_entry_size = INDEX_BYTES_m11;
			break;
		case TIME_SERIES_METADATA_FILE_TYPE_CODE_m11:
		case VIDEO_METADATA_FILE_TYPE_CODE_m11:
			bytes = METADATA_BYTES_m11;
			uh->maximum_entry_size = METADATA_BYTES_m11;
			*number_of_items = 1;
			break;
	}
	if (bytes)
		return(bytes);
	
	read_file_offset = REMOVE_DISCONTINUITY_m11(read_file_offset);
	if (read_file_offset == 0) {  // writing (data is in memory)
		switch (uh->type_code) {
			case TIME_SERIES_DATA_FILE_TYPE_CODE_m11:
				bh = fps->parameters.cps->block_header;
				for (i = 0; i < *number_of_items; ++i) {
					entry_size = (si8) bh->total_block_bytes;
					if (uh->maximum_entry_size < entry_size)  // caller should've done this, but just in case
						uh->maximum_entry_size = entry_size;
					bytes += entry_size;
					bh = (CMP_BLOCK_FIXED_HEADER_m11 *) ((ui1 *) bh + entry_size);
				}
				break;
			case RECORD_DATA_FILE_TYPE_CODE_m11:
				rh = (RECORD_HEADER_m11 *) fps->record_data;
				for (i = 0; i < *number_of_items; ++i) {
					entry_size = (si8) rh->total_record_bytes;
					if (uh->maximum_entry_size < entry_size)  // caller should've done this, but just in case
						uh->maximum_entry_size = entry_size;
					bytes += entry_size;
					rh = (RECORD_HEADER_m11 *) ((ui1 *) rh + entry_size);
				}
				break;
		}
		return(bytes);
	}
	
	// reading (this is why it's better to pass this value)
	max_bytes = (si8) uh->maximum_entry_size * *number_of_items;
	FPS_reallocate_processing_struct_m11(fps, max_bytes);
	max_bytes = FPS_read_m11(fps, read_file_offset, max_bytes, __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	switch (uh->type_code) {
		case TIME_SERIES_DATA_FILE_TYPE_CODE_m11:
			bh = fps->parameters.cps->block_header;
			for (i = 0; i < *number_of_items; ++i) {
				entry_size = (si8) bh->total_block_bytes;
				bytes += entry_size;
				if (bytes > max_bytes) {
					*number_of_items = i;
					bytes -= entry_size;
					break;
				}
				bh = (CMP_BLOCK_FIXED_HEADER_m11 *) ((ui1 *) bh + entry_size);
			}
			break;
		case RECORD_DATA_FILE_TYPE_CODE_m11:
			rh = (RECORD_HEADER_m11 *) fps->record_data;;
			for (i = 0; i < *number_of_items; ++i) {
				entry_size = (si8) rh->total_record_bytes;
				bytes += entry_size;
				if (bytes > max_bytes) {
					*number_of_items = i;
					bytes -= entry_size;
					break;
				}
				rh = (RECORD_HEADER_m11 *) ((ui1 *) rh + entry_size);
			}
			break;
	}
	
	return(bytes);
}


void	calculate_metadata_CRC_m11(FILE_PROCESSING_STRUCT_m11 *fps)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	fps->universal_header->body_CRC = CRC_calculate_m11((ui1 *) fps->data_pointers, METADATA_BYTES_m11);
	
	return;
}


void    calculate_record_data_CRCs_m11(FILE_PROCESSING_STRUCT_m11 *fps)
{
	ui4			temp_CRC, full_record_CRC;
	si8			i;
	RECORD_HEADER_m11	*rh;
	UNIVERSAL_HEADER_m11	*uh;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	uh = fps->universal_header;
	rh = (RECORD_HEADER_m11 *) fps->data_pointers;
	for (i = fps->number_of_items; i--;) {
		rh->record_CRC = CRC_calculate_m11((ui1 *) rh + RECORD_HEADER_RECORD_CRC_START_OFFSET_m11, rh->total_record_bytes - RECORD_HEADER_RECORD_CRC_START_OFFSET_m11);
		
		temp_CRC = CRC_calculate_m11((ui1 *) rh, RECORD_HEADER_RECORD_CRC_START_OFFSET_m11);
		full_record_CRC = CRC_combine_m11(temp_CRC, rh->record_CRC, rh->total_record_bytes - RECORD_HEADER_RECORD_CRC_START_OFFSET_m11);
		uh->body_CRC = CRC_combine_m11(uh->body_CRC, full_record_CRC, rh->total_record_bytes);
		
		rh = (RECORD_HEADER_m11 *) ((ui1 *) rh + rh->total_record_bytes);
	}
	
	return;
}


void    calculate_time_series_data_CRCs_m11(FILE_PROCESSING_STRUCT_m11 *fps)
{
	ui4     			temp_CRC, full_block_CRC;
	si8     			i;
	UNIVERSAL_HEADER_m11		*uh;
	CMP_BLOCK_FIXED_HEADER_m11	*bh;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	bh = (CMP_BLOCK_FIXED_HEADER_m11 *) fps->data_pointers;
		
	uh = fps->universal_header;
	for (i = fps->number_of_items; i--;) {
		bh->block_CRC = CRC_calculate_m11((ui1 *) bh + CMP_BLOCK_CRC_START_OFFSET_m11, bh->total_block_bytes - CMP_BLOCK_CRC_START_OFFSET_m11);
		
		temp_CRC = CRC_calculate_m11((ui1 *) bh, CMP_BLOCK_CRC_START_OFFSET_m11);
		full_block_CRC = CRC_combine_m11(temp_CRC, bh->block_CRC, bh->total_block_bytes - CMP_BLOCK_CRC_START_OFFSET_m11);
		uh->body_CRC = CRC_combine_m11(uh->body_CRC, full_block_CRC, bh->total_block_bytes);
		
		bh = (CMP_BLOCK_FIXED_HEADER_m11 *) ((ui1 *) bh + bh->total_block_bytes);
	}
	
	return;
}


void    calculate_indices_CRCs_m11(FILE_PROCESSING_STRUCT_m11 *fps)
{
	si8     		i;
	INDEX_m11		*idx;
	UNIVERSAL_HEADER_m11	*uh;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	idx = (INDEX_m11 *) fps->data_pointers;
	uh = fps->universal_header;
	for (i = fps->number_of_items; i--; ++idx)
		uh->body_CRC = CRC_update_m11((ui1 *) idx, INDEX_BYTES_m11, uh->body_CRC);
	
	return;
}


void	change_reference_channel_m11(SESSION_m11 *sess, CHANNEL_m11 *channel, si1 *channel_name, si1 channel_type)
{
	TERN_m11			use_default_channel;
	si8				i, n_chans;
	FILE_PROCESSING_STRUCT_m11	*ri_fps, *rd_fps;
	CHANNEL_m11			*chan;

	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// pass either channel, or channel name (if both passed channel will be used)
	// if both NULL, use channel_type
	
	use_default_channel = FALSE_m11;
	if (channel == NULL) {
		if (channel_name == NULL)
			use_default_channel = TRUE_m11;
		else if (*channel_name == 0)
			use_default_channel = TRUE_m11;
	}
	if (use_default_channel == FALSE_m11)  // if channel or channel_name passed, channel type options ignored
		channel_type = DEFAULT_CHANNEL_m11;

	switch (channel_type) {
		case DEFAULT_CHANNEL_m11:
		case DEFAULT_TIME_SERIES_CHANNEL_m11:
		case DEFAULT_VIDEO_CHANNEL_m11:
			break;
		case HIGHEST_RATE_TIME_SERIES_CHANNEL_m11:
			channel = globals_m11->maximum_time_series_frequency_channel;
			channel_type = DEFAULT_TIME_SERIES_CHANNEL_m11;
			break;
		case LOWEST_RATE_TIME_SERIES_CHANNEL_m11:
			channel = globals_m11->minimum_time_series_frequency_channel;
			channel_type = DEFAULT_TIME_SERIES_CHANNEL_m11;
			break;
		case HIGHEST_RATE_VIDEO_CHANNEL_m11:
			channel = globals_m11->maximum_video_frame_rate_channel;
			channel_type = DEFAULT_VIDEO_CHANNEL_m11;
			break;
		case LOWEST_RATE_VIDEO_CHANNEL_m11:
			channel = globals_m11->minimum_video_frame_rate_channel;
			channel_type = DEFAULT_VIDEO_CHANNEL_m11;
			break;
		default:
			channel_type = DEFAULT_CHANNEL_m11;
			break;
	}
	
	// find channel from name
	globals_m11->reference_channel = NULL;
	if (channel == NULL) {
		if (use_default_channel == TRUE_m11) {
			if ((channel = get_active_channel_m11(sess, channel_type)) == NULL) {
				error_message_m11("%s(): no matching active channels\n", __FUNCTION__);
				exit_m11(-1);
			}
			goto CHANGE_REF_MATCH_m11;
		}
		if (channel_type == DEFAULT_CHANNEL_m11 || channel_type == DEFAULT_TIME_SERIES_CHANNEL_m11) {
			n_chans = sess->number_of_time_series_channels;  // check for match in time_series_channels
			for (i = 0; i < n_chans; ++i) {
				chan = sess->time_series_channels[i];
				if (chan->flags & LH_CHANNEL_ACTIVE_m11) {
					if (strcmp(chan->name, channel_name) == 0) {
						channel = chan;
						goto CHANGE_REF_MATCH_m11;
					}
				}
			}
		}
		if (channel_type == DEFAULT_CHANNEL_m11 || channel_type == DEFAULT_VIDEO_CHANNEL_m11) {
			n_chans = sess->number_of_video_channels;
			for (i = 0; i < n_chans; ++i) {
				chan = sess->video_channels[i];
				if (chan->flags & LH_CHANNEL_ACTIVE_m11) {
					if (strcmp(chan->name, channel_name) == 0) {
						channel = chan;
						goto CHANGE_REF_MATCH_m11;
					}
				}
			}
		}
	} else {  // channel known
		if (channel->flags & LH_CHANNEL_ACTIVE_m11)
			goto CHANGE_REF_MATCH_m11;
		if ((channel = get_active_channel_m11(sess, channel_type)) != NULL)
			goto CHANGE_REF_MATCH_m11;
	}

	error_message_m11("%s(): no matching active channels\n", __FUNCTION__);
	exit_m11(-1);
	
CHANGE_REF_MATCH_m11:
	globals_m11->reference_channel = channel;
	strcpy(globals_m11->reference_channel_name, channel->name);

	if (sess->Sgmt_records != NULL)
		free_m11((void *) sess->Sgmt_records, __FUNCTION__);
	ri_fps = channel->record_indices_fps;
	rd_fps = channel->record_data_fps;
	if (sess->Sgmt_records != NULL)
		free_m11(sess->Sgmt_records, __FUNCTION__);
	sess->Sgmt_records = build_Sgmt_records_array_m11(ri_fps, rd_fps, channel);
			    
	return;
		

}


ui4	channel_type_from_path_m11(si1 *path)
{
	si1	*c, temp_path[FULL_FILE_NAME_BYTES_m11], name[SEGMENT_BASE_FILE_NAME_BYTES_m11], extension[TYPE_BYTES_m11];
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// move pointer to end of string
	c = path + strlen(path) - 1;
	
	// ignore terminal "/" if present
	if (*c == '/')
		c--;
	
	// copy extension
	c -= 4;
	if (*c != '.')
		return(UNKNOWN_CHANNEL_TYPE_m11);
	strncpy_m11(extension, ++c, TYPE_BYTES_m11);
	
	// compare extension: record types => get extension of next level up the hierarchy
	if (!(strcmp(extension, RECORD_DATA_FILE_TYPE_STRING_m11)) || !(strcmp(extension, RECORD_INDICES_FILE_TYPE_STRING_m11))) {
		extract_path_parts_m11(path, temp_path, NULL, NULL);
		extract_path_parts_m11(temp_path, temp_path, name, extension);
	}
	
	// compare extension: TIMES_SERIES_CHANNEL_TYPE
	if (!(strcmp(extension, TIME_SERIES_CHANNEL_DIRECTORY_TYPE_STRING_m11)))
		return(TIME_SERIES_CHANNEL_TYPE_m11);
	else if (!(strcmp(extension, TIME_SERIES_SEGMENT_DIRECTORY_TYPE_STRING_m11)))
		return(TIME_SERIES_CHANNEL_TYPE_m11);
	else if (!(strcmp(extension, TIME_SERIES_METADATA_FILE_TYPE_STRING_m11)))
		return(TIME_SERIES_CHANNEL_TYPE_m11);
	else if (!(strcmp(extension, TIME_SERIES_DATA_FILE_TYPE_STRING_m11)))
		return(TIME_SERIES_CHANNEL_TYPE_m11);
	else if (!(strcmp(extension, TIME_SERIES_INDICES_FILE_TYPE_STRING_m11)))
		return(TIME_SERIES_CHANNEL_TYPE_m11);
	
	// compare extension: VIDEO_CHANNEL_TYPE
	else if (!(strcmp(extension, VIDEO_CHANNEL_DIRECTORY_TYPE_STRING_m11)))
		return(VIDEO_CHANNEL_TYPE_m11);
	else if (!(strcmp(extension, VIDEO_SEGMENT_DIRECTORY_TYPE_STRING_m11)))
		return(VIDEO_CHANNEL_TYPE_m11);
	else if (!(strcmp(extension, VIDEO_METADATA_FILE_TYPE_STRING_m11)))
		return(VIDEO_CHANNEL_TYPE_m11);
	else if (!(strcmp(extension, VIDEO_INDICES_FILE_TYPE_STRING_m11)))
		return(VIDEO_CHANNEL_TYPE_m11);
	
	// unknown channel type
	return(UNKNOWN_CHANNEL_TYPE_m11);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
wchar_t	*char2wchar_m11(wchar_t *target, si1 *source)
{
	si1	*c, *c2, *tmp_source = NULL;
	si8	len, wsz;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// if source == target, done in place
	// if not actually ascii, results may be weird
	// assumes target is big enough
	
	wsz = sizeof(wchar_t);  // 2 or 4 => varies by OS & compiler
	c = (si1 *) target - wsz;
	len = strlen(source);
	if ((void *) source == (void *) target) {
		tmp_source = (si1 *) malloc((size_t) len + 1);
		memcpy((void *) tmp_source, (void *)  source, (size_t)len + 1);
		c2 = tmp_source;
	}
	else {
		c2 = source;
	}
	memset((void *) target, 0, wsz * (len + 1));
	
	while (len--)
		*(c += wsz) = *c2++;  // little endian version
	
	if (tmp_source != NULL)
		free((void *) tmp_source);
	
	return(target);
}


TERN_m11        check_all_alignments_m11(void)
{
	TERN_m11        return_value;
	ui1		*bytes;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// see if already checked
	if (globals_m11->all_structures_aligned != UNKNOWN_m11)
		return(globals_m11->all_structures_aligned);
	
	return_value = TRUE_m11;
	bytes = (ui1 *) malloc(METADATA_FILE_BYTES_m11);  // METADATA is largest file structure
	
	// check all structures
	if ((check_universal_header_alignment_m11(bytes)) == FALSE_m11)
		return_value = FALSE_m11;
	if ((check_metadata_alignment_m11(bytes)) == FALSE_m11)
		return_value = FALSE_m11;
	if ((check_CMP_block_header_alignment_m11(bytes)) == FALSE_m11)
		return_value = FALSE_m11;
	if ((check_CMP_record_header_alignment_m11(bytes)) == FALSE_m11)
		return_value = FALSE_m11;
	if ((check_time_series_indices_alignment_m11(bytes)) == FALSE_m11)
		return_value = FALSE_m11;
	if ((check_video_indices_alignment_m11(bytes)) == FALSE_m11)
		return_value = FALSE_m11;
	if ((check_record_indices_alignment_m11(bytes)) == FALSE_m11)
		return_value = FALSE_m11;
	if ((check_record_header_alignment_m11(bytes)) == FALSE_m11)
		return_value = FALSE_m11;
	if ((check_record_structure_alignments_m11(bytes)) == FALSE_m11)
		return_value = FALSE_m11;

	free((void *) bytes);
	
	if (return_value == TRUE_m11) {
		globals_m11->all_structures_aligned = TRUE_m11;
		if (globals_m11->verbose == TRUE_m11)
			message_m11("All MED Library structures are aligned\n");
	} else {
		error_message_m11("%s(): unaligned MED structures\n", __FUNCTION__);
	}
	
	return(return_value);
}


TERN_m11	check_char_type_m11(void)
{
	char	c;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// check size of "char"
	if (sizeof(char) != 1)
		return(FALSE_m11);

	// check signedness of "char"
	c = -1;
	if ((si4) c != (si4) -1)
		return(FALSE_m11);
	
	return(TRUE_m11);
}


TERN_m11        check_CMP_block_header_alignment_m11(ui1 *bytes)
{
	CMP_BLOCK_FIXED_HEADER_m11	*cbh;
	TERN_m11			free_flag = FALSE_m11;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// see if already checked
	if (globals_m11->CMP_block_header_aligned == UNKNOWN_m11)
		globals_m11->CMP_block_header_aligned = FALSE_m11;
	else
		return(globals_m11->CMP_block_header_aligned);
	
	// check overall size
	if (sizeof(CMP_BLOCK_FIXED_HEADER_m11) != CMP_BLOCK_FIXED_HEADER_BYTES_m11)
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m11;
	
	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(CMP_BLOCK_FIXED_HEADER_BYTES_m11);
		free_flag = TRUE_m11;
	}
	cbh = (CMP_BLOCK_FIXED_HEADER_m11 *)bytes;
	if (&cbh->block_start_UID != (ui8 *) (bytes + CMP_BLOCK_START_UID_OFFSET_m11))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m11;
	if (&cbh->block_CRC != (ui4 *) (bytes + CMP_BLOCK_CRC_OFFSET_m11))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m11;
	if (&cbh->block_flags != (ui4 *) (bytes + CMP_BLOCK_BLOCK_FLAGS_OFFSET_m11))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m11;
	if (&cbh->start_time != (si8 *) (bytes + CMP_BLOCK_START_TIME_OFFSET_m11))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m11;
	if (&cbh->acquisition_channel_number != (si4 *) (bytes + CMP_BLOCK_ACQUISITION_CHANNEL_NUMBER_OFFSET_m11))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m11;
	if (&cbh->total_block_bytes != (ui4 *) (bytes + CMP_BLOCK_TOTAL_BLOCK_BYTES_OFFSET_m11))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m11;
	if (&cbh->number_of_samples != (ui4 *) (bytes + CMP_BLOCK_NUMBER_OF_SAMPLES_OFFSET_m11))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m11;
	if (&cbh->number_of_records != (ui2 *) (bytes + CMP_BLOCK_NUMBER_OF_RECORDS_OFFSET_m11))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m11;
	if (&cbh->record_region_bytes != (ui2 *) (bytes + CMP_BLOCK_RECORD_REGION_BYTES_OFFSET_m11))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m11;
	if (&cbh->parameter_flags != (ui4 *) (bytes + CMP_BLOCK_PARAMETER_FLAGS_OFFSET_m11))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m11;
	if (&cbh->parameter_region_bytes != (ui2 *) (bytes + CMP_BLOCK_PARAMETER_REGION_BYTES_OFFSET_m11))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m11;
	if (&cbh->protected_region_bytes != (ui2 *) (bytes + CMP_BLOCK_PROTECTED_REGION_BYTES_OFFSET_m11))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m11;
	if (&cbh->discretionary_region_bytes != (ui2 *) (bytes + CMP_BLOCK_DISCRETIONARY_REGION_BYTES_OFFSET_m11))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m11;
	if (&cbh->model_region_bytes != (ui2 *) (bytes + CMP_BLOCK_MODEL_REGION_BYTES_OFFSET_m11))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m11;
	if (&cbh->total_header_bytes != (ui4 *) (bytes + CMP_BLOCK_TOTAL_HEADER_BYTES_OFFSET_m11))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m11;
	
	// aligned
	globals_m11->CMP_block_header_aligned = TRUE_m11;
	
	if (free_flag == TRUE_m11)
		free((void *) bytes);
	
	if (globals_m11->verbose == TRUE_m11)
		message_m11("CMP_BLOCK_FIXED_HEADER_m11 structure is aligned\n", __FUNCTION__);
	
	return(TRUE_m11);
	
	// not aligned
CMP_BLOCK_HEADER_NOT_ALIGNED_m11:
	
	if (free_flag == TRUE_m11)
		free((void *) bytes);
	
	if (globals_m11->verbose == TRUE_m11)
		error_message_m11("%s(): CMP_BLOCK_FIXED_HEADER_m11 structure is NOT aligned\n", __FUNCTION__);
	
	return(FALSE_m11);
}


TERN_m11        check_CMP_record_header_alignment_m11(ui1 *bytes)
{
	CMP_RECORD_HEADER_m11	*crh;
	TERN_m11		free_flag = FALSE_m11;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// see if already checked
	if (globals_m11->CMP_record_header_aligned == UNKNOWN_m11)
		globals_m11->CMP_record_header_aligned = FALSE_m11;
	else
		return(globals_m11->CMP_record_header_aligned);
	
	// check overall size
	if (sizeof(CMP_RECORD_HEADER_m11) != CMP_RECORD_HEADER_BYTES_m11)
		goto CMP_RECORD_HEADER_NOT_ALIGNED_m11;
	
	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(CMP_RECORD_HEADER_BYTES_m11);
		free_flag = TRUE_m11;
	}
	crh = (CMP_RECORD_HEADER_m11 *)bytes;
	if (&crh->type_code != (ui4 *) (bytes + CMP_RECORD_HEADER_TYPE_CODE_OFFSET_m11))
		goto CMP_RECORD_HEADER_NOT_ALIGNED_m11;
	if (&crh->version_major != (ui1 *) (bytes + CMP_RECORD_HEADER_VERSION_MAJOR_OFFSET_m11))
		goto CMP_RECORD_HEADER_NOT_ALIGNED_m11;
	if (&crh->version_minor != (ui1 *) (bytes + CMP_RECORD_HEADER_VERSION_MINOR_OFFSET_m11))
		goto CMP_RECORD_HEADER_NOT_ALIGNED_m11;
	if (&crh->total_bytes != (ui2 *) (bytes + CMP_RECORD_HEADER_TOTAL_BYTES_OFFSET_m11))
		goto CMP_RECORD_HEADER_NOT_ALIGNED_m11;
	
	// aligned
	globals_m11->CMP_record_header_aligned = TRUE_m11;
	
	if (free_flag == TRUE_m11)
		free((void *) bytes);
	
	if (globals_m11->verbose == TRUE_m11)
		message_m11("CMP_RECORD_HEADER_m11 structure is aligned\n");
	
	return(TRUE_m11);
	
	// not aligned
CMP_RECORD_HEADER_NOT_ALIGNED_m11:
	
	if (free_flag == TRUE_m11)
		free((void *) bytes);
	
	if (globals_m11->verbose == TRUE_m11)
		error_message_m11("%s(): CMP_RECORD_HEADER_m11 structure is NOT aligned\n", __FUNCTION__);
	
	return(FALSE_m11);
}


TERN_m11	check_file_list_m11(si1 **file_list, si4 n_files)
{
	si4	i;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// generate_file_list_m11() does a lot of stuff, but often just need to ensure list contains full paths with no regex
	
	if (file_list == NULL)
		return(FALSE_m11);
	if (file_list[0] == NULL)
		return(FALSE_m11);
	
	for (i = 0; i < n_files; ++i) {
		if (STR_contains_regex_m11(file_list[i]) == TRUE_m11)
			return(FALSE_m11);
		if (path_from_root_m11(file_list[i], NULL) == FALSE_m11)
			return(FALSE_m11);
	}

	return(TRUE_m11);
}


TERN_m11        check_metadata_alignment_m11(ui1 *bytes)
{
	TERN_m11	return_value, free_flag = FALSE_m11;
	METADATA_m11	*md;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// see if already checked
	if (globals_m11->all_metadata_structures_aligned != UNKNOWN_m11)
		return(globals_m11->all_metadata_structures_aligned);
	
	return_value = TRUE_m11;
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(METADATA_FILE_BYTES_m11);
		free_flag = TRUE_m11;
	}
	
	// check overall size
	if (sizeof(METADATA_m11) != METADATA_BYTES_m11)
		return_value = FALSE_m11;

	// check substructure offsets
	md = (METADATA_m11 *) bytes;
	if (&md->section_1 != (METADATA_SECTION_1_m11 *) bytes)
		return_value = FALSE_m11;
	if (&md->time_series_section_2 != (TIME_SERIES_METADATA_SECTION_2_m11 *) (bytes + METADATA_SECTION_1_BYTES_m11))
		return_value = FALSE_m11;
	if (&md->video_section_2 != (VIDEO_METADATA_SECTION_2_m11 *) (bytes + METADATA_SECTION_1_BYTES_m11))
		return_value = FALSE_m11;
	if (&md->section_3 != (METADATA_SECTION_3_m11 *) (bytes + METADATA_SECTION_1_BYTES_m11 + METADATA_SECTION_2_BYTES_m11))
		return_value = FALSE_m11;

	// check substructure contents
	if (check_metadata_section_1_alignment_m11(bytes) == FALSE_m11)
		return_value = FALSE_m11;
	if (check_time_series_metadata_section_2_alignment_m11(bytes) == FALSE_m11)
		return_value = FALSE_m11;
	if (check_video_metadata_section_2_alignment_m11(bytes) == FALSE_m11)
		return_value = FALSE_m11;
	if (check_metadata_section_3_alignment_m11(bytes) == FALSE_m11)
			return_value = FALSE_m11;
	
	if (free_flag == TRUE_m11)
		free((void *) bytes);
	
	if (return_value == TRUE_m11)
		globals_m11->all_metadata_structures_aligned = TRUE_m11;

	return(return_value);
}


TERN_m11	check_metadata_section_1_alignment_m11(ui1 *bytes)
{
	METADATA_SECTION_1_m11	*md1;
	TERN_m11		free_flag = FALSE_m11;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// see if already checked
	if (globals_m11->metadata_section_1_aligned == UNKNOWN_m11)
		globals_m11->metadata_section_1_aligned = FALSE_m11;
	else
		return(globals_m11->metadata_section_1_aligned);
	
	// check overall size
	if (sizeof(METADATA_SECTION_1_m11) != METADATA_SECTION_1_BYTES_m11)
		goto METADATA_SECTION_1_NOT_ALIGNED_m11;
	
	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(METADATA_FILE_BYTES_m11);
		free_flag = TRUE_m11;
	}
	md1 = (METADATA_SECTION_1_m11 *) (bytes + UNIVERSAL_HEADER_BYTES_m11);
	if (md1->level_1_password_hint != (si1 *) (bytes + METADATA_LEVEL_1_PASSWORD_HINT_OFFSET_m11))
		goto METADATA_SECTION_1_NOT_ALIGNED_m11;
	if (md1->level_2_password_hint != (si1 *) (bytes + METADATA_LEVEL_2_PASSWORD_HINT_OFFSET_m11))
		goto METADATA_SECTION_1_NOT_ALIGNED_m11;
	if (&md1->section_2_encryption_level != (si1 *) (bytes + METADATA_SECTION_2_ENCRYPTION_LEVEL_OFFSET_m11))
		goto METADATA_SECTION_1_NOT_ALIGNED_m11;
	if (&md1->section_3_encryption_level != (si1 *) (bytes + METADATA_SECTION_3_ENCRYPTION_LEVEL_OFFSET_m11))
		goto METADATA_SECTION_1_NOT_ALIGNED_m11;
	if (&md1->time_series_data_encryption_level != (si1 *) (bytes + METADATA_TIME_SERIES_DATA_ENCRYPTION_LEVEL_OFFSET_m11))
		goto METADATA_SECTION_1_NOT_ALIGNED_m11;
	if (md1->protected_region != (ui1 *) (bytes + METADATA_SECTION_1_PROTECTED_REGION_OFFSET_m11))
		goto METADATA_SECTION_1_NOT_ALIGNED_m11;
	if (md1->discretionary_region != (ui1 *) (bytes + METADATA_SECTION_1_DISCRETIONARY_REGION_OFFSET_m11))
		goto METADATA_SECTION_1_NOT_ALIGNED_m11;
	
	// aligned
	globals_m11->metadata_section_1_aligned = TRUE_m11;
	
	if (free_flag == TRUE_m11)
		free((void *) bytes);
	
	if (globals_m11->verbose == TRUE_m11)
		message_m11("METADATA_SECTION_1_m11 structure is aligned\n");
	
	return(TRUE_m11);
	
	// not aligned
METADATA_SECTION_1_NOT_ALIGNED_m11:
	
	if (free_flag == TRUE_m11)
		free((void *) bytes);
	
	if (globals_m11->verbose == TRUE_m11)
		error_message_m11("%s(): METADATA_SECTION_1_m11 structure is NOT aligned\n", __FUNCTION__);
	
	return(FALSE_m11);
}


TERN_m11	check_metadata_section_3_alignment_m11(ui1 *bytes)
{
	METADATA_SECTION_3_m11	*md3;
	TERN_m11		free_flag = FALSE_m11;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// see if already checked
	if (globals_m11->metadata_section_3_aligned == UNKNOWN_m11)
		globals_m11->metadata_section_3_aligned = FALSE_m11;
	else
		return(globals_m11->metadata_section_3_aligned);
	
	// check overall size
	if (sizeof(METADATA_SECTION_3_m11) != METADATA_SECTION_3_BYTES_m11)
		goto METADATA_SECTION_3_NOT_ALIGNED_m11;
	
	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(METADATA_FILE_BYTES_m11);
		free_flag = TRUE_m11;
	}
	md3 = (METADATA_SECTION_3_m11 *) (bytes + METADATA_SECTION_3_OFFSET_m11);
	if (&md3->recording_time_offset != (si8 *) (bytes + METADATA_RECORDING_TIME_OFFSET_OFFSET_m11))
		goto METADATA_SECTION_3_NOT_ALIGNED_m11;
	if (&md3->daylight_time_start_code != (DAYLIGHT_TIME_CHANGE_CODE_m11 *) (bytes + METADATA_DAYLIGHT_TIME_START_CODE_OFFSET_m11))
		goto METADATA_SECTION_3_NOT_ALIGNED_m11;
	if (&md3->daylight_time_end_code != (DAYLIGHT_TIME_CHANGE_CODE_m11 *) (bytes + METADATA_DAYLIGHT_TIME_END_CODE_OFFSET_m11))
		goto METADATA_SECTION_3_NOT_ALIGNED_m11;
	if (md3->standard_timezone_acronym != (si1 *) (bytes + METADATA_STANDARD_TIMEZONE_ACRONYM_OFFSET_m11))
		goto METADATA_SECTION_3_NOT_ALIGNED_m11;
	if (md3->standard_timezone_string != (si1 *) (bytes + METADATA_STANDARD_TIMEZONE_STRING_OFFSET_m11))
		goto METADATA_SECTION_3_NOT_ALIGNED_m11;
	if (md3->daylight_timezone_acronym != (si1 *) (bytes + METADATA_DAYLIGHT_TIMEZONE_ACRONYM_OFFSET_m11))
		goto METADATA_SECTION_3_NOT_ALIGNED_m11;
	if (md3->daylight_timezone_string != (si1 *) (bytes + METADATA_DAYLIGHT_TIMEZONE_STRING_OFFSET_m11))
		goto METADATA_SECTION_3_NOT_ALIGNED_m11;
	if (md3->subject_name_1 != (si1 *) (bytes + METADATA_SUBJECT_NAME_1_OFFSET_m11))
		goto METADATA_SECTION_3_NOT_ALIGNED_m11;
	if (md3->subject_name_2 != (si1 *) (bytes + METADATA_SUBJECT_NAME_2_OFFSET_m11))
		goto METADATA_SECTION_3_NOT_ALIGNED_m11;
	if (md3->subject_name_3 != (si1 *) (bytes + METADATA_SUBJECT_NAME_3_OFFSET_m11))
		goto METADATA_SECTION_3_NOT_ALIGNED_m11;
	if (md3->subject_ID != (si1 *) (bytes + METADATA_SUBJECT_ID_OFFSET_m11))
		goto METADATA_SECTION_3_NOT_ALIGNED_m11;
	if (md3->recording_country != (si1 *) (bytes + METADATA_RECORDING_COUNTRY_OFFSET_m11))
		goto METADATA_SECTION_3_NOT_ALIGNED_m11;
	if (md3->recording_territory != (si1 *) (bytes + METADATA_RECORDING_TERRITORY_OFFSET_m11))
		goto METADATA_SECTION_3_NOT_ALIGNED_m11;
	if (md3->recording_locality != (si1 *) (bytes + METADATA_RECORDING_LOCALITY_OFFSET_m11))
		goto METADATA_SECTION_3_NOT_ALIGNED_m11;
	if (md3->recording_institution != (si1 *) (bytes + METADATA_RECORDING_INSTITUTION_OFFSET_m11))
		goto METADATA_SECTION_3_NOT_ALIGNED_m11;
	if (md3->geotag_format != (si1 *) (bytes + METADATA_GEOTAG_FORMAT_OFFSET_m11))
		goto METADATA_SECTION_3_NOT_ALIGNED_m11;
	if (md3->geotag_data != (si1 *) (bytes + METADATA_GEOTAG_DATA_OFFSET_m11))
		goto METADATA_SECTION_3_NOT_ALIGNED_m11;
	if (&md3->standard_UTC_offset != (si4 *) (bytes + METADATA_STANDARD_UTC_OFFSET_OFFSET_m11))
		goto METADATA_SECTION_3_NOT_ALIGNED_m11;
	if (md3->protected_region != (ui1 *) (bytes + METADATA_SECTION_3_PROTECTED_REGION_OFFSET_m11))
		goto METADATA_SECTION_3_NOT_ALIGNED_m11;
	if (md3->discretionary_region != (ui1 *) (bytes + METADATA_SECTION_3_DISCRETIONARY_REGION_OFFSET_m11))
		goto METADATA_SECTION_3_NOT_ALIGNED_m11;
	
	// aligned
	globals_m11->metadata_section_3_aligned = TRUE_m11;
	
	if (free_flag == TRUE_m11)
		free((void *) bytes);
	
	if (globals_m11->verbose == TRUE_m11)
		message_m11("METADATA_SECTION_3_m11 structure is aligned\n");
	
	return(TRUE_m11);
	
	// not aligned
METADATA_SECTION_3_NOT_ALIGNED_m11:
	
	if (free_flag == TRUE_m11)
		free((void *) bytes);
	
	if (globals_m11->verbose == TRUE_m11)
		error_message_m11("%s(): METADATA_SECTION_3_m11 structure is NOT aligned\n", __FUNCTION__);
	
	return(FALSE_m11);
}


TERN_m11	check_password_m11(si1 *password)
{
	si4	pw_len;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// check pointer: return FALSE_m11 for NULL
	if (password == NULL) {
		warning_message_m11("%s(): password is NULL\n", __FUNCTION__);
		return(FALSE_m11);
	}
		
	// check password length:  return +1 for length error
	pw_len = UTF8_strlen_m11(password);
	if (pw_len == 0) {
		warning_message_m11("%s(): password has no characters\n", __FUNCTION__);
		return(FALSE_m11);
	}
	if (pw_len > MAX_PASSWORD_CHARACTERS_m11) {
		warning_message_m11("%s(): password too long (1 to  %d characters)\n", __FUNCTION__, MAX_PASSWORD_CHARACTERS_m11);
		return(FALSE_m11);
	}
	
	if (globals_m11->verbose == TRUE_m11)
		message_m11("Password is of valid form\n", __FUNCTION__);
	
	// return TRUE_m11 for valid password
	return(TRUE_m11);
}


TERN_m11	check_record_header_alignment_m11(ui1 *bytes)
{
	RECORD_HEADER_m11	*rh;
	TERN_m11                free_flag = FALSE_m11;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// see if already checked
	if (globals_m11->record_header_aligned == UNKNOWN_m11)
		globals_m11->record_header_aligned = FALSE_m11;
	else
		return(globals_m11->record_header_aligned);
	
	// check overall size
	if (sizeof(RECORD_HEADER_m11) != RECORD_HEADER_BYTES_m11)
		goto RECORD_HEADER_NOT_ALIGNED_m11;
	
	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(RECORD_HEADER_BYTES_m11);
		free_flag = TRUE_m11;
	}
	rh = (RECORD_HEADER_m11 *) bytes;
	if (&rh->record_CRC != (ui4 *) (bytes + RECORD_HEADER_RECORD_CRC_OFFSET_m11))
		goto RECORD_HEADER_NOT_ALIGNED_m11;
	if (&rh->total_record_bytes != (ui4 *) (bytes + RECORD_HEADER_TOTAL_RECORD_BYTES_OFFSET_m11))
		goto RECORD_HEADER_NOT_ALIGNED_m11;
	if (&rh->start_time != (si8 *) (bytes + RECORD_HEADER_START_TIME_OFFSET_m11))
		goto RECORD_HEADER_NOT_ALIGNED_m11;
	if (rh->type_string != (si1 *) (bytes + RECORD_HEADER_TYPE_STRING_OFFSET_m11))
		goto RECORD_HEADER_NOT_ALIGNED_m11;
	if (&rh->type_code != (ui4 *) (bytes + RECORD_HEADER_TYPE_CODE_OFFSET_m11))
		goto RECORD_HEADER_NOT_ALIGNED_m11;
	if (&rh->type_string_terminal_zero != (si1 *) (bytes + RECORD_HEADER_TYPE_STRING_TERMINAL_ZERO_OFFSET_m11))
		goto RECORD_HEADER_NOT_ALIGNED_m11;
	if (&rh->version_major != (ui1 *) (bytes + RECORD_HEADER_VERSION_MAJOR_OFFSET_m11))
		goto RECORD_HEADER_NOT_ALIGNED_m11;
	if (&rh->version_minor != (ui1 *) (bytes + RECORD_HEADER_VERSION_MINOR_OFFSET_m11))
		goto RECORD_HEADER_NOT_ALIGNED_m11;
	if (&rh->encryption_level != (si1 *) (bytes + RECORD_HEADER_ENCRYPTION_LEVEL_OFFSET_m11))
		goto RECORD_HEADER_NOT_ALIGNED_m11;
	
	// aligned
	globals_m11->record_header_aligned = TRUE_m11;
	
	if (free_flag == TRUE_m11)
		free((void *) bytes);
	
	if (globals_m11->verbose == TRUE_m11)
		message_m11("RECORD_HEADER_m11 structure is aligned\n");
	
	return(TRUE_m11);
	
	// not aligned
RECORD_HEADER_NOT_ALIGNED_m11:
	
	if (free_flag == TRUE_m11)
		free((void *) bytes);
	
	if (globals_m11->verbose == TRUE_m11)
		error_message_m11("%s(): RECORD_HEADER_m11 structure is NOT aligned\n", __FUNCTION__);
	
	return(FALSE_m11);
}


TERN_m11	check_record_indices_alignment_m11(ui1 *bytes)
{
	RECORD_INDEX_m11	*ri;
	TERN_m11		free_flag = FALSE_m11;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// see if already checked
	if (globals_m11->record_indices_aligned == UNKNOWN_m11)
		globals_m11->record_indices_aligned = FALSE_m11;
	else
		return(globals_m11->record_indices_aligned);
	
	// check overall size
	if (sizeof(RECORD_INDEX_m11) != RECORD_INDEX_BYTES_m11)
		goto RECORD_INDICES_NOT_ALIGNED_m11;
	
	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(RECORD_INDEX_BYTES_m11);
		free_flag = TRUE_m11;
	}
	ri = (RECORD_INDEX_m11 *) bytes;
	if (&ri->file_offset != (si8 *) (bytes + RECORD_INDEX_FILE_OFFSET_OFFSET_m11))
		goto RECORD_INDICES_NOT_ALIGNED_m11;
	if (&ri->start_time != (si8 *) (bytes + RECORD_INDEX_START_TIME_OFFSET_m11))
		goto RECORD_INDICES_NOT_ALIGNED_m11;
	if (ri->type_string != (si1 *) (bytes + RECORD_INDEX_TYPE_STRING_OFFSET_m11))
		goto RECORD_INDICES_NOT_ALIGNED_m11;
	if (&ri->type_code != (ui4 *) (bytes + RECORD_INDEX_TYPE_CODE_OFFSET_m11))
		goto RECORD_INDICES_NOT_ALIGNED_m11;
	if (&ri->type_string_terminal_zero != (si1 *) (bytes + RECORD_INDEX_TYPE_STRING_TERMINAL_ZERO_OFFSET_m11))
		goto RECORD_INDICES_NOT_ALIGNED_m11;
	if (&ri->version_major != (ui1 *) (bytes + RECORD_INDEX_VERSION_MAJOR_OFFSET_m11))
		goto RECORD_INDICES_NOT_ALIGNED_m11;
	if (&ri->version_minor != (ui1 *) (bytes + RECORD_INDEX_VERSION_MINOR_OFFSET_m11))
		goto RECORD_INDICES_NOT_ALIGNED_m11;
	if (&ri->encryption_level != (si1 *) (bytes + RECORD_INDEX_ENCRYPTION_LEVEL_OFFSET_m11))
		goto RECORD_INDICES_NOT_ALIGNED_m11;
	
	// aligned
	globals_m11->record_indices_aligned = TRUE_m11;
	
	if (free_flag == TRUE_m11)
		free((void *) bytes);
	
	if (globals_m11->verbose == TRUE_m11)
		printf_m11("RECORD_INDEX_m11 structure is aligned\n");
	
	return(TRUE_m11);
	
	// not aligned
RECORD_INDICES_NOT_ALIGNED_m11:
	
	if (free_flag == TRUE_m11)
		free((void *) bytes);
	
	if (globals_m11->verbose == TRUE_m11)
		error_message_m11("%s(): RECORD_INDEX_m11 structure is NOT aligned\n", __FUNCTION__);
	
	return(FALSE_m11);
}


TERN_m11	check_time_series_indices_alignment_m11(ui1 *bytes)
{
	TIME_SERIES_INDEX_m11	*tsi;
	TERN_m11		free_flag = FALSE_m11;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// see if already checked
	if (globals_m11->time_series_indices_aligned == UNKNOWN_m11)
		globals_m11->time_series_indices_aligned = FALSE_m11;
	else
		return(globals_m11->time_series_indices_aligned);
	
	// check overall size
	if (sizeof(TIME_SERIES_INDEX_m11) != TIME_SERIES_INDEX_BYTES_m11)
		goto TIME_SERIES_INDICES_NOT_ALIGNED_m11;
	
	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(TIME_SERIES_INDEX_BYTES_m11);
		free_flag = TRUE_m11;
	}
	tsi = (TIME_SERIES_INDEX_m11 *) bytes;
	if (&tsi->file_offset != (si8 *) (bytes + TIME_SERIES_INDEX_FILE_OFFSET_OFFSET_m11))
		goto TIME_SERIES_INDICES_NOT_ALIGNED_m11;
	if (&tsi->start_time != (si8 *) (bytes + TIME_SERIES_INDEX_START_TIME_OFFSET_m11))
		goto TIME_SERIES_INDICES_NOT_ALIGNED_m11;
	if (&tsi->start_sample_number != (si8 *) (bytes + TIME_SERIES_INDEX_START_SAMPLE_NUMBER_OFFSET_m11))
		goto TIME_SERIES_INDICES_NOT_ALIGNED_m11;
	
	// aligned
	globals_m11->time_series_indices_aligned = TRUE_m11;
	
	if (free_flag == TRUE_m11)
		free((void *) bytes);
	
	if (globals_m11->verbose == TRUE_m11)
		message_m11("TIME_SERIES_INDEX_m11 structure is aligned\n");
	
	return(TRUE_m11);
	
	// not aligned
TIME_SERIES_INDICES_NOT_ALIGNED_m11:
	
	if (free_flag == TRUE_m11)
		free((void *) bytes);
	
	if (globals_m11->verbose == TRUE_m11)
		error_message_m11("%s(): TIME_SERIES_INDEX_m11 structure is NOT aligned\n", __FUNCTION__);
	
	return(FALSE_m11);
}


TERN_m11	check_time_series_metadata_section_2_alignment_m11(ui1 *bytes)
{
	TIME_SERIES_METADATA_SECTION_2_m11	*md2;
	TERN_m11				free_flag = FALSE_m11;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// see if already checked
	if (globals_m11->time_series_metadata_section_2_aligned == UNKNOWN_m11)
		globals_m11->time_series_metadata_section_2_aligned = FALSE_m11;
	else
		return(globals_m11->time_series_metadata_section_2_aligned);
	
	// check overall size
	if (sizeof(TIME_SERIES_METADATA_SECTION_2_m11) != METADATA_SECTION_2_BYTES_m11)
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11;
	
	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(METADATA_FILE_BYTES_m11);
		free_flag = TRUE_m11;
	}
	md2 = (TIME_SERIES_METADATA_SECTION_2_m11 *) (bytes + METADATA_SECTION_2_OFFSET_m11);
	// channel type independent fields
	if (md2->session_description != (si1 *) (bytes + METADATA_SESSION_DESCRIPTION_OFFSET_m11))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (md2->channel_description != (si1 *) (bytes + METADATA_CHANNEL_DESCRIPTION_OFFSET_m11))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (md2->segment_description != (si1 *) (bytes + METADATA_SEGMENT_DESCRIPTION_OFFSET_m11))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (md2->equipment_description != (si1 *) (bytes + METADATA_EQUIPMENT_DESCRIPTION_OFFSET_m11))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&md2->acquisition_channel_number != (si4 *) (bytes + METADATA_ACQUISITION_CHANNEL_NUMBER_OFFSET_m11))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11;
	// channel type specific fields
	if (md2->reference_description != (si1 *) (bytes + TIME_SERIES_METADATA_REFERENCE_DESCRIPTION_OFFSET_m11))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&md2->sampling_frequency != (sf8 *) (bytes + TIME_SERIES_METADATA_SAMPLING_FREQUENCY_OFFSET_m11))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&md2->low_frequency_filter_setting != (sf8 *) (bytes + TIME_SERIES_METADATA_LOW_FREQUENCY_FILTER_SETTING_OFFSET_m11))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&md2->high_frequency_filter_setting != (sf8 *) (bytes + TIME_SERIES_METADATA_HIGH_FREQUENCY_FILTER_SETTING_OFFSET_m11))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&md2->notch_filter_frequency_setting != (sf8 *) (bytes + TIME_SERIES_METADATA_NOTCH_FILTER_FREQUENCY_SETTING_OFFSET_m11))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&md2->AC_line_frequency != (sf8 *) (bytes + TIME_SERIES_METADATA_AC_LINE_FREQUENCY_OFFSET_m11))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&md2->amplitude_units_conversion_factor != (sf8 *) (bytes + TIME_SERIES_METADATA_AMPLITUDE_UNITS_CONVERSION_FACTOR_OFFSET_m11))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (md2->amplitude_units_description != (si1 *) (bytes + TIME_SERIES_METADATA_AMPLITUDE_UNITS_DESCRIPTION_OFFSET_m11))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&md2->time_base_units_conversion_factor != (sf8 *) (bytes + TIME_SERIES_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_OFFSET_m11))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (md2->time_base_units_description != (si1 *) (bytes + TIME_SERIES_METADATA_TIME_BASE_UNITS_DESCRIPTION_OFFSET_m11))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&md2->absolute_start_sample_number != (si8 *) (bytes + TIME_SERIES_METADATA_ABSOLUTE_START_SAMPLE_NUMBER_OFFSET_m11))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&md2->number_of_samples != (si8 *) (bytes + TIME_SERIES_METADATA_NUMBER_OF_SAMPLES_OFFSET_m11))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&md2->number_of_blocks != (si8 *) (bytes + TIME_SERIES_METADATA_NUMBER_OF_BLOCKS_OFFSET_m11))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&md2->maximum_block_bytes != (si8 *) (bytes + TIME_SERIES_METADATA_MAXIMUM_BLOCK_BYTES_OFFSET_m11))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&md2->maximum_block_samples != (ui4 *) (bytes + TIME_SERIES_METADATA_MAXIMUM_BLOCK_SAMPLES_OFFSET_m11))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&md2->maximum_block_keysample_bytes != (ui4 *) (bytes + TIME_SERIES_METADATA_MAXIMUM_BLOCK_KEYSAMPLE_BYTES_OFFSET_m11))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&md2->maximum_block_duration != (sf8 *) (bytes + TIME_SERIES_METADATA_MAXIMUM_BLOCK_DURATION_OFFSET_m11))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&md2->number_of_discontinuities != (si8 *) (bytes + TIME_SERIES_METADATA_NUMBER_OF_DISCONTINUITIES_OFFSET_m11))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&md2->maximum_contiguous_blocks != (si8 *) (bytes + TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_BLOCKS_OFFSET_m11))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&md2->maximum_contiguous_block_bytes != (si8 *) (bytes + TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_BLOCK_BYTES_OFFSET_m11))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&md2->maximum_contiguous_samples != (si8 *) (bytes + TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_SAMPLES_OFFSET_m11))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (md2->protected_region != (ui1 *) (bytes + TIME_SERIES_METADATA_SECTION_2_PROTECTED_REGION_OFFSET_m11))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (md2->discretionary_region != (ui1 *) (bytes + TIME_SERIES_METADATA_SECTION_2_DISCRETIONARY_REGION_OFFSET_m11))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11;
	
	// aligned
	globals_m11->time_series_metadata_section_2_aligned = TRUE_m11;
	
	if (free_flag == TRUE_m11)
		free((void *) bytes);
	
	if (globals_m11->verbose == TRUE_m11)
		message_m11("TIME_SERIES_METADATA_SECTION_2_m11 structure is aligned\n");
	
	return(TRUE_m11);
	
	// not aligned
TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m11:
	
	if (free_flag == TRUE_m11)
		free((void *) bytes);
	
	if (globals_m11->verbose == TRUE_m11)
		error_message_m11("%s(): TIME_SERIES_METADATA_SECTION_2_m11 structure is NOT aligned\n", __FUNCTION__);
	
	return(FALSE_m11);
}


TERN_m11	check_universal_header_alignment_m11(ui1 *bytes)
{
	UNIVERSAL_HEADER_m11	*uh;
	TERN_m11		free_flag = FALSE_m11;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// see if already checked
	if (globals_m11->universal_header_aligned == UNKNOWN_m11)
		globals_m11->universal_header_aligned = FALSE_m11;
	else
		return(globals_m11->universal_header_aligned);
	
	// check overall size
	if (sizeof(UNIVERSAL_HEADER_m11) != UNIVERSAL_HEADER_BYTES_m11)
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m11;
	
	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(UNIVERSAL_HEADER_BYTES_m11);
		free_flag = TRUE_m11;
	}
	uh = (UNIVERSAL_HEADER_m11 *) bytes;
	if (&uh->header_CRC != (ui4 *) (bytes + UNIVERSAL_HEADER_HEADER_CRC_OFFSET_m11))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m11;
	if (&uh->body_CRC != (ui4 *) (bytes + UNIVERSAL_HEADER_BODY_CRC_OFFSET_m11))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m11;
	if (&uh->segment_end_time != (si8 *) (bytes + UNIVERSAL_HEADER_FILE_END_TIME_OFFSET_m11))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m11;
	if (&uh->number_of_entries != (si8 *) (bytes + UNIVERSAL_HEADER_NUMBER_OF_ENTRIES_OFFSET_m11))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m11;
	if (&uh->maximum_entry_size != (ui4 *) (bytes + UNIVERSAL_HEADER_MAXIMUM_ENTRY_SIZE_OFFSET_m11))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m11;
	if (&uh->segment_number != (si4 *) (bytes + UNIVERSAL_HEADER_SEGMENT_NUMBER_OFFSET_m11))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m11;
	if (uh->type_string != (si1 *) (bytes + UNIVERSAL_HEADER_TYPE_STRING_OFFSET_m11))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m11;
	if (&uh->type_code != (ui4 *) (bytes + UNIVERSAL_HEADER_TYPE_CODE_OFFSET_m11))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m11;
	if (&uh->type_string_terminal_zero != (si1 *) (bytes + UNIVERSAL_HEADER_TYPE_STRING_TERMINAL_ZERO_OFFSET_m11))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m11;
	if (&uh->MED_version_major != (ui1 *) (bytes + UNIVERSAL_HEADER_MED_VERSION_MAJOR_OFFSET_m11))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m11;
	if (&uh->MED_version_minor != (ui1 *) (bytes + UNIVERSAL_HEADER_MED_VERSION_MINOR_OFFSET_m11))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m11;
	if (&uh->byte_order_code != (ui1 *) (bytes + UNIVERSAL_HEADER_BYTE_ORDER_CODE_OFFSET_m11))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m11;
	if (&uh->session_start_time != (si8 *) (bytes + UNIVERSAL_HEADER_SESSION_START_TIME_OFFSET_m11))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m11;
	if (&uh->segment_start_time != (si8 *) (bytes + UNIVERSAL_HEADER_FILE_START_TIME_OFFSET_m11))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m11;
	if (uh->session_name != (si1 *) (bytes + UNIVERSAL_HEADER_SESSION_NAME_OFFSET_m11))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m11;
	if (uh->channel_name != (si1 *)  (bytes + UNIVERSAL_HEADER_CHANNEL_NAME_OFFSET_m11))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m11;
	if (uh->anonymized_subject_ID != (si1 *) (bytes + UNIVERSAL_HEADER_ANONYMIZED_SUBJECT_ID_OFFSET_m11))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m11;
	if (&uh->session_UID != (ui8 *) (bytes + UNIVERSAL_HEADER_SESSION_UID_OFFSET_m11))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m11;
	if (&uh->channel_UID != (ui8 *) (bytes + UNIVERSAL_HEADER_CHANNEL_UID_OFFSET_m11))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m11;
	if (&uh->segment_UID != (ui8 *) (bytes + UNIVERSAL_HEADER_SEGMENT_UID_OFFSET_m11))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m11;
	if (&uh->file_UID != (ui8 *) (bytes + UNIVERSAL_HEADER_FILE_UID_OFFSET_m11))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m11;
	if (&uh->provenance_UID != (ui8 *) (bytes + UNIVERSAL_HEADER_PROVENANCE_UID_OFFSET_m11))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m11;
	if (uh->level_1_password_validation_field != (ui1 *) (bytes + UNIVERSAL_HEADER_LEVEL_1_PASSWORD_VALIDATION_FIELD_OFFSET_m11))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m11;
	if (uh->level_2_password_validation_field != (ui1 *) (bytes + UNIVERSAL_HEADER_LEVEL_2_PASSWORD_VALIDATION_FIELD_OFFSET_m11))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m11;
	if (uh->level_3_password_validation_field != (ui1 *) (bytes + UNIVERSAL_HEADER_LEVEL_3_PASSWORD_VALIDATION_FIELD_OFFSET_m11))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m11;
	if (uh->protected_region != (ui1 *) (bytes + UNIVERSAL_HEADER_PROTECTED_REGION_OFFSET_m11))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m11;
	if (uh->discretionary_region != (ui1 *) (bytes + UNIVERSAL_HEADER_DISCRETIONARY_REGION_OFFSET_m11))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m11;
	
	// aligned
	globals_m11->universal_header_aligned = TRUE_m11;
	
	if (free_flag == TRUE_m11)
		free((void *) bytes);
	
	if (globals_m11->verbose == TRUE_m11)
		message_m11("UNIVERSAL_HEADER_m11 structure is aligned\n");
	
	return(TRUE_m11);
	
	// not aligned
UNIVERSAL_HEADER_NOT_ALIGNED_m11:
	
	if (free_flag == TRUE_m11)
		free((void *) bytes);
	
	if (globals_m11->verbose == TRUE_m11)
		error_message_m11("%s(): UNIVERSAL_HEADER_m11 structure is NOT aligned\n", __FUNCTION__);
	
	return(FALSE_m11);
}


TERN_m11	check_video_indices_alignment_m11(ui1 *bytes)
{
	VIDEO_INDEX_m11		*vi;
	TERN_m11		free_flag = FALSE_m11;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// see if already checked
	if (globals_m11->video_indices_aligned == UNKNOWN_m11)
		globals_m11->video_indices_aligned = FALSE_m11;
	else
		return(globals_m11->video_indices_aligned);
	
	// check overall size
	if (sizeof(VIDEO_INDEX_m11) != VIDEO_INDEX_BYTES_m11)
		goto VIDEO_INDICES_NOT_ALIGNED_m11;
	
	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(VIDEO_INDEX_BYTES_m11);
		free_flag = TRUE_m11;
	}
	vi = (VIDEO_INDEX_m11 *) bytes;
	if (&vi->file_offset != (si8 *) (bytes + VIDEO_INDEX_FILE_OFFSET_OFFSET_m11))
		goto VIDEO_INDICES_NOT_ALIGNED_m11;
	if (&vi->start_time != (si8 *) (bytes + VIDEO_INDEX_START_TIME_OFFSET_m11))
		goto VIDEO_INDICES_NOT_ALIGNED_m11;
	if (&vi->start_frame_number != (ui4 *) (bytes + VIDEO_INDEX_START_FRAME_OFFSET_m11))
		goto VIDEO_INDICES_NOT_ALIGNED_m11;
	if (&vi->video_file_number != (ui4 *) (bytes + VIDEO_INDEX_VIDEO_FILE_NUMBER_OFFSET_m11))
		goto VIDEO_INDICES_NOT_ALIGNED_m11;
	
	// aligned
	globals_m11->video_indices_aligned = TRUE_m11;
	
	if (free_flag == TRUE_m11)
		free((void *) bytes);
	
	if (globals_m11->verbose == TRUE_m11)
		message_m11("VIDEO_INDEX_m11 structure is aligned\n");
	
	return(TRUE_m11);
	
	// not aligned
VIDEO_INDICES_NOT_ALIGNED_m11:
	
	if (free_flag == TRUE_m11)
		free((void *) bytes);
	
	if (globals_m11->verbose == TRUE_m11)
		error_message_m11("%s(): VIDEO_INDEX_m11 structure is NOT aligned\n", __FUNCTION__);
	
	return(FALSE_m11);
}


TERN_m11	check_video_metadata_section_2_alignment_m11(ui1 *bytes)
{
	VIDEO_METADATA_SECTION_2_m11	*vmd2;
	TERN_m11			free_flag = FALSE_m11;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// see if already checked
	if (globals_m11->video_metadata_section_2_aligned == UNKNOWN_m11)
		globals_m11->video_metadata_section_2_aligned = FALSE_m11;
	else
		return(globals_m11->video_metadata_section_2_aligned);
	
	// check overall size
	if (sizeof(VIDEO_METADATA_SECTION_2_m11) != METADATA_SECTION_2_BYTES_m11)
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m11;
	
	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(METADATA_FILE_BYTES_m11);
		free_flag = TRUE_m11;
	}
	vmd2 = (VIDEO_METADATA_SECTION_2_m11 *) (bytes + METADATA_SECTION_2_OFFSET_m11);
	// channel type independent fields
	if (vmd2->session_description != (si1 *) (bytes + METADATA_SESSION_DESCRIPTION_OFFSET_m11))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (vmd2->channel_description != (si1 *) (bytes + METADATA_CHANNEL_DESCRIPTION_OFFSET_m11))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (vmd2->equipment_description != (si1 *) (bytes + METADATA_EQUIPMENT_DESCRIPTION_OFFSET_m11))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&vmd2->acquisition_channel_number != (si4 *) (bytes + METADATA_ACQUISITION_CHANNEL_NUMBER_OFFSET_m11))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m11;
	// channel type specific fields
	if (&vmd2->time_base_units_conversion_factor != (sf8 *) (bytes + VIDEO_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_OFFSET_m11))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (vmd2->time_base_units_description != (si1 *) (bytes + VIDEO_METADATA_TIME_BASE_UNITS_DESCRIPTION_OFFSET_m11))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&vmd2->absolute_start_frame_number != (si8 *) (bytes + VIDEO_METADATA_ABSOLUTE_START_FRAME_NUMBER_OFFSET_m11))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&vmd2->number_of_frames != (si8 *) (bytes + VIDEO_METADATA_NUMBER_OF_FRAMES_OFFSET_m11))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&vmd2->frame_rate != (sf8 *) (bytes + VIDEO_METADATA_FRAME_RATE_OFFSET_m11))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&vmd2->number_of_clips != (si8 *) (bytes + VIDEO_METADATA_NUMBER_OF_CLIPS_OFFSET_m11))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&vmd2->maximum_clip_bytes != (si8 *) (bytes + VIDEO_METADATA_MAXIMUM_CLIP_BYTES_OFFSET_m11))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&vmd2->maximum_clip_frames != (ui4 *) (bytes + VIDEO_METADATA_MAXIMUM_CLIP_FRAMES_OFFSET_m11))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&vmd2->number_of_video_files != (si4 *) (bytes + VIDEO_METADATA_NUMBER_OF_VIDEO_FILES_OFFSET_m11))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&vmd2->maximum_clip_duration != (sf8 *) (bytes + VIDEO_METADATA_MAXIMUM_CLIP_DURATION_OFFSET_m11))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&vmd2->number_of_discontinuities != (si8 *) (bytes + VIDEO_METADATA_NUMBER_OF_DISCONTINUITIES_OFFSET_m11))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&vmd2->maximum_contiguous_clips != (si8 *) (bytes + VIDEO_METADATA_MAXIMUM_CONTIGUOUS_CLIPS_OFFSET_m11))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&vmd2->maximum_contiguous_clip_bytes != (si8 *) (bytes + VIDEO_METADATA_MAXIMUM_CONTIGUOUS_CLIP_BYTES_OFFSET_m11))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&vmd2->maximum_contiguous_frames != (si8 *) (bytes + VIDEO_METADATA_MAXIMUM_CONTIGUOUS_FRAMES_OFFSET_m11))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&vmd2->horizontal_pixels != (ui4 *) (bytes + VIDEO_METADATA_HORIZONTAL_PIXELS_OFFSET_m11))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (&vmd2->vertical_pixels != (ui4 *) (bytes + VIDEO_METADATA_VERTICAL_PIXELS_OFFSET_m11))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (vmd2->video_format != (si1 *) (bytes + VIDEO_METADATA_VIDEO_FORMAT_OFFSET_m11))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (vmd2->protected_region != (ui1 *) (bytes + VIDEO_METADATA_SECTION_2_PROTECTED_REGION_OFFSET_m11))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m11;
	if (vmd2->discretionary_region != (ui1 *) (bytes + VIDEO_METADATA_SECTION_2_DISCRETIONARY_REGION_OFFSET_m11))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m11;
	
	// aligned
	globals_m11->video_metadata_section_2_aligned = TRUE_m11;
	
	if (free_flag == TRUE_m11)
		free((void *) bytes);
	
	if (globals_m11->verbose == TRUE_m11)
		message_m11("VIDEO_METADATA_SECTION_2_m11 structure is aligned\n");
	
	return(TRUE_m11);
	
	// not aligned
VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m11:
	
	if (free_flag == TRUE_m11)
		free((void *) bytes);
	
	if (globals_m11->verbose == TRUE_m11)
		error_message_m11("%s(): VIDEO_METADATA_SECTION_2_m11 structure is NOT aligned\n", __FUNCTION__);
	
	return(FALSE_m11);
}


void clear_terminal_m11(void)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif

#if defined MACOS_m11 || defined LINUX_m11
	system_m11("clear", FALSE_m11, __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
#endif
#ifdef WINDOWS_m11
	win_clear_m11();
#endif
	
	return;
}


si4	compare_acq_nums_m11(const void *a, const void *b)
{
	ACQ_NUM_SORT_m11	*as, *bs;
	
	as = (ACQ_NUM_SORT_m11 *) a;
	bs = (ACQ_NUM_SORT_m11 *) b;
	
	if (as->acq_num > bs->acq_num)
		return(1);
	if (as->acq_num < bs->acq_num)
		return(-1);
	return(0);
}


void	condition_timezone_info_m11(TIMEZONE_INFO_m11 *tz_info)
{
	si4			i;
	si8			len;
	TIMEZONE_ALIAS_m11	*tz_aliases_table;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (globals_m11->timezone_table == NULL)
		initialize_timezone_tables_m11();

	// Country: at this time there are no 2 or 3 letter country names => user probably entered acronym
	if (*tz_info->country) {
		len = strlen(tz_info->country);
		if (len == 2) {
			strcpy(tz_info->country_acronym_2_letter, tz_info->country);
			*tz_info->country = 0;
		} else if (len == 3) {
			strcpy(tz_info->country_acronym_3_letter, tz_info->country);
			*tz_info->country = 0;
		}
	}
	
	// Territory: at this time there are no 2 letter territory names => user probably entered acronym
	if (*tz_info->territory) {
		len = strlen(tz_info->territory);
		if (len == 2) {
			strcpy(tz_info->territory_acronym, tz_info->territory);
			*tz_info->territory = 0;
		}
	}
	
	// change potential matching strings to caps
	STR_to_upper_m11(tz_info->country);
	STR_to_upper_m11(tz_info->country_acronym_2_letter);
	STR_to_upper_m11(tz_info->country_acronym_3_letter);
	STR_to_upper_m11(tz_info->territory);
	STR_to_upper_m11(tz_info->territory_acronym);
	STR_to_upper_m11(tz_info->standard_timezone);
	STR_to_upper_m11(tz_info->standard_timezone_acronym);
	STR_to_upper_m11(tz_info->daylight_timezone);
	STR_to_upper_m11(tz_info->daylight_timezone_acronym);
	
	// check country aliases
	tz_aliases_table = globals_m11->country_aliases_table;
	
	if (*tz_info->country) {
		for (i = 0; i < TZ_COUNTRY_ALIASES_ENTRIES_m11; ++i) {
			if ((strcmp(tz_info->country, tz_aliases_table[i].alias)) == 0) {
				strcpy(tz_info->country, tz_aliases_table[i].table_name);
				break;
			}
		}
	}
	
	// check country acronyms
	tz_aliases_table = globals_m11->country_acronym_aliases_table;
	
	if (*tz_info->country_acronym_2_letter) {
		for (i = 0; i < TZ_COUNTRY_ACRONYM_ALIASES_ENTRIES_m11; ++i) {
			if ((strcmp(tz_info->country_acronym_2_letter, tz_aliases_table[i].alias)) == 0) {
				strcpy(tz_info->country_acronym_2_letter, tz_aliases_table[i].table_name);
				break;
			}
		}
	}
	
	if (*tz_info->country_acronym_3_letter) {
		for (i = 0; i < TZ_COUNTRY_ALIASES_ENTRIES_m11; ++i) {
			if ((strcmp(tz_info->country_acronym_3_letter, tz_aliases_table[i].alias)) == 0) {
				strcpy(tz_info->country_acronym_3_letter, tz_aliases_table[i].table_name);
				break;
			}
		}
	}

	return;
}


void	condition_time_slice_m11(TIME_SLICE_m11 *slice)
{
	si8		test_time;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (slice == NULL) {
		warning_message_m11("%s(): passed time slice is NULL\n");
		return;
	}
	
	if (globals_m11->recording_time_offset == FALSE_m11) {
		if (globals_m11->verbose == TRUE_m11)
			warning_message_m11("%s(): recording time offset is not known => assuming no offset\n", __FUNCTION__);
		globals_m11->recording_time_offset = GLOBALS_RECORDING_TIME_OFFSET_DEFAULT_m11;  // == 0
		if (globals_m11->session_start_time == UUTC_NO_ENTRY_m11)
			globals_m11->session_start_time = BEGINNING_OF_TIME_m11;
	}
	
	if (slice->start_time <= 0) {
		if (slice->start_time == UUTC_NO_ENTRY_m11) {
			if (slice->start_sample_number == SAMPLE_NUMBER_NO_ENTRY_m11)
				slice->start_time = BEGINNING_OF_TIME_m11;
		} else {  // relative time
			slice->start_time = globals_m11->session_start_time - slice->start_time;
		}
	} else {  // ? unoffset time
		test_time = slice->start_time - globals_m11->recording_time_offset;
		if (test_time > 0)  // start time is not offset
			slice->start_time = test_time;
	}
	
	if (slice->end_time <= 0) {
		if (slice->end_time == UUTC_NO_ENTRY_m11) {
			if (slice->end_sample_number == SAMPLE_NUMBER_NO_ENTRY_m11)
				slice->end_time = END_OF_TIME_m11;
		} else {  // relative time
			slice->end_time = globals_m11->session_start_time - slice->end_time;
		}
	} else {  // ? unoffset time
		test_time = slice->end_time - globals_m11->recording_time_offset;
		if (test_time > 0 && slice->end_time != END_OF_TIME_m11)  // end time is not offset
			slice->end_time = test_time;
	}
	
	slice->conditioned = TRUE_m11;
		
	return;
}


#if defined MACOS_m11 || defined LINUX_m11
inline si8      current_uutc_m11(void)
{
	struct timeval  tv;
	si8             uutc;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	gettimeofday(&tv, NULL);
	uutc = (si8) tv.tv_sec * (si8) 1000000 + (si8) tv.tv_usec;
	
	return(uutc);
}
#endif


#ifdef WINDOWS_m11
si8      current_uutc_m11(void)
{
	static const ui8    EPOCH = (ui8) 116444736000000000;
	struct timeval      tv;
	si8                 uutc;
	SYSTEMTIME          system_time;
	FILETIME            file_time;
	ui8                 time;
	
	
	GetSystemTime(&system_time);
	SystemTimeToFileTime(&system_time, &file_time);
	time = ((ui8)file_time.dwLowDateTime);
	time += ((ui8)file_time.dwHighDateTime) << 32;
	
	tv.tv_sec = (si4)((time - EPOCH) / (ui8) 10000000);
	tv.tv_usec = (si4)(system_time.wMilliseconds * 1000);
	uutc = (si8)tv.tv_sec * (si8) 1000000 + (si8) tv.tv_usec;
	
	return(uutc);
}
#endif


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4      days_in_month_m11(si4 month, si4 year)
// Note month is [0 - 11], January == 0, as in unix struct tm.tm_mon
// Note struct tm.tm_year is (year - 1900), this function expects the full value
{
	static const si4        standard_days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	si4                     days;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	days = standard_days[month];
	
	// leap years
	if (month == 1) {  // February
		if ((year % 4) == 0) {  // leap year
			++days;
			if ((year % 100) == 0) {  // centurial exception
				--days;
				if ((year % 400) == 0) {  // centurial exception exception
					++days;
				}
			}
		}
	}
	
	return(days);
}


TERN_m11	decrypt_metadata_m11(FILE_PROCESSING_STRUCT_m11 *fps)
{
	ui1			*ui1_p, *decryption_key;
	si4		        i, decryption_blocks;
	PASSWORD_DATA_m11	*pwd;
	METADATA_SECTION_3_m11	*section_3;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (fps == NULL) {
		error_message_m11("%s(): FILE_PROCESSING_STRUCT is NULL\n", __FUNCTION__);
		return(FALSE_m11);
	}
		
	pwd = fps->parameters.password_data;
	if (pwd == NULL)
		pwd = &globals_m11->password_data;
	
	// time series encryption global
	globals_m11->time_series_data_encryption_level = fps->metadata->section_1.time_series_data_encryption_level;
	
	// section 2 decryption
	if (fps->metadata->section_1.section_2_encryption_level > NO_ENCRYPTION_m11) {  // natively encrypted and currently encrypted
		if (pwd->access_level >= fps->metadata->section_1.section_2_encryption_level) {
			if (fps->metadata->section_1.section_2_encryption_level == LEVEL_1_ENCRYPTION_m11)
				decryption_key = pwd->level_1_encryption_key;
			else
				decryption_key = pwd->level_2_encryption_key;
			decryption_blocks = METADATA_SECTION_2_BYTES_m11 / ENCRYPTION_BLOCK_BYTES_m11;
			ui1_p = fps->parameters.raw_data + METADATA_SECTION_2_OFFSET_m11;
			for (i = 0; i < decryption_blocks; ++i) {
				AES_decrypt_m11(ui1_p, ui1_p, NULL, decryption_key);
				ui1_p += ENCRYPTION_BLOCK_BYTES_m11;
			}
			fps->metadata->section_1.section_2_encryption_level = -fps->metadata->section_1.section_2_encryption_level;  // mark as currently decrypted
		} else {
			error_message_m11("%s(): Section 2 of the Metadata is encrypted at level %hhd => cannot decrypt\n", __FUNCTION__, fps->metadata->section_1.section_2_encryption_level);
			show_password_hints_m11(pwd);
			set_error_m11(E_BAD_PASSWORD_m11, __FUNCTION__, __LINE__);
			return(FALSE_m11);  // can't do anything without section 2, so fail
		}
	}
	
	// section 3 decryption
	if (fps->metadata->section_1.section_3_encryption_level > NO_ENCRYPTION_m11) {  // natively encrypted and currently encrypted
		if (pwd->access_level >= fps->metadata->section_1.section_3_encryption_level) {
			if (fps->metadata->section_1.section_3_encryption_level == LEVEL_1_ENCRYPTION_m11)
				decryption_key = pwd->level_1_encryption_key;
			else
				decryption_key = pwd->level_2_encryption_key;
			decryption_blocks = METADATA_SECTION_3_BYTES_m11 / ENCRYPTION_BLOCK_BYTES_m11;
			ui1_p = fps->parameters.raw_data + METADATA_SECTION_3_OFFSET_m11;
			for (i = 0; i < decryption_blocks; ++i) {
				AES_decrypt_m11(ui1_p, ui1_p, NULL, decryption_key);
				ui1_p += ENCRYPTION_BLOCK_BYTES_m11;
			}
			fps->metadata->section_1.section_3_encryption_level = -fps->metadata->section_1.section_3_encryption_level;  // mark as currently decrypted
		} else {
			if (globals_m11->verbose == TRUE_m11) {
				warning_message_m11("%s(): Metadata section 3 encrypted at level %hhd => cannot decrypt\n", __FUNCTION__, fps->metadata->section_1.section_3_encryption_level);
				show_password_hints_m11(pwd);
			}
			globals_m11->RTO_known = FALSE_m11;
			globals_m11->time_constants_set = TRUE_m11;  // set to defaults
			return(TRUE_m11);  // can function without section 3, so return TRUE_m11
		}
	}
	
	// set global time data
	if (globals_m11->RTO_known != TRUE_m11) {  // UNKNOWN || FALSE
		section_3 = &fps->metadata->section_3;
		globals_m11->recording_time_offset = section_3->recording_time_offset;
		globals_m11->standard_UTC_offset = section_3->standard_UTC_offset;
		strncpy_m11(globals_m11->standard_timezone_acronym, section_3->standard_timezone_acronym, TIMEZONE_ACRONYM_BYTES_m11);
		strncpy_m11(globals_m11->standard_timezone_string, section_3->standard_timezone_string, TIMEZONE_STRING_BYTES_m11);
		strncpy_m11(globals_m11->daylight_timezone_acronym, section_3->daylight_timezone_acronym, TIMEZONE_ACRONYM_BYTES_m11);
		strncpy_m11(globals_m11->daylight_timezone_string, section_3->daylight_timezone_string, TIMEZONE_STRING_BYTES_m11);
		if ((globals_m11->daylight_time_start_code.value = section_3->daylight_time_start_code.value) == DTCC_VALUE_NOT_OBSERVED_m11)
			globals_m11->observe_DST = FALSE_m11;
		else
			globals_m11->observe_DST = TRUE_m11;
		globals_m11->RTO_known = TRUE_m11;
		globals_m11->daylight_time_end_code.value = section_3->daylight_time_end_code.value;
		globals_m11->time_constants_set = TRUE_m11;
	}
	
	return(TRUE_m11);
}


TERN_m11	decrypt_record_data_m11(FILE_PROCESSING_STRUCT_m11 *fps, ...)  // varargs (fps == NULL): RECORD_HEADER_m11 *rh, si8 number_of_records
{
	ui1			*ui1_p, *encryption_key;
	ui4			j, encryption_blocks;
	si8			i, failed_decryption_count, number_of_records;
	va_list			args;
	RECORD_HEADER_m11	*rh;
	PASSWORD_DATA_m11	*pwd;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (fps == NULL) {
		va_start(args, fps);
		rh = va_arg(args, RECORD_HEADER_m11 *);
		number_of_records = va_arg(args, si8);
		va_end(args);
		pwd = NULL;
	} else {
		rh = (RECORD_HEADER_m11 *) fps->record_data;
		number_of_records = fps->number_of_items;
		pwd = fps->parameters.password_data;
	}
	if (number_of_records == 0)  // failure == all records unreadable
		return(TRUE_m11);
	if (pwd == NULL)
		pwd = &globals_m11->password_data;

	for (i = failed_decryption_count = 0; i < number_of_records; ++i) {
		ui1_p = (ui1 *) rh;
		if (rh->encryption_level > NO_ENCRYPTION_m11) {
			if (pwd->access_level >= rh->encryption_level) {
				if (rh->encryption_level == LEVEL_1_ENCRYPTION_m11)
					encryption_key = pwd->level_1_encryption_key;
				else
					encryption_key = pwd->level_2_encryption_key;
				encryption_blocks = (rh->total_record_bytes - RECORD_HEADER_BYTES_m11) / ENCRYPTION_BLOCK_BYTES_m11;
				ui1_p += RECORD_HEADER_BYTES_m11;
				for (j = 0; j < encryption_blocks; ++j) {
					AES_decrypt_m11(ui1_p, ui1_p, NULL, encryption_key);
					ui1_p += ENCRYPTION_BLOCK_BYTES_m11;
				}
				rh->encryption_level = -rh->encryption_level;  // mark as currently decrypted
			} else {
				++failed_decryption_count;
				if (globals_m11->verbose == TRUE_m11)
					warning_message_m11("%s(): Cannot decrypt record => skipping\n", __FUNCTION__);
			}
		}
		rh = (RECORD_HEADER_m11 *) ((ui1 *) rh + rh->total_record_bytes);
	}

	if (failed_decryption_count == number_of_records)  // failure == all records unreadable
		return(FALSE_m11);
	
	return(TRUE_m11);
}


TERN_m11     decrypt_time_series_data_m11(FILE_PROCESSING_STRUCT_m11 *fps)
{
	ui1				*ui1_p, *key = NULL;
	si4                             encryption_blocks, encryptable_blocks;
	si8                             i, encryption_bytes, number_of_items;
	CMP_BLOCK_FIXED_HEADER_m11	*bh;
	PASSWORD_DATA_m11		*pwd;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (fps->metadata->section_1.time_series_data_encryption_level == NO_ENCRYPTION_m11)
		return(TRUE_m11);
	
	// get decryption key - assume all blocks encrypted at same level
	pwd = fps->parameters.password_data;
	if (pwd == NULL)
		pwd = &globals_m11->password_data;
	bh = fps->parameters.cps->block_header;
	
	number_of_items = fps->number_of_items;	// looks like big loop but breaks out as soon as encrypted block encountered
	for (i = 0; i < number_of_items; ++i) {	// if none encountered, function done
		// check if already decrypted
		if ((bh->block_flags & CMP_BF_ENCRYPTION_MASK_m11) == 0) {
			bh = (CMP_BLOCK_FIXED_HEADER_m11 *) ((ui1 *)bh + bh->total_block_bytes);
			continue;
		}
		if (bh->block_flags & CMP_BF_LEVEL_1_ENCRYPTION_MASK_m11) {
			if (bh->block_flags & CMP_BF_LEVEL_2_ENCRYPTION_MASK_m11) {
				error_message_m11("%s(): Cannot decrypt data: flags indicate both level 1 & level 2 encryption\n", __FUNCTION__);
				return(FALSE_m11);
			}
			if (pwd->access_level >= LEVEL_1_ENCRYPTION_m11) {
				key = pwd->level_1_encryption_key;
				break;
			} else {
				error_message_m11("%s(): Cannot decrypt data: insufficient access\n", __FUNCTION__);
				return(FALSE_m11);
			}
		} else {  // level 2 bit is set
			if (pwd->access_level == LEVEL_2_ENCRYPTION_m11) {
				key = pwd->level_2_encryption_key;
				break;
			} else {
				error_message_m11("%s(): Cannot decrypt data: insufficient access\n", __FUNCTION__);
				return(FALSE_m11);
			}
		}
		bh = (CMP_BLOCK_FIXED_HEADER_m11 *) ((ui1 *) bh + bh->total_block_bytes);
	}
	
	// no blocks encrypted
	if (i == number_of_items)
		return(TRUE_m11);
	
	// decrypt
	bh = fps->parameters.cps->block_header;
	for (i = number_of_items; i--;) {
		
		// block if already decrypted
		if ((bh->block_flags & CMP_BF_ENCRYPTION_MASK_m11) == 0) {
			bh = (CMP_BLOCK_FIXED_HEADER_m11 *) ((ui1 *) bh + bh->total_block_bytes);
			continue;
		}
		
		// calculated encryption blocks
		encryptable_blocks = (bh->total_block_bytes - CMP_BLOCK_ENCRYPTION_START_OFFSET_m11) / ENCRYPTION_BLOCK_BYTES_m11;
		if (bh->block_flags | CMP_BF_MBE_ENCODING_MASK_m11) {
			encryption_blocks = encryptable_blocks;
		} else {
			encryption_bytes = bh->total_header_bytes - CMP_BLOCK_ENCRYPTION_START_OFFSET_m11 + ENCRYPTION_BLOCK_BYTES_m11;
			encryption_blocks = ((encryption_bytes - 1) / ENCRYPTION_BLOCK_BYTES_m11) + 1;
			if (encryptable_blocks < encryption_blocks)
				encryption_blocks = encryptable_blocks;
		}
		
		// decrypt
		ui1_p = (ui1 *) bh + CMP_BLOCK_ENCRYPTION_START_OFFSET_m11;
		for (i = 0; i < encryption_blocks; ++i) {
			AES_decrypt_m11(ui1_p, ui1_p, NULL, key);
			ui1_p += ENCRYPTION_BLOCK_BYTES_m11;
		}
		
		// set block flags to decrypted
		bh->block_flags &= ~CMP_BF_ENCRYPTION_MASK_m11;
		
		// set pointer to next block
		bh = (CMP_BLOCK_FIXED_HEADER_m11 *) ((ui1 *) bh + bh->total_block_bytes);
	}
	
	return(TRUE_m11);
}


si4     DST_offset_m11(si8 uutc)
{
	si4                             i, month, DST_start_month, DST_end_month;
	si4                             first_weekday_of_month, target_day_of_month, last_day_of_month;
	time_t                          utc, local_utc, change_utc;
	struct tm                       time_info = { 0 }, change_time_info = { 0 };
	DAYLIGHT_TIME_CHANGE_CODE_m11	*first_DTCC, *last_DTCC, *change_DTCC;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// returns seconds to add to standard time (as UUTC) to adjust for DST on that date, in the globally specified timezone
	
	if (globals_m11->time_constants_set == FALSE_m11) {
		warning_message_m11("%s(): library time constants not set\n", __FUNCTION__);
		return(0);
	}
	if (globals_m11->observe_DST < TRUE_m11)
		return(0);
	if (globals_m11->daylight_time_start_code.value == DTCC_VALUE_NO_ENTRY_m11) {
		warning_message_m11("%s(): daylight change data not available\n", __FUNCTION__);
		return(0);
	}

	utc = uutc / (si8) 1000000;
	
	// get broken out time info
#if defined MACOS_m11 || defined LINUX_m11
	if (globals_m11->daylight_time_start_code.reference_time == DTCC_LOCAL_REFERENCE_TIME) {
		local_utc = utc + (si8) globals_m11->standard_UTC_offset;
		gmtime_r(&local_utc, &time_info);
	}
	else {
		gmtime_r(&utc, &time_info);
	}
#endif
#ifdef WINDOWS_m11
	if (globals_m11->daylight_time_start_code.reference_time == DTCC_LOCAL_REFERENCE_TIME) {
		local_utc = utc + (si8) globals_m11->standard_UTC_offset;
		time_info = *(gmtime(&local_utc));
	}
	else {
		time_info = *(gmtime(&utc));
	}
#endif
	
	month = time_info.tm_mon;
	DST_start_month = globals_m11->daylight_time_start_code.month;
	DST_end_month = globals_m11->daylight_time_end_code.month;
	if (DST_start_month < DST_end_month) {
		first_DTCC = &globals_m11->daylight_time_start_code;
		last_DTCC = &globals_m11->daylight_time_end_code;
	}
	else {
		first_DTCC = &globals_m11->daylight_time_end_code;
		last_DTCC = &globals_m11->daylight_time_start_code;
	}
	
	// take care of dates not in change months
	if (month != DST_start_month && month != DST_end_month) {
		if (month > first_DTCC->month && month < last_DTCC->month) {
			if (first_DTCC->month == DST_start_month)
				return((si4)first_DTCC->shift_minutes * (si4)60);
			else
				return(0);
		} else if (month < first_DTCC->month) {
			if (first_DTCC->month == DST_start_month)
				return(0);
			else
				return((si4)first_DTCC->shift_minutes * (si4)60);
		} else {  // month > last_DTCC->month
			if (last_DTCC->month == DST_end_month)
				return(0);
			else
				return((si4)first_DTCC->shift_minutes * (si4)60);
		}
	}
	
	// get change utc
	if (month == first_DTCC->month)
		change_DTCC = first_DTCC;
	else
		change_DTCC = last_DTCC;
	
	change_time_info.tm_hour = change_DTCC->hours_of_day;
	change_time_info.tm_mon = month;
	change_time_info.tm_year = time_info.tm_year;
	
	if (change_DTCC->day_of_month == DTCC_DAY_OF_MONTH_NO_ENTRY) {   // get target day of month
		first_weekday_of_month = time_info.tm_wday - ((time_info.tm_mday - 1) % 7);
		target_day_of_month = (change_DTCC->day_of_week - first_weekday_of_month) + 1;
		if (target_day_of_month < 1)
			target_day_of_month += 7;
		if (change_DTCC->relative_weekday_of_month == DTCC_LAST_RELATIVE_WEEKDAY_OF_MONTH) {
			last_day_of_month = days_in_month_m11(month, time_info.tm_year + 1900);
			while (target_day_of_month <= last_day_of_month)
				target_day_of_month += 7;
			target_day_of_month -= 7;
		} else {
			for (i = 1; i < change_DTCC->relative_weekday_of_month; ++i)
				target_day_of_month += 7;
		}
		change_time_info.tm_mday = target_day_of_month;
	} else {
		change_time_info.tm_mday = change_DTCC->day_of_month;
	}
	
#if defined MACOS_m11 || defined LINUX_m11
	change_utc = timegm(&change_time_info);
#endif
#ifdef WINDOWS_m11
	change_utc = _mkgmtime(&change_time_info);
#endif
	if (globals_m11->daylight_time_start_code.reference_time == DTCC_LOCAL_REFERENCE_TIME)
		change_utc -= globals_m11->standard_UTC_offset;
	
	if (change_DTCC->month == DST_start_month) {
		if (utc >= change_utc)
			return((si4)change_DTCC->shift_minutes * (si4)60);
		else
			return(0);
	} else {  // change_DTCC->month == DST_end_month
		if (utc < change_utc)
			return((si4)change_DTCC->shift_minutes * (si4)-60);
		else
			return(0);
	}
	
	return(0);
}


void    error_message_m11(si1 *fmt, ...)
{
	va_list		args;
	
	
	// RED suppressible text to stderr with option to exit program
	if (!(globals_m11->behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m11)) {
		#ifndef MATLAB_m11
		fprintf(stderr, TC_RED_m11);
		#endif
		
		va_start(args, fmt);
		UTF8_vfprintf_m11(stderr, fmt, args);
		va_end(args);
			
		#ifndef MATLAB_m11
		fprintf(stderr, TC_RESET_m11);
		fflush(stderr);
		#endif
	}
	
	if (globals_m11->behavior_on_fail & EXIT_ON_FAIL_m11) {
		#ifdef MATLAB_m11
		mexPrintf("Exiting.\n\n");
		#else
		fprintf(stderr, "Exiting.\n\n");
		#endif
		exit_m11(-1);
	}
	
	return;
}


void	error_string_m11(void)
{
	si1	*str;
	
#ifdef FN_DEBUG_m11
	#ifdef MATLAB_m11
	mexPrintf("%s()\n", __FUNCTION__);
	#else
	fprintf(stderr, "%s()\n", __FUNCTION__);
	#endif
#endif

	switch (globals_m11->err_code) {
		case E_NO_ERR_m11:
			str = E_NO_ERR_STR_m11;
			break;
		case E_NO_FILE_m11:
			str = E_NO_FILE_STR_m11;
			break;
		case E_READ_ERR_m11:
			str = E_READ_ERR_STR_m11;
			break;
		case E_WRITE_ERR_m11:
			str = E_WRITE_ERR_STR_m11;
			break;
		case E_NOT_MED_m11:
			str = E_NOT_MED_STR_m11;
			break;
		case E_BAD_PASSWORD_m11:
			str = E_BAD_PASSWORD_STR_m11;
			break;
		case E_NO_METADATA_m11:
			str = E_NO_METADATA_STR_m11;
			break;
		case E_NO_INET_m11:
			str = E_NO_INET_STR_m11;
			break;
		default:
			str = "unknown error";
			break;
	}
	
	if (globals_m11->err_func != NULL) {
		#ifdef MATLAB_m11
		mexPrintf("%s  (code %d, func %s, line %d)\n\n", str, globals_m11->err_code, globals_m11->err_func, globals_m11->err_line);
		#else
		fprintf(stderr, "%s%s%s  (code %d, func %s, line %d)\n\n", TC_RED_m11, str, TC_RESET_m11, globals_m11->err_code, globals_m11->err_func, globals_m11->err_line);
		#endif
	} else {
		#ifdef MATLAB_m11
		mexPrintf("%s  (code %d, line %d)\n\n", str, globals_m11->err_code, globals_m11->err_line);
		#else
		fprintf(stderr, "%s%s%s  (code %d, line %d)\n\n", TC_RED_m11, str, TC_RESET_m11, globals_m11->err_code, globals_m11->err_line);
		#endif
	}

	return;
}
		
		
void    escape_chars_m11(si1 *string, si1 target_char, si8 buffer_len)
{
	si1	*c1, *c2, *tmp_str, backslash;
	si8     n_target_chars, len;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	backslash = (si1) 0x5c;
	
	// count
	for (n_target_chars = 0, c1 = string; *c1++;)
		if (*c1 == target_char)
			if (*(c1 - 1) != backslash)
				++n_target_chars;
	len = (c1 - string) + n_target_chars;
	if (buffer_len != 0) {  // if zero, proceed at caller's peril
		if (buffer_len < len) {
			error_message_m11("%s(): string buffer too small\n", __FUNCTION__);
			return;
		}
	}
	
	tmp_str = (si1 *) malloc(len);
	
	c1 = string;
	c2 = tmp_str;
	while (*c1) {
		if (*c1 == target_char) {
			if (*(c1 - 1) != backslash)
				*c2++ = backslash;
		}
		*c2++ = *c1++;
	}
	*c2 = 0;
	strcpy(string, tmp_str);
	
	free((void *) tmp_str);
	
	return;
}


void	extract_path_parts_m11(si1 *full_file_name, si1 *path, si1 *name, si1 *extension)
{
	si1	*c, *cc, temp_full_file_name[FULL_FILE_NAME_BYTES_m11], dir_break;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// get path from root
	path_from_root_m11(full_file_name, temp_full_file_name);

	// move pointer to end of string
	c = temp_full_file_name + strlen(temp_full_file_name) - 1;
	
#ifdef WINDOWS_m11
	dir_break = '\\';
#endif
#if defined MACOS_m11 || defined LINUX_m11
	dir_break = '/';
#endif
	// step back to first extension
	cc = c;
	while (*--c != '.') {
		if (*c == dir_break) {
			c = cc;
			break;
		}
	}

	// copy extension if allocated
	if (extension != NULL) {
		if (*c == '.') {
			strcpy(extension, c + 1);
			*c-- = 0;
		} else {
			*extension = 0;
		}
	} else if (*c == '.') {
		*c-- = 0;
	}

	// step back to next directory break
	while (*--c != dir_break);
	
	// copy name if allocated
	if (name != NULL)
		strncpy_m11(name, c + 1, BASE_FILE_NAME_BYTES_m11);
	*c = 0;
	
	// copy path if allocated
	if (path != NULL)
		strncpy_m11(path, temp_full_file_name, FULL_FILE_NAME_BYTES_m11);
	
	return;
}
		
		
void	extract_terminal_password_bytes_m11(si1 *password, si1 *password_bytes)
{
	si1	*s;     // terminal (most unique) bytes of UTF-8 password
	si4     i, j;
	ui4     ch;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	s = password;
	for (i = j = 0; (ch = UTF8_next_char_m11(s, &i)); ++j)  // "i" modified in UTF8_next_char_m11()
		password_bytes[j] = (ui1) (ch & 0x000000FF);
	
	for (; j < PASSWORD_BYTES_m11; ++j)
		password_bytes[j] = 0;
	
	return;
}
		
		
ui4     file_exists_m11(si1 *path)  // can be used for directories also
{
	si1			tmp_path[FULL_FILE_NAME_BYTES_m11];
	si4             	err;
#if defined MACOS_m11 || defined LINUX_m11
	struct stat     	sb;
#endif
#ifdef WINDOWS_m11
	struct _stat64		sb;
#endif
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (path == NULL)
		return(DOES_NOT_EXIST_m11);
	
	if (*path == 0)
		return(DOES_NOT_EXIST_m11);
	
	tmp_path[0] = 0;
	path_from_root_m11(path, tmp_path);
	
	errno = 0;
#if defined MACOS_m11 || defined LINUX_m11
	err = stat(tmp_path, &sb);
	if (err == -1) {
		if (errno == ENOENT)
			return(DOES_NOT_EXIST_m11);
		return(FILE_EXISTS_ERROR_m11);
	} else if (S_ISDIR(sb.st_mode)) {
		return(DIR_EXISTS_m11);
	}
#endif
#ifdef WINDOWS_m11
	err = _stat64(tmp_path, &sb);
	if (err == -1) {
		if (errno == ENOENT)
			return(DOES_NOT_EXIST_m11);
		return(FILE_EXISTS_ERROR_m11);
	} else if ((sb.st_mode & S_IFMT) == S_IFDIR) {
		return(DIR_EXISTS_m11);
	}
#endif
	
	return(FILE_EXISTS_m11);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si8	file_length_m11(FILE *fp, si1 *path)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// pass either FILE pointer, or path to file
	
	if (fp == NULL && path == NULL)
		return(-1);

#if defined MACOS_m11 || defined LINUX_m11
	si4		fd;
	struct stat	sb;
	
	if (fp == NULL) {
		stat(path, &sb);
	} else {
		fd = fileno(fp);
		fstat(fd, &sb);
	}
#endif

#ifdef WINDOWS_m11
	si4		fd;
	struct _stat64	sb;
	
	if (fp == NULL) {
		_stat64(path, &sb);
	} else {
		fd = _fileno(fp);
		_fstat64(fd, &sb);
	}
#endif
	
	return((si8) sb.st_size);
}
		
		
FILE_TIMES_m11	*file_times_m11(FILE *fp, si1 *path, FILE_TIMES_m11 *ft, TERN_m11 set_time)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// pass either FILE pointer, or path to file
	if (fp == NULL && path == NULL)
		return(NULL);

	// caller must free
	if (ft == NULL)
		ft = (FILE_TIMES_m11 *) malloc_m11(sizeof(FILE_TIMES_m11), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);

#if defined MACOS_m11 || defined LINUX_m11
	si4		fd;
	struct stat	sb;
	struct timeval 	set_times[2] = {0};
	
	
	// set times: access and modification only
	if (set_time == TRUE_m11) {
		// set access & modification times to current time
		if (ft == NULL) {
			gettimeofday(set_times, NULL);
			set_times[1] = set_times[0];
		} else {  // use passed times (if non-zero)
			if (ft->access) {
				set_times[0].tv_sec = ft->access / (si8) 1000000;
				set_times[0].tv_usec = ft->access % (si8) 1000000;
			}
			if (ft->modification) {
				set_times[1].tv_sec = ft->modification / (si8) 1000000;
				set_times[1].tv_usec = ft->modification % (si8) 1000000;
			}
		}
	}
	
	if (fp == NULL) {
		stat(path, &sb);
	} else {
		fd = fileno(fp);
		fstat(fd, &sb);
	}

	#ifdef MACOS_m11
		#ifdef _DARWIN_FEATURE_64_BIT_INODE
			ft->creation = ((si8) sb.st_birthtimespec.tv_sec * (si8) 1000000) + ((si8) sb.st_birthtimespec.tv_nsec / (si8) 1000);
		#else
			ft->creation = ((si8) sb.st_ctimespec.tv_sec * (si8) 1000000) + ((si8) sb.st_ctim.tv_nsec / (si8) 1000);  // time of last status change - may be creation time - not guaranteed
		#endif
		ft->access = ((si8) sb.st_atimespec.tv_sec * (si8) 1000000) + ((si8) sb.st_atimespec.tv_nsec / (si8) 1000);
		ft->modification = ((si8) sb.st_mtimespec.tv_sec * (si8) 1000000) + ((si8) sb.st_mtimespec.tv_nsec / (si8) 1000);
	#endif
	#ifdef LINUX_m11
		ft->creation = ((si8) sb.st_ctim.tv_sec * (si8) 1000000) + ((si8) sb.st_ctim.tv_nsec / (si8) 1000);  // time of last status change - may be creation time - not guaranteed
		ft->access = ((si8) sb.st_atim.tv_sec * (si8) 1000000) + ((si8) sb.st_atim.tv_nsec / (si8) 1000);
		ft->modification = ((si8) sb.st_mtim.tv_sec * (si8) 1000000) + ((si8) sb.st_mtim.tv_nsec / (si8) 1000);
	#endif

	// set times: access and modification only
	if (set_time == TRUE_m11) {
		if (set_times[0].tv_sec == 0) {
			set_times[0].tv_sec = ft->access / (si8) 1000000;
			set_times[0].tv_usec = ft->access % (si8) 1000000;
		}
		if (set_times[1].tv_sec == 0) {
			set_times[1].tv_sec = ft->modification / (si8) 1000000;
			set_times[1].tv_usec = ft->modification % (si8) 1000000;
		}
		utimes(path, set_times);
	}
	
	return(ft);
#endif  // MACOS_m11 || LINUX_m11

#ifdef WINDOWS_m11
	si4		fd;
	HANDLE		file_h;
	FILETIME	win_create_time, win_access_time, win_modify_time;
	FILETIME	set_access_time, set_modify_time;
	SYSTEMTIME 	sys_time;
	
	
	if (set_time == TRUE_m11) {
		if (ft == NULL) {
			GetSystemTime(&sys_time);
			SystemTimeToFileTime(&sys_time, &set_access_time);
			set_modify_time = set_access_time;
		}
		set_access_time = uutc_to_win_time_m11(ft->access);
		set_modify_time = uutc_to_win_time_m11(ft->modification);
	}

	if (fp == NULL) {
		if ((file_h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE) {
		    warning_message_m11("%s(): CreateFile failed with error %d\n", __FUNCTION__, GetLastError());
		    return(NULL);
		}
	} else {
		fd = _fileno(fp);
		if ((file_h = (HANDLE) _get_osfhandle(fd)) == INVALID_HANDLE_VALUE) {
		    warning_message_m11("%s(): get_osfhandle failed with error %d\n", __FUNCTION__, GetLastError());
		    return(NULL);
		}
	}

	if (!GetFileTime(file_h, &win_create_time, &win_access_time, &win_modify_time)) {
		warning_message_m11("%s(): GetFileTime failed with error %d\n", __FUNCTION__, GetLastError());
		return(NULL);
	}
	
	if (ft == NULL)
		ft = (FILE_TIMES_m11 *) malloc_m11(sizeof(FILE_TIMES_m11), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);

	ft->creation = win_time_to_uutc_m11(win_create_time);
	ft->access = win_time_to_uutc_m11(win_access_time);
	ft->modification = win_time_to_uutc_m11(win_modify_time);
	
	if (set_time == TRUE_m11) {
		if (!SetFileTime(file_h, NULL, &set_access_time, &set_modify_time))
			warning_message_m11("%s(): SetFileTime failed with error %d\n", __FUNCTION__, GetLastError());
	}

	if (fp == NULL)
		CloseHandle(file_h);
	
	return(ft);
#endif
}


CONTIGUON_m11	*find_discontinuities_m11(LEVEL_HEADER_m11 *level_header, si8 *num_contigua)
{
	si1				seg_num_str[FILE_NUMBERING_DIGITS_m11 + 1], temp_str[FULL_FILE_NAME_BYTES_m11];
	si4				i, start_seg_num, end_seg_num, n_segs, last_seg_num;
	si8				j, k, n_contigua, n_indices, *sample_offsets;
	sf8				samp_period, sf8_samps;
	FILE_PROCESSING_STRUCT_m11	**tsi_fps, *md_fps;
	TIME_SERIES_INDEX_m11		*tsi, *last_tsi;
	SEGMENT_m11			*seg;
	CHANNEL_m11			*chan;
	SESSION_m11			*sess;
	CONTIGUON_m11			*contigua;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// The difference between this function & build_contigua_m11(), is that this does not use time slice as delimiter for returned contigua.
	// Caller is responsible for freeing contigua.
	
	switch (level_header->type_code) {
		case LH_TIME_SERIES_SEGMENT_m11:
			seg = (SEGMENT_m11 *) level_header;
			chan = NULL;
			start_seg_num = end_seg_num = seg->metadata_fps->universal_header->segment_number;
			break;
		case LH_TIME_SERIES_CHANNEL_m11:
		case LH_SESSION_m11:
			if (level_header->type_code == LH_TIME_SERIES_CHANNEL_m11) {
				chan = (CHANNEL_m11 *) level_header;
			} else {
				chan = globals_m11->reference_channel;
				if (chan->type_code != LH_TIME_SERIES_CHANNEL_m11) {
					sess = (SESSION_m11 *) level_header;
					chan = sess->time_series_channels[0];
				}
			}
			start_seg_num = 1;
			end_seg_num = globals_m11->number_of_session_segments;
			break;
		case LH_VIDEO_CHANNEL_m11:
		case LH_VIDEO_SEGMENT_m11:
			warning_message_m11("%s(): not coded for video channels yet\n", __FUNCTION__);
			return(NULL);
		default:
			warning_message_m11("%s(): invalid level type\n", __FUNCTION__);
			return(NULL);
	}

	n_segs = (end_seg_num - start_seg_num) + 1;
	tsi_fps = (FILE_PROCESSING_STRUCT_m11 **) malloc(n_segs * sizeof(FILE_PROCESSING_STRUCT_m11 *));
	sample_offsets = (si8 *) malloc(n_segs * sizeof(si8));
	
	// get time series indices & sample offsets
	for (i = start_seg_num, j = 0; i <= end_seg_num; ++i, ++j) {
		if (chan != NULL) {
			numerical_fixed_width_string_m11(seg_num_str, FILE_NUMBERING_DIGITS_m11, i);
			sprintf_m11(temp_str, "%s/%s_s%s.tisd/%s_s%s.tidx", chan->path, chan->name, seg_num_str, chan->name, seg_num_str);
			tsi_fps[j] = read_file_m11(NULL, temp_str, 0, 0, FPS_FULL_FILE_m11, NULL, NULL, USE_GLOBAL_BEHAVIOR_m11);
			sprintf_m11(temp_str, "%s/%s_s%s.tisd/%s_s%s.tmet", chan->path, chan->name, seg_num_str, chan->name, seg_num_str);
			md_fps = read_file_m11(NULL, temp_str, 0, 0, FPS_FULL_FILE_m11, NULL, NULL, USE_GLOBAL_BEHAVIOR_m11);
			sample_offsets[j] = md_fps->metadata->time_series_section_2.absolute_start_sample_number;
			samp_period = (sf8)1e6 / md_fps->metadata->time_series_section_2.sampling_frequency;
			FPS_free_processing_struct_m11(md_fps, TRUE_m11);
		} else {
			tsi_fps[j] = seg->time_series_indices_fps;
			sample_offsets[j] = seg->metadata_fps->metadata->time_series_section_2.absolute_start_sample_number;
		}
	}
	
	// count contigua
	n_contigua = 0;
	for (i = 0; i < n_segs; ++i) {
		tsi = tsi_fps[i]->time_series_indices;
		n_indices = tsi_fps[i]->universal_header->number_of_entries - 1;  // exclude terminal index
		for (j = 0; j < n_indices; ++j)
			if (tsi[j].file_offset < 0)
				++n_contigua;
	}
	contigua = (CONTIGUON_m11 *) calloc_m11((size_t) n_contigua, sizeof(CONTIGUON_m11), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	
	// start first contiguon
	tsi = tsi_fps[0]->time_series_indices;
	contigua[0].start_time = tsi[0].start_time;
	contigua[0].start_sample_number = 0;
	contigua[0].start_segment_number = start_seg_num;
	n_contigua = 0;
	last_tsi = tsi;
	last_seg_num = 1;

	// fill in contigua
	for (i = start_seg_num, j = 0; i <= end_seg_num; ++i, ++j) {
		tsi = tsi_fps[j]->time_series_indices;
		n_indices = tsi_fps[j]->universal_header->number_of_entries - 1;  // exclude terminal index
		for (k = 0; k < n_indices; ++k) {
			// make sample numbers global
			tsi[k].start_sample_number += sample_offsets[j];
			if (tsi[k].file_offset < 0) {
				// skip first block encountered, as it is a duplicate of contigua[0]'s start info
				if (k == 0 && j == 0) {
					last_tsi = tsi + k;
					last_seg_num = i;
					continue;
				}
				// finish last contiguon
				sf8_samps = (sf8) (tsi[k].start_sample_number - last_tsi->start_sample_number);
				contigua[n_contigua].end_time = last_tsi->start_time + (si8) ((sf8_samps * samp_period) + (sf8) 0.5);
				contigua[n_contigua].end_sample_number = tsi[k].start_sample_number - 1;
				contigua[n_contigua].end_segment_number = last_seg_num;
				// start new contiguon
				++n_contigua;
				contigua[n_contigua].start_time = tsi[k].start_time;
				contigua[n_contigua].start_sample_number = tsi[k].start_sample_number;
				contigua[n_contigua].start_segment_number = i;
				// n_contigua = 0;
			}
		    last_tsi = tsi + k;
		    last_seg_num = i;
		}
	}
	// finish last contiguon (using terminal index values)
	contigua[n_contigua].end_time = tsi[k].start_time;
	contigua[n_contigua].end_sample_number = tsi[k].start_sample_number + sample_offsets[j - 1] - 1;
	contigua[n_contigua].end_segment_number = last_seg_num;
	
	// clean up
	if (level_header->type_code == LH_TIME_SERIES_SEGMENT_m11)
		for (i = 0; i < n_segs; ++i)
			FPS_free_processing_struct_m11(tsi_fps[i], TRUE_m11);
	free((void *) tsi_fps);
	free((void *) sample_offsets);
	
	*num_contigua = n_contigua + 1;
	
	return(contigua);
}
	
		
si8	find_index_m11(SEGMENT_m11 *seg, si8 target, ui4 mode)  // returns index containing requested time/sample
{
	TERN_m11		no_overflows = FALSE_m11;
	ui4			target_frame_number;
	si8			i, n_inds, seg_start_time, seg_end_time;
	si8			block_samples, block_duration, seg_samples, clip_frames, clip_duration, seg_frames;
	TIME_SERIES_INDEX_m11	*tsi;
	VIDEO_INDEX_m11		*vi;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// returns -1 if before first index
	// returns n_inds - 1 (terminal index index) if after last index
	// NO_OVERFLOWS_m11: restrict returned index to valid segment values (ORed with search type)
	// SAMPLE_SEARCH_m11 indices must be session-relative (global indexing)
	
	if (mode & NO_OVERFLOWS_m11) {
		mode &= ~NO_OVERFLOWS_m11;
		no_overflows = TRUE_m11;
	}

	if (seg->type_code == LH_TIME_SERIES_SEGMENT_m11) {
		tsi = seg->time_series_indices_fps->time_series_indices;
		n_inds = seg->time_series_indices_fps->universal_header->number_of_entries - 1;  // account for terminal index here - cleaner code below
		if (mode == TIME_SEARCH_m11) {
			seg_start_time = seg->time_series_indices_fps->universal_header->segment_start_time;
			if (target < seg_start_time) {
				if (no_overflows == TRUE_m11)
					return(0);
				return(-1);
			}
			seg_end_time = seg->time_series_indices_fps->universal_header->segment_end_time;
			if (target > seg_end_time) {
				if (no_overflows == TRUE_m11)
					return(n_inds - 1);
				return(n_inds);
			}
			block_duration = (si8) (seg->metadata_fps->metadata->time_series_section_2.maximum_block_duration + (sf8) 0.5);
			i = ((target - seg_start_time) / block_duration) + 1;  // more efficient to serch backward
			if (i > n_inds)
				i = n_inds;
			if (tsi[i].start_time <= target) {  // forward linear search
				while (tsi[++i].start_time <= target);
				--i;
			} else {  // backward linear search
				while (tsi[--i].start_time > target);
			}
		} else {  //  SAMPLE_SEARCH_m11
			target -= seg->metadata_fps->metadata->time_series_section_2.absolute_start_sample_number;  // convert target to local indexing
			if (target < 0) {
				if (no_overflows == TRUE_m11)
					return(0);
				return(-1);
			}
			seg_samples = seg->metadata_fps->metadata->time_series_section_2.number_of_samples;
			if (target >= seg_samples) {
				if (no_overflows == TRUE_m11)
					return(n_inds - 1);
				return(n_inds);
			}
			block_samples = (si8) seg->metadata_fps->metadata->time_series_section_2.maximum_block_samples;
			i = (target / block_samples) + 1;  // more efficient to search backward
			if (i > n_inds)
				i = n_inds;
			if (tsi[i].start_sample_number <= target) {  // forward linear search
				while (tsi[++i].start_sample_number <= target);
				--i;
			} else {  // backward linear search
				while (tsi[--i].start_sample_number > target);
			}
		}
	} else {  // LEVEL_VIDEO_SEGMENT_m11
		vi = seg->video_indices_fps->video_indices;
		n_inds = seg->video_indices_fps->universal_header->number_of_entries - 1;  // account for terminal index here - cleaner code below
		if (mode == TIME_SEARCH_m11) {
			seg_start_time = seg->video_indices_fps->universal_header->segment_start_time;
			if (target < seg_start_time) {
				if (no_overflows == TRUE_m11)
					return(0);
				return(-1);
			}
			seg_end_time = seg->video_indices_fps->universal_header->segment_end_time;
			if (target > seg_end_time) {
				if (no_overflows == TRUE_m11)
					return(n_inds - 1);
				return(n_inds);
			}

			clip_duration = (si8) (seg->metadata_fps->metadata->video_section_2.maximum_clip_duration + (sf8) 0.5);
			i = ((target - seg_start_time) / clip_duration) + 1;  // more efficient to serch backward
			if (i > n_inds)
				i = n_inds;
			if (vi[i].start_time <= target) {  // forward linear search
				while (vi[++i].start_time <= target);
				--i;
			} else {  // backward linear search
				while (vi[--i].start_time > target);
			}
		} else {  //  SAMPLE_SEARCH_m11  (target frame numbers must be in absolute frame)
			target -= seg->metadata_fps->metadata->video_section_2.absolute_start_frame_number;  // convert target to local indexing
			if (target < 0) {
				if (no_overflows == TRUE_m11)
					return(0);
				return(-1);
			}
			seg_frames = seg->metadata_fps->metadata->video_section_2.number_of_frames;
			if (target >= seg_frames) {
				if (no_overflows == TRUE_m11)
					return(n_inds - 1);
				return(n_inds);
			}
			clip_frames = (si8) seg->metadata_fps->metadata->video_section_2.maximum_clip_frames;
			i = (target / clip_frames) + 1;  // more efficient to serch backward
			if (i > n_inds)
				i = n_inds;
			target_frame_number = (ui4) target;
			if (vi[i].start_frame_number <= target_frame_number) {  // forward linear search
				while (vi[++i].start_frame_number <= target_frame_number);
				--i;
			} else {  // backward linear search
				while (vi[--i].start_frame_number > target_frame_number);
			}
		}
	}
	
	if (no_overflows == TRUE_m11) {
		if (i == -1)
			return(0);
		else if (i == n_inds)
			return(n_inds - 1);
	}
	
	return(i);
}


#if defined MACOS_m11 || defined LINUX_m11
si1	*find_metadata_file_m11(si1 *path, si1 *md_path)
{
	TERN_m11	match;
	si1		*c, *name;
	ui4		code;
	size_t		len;
	DIR		*dir;
	struct dirent	*entry;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// large directory trees can take a long time to search with "find" or "ls"
	// cumbersome code => function unrolled for speed

	// caller responsible for freeing, if allocated
	if (md_path == NULL)
		md_path = (si1 *) malloc_m11((size_t) FULL_FILE_NAME_BYTES_m11, __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	
	// find entry level
	path_from_root_m11(path, md_path);
	code = MED_type_code_from_string_m11(md_path);
	switch(code) {
		case SESSION_DIRECTORY_TYPE_CODE_m11:
			break;
		case TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m11:
		case VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m11:
			goto FIND_MDF_CHAN_LEVEL_m11;
		case TIME_SERIES_SEGMENT_DIRECTORY_TYPE_CODE_m11:
		case VIDEO_SEGMENT_DIRECTORY_TYPE_CODE_m11:
			goto FIND_MDF_SEG_LEVEL_m11;
		default:
			warning_message_m11("%s(): input path must be a MED session, channel, or segment directory\n", __FUNCTION__);
			set_error_m11(E_NOT_MED_m11, __FUNCTION__, __LINE__);
			return(NULL);
	}
	
	// session level
	dir = opendir(md_path);
	if (dir == NULL)
		return(NULL);
	match = FALSE_m11;
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type != DT_DIR)
			continue;
		name = entry->d_name;
		if (*name == '.')
			continue;
		len = strlen(name);
		if (len < 6)  // min 1 letter channel name + 5 chars for extension
			continue;
		c = name + len - 5;
		if (*c++ != '.')
			continue;
		// check for MED channel ([tv]icd extension)
		if (*c == 't' || *c == 'v') {
			if (*++c == 'i') {
				if (*++c == 'c') {
					if (*++c == 'd') {
						match = TRUE_m11;
						break;
					}
				}
			}
		}
	}
	
	if (match == FALSE_m11) {
		set_error_m11(E_NO_METADATA_m11, __FUNCTION__, __LINE__);
		return(NULL);
	}
	
	len = strlen(md_path);
	md_path[len++] = '/';
	strcpy(md_path + len, name);
	closedir(dir);
	
FIND_MDF_CHAN_LEVEL_m11:
	dir = opendir(md_path);
	if (dir == NULL)
		return(NULL);
	match = FALSE_m11;
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type != DT_DIR)
			continue;
		name = entry->d_name;
		if (*name == '.')
			continue;
		len = strlen(name);
		if (len < 12)  // min 1 letter channel name + 6 chars for segment designation + 5 chars for extension
			continue;
		c = name + len - 5;
		if (*c++ != '.')
			continue;
		// check for MED segment ([tv]isd extension)
		if (*c == 't' || *c == 'v') {
			if (*++c == 'i') {
				if (*++c == 's') {
					if (*++c == 'd') {
						match = TRUE_m11;
						break;
					}
				}
			}
		}
	}
	
	if (match == FALSE_m11) {
		set_error_m11(E_NO_METADATA_m11, __FUNCTION__, __LINE__);
		return(NULL);
	}
	
	len = strlen(md_path);
	md_path[len++] = '/';
	strcpy(md_path + len, name);
	closedir(dir);

FIND_MDF_SEG_LEVEL_m11:
	dir = opendir(md_path); // open the path
	if (dir == NULL) {
		set_error_m11(E_NO_METADATA_m11, __FUNCTION__, __LINE__);
		return(NULL); // if was not able, return
	}
	match = FALSE_m11;
	while ((entry = readdir(dir)) != NULL) {  // if we were able to read something from the directory
		if (entry->d_type != DT_REG && entry->d_type != DT_LNK)
			continue;
		name = entry->d_name;
		if (*name == '.')
			continue;
		len = strlen(name);
		if (len < 12)  // min 1 letter channel name + 6 chars for segment desiognation + 5 chars for extension
			continue;
		c = name + len - 5;
		if (*c++ != '.')
			continue;
		// check for MED metadata ([tv]met extension)
		if (*c == 't' || *c == 'v') {
			if (*++c == 'm') {
				if (*++c == 'e') {
					if (*++c == 't') {
						match = TRUE_m11;
						break;
					}
				}
			}
		}
	}
	
	if (match == FALSE_m11) {
		set_error_m11(E_NO_METADATA_m11, __FUNCTION__, __LINE__);
		return(NULL);
	}
	
	len = strlen(md_path);
	md_path[len++] = '/';
	strcpy(md_path + len, name);
	closedir(dir);

	return(md_path);
}
#endif  // MACOS_m11 || LINUX_m11


#ifdef WINDOWS_m11
si1	*find_metadata_file_m11(si1 *path, si1 *md_path)
{
	si1			*name, *c;
	ui4			code;
	size_t			len;
	WIN32_FIND_DATAA 	ffd;
	HANDLE 		        find_h;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// large directory trees can take a long time to search with "find" or "ls"
	// cumbersome code => function unrolled for speed

	// caller responsible for freeing, if allocated
	if (md_path == NULL)
		md_path = (si1 *) malloc_m11((size_t) FULL_FILE_NAME_BYTES_m11, __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	
	// find entry level
	path_from_root_m11(path, md_path);
	code = MED_type_code_from_string_m11(path);
	switch(code) {
		case SESSION_DIRECTORY_TYPE_CODE_m11:
			break;
		case TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m11:
		case VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m11:
			goto WIN_FIND_MDF_CHAN_LEVEL_m11;
		case TIME_SERIES_SEGMENT_DIRECTORY_TYPE_CODE_m11:
		case VIDEO_SEGMENT_DIRECTORY_TYPE_CODE_m11:
			goto WIN_FIND_MDF_SEG_LEVEL_m11;
		default:
			warning_message_m11("%s(): input path must be a MED session, channel, or segment directory\n", __FUNCTION__);
			return(NULL);
	}
	
	// session level
	len = strlen(md_path);
	strcpy(md_path + len, "\\*.?icd");
	find_h = FindFirstFileA((LPCSTR) md_path, &ffd);
	if (find_h == INVALID_HANDLE_VALUE)
		return(NULL);
	name = ffd.cFileName;
	while (*name == '.') {
		if (FindNextFileA(find_h, &ffd) == 0)
			return(NULL);
		name = ffd.cFileName;
	}
	strcpy(md_path + len + 1, name);
	FindClose(find_h);
	
	// channel level
WIN_FIND_MDF_CHAN_LEVEL_m11:
	len = strlen(md_path);
	strcpy(md_path + len, "\\*.?isd");
	find_h = FindFirstFileA((LPCSTR) md_path, &ffd);
	if (find_h == INVALID_HANDLE_VALUE)
		return(NULL);
	name = ffd.cFileName;
	while (*name == '.') {
		if (FindNextFileA(find_h, &ffd) == 0)
			return(NULL);
		name = ffd.cFileName;
	}
	strcpy(md_path + len + 1, name);
	FindClose(find_h);

	// segment level
WIN_FIND_MDF_SEG_LEVEL_m11:
	len = strlen(md_path);
	strcpy(md_path + len, "\\*.?met");
	find_h = FindFirstFileA((LPCSTR) md_path, &ffd);
	if (find_h == INVALID_HANDLE_VALUE)
		return(NULL);
	name = ffd.cFileName;
	while (*name == '.') {
		if (FindNextFileA(find_h, &ffd) == 0)
			return(NULL);
		name = ffd.cFileName;
	}
	strcpy(md_path + len + 1, name);
	FindClose(find_h);

	return(md_path);
}
#endif  // WINDOWS_m11


si8	find_record_index_m11(FILE_PROCESSING_STRUCT_m11 *record_indices_fps, si8 target_time, ui4 mode, si8 low_idx)
{
	si8			i, idx, n_inds, high_idx, high_time_diff, low_time_diff;
	RECORD_INDEX_m11	*ri;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// Modes (note there can be multiple records with the same start time):
	//	FIND_CLOSEST_m11: If target_time == start_time, and there are multiple indices with the
	//			  same start time, the function will return the first of these indices.
	//			  If they are equally close, the lower index will be returned.
	//	FIND_LAST_BEFORE_m11
	//	FIND_FIRST_ON_OR_AFTER_m11
	//	FIND_LAST_ON_OR_BEFORE_m11
	//	FIND_FIRST_AFTER_m11
	//
	//	low_idx: as indices are often needed in pairs, pass low_idx to reduce search time if known (if not, pass zero)


	ri = record_indices_fps->record_indices;
	n_inds = record_indices_fps->universal_header->number_of_entries;
	if (n_inds == 1)  // only a terminal index, no records
		return(NO_INDEX_m11);

	if (target_time < ri[low_idx].start_time) {
		switch (mode) {
			case FIND_FIRST_AFTER_m11:
			case FIND_CLOSEST_m11:
			case FIND_FIRST_ON_OR_AFTER_m11:
				return(low_idx);
			// "on" or "before" condition impossible
			case FIND_LAST_ON_OR_BEFORE_m11:
			case FIND_LAST_BEFORE_m11:
				return(NO_INDEX_m11);
			default:
				warning_message_m11("%s(): unsupported mode (%u)\n", __FUNCTION__, mode);
				return(NO_INDEX_m11);
		}
	}
	high_idx = n_inds - 1; // terminal index index
	if (target_time >= ri[high_idx].start_time) {  // terminal start_time == next segment start time
		switch (mode) {
			case FIND_CLOSEST_m11:
			case FIND_LAST_BEFORE_m11:
			case FIND_LAST_ON_OR_BEFORE_m11:
				return(high_idx - 1);  // last true index
			// "after" condition impossible
			case FIND_FIRST_ON_OR_AFTER_m11:
			case FIND_FIRST_AFTER_m11:
				return(NO_INDEX_m11);
		}
	}
	if (low_idx == high_idx)
		return(low_idx);

	// binary search
	do {
		idx = (low_idx + high_idx) >> 1;
		if (ri[idx].start_time > target_time)
			high_idx = idx;
		else
			low_idx = idx;
	} while ((high_idx - low_idx) > 1);
	if (target_time >= ri[high_idx].start_time)
	    idx = high_idx;
	else if (target_time < ri[high_idx].start_time)
	    idx = low_idx;
	// search exits with idx == FIND_LAST_ON_OR_BEFORE_m11 condition
	// i.e. where:  ri[idx].start_time <= target_time < ri[idx + 1].start_time
	// i.e. the last index where target_time >= index start_time

	switch (mode) {
		case FIND_CLOSEST_m11:
			low_time_diff = target_time - ri[idx].start_time;
			high_time_diff = ri[idx + 1].start_time - target_time;
			if (high_time_diff < low_time_diff)
				++idx;  // advance to first record of next time
			else
				for (i = idx - 1; ri[i].start_time == ri[idx].start_time; idx = i--);  // rewind to first record of this time
			break;
		case FIND_LAST_BEFORE_m11:
			if (target_time == ri[idx].start_time) {
				for (i = idx - 1; ri[i].start_time == ri[idx].start_time; idx = i--);  // rewind to first record of this time
				--idx;  // step back to last record of last time
			}
			// there may be other records with the same time preceding this one
			break;
		case FIND_FIRST_ON_OR_AFTER_m11:
			if (target_time == ri[idx].start_time)  // "on" condition
				for (i = idx - 1; ri[i].start_time == ri[idx].start_time; idx = i--);  // rewind to first record of this time
			else
				++idx;  // advance to first record of next time
		case FIND_LAST_ON_OR_BEFORE_m11:
			break;  // no rewind: there may be other records with the same time preceding this one
		case FIND_FIRST_AFTER_m11:
			++idx;  // advance to first record of next time
			break;
	}
	
	return(idx);
}


si8     frame_number_for_uutc_m11(LEVEL_HEADER_m11 *level_header, si8 target_uutc, ui4 mode, ...)  // varargs: si8 ref_frame_number, si8 ref_uutc, sf8 frame_rate
{
	si1			tmp_str[FULL_FILE_NAME_BYTES_m11], num_str[FILE_NUMBERING_DIGITS_m11 + 1];
	si4			seg_num, seg_idx;
	si8                     frame_number, n_inds, i, absolute_numbering_offset;
	si8			ref_frame_number, ref_uutc;
	sf8                     tmp_sf8, frame_rate, rounded_frame_num, frame_num_eps;
	ui4			mask;
	va_list			args;
	SEGMENT_m11		*seg;
	CHANNEL_m11		*chan;
	SESSION_m11		*sess;
	VIDEO_INDEX_m11		*vi;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// frame_number_for_uutc_m11(NULL, si8 target_uutc, ui4 mode, si8 ref_index, si8 ref_uutc, sf8 frame_rate);
	// returns frame number extrapolated from ref_frame_number (relative / absolute is determined by magnitude of reference values)
	
	// frame_number_for_uutc_m11(seg, target_uutc, mode);
	// returns frame number extrapolated from closest video index in reference frame specified by mode
	
	// mode FIND_ABSOLUTE_m11 (default): session relative frame numbering
	// mode FIND_RELATIVE_m11: segment relative frame numbering
	// mode FIND_CURRENT_m11 (default): frame period within which the target_uutc falls
	// mode FIND_CLOSEST_m11: frame number closest to the target_uutc
	// mode FIND_NEXT_m11: frame number following the frame period within which the target_uutc falls ( == FIND_CURRENT_m11 + 1)
	// mode FIND_PREVIOUS_m11: frame number preceding the frame period within which the target_uutc falls ( == FIND_CURRENT_m11 - 1)

	if (level_header == NULL) {  // reference points passed
		va_start(args, mode);
		ref_frame_number = va_arg(args, si8);
		ref_uutc = va_arg(args, si8);
		frame_rate = va_arg(args, sf8);
		va_end(args);
		absolute_numbering_offset = 0;
		vi = NULL;
	} else {  // level header passed
		switch (level_header->type_code) {
			case LH_VIDEO_SEGMENT_m11:
				seg = (SEGMENT_m11 *) level_header;
				break;
			case LH_VIDEO_CHANNEL_m11:
			case LH_SESSION_m11:
				seg_num = segment_for_uutc_m11(level_header, target_uutc);
				seg_idx = get_segment_index_m11(seg_num);
				if (seg_idx == FALSE_m11)
					return(FRAME_NUMBER_NO_ENTRY_m11);
				if (level_header->type_code == LH_VIDEO_CHANNEL_m11) {
					chan = (CHANNEL_m11 *) level_header;
				} else {
					chan = globals_m11->reference_channel;
					if (chan->type_code != LH_VIDEO_CHANNEL_m11) {
						sess = (SESSION_m11 *) level_header;
						chan = sess->video_channels[0];
					}
				}
				seg = chan->segments[seg_idx];
				break;
			case LH_TIME_SERIES_CHANNEL_m11:
			case LH_TIME_SERIES_SEGMENT_m11:
				return(sample_number_for_uutc_m11(level_header, target_uutc, mode));
			default:
				error_message_m11("%s(): invalid level type\n", __FUNCTION__);
				return(FRAME_NUMBER_NO_ENTRY_m11);
		}
		if (seg == NULL) {  // channel or session
			numerical_fixed_width_string_m11(num_str, FILE_NUMBERING_DIGITS_m11, seg_num);
			sprintf_m11(tmp_str, "%s/%s_s%s.%s", chan->path, chan->name, num_str, VIDEO_SEGMENT_DIRECTORY_TYPE_STRING_m11);
			seg = chan->segments[seg_idx] = open_segment_m11(NULL, NULL, tmp_str, chan->flags, NULL);
		} else if (!(seg->flags & LH_OPEN_m11)) {  // closed segment
			open_segment_m11(seg, NULL, NULL, seg->flags, NULL);
		}
		if (seg == NULL) {
			warning_message_m11("%s(): can't open segment\n", __FUNCTION__);
			return(FRAME_NUMBER_NO_ENTRY_m11);
		}

		vi = seg->video_indices_fps->video_indices;
		if (vi == NULL) {
			warning_message_m11("%s(): video indices are NULL => returning FRAME_NUMBER_NO_ENTRY_m11\n", __FUNCTION__);
			return(FRAME_NUMBER_NO_ENTRY_m11);
		}
		n_inds = seg->video_indices_fps->universal_header->number_of_entries - 1;  // account for terminal index here - cleaner code below
		if (mode & FIND_RELATIVE_m11)
			absolute_numbering_offset = 0;
		else  // FIND_ABSOLUTE_m11 (default)
			absolute_numbering_offset = seg->metadata_fps->metadata->video_section_2.absolute_start_frame_number;

		i = find_index_m11(seg, target_uutc, TIME_SEARCH_m11);
		if (i == -1)  // target time earlier than segment start => return segment start frame
			return(absolute_numbering_offset);
		
		vi += i;
		if (i == n_inds) {  // target time later than segment end => return segment end frame number
			i = (vi->start_frame_number - 1) + absolute_numbering_offset;
			return(i);
		}
		ref_uutc = vi->start_time;
		ref_frame_number = vi->start_frame_number;
		++vi;  // advance to next index
		if (vi->file_offset > 0) {  // get local frame rate, unless discontinuity
			frame_rate = (sf8) (vi->start_frame_number - ref_frame_number);
			frame_rate /= ((sf8) (vi->start_time - ref_uutc) / (sf8) 1e6);
		} else {
			frame_rate = seg->metadata_fps->metadata->video_section_2.frame_rate;
		}
	}
	
	// round up if very close to next frame
	tmp_sf8 = ((sf8) (target_uutc - ref_uutc) / (sf8) 1e6) * frame_rate;
	rounded_frame_num = (sf8) ((si8) (tmp_sf8 + (sf8) 0.5));
	frame_num_eps = rounded_frame_num - tmp_sf8;
	if (frame_num_eps > (sf8) 0.0)
		if (frame_num_eps < FRAME_NUMBER_EPS_m11)
			tmp_sf8 = rounded_frame_num;
	
	mask = (ui4) (FIND_CLOSEST_m11 | FIND_NEXT_m11 | FIND_CURRENT_m11 | FIND_PREVIOUS_m11);
	switch (mode & mask) {
		case FIND_CLOSEST_m11:
			tmp_sf8 += (sf8) 0.5;
			break;
		case FIND_NEXT_m11:
			tmp_sf8 += (sf8) 1.0;
			break;
		case FIND_PREVIOUS_m11:
			if ((tmp_sf8 -= (sf8) 1.0) < (sf8) 0.0)
				tmp_sf8 = (sf8) 0.0;
			break;
		case FIND_CURRENT_m11:
		default:
			break;
	}
	frame_number = ref_frame_number + (si8) tmp_sf8;
	if (vi != NULL) {
		if (frame_number >= vi->start_frame_number) {
			frame_number = vi->start_frame_number;
			if (mode & (FIND_CURRENT_m11 | FIND_PREVIOUS_m11))
				--frame_number;  // these should not go into next index
		}
	}
	frame_number += absolute_numbering_offset;
	
	return(frame_number);
}


void	free_channel_m11(CHANNEL_m11 *channel, TERN_m11 free_channel_structure)
{
	si4		i;
	SEGMENT_m11	*seg;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (channel == NULL) {
		warning_message_m11("%s(): trying to free a NULL CHANNEL_m11 structure => returning with no action\n", __FUNCTION__);
		return;
	}
	if (channel->segments != NULL) {
		for (i = 0; i < globals_m11->number_of_mapped_segments; ++i) {
			seg = channel->segments[i];
			if (seg != NULL)
				free_segment_m11(seg, TRUE_m11);
		}
		free_m11((void *) channel->segments, __FUNCTION__);
	}
	if (channel->metadata_fps != NULL)
		FPS_free_processing_struct_m11(channel->metadata_fps, TRUE_m11);
	if (channel->Sgmt_records != NULL)
		free_m11((void *) channel->Sgmt_records, __FUNCTION__);
	if (channel->record_data_fps != NULL)
		FPS_free_processing_struct_m11(channel->record_data_fps, TRUE_m11);
	if (channel->record_indices_fps != NULL)
		FPS_free_processing_struct_m11(channel->record_indices_fps, TRUE_m11);
	if (channel->contigua != NULL)
		free_m11(channel->contigua, __FUNCTION__);

	if (free_channel_structure == TRUE_m11) {
		free_m11((void *) channel, __FUNCTION__);
	} else {  // leave name, path, flags, & slice intact (i.e. clear everything with allocated memory)
		channel->flags &= ~(LH_OPEN_m11 | LH_CHANNEL_ACTIVE_m11);
		channel->last_access_time = UUTC_NO_ENTRY_m11;
		channel->metadata_fps = NULL;
		channel->record_data_fps = NULL;
		channel->record_indices_fps = NULL;
		channel->segments = NULL;
		channel->contigua = NULL;
		channel->number_of_contigua = 0;
	}

	return;
}


void    free_globals_m11(TERN_m11 cleanup_for_exit)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif

	if (globals_m11 == NULL)
		return;

	pthread_mutex_lock_m11(&globals_m11_mutex);
	
	if (cleanup_for_exit == TRUE_m11) {
		si1	command[FULL_FILE_NAME_BYTES_m11];

		#if defined MACOS_m11 || defined LINUX_m11
		sprintf_m11(command, "rm -f %s", globals_m11->temp_file);
		#endif
		#ifdef WINDOWS_m11
		win_cleanup_m11();
		sprintf_m11(command, "del %s", globals_m11->temp_file);
		#endif
		system_m11(command, TRUE_m11, __FUNCTION__, RETURN_ON_FAIL_m11 | SUPPRESS_OUTPUT_m11);
	}
	
	if (globals_m11->record_filters != NULL)
		free_m11((void *) globals_m11->record_filters, __FUNCTION__);
		// often statically allocated, so can't use regular free()
		// e.g. si4 rec_filts = { REC_Seiz_TYPE_CODE_m11, REC_Note_TYPE_CODE_m11, NO_TYPE_CODE_m11 };
		// globals_m11->record_filters = rec_filts;
		
	if (globals_m11->timezone_table != NULL)
		free_m11((void *) globals_m11->timezone_table, __FUNCTION__);
	
	if (globals_m11->country_aliases_table != NULL)
		free_m11((void *) globals_m11->country_aliases_table, __FUNCTION__);
	
	if (globals_m11->country_acronym_aliases_table != NULL)
		free_m11((void *) globals_m11->country_acronym_aliases_table, __FUNCTION__);
	
	if (globals_m11->CRC_table != NULL)
		free_m11((void *) globals_m11->CRC_table, __FUNCTION__);
	
	if (globals_m11->AES_sbox_table != NULL)
		free_m11((void *) globals_m11->AES_sbox_table, __FUNCTION__);
	
	if (globals_m11->AES_rsbox_table != NULL)
		free_m11((void *) globals_m11->AES_rsbox_table, __FUNCTION__);
	
	if (globals_m11->AES_rcon_table != NULL)
		free_m11((void *) globals_m11->AES_rcon_table, __FUNCTION__);
	
	if (globals_m11->SHA_h0_table != NULL)
		free_m11((void *) globals_m11->SHA_h0_table, __FUNCTION__);
	
	if (globals_m11->SHA_k_table != NULL)
		free_m11((void *) globals_m11->SHA_k_table, __FUNCTION__);
	
	if (globals_m11->behavior_stack != NULL)
		free_m11((void *) globals_m11->behavior_stack, __FUNCTION__);

#ifdef MATLAB_PERSISTENT_m11
	if (globals_m11->AT_nodes != NULL) {
		#ifdef AT_DEBUG_m11
		AT_free_all_m11();  // display memory still allocated & free it
		#endif
		mxFree((void *) globals_m11->AT_nodes);  // AT nodes are not allocted with AT functions
	}
	
	// UTF8 tables are not allocted with AT functions
	if (globals_m11->UTF8_offsets_table != NULL)
		mxFree((void *) globals_m11->UTF8_offsets_table);
	if (globals_m11->UTF8_trailing_bytes_table != NULL)
		mxFree((void *) globals_m11->UTF8_trailing_bytes_table);
	
	// destroy mutices
	pthread_mutex_destroy_m11(&globals_m11->TZ_mutex);
	pthread_mutex_destroy_m11(&globals_m11->CRC_mutex);
	pthread_mutex_destroy_m11(&globals_m11->AES_mutex);
	pthread_mutex_destroy_m11(&globals_m11->SHA_mutex);
	pthread_mutex_destroy_m11(&globals_m11->UTF8_mutex);
	pthread_mutex_destroy_m11(&globals_m11->behavior_mutex);
	pthread_mutex_destroy_m11(&globals_m11->AT_mutex);
	
	mxFree((void *) globals_m11);
#else
	if (globals_m11->AT_nodes != NULL) {
		#ifdef AT_DEBUG_m11
		AT_free_all_m11();  // display memory still allocated & free it
		#endif
		free((void *) globals_m11->AT_nodes);  // AT nodes are not allocted with AT functions
	}
	pthread_mutex_destroy_m11(&globals_m11->AT_mutex);

	// UTF8 tables are not allocted with AT functions
	if (globals_m11->UTF8_offsets_table != NULL)
		free((void *) globals_m11->UTF8_offsets_table);
	if (globals_m11->UTF8_trailing_bytes_table != NULL)
		free((void *) globals_m11->UTF8_trailing_bytes_table);
	
	// destroy mutices
	pthread_mutex_destroy_m11(&globals_m11->TZ_mutex);
	pthread_mutex_destroy_m11(&globals_m11->CRC_mutex);
	pthread_mutex_destroy_m11(&globals_m11->AES_mutex);
	pthread_mutex_destroy_m11(&globals_m11->SHA_mutex);
	pthread_mutex_destroy_m11(&globals_m11->UTF8_mutex);
	pthread_mutex_destroy_m11(&globals_m11->behavior_mutex);
	pthread_mutex_destroy_m11(&globals_m11->AT_mutex);
	
	free((void *) globals_m11);
#endif
	globals_m11 = NULL;

	pthread_mutex_unlock_m11(&globals_m11_mutex);
	pthread_mutex_destroy_m11(&globals_m11_mutex);

	return;
}


void	free_segment_m11(SEGMENT_m11 *segment, TERN_m11 free_segment_structure)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (segment == NULL) {
		warning_message_m11("$s(): trying to free a NULL SEGMENT_m11 structure => returning with no action\n", __FUNCTION__);
		return;
	}
	if (segment->metadata_fps != NULL)
		FPS_free_processing_struct_m11(segment->metadata_fps, TRUE_m11);
	if (segment->time_series_data_fps != NULL)  // also does video data fps (when it exists), due to union
		FPS_free_processing_struct_m11(segment->time_series_data_fps, TRUE_m11);
	if (segment->time_series_indices_fps != NULL)  // also does video indices, due to union
		FPS_free_processing_struct_m11(segment->time_series_indices_fps, TRUE_m11);
	if (segment->record_data_fps != NULL)
		FPS_free_processing_struct_m11(segment->record_data_fps, TRUE_m11);
	if (segment->record_indices_fps != NULL)
		FPS_free_processing_struct_m11(segment->record_indices_fps, TRUE_m11);
	if (segment->contigua != NULL)
		free_m11(segment->contigua, __FUNCTION__);

	if (free_segment_structure == TRUE_m11) {
		free_m11((void *) segment, __FUNCTION__);
	} else {
		// leave name, path, & slice intact (i.e. clear everything with allocated memory)
		segment->flags &= ~(LH_OPEN_m11 | LH_CHANNEL_ACTIVE_m11);
		segment->last_access_time = UUTC_NO_ENTRY_m11;
		segment->metadata_fps = NULL;
		segment->time_series_data_fps = NULL;
		segment->time_series_indices_fps = NULL;  // == video_indices_fps;
		segment->record_data_fps = NULL;
		segment->record_indices_fps = NULL;
		segment->contigua = NULL;
		segment->number_of_contigua = 0;
	}

	return;
}


void	free_segmented_ses_recs_m11(SEGMENTED_SESS_RECS_m11 *ssr, TERN_m11 free_segmented_ses_rec_structure)
{
	si4				i, n_segs;
	FILE_PROCESSING_STRUCT_m11	*gen_fps;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (ssr == NULL) {
		warning_message_m11("%s(): trying to free a NULL SEGMENTED_SESS_RECS_m11 structure => returning with no action\n", __FUNCTION__);
		return;
	}
	
	n_segs = globals_m11->number_of_mapped_segments;
	for (i = 0; i < n_segs; ++i) {
		gen_fps = ssr->record_indices_fps[i];
		if (gen_fps != NULL)
			FPS_free_processing_struct_m11(gen_fps, TRUE_m11);
		gen_fps = ssr->record_data_fps[i];
		if (gen_fps != NULL)
			FPS_free_processing_struct_m11(gen_fps, TRUE_m11);
	}
	free_m11((void *) ssr->record_indices_fps, __FUNCTION__);
	free_m11((void *) ssr->record_data_fps, __FUNCTION__);

	if (free_segmented_ses_rec_structure == TRUE_m11)
		free_m11((void *) ssr, __FUNCTION__);
	
	return;
}


void	free_session_m11(SESSION_m11 *session, TERN_m11 free_session_structure)
{
	si4		i;
	CHANNEL_m11	*chan;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (session == NULL) {
		warning_message_m11("%s(): trying to free a NULL SESSION_m11 structure => returning with no action\n", __FUNCTION__);
		return;
	}
	if (session->time_series_metadata_fps != NULL)
		FPS_free_processing_struct_m11(session->time_series_metadata_fps, TRUE_m11);
	if (session->video_metadata_fps != NULL)
		FPS_free_processing_struct_m11(session->video_metadata_fps, TRUE_m11);
	if (session->Sgmt_records != NULL)
		free_m11((void *) session->Sgmt_records, __FUNCTION__);
	if (session->record_data_fps != NULL)
		FPS_free_processing_struct_m11(session->record_data_fps, TRUE_m11);
	if (session->record_indices_fps != NULL)
		FPS_free_processing_struct_m11(session->record_indices_fps, TRUE_m11);
	if (session->time_series_channels != NULL) {
		for (i = 0; i < session->number_of_time_series_channels; ++i) {
			chan = session->time_series_channels[i];
			if (chan != NULL)
				free_channel_m11(chan, TRUE_m11);
		}
		free_m11((void *) session->time_series_channels, __FUNCTION__);
	}
	if (session->video_channels != NULL) {
		for (i = 0; i < session->number_of_video_channels; ++i) {
			chan = session->video_channels[i];
			if (chan != NULL)
				free_channel_m11(chan, TRUE_m11);
		}
		free_m11((void *) session->video_channels, __FUNCTION__);
	}
	if (session->segmented_sess_recs != NULL)
		free_segmented_ses_recs_m11(session->segmented_sess_recs, TRUE_m11);

	if (session->contigua != NULL)
		free_m11(session->contigua, __FUNCTION__);
	
	if (free_session_structure == TRUE_m11) {
		free_m11((void *) session, __FUNCTION__);
		// reset current session globals
		globals_m11->session_UID = UID_NO_ENTRY_m11;
		*globals_m11->session_directory = 0;
		globals_m11->session_start_time = GLOBALS_SESSION_START_TIME_DEFAULT_m11;
		globals_m11->session_end_time = GLOBALS_SESSION_END_TIME_DEFAULT_m11;
		globals_m11->session_name = NULL;
		*globals_m11->uh_session_name = 0;
		*globals_m11->fs_session_name = 0;
		globals_m11->session_start_time = UUTC_NO_ENTRY_m11;
		globals_m11->session_end_time = UUTC_NO_ENTRY_m11;
		globals_m11->number_of_session_samples = SAMPLE_NUMBER_NO_ENTRY_m11;  // == number_of_session_frames
		globals_m11->number_of_session_segments = SEGMENT_NUMBER_NO_ENTRY_m11;
		globals_m11->number_of_mapped_segments = SEGMENT_NUMBER_NO_ENTRY_m11;
		globals_m11->reference_channel = NULL;
		*globals_m11->reference_channel_name = 0;
		// reset time globals
		globals_m11->time_constants_set = FALSE_m11;
		globals_m11->RTO_known = GLOBALS_RTO_KNOWN_DEFAULT_m11;
		// do not zero password data so hints can be shown, if they exist
		// reset miscellaneous globals
		globals_m11->mmap_block_bytes = GLOBALS_MMAP_BLOCK_BYTES_NO_ENTRY_m11;
	} else {  // leave name, path, slice, & globals intact (i.e. clear everything with allocated memory)
		session->flags &= ~LH_OPEN_m11;
		session->last_access_time = UUTC_NO_ENTRY_m11;
		session->number_of_time_series_channels = 0;
		session->time_series_channels = NULL;
		session->time_series_metadata_fps = NULL;
		session->number_of_video_channels = 0;
		session->video_channels = NULL;
		session->video_metadata_fps = NULL;
		session->record_data_fps = NULL;
		session->record_indices_fps = NULL;
		session->segmented_sess_recs = NULL;
		session->contigua = NULL;
		session->number_of_contigua = 0;
	}
	
	return;
}


void	frequencies_vary_m11(SESSION_m11 *sess)
{
	si4					i, n_chans, seg_idx;
	sf8					rate, min_rate, max_rate;
	CHANNEL_m11				*chan, *max_chan, *min_chan;
	SEGMENT_m11				*seg;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// check time series channels
	seg_idx = get_segment_index_m11(FIRST_OPEN_SEGMENT_m11);
	n_chans = sess->number_of_time_series_channels;
	globals_m11->time_series_frequencies_vary = UNKNOWN_m11;
	globals_m11->minimum_time_series_frequency = globals_m11->maximum_time_series_frequency = FREQUENCY_NO_ENTRY_m11;
	globals_m11->minimum_time_series_frequency_channel = globals_m11->maximum_time_series_frequency_channel = NULL;
	if (n_chans) {
		for (i = 0; i < n_chans; ++i) {
			chan = sess->time_series_channels[i];
			if (chan->flags & LH_CHANNEL_ACTIVE_m11)
				break;
		}
		if (i < n_chans) {
			min_chan = max_chan = chan;
			seg = chan->segments[seg_idx];
			min_rate = max_rate = seg->metadata_fps->metadata->time_series_section_2.sampling_frequency;
			for (++i; i < n_chans; ++i) {
				chan = sess->time_series_channels[i];
				if (chan->flags & LH_CHANNEL_ACTIVE_m11) {
					seg = chan->segments[seg_idx];
					rate = seg->metadata_fps->metadata->time_series_section_2.sampling_frequency;
					if (min_rate > rate) {
						min_rate = rate;
						min_chan = chan;
					} else if (max_rate < rate) {
						max_rate = rate;
						max_chan = chan;
					}
				}
			}
			if (min_rate == max_rate)
				globals_m11->time_series_frequencies_vary = FALSE_m11;
			else
				globals_m11->time_series_frequencies_vary = TRUE_m11;
			globals_m11->minimum_time_series_frequency = min_rate;
			globals_m11->maximum_time_series_frequency = max_rate;
			globals_m11->minimum_time_series_frequency_channel = min_chan;
			globals_m11->maximum_time_series_frequency_channel = max_chan;
		}
	}

	// check video channels
	n_chans = sess->number_of_video_channels;
	globals_m11->video_frame_rates_vary = UNKNOWN_m11;
	globals_m11->minimum_video_frame_rate = globals_m11->maximum_video_frame_rate = FREQUENCY_NO_ENTRY_m11;
	globals_m11->minimum_video_frame_rate_channel = globals_m11->maximum_video_frame_rate_channel = NULL;
	if (n_chans) {
		for (i = 0; i < n_chans; ++i) {
			chan = sess->video_channels[i];
			if (chan->flags & LH_CHANNEL_ACTIVE_m11)
				break;
		}
		if (i < n_chans) {
			min_chan = max_chan = chan;
			seg = chan->segments[seg_idx];
			min_rate = max_rate = seg->metadata_fps->metadata->video_section_2.frame_rate;
			for (++i; i < n_chans; ++i) {
				chan = sess->video_channels[i];
				if (chan->flags & LH_CHANNEL_ACTIVE_m11) {
					seg = chan->segments[seg_idx];
					rate = seg->metadata_fps->metadata->video_section_2.frame_rate;
					if (min_rate > rate) {
						min_rate = rate;
						min_chan = chan;
					} else if (max_rate < rate) {
						max_rate = rate;
						max_chan = chan;
					}
				}
				if (min_rate == max_rate)
					globals_m11->video_frame_rates_vary = FALSE_m11;
				else
					globals_m11->video_frame_rates_vary = TRUE_m11;
				globals_m11->minimum_video_frame_rate = min_rate;
				globals_m11->maximum_video_frame_rate = max_rate;
				globals_m11->minimum_video_frame_rate_channel = min_chan;
				globals_m11->maximum_video_frame_rate_channel = max_chan;
			}
		}
	}

	return;
}


si1	**generate_file_list_m11(si1 **file_list, si4 *n_files, si1 *enclosing_directory, si1 *name, si1 *extension, ui4 flags)
{
	TERN_m11	regex;
	si1		tmp_enclosing_directory[FULL_FILE_NAME_BYTES_m11], tmp_path[FULL_FILE_NAME_BYTES_m11];
	si1		tmp_name[FULL_FILE_NAME_BYTES_m11], tmp_extension[16], tmp_ext[16];
	si1		**tmp_ptr_ptr;
	ui4		path_parts;
	si4		i, j, ret_val, n_in_files, *n_out_files;
	size_t		command_len;
	FILE		*fp;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// can be used to get a directory list also
	// file_list entries, enclosing_directory, name, & extension can contain regexp
	// if file_list is NULL it will be allocated
	
	n_in_files = *n_files;
	n_out_files = n_files;
	path_parts = flags & GFL_PATH_PARTS_MASK_m11;
	
	// quick bailout for nothing to do (file_list passed, paths are from root, & contain no regex)
	if (check_file_list_m11(file_list, n_in_files) == TRUE_m11) {
		if ((flags & GFL_FREE_INPUT_FILE_LIST_m11) == 0) {  // caller expects a copy to be returned
			tmp_ptr_ptr = (si1 **) calloc_2D_m11((size_t) n_in_files, FULL_FILE_NAME_BYTES_m11, sizeof(si1), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
			for (i = 0; i < n_in_files; ++i)
				strcpy(tmp_ptr_ptr[i], file_list[i]);
			file_list = tmp_ptr_ptr;
		}
		goto GFL_CONDITION_RETURN_DATA_m11;
	}
	
	// copy incoming arguments so as not to modify, and in case they are const type
	if (enclosing_directory == NULL)
		*tmp_enclosing_directory = 0;
	else
		strcpy(tmp_enclosing_directory, enclosing_directory);
	enclosing_directory = tmp_enclosing_directory;
				
	if (name == NULL)
		*tmp_name = 0;
	else
		strcpy(tmp_name, name);
	name = tmp_name;
	
	if (extension == NULL)
		*tmp_extension = 0;
	else
		strcpy(tmp_extension, extension);
	extension = tmp_extension;
	
	// file list passed:
	// If list components do not have a file name, and one is passed, it is used.
	// If list components do not have a file name, and none is passed, "*" is used.
	// If list components do not have an enclosing directory, and one is passed, it is used.
	// If list components do not have an enclosing directory, and none is passed, path_from_root_m11() is used.
	// If list components do not have an extension, and one is passed, it is used.
	// If list components do not have an extension, and none is passed, none is used.
	regex = FALSE_m11;
	if (file_list != NULL) {
		tmp_ptr_ptr = (si1 **) calloc_2D_m11((size_t) n_in_files, FULL_FILE_NAME_BYTES_m11, sizeof(si1), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
		// copy file_list
		for (i = 0; i < n_in_files; ++i) {
			// check for regex
			if (regex == FALSE_m11)
				regex = STR_contains_regex_m11(file_list[i]);
			// fill in list entry path components
			extract_path_parts_m11(file_list[i], tmp_path, NULL, tmp_ext);
			if (*tmp_path == 0) {
				if (*enclosing_directory == 0)
					sprintf_m11(tmp_ptr_ptr[i], "%s/%s", enclosing_directory, file_list[i]);
				else
					path_from_root_m11(file_list[i], file_list[i]);
				
			} else {
				strcpy(tmp_ptr_ptr[i], file_list[i]);
			}
			if (*tmp_ext == 0 && *extension)
				sprintf_m11(tmp_ptr_ptr[i], "%s.%s", tmp_ptr_ptr[i], extension);
		}
		if (flags & GFL_FREE_INPUT_FILE_LIST_m11)
			free_2D_m11((void **) file_list, n_in_files, __FUNCTION__);
		file_list = tmp_ptr_ptr;
	}

	// no file_list passed (+/- enclosing_directory, +/- name, +/- extension, are passed instead)
	// If no enclosing_directory passed, path_from_root_m110() is used.
	// If no name is passed, "*" is used.
	// If no extension is passed, none is used.
	else {  // file_list == NULL
		file_list = (si1 **) calloc_2D_m11((size_t) 1, FULL_FILE_NAME_BYTES_m11, sizeof(si1), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
		path_from_root_m11(enclosing_directory, enclosing_directory);
		if (*name)
			sprintf_m11(file_list[0], "%s/%s", enclosing_directory, name);
		else
			sprintf_m11(file_list[0], "%s/*", enclosing_directory);
		if (*extension)
			sprintf_m11(file_list[0], "%s.%s", file_list[0], extension);
		n_in_files = 1;
		if (STR_contains_regex_m11(file_list[0]) == TRUE_m11)
			regex = TRUE_m11;
	}

	// expand regex (use system shell to expand regex)
	if (regex == TRUE_m11) {
		
	#if defined MACOS_m11 || defined LINUX_m11
		si1	*command, *tmp_str;
		
		
		command_len = n_in_files * FULL_FILE_NAME_BYTES_m11;
		if (flags & GFL_INCLUDE_INVISIBLE_FILES_m11)
			command_len <<= 1;
		command = (si1 *) calloc(command_len, sizeof(si1));

		strcpy(command, "ls -1d");
		for (i = 0; i < n_in_files; ++i) {
			escape_chars_m11(file_list[i], (si1) 0x20, FULL_FILE_NAME_BYTES_m11);  // escape spaces
			escape_chars_m11(file_list[i], (si1) 0x27, FULL_FILE_NAME_BYTES_m11);  // escape apostrophes
			sprintf_m11(command, "%s %s", command, file_list[i]);
			if (flags & GFL_INCLUDE_INVISIBLE_FILES_m11) {
				extract_path_parts_m11(file_list[i], NULL, name, extension);
				sprintf_m11(command, "%s %s/.%s", command, enclosing_directory, name);  // explicitly include hidden files & directories with a prepended "."
				if (*extension)
					sprintf_m11(command, "%s.%s", command, extension);
			}
		}
		sprintf_m11(command, "%s > %s 2> %s", command, globals_m11->temp_file, NULL_DEVICE_m11);
		free_m11((void *) file_list, __FUNCTION__);
		
		// count expanded file list
		*n_out_files = 0;
		ret_val = system_m11(command, FALSE_m11, __FUNCTION__, RETURN_ON_FAIL_m11 | SUPPRESS_ERROR_OUTPUT_m11);
		if (ret_val) {
			free((void *) command);
			return(NULL);
		}
		fp = fopen_m11(globals_m11->temp_file, "r", __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
		
		tmp_str = command;
		while (fscanf(fp, "%[^\n]", tmp_str) != EOF) {
			fgetc(fp);
			++(*n_out_files);
		}
		free((void *) tmp_str);
		
		if (*n_out_files == 0) {
			fclose(fp);
			return(NULL);
		}
		rewind(fp);
	#endif  // MACOS_m11 || LINUX_m11
		
	#ifdef WINDOWS_m11
		*n_out_files = win_ls_1d_to_tmp_m11(file_list, n_in_files, TRUE_m11);
		free_m11((void *) file_list, __FUNCTION__);
		if (*n_out_files == -1) {  // error
			*n_out_files = 0;
			return(NULL);
		}
		fp = fopen_m11(globals_m11->temp_file, "r", __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	#endif  // WINDOWS_m11

		// re-allocate
		file_list = (si1 **) calloc_2D_m11((size_t) *n_out_files, FULL_FILE_NAME_BYTES_m11, sizeof(si1), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
		
		// build file list
		for (i = 0; i < *n_out_files; ++i) {
			fscanf(fp, "%[^\n]", file_list[i]);
			fgetc(fp);
		}

		// clean up
		fclose(fp);
	}

GFL_CONDITION_RETURN_DATA_m11:
	
	// return requested path parts
	for (i = j = 0; i < *n_out_files; ++i) {
		extract_path_parts_m11(file_list[i], enclosing_directory, tmp_name, tmp_extension);
		if ((flags & GFL_INCLUDE_INVISIBLE_FILES_m11) == 0)
			if (*tmp_name == '.')  // exclude invisible files
				continue;
		switch (path_parts) {
			case (GFL_FULL_PATH_m11):
				if (i != j)
					strcpy(file_list[j], file_list[i]);
				break;
			case (GFL_PATH_m11 | GFL_NAME_m11):
				sprintf_m11(file_list[j], "%s/%s", enclosing_directory, tmp_name);
				break;
			case (GFL_NAME_m11 | GFL_EXTENSION_m11):
				sprintf_m11(file_list[j], "%s.%s", tmp_name, tmp_extension);
				break;
			case GFL_NAME_m11:
				strcpy(file_list[j], tmp_name);
				break;
			default:
				error_message_m11("%s(): unrecognized path component combination (path_parts == %hhu)\n", __FUNCTION__, path_parts);
				break;
		}
		++j;
	}
	*n_out_files = j;
	
	// sort file list (so results are consistent across operating systems)
	STR_sort_m11(file_list, *n_out_files);

	return(file_list);
}


si1	*generate_hex_string_m11(ui1 *bytes, si4 num_bytes, si1 *string)
{
	si4	i;
	si1	*s;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (string == NULL)  // allocate if NULL is passed
		string = (si1 *) calloc_m11((size_t)((num_bytes + 1) * 3), sizeof(si1), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	
	s = string;
	*s++ = '0';
	*s++ = 'x';
	
	for (i = 0; i < num_bytes; ++i) {
		sprintf_m11(s, " %02x", bytes[i]);
		if (bytes[i] < 0x10)
			*(s + 1) = '0';
		s += 3;
	}
	*s = 0;
	
	return(string);
}


ui4    generate_MED_path_components_m11(si1 *path, si1 *MED_dir, si1 *MED_name)
{
	si1     extension[TYPE_BYTES_m11], local_MED_name[SEGMENT_BASE_FILE_NAME_BYTES_m11];
	si1     local_MED_dir[FULL_FILE_NAME_BYTES_m11];;
	si4     fe, name_bytes;
	ui4     code;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (MED_dir == NULL)
		*(MED_dir = local_MED_dir) = 0;
	if (MED_name == NULL)
		*(MED_name = local_MED_name) = 0;
	
	// copy & condition path
	strcpy(local_MED_dir, path);
	// escaped characters can happen if string with escaped chars is also quoted (e.g. by a shell script) => pretty uncommon
	unescape_chars_m11(local_MED_dir, (si1) 0x20);  // spaces
	unescape_chars_m11(local_MED_dir, (si1) 0x27);  // apostrophes
	path_from_root_m11(local_MED_dir, local_MED_dir);

	// check path: if file passed, get enclosing directory
	fe = file_exists_m11(local_MED_dir);
	if (fe == FILE_EXISTS_m11) {
		extract_path_parts_m11(local_MED_dir, local_MED_dir, NULL, NULL);
	} else if (fe == DOES_NOT_EXIST_m11) {
		error_message_m11("%s(): passed path \"%s\" does not exist => returning\n", __FUNCTION__, local_MED_dir);
		return(NO_TYPE_CODE_m11);
	} else if (fe == FILE_EXISTS_ERROR_m11) {
		error_message_m11("%s(): file_exists_m11 error() => returning\n", __FUNCTION__);
		return(NO_TYPE_CODE_m11);
	}

	// get name & extension
	extract_path_parts_m11(local_MED_dir, NULL, local_MED_name, extension);

	code = MED_type_code_from_string_m11(extension);
	switch (code) {
		case SESSION_DIRECTORY_TYPE_CODE_m11:
		case TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m11:
		case VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m11:
		case RECORD_DIRECTORY_TYPE_CODE_m11:
			name_bytes = BASE_FILE_NAME_BYTES_m11;
			break;
		case TIME_SERIES_SEGMENT_DIRECTORY_TYPE_CODE_m11:
		case VIDEO_SEGMENT_DIRECTORY_TYPE_CODE_m11:
			name_bytes = SEGMENT_BASE_FILE_NAME_BYTES_m11;
			break;
		default:
			error_message_m11("%s(): passed path \"%s\" is not a MED directory\n", __FUNCTION__, local_MED_dir);
			return(NO_TYPE_CODE_m11);
	}

	// copy to outputs, if provided
	if (MED_dir != NULL)
		snprintf_m11(MED_dir, FULL_FILE_NAME_BYTES_m11, "%s", local_MED_dir);
	if (MED_name != NULL)
		snprintf_m11(MED_name, name_bytes, "%s", local_MED_name);

	return(code);
}


si8	generate_recording_time_offset_m11(si8 recording_start_time_uutc)
{
	si4		dst_offset;
	time_t		epoch_utc, recording_start_time_utc, offset_utc_time;
	struct tm	local_time_info, offset_time_info;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// receives UNOFFSET recording start time (or CURRENT_TIME_m11); returns OFFSET recording start time
	
	if (recording_start_time_uutc == CURRENT_TIME_m11) // use current system time
		recording_start_time_uutc = current_uutc_m11();
	
	recording_start_time_utc = recording_start_time_uutc / (si8)1000000;
	
	// get epoch & local time
	epoch_utc = 0;
#if defined MACOS_m11 || defined LINUX_m11
	gmtime_r(&epoch_utc, &offset_time_info);
	localtime_r(&recording_start_time_utc, &local_time_info);
#endif
#ifdef WINDOWS_m11
	offset_time_info = *(gmtime(&epoch_utc));
	local_time_info = *(localtime(&recording_start_time_utc));
#endif
	
	// set offset time info
	offset_time_info.tm_sec = local_time_info.tm_sec;
	offset_time_info.tm_min = local_time_info.tm_min;
	offset_time_info.tm_hour = local_time_info.tm_hour;
	
	// get offset UTC time
#if defined MACOS_m11 || defined LINUX_m11
	offset_utc_time = timegm(&offset_time_info);
#endif
#ifdef WINDOWS_m11
	offset_utc_time = _mkgmtime(&offset_time_info);
#endif
	dst_offset = DST_offset_m11(recording_start_time_uutc);
	if (dst_offset)  // adjust to standard time if DST in effect
		offset_utc_time -= dst_offset;
	
	// set global offset
	globals_m11->recording_time_offset = (recording_start_time_utc - offset_utc_time) * (si8)1000000;
	
	if (globals_m11->verbose == TRUE_m11)
		message_m11("Recording Time Offset = %ld", globals_m11->recording_time_offset);
	
	globals_m11->RTO_known = TRUE_m11;
	
	return(recording_start_time_uutc - globals_m11->recording_time_offset);
}


si1	*generate_segment_name_m11(FILE_PROCESSING_STRUCT_m11 *fps, si1 *segment_name)
{
	si1	segment_number_str[FILE_NUMBERING_DIGITS_m11 + 1];
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (segment_name == NULL)  // if NULL is passed, this will be allocated, but the calling function has the responsibility to free it.
		segment_name = (si1 *) malloc_m11((size_t) SEGMENT_BASE_FILE_NAME_BYTES_m11, __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	
	numerical_fixed_width_string_m11(segment_number_str, FILE_NUMBERING_DIGITS_m11, fps->universal_header->segment_number);
	
	snprintf_m11(segment_name, SEGMENT_BASE_FILE_NAME_BYTES_m11, "%s_s%s", fps->universal_header->channel_name, segment_number_str);
	
	return(segment_name);
}


ui8	generate_UID_m11(ui8 *uid)
{
	si4	        i;
	ui1		*ui1_p;
	static ui8      local_UID;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// Note if NULL is passed for uid, this function is not thread-safe
	if (uid == NULL)
		uid = (ui8 *)&local_UID;
	ui1_p = (ui1 *) uid;
	
RESERVED_UID_VALUE_m11:
#if defined MACOS_m11 || defined LINUX_m11
	for (i = 0; i < UID_BYTES_m11; ++i)
		ui1_p[i] = (ui1) (random() % (ui8) 0x100);
#endif
#ifdef WINDOWS_m11
	for (i = 0; i < UID_BYTES_m11; ++i)
		ui1_p[i] = (ui1) ((ui4) rand() % (ui4) 0x100);
#endif
	switch (*uid) {
		case UID_NO_ENTRY_m11:
			goto RESERVED_UID_VALUE_m11;
		case CMP_BLOCK_START_UID_m11:
			goto RESERVED_UID_VALUE_m11;
	}
	
	return(*uid);
}


CHANNEL_m11	*get_active_channel_m11(SESSION_m11 *sess, si1 chan_type)
{
	si4		i, n_chans;
	CHANNEL_m11	*chan;


	// check time series channels
	if (chan_type == DEFAULT_CHANNEL_m11 || chan_type == DEFAULT_TIME_SERIES_CHANNEL_m11) {
		n_chans = sess->number_of_time_series_channels;
		for (i = 0; i < n_chans; ++i) {
			chan = sess->time_series_channels[i];
			if (chan->flags & LH_CHANNEL_ACTIVE_m11)
				return(chan);
		}
	}

	// check video channels
	if (chan_type == DEFAULT_CHANNEL_m11 || chan_type == DEFAULT_VIDEO_CHANNEL_m11) {
		n_chans = sess->number_of_video_channels;
		for (i = 0; i < n_chans; ++i) {
			chan = sess->video_channels[i];
			if (chan->flags & LH_CHANNEL_ACTIVE_m11)
				return(chan);
		}
	}
	
	warning_message_m11("%s((): no active channels\n", __FUNCTION__);
	
	return(NULL);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
ui1	get_cpu_endianness_m11(void)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	ui2	x = 1;

	return(*((ui1 *) &x));
}


ui4	get_level_m11(si1 *full_file_name, ui4 *input_type_code)
{
	si1	enclosing_directory[FULL_FILE_NAME_BYTES_m11];
	ui4	code;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	code = MED_type_code_from_string_m11(full_file_name);
	if (input_type_code != NULL)
		*input_type_code = code;
	
	// Note: if code == RECORD_DIRECTORY_TYPE_CODE_m11, this is session level, but from segmented session records; return this code so caller knows it was not global session records
	switch (code) {
		case NO_FILE_TYPE_CODE_m11:
		case SESSION_DIRECTORY_TYPE_CODE_m11:
		case VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m11:
		case TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m11:
		case TIME_SERIES_SEGMENT_DIRECTORY_TYPE_CODE_m11:
		case VIDEO_SEGMENT_DIRECTORY_TYPE_CODE_m11:
			return(code);
		case RECORD_DIRECTORY_TYPE_CODE_m11:
			return(LH_SESSION_m11);
		case TIME_SERIES_METADATA_FILE_TYPE_CODE_m11:
		case TIME_SERIES_DATA_FILE_TYPE_CODE_m11:
		case TIME_SERIES_INDICES_FILE_TYPE_CODE_m11:
			return(LH_TIME_SERIES_SEGMENT_m11);
		case VIDEO_METADATA_FILE_TYPE_CODE_m11:
		case VIDEO_INDICES_FILE_TYPE_CODE_m11:
			return(LH_VIDEO_SEGMENT_m11);
	}
	
	// record data or indices file
	extract_path_parts_m11(full_file_name, enclosing_directory, NULL, NULL);
	code = MED_type_code_from_string_m11(enclosing_directory);

	return(code);
}


LOCATION_INFO_m11	*get_location_info_m11(LOCATION_INFO_m11 *loc_info, TERN_m11 set_timezone_globals, TERN_m11 prompt)
{
	TERN_m11	free_loc_info = FALSE_m11;
	si1		*command, temp_str[128], *buffer, *pattern, *c;
	si4		ret_val;
	si8		sz, len;
	FILE		*fp;
	time_t 		curr_time;
	struct tm 	loc_time;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (loc_info == NULL) {
		loc_info = (LOCATION_INFO_m11 *) calloc((size_t)1, sizeof(LOCATION_INFO_m11));
		free_loc_info = TRUE_m11;
	} else {
		memset((void *) loc_info, 0, sizeof(LOCATION_INFO_m11));
	}
	
#if defined MACOS_m11 || defined LINUX_m11
	command = "curl -s ipinfo.io";
#endif
#ifdef WINDOWS_m11
	command = "curl.exe -s ipinfo.io";
#endif
	sprintf_m11(temp_str, "%s > %s 2> %s", command, globals_m11->temp_file, NULL_DEVICE_m11);
	ret_val = system_m11(temp_str, FALSE_m11, __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	if (ret_val)
		return(NULL);
	fp = fopen_m11(globals_m11->temp_file, "r", __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	
	// get file length
	sz = file_length_m11(fp, NULL);
	
	// read output
	buffer = (si1 *) calloc((size_t)sz, sizeof(si1));
	fread_m11(buffer, sizeof(si1), (size_t) sz, fp, globals_m11->temp_file, __FUNCTION__, EXIT_ON_FAIL_m11);
	fclose(fp);
	
	// condition output
	STR_strip_character_m11(buffer, '"');
	
	// parse output
	pattern = "ip: ";
	if ((c = STR_match_end_m11(pattern, buffer)) == NULL)
		warning_message_m11("%s(): Could not match pattern \"%s\" in output of \"%s\"\n", __FUNCTION__, pattern, command);
	else
		sscanf(c, "%[^,]", loc_info->WAN_IPv4_address);
	
	pattern = "city: ";
	if ((c = STR_match_end_m11(pattern, buffer)) == NULL)
		warning_message_m11("%s(): Could not match pattern \"%s\" in output of \"%s\"\n", __FUNCTION__, pattern, command);
	else
		sscanf(c, "%[^,]", loc_info->locality);
	
	pattern = "region: ";
	if ((c = STR_match_end_m11(pattern, buffer)) == NULL)
		warning_message_m11("%s(): Could not match pattern \"%s\" in output of \"%s\"\n", __FUNCTION__, pattern, command);
	else
		sscanf(c, "%[^,]", loc_info->timezone_info.territory);
	
	pattern = "country: ";
	if ((c = STR_match_end_m11(pattern, buffer)) == NULL)
		warning_message_m11("%s(): Could not match pattern \"%s\" in output of \"%s\"\n", __FUNCTION__, pattern, command);
	else
		sscanf(c, "%[^,]", loc_info->timezone_info.country_acronym_2_letter);
	
	pattern = "loc: ";
	if ((c = STR_match_end_m11(pattern, buffer)) == NULL)
		warning_message_m11("%s(): Could not match pattern \"%s\" in output of \"%s\"\n", __FUNCTION__, pattern, command);
	else
		sscanf(c, "%lf,%lf", &loc_info->latitude, &loc_info->longitude);
	
	pattern = "postal: ";
	if ((c = STR_match_end_m11(pattern, buffer)) == NULL)
		warning_message_m11("%s(): Could not match pattern \"%s\" in output of \"%s\"\n", __FUNCTION__, pattern, command);
	else
		sscanf(c, "%[^,]", loc_info->postal_code);
	
	pattern = "timezone: ";
	if ((c = STR_match_end_m11(pattern, buffer)) == NULL)
		warning_message_m11("%s(): Could not match pattern \"%s\" in output of \"%s\"\n", __FUNCTION__, pattern, command);
	else
		sscanf(c, "%[^, ]", loc_info->timezone_description);
	
	free((void *) buffer);
	
	// get timezone acronym from system
	curr_time = time(NULL);
#if defined MACOS_m11 || defined LINUX_m11
	localtime_r(&curr_time, &loc_time);
	len = strlen(loc_time.tm_zone);
	if (len >= 3) { // the table does not contain 2 letter timezone acronyms (e.g. MT for MST)
		if (loc_time.tm_isdst)
			strcpy(loc_info->timezone_info.daylight_timezone_acronym, loc_time.tm_zone);
		else
			strcpy(loc_info->timezone_info.standard_timezone_acronym, loc_time.tm_zone);
	}
#endif
#ifdef WINDOWS_m11
	loc_time = *(localtime(&curr_time));
	if (*_tzname[0])
		strcpy(loc_info->timezone_info.standard_timezone, _tzname[0]);
	if (*_tzname[1])
		strcpy(loc_info->timezone_info.daylight_timezone, _tzname[1]);
#endif

	if (set_global_time_constants_m11(&loc_info->timezone_info, 0, prompt) == FALSE_m11)
		warning_message_m11("%s(): could not set timezone globals => returning NULL\n", __FUNCTION__);
	
	if (free_loc_info == TRUE_m11) {
		free((void *) loc_info);
		return(NULL);
	}

	return(loc_info);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4	get_search_mode_m11(TIME_SLICE_m11 *slice)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// time takes priority
	if (slice->start_time != UUTC_NO_ENTRY_m11 && slice->end_time != UUTC_NO_ENTRY_m11)
		return(TIME_SEARCH_m11);
	
	if (slice->start_sample_number != SAMPLE_NUMBER_NO_ENTRY_m11 && slice->end_sample_number != SAMPLE_NUMBER_NO_ENTRY_m11)
		return(SAMPLE_SEARCH_m11);
	
	warning_message_m11("%s(): no valid limit pair\n", __FUNCTION__);
	
	return(FALSE_m11);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4	get_segment_index_m11(si4 segment_number)
{
	si4		i, mapped_segs, sess_segs, first_seg, seg_idx;
	CHANNEL_m11	*chan;
	SEGMENT_m11	*seg;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// returns offset of segment_number into segments array
	// FIRST_OPEN_SEGMENT_m11 returns the first open segment in the reference channel
	// returns FALSE_m11 on error

	mapped_segs = globals_m11->number_of_mapped_segments;
	if (mapped_segs == 0) {
		warning_message_m11("%s(): no mapped segments\n", __FUNCTION__);
		return((si4) FALSE_m11);
	}

	if (segment_number == FIRST_OPEN_SEGMENT_m11 || segment_number == SEGMENT_NUMBER_NO_ENTRY_m11) {
		chan = globals_m11->reference_channel;
		if (chan == NULL) {
			warning_message_m11("%s(): cannot find open segment\n", __FUNCTION__);
			return((si4) FALSE_m11);
		}
		for (i = 0; i < mapped_segs; ++i) {
			seg = chan->segments[i];
			if (seg != NULL)
				if (seg->flags & LH_OPEN_m11)
					break;
					
		}
		if (i == mapped_segs) {
			warning_message_m11("%s(): cannot find open segment\n", __FUNCTION__);
			return((si4) FALSE_m11);
		}
		if (segment_number == SEGMENT_NUMBER_NO_ENTRY_m11)
			warning_message_m11("%s(): segment not specified => returning first open segment from reference channel\n", __FUNCTION__);
		return(i);
	}
	
	sess_segs = globals_m11->number_of_session_segments;
	if (mapped_segs == sess_segs) {  // all segments mapped
		if (segment_number >= 1 && segment_number <= mapped_segs) {
			return(segment_number - 1);
		} else if (segment_number < 1) {
			warning_message_m11("%s(): invalid segment number\n", __FUNCTION__);
			return((si4) FALSE_m11);
		} else {
			warning_message_m11("%s(): unmapped segment\n", __FUNCTION__);
			return((si4) FALSE_m11);
		}
	}
	
	first_seg = globals_m11->first_mapped_segment_number;
	seg_idx = segment_number - first_seg;
	if (seg_idx < 0 || seg_idx >= mapped_segs) {
		warning_message_m11("%s(): unmapped segment\n", __FUNCTION__);
		return((si4) FALSE_m11);
	}
	
	return(seg_idx);
}


si4     get_segment_range_m11(LEVEL_HEADER_m11 *level_header, TIME_SLICE_m11 *slice)
{
	TERN_m11			Sgmts_adequate, free_fps;
	si1				tmp_str[FULL_FILE_NAME_BYTES_m11], sess_path[FULL_FILE_NAME_BYTES_m11], *sess_name;
	ui4				file_exists;
	si4				search_mode, n_segs;
	si8				i, n_recs;
	size_t				n_bytes;
	SESSION_m11			*sess;
	CHANNEL_m11			*chan;
	FILE_PROCESSING_STRUCT_m11	*ri_fps, *rd_fps, *md_fps;
	RECORD_INDEX_m11		*ri;
	Sgmt_RECORD_m11			*Sgmt_records, *Sgmt_rec;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (slice->conditioned == FALSE_m11)
		condition_time_slice_m11(slice);
	slice->start_segment_number = slice->end_segment_number = SEGMENT_NUMBER_NO_ENTRY_m11;
	slice->number_of_segments = UNKNOWN_m11;

	switch (level_header->type_code) {
		case LH_SESSION_m11:
			sess = (SESSION_m11 *) level_header;
			chan = globals_m11->reference_channel;
			Sgmt_records = sess->Sgmt_records;
			break;
		case LH_TIME_SERIES_CHANNEL_m11:
		case LH_VIDEO_CHANNEL_m11:
			chan = (CHANNEL_m11 *) level_header;
			sess = (SESSION_m11 *) chan->parent;
			Sgmt_records = chan->Sgmt_records;
			break;
		default:
			error_message_m11("%s(): invalid level\n", __FUNCTION__);
			return(0);
	}

	// check for valid limit pair (time takes priority)
	if ((search_mode = get_search_mode_m11(slice)) == FALSE_m11)
		return(0);

	if (Sgmt_records == NULL) {
		
		// check for channel level Sgmt records (typically most efficient: usually small files & always contain sample number references)
		sprintf_m11(tmp_str, "%s/%s.%s", chan->path, chan->name, RECORD_INDICES_FILE_TYPE_STRING_m11);
		if (file_exists_m11(tmp_str) == FILE_EXISTS_m11) {
			ri_fps = chan->record_indices_fps = read_file_m11(chan->record_indices_fps, tmp_str, 0, 0, FPS_FULL_FILE_m11, level_header, NULL, USE_GLOBAL_BEHAVIOR_m11);
			chan->record_indices_fps->parent = (void *) chan;
			n_recs = ri_fps->universal_header->number_of_entries;
			ri = ri_fps->record_indices;
			for (i = n_recs; i--; ++ri)
				if (ri->type_code == REC_Sgmt_TYPE_CODE_m11)
					break;
			if (i >= 0) {
				sprintf_m11(tmp_str, "%s/%s.%s", chan->path, chan->name, RECORD_DATA_FILE_TYPE_STRING_m11);
				rd_fps = chan->record_data_fps = read_file_m11(chan->record_data_fps, tmp_str, 0, 0, FPS_UNIVERSAL_HEADER_ONLY_m11, level_header, NULL, USE_GLOBAL_BEHAVIOR_m11);
				chan->record_data_fps->parent = (void *) chan;
				Sgmt_records = build_Sgmt_records_array_m11(ri_fps, rd_fps, NULL);
				if (Sgmt_records != NULL && level_header->type_code == LH_SESSION_m11) {  // copy ref chan Sgmt records into ref chan, if used for session
					n_bytes = (size_t) globals_m11->number_of_session_segments * sizeof(Sgmt_RECORD_m11);
					chan->Sgmt_records = (Sgmt_RECORD_m11 *) malloc_m11(n_bytes, __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
					memcpy((void *) chan->Sgmt_records, (void *) Sgmt_records, n_bytes);
				}
			}
		}
		
		// no channel level Sgmt records => check session level (may not contain sample numbers, but may not need them)
		if (Sgmt_records == NULL) {
			// get global session name(s)
			if (globals_m11->session_UID == UID_NO_ENTRY_m11) {
				find_metadata_file_m11(chan->path, tmp_str);
				md_fps = read_file_m11(NULL, tmp_str, 0, 0, FPS_FULL_FILE_m11, NULL, NULL, USE_GLOBAL_BEHAVIOR_m11);
				FPS_free_processing_struct_m11(md_fps, TRUE_m11);
			}
			sess_name = globals_m11->session_name;
			if (sess != NULL) {
				strcpy(sess_path, sess->path);
				ri_fps = sess->record_indices_fps;
				rd_fps = sess->record_data_fps;
				free_fps = FALSE_m11;
			} else {
				extract_path_parts_m11(chan->path, sess_path, NULL, NULL);
				ri_fps = NULL;
				rd_fps = NULL;
				free_fps = TRUE_m11;
			}
			if (ri_fps == NULL)
				sprintf_m11(tmp_str, "%s/%s.%s", sess_path, sess_name, RECORD_INDICES_FILE_TYPE_STRING_m11);
			else if (*ri_fps->full_file_name)
				strcpy(tmp_str, ri_fps->full_file_name);
			else
				sprintf_m11(tmp_str, "%s/%s.%s", sess_path, sess_name, RECORD_INDICES_FILE_TYPE_STRING_m11);
			file_exists = file_exists_m11(tmp_str);
			if (file_exists == DOES_NOT_EXIST_m11) {  // uh session name is default, try fs session name (e.g. a channel subset)
				if (globals_m11->session_name == globals_m11->uh_session_name) {
					extract_path_parts_m11(sess_path, tmp_str, NULL, NULL);
					sprintf_m11(sess_path, "%s/%s", tmp_str, globals_m11->fs_session_name);
					sprintf_m11(tmp_str, "%s/%s.%s", sess_path, sess_name, RECORD_INDICES_FILE_TYPE_STRING_m11);
					file_exists = file_exists_m11(tmp_str);
				}
			}
			if (file_exists == FILE_EXISTS_m11) {
				ri_fps = read_file_m11(ri_fps, tmp_str, 0, 0, FPS_FULL_FILE_m11, level_header, NULL, USE_GLOBAL_BEHAVIOR_m11);
				n_recs = ri_fps->universal_header->number_of_entries;
				ri = ri_fps->record_indices;
				for (i = n_recs; i--; ++ri)
					if (ri->type_code == REC_Sgmt_TYPE_CODE_m11)
						break;
				if (i >= 0) {  // check that session Sgmt records contain sampling frequency
					sprintf_m11(tmp_str, "%s/%s.%s", sess_path, sess_name, RECORD_DATA_FILE_TYPE_STRING_m11);
					rd_fps = read_file_m11(rd_fps, tmp_str, 0, 0, FPS_UNIVERSAL_HEADER_ONLY_m11, level_header, NULL, USE_GLOBAL_BEHAVIOR_m11);
					Sgmts_adequate = TRUE_m11;
					if (search_mode == SAMPLE_SEARCH_m11) {
						read_file_m11(rd_fps, NULL, ri->file_offset, 0, 1, 0, NULL, USE_GLOBAL_BEHAVIOR_m11);
						Sgmt_rec = (Sgmt_RECORD_m11 *) rd_fps->record_data;
						if (Sgmt_rec->sampling_frequency == FREQUENCY_VARIABLE_m11 || Sgmt_rec->sampling_frequency == FREQUENCY_NO_ENTRY_m11)
							Sgmts_adequate = FALSE_m11;
					}
				} else {
					Sgmts_adequate = FALSE_m11;
				}
				if (Sgmts_adequate == TRUE_m11) {
					Sgmt_records = build_Sgmt_records_array_m11(ri_fps, rd_fps, NULL);
				} else if (free_fps == TRUE_m11) {
					FPS_free_processing_struct_m11(ri_fps, TRUE_m11);
					if (rd_fps != NULL)
						FPS_free_processing_struct_m11(rd_fps, TRUE_m11);
				}
			}
		}

		// no adequate session level Sgmt records => build from reference channel metadata (least efficient option)
		if (Sgmt_records == NULL)
			Sgmt_records = build_Sgmt_records_array_m11(NULL, NULL, chan);
		
		// assign level Sgmt_records pointer
		switch (level_header->type_code) {
			case LH_SESSION_m11:
				sess->Sgmt_records = Sgmt_records;
				break;
			case LH_TIME_SERIES_CHANNEL_m11:
			case LH_VIDEO_CHANNEL_m11:
				chan->Sgmt_records = Sgmt_records;
				break;
		}
	}

	// search Sgmt_records array for segments
	n_segs = search_Sgmt_records_m11(Sgmt_records, slice, search_mode);
	if (n_segs) {
		slice->number_of_segments = n_segs;
		if (level_header->flags & LH_MAP_ALL_SEGMENTS_m11) {
			globals_m11->number_of_mapped_segments = globals_m11->number_of_session_segments;
			globals_m11->first_mapped_segment_number = 1;
		} else {
			globals_m11->number_of_mapped_segments = n_segs;
			globals_m11->first_mapped_segment_number = slice->start_segment_number;
		}
	} else {
		slice->number_of_segments = UNKNOWN_m11;
		globals_m11->number_of_mapped_segments = 0;
		globals_m11->first_mapped_segment_number = 0;
	}

	if (slice->start_time == BEGINNING_OF_TIME_m11) {
		slice->start_time = Sgmt_records[0].start_time;
		slice->start_sample_number = Sgmt_records[0].start_sample_number;
	}
	if (slice->end_time == END_OF_TIME_m11) {
		slice->end_time = Sgmt_records[n_segs - 1].end_time;
		slice->end_sample_number = Sgmt_records[n_segs - 1].end_sample_number;
	}

	return(n_segs);
}


ui4	*get_segment_video_start_frames_m11(FILE_PROCESSING_STRUCT_m11 *video_indices_fps, ui4 *number_of_video_files)
{
	ui4			j, *start_frames, local_number_of_video_files;
	si8			i, n_inds;
	VIDEO_INDEX_m11		*vidx;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// NOTE: start frame numbers are inserted into array at their video file number offset, so array can be used as start_frames[vidx->video_file_number], rather than subtracting 1
	// So start_frames[0] == start_frames[1] == 0
	// pass NULL for number_of_video_files if you don't need the value
	// to get global frame numbers add segment metadata absolute_start_frame_number to these numbers
	
	if (number_of_video_files == NULL)
		number_of_video_files = &local_number_of_video_files;
	vidx = video_indices_fps->video_indices;
	n_inds = video_indices_fps->universal_header->number_of_entries - 1;
	*number_of_video_files = vidx[n_inds].video_file_number - 1;  // terminal index video file number is number of imaginary next file
	start_frames = (ui4 *) calloc_m11((size_t) (*number_of_video_files + 1), sizeof(ui4), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);  // indexing from 1
	
	for (i = n_inds, j = 1; i--; ++vidx)
		if (vidx->video_file_number == j)
			start_frames[j++] = vidx->start_frame_number;
			
	return(start_frames);
}


si1	*get_session_directory_m11(si1 *session_directory, si1 *MED_file_name, FILE_PROCESSING_STRUCT_m11 *MED_fps)
{
	TERN_m11	set_global_session_name;
	si1		temp_str[FULL_FILE_NAME_BYTES_m11];
	ui4		code;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// session_directory is return vehicle. MED_file_name is input vehicle.
	// If NULL passed for session directory, sets global session directory and session name from
	// file path, not global original session name which comes from universal header.
	
	
	if (MED_file_name == NULL) {
		if (MED_fps == NULL)
			return(NULL);
		if (*MED_fps->full_file_name)
			MED_file_name = MED_fps->full_file_name;
		else
			return(NULL);
	}
	
	if (session_directory == NULL || session_directory == globals_m11->session_directory) {
		set_global_session_name = TRUE_m11;
		session_directory = globals_m11->session_directory;
	} else {
		set_global_session_name = FALSE_m11;
	}
	
	memset(session_directory, 0, FULL_FILE_NAME_BYTES_m11);
	path_from_root_m11(MED_file_name, session_directory);
	code = MED_type_code_from_string_m11(session_directory);

	switch (code) {
		case NO_FILE_TYPE_CODE_m11:
			return(NULL);
			
		// up zero levels
		case SESSION_DIRECTORY_TYPE_CODE_m11:
			break;
			
		// up one level
		case TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m11:
		case VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m11:
		case RECORD_DIRECTORY_TYPE_CODE_m11:  // segmented session records is only MED component that uses a diectory - session level
			extract_path_parts_m11(session_directory, session_directory, NULL, NULL);
			break;
			
		// up two levels
		case TIME_SERIES_SEGMENT_DIRECTORY_TYPE_CODE_m11:
		case VIDEO_SEGMENT_DIRECTORY_TYPE_CODE_m11:
			extract_path_parts_m11(session_directory, session_directory, NULL, NULL);
			extract_path_parts_m11(session_directory, session_directory, NULL, NULL);
			break;
			
		// up 3 levels
		case TIME_SERIES_METADATA_FILE_TYPE_CODE_m11:
		case TIME_SERIES_DATA_FILE_TYPE_CODE_m11:
		case TIME_SERIES_INDICES_FILE_TYPE_CODE_m11:
		case VIDEO_METADATA_FILE_TYPE_CODE_m11:
		case VIDEO_INDICES_FILE_TYPE_CODE_m11:
			extract_path_parts_m11(session_directory, session_directory, NULL, NULL);
			extract_path_parts_m11(session_directory, session_directory, NULL, NULL);
			extract_path_parts_m11(session_directory, session_directory, NULL, NULL);
			break;
		
		// up variable number of levels (recursion - need local storage)
		case RECORD_DATA_FILE_TYPE_CODE_m11:
		case RECORD_INDICES_FILE_TYPE_CODE_m11:
			extract_path_parts_m11(session_directory, temp_str, NULL, NULL);
			return(get_session_directory_m11(session_directory, temp_str, MED_fps));
			break;
	}
	
	if (set_global_session_name == TRUE_m11) {
		extract_path_parts_m11(session_directory, NULL, globals_m11->fs_session_name, NULL);
		if (MED_fps != NULL) {
			globals_m11->session_UID = MED_fps->universal_header->session_UID;
			strcpy(globals_m11->uh_session_name, MED_fps->universal_header->session_name);
			globals_m11->session_name = globals_m11->uh_session_name;
		} else {
			globals_m11->session_name = globals_m11->fs_session_name;
		}
	}

	return(session_directory);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
TERN_m11     include_record_m11(ui4 type_code, si4 *record_filters)
{
	si1			mode;
	const si1		INCLUDE_POSITIVE = 1, EXCLUDE_NEGATIVE = -1;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// record_filters is a signed, "NULL terminated" array version of MED record type codes to include or exclude when reading records.
	// The terminal entry is NO_TYPE_CODE_m11 (== zero). NULL or no filter codes includes all records (== no filters).
	// filter modes: match positive: include
	//		 match negative: exclude
	//		 no match:
	//			all filters positive: exclude
	//			else: include
	// Note: as type codes are composed of ascii bytes values (< 0x80), it is always possible to make them negative without promotion.
	//
	// Example usage: si4	my_rec_filters[] = { REC_Sgmt_TYPE_CODE_m11, REC_Note_TYPE_CODE_m11, NO_TYPE_CODE_m11 };
	//
	// If the passed record_filters is NULL, the global record_filters will be used.
	// If the global record_filters are NULL, all records will be accepted.
	// If record_filters is a "zero-length" array (i.e. record_filters = { NO_TYPE_CODE_m11 }), all records will be accepted.

	
	if (record_filters == NULL) {
		record_filters = globals_m11->record_filters;
		if (record_filters == NULL)
			return(TRUE_m11);
		if (*record_filters == NO_TYPE_CODE_m11)
			return(TRUE_m11);
	}

	for (mode = 0; *record_filters; ++record_filters) {
		if (*record_filters > 0) {
			if (type_code == (si4) *record_filters)
				return(TRUE_m11);
			if (mode)
				continue;
			mode = INCLUDE_POSITIVE;
		} else {
			if (type_code == -(*record_filters))
				return(FALSE_m11);
			mode = EXCLUDE_NEGATIVE;
		}
	}
	
	if (mode == INCLUDE_POSITIVE)
		return(FALSE_m11);
	
	return(TRUE_m11);
}


//***********************************************************************//
//****************************  MED GLOBALS  ****************************//
//***********************************************************************//

TERN_m11	initialize_globals_m11(void)
{
#ifdef FN_DEBUG_m11  // don't use message() until UTF8 tables initialized
	printf_m11("%s()\n", __FUNCTION__);
#endif

	pthread_mutex_init_m11(&globals_m11_mutex, NULL);
	pthread_mutex_lock_m11(&globals_m11_mutex);
	
	// globals themselves
	if (globals_m11 == NULL) {
		#ifdef MATLAB_PERSISTENT_m11
		globals_m11 = (GLOBALS_m11 *) mxCalloc((mwSize) 1, (mwSize) sizeof(GLOBALS_m11));
		#else
		globals_m11 = (GLOBALS_m11 *) calloc((size_t) 1, sizeof(GLOBALS_m11));
		#endif
		if (globals_m11 == NULL) {
			pthread_mutex_lock_m11(&globals_m11_mutex);
			return(FALSE_m11);
		}
	}

	// set global mutices
	pthread_mutex_init_m11(&globals_m11->TZ_mutex, NULL);
	pthread_mutex_init_m11(&globals_m11->CRC_mutex, NULL);
	pthread_mutex_init_m11(&globals_m11->AES_mutex, NULL);
	pthread_mutex_init_m11(&globals_m11->SHA_mutex, NULL);
	pthread_mutex_init_m11(&globals_m11->UTF8_mutex, NULL);
	pthread_mutex_init_m11(&globals_m11->behavior_mutex, NULL);
	pthread_mutex_init_m11(&globals_m11->AT_mutex, NULL);

	// AT (do this as soon as possible)
#ifdef MATLAB_PERSISTENT_m11
	if (globals_m11->AT_nodes != NULL) {
		mxFree((void *) globals_m11->AT_nodes);
		globals_m11->AT_nodes = NULL;
	}
	globals_m11->AT_nodes = (AT_NODE *) mxCalloc((mwSize) GLOBALS_AT_LIST_SIZE_INCREMENT_m11, (mwSize)sizeof(AT_NODE));
#else
	if (globals_m11->AT_nodes != NULL) {
		free((void *) globals_m11->AT_nodes);
		globals_m11->AT_nodes = NULL;
	}
	globals_m11->AT_nodes = (AT_NODE *) calloc((size_t) GLOBALS_AT_LIST_SIZE_INCREMENT_m11, sizeof(AT_NODE));
#endif
	if (globals_m11->AT_nodes == NULL) {
		printf_m11("%s(): calloc failure for AT list => exiting\n", __FUNCTION__);
		exit(-1);
	}
	globals_m11->AT_node_count = GLOBALS_AT_LIST_SIZE_INCREMENT_m11;
	globals_m11->AT_used_node_count = 0;
	
	// password structure
	memset((void *) &globals_m11->password_data, 0, sizeof(PASSWORD_DATA_m11));
	
	// record filters
	globals_m11->record_filters = NULL;
	
	// current session constants
	globals_m11->session_UID = UID_NO_ENTRY_m11;
	*globals_m11->session_directory = 0;
	globals_m11->session_start_time = GLOBALS_SESSION_START_TIME_DEFAULT_m11;
	globals_m11->session_end_time = GLOBALS_SESSION_END_TIME_DEFAULT_m11;
	globals_m11->session_name = NULL;
	*globals_m11->uh_session_name = 0;
	*globals_m11->fs_session_name = 0;
	globals_m11->session_start_time = UUTC_NO_ENTRY_m11;
	globals_m11->session_end_time = UUTC_NO_ENTRY_m11;
	globals_m11->number_of_session_samples = SAMPLE_NUMBER_NO_ENTRY_m11;  // == number_of_session_frames
	globals_m11->number_of_session_segments = SEGMENT_NUMBER_NO_ENTRY_m11;
	globals_m11->number_of_mapped_segments = SEGMENT_NUMBER_NO_ENTRY_m11;
	globals_m11->reference_channel = NULL;
	*globals_m11->reference_channel_name = 0;
	
	// active channel constants
	globals_m11->time_series_frequencies_vary = UNKNOWN_m11;
	globals_m11->minimum_time_series_frequency = FREQUENCY_NO_ENTRY_m11;
	globals_m11->maximum_time_series_frequency = FREQUENCY_NO_ENTRY_m11;
	globals_m11->minimum_time_series_frequency_channel = NULL;
	globals_m11->maximum_time_series_frequency_channel = NULL;
	globals_m11->video_frame_rates_vary = UNKNOWN_m11;;
	globals_m11->minimum_video_frame_rate = FREQUENCY_NO_ENTRY_m11;
	globals_m11->maximum_video_frame_rate = FREQUENCY_NO_ENTRY_m11;
	globals_m11->minimum_video_frame_rate_channel = NULL;
	globals_m11->maximum_video_frame_rate_channel = NULL;

	// time constants
	globals_m11->time_constants_set = FALSE_m11;
	globals_m11->RTO_known = GLOBALS_RTO_KNOWN_DEFAULT_m11;
	globals_m11->observe_DST = GLOBALS_OBSERVE_DST_DEFAULT_m11;
	globals_m11->recording_time_offset = GLOBALS_RECORDING_TIME_OFFSET_DEFAULT_m11;
	globals_m11->standard_UTC_offset = GLOBALS_STANDARD_UTC_OFFSET_DEFAULT_m11;
	globals_m11->daylight_time_start_code.value = DTCC_VALUE_NO_ENTRY_m11;
	globals_m11->daylight_time_end_code.value = DTCC_VALUE_NO_ENTRY_m11;
	strcpy(globals_m11->standard_timezone_acronym, GLOBALS_STANDARD_TIMEZONE_ACRONYM_DEFAULT_m11);
	strcpy(globals_m11->standard_timezone_string, GLOBALS_STANDARD_TIMEZONE_STRING_DEFAULT_m11);
	strcpy(globals_m11->daylight_timezone_acronym, GLOBALS_DAYLIGHT_TIMEZONE_ACRONYM_DEFAULT_m11);
	strcpy(globals_m11->daylight_timezone_string, GLOBALS_DAYLIGHT_TIMEZONE_STRING_DEFAULT_m11);
	if (globals_m11->timezone_table != NULL) {
		free((void *) globals_m11->timezone_table);
		globals_m11->timezone_table = NULL;
	}
	
	// alignment fields
	globals_m11->universal_header_aligned = UNKNOWN_m11;
	globals_m11->metadata_section_1_aligned = UNKNOWN_m11;
	globals_m11->time_series_metadata_section_2_aligned = UNKNOWN_m11;
	globals_m11->video_metadata_section_2_aligned = UNKNOWN_m11;
	globals_m11->metadata_section_3_aligned = UNKNOWN_m11;
	globals_m11->all_metadata_structures_aligned = UNKNOWN_m11;
	globals_m11->time_series_indices_aligned = UNKNOWN_m11;
	globals_m11->video_indices_aligned = UNKNOWN_m11;
	globals_m11->CMP_block_header_aligned = UNKNOWN_m11;
	globals_m11->CMP_record_header_aligned = UNKNOWN_m11;
	globals_m11->record_header_aligned = UNKNOWN_m11;
	globals_m11->record_indices_aligned = UNKNOWN_m11;
	globals_m11->all_record_structures_aligned = UNKNOWN_m11;
	globals_m11->all_structures_aligned = UNKNOWN_m11;
	
	// CRC
	if (globals_m11->CRC_table != NULL) {
		free_m11((void *) globals_m11->CRC_table, __FUNCTION__);
		globals_m11->CRC_table = NULL;
	}
	globals_m11->CRC_mode = GLOBALS_CRC_MODE_DEFAULT_m11;
	
	// AES
	if (globals_m11->AES_sbox_table != NULL) {
		free_m11((void *) globals_m11->AES_sbox_table, __FUNCTION__);
		globals_m11->AES_sbox_table = NULL;
	}
	if (globals_m11->AES_rsbox_table != NULL) {
		free_m11((void *) globals_m11->AES_rsbox_table, __FUNCTION__);
		globals_m11->AES_rsbox_table = NULL;
	}
	if (globals_m11->AES_rcon_table != NULL) {
		free_m11((void *) globals_m11->AES_rcon_table, __FUNCTION__);
		globals_m11->AES_rcon_table = NULL;
	}
	
	// SHA
	if (globals_m11->SHA_h0_table != NULL) {
		free_m11((void *) globals_m11->SHA_h0_table, __FUNCTION__);
		globals_m11->SHA_h0_table = NULL;
	}
	if (globals_m11->SHA_k_table != NULL) {
		free_m11((void *) globals_m11->SHA_k_table, __FUNCTION__);
		globals_m11->SHA_k_table = NULL;
	}
	
	// UTF-8 (UTF8 tables are not allocated with AT functions)
	if (globals_m11->UTF8_offsets_table != NULL) {
		#ifdef MATLAB_PERSISTENT_m11
		mxFree((void *) globals_m11->UTF8_offsets_table);
		#else
		free((void *) globals_m11->UTF8_offsets_table);
		#endif
		globals_m11->UTF8_offsets_table = NULL;
	}
	if (globals_m11->UTF8_trailing_bytes_table != NULL) {
		#ifdef MATLAB_PERSISTENT_m11
		mxFree((void *) globals_m11->UTF8_trailing_bytes_table);
		#else
		free((void *) globals_m11->UTF8_trailing_bytes_table);
		#endif
		globals_m11->UTF8_trailing_bytes_table = NULL;
	}
	
	// error
	globals_m11->err_code = E_NO_ERR_m11;
	globals_m11->err_func = NULL;
	globals_m11->err_line = 0;

	// miscellaneous
	globals_m11->verbose = GLOBALS_VERBOSE_DEFAULT_m11;
	globals_m11->behavior_on_fail = GLOBALS_BEHAVIOR_ON_FAIL_DEFAULT_m11;
	if (globals_m11->behavior_stack != NULL) {
		free_m11((void *) globals_m11->behavior_stack, __FUNCTION__);
		globals_m11->behavior_stack = NULL;
	}
	globals_m11->behavior_stack_entries = globals_m11->behavior_stack_size = 0;
	#if defined MACOS_m11 || defined LINUX_m11
	strcpy(globals_m11->temp_dir, "/tmp");
	strcpy(globals_m11->temp_file, "/tmp/junk");
	#endif
	#ifdef WINDOWS_m11
	GetTempPathA(FULL_FILE_NAME_BYTES_m11, globals_m11->temp_dir);
	sprintf_m11(globals_m11->temp_file, "%sjunk", globals_m11->temp_dir);
	#endif
	globals_m11->level_header_flags = LH_NO_FLAGS_m11;
	globals_m11->mmap_block_bytes = GLOBALS_MMAP_BLOCK_BYTES_NO_ENTRY_m11;

#ifdef AT_DEBUG_m11  // do this at end, because message() will load UTF8 tables
	printf_m11("%s(): %sAllocation tracking debug mode enabled%s\n", __FUNCTION__, TC_GREEN_m11, TC_RESET_m11);
#endif

	pthread_mutex_unlock_m11(&globals_m11_mutex);

	return(TRUE_m11);
}


//***********************************************************************//
//**************************  END MED GLOBALS  **************************//
//***********************************************************************//


TERN_m11	initialize_medlib_m11(TERN_m11 check_structure_alignments, TERN_m11 initialize_all_tables)
{
	TERN_m11			return_value = TRUE_m11;
	si1				command[FULL_FILE_NAME_BYTES_m11];

#ifdef FN_DEBUG_m11  // don't use MED print functions until UTF8 tables initialized
	printf_m11("%s()\n", __FUNCTION__);
#endif

	// set up globals
	if (globals_m11 == NULL) {
		if (initialize_globals_m11() == FALSE_m11) {
			printf_m11("%s(): error initializing globals\n", __FUNCTION__);
			exit_m11(-1);
		}
	}

#if defined FN_DEBUG_m11 || defined AT_DEBUG  // need UTF8 tables for message_m11()
	if (globals_m11->UTF8_offsets_table == NULL) {
		if (UTF8_initialize_tables_m11() == FALSE_m11)
			return_value = FALSE_m11;
	}
#endif
	
	// check cpu endianness
	if (get_cpu_endianness_m11() != LITTLE_ENDIAN_m11) {
		error_message_m11("%s(): Library only coded for little-endian machines currently\n", __FUNCTION__);
		exit_m11(-1);
	}
	
	// check "char" type
	if (check_char_type_m11() == FALSE_m11) {
		error_message_m11("%s(): Library only coded for 8-bit signed chars currently\n", __FUNCTION__);
		exit_m11(-1);
	}

	// check structure alignments
	if (check_structure_alignments == TRUE_m11)
		if (check_all_alignments_m11() == FALSE_m11)
			return_value = FALSE_m11;
	
	// seed random number generator
#if defined MACOS_m11 || defined LINUX_m11
	srandom((ui4) time(NULL));
#endif
#ifdef WINDOWS_m11
	srand((ui4) time(NULL));
#endif
	
#if defined WINDOWS_m11 && defined NEED_WIN_SOCKETS_m11
	// initialize Windows sockets DLL
	if (win_socket_startup_m11() == FALSE_m11)
		return_value = FALSE_m11;
#endif
	
#if defined WINDOWS_m11 && !defined MATLAB_m11
	// initialize Windows terminal
	if (win_initialize_terminal_m11() == FALSE_m11)
		return_value = FALSE_m11;
#endif
	
	if (initialize_all_tables == TRUE_m11) {
		
		if (globals_m11->CRC_table == NULL)
			if (CRC_initialize_tables_m11() == FALSE_m11)
				return_value = FALSE_m11;
		
		if (globals_m11->UTF8_offsets_table == NULL)
			if (UTF8_initialize_tables_m11() == FALSE_m11)
				return_value = FALSE_m11;
		
		if (globals_m11->AES_sbox_table == NULL)
			if (AES_initialize_tables_m11() == FALSE_m11)
				return_value = FALSE_m11;
		
		if (globals_m11->SHA_h0_table == NULL)
			if (SHA_initialize_tables_m11() == FALSE_m11)
				return_value = FALSE_m11;
		
		if (globals_m11->timezone_table == NULL)
			if (initialize_timezone_tables_m11() == FALSE_m11)
				return_value = FALSE_m11;
	}
	
	// clear any residual temp file
#if defined MACOS_m11 || defined LINUX_m11
	sprintf_m11(command, "rm -f %s", globals_m11->temp_file);
#endif
#ifdef WINDOWS_m11
	sprintf_m11(command, "del %s", globals_m11->temp_file);
#endif
	system_m11(command, TRUE_m11, __FUNCTION__, RETURN_ON_FAIL_m11 | SUPPRESS_OUTPUT_m11);

	return(return_value);
}


TIME_SLICE_m11	*initialize_time_slice_m11(TIME_SLICE_m11 *slice)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// NOTE: also initializes for frame numbers (via unions)
	
	if (slice == NULL)  // caller responsible for freeing
		slice = (TIME_SLICE_m11 *) malloc_m11(sizeof(TIME_SLICE_m11), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	slice->conditioned = FALSE_m11;
	slice->number_of_segments = UNKNOWN_m11;  // number_of_segments == 0
	slice->start_time = slice->end_time = UUTC_NO_ENTRY_m11;
	slice->start_sample_number = slice->end_sample_number = SAMPLE_NUMBER_NO_ENTRY_m11;
	slice->start_segment_number = slice->end_segment_number = SEGMENT_NUMBER_NO_ENTRY_m11;

	return(slice);
}


TERN_m11	initialize_timezone_tables_m11(void)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif

	pthread_mutex_lock_m11(&globals_m11->TZ_mutex);
	
	// timezone table
	if (globals_m11->timezone_table == NULL) {
		globals_m11->timezone_table = (TIMEZONE_INFO_m11 *) calloc_m11((size_t) TZ_TABLE_ENTRIES_m11, sizeof(TIMEZONE_INFO_m11), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
		{
			TIMEZONE_INFO_m11 temp[TZ_TABLE_ENTRIES_m11] = TZ_TABLE_m11;
			memcpy(globals_m11->timezone_table, temp, TZ_TABLE_ENTRIES_m11 * sizeof(TIMEZONE_INFO_m11));
		}
	}

	// country aliases
	if (globals_m11->country_aliases_table == NULL) {
		globals_m11->country_aliases_table = (TIMEZONE_ALIAS_m11 *) calloc_m11((size_t)TZ_COUNTRY_ALIASES_ENTRIES_m11, sizeof(TIMEZONE_ALIAS_m11), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
		{
			TIMEZONE_ALIAS_m11 temp[TZ_COUNTRY_ALIASES_ENTRIES_m11] = TZ_COUNTRY_ALIASES_TABLE_m11;
			memcpy(globals_m11->country_aliases_table, temp, TZ_COUNTRY_ALIASES_ENTRIES_m11 * sizeof(TIMEZONE_ALIAS_m11));
		}
	}
	
	// country acronym aliases
	if (globals_m11->country_acronym_aliases_table == NULL) {
		globals_m11->country_acronym_aliases_table = (TIMEZONE_ALIAS_m11 *) calloc_m11((size_t)TZ_COUNTRY_ACRONYM_ALIASES_ENTRIES_m11, sizeof(TIMEZONE_ALIAS_m11), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
		{
			TIMEZONE_ALIAS_m11 temp[TZ_COUNTRY_ACRONYM_ALIASES_ENTRIES_m11] = TZ_COUNTRY_ACRONYM_ALIASES_TABLE_m11;
			memcpy(globals_m11->country_acronym_aliases_table, temp, TZ_COUNTRY_ACRONYM_ALIASES_ENTRIES_m11 * sizeof(TIMEZONE_ALIAS_m11));
		}
	}
	
	pthread_mutex_unlock_m11(&globals_m11->TZ_mutex);

	return(TRUE_m11);
}


void	initialize_universal_header_m11(FILE_PROCESSING_STRUCT_m11 *fps, ui4 type_code, TERN_m11 generate_file_UID, TERN_m11 originating_file)
{
	UNIVERSAL_HEADER_m11	*uh;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	uh = fps->universal_header;
	
	uh->header_CRC = uh->body_CRC = CRC_START_VALUE_m11;
	uh->segment_number = UNIVERSAL_HEADER_SEGMENT_NUMBER_NO_ENTRY_m11;
	uh->type_code = type_code;
	uh->MED_version_major = MED_FORMAT_VERSION_MAJOR_m11;
	uh->MED_version_minor = MED_FORMAT_VERSION_MINOR_m11;
	uh->byte_order_code = LITTLE_ENDIAN_m11;
	uh->session_start_time = UUTC_NO_ENTRY_m11;
	uh->segment_start_time = UUTC_NO_ENTRY_m11;
	uh->segment_end_time = UUTC_NO_ENTRY_m11;
	uh->number_of_entries = 0;
	uh->maximum_entry_size = 0;
	
	if (generate_file_UID == TRUE_m11)
		generate_UID_m11(&uh->file_UID);
	if (originating_file == TRUE_m11)
		uh->provenance_UID = uh->file_UID;
	
	return;
}


si8	items_for_bytes_m11(FILE_PROCESSING_STRUCT_m11 *fps, si8 *number_of_bytes)
{
	si8				items, bytes;
	ui4				entry_size;
	UNIVERSAL_HEADER_m11		*uh;
	RECORD_HEADER_m11		*rh;
	CMP_BLOCK_FIXED_HEADER_m11	*bh;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	items = 0;
	uh = fps->universal_header;
	switch (uh->type_code) {
		case TIME_SERIES_INDICES_FILE_TYPE_CODE_m11:
		case VIDEO_INDICES_FILE_TYPE_CODE_m11:
		case RECORD_INDICES_FILE_TYPE_CODE_m11:
			items = *number_of_bytes / INDEX_BYTES_m11;
			uh->maximum_entry_size = INDEX_BYTES_m11;
			return(items);
		case TIME_SERIES_METADATA_FILE_TYPE_CODE_m11:
		case VIDEO_METADATA_FILE_TYPE_CODE_m11:
			items = 1;
			*number_of_bytes = METADATA_BYTES_m11;
			uh->maximum_entry_size = METADATA_BYTES_m11;
			return(items);
	}
	
	switch (uh->type_code) {
		case TIME_SERIES_DATA_FILE_TYPE_CODE_m11:
			bh = fps->parameters.cps->block_header;
			for (bytes = 0; bytes < *number_of_bytes; ++items) {
				entry_size = bh->total_block_bytes;
				if (uh->maximum_entry_size < entry_size)  // caller should've done this, but just in case
					uh->maximum_entry_size = entry_size;
				bytes += (si8) entry_size;
				bh = (CMP_BLOCK_FIXED_HEADER_m11 *) ((ui1 *) bh + (si8) entry_size);
			}
			break;
		case RECORD_DATA_FILE_TYPE_CODE_m11:
			rh = (RECORD_HEADER_m11 *) fps->record_data;
			for (bytes = 0; bytes < *number_of_bytes; ++items) {
				entry_size = rh->total_record_bytes;
				if (uh->maximum_entry_size < entry_size)  // caller should've done this, but just in case
					uh->maximum_entry_size = entry_size;
				bytes += (si8) entry_size;
				rh = (RECORD_HEADER_m11 *) ((ui1 *) rh + (si8) entry_size);
			}
	}
	*number_of_bytes = bytes;
	
	return(items);
}


void	lh_set_directives_m11(si1 *full_file_name, ui8 lh_flags, TERN_m11 *mmap_flag, TERN_m11 *close_flag, si8 *number_of_items)
{
	TERN_m11	read_flag, read_full_flag, tmp_mmap_flag;
	ui4		level_code, type_code;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if ((lh_flags & (LH_ALL_READ_FLAGS_MASK_m11 | LH_ALL_MEM_MAP_FLAGS_MASK_m11)) == 0)
		return;
	
	level_code = get_level_m11(full_file_name, &type_code);
	
	// all record types
	if (type_code == RECORD_DATA_FILE_TYPE_CODE_m11 || type_code == RECORD_INDICES_FILE_TYPE_CODE_m11) {
		read_flag = read_full_flag = tmp_mmap_flag = FALSE_m11;
		switch (level_code) {
			case LH_SESSION_m11:
				if (lh_flags & LH_READ_SLICE_SESSION_RECORDS_m11)
					read_flag = TRUE_m11;
				else if (lh_flags & LH_READ_FULL_SESSION_RECORDS_m11)
					read_full_flag = TRUE_m11;
				if (lh_flags & LH_MEM_MAP_SESSION_RECORDS_m11)
					tmp_mmap_flag = TRUE_m11;
				break;
			case LH_SEGMENTED_SESS_RECS_m11:
				if (lh_flags & LH_READ_SLICE_SEGMENTED_SESS_RECS_m11)
					read_flag = TRUE_m11;
				else if (lh_flags & LH_READ_FULL_SEGMENTED_SESS_RECS_m11)
					read_full_flag = TRUE_m11;
				if (lh_flags & LH_MEM_MAP_SEGMENTED_SESS_RECS_m11)
					tmp_mmap_flag = TRUE_m11;
				break;
			case LH_TIME_SERIES_CHANNEL_m11:
			case LH_VIDEO_CHANNEL_m11:
				if (lh_flags & LH_READ_SLICE_CHANNEL_RECORDS_m11)
					read_flag = TRUE_m11;
				else if (lh_flags & LH_READ_FULL_CHANNEL_RECORDS_m11)
					read_full_flag = TRUE_m11;
				if (lh_flags & LH_MEM_MAP_CHANNEL_RECORDS_m11)
					tmp_mmap_flag = TRUE_m11;
				break;
			case LH_TIME_SERIES_SEGMENT_m11:
			case LH_VIDEO_SEGMENT_m11:
				if (lh_flags & LH_READ_SLICE_SEGMENT_RECORDS_m11)
					read_flag = TRUE_m11;
				else if (lh_flags & LH_READ_FULL_SEGMENT_RECORDS_m11)
					read_full_flag = TRUE_m11;
				if (lh_flags & LH_MEM_MAP_SEGMENT_RECORDS_m11)
					tmp_mmap_flag = TRUE_m11;
				break;
		}
		if (type_code == RECORD_INDICES_FILE_TYPE_CODE_m11) {
			if (read_flag == TRUE_m11 || read_full_flag == TRUE_m11) {
				*number_of_items = FPS_FULL_FILE_m11;
				*close_flag = TRUE_m11;
				*mmap_flag = FALSE_m11;
			}
		} else if (type_code == RECORD_DATA_FILE_TYPE_CODE_m11) {
			if (read_flag == TRUE_m11) {
				*number_of_items = FPS_UNIVERSAL_HEADER_ONLY_m11;
				*close_flag = FALSE_m11;
				if (tmp_mmap_flag == TRUE_m11)
					*mmap_flag = TRUE_m11;
				else
					*mmap_flag = FALSE_m11;
			} else if (read_full_flag == TRUE_m11) {
				*number_of_items = FPS_FULL_FILE_m11;
				*close_flag = TRUE_m11;
				*mmap_flag = FALSE_m11;
			}
		}
	}
	
	// segment data types
	else if (level_code == LH_TIME_SERIES_SEGMENT_m11 || level_code == LH_VIDEO_SEGMENT_m11) {
		switch (type_code) {
			case TIME_SERIES_INDICES_FILE_TYPE_CODE_m11:
			case VIDEO_INDICES_FILE_TYPE_CODE_m11:
				if (lh_flags & LH_READ_SEGMENT_DATA_MASK_m11) {
					*number_of_items = FPS_FULL_FILE_m11;
					*close_flag = TRUE_m11;
					*mmap_flag = FALSE_m11;
				}
				break;
			case TIME_SERIES_DATA_FILE_TYPE_CODE_m11:
				if (lh_flags & LH_READ_SLICE_SEGMENT_DATA_m11) {
					*number_of_items = FPS_UNIVERSAL_HEADER_ONLY_m11;
					*close_flag = FALSE_m11;
					if (lh_flags & LH_MEM_MAP_SEGMENT_DATA_m11)
						*mmap_flag = TRUE_m11;
					else
						*mmap_flag = FALSE_m11;
				} else if (lh_flags & LH_READ_FULL_SEGMENT_DATA_m11) {
					*number_of_items = FPS_FULL_FILE_m11;
					*close_flag = TRUE_m11;
					*mmap_flag = FALSE_m11;
				}
				break;
			case TIME_SERIES_METADATA_FILE_TYPE_CODE_m11:
			case VIDEO_METADATA_FILE_TYPE_CODE_m11:
				if (lh_flags & LH_READ_SEGMENT_DATA_MASK_m11) {
					*number_of_items = FPS_FULL_FILE_m11;
					*close_flag = TRUE_m11;
					*mmap_flag = FALSE_m11;
				}
				break;
		}
	}
	
	return;
}
		    
		    
si1	*MED_type_string_from_code_m11(ui4 code)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// could have written this differently, since the string bytes are the code bytes, just NULL terminated
	// but would've required accounting for endianness, and handling thread safety
	
	switch (code) {
		case NO_FILE_TYPE_CODE_m11:
			return NO_FILE_TYPE_STRING_m11;
		case SESSION_DIRECTORY_TYPE_CODE_m11:
			return SESSION_DIRECTORY_TYPE_STRING_m11;
		case TIME_SERIES_SEGMENT_DIRECTORY_TYPE_CODE_m11:
			return TIME_SERIES_SEGMENT_DIRECTORY_TYPE_STRING_m11;
		case VIDEO_SEGMENT_DIRECTORY_TYPE_CODE_m11:
			return VIDEO_SEGMENT_DIRECTORY_TYPE_STRING_m11;
		case RECORD_DIRECTORY_TYPE_CODE_m11:
			return RECORD_DIRECTORY_TYPE_STRING_m11;
		case RECORD_DATA_FILE_TYPE_CODE_m11:
			return RECORD_DATA_FILE_TYPE_STRING_m11;
		case RECORD_INDICES_FILE_TYPE_CODE_m11:
			return RECORD_INDICES_FILE_TYPE_STRING_m11;
		case VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m11:
			return VIDEO_CHANNEL_DIRECTORY_TYPE_STRING_m11;
		case VIDEO_METADATA_FILE_TYPE_CODE_m11:
			return VIDEO_METADATA_FILE_TYPE_STRING_m11;
		case VIDEO_INDICES_FILE_TYPE_CODE_m11:
			return VIDEO_INDICES_FILE_TYPE_STRING_m11;
		case TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m11:
			return TIME_SERIES_CHANNEL_DIRECTORY_TYPE_STRING_m11;
		case TIME_SERIES_METADATA_FILE_TYPE_CODE_m11:
			return TIME_SERIES_METADATA_FILE_TYPE_STRING_m11;
		case TIME_SERIES_DATA_FILE_TYPE_CODE_m11:
			return TIME_SERIES_DATA_FILE_TYPE_STRING_m11;
		case TIME_SERIES_INDICES_FILE_TYPE_CODE_m11:
			return TIME_SERIES_INDICES_FILE_TYPE_STRING_m11;
	}
	
	warning_message_m11("%s(): 0x%08x is not a recognized MED file type code\n", __FUNCTION__, code);

	return(NULL);
}


ui4     MED_type_code_from_string_m11(si1 *string)
{
	ui4     code;
	si4     len;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (string == NULL) {
		warning_message_m11("%s(): string is NULL\n", __FUNCTION__);
		return(NO_FILE_TYPE_CODE_m11);
	}
	
	len = strlen(string);
	if (len < 5) {
		if (len != 4)
			return(NO_FILE_TYPE_CODE_m11);
	}
	else {
		string += (len - 5);
		if (*string++ != '.')
			return(NO_FILE_TYPE_CODE_m11);
	}
	
	memcpy((void *) &code, (void *) string, sizeof(ui4));
	
	switch (code) {
		case NO_FILE_TYPE_CODE_m11:
		case SESSION_DIRECTORY_TYPE_CODE_m11:
		case TIME_SERIES_SEGMENT_DIRECTORY_TYPE_CODE_m11:
		case VIDEO_SEGMENT_DIRECTORY_TYPE_CODE_m11:
		case RECORD_DIRECTORY_TYPE_CODE_m11:
		case RECORD_DATA_FILE_TYPE_CODE_m11:
		case RECORD_INDICES_FILE_TYPE_CODE_m11:
		case VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m11:
		case VIDEO_METADATA_FILE_TYPE_CODE_m11:
		case VIDEO_INDICES_FILE_TYPE_CODE_m11:
		case TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m11:
		case TIME_SERIES_METADATA_FILE_TYPE_CODE_m11:
		case TIME_SERIES_DATA_FILE_TYPE_CODE_m11:
		case TIME_SERIES_INDICES_FILE_TYPE_CODE_m11:
			return code;
	}
	
	warning_message_m11("%s(): \"%s\" is not a recognized MED file type\n", __FUNCTION__, string);
	
	return(NO_FILE_TYPE_CODE_m11);
}


TERN_m11        merge_metadata_m11(FILE_PROCESSING_STRUCT_m11 *md_fps_1, FILE_PROCESSING_STRUCT_m11 *md_fps_2, FILE_PROCESSING_STRUCT_m11 *merged_md_fps)
{
	ui4                                     type_code;
	METADATA_SECTION_1_m11			*md1_1, *md1_2, *md1_m;
	TIME_SERIES_METADATA_SECTION_2_m11	*tmd2_1, *tmd2_2, *tmd2_m;
	VIDEO_METADATA_SECTION_2_m11		*vmd2_1, *vmd2_2, *vmd2_m;
	METADATA_SECTION_3_m11			*md3_1, *md3_2, *md3_m;
	TERN_m11                                equal;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// if merged_md_fps == NULL, comparison results will be placed in md_fps_1->metadata
	// returns TRUE_m11 if md_fps_1->metadata == md_fps_2->metadata, FALSE_m11 otherwise

	// decrypt if needed
	md1_1 = &md_fps_1->metadata->section_1;
	if (md1_1->section_2_encryption_level > NO_ENCRYPTION_m11 || md1_1->section_3_encryption_level > NO_ENCRYPTION_m11)
		decrypt_metadata_m11(md_fps_1);
	md1_2 = &md_fps_2->metadata->section_1;
	if (md1_2->section_2_encryption_level > NO_ENCRYPTION_m11 || md1_2->section_3_encryption_level > NO_ENCRYPTION_m11)
		decrypt_metadata_m11(md_fps_2);
	
	// setup
	if (merged_md_fps == NULL)
		merged_md_fps = md_fps_1;
	else
		memcpy(merged_md_fps->metadata, md_fps_1->metadata, METADATA_BYTES_m11);
	md1_m = &merged_md_fps->metadata->section_1;
	
	type_code = md_fps_1->universal_header->type_code;
	if (type_code != md_fps_2->universal_header->type_code) {
		error_message_m11("%s(): mismatched type codes\n", __FUNCTION__);
		return(UNKNOWN_m11);
	}
	
	switch (type_code) {
		case TIME_SERIES_METADATA_FILE_TYPE_CODE_m11:
		case VIDEO_METADATA_FILE_TYPE_CODE_m11:
			break;
		default:
			error_message_m11("%s(): unrecognized type code 0x%x\n", __FUNCTION__, type_code);
			return(UNKNOWN_m11);
	}
	equal = TRUE_m11;
	
	// section 1
	if (memcmp(md1_1->level_1_password_hint, md1_2->level_1_password_hint, PASSWORD_HINT_BYTES_m11)) {
		memset(md1_m->level_1_password_hint, 0, PASSWORD_HINT_BYTES_m11); equal = FALSE_m11;
	}
	if (memcmp(md1_1->level_2_password_hint, md1_2->level_2_password_hint, PASSWORD_HINT_BYTES_m11)) {
		memset(md1_m->level_2_password_hint, 0, PASSWORD_HINT_BYTES_m11); equal = FALSE_m11;
	}
	if (md1_1->section_2_encryption_level != md1_2->section_2_encryption_level) {
		md1_m->section_2_encryption_level = ENCRYPTION_LEVEL_NO_ENTRY_m11; equal = FALSE_m11;
	}
	if (md1_1->section_3_encryption_level != md1_2->section_3_encryption_level) {
		md1_m->section_3_encryption_level = ENCRYPTION_LEVEL_NO_ENTRY_m11; equal = FALSE_m11;
	}
	if (memcmp(md1_1->protected_region, md1_2->protected_region, METADATA_SECTION_1_PROTECTED_REGION_BYTES_m11)) {
		memset(md1_m->protected_region, 0, METADATA_SECTION_1_PROTECTED_REGION_BYTES_m11); equal = FALSE_m11;
	}
	if (memcmp(md1_1->discretionary_region, md1_2->discretionary_region, METADATA_SECTION_1_DISCRETIONARY_REGION_BYTES_m11)) {
		memset(md1_m->discretionary_region, 0, METADATA_SECTION_1_DISCRETIONARY_REGION_BYTES_m11); equal = FALSE_m11;
	}
	
	// section 2: times series channel
	if (type_code == TIME_SERIES_METADATA_FILE_TYPE_CODE_m11) {
		tmd2_1 = &md_fps_1->metadata->time_series_section_2;
		tmd2_2 = &md_fps_2->metadata->time_series_section_2;
		tmd2_m = &merged_md_fps->metadata->time_series_section_2;
		if (memcmp(tmd2_1->session_description, tmd2_2->session_description, METADATA_SESSION_DESCRIPTION_BYTES_m11)) {
			memset(tmd2_m->session_description, 0, METADATA_SESSION_DESCRIPTION_BYTES_m11); equal = FALSE_m11;
		}
		if (memcmp(tmd2_1->channel_description, tmd2_2->channel_description, METADATA_CHANNEL_DESCRIPTION_BYTES_m11)) {
			memset(tmd2_m->channel_description, 0, METADATA_CHANNEL_DESCRIPTION_BYTES_m11); equal = FALSE_m11;
		}
		if (memcmp(tmd2_1->segment_description, tmd2_2->segment_description, METADATA_SEGMENT_DESCRIPTION_BYTES_m11)) {
			memset(tmd2_m->segment_description, 0, METADATA_SEGMENT_DESCRIPTION_BYTES_m11); equal = FALSE_m11;
		}
		if (memcmp(tmd2_1->equipment_description, tmd2_2->equipment_description, METADATA_EQUIPMENT_DESCRIPTION_BYTES_m11)) {
			memset(tmd2_m->equipment_description, 0, METADATA_EQUIPMENT_DESCRIPTION_BYTES_m11); equal = FALSE_m11;
		}
		if (tmd2_1->acquisition_channel_number != tmd2_2->acquisition_channel_number) {
			tmd2_m->acquisition_channel_number = METADATA_ACQUISITION_CHANNEL_NUMBER_NO_ENTRY_m11; equal = FALSE_m11;
		}
		if (memcmp(tmd2_1->reference_description, tmd2_2->reference_description, TIME_SERIES_METADATA_REFERENCE_DESCRIPTION_BYTES_m11)) {
			memset(tmd2_m->reference_description, 0, TIME_SERIES_METADATA_REFERENCE_DESCRIPTION_BYTES_m11); equal = FALSE_m11;
		}
		if (tmd2_1->sampling_frequency != tmd2_2->sampling_frequency) {
			if (tmd2_1->sampling_frequency == FREQUENCY_NO_ENTRY_m11 || tmd2_2->sampling_frequency == FREQUENCY_NO_ENTRY_m11)
				tmd2_m->sampling_frequency = FREQUENCY_NO_ENTRY_m11; // no entry supercedes variable frequency
			else
				tmd2_m->sampling_frequency = FREQUENCY_VARIABLE_m11;
			equal = FALSE_m11;
		}
		if (tmd2_1->low_frequency_filter_setting != tmd2_2->low_frequency_filter_setting) {
			tmd2_m->low_frequency_filter_setting = TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m11; equal = FALSE_m11;
		}
		if (tmd2_1->high_frequency_filter_setting != tmd2_2->high_frequency_filter_setting) {
			tmd2_m->high_frequency_filter_setting = TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m11; equal = FALSE_m11;
		}
		if (tmd2_1->notch_filter_frequency_setting != tmd2_2->notch_filter_frequency_setting) {
			tmd2_m->notch_filter_frequency_setting = TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m11; equal = FALSE_m11;
		}
		if (tmd2_1->AC_line_frequency != tmd2_2->AC_line_frequency) {
			tmd2_m->AC_line_frequency = TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m11; equal = FALSE_m11;
		}
		if (tmd2_1->amplitude_units_conversion_factor != tmd2_2->amplitude_units_conversion_factor) {
			tmd2_m->amplitude_units_conversion_factor = TIME_SERIES_METADATA_AMPLITUDE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m11; equal = FALSE_m11;
		}
		if (memcmp(tmd2_1->amplitude_units_description, tmd2_2->amplitude_units_description, TIME_SERIES_METADATA_AMPLITUDE_UNITS_DESCRIPTION_BYTES_m11)) {
			memset(tmd2_m->amplitude_units_description, 0, TIME_SERIES_METADATA_AMPLITUDE_UNITS_DESCRIPTION_BYTES_m11); equal = FALSE_m11;
		}
		if (tmd2_1->time_base_units_conversion_factor != tmd2_2->time_base_units_conversion_factor) {
			tmd2_m->time_base_units_conversion_factor = TIME_SERIES_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m11; equal = FALSE_m11;
		}
		if (memcmp(tmd2_1->time_base_units_description, tmd2_2->time_base_units_description, TIME_SERIES_METADATA_TIME_BASE_UNITS_DESCRIPTION_BYTES_m11)) {
			memset(tmd2_m->time_base_units_description, 0, TIME_SERIES_METADATA_TIME_BASE_UNITS_DESCRIPTION_BYTES_m11); equal = FALSE_m11;
		}
		if (tmd2_1->absolute_start_sample_number > tmd2_2->absolute_start_sample_number) {
			tmd2_m->absolute_start_sample_number = tmd2_2->absolute_start_sample_number; equal = FALSE_m11;
		}
		if (tmd2_1->number_of_samples < tmd2_2->number_of_samples) {
			tmd2_m->number_of_samples = tmd2_2->number_of_samples; equal = FALSE_m11;
		}
		if (tmd2_1->number_of_blocks < tmd2_2->number_of_blocks) {
			tmd2_m->number_of_blocks = tmd2_2->number_of_blocks; equal = FALSE_m11;
		}
		if (tmd2_1->maximum_block_bytes < tmd2_2->maximum_block_bytes) {
			tmd2_m->maximum_block_bytes = tmd2_2->maximum_block_bytes; equal = FALSE_m11;
		}
		if (tmd2_1->maximum_block_samples < tmd2_2->maximum_block_samples) {
			tmd2_m->maximum_block_samples = tmd2_2->maximum_block_samples; equal = FALSE_m11;
		}
		if (tmd2_1->maximum_block_keysample_bytes < tmd2_2->maximum_block_keysample_bytes) {
			tmd2_m->maximum_block_keysample_bytes = tmd2_2->maximum_block_keysample_bytes; equal = FALSE_m11;
		}
		if (tmd2_1->maximum_block_duration < tmd2_2->maximum_block_duration) {
			tmd2_m->maximum_block_duration = tmd2_2->maximum_block_duration; equal = FALSE_m11;
		}
		if (tmd2_1->number_of_discontinuities < tmd2_2->number_of_discontinuities) {
			tmd2_m->number_of_discontinuities = tmd2_2->number_of_discontinuities; equal = FALSE_m11;
		}
		if (tmd2_1->maximum_contiguous_blocks < tmd2_2->maximum_contiguous_blocks) {
			tmd2_m->maximum_contiguous_blocks = tmd2_2->maximum_contiguous_blocks; equal = FALSE_m11;
		}
		if (tmd2_1->maximum_contiguous_block_bytes < tmd2_2->maximum_contiguous_block_bytes) {
			tmd2_m->maximum_contiguous_block_bytes = tmd2_2->maximum_contiguous_block_bytes; equal = FALSE_m11;
		}
		if (tmd2_1->maximum_contiguous_samples < tmd2_2->maximum_contiguous_samples) {
			tmd2_m->maximum_contiguous_samples = tmd2_2->maximum_contiguous_samples; equal = FALSE_m11;
		}
		if (memcmp(tmd2_1->protected_region, tmd2_2->protected_region, TIME_SERIES_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m11)) {
			memset(tmd2_m->protected_region, 0, TIME_SERIES_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m11); equal = FALSE_m11;
		}
		if (memcmp(tmd2_1->discretionary_region, tmd2_2->discretionary_region, TIME_SERIES_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m11)) {
			memset(tmd2_m->discretionary_region, 0, TIME_SERIES_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m11); equal = FALSE_m11;
		}
		// section 2: times series channel
	}
	else if (type_code == VIDEO_METADATA_FILE_TYPE_CODE_m11) {
		vmd2_1 = &md_fps_1->metadata->video_section_2;
		vmd2_2 = &md_fps_2->metadata->video_section_2;
		vmd2_m = &merged_md_fps->metadata->video_section_2;
		if (memcmp(vmd2_1->session_description, vmd2_2->session_description, METADATA_SESSION_DESCRIPTION_BYTES_m11)) {
			memset(vmd2_m->session_description, 0, METADATA_SESSION_DESCRIPTION_BYTES_m11); equal = FALSE_m11;
		}
		if (memcmp(vmd2_1->channel_description, vmd2_2->channel_description, METADATA_CHANNEL_DESCRIPTION_BYTES_m11)) {
			memset(vmd2_m->channel_description, 0, METADATA_CHANNEL_DESCRIPTION_BYTES_m11); equal = FALSE_m11;
		}
		if (memcmp(vmd2_1->segment_description, vmd2_2->segment_description, METADATA_SEGMENT_DESCRIPTION_BYTES_m11)) {
			memset(vmd2_m->segment_description, 0, METADATA_SEGMENT_DESCRIPTION_BYTES_m11); equal = FALSE_m11;
		}
		if (memcmp(vmd2_1->equipment_description, vmd2_2->equipment_description, METADATA_EQUIPMENT_DESCRIPTION_BYTES_m11)) {
			memset(vmd2_m->equipment_description, 0, METADATA_EQUIPMENT_DESCRIPTION_BYTES_m11); equal = FALSE_m11;
		}
		if (vmd2_1->acquisition_channel_number != vmd2_2->acquisition_channel_number) {
			vmd2_m->acquisition_channel_number = METADATA_ACQUISITION_CHANNEL_NUMBER_NO_ENTRY_m11; equal = FALSE_m11;
		}
		if (vmd2_1->time_base_units_conversion_factor != vmd2_2->time_base_units_conversion_factor) {
			vmd2_m->time_base_units_conversion_factor = VIDEO_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m11; equal = FALSE_m11;
		}
		if (memcmp(vmd2_1->time_base_units_description, vmd2_2->time_base_units_description, VIDEO_METADATA_TIME_BASE_UNITS_DESCRIPTION_BYTES_m11)) {
			memset(vmd2_m->time_base_units_description, 0, VIDEO_METADATA_TIME_BASE_UNITS_DESCRIPTION_BYTES_m11); equal = FALSE_m11;
		}
		if (vmd2_1->absolute_start_frame_number > vmd2_2->absolute_start_frame_number) {
			vmd2_m->absolute_start_frame_number = vmd2_2->absolute_start_frame_number; equal = FALSE_m11;
		}
		if (vmd2_1->number_of_frames < vmd2_2->number_of_frames) {
			vmd2_m->number_of_frames = vmd2_2->number_of_frames; equal = FALSE_m11;
		}
		if (vmd2_1->frame_rate != vmd2_2->frame_rate) {
			vmd2_m->frame_rate = VIDEO_METADATA_FRAME_RATE_NO_ENTRY_m11; equal = FALSE_m11;
		}
		if (vmd2_1->number_of_clips < vmd2_2->number_of_clips) {
			vmd2_m->number_of_clips = vmd2_2->number_of_clips; equal = FALSE_m11;
		}
		if (vmd2_1->maximum_clip_bytes < vmd2_2->maximum_clip_bytes) {
			vmd2_m->maximum_clip_bytes = vmd2_2->maximum_clip_bytes; equal = FALSE_m11;
		}
		if (vmd2_1->maximum_clip_frames < vmd2_2->maximum_clip_frames) {
			vmd2_m->maximum_clip_frames = vmd2_2->maximum_clip_frames; equal = FALSE_m11;
		}
		if (vmd2_1->number_of_video_files < vmd2_2->number_of_video_files) {
			vmd2_m->number_of_video_files = vmd2_2->number_of_video_files; equal = FALSE_m11;
		}
		if (vmd2_1->maximum_clip_duration < vmd2_2->maximum_clip_duration) {
			vmd2_m->maximum_clip_duration = vmd2_2->maximum_clip_duration; equal = FALSE_m11;
		}
		if (vmd2_1->number_of_discontinuities < vmd2_2->number_of_discontinuities) {
			vmd2_m->number_of_discontinuities = vmd2_2->number_of_discontinuities; equal = FALSE_m11;
		}
		if (vmd2_1->maximum_contiguous_clips < vmd2_2->maximum_contiguous_clips) {
			vmd2_m->maximum_contiguous_clips = vmd2_2->maximum_contiguous_clips; equal = FALSE_m11;
		}
		if (vmd2_1->maximum_contiguous_clip_bytes < vmd2_2->maximum_contiguous_clip_bytes) {
			vmd2_m->maximum_contiguous_clip_bytes = vmd2_2->maximum_contiguous_clip_bytes; equal = FALSE_m11;
		}
		if (vmd2_1->maximum_contiguous_frames < vmd2_2->maximum_contiguous_frames) {
			vmd2_m->maximum_contiguous_frames = vmd2_2->maximum_contiguous_frames; equal = FALSE_m11;
		}
		if (vmd2_1->horizontal_pixels != vmd2_2->horizontal_pixels) {
			vmd2_m->horizontal_pixels = VIDEO_METADATA_HORIZONTAL_PIXELS_NO_ENTRY_m11; equal = FALSE_m11;
		}
		if (vmd2_1->vertical_pixels != vmd2_2->vertical_pixels) {
			vmd2_m->vertical_pixels = VIDEO_METADATA_VERTICAL_PIXELS_NO_ENTRY_m11; equal = FALSE_m11;
		}
		if (memcmp(vmd2_1->video_format, vmd2_2->video_format, VIDEO_METADATA_VIDEO_FORMAT_BYTES_m11)) {
			memset(vmd2_1->video_format, 0, VIDEO_METADATA_VIDEO_FORMAT_BYTES_m11); equal = FALSE_m11;
		}
		if (memcmp(vmd2_1->protected_region, vmd2_2->protected_region, VIDEO_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m11)) {
			memset(vmd2_m->protected_region, 0, VIDEO_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m11); equal = FALSE_m11;
		}
		if (memcmp(vmd2_1->discretionary_region, vmd2_2->discretionary_region, VIDEO_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m11)) {
			memset(vmd2_m->discretionary_region, 0, VIDEO_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m11); equal = FALSE_m11;
		}
	}
	
	// section 3
	md3_1 = &md_fps_1->metadata->section_3;
	md3_2 = &md_fps_2->metadata->section_3;
	md3_m = &merged_md_fps->metadata->section_3;
	if (md3_1->recording_time_offset != md3_2->recording_time_offset) {
		md3_m->recording_time_offset = UUTC_NO_ENTRY_m11; equal = FALSE_m11;
	}
	if (md3_1->daylight_time_start_code.value != md3_2->daylight_time_start_code.value) {
		md3_m->daylight_time_start_code.value = DTCC_VALUE_NO_ENTRY_m11; equal = FALSE_m11;
	}
	if (md3_1->daylight_time_end_code.value != md3_2->daylight_time_end_code.value) {
		md3_m->daylight_time_end_code.value = DTCC_VALUE_NO_ENTRY_m11; equal = FALSE_m11;
	}
	if (memcmp(md3_1->standard_timezone_acronym, md3_2->standard_timezone_acronym, TIMEZONE_ACRONYM_BYTES_m11)) {
		memset(md3_m->standard_timezone_acronym, 0, TIMEZONE_ACRONYM_BYTES_m11); equal = FALSE_m11;
	}
	if (memcmp(md3_1->standard_timezone_string, md3_2->standard_timezone_string, TIMEZONE_STRING_BYTES_m11)) {
		memset(md3_m->standard_timezone_string, 0, TIMEZONE_STRING_BYTES_m11); equal = FALSE_m11;
	}
	if (memcmp(md3_1->daylight_timezone_acronym, md3_2->daylight_timezone_acronym, TIMEZONE_ACRONYM_BYTES_m11)) {
		memset(md3_m->daylight_timezone_acronym, 0, TIMEZONE_ACRONYM_BYTES_m11); equal = FALSE_m11;
	}
	if (memcmp(md3_1->daylight_timezone_string, md3_2->daylight_timezone_string, TIMEZONE_STRING_BYTES_m11)) {
		memset(md3_m->daylight_timezone_string, 0, TIMEZONE_STRING_BYTES_m11); equal = FALSE_m11;
	}
	if (memcmp(md3_1->subject_name_1, md3_2->subject_name_1, METADATA_SUBJECT_NAME_BYTES_m11)) {
		memset(md3_m->subject_name_1, 0, METADATA_SUBJECT_NAME_BYTES_m11); equal = FALSE_m11;
	}
	if (memcmp(md3_1->subject_name_2, md3_2->subject_name_2, METADATA_SUBJECT_NAME_BYTES_m11)) {
		memset(md3_m->subject_name_2, 0, METADATA_SUBJECT_NAME_BYTES_m11); equal = FALSE_m11;
	}
	if (memcmp(md3_1->subject_name_3, md3_2->subject_name_3, METADATA_SUBJECT_NAME_BYTES_m11)) {
		memset(md3_m->subject_name_3, 0, METADATA_SUBJECT_NAME_BYTES_m11); equal = FALSE_m11;
	}
	if (memcmp(md3_1->subject_ID, md3_2->subject_ID, METADATA_SUBJECT_ID_BYTES_m11)) {
		memset(md3_m->subject_ID, 0, METADATA_SUBJECT_ID_BYTES_m11); equal = FALSE_m11;
	}
	if (memcmp(md3_1->recording_country, md3_2->recording_country, METADATA_RECORDING_LOCATION_BYTES_m11)) {
		memset(md3_m->recording_country, 0, METADATA_SUBJECT_ID_BYTES_m11); equal = FALSE_m11;
	}
	if (memcmp(md3_1->recording_territory, md3_2->recording_territory, METADATA_RECORDING_LOCATION_BYTES_m11)) {
		memset(md3_m->recording_territory, 0, METADATA_SUBJECT_ID_BYTES_m11); equal = FALSE_m11;
	}
	if (memcmp(md3_1->recording_locality, md3_2->recording_locality, METADATA_RECORDING_LOCATION_BYTES_m11)) {
		memset(md3_m->recording_locality, 0, METADATA_RECORDING_LOCATION_BYTES_m11); equal = FALSE_m11;
	}
	if (memcmp(md3_1->recording_institution, md3_2->recording_institution, METADATA_RECORDING_LOCATION_BYTES_m11)) {
		memset(md3_m->recording_institution, 0, METADATA_RECORDING_LOCATION_BYTES_m11); equal = FALSE_m11;
	}
	if (memcmp(md3_1->geotag_format, md3_2->geotag_format, METADATA_GEOTAG_FORMAT_BYTES_m11)) {
		memset(md3_m->geotag_format, 0, METADATA_GEOTAG_FORMAT_BYTES_m11); equal = FALSE_m11;
	}
	if (memcmp(md3_1->geotag_data, md3_2->geotag_data, METADATA_GEOTAG_DATA_BYTES_m11)) {
		memset(md3_m->geotag_data, 0, METADATA_GEOTAG_DATA_BYTES_m11); equal = FALSE_m11;
	}
	if (md3_1->standard_UTC_offset != md3_2->standard_UTC_offset) {
		md3_m->standard_UTC_offset = STANDARD_UTC_OFFSET_NO_ENTRY_m11; equal = FALSE_m11;
	}
	if (memcmp(md3_1->protected_region, md3_2->protected_region, METADATA_SECTION_3_PROTECTED_REGION_BYTES_m11)) {
		memset(md3_m->protected_region, 0, METADATA_SECTION_3_PROTECTED_REGION_BYTES_m11); equal = FALSE_m11;
	}
	if (memcmp(md3_1->discretionary_region, md3_2->discretionary_region, METADATA_SECTION_3_DISCRETIONARY_REGION_BYTES_m11)) {
		memset(md3_m->discretionary_region, 0, METADATA_SECTION_3_DISCRETIONARY_REGION_BYTES_m11); equal = FALSE_m11;
	}
	
	if (globals_m11->verbose == TRUE_m11) {
		switch (type_code) {
			case TIME_SERIES_METADATA_FILE_TYPE_CODE_m11:
				printf_m11("------------ Merged Time Series Metadata --------------\n");
				break;
			case VIDEO_METADATA_FILE_TYPE_CODE_m11:
				printf_m11("--------------- Merged Video Metadata -----------------\n");
				break;
		}
		show_metadata_m11(NULL, merged_md_fps->metadata, type_code);
	}
	
	return(equal);
}


TERN_m11        merge_universal_headers_m11(FILE_PROCESSING_STRUCT_m11 *fps_1, FILE_PROCESSING_STRUCT_m11 * fps_2, FILE_PROCESSING_STRUCT_m11 * merged_fps)
{
	UNIVERSAL_HEADER_m11	*uh_1, *uh_2, *merged_uh;
	TERN_m11                equal = TRUE_m11;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// if merged_fps == NULL, comparison results will be placed in fps_1->universal_header
	// returns TRUE_m11 if fps_1->universal_header == fps_2->universal_header, FALSE_m11 otherwise
	
	if (merged_fps == NULL)
		merged_fps = fps_1;
	else
		memcpy(merged_fps->universal_header, fps_1->universal_header, UNIVERSAL_HEADER_BYTES_m11);
	
	equal = TRUE_m11;
	uh_1 = fps_1->universal_header;
	uh_2 = fps_2->universal_header;
	merged_uh = merged_fps->universal_header;
	
	merged_uh->header_CRC = CRC_NO_ENTRY_m11; // CRCs not compared / merged
	merged_uh->body_CRC = CRC_NO_ENTRY_m11; // CRCs not compared / merged
	if (uh_1->type_code != uh_2->type_code) {
		// special case: merging metadata & record data universal headers for ephemeral data
		if ((uh_1->type_code == TIME_SERIES_METADATA_FILE_TYPE_CODE_m11 || uh_1->type_code == VIDEO_METADATA_FILE_TYPE_CODE_m11) && uh_2->type_code == RECORD_DATA_FILE_TYPE_CODE_m11) {
			merged_uh->type_code = uh_1->type_code;
		} else if ((uh_2->type_code == TIME_SERIES_METADATA_FILE_TYPE_CODE_m11 || uh_2->type_code == VIDEO_METADATA_FILE_TYPE_CODE_m11) && uh_1->type_code == RECORD_DATA_FILE_TYPE_CODE_m11) {
			merged_uh->type_code = uh_2->type_code;
		} else {
			merged_uh->type_code = NO_TYPE_CODE_m11; equal = FALSE_m11;
		}
	}
	if (uh_1->MED_version_major != uh_2->MED_version_major) {
		merged_uh->MED_version_major = UNIVERSAL_HEADER_MED_VERSION_MAJOR_NO_ENTRY_m11; equal = FALSE_m11;
	}
	if (uh_1->MED_version_minor != uh_2->MED_version_minor) {
		merged_uh->MED_version_minor = UNIVERSAL_HEADER_MED_VERSION_MINOR_NO_ENTRY_m11; equal = FALSE_m11;
	}
	if (uh_1->byte_order_code != uh_2->byte_order_code) {
		merged_uh->byte_order_code = UNIVERSAL_HEADER_BYTE_ORDER_CODE_NO_ENTRY_m11; equal = FALSE_m11;
	}
	if (uh_1->session_start_time == UUTC_NO_ENTRY_m11) {
		if (uh_2->session_start_time == UUTC_NO_ENTRY_m11)
			merged_uh->session_start_time = UUTC_NO_ENTRY_m11;
		else
			merged_uh->session_start_time = uh_2->session_start_time;
	} else if (uh_2->session_start_time == UUTC_NO_ENTRY_m11) {
		merged_uh->session_start_time = uh_1->session_start_time;
	} else if (uh_1->session_start_time > uh_2->session_start_time) {
		merged_uh->session_start_time = uh_2->session_start_time; equal = FALSE_m11;
	}
	if (uh_1->segment_start_time == UUTC_NO_ENTRY_m11) {
		if (uh_2->segment_start_time == UUTC_NO_ENTRY_m11)
			merged_uh->segment_start_time = UUTC_NO_ENTRY_m11;
		else
			merged_uh->segment_start_time = uh_2->segment_start_time;
	} else if (uh_2->segment_start_time == UUTC_NO_ENTRY_m11) {
		merged_uh->segment_start_time = uh_1->segment_start_time;
	} else if (uh_1->segment_start_time > uh_2->segment_start_time) {
		merged_uh->segment_start_time = uh_2->segment_start_time; equal = FALSE_m11;
	}
	if (uh_1->segment_end_time == UUTC_NO_ENTRY_m11) {
		if (uh_2->segment_end_time == UUTC_NO_ENTRY_m11)
			merged_uh->segment_end_time = UUTC_NO_ENTRY_m11;
		else
			merged_uh->segment_end_time = uh_2->segment_end_time;
	} else if (uh_2->segment_end_time == UUTC_NO_ENTRY_m11) {
		merged_uh->segment_end_time = uh_1->segment_start_time;
	} else if (uh_1->segment_end_time < uh_2->segment_end_time) {
		merged_uh->segment_end_time = uh_2->segment_end_time; equal = FALSE_m11;
	}
	if (uh_1->number_of_entries < uh_2->number_of_entries) {
		merged_uh->number_of_entries = uh_2->number_of_entries; equal = FALSE_m11;
	}
	if (uh_1->maximum_entry_size < uh_2->maximum_entry_size) {
		merged_uh->maximum_entry_size = uh_2->maximum_entry_size; equal = FALSE_m11;
	}
	if (uh_1->segment_number != uh_2->segment_number) {
		merged_uh->segment_number = UNIVERSAL_HEADER_SEGMENT_NUMBER_NO_ENTRY_m11; equal = FALSE_m11;
	}
	if (memcmp(uh_1->session_name, uh_2->session_name, BASE_FILE_NAME_BYTES_m11)) {
		memset(merged_uh->session_name, 0, BASE_FILE_NAME_BYTES_m11); equal = FALSE_m11;
	}
	if (memcmp(uh_1->channel_name, uh_2->channel_name, BASE_FILE_NAME_BYTES_m11)) {
		memset(merged_uh->channel_name, 0, BASE_FILE_NAME_BYTES_m11); equal = FALSE_m11;
	}
	if (memcmp(uh_1->anonymized_subject_ID, uh_2->anonymized_subject_ID, BASE_FILE_NAME_BYTES_m11)) {
		memset(merged_uh->anonymized_subject_ID, 0, UNIVERSAL_HEADER_ANONYMIZED_SUBJECT_ID_BYTES_m11); equal = FALSE_m11;
	}
	if (uh_1->session_UID != uh_2->session_UID) {
		merged_uh->session_UID = UID_NO_ENTRY_m11; equal = FALSE_m11;
	}
	if (uh_1->channel_UID != uh_2->channel_UID) {
		merged_uh->channel_UID = UID_NO_ENTRY_m11; equal = FALSE_m11;
	}
	if (uh_1->segment_UID != uh_2->segment_UID) {
		merged_uh->segment_UID = UID_NO_ENTRY_m11; equal = FALSE_m11;
	}
	if (uh_1->file_UID != uh_2->file_UID) {
		merged_uh->file_UID = UID_NO_ENTRY_m11; equal = FALSE_m11;
	}
	if (uh_1->provenance_UID != uh_2->provenance_UID) {
		merged_uh->provenance_UID = UID_NO_ENTRY_m11; equal = FALSE_m11;
	}
	if (memcmp(uh_1->level_1_password_validation_field, uh_2->level_1_password_validation_field, PASSWORD_VALIDATION_FIELD_BYTES_m11)) {
		memset(merged_uh->level_1_password_validation_field, 0, PASSWORD_VALIDATION_FIELD_BYTES_m11); equal = FALSE_m11;
	}
	if (memcmp(uh_1->level_2_password_validation_field, uh_2->level_2_password_validation_field, PASSWORD_VALIDATION_FIELD_BYTES_m11)) {
		memset(merged_uh->level_2_password_validation_field, 0, PASSWORD_VALIDATION_FIELD_BYTES_m11); equal = FALSE_m11;
	}
	if (memcmp(uh_1->level_3_password_validation_field, uh_2->level_3_password_validation_field, PASSWORD_VALIDATION_FIELD_BYTES_m11)) {
		memset(merged_uh->level_3_password_validation_field, 0, PASSWORD_VALIDATION_FIELD_BYTES_m11); equal = FALSE_m11;
	}
	if (memcmp(uh_1->protected_region, uh_2->protected_region, UNIVERSAL_HEADER_PROTECTED_REGION_BYTES_m11)) {
		memset(merged_uh->protected_region, 0, UNIVERSAL_HEADER_PROTECTED_REGION_BYTES_m11); equal = FALSE_m11;
	}
	if (memcmp(uh_1->discretionary_region, uh_2->discretionary_region, UNIVERSAL_HEADER_DISCRETIONARY_REGION_BYTES_m11)) {
		memset(merged_uh->discretionary_region, 0, UNIVERSAL_HEADER_DISCRETIONARY_REGION_BYTES_m11); equal = FALSE_m11;
	}
	
	return(equal);
}


void    message_m11(si1 *fmt, ...)
{
	va_list		args;
	
	
	// uncolored suppressible text to stdout
	if (!(globals_m11->behavior_on_fail & SUPPRESS_MESSAGE_OUTPUT_m11)) {
		va_start(args, fmt);
		UTF8_vprintf_m11(fmt, args);
		va_end(args);
		#ifndef MATLAB_m11
		fflush(stdout);
		#endif
	}
	
	return;
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void     nap_m11(si1 *nap_str)
{
	si1             *c;
	struct timespec nap;
	si8             num;
	
	
	// string format: <number>[<space>]<unit letter>
	// e.g. to sleep for 1 millisecond:
	// "1 millisecond" == "1millisecond" == "1 ms" == "1ms" == "1 m" == "1m"

	if (nap_str == NULL) {
		error_message_m11("%s(): NULL input string => not napping", __FUNCTION__, nap_str);
		return;
	}
	
	if (*nap_str == 0) {
		error_message_m11("%s(): no input string => not napping", __FUNCTION__, nap_str);
		return;
	}

	c = nap_str;
	num = *c++ - '0';
	while (*c >= '0' && *c <= '9' && *c) {
		num *= 10;
		num += *c++ - '0';
	}
	
	// optional space
	if (*c == 32)
		++c;

	// units: ns, us, ms, sec
	switch(*c) {
		case 'n':  // nanoseconds
			nap.tv_sec = 0;
			nap.tv_nsec = num;
			break;
		case 'u':  // microseconds
			nap.tv_sec = 0;
			nap.tv_nsec = num * (ui8) 1e3;
			break;
		case 'm':  // milliseconds
			nap.tv_sec = 0;
			nap.tv_nsec = num * (ui8) 1e6;
			break;
		case 's':  // seconds
			nap.tv_sec = num;
			nap.tv_nsec = 0;
			break;
		default:
			error_message_m11("%s(): \"%s\" is not a valid input string => not napping", __FUNCTION__, nap_str);
			return;
	}
	
	// overflow
	if (nap.tv_nsec >= (ui8) 1e9) {
		nap.tv_sec = nap.tv_nsec / (ui8) 1e9;
		nap.tv_nsec -= (nap.tv_sec * (ui8) 1e9);
	}
	
	// sleep
#if defined MACOS_m11 || defined LINUX_m11
	nanosleep(&nap, NULL);
#endif
#ifdef WINDOWS_m11  // limited to millisecond resolution (can do better, but requires much more code)
	ui8	ms;

	ms = (ui8) nap.tv_sec * (ui8) 1000;
	ms += (ui8) round((sf8) nap.tv_nsec / (sf8) 1e6);
	if (ms == 0) {
		ms = 1;  // Windows limited to 1 ms rseolution
	} else if (ms > 0x7FFFFFFF) {
		warning_message_m11("%s(): millisecond overflow\n", __FUNCTION__);
		ms = 0x7FFFFFFF;
	}
	Sleep((si4) ms);
#endif

	return;
}


si1	*numerical_fixed_width_string_m11(si1 *string, si4 string_bytes, si4 number)
{
	si4	native_numerical_length, temp;
	si1	*c;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// string bytes does not include terminal zero
	
	if (string == NULL)
		string = (si1 *) calloc_m11((size_t)(string_bytes + 1), sizeof(si1), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	
	native_numerical_length = 0;
	temp = number;
	while (temp) {
		temp /= 10;
		++native_numerical_length;
	}
	if (number <= 0)
		++native_numerical_length;
	
	c = string;
	temp = string_bytes - native_numerical_length;
	if (temp < 0)
		warning_message_m11("%s(): required digits exceed string length\n", __FUNCTION__);
	while (temp--)
		*c++ = '0';
	
	sprintf_m11(c, "%d", number);
	
	return(string);
}


CHANNEL_m11	*open_channel_m11(CHANNEL_m11 *chan, TIME_SLICE_m11 *slice, si1 *chan_path, ui8 flags, si1 *password)
{
	TERN_m11		free_channel;
	si1			tmp_str[FULL_FILE_NAME_BYTES_m11], num_str[FILE_NUMBERING_DIGITS_m11 + 1];
	si4			i, j, mapped_segs, seg_idx, n_segs, null_segment_cnt;
	SEGMENT_m11		*seg;
	UNIVERSAL_HEADER_m11	*uh;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// allocate channel
	free_channel = FALSE_m11;
	if (chan == NULL) {
		chan = (CHANNEL_m11 *) calloc_m11((size_t) 1, sizeof(CHANNEL_m11), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
		free_channel = TRUE_m11;
	} else if (chan->flags & LH_OPEN_m11) {
		return(chan);
	}

	// set basic info (path, name, type, flags)
	if (chan_path != NULL)
		if (*chan_path)
			chan->type_code = generate_MED_path_components_m11(chan_path, chan->path, chan->name);
	if (chan->type_code == NO_TYPE_CODE_m11 && *chan->path)
		chan->type_code = generate_MED_path_components_m11(chan->path, NULL, chan->name);
	if (chan->type_code != LH_TIME_SERIES_CHANNEL_m11 && chan->type_code != LH_VIDEO_CHANNEL_m11) {
		if (chan->type_code == LH_TIME_SERIES_SEGMENT_m11 || chan->type_code == LH_VIDEO_SEGMENT_m11) {  // segment passed: don't think it will be used this way, but never know
			extract_path_parts_m11(chan->path, chan->path, NULL, NULL);
			chan->type_code = generate_MED_path_components_m11(chan->path, NULL, chan->name);
		} else {
			if (free_channel == TRUE_m11)
				free_channel_m11(chan, TRUE_m11);
			error_message_m11("%s(): indeterminate channel type\n", __FUNCTION__);
			return(NULL);
		}
	}
	if (flags == LH_NO_FLAGS_m11) {
		flags = chan->flags;  // use existing channel flags, if none passed
		if (flags == LH_NO_FLAGS_m11)
			flags = globals_m11->level_header_flags;  // use global flags, if no channel flags
	}
	chan->flags = flags | (LH_OPEN_m11 | LH_CHANNEL_ACTIVE_m11);
	
	// set up time & generate password data (note do this before slice is conditioned)
	if (globals_m11->password_data.processed == 0) {
		if (set_time_and_password_data_m11(password, chan->path, NULL, NULL) == FALSE_m11) {
			if (free_channel == TRUE_m11)
				free_channel_m11(chan, TRUE_m11);
			return(NULL);
		}
	}
	
	// process time slice (passed slice is not modified)
	if (slice == NULL) {
		if (all_zeros_m11((ui1 *) &chan->time_slice, (si4) sizeof(TIME_SLICE_m11)) == TRUE_m11)
			initialize_time_slice_m11(&chan->time_slice);  // read whole channel
	} else {  // passed slice supersedes structure slice
		chan->time_slice = *slice;
	}
	slice = &chan->time_slice;
	if (slice->conditioned == FALSE_m11)
		condition_time_slice_m11(slice);
			
	// get segment range
	if (slice->number_of_segments == UNKNOWN_m11) {
		if (get_segment_range_m11((LEVEL_HEADER_m11 *) chan, slice) == 0) {
			if (free_channel == TRUE_m11)
				free_channel_m11(chan, TRUE_m11);
			return(NULL);
		}
	}
	
	// open segments
	seg_idx = get_segment_index_m11(slice->start_segment_number);
	if (seg_idx == FALSE_m11) {
		if (free_channel == TRUE_m11)
			free_channel_m11(chan, TRUE_m11);
		return(NULL);
	}
	n_segs = slice->number_of_segments;
	mapped_segs = globals_m11->number_of_mapped_segments;

	chan->segments = (SEGMENT_m11 **) calloc_m11((size_t) mapped_segs, sizeof(SEGMENT_m11 *), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);  // map segments
	null_segment_cnt = 0;
	for (i = slice->start_segment_number, j = seg_idx; i <= slice->end_segment_number; ++i, ++j) {
		seg = chan->segments[j];
		if (seg == NULL) {
			numerical_fixed_width_string_m11(num_str, FILE_NUMBERING_DIGITS_m11, i);
			if (chan->type_code == LH_TIME_SERIES_CHANNEL_m11)
				sprintf_m11(tmp_str, "%s/%s_s%s.%s", chan->path, chan->name, num_str, TIME_SERIES_SEGMENT_DIRECTORY_TYPE_STRING_m11);
			else if (chan->type_code == LH_VIDEO_CHANNEL_m11)
				sprintf_m11(tmp_str, "%s/%s_s%s.%s", chan->path, chan->name, num_str, VIDEO_SEGMENT_DIRECTORY_TYPE_STRING_m11);
			if (file_exists_m11(tmp_str) == DIR_EXISTS_m11)  // not every segment may be present
				seg = chan->segments[j] = open_segment_m11(NULL, slice, tmp_str, (flags & ~LH_OPEN_m11), password);
		} else {
			seg = open_segment_m11(seg, slice, NULL, LH_NO_FLAGS_m11, NULL);  // use existing segment flags
		}
		if (seg == NULL)
			++null_segment_cnt;
		else
			seg->parent = (void *) chan;
	}

	// channel records
	if (chan->flags & LH_READ_CHANNEL_RECORDS_MASK_m11) {
		sprintf_m11(tmp_str, "%s/%s.%s", chan->path, chan->name, RECORD_INDICES_FILE_TYPE_STRING_m11);
		if (file_exists_m11(tmp_str) == FILE_EXISTS_m11)
			chan->record_indices_fps = read_file_m11(chan->record_indices_fps, tmp_str, 0, 0, 0, (LEVEL_HEADER_m11 *) chan, NULL, USE_GLOBAL_BEHAVIOR_m11);
		sprintf_m11(tmp_str, "%s/%s.%s", chan->path, chan->name, RECORD_DATA_FILE_TYPE_STRING_m11);
		if (file_exists_m11(tmp_str) == FILE_EXISTS_m11) {
			if (chan->flags & LH_READ_FULL_CHANNEL_RECORDS_m11)
				chan->record_data_fps = read_file_m11(chan->record_data_fps, tmp_str, 0, 0, 0, (LEVEL_HEADER_m11 *) chan, NULL, USE_GLOBAL_BEHAVIOR_m11);
			else  // just read in data universal header & leave open
				chan->record_data_fps = read_file_m11(chan->record_data_fps, tmp_str, 0, 0, FPS_UNIVERSAL_HEADER_ONLY_m11, (LEVEL_HEADER_m11 *) chan, NULL, USE_GLOBAL_BEHAVIOR_m11);
		}
	}
	
	// empty slice
	if (null_segment_cnt == n_segs) {
		slice->number_of_segments = EMPTY_SLICE_m11;
		if (free_channel == TRUE_m11)
			free_channel_m11(chan, TRUE_m11);
		return(NULL);
	}

	// update slice
	for (i = 0, j = seg_idx; i < n_segs; ++i, ++j) {
		seg = chan->segments[j];
		if (seg != NULL)
			break;
	}
	slice->start_time = seg->time_slice.start_time;
	slice->start_sample_number = seg->time_slice.start_sample_number;
	slice->start_segment_number = seg->time_slice.start_segment_number;
	for (++i, ++j; i < n_segs; ++i, ++j) {
		if (chan->segments[j] != NULL)
			seg = chan->segments[j];
	}
	slice->end_time = seg->time_slice.end_time;
	slice->end_sample_number = seg->time_slice.end_sample_number;
	slice->end_segment_number = seg->time_slice.end_segment_number;
	slice->number_of_segments = TIME_SLICE_SEGMENT_COUNT_m11(slice);

	// ephemeral data
	if (chan->flags & LH_GENERATE_EPHEMERAL_DATA_m11) {
		if (chan->metadata_fps != NULL)
			FPS_free_processing_struct_m11(chan->metadata_fps, TRUE_m11);
		for (i = 0, j = seg_idx; i < n_segs; ++i, ++j) {
			seg = chan->segments[j];
			if (seg != NULL)
				break;
		}
		if (chan->type_code == LH_TIME_SERIES_CHANNEL_m11) {
			sprintf_m11(tmp_str, "%s/%s.%s", chan->path, chan->name, TIME_SERIES_METADATA_FILE_TYPE_STRING_m11);
			chan->metadata_fps = FPS_allocate_processing_struct_m11(NULL, tmp_str, TIME_SERIES_METADATA_FILE_TYPE_CODE_m11, METADATA_BYTES_m11, (LEVEL_HEADER_m11 *) chan, seg->metadata_fps, METADATA_BYTES_m11);
		} else if (chan->type_code == LH_VIDEO_CHANNEL_m11) {
			sprintf_m11(tmp_str, "%s/%s.%s", chan->path, chan->name, VIDEO_METADATA_FILE_TYPE_STRING_m11);
			chan->metadata_fps = FPS_allocate_processing_struct_m11(NULL, tmp_str, VIDEO_METADATA_FILE_TYPE_CODE_m11, METADATA_BYTES_m11, (LEVEL_HEADER_m11 *) chan, seg->metadata_fps, METADATA_BYTES_m11);
		}
		// merge segments
		for (i++, j++; i < n_segs; ++i, ++j) {
			if (chan->segments[j] == NULL)
				continue;
			seg = chan->segments[j];
			merge_universal_headers_m11(chan->metadata_fps, seg->metadata_fps, NULL);
			merge_metadata_m11(chan->metadata_fps, seg->metadata_fps, NULL);
			if (seg->record_indices_fps != NULL && seg->record_data_fps != NULL)  // record data, not record indices universal header is merged in ephemeral data
				merge_universal_headers_m11(chan->metadata_fps, seg->record_data_fps, NULL);
			seg->flags &= ~LH_UPDATE_EPHEMERAL_DATA_m11;  // clear segment flag
		}
		// merge channel records
		if (chan->record_indices_fps != NULL && chan->record_data_fps != NULL)  // record data, not record indices universal header is merged in ephemeral data
			merge_universal_headers_m11(chan->metadata_fps, chan->record_data_fps, NULL);
		// fix channel ephemeral universal headers (from merge functions)
		uh = chan->metadata_fps->universal_header;
		if (chan->type_code == LH_TIME_SERIES_CHANNEL_m11)
			uh->type_code = TIME_SERIES_METADATA_FILE_TYPE_CODE_m11;
		else if (chan->type_code == LH_VIDEO_CHANNEL_m11)
			uh->type_code = VIDEO_METADATA_FILE_TYPE_CODE_m11;
		uh->segment_number = UNIVERSAL_HEADER_CHANNEL_LEVEL_CODE_m11;
		uh->session_UID = globals_m11->session_UID;
		uh->channel_UID = seg->metadata_fps->universal_header->channel_UID;;
		uh->segment_UID = UID_NO_ENTRY_m11;
		chan->metadata_fps->parameters.fd = FPS_FD_EPHEMERAL_m11;
		chan->flags |= LH_UPDATE_EPHEMERAL_DATA_m11;
	}
	
	chan->last_access_time = current_uutc_m11();

	return(chan);
}


SEGMENT_m11	*open_segment_m11(SEGMENT_m11 *seg, TIME_SLICE_m11 *slice, si1 *seg_path, ui8 flags, si1 *password)
{
	TERN_m11	free_segment;
	si1		tmp_str[FULL_FILE_NAME_BYTES_m11];
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// allocate segment
	free_segment = FALSE_m11;
	if (seg == NULL) {
		seg = (SEGMENT_m11 *) calloc_m11((size_t) 1, sizeof(SEGMENT_m11), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
		free_segment = TRUE_m11;
	} else if (seg->flags & LH_OPEN_m11) {
		return(seg);
	}
	
	// set basic info (path, name, type, flags)
	if (seg_path != NULL) {
		seg->type_code = generate_MED_path_components_m11(seg_path, seg->path, seg->name);
	} else {
		if (file_exists_m11(seg_path) == DOES_NOT_EXIST_m11) {
			if (free_segment == TRUE_m11)
				free_segment_m11(seg, TRUE_m11);
			warning_message_m11("%s(): segment does not exist\n", __FUNCTION__);
			return(NULL);
		}
		seg->type_code = generate_MED_path_components_m11(seg->path, NULL, seg->name);
	}
	if (seg->type_code != LH_TIME_SERIES_SEGMENT_m11 && seg->type_code != LH_VIDEO_SEGMENT_m11) {
		if (free_segment == TRUE_m11)
			free_segment_m11(seg, TRUE_m11);
		error_message_m11("%s(): indeterminate segment type\n", __FUNCTION__);
		return(NULL);
	}
	if (flags == LH_NO_FLAGS_m11) {
		flags = seg->flags;  // use existing segment flags, if none passed
		if (flags == LH_NO_FLAGS_m11)
			flags = globals_m11->level_header_flags;  // use global flags, if no segment flags
	}
	seg->flags = flags | LH_OPEN_m11;

	// set up time & generate password data (note do this before slice is conditioned)
	if (globals_m11->password_data.processed == 0) {
		if (set_time_and_password_data_m11(password, seg->path, NULL, NULL) == FALSE_m11) {
			if (free_segment == TRUE_m11)
				free_segment_m11(seg, TRUE_m11);
			return(NULL);
		}
	}
	
	// process time slice (passed slice is not modified)
	if (slice == NULL) {
		if (all_zeros_m11((ui1 *) &seg->time_slice, (si4) sizeof(TIME_SLICE_m11)) == TRUE_m11)
			initialize_time_slice_m11(&seg->time_slice);  // read whole segment
	} else {  // passed slice supersedes structure slice
		seg->time_slice = *slice;  // passed slice is not modified
	}
	slice = &seg->time_slice;
	if (slice->conditioned == FALSE_m11)
		condition_time_slice_m11(slice);
		
	// metadata
	if (seg->flags & (LH_READ_SEGMENT_DATA_MASK_m11 | LH_READ_SEGMENT_METADATA_m11 | LH_GENERATE_EPHEMERAL_DATA_m11)) {
		if (seg->type_code == LH_TIME_SERIES_SEGMENT_m11)
			sprintf_m11(tmp_str, "%s/%s.%s", seg->path, seg->name, TIME_SERIES_METADATA_FILE_TYPE_STRING_m11);
		else // seg->type_code == LH_VIDEO_SEGMENT_m11
			sprintf_m11(tmp_str, "%s/%s.%s", seg->path, seg->name, VIDEO_METADATA_FILE_TYPE_STRING_m11);
		if (file_exists_m11(tmp_str) == FILE_EXISTS_m11)
			seg->metadata_fps = read_file_m11(NULL, tmp_str, 0, 0, 0, (LEVEL_HEADER_m11 *) seg, NULL, USE_GLOBAL_BEHAVIOR_m11);
	}
	
	// segment data
	if (seg->flags & LH_READ_SEGMENT_DATA_MASK_m11) {

		// indices
		if (seg->type_code == LH_TIME_SERIES_SEGMENT_m11)
			sprintf_m11(tmp_str, "%s/%s.%s", seg->path, seg->name, TIME_SERIES_INDICES_FILE_TYPE_STRING_m11);
		else // seg->type_code == LH_VIDEO_SEGMENT_m11
			sprintf_m11(tmp_str, "%s/%s.%s", seg->path, seg->name, VIDEO_INDICES_FILE_TYPE_STRING_m11);
		if (file_exists_m11(tmp_str) == FILE_EXISTS_m11)  // note seg->video_indices_fps is the same pointer, so works for either
			seg->time_series_indices_fps = read_file_m11(NULL, tmp_str, 0, 0, 0, (LEVEL_HEADER_m11 *) seg, NULL, USE_GLOBAL_BEHAVIOR_m11);
		
		// data (time series only)
		if (seg->type_code == LH_TIME_SERIES_SEGMENT_m11) {
			sprintf_m11(tmp_str, "%s/%s.%s", seg->path, seg->name, TIME_SERIES_DATA_FILE_TYPE_STRING_m11);
			if (file_exists_m11(tmp_str) == FILE_EXISTS_m11) {
				if (seg->flags & LH_READ_FULL_SEGMENT_DATA_m11)
					seg->time_series_data_fps = read_file_m11(NULL, tmp_str, 0, 0, 0, (LEVEL_HEADER_m11 *) seg, NULL, USE_GLOBAL_BEHAVIOR_m11);
				else
					seg->time_series_data_fps = read_file_m11(NULL, tmp_str, 0, 0, FPS_UNIVERSAL_HEADER_ONLY_m11, (LEVEL_HEADER_m11 *) seg, NULL, USE_GLOBAL_BEHAVIOR_m11);
			}
		}
	}

	// segment records
	if (seg->flags & LH_READ_SEGMENT_RECORDS_MASK_m11) {
		sprintf_m11(tmp_str, "%s/%s.%s", seg->path, seg->name, RECORD_INDICES_FILE_TYPE_STRING_m11);
		if (file_exists_m11(tmp_str) == FILE_EXISTS_m11) {
			seg->record_indices_fps = read_file_m11(seg->record_indices_fps, tmp_str, 0, 0, 0, (LEVEL_HEADER_m11 *) seg, NULL, USE_GLOBAL_BEHAVIOR_m11);
			seg->record_indices_fps->parent = (void *) seg;
		}
		sprintf_m11(tmp_str, "%s/%s.%s", seg->path, seg->name, RECORD_DATA_FILE_TYPE_STRING_m11);
		if (file_exists_m11(tmp_str) == FILE_EXISTS_m11) {
			if (seg->flags & LH_READ_FULL_SEGMENT_RECORDS_m11)
				seg->record_data_fps = read_file_m11(seg->record_data_fps, tmp_str, 0, 0, 0, (LEVEL_HEADER_m11 *) seg, NULL, USE_GLOBAL_BEHAVIOR_m11);
			else  // just read in data universal header & leave open
				seg->record_data_fps = read_file_m11(seg->record_data_fps, tmp_str, 0, 0, FPS_UNIVERSAL_HEADER_ONLY_m11, (LEVEL_HEADER_m11 *) seg, NULL, USE_GLOBAL_BEHAVIOR_m11);
		}
	}
	
	if (seg->flags & LH_GENERATE_EPHEMERAL_DATA_m11)
		seg->flags |= LH_UPDATE_EPHEMERAL_DATA_m11;
	seg->last_access_time = current_uutc_m11();

	return(seg);
}


SESSION_m11	*open_session_m11(SESSION_m11 *sess, TIME_SLICE_m11 *slice, void *file_list, si4 list_len, ui8 flags, si1 *password)
{
	TERN_m11			free_session, all_channels_selected;
	si1				*sess_dir, **chan_list, **ts_chan_list, **vid_chan_list, tmp_str[FULL_FILE_NAME_BYTES_m11], *tmp_str_ptr;
	si1				**full_ts_chan_list, **full_vid_chan_list, num_str[FILE_NUMBERING_DIGITS_m11 + 1];;
	ui4				type_code;
	si4				i, j, k, n_chans, n_ts_chans, n_vid_chans, all_ts_chans, all_vid_chans, mapped_segs, n_segs, seg_idx;
	si8				curr_time;
	CHANNEL_m11			*chan;
	UNIVERSAL_HEADER_m11		*uh;
	SEGMENTED_SESS_RECS_m11		*ssr;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// if file_list is a pointer to single string, make list_len zero to indicate a one dimention char array
	// if list_len > 0, assumed to be two dimensional array
	
	// allocate session
	free_session = FALSE_m11;
	if (sess == NULL) {
		sess = (SESSION_m11 *) calloc_m11((size_t) 1, sizeof(SESSION_m11), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
		free_session = TRUE_m11;
	} else if (sess->flags & LH_OPEN_m11) {
		return(sess);
	}
	
	sess->type_code = LH_SESSION_m11;
	if (flags == LH_NO_FLAGS_m11) {
		flags = sess->flags;  // use existing session flags, if none passed
		if (flags == LH_NO_FLAGS_m11)
			flags = globals_m11->level_header_flags;  // use global flags, if no session flags
	}
	sess->flags = flags | LH_OPEN_m11;
		
	// generate channel list
	all_channels_selected = FALSE_m11;
	sess_dir = NULL;
	chan_list = NULL;
	if (list_len == 0) {  // single string
		if (STR_contains_regex_m11((si1 *) file_list) == TRUE_m11) {  // regex string passed: make 1 element channel list, NULL session directory
			chan_list = (si1 **) &file_list;
			n_chans = 1;
		} else {  // directory passed: NULL channel list
			type_code = MED_type_code_from_string_m11((si1 *) file_list);
			switch (type_code) {
				case SESSION_DIRECTORY_TYPE_CODE_m11:  // session directory passed: NULL channel list
					all_channels_selected = TRUE_m11;
					sess_dir = (si1 *) file_list;
					chan_list = NULL;
					n_chans = 0;
					break;
				case TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m11:
				case VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m11:  // channel passed: make 1 element channel list, NULL session directory
					chan_list = (si1 **) &file_list;
					n_chans = 1;
					break;
				case TIME_SERIES_SEGMENT_DIRECTORY_TYPE_CODE_m11:  // I don't think segments will actually get passed to this function, but you never know
				case VIDEO_SEGMENT_DIRECTORY_TYPE_CODE_m11:  // segment passed: truncate to channel directory, make 1 element channel list, NULL session directory
					extract_path_parts_m11(tmp_str, (si1 *) file_list, NULL, NULL);
					tmp_str_ptr = tmp_str;  // copy pointer so can take address
					chan_list = &tmp_str_ptr;
					n_chans = 1;
					break;
				default:
					if (free_session == TRUE_m11)
						free_session_m11(sess, TRUE_m11);
					error_message_m11("%s(): invalid file list\n", __FUNCTION__);
					return(NULL);
			}
		}
	} else {  // channel list passed: NULL session directory
		chan_list = (si1 **) file_list;
		n_chans = list_len;
	}
	chan_list = generate_file_list_m11(chan_list, &n_chans, sess_dir, NULL, "?icd", GFL_FULL_PATH_m11);  // extension could be more specific ("[tv]icd") in MacOS & Linux, but not Windows

	if (n_chans == 0) {
		if (free_session == TRUE_m11)
			free_session_m11(sess, TRUE_m11);
		error_message_m11("%s(): no channels found\n", __FUNCTION__);
		return(NULL);
	}

	extract_path_parts_m11(chan_list[0], sess->path, NULL, NULL);
	type_code = generate_MED_path_components_m11(sess->path, NULL, sess->fs_name);
	sess->name = sess->fs_name;  // only name known at this point
	if (type_code != SESSION_DIRECTORY_TYPE_CODE_m11) {
		if (free_session == TRUE_m11)
			free_session_m11(sess, TRUE_m11);
		error_message_m11("%s(): channels must be in a MED session directory\n", __FUNCTION__);
		return(NULL);
	}
	
	// check that all files are MED channels in the same MED session directory
	// TO DO: check that they have the same session UIDs, & not require they be in the same directory
	n_ts_chans = n_vid_chans = 0;
	for (i = 0; i < n_chans; ++i) {
		extract_path_parts_m11(chan_list[i], tmp_str, NULL, NULL);
		if (strcmp(sess->path, tmp_str)) {
			if (free_session == TRUE_m11)
				free_session_m11(sess, TRUE_m11);
			error_message_m11("%s(): channels must all be in the same session directory\n", __FUNCTION__);
			return(NULL);
		}
		type_code = MED_type_code_from_string_m11(chan_list[i]);
		switch (type_code) {
			case TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m11:
				++n_ts_chans;
				break;
			case VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m11:
				++n_vid_chans;
				break;
			default:
				if (free_session == TRUE_m11)
					free_session_m11(sess, TRUE_m11);
				error_message_m11("%s(): channels must be MED channel directories\n", __FUNCTION__);
				return(NULL);
		}
	}
	
	// divide channel lists
	if (!(sess->flags & LH_INCLUDE_TIME_SERIES_CHANNELS_m11))
		n_ts_chans = 0;
	if (!(sess->flags & LH_INCLUDE_VIDEO_CHANNELS_m11))
		n_vid_chans = 0;
	if (n_ts_chans)
		ts_chan_list = (si1 **) calloc_2D_m11((size_t) n_ts_chans, FULL_FILE_NAME_BYTES_m11, sizeof(si1), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	if (n_vid_chans)
		vid_chan_list = (si1 **) calloc_2D_m11((size_t) n_vid_chans, FULL_FILE_NAME_BYTES_m11, sizeof(si1), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	for (i = j = k = 0; i < n_chans; ++i) {
		type_code = MED_type_code_from_string_m11(chan_list[i]);
		switch (type_code) {
			case TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m11:
				if (sess->flags & LH_INCLUDE_TIME_SERIES_CHANNELS_m11)
					strcpy(ts_chan_list[j++], chan_list[i]);
				break;
			case VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m11:
				if (sess->flags & LH_INCLUDE_VIDEO_CHANNELS_m11)
					strcpy(vid_chan_list[k++], chan_list[i]);
				break;
		}
	}
	free_m11((void *) chan_list, __FUNCTION__);

	// set up time series channels
	curr_time = current_uutc_m11();
	if (sess->flags & LH_MAP_ALL_TIME_SERIES_CHANNELS_m11 && all_channels_selected == FALSE_m11) {
		// get lists of all channels, regardless of what was passed in the list
		if (sess_dir == NULL) {
			if (n_ts_chans)
				extract_path_parts_m11(ts_chan_list[0], tmp_str, NULL, NULL);
			else
				extract_path_parts_m11(vid_chan_list[0], tmp_str, NULL, NULL);
			sess_dir = tmp_str;
		}
		full_ts_chan_list = generate_file_list_m11(NULL, &all_ts_chans, sess_dir, NULL, "ticd", GFL_FULL_PATH_m11);
		if (n_ts_chans) {
			sess->time_series_channels = (CHANNEL_m11 **) calloc_2D_m11((size_t) all_ts_chans, (size_t) 1, sizeof(CHANNEL_m11), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
			for (i = 0; i < all_ts_chans; ++i) {
				chan = sess->time_series_channels[i];
				chan->type_code = LH_TIME_SERIES_CHANNEL_m11;
				chan->flags = flags;
				chan->last_access_time = curr_time;
				generate_MED_path_components_m11(full_ts_chan_list[i], chan->path, chan->name);
			}
			// match passed list to full list to mark as active
			if (all_ts_chans == n_ts_chans) {
				for (i = 0; i < all_ts_chans; ++i) {
					chan = sess->time_series_channels[i];
					chan->flags |= LH_CHANNEL_ACTIVE_m11;
				}
			} else {  // lists are in alphabetical order
				for (i = j = 0; i < n_ts_chans; ++i) {
					for (; strcmp(ts_chan_list[i], full_ts_chan_list[j]); ++j);
					chan = sess->time_series_channels[j];
					chan->flags |= LH_CHANNEL_ACTIVE_m11;
				}
			}
			free_m11((void *) full_ts_chan_list, __FUNCTION__);
			free_m11((void *) ts_chan_list, __FUNCTION__);
			sess->number_of_time_series_channels = all_ts_chans;
		}
	} else if (n_ts_chans) {
		sess->time_series_channels = (CHANNEL_m11 **) calloc_2D_m11((size_t) n_ts_chans, (size_t) 1, sizeof(CHANNEL_m11), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
		for (i = 0; i < n_ts_chans; ++i) {
			chan = sess->time_series_channels[i];
			chan->type_code = LH_TIME_SERIES_CHANNEL_m11;
			chan->flags = flags | LH_CHANNEL_ACTIVE_m11;
			chan->last_access_time = curr_time;
			generate_MED_path_components_m11(ts_chan_list[i], chan->path, chan->name);
		}
		free_m11((void *) ts_chan_list, __FUNCTION__);
		sess->number_of_time_series_channels = n_ts_chans;
	}

	// set up video channels
	if (sess->flags & LH_MAP_ALL_VIDEO_CHANNELS_m11 && all_channels_selected == FALSE_m11) {
		// get lists of all channels, regardless of what was passed in the list
		if (sess_dir == NULL) {
			if (n_vid_chans)
				extract_path_parts_m11(vid_chan_list[0], tmp_str, NULL, NULL);
			else
				extract_path_parts_m11(ts_chan_list[0], tmp_str, NULL, NULL);
			sess_dir = tmp_str;
		}
		full_vid_chan_list = generate_file_list_m11(NULL, &all_vid_chans, sess_dir, NULL, "vicd", GFL_FULL_PATH_m11);
		if (n_vid_chans) {
			sess->video_channels = (CHANNEL_m11 **) calloc_2D_m11((size_t) all_vid_chans, (size_t) 1, sizeof(CHANNEL_m11), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
			for (i = 0; i < all_vid_chans; ++i) {
				chan = sess->video_channels[i];
				chan->type_code = LH_VIDEO_CHANNEL_m11;
				chan->flags = flags;
				chan->last_access_time = curr_time;
				generate_MED_path_components_m11(full_vid_chan_list[i], chan->path, chan->name);
			}
			// match passed list to full list to mark as active
			if (all_vid_chans == n_vid_chans) {
				for (i = 0; i < all_vid_chans; ++i) {
					chan = sess->video_channels[i];
					chan->flags |= LH_CHANNEL_ACTIVE_m11;
				}
			} else {  // lists are in alphbetical order
				for (i = j = 0; i < n_vid_chans; ++i) {
					for (; strcmp(vid_chan_list[i], full_vid_chan_list[j]); ++j);
					chan = sess->video_channels[j];
					chan->flags |= LH_CHANNEL_ACTIVE_m11;
				}
			}
			free_m11((void *) full_vid_chan_list, __FUNCTION__);
			free_m11((void *) vid_chan_list, __FUNCTION__);
			sess->number_of_video_channels = all_vid_chans;
		}
	} else if (n_vid_chans) {
		sess->video_channels = (CHANNEL_m11 **) calloc_2D_m11((size_t) n_vid_chans, (size_t) 1, sizeof(CHANNEL_m11), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
		for (i = 0; i < n_vid_chans; ++i) {
			chan = sess->video_channels[i];
			chan->type_code = LH_VIDEO_CHANNEL_m11;
			chan->flags = flags | LH_CHANNEL_ACTIVE_m11;
			chan->last_access_time = curr_time;
			generate_MED_path_components_m11(vid_chan_list[i], chan->path, chan->name);
		}
		free_m11((void *) vid_chan_list, __FUNCTION__);
		sess->number_of_video_channels = n_vid_chans;
	}

	// set up time & generate password data (note do this before slice is conditioned)
	if (globals_m11->password_data.processed == 0) {
		if (set_time_and_password_data_m11(password, sess->path, NULL, NULL) == FALSE_m11) {
			if (free_session == TRUE_m11)
				free_session_m11(sess, TRUE_m11);
			return(NULL);
		}
	}
	// user generated channel subsets (setting password also sets global session names)
	if (*globals_m11->uh_session_name) {
		strcpy(sess->uh_name, globals_m11->uh_session_name);
		sess->name = sess->uh_name;  // change name to the more generally useful version
	}

	// process time slice (passed slice is not modified)
	if (slice == NULL) {
		if (all_zeros_m11((ui1 *) &sess->time_slice, (si4) sizeof(TIME_SLICE_m11)) == TRUE_m11)
			initialize_time_slice_m11(&sess->time_slice);  // read whole session
	} else {  // passed slice supersedes structure slice
		sess->time_slice = *slice;  // passed slice is not modified
	}
	slice = &sess->time_slice;
	if (slice->conditioned == FALSE_m11)
		condition_time_slice_m11(slice);
	
	// set global sample/frame number reference channel
	change_reference_channel_m11(sess, NULL, globals_m11->reference_channel_name, DEFAULT_CHANNEL_m11);

	// get segment range
	n_segs = slice->number_of_segments;
	if (n_segs == UNKNOWN_m11) {
		if (get_segment_range_m11((LEVEL_HEADER_m11 *) sess, slice) == 0) {
			if (free_session == TRUE_m11)
				free_session_m11(sess, TRUE_m11);
			return(NULL);
		}
	}
	
	// open time series channels
	for (i = 0; i < sess->number_of_time_series_channels; ++i) {
		chan = sess->time_series_channels[i];
		if (chan->flags & LH_CHANNEL_ACTIVE_m11) {
			if (open_channel_m11(chan, slice, NULL, (flags & ~LH_OPEN_m11), password) == NULL) {
				if (free_session == TRUE_m11) {
					free_session_m11(sess, TRUE_m11);
				} else if (chan != NULL) {
					if (chan->time_slice.number_of_segments == EMPTY_SLICE_m11)
						sess->time_slice.number_of_segments = EMPTY_SLICE_m11;
				}
				return(NULL);
			}
			chan->parent = (void *) sess;
		}
	}

	// open video channels
	for (i = 0; i < sess->number_of_video_channels; ++i) {
		chan = sess->video_channels[i];
		if (chan->flags & LH_CHANNEL_ACTIVE_m11) {
			if (open_channel_m11(chan, slice, NULL, (flags & ~LH_OPEN_m11), password) == NULL) {
				if (free_session == TRUE_m11) {
					free_session_m11(sess, TRUE_m11);
				} else if (chan != NULL) {
					if (chan->time_slice.number_of_segments == EMPTY_SLICE_m11)
						sess->time_slice.number_of_segments = EMPTY_SLICE_m11;
				}
				return(NULL);
			}
			chan->parent = (void *) sess;
		}
	}

	// update session slice
	chan = globals_m11->reference_channel;
	if ((chan->flags & LH_CHANNEL_ACTIVE_m11) == 0)
		chan = get_active_channel_m11(sess, DEFAULT_CHANNEL_m11);
	slice->start_time = chan->time_slice.start_time;
	slice->end_time = chan->time_slice.end_time;
	slice->start_segment_number = chan->time_slice.start_segment_number;
	slice->end_segment_number = chan->time_slice.end_segment_number;
	slice->number_of_segments = TIME_SLICE_SEGMENT_COUNT_m11(slice);

	// update for variable frequencies on active channel set
	frequencies_vary_m11(sess);
	if (globals_m11->time_series_frequencies_vary == TRUE_m11 && globals_m11->reference_channel->type_code == TIME_SERIES_CHANNEL_TYPE_m11) {
		if (get_search_mode_m11(slice) == SAMPLE_SEARCH_m11) {
			slice->start_time = uutc_for_sample_number_m11((LEVEL_HEADER_m11 *) sess, slice->start_sample_number, FIND_START_m11);
			slice->end_time = uutc_for_sample_number_m11((LEVEL_HEADER_m11 *) sess, slice->end_sample_number, FIND_END_m11);
			slice->start_sample_number = slice->end_sample_number = SAMPLE_NUMBER_NO_ENTRY_m11;
		}
	} else if (globals_m11->video_frame_rates_vary == TRUE_m11 && globals_m11->reference_channel->type_code == VIDEO_CHANNEL_TYPE_m11) {
		if (get_search_mode_m11(slice) == FRAME_SEARCH_m11) {
			slice->start_time = uutc_for_frame_number_m11((LEVEL_HEADER_m11 *) sess, slice->start_frame_number, FIND_START_m11);
			slice->end_time = uutc_for_frame_number_m11((LEVEL_HEADER_m11 *) sess, slice->end_frame_number, FIND_END_m11);
			slice->start_frame_number = slice->end_frame_number = FRAME_NUMBER_NO_ENTRY_m11;
		}
	}

	// sort channels
	sort_channels_by_acq_num_m11(sess);
	
	// session records
	if (sess->flags & LH_READ_SESSION_RECORDS_MASK_m11) {
		sprintf_m11(tmp_str, "%s/%s.%s", sess->path, sess->name, RECORD_INDICES_FILE_TYPE_STRING_m11);
		if (file_exists_m11(tmp_str) == FILE_EXISTS_m11)
			sess->record_indices_fps = read_file_m11(sess->record_indices_fps, tmp_str, 0, 0, 0, (LEVEL_HEADER_m11 *) sess, NULL, USE_GLOBAL_BEHAVIOR_m11);
		sprintf_m11(tmp_str, "%s/%s.%s", sess->path, sess->name, RECORD_DATA_FILE_TYPE_STRING_m11);
		if (file_exists_m11(tmp_str) == FILE_EXISTS_m11) {
			if (sess->flags & LH_READ_FULL_SESSION_RECORDS_m11)
				sess->record_data_fps = read_file_m11(sess->record_data_fps, tmp_str, 0, 0, 0, (LEVEL_HEADER_m11 *) sess, NULL, USE_GLOBAL_BEHAVIOR_m11);
			else  // just read in data universal header & leave open
				sess->record_data_fps = read_file_m11(sess->record_data_fps, tmp_str, 0, 0, FPS_UNIVERSAL_HEADER_ONLY_m11, (LEVEL_HEADER_m11 *) sess, NULL, USE_GLOBAL_BEHAVIOR_m11);
		}
	}

	// segmented session records level
	ssr = NULL;
	if (sess->flags & LH_READ_SEGMENTED_SESS_RECS_MASK_m11) {
		sprintf_m11(tmp_str, "%s/%s.%s", sess->path, sess->name, RECORD_DIRECTORY_TYPE_STRING_m11);
		if (file_exists_m11(tmp_str) == DIR_EXISTS_m11) {
			ssr = sess->segmented_sess_recs = (SEGMENTED_SESS_RECS_m11 *) calloc_m11((size_t) 1, sizeof(SEGMENTED_SESS_RECS_m11), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
			strcpy_m11(ssr->path, tmp_str);
			strcpy_m11(ssr->name, sess->name);
			ssr->type_code = LH_SEGMENTED_SESS_RECS_m11;
			ssr->flags = sess->flags;
			ssr->parent = (void *) sess;
			mapped_segs = globals_m11->number_of_mapped_segments;
			ssr->record_data_fps = (FILE_PROCESSING_STRUCT_m11 **) calloc_m11((size_t) mapped_segs, sizeof(FILE_PROCESSING_STRUCT_m11 *), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
			ssr->record_indices_fps = (FILE_PROCESSING_STRUCT_m11 **) calloc_m11((size_t) mapped_segs, sizeof(FILE_PROCESSING_STRUCT_m11 *), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
			seg_idx = get_segment_index_m11(slice->start_segment_number);
			for (i = slice->start_segment_number, j = seg_idx; i <= slice->end_segment_number; ++i, ++j) {
				numerical_fixed_width_string_m11(num_str, FILE_NUMBERING_DIGITS_m11, i);
				sprintf_m11(tmp_str, "%s/%s_s%s.%s", ssr->path, ssr->name, num_str, RECORD_INDICES_FILE_TYPE_STRING_m11);
				if (file_exists_m11(tmp_str) == FILE_EXISTS_m11)
					ssr->record_indices_fps[j] = read_file_m11(ssr->record_indices_fps[j], tmp_str, 0, 0, 0, (LEVEL_HEADER_m11 *) ssr, NULL, USE_GLOBAL_BEHAVIOR_m11);
				sprintf_m11(tmp_str, "%s/%s_s%s.%s", ssr->path, ssr->name, num_str, RECORD_DATA_FILE_TYPE_STRING_m11);
				if (file_exists_m11(tmp_str) == FILE_EXISTS_m11) {
					if (ssr->flags & LH_READ_FULL_SEGMENTED_SESS_RECS_m11)
						ssr->record_data_fps[j] = read_file_m11(ssr->record_data_fps[j], tmp_str, 0, 0, 0, (LEVEL_HEADER_m11 *) ssr, NULL, USE_GLOBAL_BEHAVIOR_m11);
					else  // just read in data universal header & leave open
						ssr->record_data_fps[j] = read_file_m11(ssr->record_data_fps[j], tmp_str, 0, 0, FPS_UNIVERSAL_HEADER_ONLY_m11, (LEVEL_HEADER_m11 *) ssr, NULL, USE_GLOBAL_BEHAVIOR_m11);
				}
			}
		}
	}
	
	// ephemeral data
	if (sess->flags & LH_GENERATE_EPHEMERAL_DATA_m11) {
		if (sess->number_of_time_series_channels) {
			if (sess->time_series_metadata_fps != NULL)
				FPS_free_processing_struct_m11(sess->time_series_metadata_fps, TRUE_m11);
			sprintf_m11(tmp_str, "%s/%s_time_series.%s", sess->path, sess->name, TIME_SERIES_METADATA_FILE_TYPE_STRING_m11);
			chan = sess->time_series_channels[0];
			sess->time_series_metadata_fps = FPS_allocate_processing_struct_m11(NULL, tmp_str, TIME_SERIES_METADATA_FILE_TYPE_CODE_m11, METADATA_BYTES_m11, (LEVEL_HEADER_m11 *) sess, chan->metadata_fps, METADATA_BYTES_m11);
			for (i = 1; i < sess->number_of_time_series_channels; ++i) {
				chan = sess->time_series_channels[i];
				if (chan->flags & LH_UPDATE_EPHEMERAL_DATA_m11) {
					merge_universal_headers_m11(sess->time_series_metadata_fps, chan->metadata_fps, NULL);
					merge_metadata_m11(sess->time_series_metadata_fps, chan->metadata_fps, NULL);
					chan->flags &= ~LH_UPDATE_EPHEMERAL_DATA_m11;  // clear flag
				}
			}
			// merge session records
			if (sess->record_indices_fps != NULL && sess->record_data_fps != NULL)    // record data, not record indices universal header is merged in ephemeral data
				merge_universal_headers_m11(sess->time_series_metadata_fps, sess->record_data_fps, NULL);
			if (ssr != NULL) {
				for (i = 0, j = seg_idx; i < n_segs; ++i, ++j) {
					if (ssr->record_indices_fps[j] != NULL && ssr->record_data_fps[j] != NULL)
						merge_universal_headers_m11(sess->time_series_metadata_fps, ssr->record_data_fps[j], NULL);
				}
			}
			// fix ephemeral universal header
			uh = sess->time_series_metadata_fps->universal_header;
			uh->type_code = TIME_SERIES_METADATA_FILE_TYPE_CODE_m11;
			uh->segment_number = UNIVERSAL_HEADER_SESSION_LEVEL_CODE_m11;
			uh->session_UID = globals_m11->session_UID;
			uh->channel_UID = uh->segment_UID = UID_NO_ENTRY_m11;
		}
		if (sess->number_of_video_channels) {
			if (sess->video_metadata_fps != NULL)
				FPS_free_processing_struct_m11(sess->video_metadata_fps, TRUE_m11);
			sprintf_m11(tmp_str, "%s/%s_video.%s", sess->path, sess->name, VIDEO_METADATA_FILE_TYPE_STRING_m11);
			chan = sess->video_channels[0];
			sess->video_metadata_fps = FPS_allocate_processing_struct_m11(NULL, tmp_str, VIDEO_METADATA_FILE_TYPE_CODE_m11, METADATA_BYTES_m11, (LEVEL_HEADER_m11 *) sess, chan->metadata_fps, METADATA_BYTES_m11);
			for (i = 1; i < sess->number_of_video_channels; ++i) {
				chan = sess->video_channels[i];
				if (chan->flags & LH_UPDATE_EPHEMERAL_DATA_m11) {
					merge_universal_headers_m11(sess->video_metadata_fps, chan->metadata_fps, NULL);
					merge_metadata_m11(sess->video_metadata_fps, chan->metadata_fps, NULL);
					chan->flags &= ~LH_UPDATE_EPHEMERAL_DATA_m11;  // clear flag
				}
			}
			// merge session records
			if (sess->record_indices_fps != NULL && sess->record_data_fps != NULL)    // record data, not record indices universal header is merged in ephemeral data
				merge_universal_headers_m11(sess->video_metadata_fps, sess->record_data_fps, NULL);
			if (ssr != NULL) {
				for (i = 0, j = seg_idx; i < n_segs; ++i, ++j) {
					if (ssr->record_indices_fps[j] != NULL && ssr->record_data_fps[j] != NULL)
						merge_universal_headers_m11(sess->video_metadata_fps, ssr->record_data_fps[j], NULL);
				}
			}
			// fix ephemeral universal header
			uh = sess->video_metadata_fps->universal_header;
			uh->type_code = VIDEO_METADATA_FILE_TYPE_CODE_m11;
			uh->segment_number = UNIVERSAL_HEADER_SESSION_LEVEL_CODE_m11;
			uh->session_UID = globals_m11->session_UID;
			uh->channel_UID = uh->segment_UID = UID_NO_ENTRY_m11;
		}
	}

	sess->last_access_time = curr_time;
	if (sess->segmented_sess_recs != NULL)
		sess->segmented_sess_recs->last_access_time = curr_time;
	
	return(sess);
}


si8     pad_m11(ui1 *buffer, si8 content_len, ui4 alignment)
{
	si8        i, pad_bytes;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	pad_bytes = content_len % (si8) alignment;
	if (pad_bytes) {
		i = pad_bytes = (alignment - pad_bytes);
		buffer += content_len;
		while (i--)
			*buffer++ = PAD_BYTE_VALUE_m11;
	}
	
	return(content_len + pad_bytes);
}


TERN_m11	path_from_root_m11(si1 *path, si1 *root_path)
{
	si1	*c, *c2, base_dir[FULL_FILE_NAME_BYTES_m11];
	si8	len, len2;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// assumes root_path has adequate space for new path
	
	// if root_path == NULL : return T/F on path, do not modify path
	// if root_path == path : return T/F on path, do modify path
	// if root_path != path && root_path != NULL : return T/F on path, return path to root in root_path
	
	// if path starts with "/", returns TRUE ("C:" prepended in Windows, if "modify_path" is TRUE)
	// Windows: if path starts with "<capital letter>:\" returns TRUE
	// if path starts with ".", "..", or "~", these are resolved as expected.
	
	if (path == NULL)
		return(FALSE_m11);
		
#if defined MACOS_m11 || defined LINUX_m11
	if (root_path != NULL && root_path != path)
		strcpy(root_path, path);

	// remove terminal '/' from passed path if present
	if (root_path != NULL) {
		len = strlen(path);
		if (len)
			if (root_path[len - 1] == '/')
				root_path[--len] = 0;
	}
	
	if (*path == '/')
		return(TRUE_m11);
	
	if (root_path == NULL)
		return(FALSE_m11);
	
	// get base directory
	c = root_path;
	if (*c == '~') {
		strcpy(base_dir, getenv("HOME"));
		++c;
		if (*c == '/')
			++c;
	} else {
		getcwd_m11(base_dir, FULL_FILE_NAME_BYTES_m11);
	}
	
	// drop terminal '/' from base_dir, if present
	len2 = strlen(base_dir);
	if (base_dir[len2 - 1] == '/') {
		if (len2 > 1)  // at root
			base_dir[--len2] = 0;
	}
	
	// handle "." & ".."
	while (*c == '.') {
		if (*(c + 1) == '.') {  // backup base_dir to previous directory
			c2 = base_dir + len2;
			while (*--c2 != '/');
			if (c2 == base_dir)  // at root
				*++c2 = 0;
			else
				*c2 = 0;
			len2 = strlen(base_dir);
			++c;
		}
		if (*(c + 1) == '/')
			c += 2;
		else
			break;  // ".filename" (invisible) form
	}

	if (*c)
		sprintf_m11(root_path, "%s/%s", base_dir, c);  // Note c may overlap root_path so use sprintf_m11()
	else
		strcpy(root_path, base_dir);
#endif
	
#ifdef WINDOWS_m11
	si1	tmp_path[FULL_FILE_NAME_BYTES_m11];
	DWORD	ret_val;
	

	len = (si8) GetFullPathNameA(path, (DWORD) FULL_FILE_NAME_BYTES_m11, tmp_path, NULL);
	if (len == 0)
		return(FALSE_m11);
	if (root_path == NULL) {  // just return T/F
		if (strncmp(path, tmp_path, len))
			return(FALSE_m11);
		else
			return(TRUE_m11);
	}
	strcpy(root_path, tmp_path);
#endif

	return(TRUE_m11);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void	pop_behavior_m11(void)  //*** THIS ROUTINE IS NOT THREAD SAFE - USE JUDICIOUSLY IN THREADED APPLICATIONS ***//
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// get mutex
	pthread_mutex_lock_m11(&globals_m11->behavior_mutex);
	
	if (globals_m11->behavior_stack_entries == 0) {  // this shouldn't happen, but is possible
		globals_m11->behavior_on_fail = GLOBALS_BEHAVIOR_ON_FAIL_DEFAULT_m11;
		pthread_mutex_unlock_m11(&globals_m11->behavior_mutex);
		return;
	}
	
	globals_m11->behavior_on_fail = globals_m11->behavior_stack[--globals_m11->behavior_stack_entries];

	// release mutex
	pthread_mutex_unlock_m11(&globals_m11->behavior_mutex);

	return;
}


TERN_m11	process_password_data_m11(FILE_PROCESSING_STRUCT_m11 *fps, si1 *unspecified_pw)
{
	TERN_m11		pw_ok;
	PASSWORD_DATA_m11	*pwd;
	ui1			hash[SHA_HASH_BYTES_m11];
	si1			unspecified_pw_bytes[PASSWORD_BYTES_m11] = {0}, putative_L1_pw_bytes[PASSWORD_BYTES_m11] = {0};
	si4			i;
	METADATA_SECTION_1_m11	*md1;
	UNIVERSAL_HEADER_m11	*uh;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// Returns FALSE_m11 to indicate no encryption/decryption access.
	// The password structure is set to processed, regardless of access.
	// Unencrypted data can be read without access privileges.
	
	// can't verify passwords without a universal header
	if (fps == NULL) {
		warning_message_m11("%s(): file processing struct is NULL\n", __FUNCTION__);
		return(FALSE_m11);
	}
	pwd = fps->parameters.password_data;
	if (pwd == NULL)
		pwd = fps->parameters.password_data = &globals_m11->password_data;
	memset((void *) pwd, 0, sizeof(PASSWORD_DATA_m11));
	pwd->processed = TRUE_m11;
	
	// NULL and "" are equivalent in this function
	if (unspecified_pw == NULL)
		unspecified_pw = "";
		
	// copy password hints from metadata to pwd if possible
	uh = fps->universal_header;
	if (uh->type_code == TIME_SERIES_METADATA_FILE_TYPE_CODE_m11 || uh->type_code == VIDEO_METADATA_FILE_TYPE_CODE_m11) {
		md1 = &fps->metadata->section_1;
		if (*md1->level_1_password_hint)
			strncpy_m11(pwd->level_1_password_hint, md1->level_1_password_hint, PASSWORD_HINT_BYTES_m11);
		if (*md1->level_2_password_hint)
			strncpy_m11(pwd->level_2_password_hint, md1->level_2_password_hint, PASSWORD_HINT_BYTES_m11);
	}

	pw_ok = FALSE_m11;
	if (*unspecified_pw) // don't warn if no password passed (could be intentional), but still show hints (below) if they exist
		pw_ok = check_password_m11(unspecified_pw);
	if (pw_ok == TRUE_m11) {
			
		// get terminal bytes
		extract_terminal_password_bytes_m11(unspecified_pw, unspecified_pw_bytes);

		// check if password protected (no need to check level 2, since for level 2 to exist, level 1 must exist)
		if (all_zeros_m11(uh->level_1_password_validation_field, PASSWORD_VALIDATION_FIELD_BYTES_m11) == TRUE_m11)
			return(TRUE_m11);
		
		// check for level 1 access
		SHA_hash_m11((ui1 *) unspecified_pw_bytes, PASSWORD_BYTES_m11, hash);  // generate SHA-256 hash of password bytes
		for (i = 0; i < PASSWORD_VALIDATION_FIELD_BYTES_m11; ++i)  // compare with stored level 1 hash
			if (hash[i] != uh->level_1_password_validation_field[i])
				break;
		if (i == PASSWORD_BYTES_m11) {  // Level 1 password valid (cannot be level 2 password)
			pwd->access_level = LEVEL_1_ACCESS_m11;
			AES_key_expansion_m11(pwd->level_1_encryption_key, unspecified_pw_bytes);  // generate key
			if (globals_m11->verbose == TRUE_m11)
				message_m11("Unspecified password is valid for Level 1 access");
			return(TRUE_m11);
		}
			
		// invalid level 1 => check if level 2 password
		for (i = 0; i < PASSWORD_BYTES_m11; ++i)  // xor with level 2 password validation field
			putative_L1_pw_bytes[i] = hash[i] ^ uh->level_2_password_validation_field[i];
			
		SHA_hash_m11((ui1 *) putative_L1_pw_bytes, PASSWORD_BYTES_m11, hash); // generate SHA-256 hash of putative level 1 password
			
		for (i = 0; i < PASSWORD_VALIDATION_FIELD_BYTES_m11; ++i)  // compare with stored level 1 hash
			if (hash[i] != uh->level_1_password_validation_field[i])
				break;
		if (i == PASSWORD_VALIDATION_FIELD_BYTES_m11) {  // Level 2 password valid
			pwd->access_level = LEVEL_2_ACCESS_m11;
			AES_key_expansion_m11(pwd->level_1_encryption_key, putative_L1_pw_bytes);  // generate level 1 key
			AES_key_expansion_m11(pwd->level_2_encryption_key, unspecified_pw_bytes);  // generate level 2 key
			if (globals_m11->verbose == TRUE_m11)
				message_m11("Unspecified password is valid for Level 1 and Level 2 access\n");
			return(TRUE_m11);
		}

		// invalid as level 2 password
		warning_message_m11("%s(): password is not valid for Level 1 or Level 2 access\n", __FUNCTION__);
	}
	// check_password_m11() == FALSE_m11 or unspecified password invalid
	show_password_hints_m11(pwd); // if hints exist

	return(FALSE_m11);
}


void	propogate_flags_m11(LEVEL_HEADER_m11 *level_header, ui8 new_flags)
{
	si4			n_ts_chans, n_vid_chans, n_segs;
	ui8			open_status, active_status;
	si8			i, j;
	SEGMENT_m11		*seg;
	CHANNEL_m11		*chan;
	CHANNEL_m11		**ts_chans;
	CHANNEL_m11		**vid_chans;
	SESSION_m11		*sess = NULL;
	SEGMENTED_SESS_RECS_m11	*ssr;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif

	switch (level_header->type_code) {
		case LH_TIME_SERIES_SEGMENT_m11:
			seg = (SEGMENT_m11 *) level_header;
			chan = NULL;
			ts_chans =  &chan;
			n_ts_chans = 1;
			vid_chans =  &chan;
			n_vid_chans = 0;
			n_segs = 1;
			break;
		case LH_VIDEO_SEGMENT_m11:
			seg = (SEGMENT_m11 *) level_header;
			chan = NULL;
			ts_chans =  &chan;
			n_ts_chans = 0;
			vid_chans =  &chan;
			n_vid_chans = 1;
			n_segs = 1;
			break;
		case LH_TIME_SERIES_CHANNEL_m11:
			chan = (CHANNEL_m11 *) level_header;
			ts_chans =  &chan;
			n_ts_chans = 1;
			n_vid_chans = 0;
			n_segs = globals_m11->number_of_mapped_segments;
			break;
		case LH_VIDEO_CHANNEL_m11:
			chan = (CHANNEL_m11 *) level_header;
			vid_chans = &chan;
			n_ts_chans = 0;
			n_vid_chans = 1;
			n_segs = globals_m11->number_of_mapped_segments;
			break;
		case LH_SESSION_m11:
			sess = (SESSION_m11 *) level_header;
			ts_chans = sess->time_series_channels;
			n_ts_chans = sess->number_of_time_series_channels;
			vid_chans = sess->video_channels;
			n_vid_chans = sess->number_of_video_channels;
			n_segs = globals_m11->number_of_mapped_segments;
			break;
		default:
			warning_message_m11("%s(): invalid level type\n", __FUNCTION__);
			return;
	}
	
	// condition new flags
	new_flags &= ~(LH_OPEN_m11 | LH_CHANNEL_ACTIVE_m11);
	
	// session
	if (sess != NULL) {
		open_status = sess->flags & LH_OPEN_m11;
		sess->flags = new_flags | open_status;
		// segmented session records
		ssr = sess->segmented_sess_recs;
		if (ssr != NULL) {
			open_status = ssr->flags & LH_OPEN_m11;
			ssr->flags = new_flags | open_status;
		}
	}
	
	// time series channels
	for (i = 0; i < n_ts_chans; ++i) {
		for (j = 0; j < n_segs; ++j) {
			if (ts_chans[i] != NULL)
				seg = ts_chans[i]->segments[j];
			if (seg != NULL) {
				open_status = seg->flags & LH_OPEN_m11;
				seg->flags = new_flags | open_status;
			}
		}
		if (ts_chans[i] != NULL) {
			open_status = ts_chans[i]->flags & LH_OPEN_m11;
			active_status = ts_chans[i]->flags & LH_CHANNEL_ACTIVE_m11;
			ts_chans[i]->flags = new_flags | open_status | active_status;
		}
	}
	
	// video channels
	for (i = 0; i < n_vid_chans; ++i) {
		for (j = 0; j < n_segs; ++j) {
			if (vid_chans[i] != NULL)
				seg = vid_chans[i]->segments[j];
			if (seg != NULL) {
				open_status = seg->flags & LH_OPEN_m11;
				seg->flags = new_flags | open_status;
			}
		}
		if (vid_chans[i] != NULL) {
			open_status = vid_chans[i]->flags & LH_OPEN_m11;
			active_status = vid_chans[i]->flags & LH_CHANNEL_ACTIVE_m11;
			vid_chans[i]->flags = new_flags | open_status | active_status;
		}
	}

	return;
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4	pthread_mutex_destroy_m11(pthread_mutex_t_m11 *mutex)
{
	si4	ret_val;
	
#if defined MACOS_m11 || defined LINUX_m11
	ret_val = pthread_mutex_destroy(mutex);
#endif
#ifdef WINDOWS_m11
	ret_val = 1 - (si4) CloseHandle(*mutex);  // CloseHandle returns zero on fail
#endif
	
	return(ret_val);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4	pthread_mutex_init_m11(pthread_mutex_t_m11 *mutex, pthread_mutexattr_t_m11 *attr)
{
	si4	ret_val;
	
#if defined MACOS_m11 || defined LINUX_m11
	ret_val = pthread_mutex_init(mutex, attr);
#endif
#ifdef WINDOWS_m11
	if ((*mutex = CreateMutex(attr, 0, NULL)) == NULL)
		ret_val = -1;
	else
		ret_val = 0;
#endif
	
	return(ret_val);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4	pthread_mutex_lock_m11(pthread_mutex_t_m11 *mutex)
{
	si4	ret_val;
	
#if defined MACOS_m11 || defined LINUX_m11
	ret_val = pthread_mutex_lock(mutex);
#endif
#ifdef WINDOWS_m11
	if (WaitForSingleObject(*mutex, INFINITE) == WAIT_OBJECT_0)
		ret_val = 0;
	else
		ret_val = -1;
#endif
	
	return(ret_val);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4	pthread_mutex_unlock_m11(pthread_mutex_t_m11 *mutex)
{
	si4	ret_val;
	
#if defined MACOS_m11 || defined LINUX_m11
	ret_val = pthread_mutex_unlock(mutex);
#endif
#ifdef WINDOWS_m11
	if (ReleaseMutex(*mutex) == 0)
		ret_val = -1;
	else
		ret_val = 0;
#endif
	
	return(ret_val);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void	push_behavior_m11(ui4 behavior)  //*** THIS ROUTINE IS NOT THREAD SAFE - USE JUDICIOUSLY IN THREADED APPLICATIONS ***//
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (behavior == RESTORE_BEHAVIOR_m11) {
		pop_behavior_m11();
		return;
	}
	
	// get mutex
	pthread_mutex_lock_m11(&globals_m11->behavior_mutex);
	
	if (globals_m11->behavior_stack_entries == globals_m11->behavior_stack_size) {
		globals_m11->behavior_stack_size += GLOBALS_BEHAVIOR_STACK_SIZE_INCREMENT_m11;
		globals_m11->behavior_stack = (ui4 *) realloc_m11((void *) globals_m11->behavior_stack, (size_t) globals_m11->behavior_stack_size * sizeof(ui4), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	}
	
	globals_m11->behavior_stack[globals_m11->behavior_stack_entries++] = globals_m11->behavior_on_fail;
	globals_m11->behavior_on_fail = behavior;
	
	// release mutex
	pthread_mutex_unlock_m11(&globals_m11->behavior_mutex);

	return;
}


CHANNEL_m11	*read_channel_m11(CHANNEL_m11 *chan, TIME_SLICE_m11 *slice, ...)  // varargs: si1 *chan_path, ui8 flags, si1 *password
{
	TERN_m11			open_channel, free_channel;
	si1                             tmp_str[FULL_FILE_NAME_BYTES_m11], *chan_path, *password;
	si1                             num_str[FILE_NUMBERING_DIGITS_m11 + 1];
	si4                             i, j, seg_idx, n_segs, null_segment_cnt;
	ui8                             flags;
	va_list				args;
	SEGMENT_m11			*seg;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// open channel
	open_channel = free_channel = FALSE_m11;
	if (chan == NULL)
		open_channel = free_channel = TRUE_m11;
	else if (!(chan->flags & LH_OPEN_m11))
		open_channel = TRUE_m11;
	if (open_channel == TRUE_m11) {
		// get varargs
		va_start(args, slice);
		chan_path = va_arg(args, si1 *);
		flags = va_arg(args, ui8);
		password = va_arg(args, si1 *);
		va_end(args);
		// open channel
		chan = open_channel_m11(chan, slice, chan_path, (flags & ~LH_OPEN_m11), password);
		if (chan == NULL) {
			error_message_m11("%s(): error opening channel\n", __FUNCTION__);
			return(NULL);
		}
	}

	// process time slice (passed slice is not modified)
	if (slice == NULL) {
		if (all_zeros_m11((ui1 *) &chan->time_slice, (si4) sizeof(TIME_SLICE_m11)) == TRUE_m11)
			initialize_time_slice_m11(&chan->time_slice);  // read whole channel
	} else {  // passed slice supersedes structure slice
		chan->time_slice = *slice;  // passed slice is not modified
	}
	slice = &chan->time_slice;
	if (slice->conditioned == FALSE_m11)
		condition_time_slice_m11(slice);
		
	// get segment range
	if (slice->number_of_segments == UNKNOWN_m11) {
		n_segs = get_segment_range_m11((LEVEL_HEADER_m11 *) chan, slice);
		if (n_segs == 0) {
			if (free_channel == TRUE_m11)
				free_channel_m11(chan, TRUE_m11);
			return(NULL);
		}
	} else {
		n_segs = slice->number_of_segments;
	}
	seg_idx = get_segment_index_m11(slice->start_segment_number);
	if (seg_idx == FALSE_m11) {
		if (free_channel == TRUE_m11)
			free_channel_m11(chan, TRUE_m11);
		return(NULL);
	}

	// read segments
	null_segment_cnt = 0;
	for (i = slice->start_segment_number, j = seg_idx; i <= slice->end_segment_number; ++i, ++j) {
		seg = chan->segments[j];
		if (seg == NULL) {
			numerical_fixed_width_string_m11(num_str, FILE_NUMBERING_DIGITS_m11, i);
			if (chan->type_code == LH_TIME_SERIES_CHANNEL_m11)
				sprintf_m11(tmp_str, "%s/%s_s%s.%s", chan->path, chan->name, num_str, TIME_SERIES_SEGMENT_DIRECTORY_TYPE_STRING_m11);
			else  // LH_VIDEO_CHANNEL_m11
				sprintf_m11(tmp_str, "%s/%s_s%s.%s", chan->path, chan->name, num_str, VIDEO_SEGMENT_DIRECTORY_TYPE_STRING_m11);
			seg = chan->segments[j] = read_segment_m11(NULL, slice, tmp_str, (chan->flags & ~LH_OPEN_m11), NULL);
		} else {
			seg = read_segment_m11(seg, slice);
		}
		if (seg == NULL)
			++null_segment_cnt;
		else
			seg->parent = (void *) chan;
	}
	
	// empty slice
	if (null_segment_cnt == n_segs) {
		slice->number_of_segments = EMPTY_SLICE_m11;
		if (free_channel == TRUE_m11)
			free_channel_m11(chan, TRUE_m11);
		return(NULL);
	}
	
	// update slice
	for (i = 0, j = seg_idx; i < n_segs; ++i, ++j) {
		seg = chan->segments[j];
		if (seg != NULL)
			break;
	}
	slice->start_time = seg->time_slice.start_time;
	slice->start_sample_number = seg->time_slice.start_sample_number;
	slice->start_segment_number = seg->time_slice.start_segment_number;
	for (++i, ++j; i < n_segs; ++i, ++j) {
		if (chan->segments[j] != NULL)
			seg = chan->segments[j];
	}
	slice->end_time = seg->time_slice.end_time;
	slice->end_sample_number = seg->time_slice.end_sample_number;
	slice->end_segment_number = seg->time_slice.end_segment_number;
	slice->number_of_segments = TIME_SLICE_SEGMENT_COUNT_m11(slice);

	// records
	if (chan->flags & LH_READ_CHANNEL_RECORDS_MASK_m11)
		if (chan->record_indices_fps != NULL && chan->record_data_fps != NULL)
			read_record_data_m11((LEVEL_HEADER_m11 *) chan, slice);
	
	// update ephemeral data
	if (chan->flags & LH_GENERATE_EPHEMERAL_DATA_m11) {
		for (i = 0, j = seg_idx; i < n_segs; ++i, ++j) {
			seg = chan->segments[j];
			if (seg == NULL)
				continue;
			if (seg->flags & LH_UPDATE_EPHEMERAL_DATA_m11) {
				merge_universal_headers_m11(chan->metadata_fps, seg->metadata_fps, NULL);
				merge_metadata_m11(chan->metadata_fps, seg->metadata_fps, NULL);
				if (seg->record_indices_fps != NULL && seg->record_data_fps != NULL)
					merge_universal_headers_m11(chan->metadata_fps, seg->record_data_fps, NULL);
				seg->flags &= ~LH_UPDATE_EPHEMERAL_DATA_m11;  // clear segment flag
				chan->flags |= LH_UPDATE_EPHEMERAL_DATA_m11;  // set channel flag (for session)
			}
		}
	
		// fix session ephemeral FPS (from merge functions)
		if (chan->type_code == LH_TIME_SERIES_CHANNEL_m11)
			chan->metadata_fps->universal_header->type_code = TIME_SERIES_METADATA_FILE_TYPE_CODE_m11;
		else if (chan->type_code == LH_VIDEO_CHANNEL_m11)
			chan->metadata_fps->universal_header->type_code = VIDEO_METADATA_FILE_TYPE_CODE_m11;
		chan->metadata_fps->universal_header->segment_number = UNIVERSAL_HEADER_CHANNEL_LEVEL_CODE_m11;
		chan->metadata_fps->universal_header->segment_UID = UID_NO_ENTRY_m11;
	}
	
	// verbose
	if (globals_m11->verbose == TRUE_m11) {
		printf_m11("--------- Channel Universal Header ---------\n");
		show_universal_header_m11(chan->metadata_fps, NULL);
		printf_m11("------------ Channel Metadata --------------\n");
		show_metadata_m11(chan->metadata_fps, NULL, 0);
	}

	chan->last_access_time = current_uutc_m11();
	
	return(chan);
}


LEVEL_HEADER_m11	*read_data_m11(LEVEL_HEADER_m11 *level_header, TIME_SLICE_m11 *slice, ...)  // varargs (level_header == NULL): void *file_list, si4 list_len, ui8 flags, si1 *password
{
	void				*file_list;
	si1				*password, tmp_str[FULL_FILE_NAME_BYTES_m11];
	ui4				type_code;
	si4				list_len;
	ui8				flags;
	va_list				args;
	SESSION_m11			*sess;
	CHANNEL_m11			*chan;
	SEGMENT_m11			*seg;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (level_header == NULL) {
		// get varargs
		va_start(args, slice);
		file_list = va_arg(args, void *);
		list_len = va_arg(args, si4);
		flags = va_arg(args, ui8);
		password = va_arg(args, si1 *);
		va_end(args);
		
		// get level
		if (list_len == 0) { // single string
			type_code = get_level_m11((si1 *) file_list, NULL);
		} else {
			extract_path_parts_m11((si1 *) *((si1 **) file_list), tmp_str, NULL, NULL);
			type_code = get_level_m11(tmp_str, NULL);
		}
		
		switch (type_code) {
			case LH_SESSION_m11:
				sess = open_session_m11(NULL, slice, file_list, list_len, flags, password);
				if (sess == NULL) {
					error_message_m11("%s(): error opening session\n", __FUNCTION__);
					return(NULL);
				}
				level_header = (LEVEL_HEADER_m11 *) sess;
				break;
			case LH_SEGMENTED_SESS_RECS_m11:
				error_message_m11("%s(): can not currently process segmented session records as a level\n", __FUNCTION__);
				return(NULL);
			case LH_TIME_SERIES_CHANNEL_m11:
			case LH_VIDEO_CHANNEL_m11:
				chan = open_channel_m11(NULL, slice, (si1 *) file_list, flags, password);
				if (chan == NULL) {
					error_message_m11("%s(): error opening channel\n", __FUNCTION__);
					return(NULL);
				}
				level_header = (LEVEL_HEADER_m11 *) chan;
			case LH_TIME_SERIES_SEGMENT_m11:
			case LH_VIDEO_SEGMENT_m11:
				seg = open_segment_m11(NULL, slice, (si1 *) file_list, flags, password);
				if (seg == NULL) {
					error_message_m11("%s(): error opening segment\n", __FUNCTION__);
					return(NULL);
				}
				level_header = (LEVEL_HEADER_m11 *) seg;
				break;
		}
	}

	switch (level_header->type_code) {
		case LH_SESSION_m11:
			sess = (SESSION_m11 *) level_header;
			sess = read_session_m11(sess, slice);
			if (sess == NULL) {
				error_message_m11("%s(): error reading session\n", __FUNCTION__);
				return(NULL);
			}
			break;
		case LH_SEGMENTED_SESS_RECS_m11:
			error_message_m11("%s(): can not currently process segmented session records as a level\n", __FUNCTION__);
			return(NULL);
		case LH_TIME_SERIES_CHANNEL_m11:
		case LH_VIDEO_CHANNEL_m11:
			chan = (CHANNEL_m11 *) level_header;
			chan = read_channel_m11(chan, slice);
			if (chan == NULL) {
				error_message_m11("%s(): error reading channel\n", __FUNCTION__);
				return(NULL);
			}
			break;
		case LH_TIME_SERIES_SEGMENT_m11:
		case LH_VIDEO_SEGMENT_m11:
			seg = (SEGMENT_m11 *) level_header;
			seg = read_segment_m11(seg, slice);
			if (chan == NULL) {
				error_message_m11("%s(): error reading segment\n", __FUNCTION__);
				return(NULL);
			}
			break;
	}

	return(level_header);
}


FILE_PROCESSING_STRUCT_m11	*read_file_m11(FILE_PROCESSING_STRUCT_m11 *fps, si1 *full_file_name, si8 file_offset, si8 bytes_to_read, si8 number_of_items, LEVEL_HEADER_m11 *lh, si1 *password, ui4 behavior_on_fail)
{
	TERN_m11		opened_flag, mmap_flag, close_flag, data_read_flag, allocated_flag, readable, CRC_valid;
	ui8			lh_flags;
	si8			bytes_read, required_bytes;
	UNIVERSAL_HEADER_m11	*uh;
	FILE_TIMES_m11		ft;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// if fps == NULL, it will be allocated using full file name, if not full_file name can be NULL
	
	// fps file type pointer will be set to beginning of read data section
	// bytes_to_read == bytes to read or zero, if specified in another way (e.g. number_of_items == FPS_FULL_FILE_m11 || FPS_UNIVERSAL_HEADER_ONLY_m11 or via lh_flags)
	// if bytes_to_read == 0, and it is needed, it will be calculated (passing is more efficient; in general the caller will know this value)
	// fps data_pointers will be set to beginning of bytes read in raw data (where this is in memory will depend on the memory_map & full_file_read flags)
	// 	if memory mapping is true, the data will be read to the same offset in memory as found in the file
	// 	if memory mapping is false, the data will be read to right after the universal header in memory
	// number_of_items == 0 || FPS_FULL_FILE_m11 || FPS_UNIVERSAL_HEADER_ONLY_m11
	// number_of_items is only needed for decrypting partial timeseries and record data file reads
	// if number_of_items == 0, and it is needed, it will be calculated (passing is more efficient; in general the caller will know this value)
	// if number_of_items == FPS_FULL_FILE_m11, memory mapping is FALSE, full_file_read is TRUE, and file is closed
	// if number_of_items == FPS_UNIVERSAL_HEADER_ONLY_m11, the universal header is read, and file is left open (close flag is not changed, so will close on next read if set)
	// if lh_flags != 0, they are interpreted as LEVEL_HEADER flags, and used to determined whether the should be opened with FPS_FULL_FILE_m11, FPS_UNIVERSAL_HEADER_ONLY_m11, or memory mapping
	
	if (lh == NULL)
		lh_flags = LH_NO_FLAGS_m11;
	else
		lh_flags = lh->flags;
	
	if (bytes_to_read == 0 && number_of_items == 0 && lh_flags == LH_NO_FLAGS_m11) {
		error_message_m11("%s(): must specify either bytes_to_read, number_of_items, or lh_flags \n", __FUNCTION__);
		set_error_m11(E_READ_ERR_m11, __FUNCTION__, __LINE__);
		return(NULL);
	}
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m11)
		behavior_on_fail = globals_m11->behavior_on_fail;
	file_offset = REMOVE_DISCONTINUITY_m11(file_offset);
	
	// allocate FPS
	allocated_flag = FALSE_m11;
	if (fps == NULL) {
		if (full_file_name == NULL) {
			warning_message_m11("%s(): FILE_PROCESSING_STRUCT_m11 and full_file_name are both NULL\n", __FUNCTION__);
			set_error_m11(E_NO_FILE_m11, __FUNCTION__, __LINE__);
			return(NULL);
		}
		close_flag = FPS_DIRECTIVES_CLOSE_FILE_DEFAULT_m11;
		mmap_flag = FPS_DIRECTIVES_MEMORY_MAP_DEFAULT_m11;
		if (lh_flags == LH_NO_FLAGS_m11)  // use global flags if none passed (these are LH_NO_FLAGS_m11 by default)
			lh_flags = globals_m11->level_header_flags;
		if (lh_flags)
			lh_set_directives_m11(full_file_name, lh_flags, &mmap_flag, &close_flag, &number_of_items);
		if (number_of_items == FPS_FULL_FILE_m11)
			mmap_flag = FALSE_m11;
		// allocate
		if (mmap_flag == TRUE_m11 || number_of_items == FPS_FULL_FILE_m11)
			fps = FPS_allocate_processing_struct_m11(NULL, full_file_name, NO_FILE_TYPE_CODE_m11, FPS_FULL_FILE_m11, lh, NULL, 0);
		else
			fps = FPS_allocate_processing_struct_m11(NULL, full_file_name, NO_FILE_TYPE_CODE_m11, bytes_to_read, lh, NULL, 0);
		if (fps == NULL)
			return(NULL);
		fps->directives.memory_map = mmap_flag;
		fps->directives.close_file = close_flag;
		allocated_flag = TRUE_m11;
	}
	
	// full file already read
	if (fps->parameters.full_file_read == TRUE_m11) {
		file_times_m11(NULL, fps->full_file_name, &ft, FALSE_m11);
		if (fps->parameters.last_access_time >= ft.modification) {  // no change since last read: common scenario for metadata files & index files
			if (file_offset < UNIVERSAL_HEADER_BYTES_m11)  // set pointers to beginning of data if none provided
				file_offset = UNIVERSAL_HEADER_BYTES_m11;
			if (number_of_items == FPS_FULL_FILE_m11) {
				file_offset = UNIVERSAL_HEADER_BYTES_m11;  // (often file_offset == 0 when number_of_items == FPS_FULL_FILE_m11)
				number_of_items = fps->universal_header->number_of_entries;
			} else if (number_of_items == 0) {
				number_of_items = items_for_bytes_m11(fps, &bytes_to_read);
			}
			// set pointers & number of items to values for current read
			FPS_set_pointers_m11(fps, file_offset);
			fps->number_of_items = number_of_items;
			return(fps);
		}
		fps->parameters.full_file_read = FALSE_m11;  // file changed, reset
	}

	// open file
	opened_flag = FALSE_m11;
	if (fps->parameters.fp == NULL) {
		if (!(fps->directives.open_mode & FPS_GENERIC_READ_OPEN_MODE_m11))
			fps->directives.open_mode = FPS_R_OPEN_MODE_m11;
		FPS_open_m11(fps, __FUNCTION__, behavior_on_fail);
		opened_flag = TRUE_m11;
	}

	// universal header
	uh = fps->universal_header;
	if (number_of_items == FPS_UNIVERSAL_HEADER_ONLY_m11 || number_of_items == FPS_FULL_FILE_m11 || opened_flag == TRUE_m11) {
		FPS_read_m11(fps, 0, UNIVERSAL_HEADER_BYTES_m11, __FUNCTION__, behavior_on_fail);
		if (uh->session_UID != globals_m11->session_UID)  // set current session directory globals
			get_session_directory_m11(NULL, NULL, fps);
		if (number_of_items == FPS_UNIVERSAL_HEADER_ONLY_m11) {
			if (fps->parameters.password_data->processed == 0)	// better if done with a metadata file read (for password hints) below
				process_password_data_m11(fps, password);	// done here to satify rule that any read of any MED file will process password
			FPS_set_pointers_m11(fps, UNIVERSAL_HEADER_BYTES_m11);
			fps->number_of_items = 0;
			return(fps);
		} else if (number_of_items == FPS_FULL_FILE_m11) {
			file_offset = UNIVERSAL_HEADER_BYTES_m11;
			bytes_to_read = fps->parameters.flen - UNIVERSAL_HEADER_BYTES_m11;
			number_of_items = uh->number_of_entries;
			fps->parameters.full_file_read = TRUE_m11;
			fps->directives.memory_map = FALSE_m11;	// full file doesn't need memory mapping
			fps->directives.close_file = TRUE_m11;	// full file reads are closed to keep open file count down
		}
	}
	
	// get bytes_to_read (preferably this is passed)
	data_read_flag = FALSE_m11;
	if (bytes_to_read == 0) {
		bytes_to_read = bytes_for_items_m11(fps, &number_of_items, file_offset);
		if (uh->type_code == TIME_SERIES_DATA_FILE_TYPE_CODE_m11 || uh->type_code == RECORD_DATA_FILE_TYPE_CODE_m11) {  // variable item size types require data to be read in bytes_for_items_m11()
			data_read_flag = TRUE_m11;
			bytes_read = bytes_to_read;  // set bytes_to_read == bytes_read to circumvent error below
		}
	}

	// allocate memory
	if (fps->directives.memory_map == TRUE_m11 || fps->parameters.full_file_read == TRUE_m11)
		required_bytes = fps->parameters.flen;
	else
		required_bytes = bytes_to_read + UNIVERSAL_HEADER_BYTES_m11;
	if (required_bytes > fps->parameters.raw_data_bytes)
		FPS_reallocate_processing_struct_m11(fps, required_bytes);
	
	// set memory pointers
	FPS_set_pointers_m11(fps, file_offset);
		
	// read
	if (data_read_flag == FALSE_m11)
		bytes_read = FPS_read_m11(fps, file_offset, bytes_to_read, __FUNCTION__, behavior_on_fail);
	if (fps->directives.close_file == TRUE_m11)
		FPS_close_m11(fps);
	if (bytes_read != bytes_to_read) {
		error_message_m11("%s(): file read error\n", __FUNCTION__);
		if (allocated_flag == TRUE_m11)
			FPS_free_processing_struct_m11(fps, TRUE_m11);
		set_error_m11(E_READ_ERR_m11, __FUNCTION__, __LINE__);
		return(NULL);
	}

	// process password (better done here than above because may be reading a metadata file)
	if (fps->parameters.password_data->processed == 0)	// if metadata file, hints from section 1 will be added to password
		process_password_data_m11(fps, password);	// data structure, and displayed if the password is invalid
	
	// get number_of_items (preferably this is passed)
	if (number_of_items == 0)
		number_of_items = items_for_bytes_m11(fps, &bytes_to_read);
	fps->number_of_items = number_of_items;

	// validate CRCs
	if (globals_m11->CRC_mode & (CRC_VALIDATE_m11 | CRC_VALIDATE_ON_INPUT_m11)) {
		CRC_valid = CRC_validate_m11(fps->parameters.raw_data + UNIVERSAL_HEADER_HEADER_CRC_START_OFFSET_m11, UNIVERSAL_HEADER_BYTES_m11 - UNIVERSAL_HEADER_HEADER_CRC_START_OFFSET_m11, uh->header_CRC);
		if (CRC_valid == FALSE_m11)
			warning_message_m11("%s(): universal header CRC invalid for \"%s\"\n", __FUNCTION__, fps->full_file_name);
		CRC_valid = UNKNOWN_m11;
		switch (fps->universal_header->type_code) {
			case TIME_SERIES_DATA_FILE_TYPE_CODE_m11:
				CRC_valid = validate_time_series_data_CRCs_m11(fps);
				break;
			case TIME_SERIES_INDICES_FILE_TYPE_CODE_m11:
				if (fps->parameters.full_file_read == TRUE_m11)
					CRC_valid = CRC_validate_m11(fps->data_pointers, fps->parameters.flen - UNIVERSAL_HEADER_BYTES_m11, uh->body_CRC);
				break;
			case RECORD_DATA_FILE_TYPE_CODE_m11:
				CRC_valid = validate_record_data_CRCs_m11(fps);
				break;
			case RECORD_INDICES_FILE_TYPE_CODE_m11:
				if (fps->parameters.full_file_read == TRUE_m11)
					CRC_valid = CRC_validate_m11(fps->data_pointers, fps->parameters.flen - UNIVERSAL_HEADER_BYTES_m11, uh->body_CRC);
				break;
			case TIME_SERIES_METADATA_FILE_TYPE_CODE_m11:
			case VIDEO_METADATA_FILE_TYPE_CODE_m11:
				if (fps->parameters.full_file_read == TRUE_m11)
					CRC_valid = CRC_validate_m11(fps->data_pointers, fps->parameters.flen - UNIVERSAL_HEADER_BYTES_m11, uh->body_CRC);
				break;
		}
		if (CRC_valid == FALSE_m11)
			warning_message_m11("%s(): body CRC invalid for \"%s\"\n", __FUNCTION__, fps->full_file_name);
	}

	// decrypt
	switch (fps->universal_header->type_code) {
		case TIME_SERIES_DATA_FILE_TYPE_CODE_m11:
			readable = TRUE_m11;
			if (globals_m11->time_series_data_encryption_level)
				readable = decrypt_time_series_data_m11(fps);
			break;
		case RECORD_DATA_FILE_TYPE_CODE_m11:
			readable = decrypt_record_data_m11(fps);
			break;
		case TIME_SERIES_METADATA_FILE_TYPE_CODE_m11:
		case VIDEO_METADATA_FILE_TYPE_CODE_m11:
			readable = decrypt_metadata_m11(fps);
			break;
		default:
			readable = TRUE_m11;  // file types without (possible) encryption
			break;
	}
	if (readable == FALSE_m11) {
		warning_message_m11("%s(): cannot read file \"%s\"\n", __FUNCTION__, fps->full_file_name);
		if (allocated_flag == TRUE_m11)
			FPS_free_processing_struct_m11(fps, TRUE_m11);
		set_error_m11(E_READ_ERR_m11, __FUNCTION__, __LINE__);
		return(NULL);
	}
	
	return(fps);
}


si8     read_record_data_m11(LEVEL_HEADER_m11 *level_header, TIME_SLICE_m11 *slice, ...)  // varags: si4 seg_num
{
	si4				seg_num;
	si8				start_idx, end_idx, n_recs, bytes_to_read, offset;
	FILE_PROCESSING_STRUCT_m11	*ri_fps, *rd_fps;
	RECORD_INDEX_m11		*ri;
	SESSION_m11			*sess;
	SEGMENTED_SESS_RECS_m11		*ssr;
	CHANNEL_m11			*chan;
	SEGMENT_m11			*seg;
	va_list				args;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// seg_num only reqired for segmented session records levels
	
	switch (level_header->type_code) {
		case LH_SESSION_m11:
			sess = (SESSION_m11 *) level_header;
			ri_fps = sess->record_indices_fps;
			rd_fps = sess->record_data_fps;
			break;
		case LH_SEGMENTED_SESS_RECS_m11:
			va_start(args, slice);
			seg_num = va_arg(args, si4);
			va_end(args);
			ssr = (SEGMENTED_SESS_RECS_m11 *) level_header;
			if (ssr->flags & LH_MAP_ALL_SEGMENTS_m11)
				--seg_num;
			else
				seg_num -= slice->start_segment_number;
			ri_fps = ssr->record_indices_fps[seg_num];
			rd_fps = ssr->record_data_fps[seg_num];
			break;
		case LH_TIME_SERIES_CHANNEL_m11:
		case LH_VIDEO_CHANNEL_m11:
			chan = (CHANNEL_m11 *) level_header;
			ri_fps = chan->record_indices_fps;
			rd_fps = chan->record_data_fps;
			break;
		case LH_TIME_SERIES_SEGMENT_m11:
		case LH_VIDEO_SEGMENT_m11:
			seg = (SEGMENT_m11 *) level_header;
			ri_fps = seg->record_indices_fps;
			rd_fps = seg->record_data_fps;
			break;
	}

	start_idx = find_record_index_m11(ri_fps, slice->start_time, FIND_FIRST_ON_OR_AFTER_m11, 0);
	if (start_idx == NO_INDEX_m11) {  // no records "on or after" slice beginning
		if (rd_fps != NULL)
			rd_fps->number_of_items = 0;;
		return(0);
	}
	ri = ri_fps->record_indices;
	if (ri[start_idx].start_time > slice->end_time) {  // no records "on or before" slice end
		if (rd_fps != NULL)
			rd_fps->number_of_items = 0;
		return(0);
	}
	end_idx = find_record_index_m11(ri_fps, slice->end_time, FIND_FIRST_AFTER_m11, start_idx);
	if (end_idx == NO_INDEX_m11) // no records after slice end, but some in slice => use terminal index
		end_idx = ri_fps->universal_header->number_of_entries - 1;
	n_recs = end_idx - start_idx;
	offset = REMOVE_DISCONTINUITY_m11(ri_fps->record_indices[start_idx].file_offset);
	bytes_to_read = REMOVE_DISCONTINUITY_m11(ri_fps->record_indices[end_idx].file_offset) - offset;
	rd_fps = read_file_m11(rd_fps, NULL, offset, bytes_to_read, n_recs, level_header, NULL, USE_GLOBAL_BEHAVIOR_m11);
	if (rd_fps == NULL)
		return((si8) FALSE_m11);
	
	return(n_recs);
}


SEGMENT_m11	*read_segment_m11(SEGMENT_m11 *seg, TIME_SLICE_m11 *slice, ...)  // varargs: si1 *seg_path, ui8 flags, si1 *password
{
	TERN_m11	open_segment, free_segment;
	si1		*seg_path, *password;
	si4		search_mode;
	ui8		flags;
	si8		seg_abs_start_samp_num, seg_abs_end_samp_num;
	va_list		args;
	UNIVERSAL_HEADER_m11			*uh;
	TIME_SERIES_METADATA_SECTION_2_m11	*tmd2;
	VIDEO_METADATA_SECTION_2_m11		*vmd2;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// open segment
	open_segment = free_segment = FALSE_m11;
	if (seg == NULL)
		open_segment = free_segment = TRUE_m11;
	else if (!(seg->flags & LH_OPEN_m11))
		open_segment = TRUE_m11;
	if (open_segment == TRUE_m11) {
		// get varargs
		va_start(args, slice);
		seg_path = va_arg(args, si1 *);
		flags = va_arg(args, ui8);
		password = va_arg(args, si1 *);
		va_end(args);
		// open segment
		seg = open_segment_m11(seg, slice, seg_path, (flags & ~LH_OPEN_m11), password);
		if (seg == NULL) {
			warning_message_m11("%s(): error opening segment\n", __FUNCTION__);
			return(NULL);
		}
	}

	// process time slice (passed slice is not modified)
	if (slice == NULL) {
		if (all_zeros_m11((ui1 *) &seg->time_slice, (si4) sizeof(TIME_SLICE_m11)) == TRUE_m11)
			initialize_time_slice_m11(&seg->time_slice);  // read whole segment
	} else {  // passed slice supersedes structure slice
		seg->time_slice = *slice;  // passed slice is not modified
	}
	slice = &seg->time_slice;
	if (slice->conditioned == FALSE_m11)
		condition_time_slice_m11(slice);
	
	// check for valid limit pair (time takes priority)
	if ((search_mode = get_search_mode_m11(slice)) == FALSE_m11) {
		if (free_segment == TRUE_m11)
			free_segment_m11(seg, TRUE_m11);
		return(NULL);
	}
	uh = seg->metadata_fps->universal_header;
	if (search_mode == TIME_SEARCH_m11) {
		if (slice->start_time < uh->segment_start_time)
			slice->start_time = uh->segment_start_time;
		if (slice->end_time > uh->segment_end_time)
			slice->end_time = uh->segment_end_time;
	}
			
	// get local indices
	if (seg->type_code == LH_TIME_SERIES_SEGMENT_m11) {
		tmd2 = &seg->metadata_fps->metadata->time_series_section_2;
		seg_abs_start_samp_num = tmd2->absolute_start_sample_number;
		seg_abs_end_samp_num = seg_abs_start_samp_num + tmd2->number_of_samples - (si8) 1;
	} else {  // seg->type_code == LH_VIDEO_SEGMENT_m11
		vmd2 = &seg->metadata_fps->metadata->video_section_2;
		seg_abs_start_samp_num = vmd2->absolute_start_frame_number;
		seg_abs_end_samp_num = seg_abs_start_samp_num + (vmd2->number_of_frames - (si8) 1);
	}
	
	// get local indices (sample number == frame number == idx)
	if (search_mode == SAMPLE_SEARCH_m11) {
		if (slice->start_sample_number < seg_abs_start_samp_num)
			slice->start_sample_number = seg_abs_start_samp_num;
		if (slice->end_sample_number > seg_abs_end_samp_num)
			slice->end_sample_number = seg_abs_end_samp_num;
		slice->start_time = uutc_for_sample_number_m11((LEVEL_HEADER_m11 *) seg, slice->start_sample_number, FIND_START_m11);
		slice->end_time = uutc_for_sample_number_m11((LEVEL_HEADER_m11 *) seg, slice->end_sample_number, FIND_END_m11);
	} else {  // search_mode == TIME_SEARCH_m11, convert input times to local indices
		if (slice->start_time < uh->segment_start_time)
			slice->start_time = uh->segment_start_time;
		if (slice->end_time > uh->segment_end_time)
			slice->end_time = uh->segment_end_time;
		slice->start_sample_number = sample_number_for_uutc_m11((LEVEL_HEADER_m11 *) seg, slice->start_time, FIND_CURRENT_m11);
		slice->end_sample_number = sample_number_for_uutc_m11((LEVEL_HEADER_m11 *) seg, slice->end_time, FIND_CURRENT_m11);
	}
	slice->start_segment_number = slice->end_segment_number = seg->metadata_fps->universal_header->segment_number;
	slice->number_of_segments = 1;

	// read segment data
	if (seg->flags & LH_READ_SEGMENT_DATA_MASK_m11) {
		switch (seg->type_code) {
			case LH_TIME_SERIES_SEGMENT_m11:
				read_time_series_data_m11(seg, slice);
				break;
			case LH_VIDEO_SEGMENT_m11:
				// nothing for now - video segment data are native video files
				break;
		}
	}
	
	// read segment records
	if (seg->flags & LH_READ_SEGMENT_RECORDS_MASK_m11)
		if (seg->record_indices_fps != NULL && seg->record_data_fps != NULL)
			read_record_data_m11((LEVEL_HEADER_m11 *) seg, slice);

	return(seg);
}


SESSION_m11	*read_session_m11(SESSION_m11 *sess, TIME_SLICE_m11 *slice, ...)  // varargs: void *file_list, si4 list_len, ui8 flags, si1 *password
{
	TERN_m11			open_session, free_session;
	si1                             *password, num_str[FILE_NUMBERING_DIGITS_m11 + 1];
	si1				tmp_str[FULL_FILE_NAME_BYTES_m11];
	si4                             i, j, list_len, seg_idx;
	ui8                             flags;
	void				*file_list;
	va_list				args;
	CHANNEL_m11			*chan;
	UNIVERSAL_HEADER_m11		*uh;
	SEGMENTED_SESS_RECS_m11		*ssr;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// open session
	open_session = free_session = FALSE_m11;
	if (sess == NULL)
		open_session = free_session = TRUE_m11;
	else if (!(sess->flags & LH_OPEN_m11)) {
		free_session_m11(sess, FALSE_m11);
		open_session = TRUE_m11;
	}
	if (open_session == TRUE_m11) {
		// get varargs
		va_start(args, slice);
		file_list = va_arg(args, void *);
		list_len = va_arg(args, si4);
		flags = va_arg(args, ui8);
		password = va_arg(args, si1 *);
		va_end(args);
		// open session
		sess = open_session_m11(sess, slice, file_list, list_len, flags, password);
		if (sess == NULL) {
			error_message_m11("%s(): error opening session\n", __FUNCTION__);
			return(NULL);
		}
	} else {  // process time slice (passed slice is not modified)
		if (slice == NULL) {
			if (all_zeros_m11((ui1 *) &sess->time_slice, (si4) sizeof(TIME_SLICE_m11)) == TRUE_m11)
				initialize_time_slice_m11(&sess->time_slice);  // read whole session
		} else {  // passed slice supersedes structure slice
			sess->time_slice = *slice;  // passed slice is not modified
		}
		if (sess->time_slice.conditioned == FALSE_m11)
			condition_time_slice_m11(slice);
	}
	slice = &sess->time_slice;

	// set global sample/frame number reference channel (before get segment range)
	if ((globals_m11->reference_channel->flags & LH_CHANNEL_ACTIVE_m11) == 0) {
		if (globals_m11->reference_channel->type_code == TIME_SERIES_CHANNEL_TYPE_m11)
			change_reference_channel_m11(sess, NULL, NULL, DEFAULT_TIME_SERIES_CHANNEL_m11);
		else
			change_reference_channel_m11(sess, NULL, NULL, DEFAULT_VIDEO_CHANNEL_m11);
	}

	// get segment range
	if (slice->number_of_segments == UNKNOWN_m11) {
		if (get_segment_range_m11((LEVEL_HEADER_m11 *) sess, slice) == 0) {
			if (free_session == TRUE_m11)
				free_session_m11(sess, TRUE_m11);
			return(NULL);
		}
	}
	seg_idx = get_segment_index_m11(slice->start_segment_number);
	if (seg_idx == FALSE_m11) {
		if (free_session == TRUE_m11)
			free_session_m11(sess, TRUE_m11);
		return(NULL);
	}

	// update for variable frequencies on active channel set
	frequencies_vary_m11(sess);
	if (globals_m11->time_series_frequencies_vary == TRUE_m11 && globals_m11->reference_channel->type_code == TIME_SERIES_CHANNEL_TYPE_m11) {
		if (get_search_mode_m11(slice) == SAMPLE_SEARCH_m11) {
			slice->start_time = uutc_for_sample_number_m11((LEVEL_HEADER_m11 *) sess, slice->start_sample_number, FIND_START_m11);
			slice->end_time = uutc_for_sample_number_m11((LEVEL_HEADER_m11 *) sess, slice->end_sample_number, FIND_END_m11);
			slice->start_sample_number = slice->end_sample_number = SAMPLE_NUMBER_NO_ENTRY_m11;
		}
	} else if (globals_m11->video_frame_rates_vary == TRUE_m11 && globals_m11->reference_channel->type_code == VIDEO_CHANNEL_TYPE_m11) {
		if (get_search_mode_m11(slice) == FRAME_SEARCH_m11) {
			slice->start_time = uutc_for_frame_number_m11((LEVEL_HEADER_m11 *) sess, slice->start_frame_number, FIND_START_m11);
			slice->end_time = uutc_for_frame_number_m11((LEVEL_HEADER_m11 *) sess, slice->end_frame_number, FIND_END_m11);
			slice->start_frame_number = slice->end_frame_number = FRAME_NUMBER_NO_ENTRY_m11;
		}
	}

	// read time series channels
	for (i = 0; i < sess->number_of_time_series_channels; ++i) {
		chan = sess->time_series_channels[i];
		if (chan->flags & LH_CHANNEL_ACTIVE_m11) {
			if (read_channel_m11(chan, slice) == NULL) {
				if (free_session == TRUE_m11) {
					free_session_m11(sess, TRUE_m11);
				} else if (chan != NULL) {
					if (chan->time_slice.number_of_segments == EMPTY_SLICE_m11)
						sess->time_slice.number_of_segments = EMPTY_SLICE_m11;
				}
				return(NULL);
			}
		}
	}
	
	// read video channels
	for (i = 0; i < sess->number_of_video_channels; ++i) {
		chan = sess->video_channels[i];
		if (chan->flags & LH_CHANNEL_ACTIVE_m11) {
			if (read_channel_m11(chan, slice) == NULL) {
				if (free_session == TRUE_m11) {
					free_session_m11(sess, TRUE_m11);
				} else if (chan != NULL) {
					if (chan->time_slice.number_of_segments == EMPTY_SLICE_m11)
						sess->time_slice.number_of_segments = EMPTY_SLICE_m11;
				}
				return(NULL);
			}
		}
	}

	// update session slice
	chan = globals_m11->reference_channel;
	if ((chan->flags & LH_CHANNEL_ACTIVE_m11) == 0)
		chan = get_active_channel_m11(sess, DEFAULT_CHANNEL_m11);
	slice->start_time = chan->time_slice.start_time;
	slice->end_time = chan->time_slice.end_time;
	slice->start_segment_number = chan->time_slice.start_segment_number;
	slice->end_segment_number = chan->time_slice.end_segment_number;
	slice->number_of_segments = TIME_SLICE_SEGMENT_COUNT_m11(slice);
	if (globals_m11->time_series_frequencies_vary == FALSE_m11 && globals_m11->reference_channel->type_code == TIME_SERIES_CHANNEL_TYPE_m11) {
		slice->start_sample_number = chan->time_slice.start_sample_number;
		slice->end_sample_number = chan->time_slice.end_sample_number;
	} else if (globals_m11->video_frame_rates_vary == FALSE_m11 && globals_m11->reference_channel->type_code == VIDEO_CHANNEL_TYPE_m11) {
		slice->start_frame_number = chan->time_slice.start_frame_number;
		slice->end_frame_number = chan->time_slice.end_frame_number;
	}

	// read session record data
	if (sess->flags & LH_READ_SESSION_RECORDS_MASK_m11)
		if (sess->record_indices_fps != NULL && sess->record_data_fps != NULL)
			read_record_data_m11((LEVEL_HEADER_m11 *) sess, slice);
		
	// read segmented session record data
	ssr = sess->segmented_sess_recs;
	if (sess->flags & LH_READ_SEGMENTED_SESS_RECS_MASK_m11 && ssr != NULL) {
		for (i = slice->start_segment_number, j = seg_idx; i <= slice->end_segment_number; ++i, ++j) {
			// allocate new segment records
			if (ssr->record_indices_fps[j] == NULL && ssr->record_data_fps[j] == NULL) {
				numerical_fixed_width_string_m11(num_str, FILE_NUMBERING_DIGITS_m11, i);
				sprintf_m11(tmp_str, "%s/%s_s%s.%s", ssr->path, ssr->name, num_str, RECORD_INDICES_FILE_TYPE_STRING_m11);
				if (file_exists_m11(tmp_str) == FILE_EXISTS_m11)
					ssr->record_indices_fps[j] = read_file_m11(ssr->record_indices_fps[j], tmp_str, 0, 0, 0, (LEVEL_HEADER_m11 *) ssr, NULL, USE_GLOBAL_BEHAVIOR_m11);
				sprintf_m11(tmp_str, "%s/%s_s%s.%s", ssr->path, ssr->name, num_str, RECORD_DATA_FILE_TYPE_STRING_m11);
				if (file_exists_m11(tmp_str) == FILE_EXISTS_m11)
					ssr->record_data_fps[j] = read_file_m11(ssr->record_data_fps[j], tmp_str, 0, 0, 0, (LEVEL_HEADER_m11 *) ssr, NULL, USE_GLOBAL_BEHAVIOR_m11);
			}
			if (ssr->record_indices_fps[j] != NULL && ssr->record_data_fps[j] != NULL)
				read_record_data_m11((LEVEL_HEADER_m11 *) ssr, slice, i);
		}
	}
	
	// update ephemeral data (session record ephemeral data updated on session / segment open)
	if (sess->flags & LH_GENERATE_EPHEMERAL_DATA_m11) {
		// time series ephemeral data
		for (i = 0; i < sess->number_of_time_series_channels; ++i) {
			chan = sess->time_series_channels[i];
			if (chan->flags & LH_CHANNEL_ACTIVE_m11) {
				merge_universal_headers_m11(sess->time_series_metadata_fps, chan->metadata_fps, NULL);
				merge_metadata_m11(sess->time_series_metadata_fps, chan->metadata_fps, NULL);
				chan->flags &= ~LH_UPDATE_EPHEMERAL_DATA_m11;  // clear flag
			}
		}
		// video ephemeral data
		for (i = 0; i < sess->number_of_video_channels; ++i) {
			chan = sess->video_channels[i];
			if (chan->flags & LH_CHANNEL_ACTIVE_m11) {
				merge_universal_headers_m11(sess->video_metadata_fps, chan->metadata_fps, NULL);
				merge_metadata_m11(sess->video_metadata_fps, chan->metadata_fps, NULL);
				chan->flags &= ~LH_UPDATE_EPHEMERAL_DATA_m11;  // clear flag
			}
		}
		// fix session ephemeral universal header (from merge functions)
		if (sess->number_of_time_series_channels) {
			uh = sess->time_series_metadata_fps->universal_header;
			uh->type_code = TIME_SERIES_METADATA_FILE_TYPE_CODE_m11;
			uh->segment_number = UNIVERSAL_HEADER_SESSION_LEVEL_CODE_m11;
			uh->session_UID = globals_m11->session_UID;
			uh->channel_UID = uh->segment_UID = UID_NO_ENTRY_m11;
		}
		if (sess->number_of_video_channels) {
			uh = sess->video_metadata_fps->universal_header;
			uh->type_code = VIDEO_METADATA_FILE_TYPE_CODE_m11;
			uh->segment_number = UNIVERSAL_HEADER_SESSION_LEVEL_CODE_m11;
			uh->session_UID = globals_m11->session_UID;
			uh->channel_UID = uh->segment_UID = UID_NO_ENTRY_m11;
		}
	}
	
	// verbose
	if (globals_m11->verbose == TRUE_m11) {
		if (sess->time_series_metadata_fps != NULL) {
			printf_m11("--------- Session Time Series Universal Header ---------\n");
			show_universal_header_m11(sess->time_series_metadata_fps, NULL);
			printf_m11("------------ Session Time Series Metadata --------------\n");
			show_metadata_m11(sess->time_series_metadata_fps, NULL, 0);
		}
		if (sess->video_metadata_fps != NULL) {
			printf_m11("------------ Session Video Universal Header ------------\n");
			show_universal_header_m11(sess->video_metadata_fps, NULL);
			printf_m11("--------------- Session Video Metadata -----------------\n");
			show_metadata_m11(sess->video_metadata_fps, NULL, 0);
		}
	}

	sess->last_access_time = current_uutc_m11();

	return(sess);
}


si8     read_time_series_data_m11(SEGMENT_m11 *seg, TIME_SLICE_m11 *slice)
{
	TERN_m11				cps_caching, scale;
	ui4					cached_block_samples;
	si4					cached_block_cnt, *to_ptr, *from_ptr, *si4_p, to_idx, from_idx;
	si4					first_cached_block, first_cached_block_idx, last_cached_block, last_cached_block_idx;
	si8                                     i, j, terminal_ts_ind, n_samps, n_blocks, start_offset;
	si8                                     start_block, end_block, read_start_block, read_end_block, read_n_blocks, compressed_data_bytes;
	si8					local_start_idx, local_end_idx, seg_start_samp_num, n_cached_samples, cache_offset;
	sf8					scale_factor;
	CMP_CACHE_BLOCK_INFO_m11		*cached_blocks;
	FILE_PROCESSING_STRUCT_m11		*tsd_fps, *tsi_fps;
	TIME_SERIES_INDEX_m11			*tsi;
	TIME_SERIES_METADATA_SECTION_2_m11	*tmd2;
	CMP_PROCESSING_STRUCT_m11		*cps;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (seg == NULL) {
		error_message_m11("%s(): SEGMENT_m11 structure is NULL\n", __FUNCTION__);
		return(-1);
	}
	if ((tsd_fps = seg->time_series_data_fps) == NULL) {
		error_message_m11("%s(): time series data FILE_PROCESSING_STRUCT_m11 is NULL\n", __FUNCTION__);
		return(-1);
	}
	if ((tsi_fps = seg->time_series_indices_fps) == NULL) {
		error_message_m11("%s(): time series indices FILE_PROCESSING_STRUCT_m11 is NULL\n", __FUNCTION__);
		return(-1);
	}
	if (seg->flags & LH_NO_CPS_PTR_RESET_m11){
		error_message_m11("%s(): CPS pointer resets are required for this function \n", __FUNCTION__);
		return(-1);
	}
	
	// find start and end blocks (block index is for block containing sample index)
	tsi = tsi_fps->time_series_indices;
	seg_start_samp_num = seg->metadata_fps->metadata->time_series_section_2.absolute_start_sample_number;
	start_block = find_index_m11(seg, slice->start_sample_number, SAMPLE_SEARCH_m11);
	if (start_block < 0) {  // before first block
		start_block = 0;
		local_start_idx = 0;
	} else {
		local_start_idx = slice->start_sample_number - seg_start_samp_num;
	}
	terminal_ts_ind = tsi_fps->universal_header->number_of_entries - 1;
	end_block = find_index_m11(seg, slice->end_sample_number, SAMPLE_SEARCH_m11);
	if (end_block == terminal_ts_ind) {  // after last block (points to terminal index)
		local_end_idx = tsi[end_block].start_sample_number - 1;  // terminal index start_sample_number (== total samples in segment)
		end_block = terminal_ts_ind - 1;  // index of last true block
	} else {
		local_end_idx = slice->end_sample_number - seg_start_samp_num;
	}
	n_blocks = (end_block - start_block) + 1;

	// allocate cps
	n_samps = tsi[end_block + 1].start_sample_number - tsi[start_block].start_sample_number;
	start_offset = REMOVE_DISCONTINUITY_m11(tsi[start_block].file_offset);
	tmd2 = &seg->metadata_fps->metadata->time_series_section_2;
	cached_block_cnt = 0;
	read_start_block = start_block;
	read_end_block = end_block;
	if (tsd_fps->parameters.cps == NULL) {
		compressed_data_bytes = REMOVE_DISCONTINUITY_m11(tsi[end_block + 1].file_offset) - start_offset;
		cps = CMP_allocate_processing_struct_m11(tsd_fps, CMP_DECOMPRESSION_MODE_m11, n_samps, compressed_data_bytes, tmd2->maximum_block_keysample_bytes, tmd2->maximum_block_samples, NULL, NULL);
		if ((cps_caching = cps->directives.cps_caching) == TRUE_m11) {
			cached_blocks = cps->parameters.cached_blocks = (CMP_CACHE_BLOCK_INFO_m11 *) calloc_m11((size_t) n_blocks, sizeof(CMP_CACHE_BLOCK_INFO_m11), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
			cps->parameters.cached_block_list_len = n_blocks;
		}
	} else {
		cps = tsd_fps->parameters.cps;
		cps_caching = cps->directives.cps_caching;
		if (cps_caching == TRUE_m11) {
			if (cps->parameters.cached_block_list_len < n_blocks) {
				cps->parameters.cached_blocks = (CMP_CACHE_BLOCK_INFO_m11 *) realloc_m11((void *) cps->parameters.cached_blocks, (size_t) n_blocks * sizeof(CMP_CACHE_BLOCK_INFO_m11), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
				cps->parameters.cached_block_list_len = n_blocks;
			}
			cached_blocks = cps->parameters.cached_blocks;
			cached_block_cnt = cps->parameters.cached_block_cnt;
			if (cached_block_cnt) {
				// reallocate cache manually so CMP_reallocate_processing_struct_m11() does not free
				if (n_samps > cps->parameters.allocated_decompressed_samples) {
					cps->decompressed_data = cps->parameters.cache = (si4 *) realloc_m11((void *) cps->parameters.cache, n_samps * sizeof(si4), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
					cps->parameters.allocated_decompressed_samples = n_samps;
				}
				first_cached_block = cached_blocks[0].block_number;
				last_cached_block = cached_blocks[cached_block_cnt - 1].block_number;
				if (first_cached_block <= end_block && last_cached_block >= start_block) {
					last_cached_block_idx = end_block - first_cached_block;
					if (last_cached_block_idx >= cached_block_cnt)
						last_cached_block_idx = cached_block_cnt - 1;
					last_cached_block = cached_blocks[last_cached_block_idx].block_number;
					first_cached_block_idx = start_block - first_cached_block;
					if (first_cached_block_idx < 0)
						first_cached_block_idx = 0;
					first_cached_block = cached_blocks[first_cached_block_idx].block_number;
					cached_block_cnt = (last_cached_block - first_cached_block) + 1;
					// full request is cached (don't update cache until there's at least one uncached block)
					if (first_cached_block <= start_block && last_cached_block >= end_block) {  // full request is cached
						cache_offset = cached_blocks[first_cached_block_idx].cache_offset + (local_start_idx - tsi[start_block].start_sample_number);
						cps->decompressed_ptr = cps->decompressed_data = cps->parameters.cache + cache_offset;
						n_samps = (local_end_idx - local_start_idx) + 1;
						return(n_samps);
					}
					// shift samples
					to_ptr = cps->parameters.cache + (tsi[first_cached_block].start_sample_number - tsi[start_block].start_sample_number);
					from_ptr = cps->parameters.cache + cached_blocks[first_cached_block_idx].cache_offset;
					n_cached_samples = (cached_blocks[last_cached_block_idx].cache_offset + (si8) cached_blocks[last_cached_block_idx].block_samples) - cached_blocks[first_cached_block_idx].cache_offset;
					if (to_ptr > from_ptr) {  // right shift (move backwards)
						to_ptr += n_cached_samples;
						from_ptr += n_cached_samples;
						for (i = n_cached_samples; i--;)
							*--to_ptr = *--from_ptr;
					} else if (to_ptr < from_ptr) {  // left shift (move forwards)
						for (i = n_cached_samples; i--;)
							*to_ptr++ = *from_ptr++;
					}  // else don't move anything
					// shift cached block infos
					to_idx = first_cached_block - start_block;
					from_idx = first_cached_block_idx;
					first_cached_block_idx = to_idx;
					last_cached_block_idx = (first_cached_block_idx + cached_block_cnt) - 1;
					if (to_idx > from_idx) {  // right shift (move backwards)
						to_idx += cached_block_cnt;
						from_idx += cached_block_cnt;
						for (i = 0; i < cached_block_cnt; ++i)
							cached_blocks[--to_idx] = cached_blocks[--from_idx];
					} else if (to_idx < from_idx) {  // left shift (move forwards)
						for (i = 0; i < cached_block_cnt; ++i)
							cached_blocks[to_idx++] = cached_blocks[from_idx++];
					}  // else don't move anything
					// set uncached blocks to zero samples (used as flag to cache)
					for (i = 0; i < first_cached_block_idx; ++i)
						cached_blocks[i].block_samples = 0;
					for (i = last_cached_block_idx + 1; i < n_blocks; ++i)
						cached_blocks[i].block_samples = 0;
					// full request is cached (cache updated here)
				} else {
					cached_block_cnt = 0;  // none of requested blocks in requested range
				}
			}
		}
		// limit read, if possible (if cached blocks are in middle, they are read - but this should be uncommon)
		if (cached_block_cnt) {
			if (first_cached_block == start_block) {
				read_start_block = last_cached_block + 1;
				for (i = 0; i <= last_cached_block_idx; ++i)
					cached_blocks[i].data_read = FALSE_m11;
				for (; i < n_blocks; ++i)
					cached_blocks[i].data_read = TRUE_m11;
			} else if (last_cached_block == end_block) {
				read_end_block = first_cached_block - 1;
				for (i = 0; i < first_cached_block_idx; ++i)
					cached_blocks[i].data_read = TRUE_m11;
				for (; i < n_blocks; ++i)
					cached_blocks[i].data_read = FALSE_m11;
			} else {
				for (i = 0; i < n_blocks; ++i)
					cached_blocks[i].data_read = FALSE_m11;
			}
			start_offset = REMOVE_DISCONTINUITY_m11(tsi[read_start_block].file_offset);
			compressed_data_bytes = REMOVE_DISCONTINUITY_m11(tsi[read_end_block + 1].file_offset) - start_offset;
		} else {
			start_offset = REMOVE_DISCONTINUITY_m11(tsi[start_block].file_offset);
			compressed_data_bytes = REMOVE_DISCONTINUITY_m11(tsi[end_block + 1].file_offset) - start_offset;
		}
		CMP_update_CPS_pointers_m11(tsd_fps, CMP_RESET_DECOMPRESSED_PTR_m11 | CMP_RESET_BLOCK_HEADER_PTR_m11);
		cps = CMP_reallocate_processing_struct_m11(tsd_fps, CMP_DECOMPRESSION_MODE_m11, n_samps, tmd2->maximum_block_samples);
	}

	// read in compressed data
	read_n_blocks = (read_end_block - read_start_block) + 1;
	read_file_m11(tsd_fps, NULL, start_offset, compressed_data_bytes, read_n_blocks, (LEVEL_HEADER_m11 *) seg, NULL, USE_GLOBAL_BEHAVIOR_m11);

	// set limit on first block
	cps->parameters.block_start_index = local_start_idx - tsi[start_block].start_sample_number;
	
	scale = FALSE_m11;
	if (cps->directives.convert_to_native_units == TRUE_m11) {
		scale_factor = tmd2->amplitude_units_conversion_factor;
		if (scale_factor != 1.0 && scale_factor != TIME_SERIES_METADATA_AMPLITUDE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m11)
			scale = TRUE_m11;
	}
		
	// loop over blocks
	cache_offset = 0;
	cps->decompressed_ptr = cps->parameters.cache;
	for (i = 0, j = start_block; i < n_blocks; ++i, ++j) {
		if (cached_block_cnt) {
			if ((cached_block_samples = cached_blocks[i].block_samples)) {
				cps->decompressed_ptr += cached_block_samples;
				if (cached_blocks[i].data_read == TRUE_m11)  // data read because in middle, but block is still cached
					CMP_update_CPS_pointers_m11(tsd_fps, CMP_UPDATE_BLOCK_HEADER_PTR_m11);
				cached_blocks[i].cache_offset = cache_offset;
				cache_offset += cached_block_samples;
				continue;
			}
		}
		// set limit on last block
		if (j == end_block)
			cps->parameters.block_end_index = local_end_idx - tsi[j].start_sample_number;
		CMP_decode_m11(tsd_fps);

		if (cps_caching == TRUE_m11) {
			cached_blocks[i].cache_offset = cache_offset;
			cached_blocks[i].block_samples = cps->block_header->number_of_samples;
			cache_offset += cached_blocks[i].block_samples;
			cached_blocks[i].block_number = j;
		}
		if (scale == TRUE_m11) {  // scale to native units
			i = cps->block_header->number_of_samples;
			si4_p = cps->decompressed_ptr;
			while (i--) {
				*si4_p = CMP_round_si4_m11((sf8) *si4_p * scale_factor);
				++si4_p;
			}
		}
		CMP_update_CPS_pointers_m11(tsd_fps, CMP_UPDATE_BLOCK_HEADER_PTR_m11 | CMP_UPDATE_DECOMPRESSED_PTR_m11);
	}
	if (cps_caching == TRUE_m11)
		cps->parameters.cached_block_cnt = n_blocks;  // all blocks now cached
		
	n_samps = (local_end_idx - local_start_idx) + 1;  // trim (it did contain total samps in blocks)
	return(n_samps);
}


TERN_m11    recover_passwords_m11(si1 *L3_password, UNIVERSAL_HEADER_m11 *universal_header)
{
	ui1     hash[SHA_HASH_BYTES_m11], L3_hash[SHA_HASH_BYTES_m11];
	si1     L3_password_bytes[PASSWORD_BYTES_m11], hex_str[HEX_STRING_BYTES_m11(PASSWORD_BYTES_m11)];
	si1     putative_L1_password_bytes[PASSWORD_BYTES_m11], putative_L2_password_bytes[PASSWORD_BYTES_m11];
	si4     i;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (check_password_m11(L3_password) == FALSE_m11)
		return(FALSE_m11);
	
	// get terminal bytes
	extract_terminal_password_bytes_m11(L3_password, L3_password_bytes);
	
	// get level 3 password hash
	SHA_hash_m11((ui1 *) L3_password_bytes, PASSWORD_BYTES_m11, L3_hash);  // generate SHA-256 hash of level 3 password bytes
	
	// check for level 1 access
	for (i = 0; i < PASSWORD_BYTES_m11; ++i)  // xor with level 3 password validation field
		putative_L1_password_bytes[i] = L3_hash[i] ^ universal_header->level_3_password_validation_field[i];
	
	SHA_hash_m11((ui1 *) putative_L1_password_bytes, PASSWORD_BYTES_m11, hash); // generate SHA-256 hash of putative level 1 password
	
	for (i = 0; i < PASSWORD_VALIDATION_FIELD_BYTES_m11; ++i)  // compare with stored level 1 hash
		if (hash[i] != universal_header->level_1_password_validation_field[i])
			break;
	if (i == PASSWORD_VALIDATION_FIELD_BYTES_m11) {  // Level 1 password recovered
		generate_hex_string_m11((ui1 *) putative_L1_password_bytes, PASSWORD_BYTES_m11, hex_str);
		message_m11("Level 1 password (bytes): '%s' (%s)", putative_L1_password_bytes, hex_str);
		return(TRUE_m11);
	}
	
	// invalid for level 1 (alone) => check if level 2 password
	memcpy(putative_L2_password_bytes, putative_L1_password_bytes, PASSWORD_BYTES_m11);
	for (i = 0; i < PASSWORD_BYTES_m11; ++i)  // xor with level 2 password validation field
		putative_L1_password_bytes[i] = hash[i] ^ universal_header->level_2_password_validation_field[i];
	
	SHA_hash_m11((ui1 *) putative_L1_password_bytes, PASSWORD_BYTES_m11, hash); // generate SHA-256 hash of putative level 1 password
	
	for (i = 0; i < PASSWORD_VALIDATION_FIELD_BYTES_m11; ++i)  // compare with stored level 1 hash
		if (hash[i] != universal_header->level_1_password_validation_field[i])
			break;
	
	if (i == PASSWORD_VALIDATION_FIELD_BYTES_m11) {  // Level 2 password valid
		generate_hex_string_m11((ui1 *)putative_L1_password_bytes, PASSWORD_BYTES_m11, hex_str);
		message_m11("Level 1 password (bytes): '%s' (%s)", putative_L1_password_bytes, hex_str);
		generate_hex_string_m11((ui1 *)putative_L2_password_bytes, PASSWORD_BYTES_m11, hex_str);
		message_m11("Level 2 password (bytes): '%s' (%s)", putative_L2_password_bytes, hex_str);
	} else {
		warning_message_m11("%s(): the passed password is not valid for Level 3 access\n", __FUNCTION__, __LINE__);
		return(FALSE_m11);
	}
	
	return(TRUE_m11);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void	remove_recording_time_offset_m11(si8 *time)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (*time != UUTC_NO_ENTRY_m11)
		*time += globals_m11->recording_time_offset;
	
	return;
}


si8     sample_number_for_uutc_m11(LEVEL_HEADER_m11 *level_header, si8 target_uutc, ui4 mode, ...)  // varargs: si8 ref_sample_number, si8 ref_uutc, sf8 sampling_frequency
{
	si1			tmp_str[FULL_FILE_NAME_BYTES_m11], num_str[FILE_NUMBERING_DIGITS_m11 + 1];
	si4			seg_num, seg_idx;
	si8                     sample_number, n_inds, i, absolute_numbering_offset;
	si8			ref_sample_number, ref_uutc;
	sf8                     tmp_sf8, sampling_frequency, rounded_samp_num, samp_num_eps;
	ui4			mask;
	va_list			args;
	TIME_SERIES_INDEX_m11	*tsi;
	SEGMENT_m11		*seg;
	CHANNEL_m11		*chan;
	SESSION_m11		*sess;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// sample_number_for_uutc_m11(NULL, si8 target_uutc, ui4 mode, si8 ref_index, si8 ref_uutc, sf8 sampling_frequency);
	// returns sample number extrapolated from ref_index (relative / absolute is determined by magnitude of reference values)
	
	// sample_number_for_uutc_m11(seg, target_uutc, mode);
	// returns sample number extrapolated from closest time series index in reference frame specified by mode
	
	// mode FIND_ABSOLUTE_m11 (default): session (or reference) relative sample numbering
	// mode FIND_RELATIVE_m11: segment relative sample numbering
	// mode FIND_CURRENT_m11 (default): sample period within which the target_uutc falls
	// mode FIND_CLOSEST_m11: sample number closest to the target_uutc
	// mode FIND_NEXT_m11: sample number following the sample period within which the target_uutc falls ( == FIND_CURRENT_m11 + 1)
	// mode FIND_PREVIOUS_m11: sample number preceding the sample period within which the target_uutc falls ( == FIND_CURRENT_m11 - 1)

	if (level_header == NULL) {  // reference points passed
		va_start(args, mode);
		ref_sample_number = va_arg(args, si8);
		ref_uutc = va_arg(args, si8);
		sampling_frequency = va_arg(args, sf8);
		va_end(args);
		absolute_numbering_offset = 0;
		tsi = NULL;
	} else {  // level header passed
		switch (level_header->type_code) {
			case LH_TIME_SERIES_SEGMENT_m11:
				seg = (SEGMENT_m11 *) level_header;
				break;
			case LH_TIME_SERIES_CHANNEL_m11:
			case LH_SESSION_m11:
				seg_num = segment_for_uutc_m11(level_header, target_uutc);
				seg_idx = get_segment_index_m11(seg_num);
				if (seg_idx == FALSE_m11)
					return(SAMPLE_NUMBER_NO_ENTRY_m11);
				if (level_header->type_code == LH_TIME_SERIES_CHANNEL_m11) {
					chan = (CHANNEL_m11 *) level_header;
				} else {  // SESSION_m11
					chan = globals_m11->reference_channel;
					if (chan->type_code != LH_TIME_SERIES_CHANNEL_m11) {
						sess = (SESSION_m11 *) level_header;
						chan = sess->time_series_channels[0];
					}
				}
				seg = chan->segments[seg_idx];
				break;
			case LH_VIDEO_CHANNEL_m11:
			case LH_VIDEO_SEGMENT_m11:
				return(frame_number_for_uutc_m11(level_header, target_uutc, mode));
			default:
				warning_message_m11("%s(): invalid level type\n", __FUNCTION__);
				return(SAMPLE_NUMBER_NO_ENTRY_m11);
		}
		// return(SAMPLE_NUMBER_NO_ENTRY_m11);
		if (seg == NULL) {  // channel or session
			numerical_fixed_width_string_m11(num_str, FILE_NUMBERING_DIGITS_m11, seg_num);
			sprintf_m11(tmp_str, "%s/%s_s%s.%s", chan->path, chan->name, num_str, TIME_SERIES_SEGMENT_DIRECTORY_TYPE_STRING_m11);
			seg = chan->segments[seg_idx] = open_segment_m11(NULL, NULL, tmp_str, chan->flags, NULL);
		} else if (!(seg->flags & LH_OPEN_m11)) {  // closed segment
			open_segment_m11(seg, NULL, NULL, seg->flags, NULL);
		}
		if (seg == NULL) {
			warning_message_m11("%s(): can't open segment\n", __FUNCTION__);
			return(SAMPLE_NUMBER_NO_ENTRY_m11);
		}

		tsi = seg->time_series_indices_fps->time_series_indices;
		if (tsi == NULL) {
			warning_message_m11("%s(): time series indices are NULL => returning SAMPLE_NUMBER_NO_ENTRY_m11\n", __FUNCTION__);
			return(SAMPLE_NUMBER_NO_ENTRY_m11);
		}
		n_inds = seg->time_series_indices_fps->universal_header->number_of_entries - 1;  // account for terminal index here - cleaner code below
		if (mode & FIND_RELATIVE_m11)
			absolute_numbering_offset = 0;
		else  // FIND_ABSOLUTE_m11 (default)
			absolute_numbering_offset = seg->metadata_fps->metadata->time_series_section_2.absolute_start_sample_number;
		
		i = find_index_m11(seg, target_uutc, TIME_SEARCH_m11);
		if (i == -1)  // target time earlier than segment start => return segment start sample
			return(absolute_numbering_offset);

		tsi += i;
		if (i == n_inds) {  // target time later than segment end => return segment end sample number
			i = (tsi->start_sample_number - 1) + absolute_numbering_offset;
			return(i);
		}

		ref_uutc = tsi->start_time;
		ref_sample_number = tsi->start_sample_number;
		++tsi;  // advance to next index
		if (tsi->file_offset > 0) {  // get local sampling frequency, unless discontinuity
			sampling_frequency = (sf8) (tsi->start_sample_number - ref_sample_number);
			sampling_frequency /= ((sf8) (tsi->start_time - ref_uutc) / (sf8) 1e6);
		} else {
			sampling_frequency = seg->metadata_fps->metadata->time_series_section_2.sampling_frequency;
		}
	}
	
	// round up if very close to next sample
	tmp_sf8 = ((sf8) (target_uutc - ref_uutc) / (sf8) 1e6) * sampling_frequency;
	rounded_samp_num = (sf8) ((si8) (tmp_sf8 + (sf8) 0.5));
	samp_num_eps = rounded_samp_num - tmp_sf8;
	if (samp_num_eps > (sf8) 0.0)
		if (samp_num_eps < SAMPLE_NUMBER_EPS_m11)
			tmp_sf8 = rounded_samp_num;
	
	mask = (ui4) (FIND_CLOSEST_m11 | FIND_NEXT_m11 | FIND_CURRENT_m11 | FIND_PREVIOUS_m11);
	switch (mode & mask) {
		case FIND_CLOSEST_m11:
			tmp_sf8 += (sf8) 0.5;
			break;
		case FIND_NEXT_m11:
			tmp_sf8 += (sf8) 1.0;
			break;
		case FIND_PREVIOUS_m11:
			if ((tmp_sf8 -= (sf8) 1.0) < (sf8) 0.0)
				tmp_sf8 = (sf8) 0.0;
			break;
		case FIND_CURRENT_m11:
		default:
			break;
	}
	sample_number = ref_sample_number + (si8) tmp_sf8;
	if (tsi != NULL) {
		if (sample_number >= tsi->start_sample_number) {
			sample_number = tsi->start_sample_number;
			if (mode & (FIND_CURRENT_m11 | FIND_PREVIOUS_m11))
				--sample_number;  // these should not go into next index
		}
	}
	sample_number += absolute_numbering_offset;
	
	return(sample_number);
}


si4	search_Sgmt_records_m11(Sgmt_RECORD_m11 *Sgmt_records, TIME_SLICE_m11 *slice, ui4 search_mode)
{
	si1				seg_name[SEGMENT_BASE_FILE_NAME_BYTES_m11], md_file[FULL_FILE_NAME_BYTES_m11], num_str[FILE_NUMBERING_DIGITS_m11 + 1];
	si4				i, idx, low_idx, high_idx;
	si8				target;
	CHANNEL_m11			*chan;
	FILE_PROCESSING_STRUCT_m11	*md_fps;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// Note: this may seem like overkill, that a simple forward linear search would suffice,
	// but in theory there can be a large number of non-uniformly spaced segments.
	
	if (search_mode == TIME_SEARCH_m11) {
		// start segment
		target = slice->start_time;
		low_idx = 0;
		high_idx = globals_m11->number_of_session_segments - 1;
		if (target > Sgmt_records[high_idx].end_time) {
			slice->start_segment_number = SEGMENT_NUMBER_NO_ENTRY_m11;
			warning_message_m11("%s(): requested start time is after session end\n", __FUNCTION__);
			idx = 0;
		} else {
			if (target < Sgmt_records[0].start_time) {
				idx = 0;
			} else {
				do {  // binary search
					idx = (low_idx + high_idx) >> 1;
					if (Sgmt_records[idx].start_time > target)
						high_idx = idx;
					else
						low_idx = idx;
				} while ((high_idx - low_idx) > 1);
				if (target > Sgmt_records[low_idx].end_time)
				    idx = high_idx;
				else if (target < Sgmt_records[high_idx].start_time)
				    idx = low_idx;
			}
			slice->start_segment_number = idx + 1;
		}
		
		// end segment
		target = slice->end_time;
		low_idx = idx;
		high_idx = globals_m11->number_of_session_segments - 1;
		if (target < Sgmt_records[low_idx].start_time) {
			slice->end_segment_number = SEGMENT_NUMBER_NO_ENTRY_m11;
			warning_message_m11("%s(): requested end time precedes requested start time\n", __FUNCTION__);
		} else {
			if (target >= Sgmt_records[high_idx].end_time) {
				idx = high_idx;
			} else {
				do {  // binary search
					idx = (low_idx + high_idx) >> 1;
					if (Sgmt_records[idx].start_time > target)
						high_idx = idx;
					else
						low_idx = idx;
				} while ((high_idx - low_idx) > 1);
				if (target > Sgmt_records[low_idx].end_time)
				    idx = high_idx;
				else if (target < Sgmt_records[high_idx].start_time)
				    idx = low_idx;
			}
			slice->end_segment_number = idx + 1;
		}
	}
	else {  // search_mode == SAMPLE_SEARCH_m11
		
		// sample search required, but no sample data in Sgmt_records => fill it in (e.g from session records in variable frequency session)
		if (Sgmt_records[0].start_sample_number == SAMPLE_NUMBER_NO_ENTRY_m11) {
			chan = globals_m11->reference_channel;
			for (i = 0; i < globals_m11->number_of_session_segments; ++i) {
				numerical_fixed_width_string_m11(num_str, FILE_NUMBERING_DIGITS_m11, Sgmt_records[i].segment_number);
				sprintf_m11(seg_name, "%s_s%s", chan->name, num_str);
				sprintf_m11(md_file, "%s/%s.%s/%s.%s", chan->path, seg_name, TIME_SERIES_SEGMENT_DIRECTORY_TYPE_STRING_m11, seg_name, TIME_SERIES_METADATA_FILE_TYPE_STRING_m11);
				md_fps = read_file_m11(NULL, md_file, 0, 0, FPS_FULL_FILE_m11, NULL, NULL, USE_GLOBAL_BEHAVIOR_m11);
				if (md_fps == NULL)
					continue;
				Sgmt_records[i].end_sample_number = Sgmt_records[i].start_sample_number = md_fps->metadata->time_series_section_2.absolute_start_sample_number;
				Sgmt_records[i].end_sample_number += (md_fps->metadata->time_series_section_2.number_of_samples - 1);
				FPS_free_processing_struct_m11(md_fps, TRUE_m11);
			}
		}

		// start segment
		target = slice->start_sample_number;
		low_idx = 0;
		high_idx = globals_m11->number_of_session_segments - 1;
		if (target > Sgmt_records[high_idx].end_sample_number) {
			slice->start_segment_number = SEGMENT_NUMBER_NO_ENTRY_m11;
			warning_message_m11("%s(): requested start sample is after session end\n", __FUNCTION__);
			idx = 0;
		} else {
			if (target < Sgmt_records[0].start_sample_number) {
				idx = 0;
			} else {
				do {  // binary search
					idx = (low_idx + high_idx) >> 1;
					if (Sgmt_records[idx].start_sample_number > target)
						high_idx = idx;
					else
						low_idx = idx;
				} while ((high_idx - low_idx) > 1);
				if (target > Sgmt_records[low_idx].end_sample_number)
				    idx = high_idx;
				else if (target < Sgmt_records[high_idx].start_sample_number)
				    idx = low_idx;
			}
			slice->start_segment_number = idx + 1;
		}
		
		// end segment
		target = slice->end_sample_number;
		low_idx = idx;
		high_idx = globals_m11->number_of_session_segments - 1;
		if (target < Sgmt_records[low_idx].start_sample_number) {
			slice->end_segment_number = SEGMENT_NUMBER_NO_ENTRY_m11;
			warning_message_m11("%s(): requested end sample precedes requested start sample\n", __FUNCTION__);
		} else {
			if (target >= Sgmt_records[high_idx].end_sample_number) {
				idx = high_idx;
			} else {
				do {  // binary search
					idx = (low_idx + high_idx) >> 1;
					if (Sgmt_records[idx].start_sample_number > target)
						high_idx = idx;
					else
						low_idx = idx;
				} while ((high_idx - low_idx) > 1);
				if (target > Sgmt_records[low_idx].end_sample_number)
				    idx = high_idx;
				else if (target < Sgmt_records[high_idx].start_sample_number)
				    idx = low_idx;
			}
			slice->end_segment_number = idx + 1;
		}
	}

	// return number of segments
	if (slice->start_segment_number == SEGMENT_NUMBER_NO_ENTRY_m11 || slice->end_segment_number == SEGMENT_NUMBER_NO_ENTRY_m11)
		return(0);
	else
		return(TIME_SLICE_SEGMENT_COUNT_m11(slice));
}

		
si4	segment_for_frame_number_m11(LEVEL_HEADER_m11 *level_header, si8 target_frame)
{
	TERN_m11		get_Sgmt_recs;
	si4			idx, low_idx, high_idx;
	CHANNEL_m11		*chan;
	SESSION_m11		*sess;
	Sgmt_RECORD_m11		*Sgmt_records;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// Note: this may seem like overkill, that a simple forward linear search would suffice,
	// but in theory there can be a large number of non-uniformly spaced segments.
	
	switch (level_header->type_code) {
		case LH_TIME_SERIES_CHANNEL_m11:
			return(segment_for_sample_number_m11(level_header, target_frame));
			break;
		case LH_VIDEO_CHANNEL_m11:
			chan = (CHANNEL_m11 *) level_header;
			Sgmt_records = chan->Sgmt_records;
			if (Sgmt_records == NULL && chan->parent != NULL)
				Sgmt_records = ((SESSION_m11 *) chan->parent)->Sgmt_records;
			else
				Sgmt_records = build_Sgmt_records_array_m11(chan->record_indices_fps, chan->record_data_fps, chan);
			break;
		case LH_SESSION_m11:
			sess = (SESSION_m11 *) level_header;
			Sgmt_records = sess->Sgmt_records;
			get_Sgmt_recs = FALSE_m11;
			if (Sgmt_records == NULL)
				get_Sgmt_recs = TRUE_m11;
			else if (Sgmt_records[0].start_frame_number == FRAME_NUMBER_NO_ENTRY_m11)
				get_Sgmt_recs = TRUE_m11;
			if (get_Sgmt_recs == TRUE_m11) {
				chan = globals_m11->reference_channel;
				if (chan->type_code != LH_VIDEO_CHANNEL_m11)
					chan = sess->video_channels[0];
				if (chan->Sgmt_records != NULL)
					Sgmt_records = chan->Sgmt_records;
				else
					Sgmt_records = chan->Sgmt_records = build_Sgmt_records_array_m11(chan->record_indices_fps, chan->record_data_fps, chan);
			}
			break;
		default:
			warning_message_m11("%s(): invalid level type\n", __FUNCTION__);
			return(SEGMENT_NUMBER_NO_ENTRY_m11);
	}

	low_idx = 0;
	high_idx = globals_m11->number_of_session_segments - 1;
	if (target_frame <= Sgmt_records[0].start_frame_number)
		return(1);  // return first segment if requested frame number <= session start
	if (target_frame >= Sgmt_records[high_idx].end_frame_number)
		return(high_idx + 1);  // return last segment if requested frame number <= session start
		
	do {  // binary search
		idx = (low_idx + high_idx) >> 1;
		if (Sgmt_records[idx].start_frame_number > target_frame)
			high_idx = idx;
		else
			low_idx = idx;
	} while ((high_idx - low_idx) > 1);
	
	if (target_frame > Sgmt_records[low_idx].end_frame_number)
	    idx = high_idx;
	else if (target_frame < Sgmt_records[high_idx].start_frame_number)
	    idx = low_idx;

	return(idx + 1);
}


si4	segment_for_sample_number_m11(LEVEL_HEADER_m11 *level_header, si8 target_sample)
{
	TERN_m11		get_Sgmt_recs;
	si4			idx, low_idx, high_idx;
	CHANNEL_m11		*chan;
	SESSION_m11		*sess;
	Sgmt_RECORD_m11		*Sgmt_records;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// Note: this may seem like overkill, that a simple forward linear search would suffice,
	// but in theory there can be a large number of non-uniformly spaced segments.
	
	switch (level_header->type_code) {
		case LH_VIDEO_CHANNEL_m11:
			return(segment_for_frame_number_m11(level_header, target_sample));
			break;
		case LH_TIME_SERIES_CHANNEL_m11:
			chan = (CHANNEL_m11 *) level_header;
			Sgmt_records = chan->Sgmt_records;
			if (Sgmt_records == NULL && chan->parent != NULL)
				Sgmt_records = ((SESSION_m11 *) chan->parent)->Sgmt_records;
			else
				Sgmt_records = build_Sgmt_records_array_m11(chan->record_indices_fps, chan->record_data_fps, chan);
			break;
		case LH_SESSION_m11:
			sess = (SESSION_m11 *) level_header;
			Sgmt_records = sess->Sgmt_records;
			get_Sgmt_recs = FALSE_m11;
			if (Sgmt_records == NULL)
				get_Sgmt_recs = TRUE_m11;
			else if (Sgmt_records[0].start_sample_number == SAMPLE_NUMBER_NO_ENTRY_m11)
				get_Sgmt_recs = TRUE_m11;
			if (get_Sgmt_recs == TRUE_m11) {
				chan = globals_m11->reference_channel;
				if (chan->type_code != LH_TIME_SERIES_CHANNEL_m11)
					chan = sess->time_series_channels[0];
				if (chan->Sgmt_records != NULL)
					Sgmt_records = chan->Sgmt_records;
				else
					Sgmt_records = chan->Sgmt_records = build_Sgmt_records_array_m11(chan->record_indices_fps, chan->record_data_fps, chan);
			}
			break;
		default:
			warning_message_m11("%s(): invalid level type\n", __FUNCTION__);
			return(SEGMENT_NUMBER_NO_ENTRY_m11);
	}
	
	low_idx = 0;
	high_idx = globals_m11->number_of_session_segments - 1;
	if (target_sample <= Sgmt_records[0].start_sample_number)
		return(1);  // return first segment if requested sample number <= session start
	if (target_sample >= Sgmt_records[high_idx].end_sample_number)
		return(high_idx + 1);  // return last segment if requested sample number >= session end
		
	do {  // binary search
		idx = (low_idx + high_idx) >> 1;
		if (Sgmt_records[idx].start_sample_number > target_sample)
			high_idx = idx;
		else
			low_idx = idx;
	} while ((high_idx - low_idx) > 1);
	
	if (target_sample > Sgmt_records[low_idx].end_sample_number)
	    idx = high_idx;
	else if (target_sample < Sgmt_records[high_idx].start_sample_number)
	    idx = low_idx;

	return(idx + 1);
}

		
si4	segment_for_uutc_m11(LEVEL_HEADER_m11 *level_header, si8 target_time)
{
	si4			idx, low_idx, high_idx;
	CHANNEL_m11		*chan;
	SESSION_m11		*sess;
	Sgmt_RECORD_m11		*Sgmt_records;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// Note: this may seem like overkill, that a simple forward linear search would suffice,
	// but in theory there can be a large number of non-uniformly spaced segments.
	
	switch (level_header->type_code) {
		case LH_VIDEO_CHANNEL_m11:
		case LH_TIME_SERIES_CHANNEL_m11:
			chan = (CHANNEL_m11 *) level_header;
			Sgmt_records = chan->Sgmt_records;
			if (Sgmt_records == NULL && chan->parent != NULL)
				Sgmt_records = ((SESSION_m11 *) chan->parent)->Sgmt_records;
			else
				Sgmt_records = build_Sgmt_records_array_m11(chan->record_indices_fps, chan->record_data_fps, chan);
			break;
		case LH_SESSION_m11:
			sess = (SESSION_m11 *) level_header;
			Sgmt_records = sess->Sgmt_records;
			if (Sgmt_records == NULL && globals_m11->reference_channel->Sgmt_records != NULL)
				Sgmt_records = globals_m11->reference_channel->Sgmt_records;
			else
				Sgmt_records = build_Sgmt_records_array_m11(sess->record_indices_fps, sess->record_data_fps, NULL);
			break;
		default:
			warning_message_m11("%s(): invalid level type\n", __FUNCTION__);
			return(SEGMENT_NUMBER_NO_ENTRY_m11);
	}
	
	low_idx = 0;
	high_idx = globals_m11->number_of_session_segments - 1;
	if (target_time <= Sgmt_records[0].start_time)
		return(1);  // return first segment if requested time <= session start
	if (target_time >= Sgmt_records[high_idx].end_time)
		return(high_idx + 1);   // return last segment if requested time >= session end
		
	do {  // binary search
		idx = (low_idx + high_idx) >> 1;
		if (Sgmt_records[idx].start_time > target_time)
			high_idx = idx;
		else
			low_idx = idx;
	} while ((high_idx - low_idx) > 1);
	
	if (target_time > Sgmt_records[low_idx].end_time)
	    idx = high_idx;
	else if (target_time < Sgmt_records[high_idx].start_time)
	    idx = low_idx;

	return(idx + 1);
}

		
void	set_error_m11(const si4 err_code, const si1 *function, const si4 line)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (globals_m11->err_code == E_NO_ERR_m11) {
		globals_m11->err_code = err_code;
		globals_m11->err_func = function;
		globals_m11->err_line = line;
	}
	
	return;
}


TERN_m11    set_global_time_constants_m11(TIMEZONE_INFO_m11 *timezone_info, si8 session_start_time, TERN_m11 prompt)
{
	si4                     n_potential_timezones, potential_timezone_entries[TZ_TABLE_ENTRIES_m11];
	si4                     i, j, response_num, items;
	TIMEZONE_INFO_m11	*tz_table;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (globals_m11->timezone_table == NULL)
		initialize_timezone_tables_m11();
	
	// reset
	globals_m11->time_constants_set = FALSE_m11;
	
	// capitalize & check aliases
	condition_timezone_info_m11(timezone_info);  // modified if alias found
	
	// start search
	n_potential_timezones = TZ_TABLE_ENTRIES_m11;
	tz_table = globals_m11->timezone_table;
	for (i = 0; i < n_potential_timezones; ++i)
		potential_timezone_entries[i] = i;
	
	// match country
	j = 0;
	if (*timezone_info->country) {
		for (i = 0; i < n_potential_timezones; ++i)
			if (strcmp(timezone_info->country, tz_table[potential_timezone_entries[i]].country) == 0)
				potential_timezone_entries[j++] = potential_timezone_entries[i];
	}
	if (j) {
		if (j == 1)
			goto SET_GTC_TIMEZONE_MATCH_m11;
		n_potential_timezones = j;
	}
	
	// match country_acronym_2_letter
	j = 0;
	if (*timezone_info->country_acronym_2_letter) {
		for (i = 0; i < n_potential_timezones; ++i)
			if (strcmp(timezone_info->country_acronym_2_letter, tz_table[potential_timezone_entries[i]].country_acronym_2_letter) == 0)
				potential_timezone_entries[j++] = potential_timezone_entries[i];
	}
	if (j) {
		if (j == 1)
			goto SET_GTC_TIMEZONE_MATCH_m11;
		n_potential_timezones = j;
	}
	
	// match country_acronym_3_letter
	j = 0;
	if (*timezone_info->country_acronym_3_letter) {
		for (i = 0; i < n_potential_timezones; ++i)
			if (strcmp(timezone_info->country_acronym_3_letter, tz_table[potential_timezone_entries[i]].country_acronym_3_letter) == 0)
				potential_timezone_entries[j++] = potential_timezone_entries[i];
	}
	if (j) {
		if (j == 1)
			goto SET_GTC_TIMEZONE_MATCH_m11;
		n_potential_timezones = j;
	}
	
	// match standard_timezone_acronym
	j = 0;
	if (*timezone_info->standard_timezone_acronym) {
		for (i = 0; i < n_potential_timezones; ++i)
			if (strcmp(timezone_info->standard_timezone_acronym, tz_table[potential_timezone_entries[i]].standard_timezone_acronym) == 0)
				potential_timezone_entries[j++] = potential_timezone_entries[i];
	}
	if (j) {
		if (j == 1)
			goto SET_GTC_TIMEZONE_MATCH_m11;
		n_potential_timezones = j;
	}

	// match standard_timezone
	j = 0;
	if (*timezone_info->standard_timezone) {
		for (i = 0; i < n_potential_timezones; ++i)
			if (strcmp(timezone_info->standard_timezone, tz_table[potential_timezone_entries[i]].standard_timezone) == 0)
				potential_timezone_entries[j++] = potential_timezone_entries[i];
	}
	if (j) {
		if (j == 1)
			goto SET_GTC_TIMEZONE_MATCH_m11;
		n_potential_timezones = j;
	}

	// match daylight_timezone_acronym
	j = 0;
	if (*timezone_info->daylight_timezone_acronym) {
		for (i = 0; i < n_potential_timezones; ++i)
			if (strcmp(timezone_info->daylight_timezone_acronym, tz_table[potential_timezone_entries[i]].daylight_timezone_acronym) == 0)
				potential_timezone_entries[j++] = potential_timezone_entries[i];
	}
	if (j) {
		if (j == 1)
			goto SET_GTC_TIMEZONE_MATCH_m11;
		n_potential_timezones = j;
	}

	// match daylight_timezone
	j = 0;
	if (*timezone_info->daylight_timezone) {
		for (i = 0; i < n_potential_timezones; ++i) 
			if (strcmp(timezone_info->daylight_timezone, tz_table[potential_timezone_entries[i]].daylight_timezone) == 0) 
				potential_timezone_entries[j++] = potential_timezone_entries[i];
	}
	if (j) {
		if (j == 1)
			goto SET_GTC_TIMEZONE_MATCH_m11;
		n_potential_timezones = j;
	}

	// match standard_UTC_offset
	j = 0;
	if (timezone_info->standard_UTC_offset) {  // zero is a valid offset, but also could be zero from calloc() - can't use it to exclude
		for (i = 0; i < n_potential_timezones; ++i)
			if (timezone_info->standard_UTC_offset == tz_table[potential_timezone_entries[i]].standard_UTC_offset)
				potential_timezone_entries[j++] = potential_timezone_entries[i];
	}
	if (j) {
		if (j == 1)
			goto SET_GTC_TIMEZONE_MATCH_m11;
		n_potential_timezones = j;
	}

	// match territory
	j = 0;
	if (*timezone_info->territory) {
		for (i = 0; i < n_potential_timezones; ++i)
			if (strcmp(timezone_info->territory, tz_table[potential_timezone_entries[i]].territory) == 0)
				potential_timezone_entries[j++] = potential_timezone_entries[i];
	}
	if (j) {
		if (j == 1)
			goto SET_GTC_TIMEZONE_MATCH_m11;
		n_potential_timezones = j;
	}
	
	// match territory_acronym
	j = 0;
	if (*timezone_info->territory_acronym) {
		for (i = 0; i < n_potential_timezones; ++i)
			if (strcmp(timezone_info->territory_acronym, tz_table[potential_timezone_entries[i]].territory_acronym) == 0)
				potential_timezone_entries[j++] = potential_timezone_entries[i];
	}
	if (j) {
		if (j == 1)
			goto SET_GTC_TIMEZONE_MATCH_m11;
		n_potential_timezones = j;
	}

	// still multiple: doesn't matter if UTC offset, daylight change info, & timezone names all match
	for (i = 1; i < n_potential_timezones; ++i) {
		if (tz_table[potential_timezone_entries[i]].standard_UTC_offset != tz_table[potential_timezone_entries[0]].standard_UTC_offset)
			break;
		if (tz_table[potential_timezone_entries[i]].daylight_time_start_code != tz_table[potential_timezone_entries[0]].daylight_time_start_code)
			break;
		if (tz_table[potential_timezone_entries[i]].daylight_time_end_code != tz_table[potential_timezone_entries[0]].daylight_time_end_code)
			break;
		if (strcmp(tz_table[potential_timezone_entries[i]].standard_timezone, tz_table[potential_timezone_entries[0]].standard_timezone))
			break;
	}
	if (i == n_potential_timezones)
		goto SET_GTC_TIMEZONE_MATCH_m11;

	// still multiple: ask user
	if (prompt == TRUE_m11) {
		fprintf_m11(stderr, "\nMultiple potential timezone entries:\n\n");
		for (i = 0; i < n_potential_timezones; ++i) {
			fprintf_m11(stderr, "%d)\n", i + 1);
			show_timezone_info_m11(&tz_table[potential_timezone_entries[i]], FALSE_m11);
			fputc_m11('\n', stderr);
		}
		fprintf_m11(stderr, "Select one (by number): ");
		items = scanf("%d", &response_num);
		if (items != 1 || response_num < 1 || response_num > n_potential_timezones) {
			error_message_m11("Invalid choice\n");
			exit_m11(-1);
		}
		potential_timezone_entries[0] = potential_timezone_entries[--response_num];
	} else {
		return(FALSE_m11);
	}
	
SET_GTC_TIMEZONE_MATCH_m11:
	*timezone_info = tz_table[potential_timezone_entries[0]];
	globals_m11->standard_UTC_offset = timezone_info->standard_UTC_offset;
	strncpy_m11(globals_m11->standard_timezone_acronym, timezone_info->standard_timezone_acronym, TIMEZONE_ACRONYM_BYTES_m11);
	strncpy_m11(globals_m11->standard_timezone_string, timezone_info->standard_timezone, TIMEZONE_STRING_BYTES_m11);
	STR_to_title_m11(globals_m11->standard_timezone_string);  // beautify
	if (timezone_info->daylight_time_start_code) {
		if (timezone_info->daylight_time_start_code == DTCC_VALUE_NO_ENTRY_m11) {
			globals_m11->observe_DST = UNKNOWN_m11;
		} else {
			globals_m11->observe_DST = TRUE_m11;
			strncpy_m11(globals_m11->daylight_timezone_acronym, timezone_info->daylight_timezone_acronym, TIMEZONE_ACRONYM_BYTES_m11);
			strncpy_m11(globals_m11->daylight_timezone_string, timezone_info->daylight_timezone, TIMEZONE_STRING_BYTES_m11);
			STR_to_title_m11(globals_m11->daylight_timezone_string);  // beautify
			globals_m11->daylight_time_start_code.value = timezone_info->daylight_time_start_code;
			globals_m11->daylight_time_end_code.value = timezone_info->daylight_time_end_code;
		}
	}
	else {
		globals_m11->observe_DST = FALSE_m11;
	}
	globals_m11->time_constants_set = TRUE_m11;

	if (session_start_time)  // pass CURRENT_TIME_m11 for session starting now; pass zero if just need to get timezone_info for a locale
		generate_recording_time_offset_m11(session_start_time);

	return(TRUE_m11);
}


TERN_m11	set_time_and_password_data_m11(si1 *unspecified_password, si1 *MED_directory, si1 *metadata_section_2_encryption_level, si1 *metadata_section_3_encryption_level)
{
	si1                             metadata_file[FULL_FILE_NAME_BYTES_m11];
	FILE_PROCESSING_STRUCT_m11	*metadata_fps;
	METADATA_SECTION_1_m11		*md1;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// find a MED metadata file
	if (find_metadata_file_m11(MED_directory, metadata_file) == NULL) {
		error_message_m11("%s(): \"%s\" does not contain any metadata files\n", __FUNCTION__, MED_directory);
		return(FALSE_m11);
	}

	// read_file_m11() will process password and set current session directory globals
	// decrypt_metadata_m11() will set global time constants, from section 3
	globals_m11->password_data.processed = 0;  // not ternary FALSE_m11 (so when structure is zeroed it is marked as not processed)
	metadata_fps = read_file_m11(NULL, metadata_file, 0, 0, FPS_FULL_FILE_m11, NULL, unspecified_password, RETURN_ON_FAIL_m11);
	if (metadata_fps == NULL)
		return(FALSE_m11);
	globals_m11->session_start_time = metadata_fps->universal_header->session_start_time;

	// return metadata encryption level info
	md1 = &metadata_fps->metadata->section_1;
	if (metadata_section_2_encryption_level != NULL)
		*metadata_section_2_encryption_level = md1->section_2_encryption_level;
	if (metadata_section_3_encryption_level != NULL)
		*metadata_section_3_encryption_level = md1->section_3_encryption_level;

	// clean up
	FPS_free_processing_struct_m11(metadata_fps, TRUE_m11);

	return(TRUE_m11);
}


void	show_behavior_m11(void)
{
	si1	behavior_string[256];
	si4	i, j;
	
	
	// get mutex
	pthread_mutex_lock_m11(&globals_m11->behavior_mutex);

	printf_m11("\nCurrent Global Behavior:\n------------------------\n");
	behavior_string_m11(globals_m11->behavior_on_fail, behavior_string);
	printf_m11("%s\n\n", behavior_string);
	
	if (globals_m11->behavior_stack_entries) {
		printf_m11("Current Behavior Stack:\n-----------------------\n");
		for (i = 0, j = (si4) globals_m11->behavior_stack_entries - 1; i < globals_m11->behavior_stack_entries; ++i, --j) {
			behavior_string_m11(globals_m11->behavior_stack[j], behavior_string);
			printf_m11("%d)\t%s\n", i, behavior_string);
		}
		printf_m11("\n");
	}
	
	// release mutex
	pthread_mutex_unlock_m11(&globals_m11->behavior_mutex);

	return;
}


void    show_daylight_change_code_m11(DAYLIGHT_TIME_CHANGE_CODE_m11 *code, si1 *prefix)
{
	static si1	*relative_days[7] = { "", "First", "Second", "Third", "Fourth", "Fifth", "Last"};
	static si1	*weekdays[8] = { "", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
	static si1	*months[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
	static si1	*mday_num_sufs[32] = { 	"", "st", "nd", "rd", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", \
						"th", "th", "st", "nd", "rd", "th", "th", "th", "th", "th", "th", "th", "st" };
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (prefix == NULL)
		prefix = "";
	printf_m11("%sStructure Content:\n", prefix);
	printf_m11("%sCode Type (DST end / DST Not Observed / DST start) ==  (-1 / 0 / +1): %hhd\n", prefix, code->code_type);
	printf_m11("%sDay of Week (No Entry / [Sunday : Saturday]) ==  (0 / [1 : 7]): %hhd\n", prefix, code->day_of_week);
	printf_m11("%sRelative Weekday of Month (No Entry / [First : Fifth] / Last) ==  (0 / [1 : 5] / 6): %hhd\n", prefix, code->relative_weekday_of_month);
	printf_m11("%sDay of Month (No Entry / [1 : 31]) ==  (0 / [1 : 31]): %hhd\n", prefix, code->day_of_month);
	printf_m11("%sMonth (No Entry / [January : December]) ==  (-1 / [0 : 11]): %hhd\n", prefix, code->month);
	printf_m11("%sHours of Day [-128 : +127] hours relative to 00:00 (midnight): %hhd\n", prefix, code->hours_of_day);
	printf_m11("%sReference Time (Local / UTC) ==  (0 / 1): %hhd\n", prefix, code->reference_time);
	printf_m11("%sShift Minutes [-120 : +120] minutes: %hhd\n", prefix, code->shift_minutes);
	printf_m11("%sValue: 0x%lX\n\n", prefix, code->value);

	// human readable
	printf_m11("%sTranslated Content: ", prefix);
	switch (code->value) {
		case DTCC_VALUE_NO_ENTRY_m11:
			printf_m11("daylight saving change information not entered\n\n");
			return;
		case DTCC_VALUE_NOT_OBSERVED_m11:
			printf_m11("daylight saving not observed\n\n");
			return;
	}
	switch (code->code_type) {
		case -1:
			printf_m11("daylight saving END\n");
			break;
		case 1:
			printf_m11("daylight saving START\n");
			break;
	}

	printf_m11("%s", prefix);
	if (code->relative_weekday_of_month) {
		printf_m11("%s ", relative_days[(si4) code->relative_weekday_of_month]);
		printf_m11("%s ", weekdays[(si4) (code->day_of_week + 1)]);
		printf_m11("in %s ", months[(si4) code->month]);
	} else if (code->day_of_month) {
		printf_m11("%s ", months[(si4) code->month]);
		printf_m11("%hhd%s ", code->day_of_month, mday_num_sufs[(si4) code->day_of_month]);
	}

	printf_m11("at %hhd:00 ", code->hours_of_day);
	switch (code->reference_time) {
		case 0:
			printf_m11("local ");
			break;
		case 1:
			printf_m11("UTC ");
			break;
	}
	if (code->shift_minutes < 0)
		printf_m11(" (shift back by %hhd minutes)\n\n", -(code->shift_minutes));
	else
		printf_m11(" (shift forward by %hhd minutes)\n\n", code->shift_minutes);

	return;
}


void	show_file_times_m11(FILE_TIMES_m11 *ft)
{
	si1	time_str[TIME_STRING_BYTES_m11];
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	printf_m11("Creation Time: ");
	if (ft->creation == UUTC_NO_ENTRY_m11) {
		printf_m11("no entry\n");
	} else {
		time_string_m11(ft->creation, time_str, TRUE_m11, FALSE_m11, FALSE_m11);
		printf_m11("%ld (oUTC), %s\n", ft->creation, time_str);
	}
	
	printf_m11("Access Time: ");
	if (ft->access == UUTC_NO_ENTRY_m11) {
		printf_m11("no entry\n");
	} else {
		time_string_m11(ft->access, time_str, TRUE_m11, FALSE_m11, FALSE_m11);
		printf_m11("%ld (oUTC), %s\n", ft->access, time_str);
	}

	printf_m11("Modification Time: ");
	if (ft->creation == UUTC_NO_ENTRY_m11) {
		printf_m11("no entry\n");
	} else {
		time_string_m11(ft->modification, time_str, TRUE_m11, FALSE_m11, FALSE_m11);
		printf_m11("%ld (oUTC), %s\n", ft->modification, time_str);
	}

	return;
}


void    show_globals_m11(void)
{
	si1     hex_str[HEX_STRING_BYTES_m11(sizeof(si8))];
	si4	i;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	printf_m11("\nMED Globals\n-----------\n-----------\n");
	
	printf_m11("\nRecord Filters\n--------------\n");
	if (globals_m11->record_filters == NULL) {
		printf_m11("no entry\n");
	} else if (globals_m11->record_filters[0] == NO_TYPE_CODE_m11) {
		printf_m11("no entry\n");
	} else {
		printf_m11("0x%08x\n", globals_m11->record_filters[0]);
		for (i = 1; globals_m11->record_filters[i] != NO_TYPE_CODE_m11; ++i)
			printf_m11(", 0x%08x", globals_m11->record_filters[i]);
		printf_m11("\n");
	}

	printf_m11("\nCurrent Session\n---------------\n");
	printf_m11("Session UID: 0x%lx\n", globals_m11->session_UID);
	printf_m11("Session Directory: %s\n", globals_m11->session_directory);  // path including file system session directory name
	printf_m11("Session Start Time: ");
	if (globals_m11->session_start_time == UUTC_NO_ENTRY_m11)
		printf_m11("no entry\n");
	else if (globals_m11->session_start_time == BEGINNING_OF_TIME_m11)
		printf_m11("beginning of time\n");
	else if (globals_m11->session_start_time == END_OF_TIME_m11)
		printf_m11("end of time\n");
	else
		printf_m11("%ld\n", globals_m11->session_start_time);
	printf_m11("Session End Time: ");
	if (globals_m11->session_end_time == UUTC_NO_ENTRY_m11)
		printf_m11("no entry\n");
	else if (globals_m11->session_end_time == BEGINNING_OF_TIME_m11)
		printf_m11("beginning of time\n");
	else if (globals_m11->session_end_time == END_OF_TIME_m11)
		printf_m11("end of time\n");
	else
		printf_m11("%ld\n", globals_m11->session_end_time);
	if (globals_m11->session_name == NULL)
		printf_m11("Session Name: NULL\n");
	else
		printf_m11("Session Name: %s\n", globals_m11->session_name);
	printf_m11("\tuh_session_name: %s\n", globals_m11->uh_session_name);  // from session universal headers
	printf_m11("\tfs_session_name: %s\n", globals_m11->fs_session_name);  // from file system (different if user created channel subset with different name)
	printf_m11("Number of Session Samples / Frames: ");
	if (globals_m11->number_of_session_samples == 0)
		printf_m11("no entry\n");
	else
		printf_m11("%ld\n", globals_m11->number_of_session_samples);
	printf_m11("Number of Session Segments: ");
	if (globals_m11->number_of_session_segments == SEGMENT_NUMBER_NO_ENTRY_m11)
		printf_m11("no entry\n");
	else
		printf_m11("%d\n", globals_m11->number_of_session_segments);
	printf_m11("Number of Mapped Segments: ");
	if (globals_m11->number_of_mapped_segments == SEGMENT_NUMBER_NO_ENTRY_m11)
		printf_m11("no entry\n");
	else
		printf_m11("%d\n", globals_m11->number_of_mapped_segments);
	printf_m11("Reference Channel Name: ");
	if (*globals_m11->reference_channel_name == 0)
		printf_m11("no entry\n");
	else
		printf_m11("%s\n", globals_m11->reference_channel_name);
	printf_m11("Reference Channel: ");
	if (globals_m11->reference_channel == NULL)
		printf_m11("not set\n");
	else
		printf_m11("set\n");

	printf_m11("\nActive Channels\n---------------\n");
	printf_m11("Time Series Frequencies Vary: ");
	if (globals_m11->time_series_frequencies_vary == UNKNOWN_m11)
		printf_m11("unknown\n");
	else if (globals_m11->time_series_frequencies_vary == TRUE_m11)
		printf_m11("true\n");
	else if (globals_m11->time_series_frequencies_vary == FALSE_m11)
		printf_m11("false\n");
	else
		printf_m11("%hhd\n", globals_m11->time_series_frequencies_vary);
	if (globals_m11->minimum_time_series_frequency == FREQUENCY_NO_ENTRY_m11)
		printf_m11("Minimum Time Series Frequency: no entry\n");
	else
		printf_m11("Minimum Time Series Frequency: %lf\n", globals_m11->minimum_time_series_frequency);
	if (globals_m11->maximum_time_series_frequency == FREQUENCY_NO_ENTRY_m11)
		printf_m11("Maximum Time Series Frequency: no entry\n");
	else
		printf_m11("Maximum Time Series Frequency: %lf\n", globals_m11->maximum_time_series_frequency);
	if (globals_m11->minimum_time_series_frequency_channel == NULL)
		printf_m11("Minimum Time Series Frequency Channel: no entry\n");
	else
		printf_m11("Minimum Time Series Frequency Channel Name: %s\n", globals_m11->minimum_time_series_frequency_channel->name);
	if (globals_m11->maximum_time_series_frequency_channel == NULL)
		printf_m11("Maximum Time Series Frequency Channel: no entry\n");
	else
		printf_m11("Maximum Time Series Frequency Channel Name: %s\n", globals_m11->maximum_time_series_frequency_channel->name);
	printf_m11("Video Frame Rates Vary: ");
	if (globals_m11->video_frame_rates_vary == UNKNOWN_m11)
		printf_m11("unknown\n");
	else if (globals_m11->video_frame_rates_vary == TRUE_m11)
		printf_m11("true\n");
	else if (globals_m11->video_frame_rates_vary == FALSE_m11)
		printf_m11("false\n");
	else
		printf_m11("%hhd\n", globals_m11->video_frame_rates_vary);
	if (globals_m11->minimum_video_frame_rate == FREQUENCY_NO_ENTRY_m11)
		printf_m11("Minimum Video Frame Rate: no entry\n");
	else
		printf_m11("Minimum Video Frame Rate: %lf\n", globals_m11->minimum_video_frame_rate);
	if (globals_m11->maximum_video_frame_rate == FREQUENCY_NO_ENTRY_m11)
		printf_m11("Maximum Video Frame Rate: no entry\n");
	else
		printf_m11("Minimum Video Frame Rate: %lf\n", globals_m11->maximum_video_frame_rate);
	if (globals_m11->minimum_video_frame_rate_channel == NULL)
		printf_m11("Minimum Video Frame Rate Channel: no entry\n");
	else
		printf_m11("Minimum Video Frame Rate Channel Name: %s\n", globals_m11->minimum_video_frame_rate_channel->name);
	if (globals_m11->maximum_video_frame_rate_channel == NULL)
		printf_m11("Maximum Video Frame Rate Channel: no entry\n");
	else
		printf_m11("Maximum Video Frame Rate Channel Name: %s\n", globals_m11->maximum_video_frame_rate_channel->name);

	printf_m11("\nTime Constants\n--------------\n");
	printf_m11("time_constants_set: %hhd\n", globals_m11->time_constants_set);
	printf_m11("RTO_known: %hhd\n", globals_m11->RTO_known);
	printf_m11("observe_DST: %hhd\n", globals_m11->observe_DST);
	printf_m11("recording_time_offset: %ld\n", globals_m11->recording_time_offset);
	printf_m11("standard_UTC_offset: %d\n", globals_m11->standard_UTC_offset);
	printf_m11("standard_timezone_acronym: %s\n", globals_m11->standard_timezone_acronym);
	printf_m11("standard_timezone_string: %s\n", globals_m11->standard_timezone_string);
	printf_m11("daylight_timezone_acronym: %s\n", globals_m11->daylight_timezone_acronym);
	printf_m11("daylight_timezone_string: %s\n", globals_m11->daylight_timezone_string);
	generate_hex_string_m11((ui1 *) &globals_m11->daylight_time_start_code.value, 8, hex_str);
	printf_m11("daylight_time_start_code: %s\n", hex_str);
	generate_hex_string_m11((ui1 *) &globals_m11->daylight_time_end_code.value, 8, hex_str);
	printf_m11("daylight_time_end_code: %s\n", hex_str);
	
	printf_m11("\nAlignment Fields\n----------------\n");
	printf_m11("universal_header_aligned: %hhd\n", globals_m11->universal_header_aligned);
	printf_m11("metadata_section_1_aligned: %hhd\n", globals_m11->metadata_section_1_aligned);
	printf_m11("time_series_metadata_section_2_aligned: %hhd\n", globals_m11->time_series_metadata_section_2_aligned);
	printf_m11("video_metadata_section_2_aligned: %hhd\n", globals_m11->video_metadata_section_2_aligned);
	printf_m11("metadata_section_3_aligned: %hhd\n", globals_m11->metadata_section_3_aligned);
	printf_m11("all_metadata_structures_aligned: %hhd\n", globals_m11->all_metadata_structures_aligned);
	printf_m11("time_series_indices_aligned: %hhd\n", globals_m11->time_series_indices_aligned);
	printf_m11("video_indices_aligned: %hhd\n", globals_m11->video_indices_aligned);
	printf_m11("CMP_block_header_aligned: %hhd\n", globals_m11->CMP_block_header_aligned);
	printf_m11("record_header_aligned: %hhd\n", globals_m11->record_header_aligned);
	printf_m11("record_indices_aligned: %hhd\n", globals_m11->record_indices_aligned);
	printf_m11("all_record_structures_aligned: %hhd\n", globals_m11->all_record_structures_aligned);
	printf_m11("all_structures_aligned: %hhd\n", globals_m11->all_structures_aligned);
	
	printf_m11("\nError\n-------------\n");
	printf_m11("err_code: %d\n", globals_m11->err_code);
	printf_m11("err_func: %s\n", globals_m11->err_func);
	printf_m11("err_line: %s\n", globals_m11->err_line);

	printf_m11("\nMiscellaneous\n-------------\n");
	printf_m11("time_series_data_encryption_level: %hhd\n", globals_m11->time_series_data_encryption_level);
	printf_m11("CRC_mode: %u\n", globals_m11->CRC_mode);
	printf_m11("verbose: %hhd\n", globals_m11->verbose);
	printf_m11("behavior_on_fail: %u\n", globals_m11->behavior_on_fail);
	printf_m11("level_header_flags: %lu\n", globals_m11->level_header_flags);
	printf_m11("mmap_block_bytes: ");
	if (globals_m11->mmap_block_bytes == GLOBALS_MMAP_BLOCK_BYTES_NO_ENTRY_m11)
		printf_m11("no entry\n");
	else
		printf_m11("%d\n", globals_m11->mmap_block_bytes);
	
	printf_m11("\n");
	
	return;
}


void	show_level_header_flags_m11(ui8 flags)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	printf_m11("\nLevel Header Flags:\n------------------\n");
	if (flags == LH_NO_FLAGS_m11) {
		printf_m11("no level header flags set\n");
		return;
	}
	if (flags & LH_OPEN_m11)
		printf_m11("LH_OPEN_m11: %strue%s\n", TC_RED_m11, TC_RESET_m11);
	else
		printf_m11("LH_OPEN_m11: %sfalse%s\n", TC_BLUE_m11, TC_RESET_m11);
	if (flags & LH_GENERATE_EPHEMERAL_DATA_m11)
		printf_m11("LH_GENERATE_EPHEMERAL_DATA_m11: %strue%s\n", TC_RED_m11, TC_RESET_m11);
	else
		printf_m11("LH_GENERATE_EPHEMERAL_DATA_m11: %sfalse%s\n", TC_BLUE_m11, TC_RESET_m11);
	if (flags & LH_UPDATE_EPHEMERAL_DATA_m11)
		printf_m11("LH_UPDATE_EPHEMERAL_DATA_m11: %strue%s\n", TC_RED_m11, TC_RESET_m11);
	else
		printf_m11("LH_UPDATE_EPHEMERAL_DATA_m11: %sfalse%s\n", TC_BLUE_m11, TC_RESET_m11);
	if (flags & LH_INCLUDE_TIME_SERIES_CHANNELS_m11)
		printf_m11("LH_INCLUDE_TIME_SERIES_CHANNELS_m11: %strue%s\n", TC_RED_m11, TC_RESET_m11);
	else
		printf_m11("LH_INCLUDE_TIME_SERIES_CHANNELS_m11: %sfalse%s\n", TC_BLUE_m11, TC_RESET_m11);
	if (flags & LH_INCLUDE_VIDEO_CHANNELS_m11)
		printf_m11("LH_INCLUDE_VIDEO_CHANNELS_m11: %strue%s\n", TC_RED_m11, TC_RESET_m11);
	else
		printf_m11("LH_INCLUDE_VIDEO_CHANNELS_m11: %sfalse%s\n", TC_BLUE_m11, TC_RESET_m11);
	if (flags & LH_MAP_ALL_TIME_SERIES_CHANNELS_m11)
		printf_m11("LH_MAP_ALL_TIME_SERIES_CHANNELS_m11: %strue%s\n", TC_RED_m11, TC_RESET_m11);
	else
		printf_m11("LH_MAP_ALL_TIME_SERIES_CHANNELS_m11: %sfalse%s\n", TC_BLUE_m11, TC_RESET_m11);
	if (flags & LH_MAP_ALL_VIDEO_CHANNELS_m11)
		printf_m11("LH_MAP_ALL_VIDEO_CHANNELS_m11: %strue%s\n", TC_RED_m11, TC_RESET_m11);
	else
		printf_m11("LH_MAP_ALL_VIDEO_CHANNELS_m11: %sfalse%s\n", TC_BLUE_m11, TC_RESET_m11);
	if (flags & LH_READ_SLICE_SESSION_RECORDS_m11)
		printf_m11("LH_READ_SLICE_SESSION_RECORDS_m11: %strue%s\n", TC_RED_m11, TC_RESET_m11);
	else
		printf_m11("LH_READ_SLICE_SESSION_RECORDS_m11: %sfalse%s\n", TC_BLUE_m11, TC_RESET_m11);
	if (flags & LH_READ_FULL_SESSION_RECORDS_m11)
		printf_m11("LH_READ_FULL_SESSION_RECORDS_m11: %strue%s\n", TC_RED_m11, TC_RESET_m11);
	else
		printf_m11("LH_READ_FULL_SESSION_RECORDS_m11: %sfalse%s\n", TC_BLUE_m11, TC_RESET_m11);
	if (flags & LH_MEM_MAP_SESSION_RECORDS_m11)
		printf_m11("LH_MEM_MAP_SESSION_RECORDS_m11: %strue%s\n", TC_RED_m11, TC_RESET_m11);
	else
		printf_m11("LH_MEM_MAP_SESSION_RECORDS_m11: %sfalse%s\n", TC_BLUE_m11, TC_RESET_m11);
	if (flags & LH_READ_SLICE_SEGMENTED_SESS_RECS_m11)
		printf_m11("LH_READ_SLICE_SEGMENTED_SESS_RECS_m11: %strue%s\n", TC_RED_m11, TC_RESET_m11);
	else
		printf_m11("LH_READ_SLICE_SEGMENTED_SESS_RECS_m11: %sfalse%s\n", TC_BLUE_m11, TC_RESET_m11);
	if (flags & LH_READ_FULL_SEGMENTED_SESS_RECS_m11)
		printf_m11("LH_READ_FULL_SEGMENTED_SESS_RECS_m11: %strue%s\n", TC_RED_m11, TC_RESET_m11);
	else
		printf_m11("LH_READ_FULL_SEGMENTED_SESS_RECS_m11: %sfalse%s\n", TC_BLUE_m11, TC_RESET_m11);
	if (flags & LH_MEM_MAP_SEGMENTED_SESS_RECS_m11)
		printf_m11("LH_MEM_MAP_SEGMENTED_SESS_RECS_m11: %strue%s\n", TC_RED_m11, TC_RESET_m11);
	else
		printf_m11("LH_MEM_MAP_SEGMENTED_SESS_RECS_m11: %sfalse%s\n", TC_BLUE_m11, TC_RESET_m11);
	if (flags & LH_CHANNEL_ACTIVE_m11)
		printf_m11("LH_CHANNEL_ACTIVE_m11: %strue%s\n", TC_RED_m11, TC_RESET_m11);
	else
		printf_m11("LH_CHANNEL_ACTIVE_m11: %sfalse%s\n", TC_BLUE_m11, TC_RESET_m11);
	if (flags & LH_MAP_ALL_SEGMENTS_m11)
		printf_m11("LH_MAP_ALL_SEGMENTS_m11: %strue%s\n", TC_RED_m11, TC_RESET_m11);
	else
		printf_m11("LH_MAP_ALL_SEGMENTS_m11: %sfalse%s\n", TC_BLUE_m11, TC_RESET_m11);
	if (flags & LH_READ_SLICE_CHANNEL_RECORDS_m11)
		printf_m11("LH_READ_SLICE_CHANNEL_RECORDS_m11: %strue%s\n", TC_RED_m11, TC_RESET_m11);
	else
		printf_m11("LH_READ_SLICE_CHANNEL_RECORDS_m11: %sfalse%s\n", TC_BLUE_m11, TC_RESET_m11);
	if (flags & LH_READ_FULL_CHANNEL_RECORDS_m11)
		printf_m11("LH_READ_FULL_CHANNEL_RECORDS_m11: %strue%s\n", TC_RED_m11, TC_RESET_m11);
	else
		printf_m11("LH_READ_FULL_CHANNEL_RECORDS_m11: %sfalse%s\n", TC_BLUE_m11, TC_RESET_m11);
	if (flags & LH_MEM_MAP_CHANNEL_RECORDS_m11)
		printf_m11("LH_MEM_MAP_CHANNEL_RECORDS_m11: %strue%s\n", TC_RED_m11, TC_RESET_m11);
	else
		printf_m11("LH_MEM_MAP_CHANNEL_RECORDS_m11: %sfalse%s\n", TC_BLUE_m11, TC_RESET_m11);
	if (flags & LH_READ_SLICE_SEGMENT_DATA_m11)
		printf_m11("LH_READ_SLICE_SEGMENT_DATA_m11: %strue%s\n", TC_RED_m11, TC_RESET_m11);
	else
		printf_m11("LH_READ_SLICE_SEGMENT_DATA_m11: %sfalse%s\n", TC_BLUE_m11, TC_RESET_m11);
	if (flags & LH_READ_FULL_SEGMENT_DATA_m11)
		printf_m11("LH_READ_FULL_SEGMENT_DATA_m11: %strue%s\n", TC_RED_m11, TC_RESET_m11);
	else
		printf_m11("LH_READ_FULL_SEGMENT_DATA_m11: %sfalse%s\n", TC_BLUE_m11, TC_RESET_m11);
	if (flags & LH_MEM_MAP_SEGMENT_DATA_m11)
		printf_m11("LH_MEM_MAP_SEGMENT_DATA_m11: %strue%s\n", TC_RED_m11, TC_RESET_m11);
	else
		printf_m11("LH_MEM_MAP_SEGMENT_DATA_m11: %sfalse%s\n", TC_BLUE_m11, TC_RESET_m11);
	if (flags & LH_READ_SEGMENT_METADATA_m11)
		printf_m11("LH_READ_SEGMENT_METADATA_m11: %strue%s\n", TC_RED_m11, TC_RESET_m11);
	else
		printf_m11("LH_READ_SEGMENT_METADATA_m11: %sfalse%s\n", TC_BLUE_m11, TC_RESET_m11);
	if (flags & LH_READ_SLICE_SEGMENT_RECORDS_m11)
		printf_m11("LH_READ_SLICE_SEGMENT_RECORDS_m11: %strue%s\n", TC_RED_m11, TC_RESET_m11);
	else
		printf_m11("LH_READ_SLICE_SEGMENT_RECORDS_m11: %sfalse%s\n", TC_BLUE_m11, TC_RESET_m11);
	if (flags & LH_READ_FULL_SEGMENT_RECORDS_m11)
		printf_m11("LH_READ_FULL_SEGMENT_RECORDS_m11: %strue%s\n", TC_RED_m11, TC_RESET_m11);
	else
		printf_m11("LH_READ_FULL_SEGMENT_RECORDS_m11: %sfalse%s\n", TC_BLUE_m11, TC_RESET_m11);
	if (flags & LH_MEM_MAP_SEGMENT_RECORDS_m11)
		printf_m11("LH_MEM_MAP_SEGMENT_RECORDS_m11: %strue%s\n", TC_RED_m11, TC_RESET_m11);
	else
		printf_m11("LH_MEM_MAP_SEGMENT_RECORDS_m11: %sfalse%s\n", TC_BLUE_m11, TC_RESET_m11);
	if (flags & LH_NO_CPS_PTR_RESET_m11)
		printf_m11("LH_NO_CPS_PTR_RESET_m11: %strue%s\n", TC_RED_m11, TC_RESET_m11);
	else
		printf_m11("LH_NO_CPS_PTR_RESET_m11: %sfalse%s\n", TC_BLUE_m11, TC_RESET_m11);
	if (flags & LH_NO_CPS_CACHING_m11)
		printf_m11("LH_NO_CPS_CACHING_m11: %strue%s\n", TC_RED_m11, TC_RESET_m11);
	else
		printf_m11("LH_NO_CPS_CACHING_m11: %sfalse%s\n", TC_BLUE_m11, TC_RESET_m11);

	printf_m11("\n");
	
	return;
}


void	show_metadata_m11(FILE_PROCESSING_STRUCT_m11 *fps, METADATA_m11 *md, ui4 type_code)
{
	si1                                     hex_str[HEX_STRING_BYTES_m11(8)];
	METADATA_SECTION_1_m11			*md1;
	TIME_SERIES_METADATA_SECTION_2_m11	*tmd2, *gmd2;
	VIDEO_METADATA_SECTION_2_m11		*vmd2;
	METADATA_SECTION_3_m11			*md3;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// if passing metadata pointer, also pass type code
	
	// assign
	if (fps != NULL) {
		md = (METADATA_m11 *) fps->data_pointers;
		type_code = fps->universal_header->type_code;
	}
	
	if (md != NULL) {
		md1 = &md->section_1;
		if (type_code == TIME_SERIES_METADATA_FILE_TYPE_CODE_m11) {
			tmd2 = &md->time_series_section_2;
			vmd2 = NULL;
		} else {  // type_code == VIDEO_METADATA_FILE_TYPE_CODE_m11
			vmd2 = &md->video_section_2;
			tmd2 = NULL;
		}
		md3 = &md->section_3;
	} else {
		error_message_m11("%s(): invalid input\n", __FUNCTION__);
		return;
	}
	
	// show
	printf_m11("------------------- Metadata - START -------------------\n");
	printf_m11("------------------ Section 1 - START -------------------\n");
	if (*md1->level_1_password_hint)
		UTF8_printf_m11("Level 1 Password Hint: %s\n", md1->level_1_password_hint);
	if (*md1->level_2_password_hint)
		UTF8_printf_m11("Level 2 Password Hint: %s\n", md1->level_2_password_hint);
	printf_m11("Section 2 Encryption Level: %d ", md1->section_2_encryption_level);
	if (md1->section_2_encryption_level == NO_ENCRYPTION_m11)
		printf_m11("(none)\n");
	else if (md1->section_2_encryption_level == LEVEL_1_ENCRYPTION_m11)
		printf_m11("(level 1, currently encrypted)\n");
	else if (md1->section_2_encryption_level == LEVEL_2_ENCRYPTION_m11)
		printf_m11("(level 2, currently encrypted)\n");
	else if (md1->section_2_encryption_level == -LEVEL_1_ENCRYPTION_m11)
		printf_m11("(level 1, currently decrypted)\n");
	else if (md1->section_2_encryption_level == -LEVEL_2_ENCRYPTION_m11)
		printf_m11("(level 2, currently decrypted)\n");
	else
		printf_m11("(unrecognized code)\n");
	printf_m11("Section 3 Encryption Level: %d ", md1->section_3_encryption_level);
	if (md1->section_3_encryption_level == NO_ENCRYPTION_m11)
		printf_m11("(none)\n");
	else if (md1->section_3_encryption_level == LEVEL_1_ENCRYPTION_m11)
		printf_m11("(level 1, currently encrypted)\n");
	else if (md1->section_3_encryption_level == LEVEL_2_ENCRYPTION_m11)
		printf_m11("(level 2, currently encrypted)\n");
	else if (md1->section_3_encryption_level == -LEVEL_1_ENCRYPTION_m11)
		printf_m11("(level 1, currently decrypted)\n");
	else if (md1->section_3_encryption_level == -LEVEL_2_ENCRYPTION_m11)
		printf_m11("(level 2, currently decrypted)\n");
	else
		printf_m11("(unrecognized code)\n");
	printf_m11("Time Series Data Encryption Level: %d ", md1->time_series_data_encryption_level);
	if (md1->time_series_data_encryption_level == NO_ENCRYPTION_m11)
		printf_m11("(none)\n");
	else if (md1->time_series_data_encryption_level == LEVEL_1_ENCRYPTION_m11)
		printf_m11("(level 1, currently encrypted)\n");
	else if (md1->time_series_data_encryption_level == LEVEL_2_ENCRYPTION_m11)
		printf_m11("(level 2, currently encrypted)\n");
	else if (md1->time_series_data_encryption_level == -LEVEL_1_ENCRYPTION_m11)
		printf_m11("(level 1, currently decrypted)\n");
	else if (md1->time_series_data_encryption_level == -LEVEL_2_ENCRYPTION_m11)
		printf_m11("(level 2, currently decrypted)\n");
	else
		printf_m11("(unrecognized code)\n");
	printf_m11("------------------- Section 1 - END --------------------\n");
	printf_m11("------------------ Section 2 - START -------------------\n");
	
	// decrypt if needed
	if (md1->section_2_encryption_level > NO_ENCRYPTION_m11 || md1->section_3_encryption_level > NO_ENCRYPTION_m11)
		if (fps != NULL)
			decrypt_metadata_m11(fps);
	
	if (md1->section_2_encryption_level <= NO_ENCRYPTION_m11) {
		
		// type-independent fields
		gmd2 = tmd2;
		if (*gmd2->session_description)
			UTF8_printf_m11("Session Description: %s\n", gmd2->session_description);
		else
			printf_m11("Session Description: no entry\n");
		if (*gmd2->channel_description)
			UTF8_printf_m11("Channel Description: %s\n", gmd2->channel_description);
		else
			printf_m11("Channel Description: no entry\n");
		if (*gmd2->segment_description)
			UTF8_printf_m11("Segment Description: %s\n", gmd2->segment_description);
		else
			printf_m11("Segment Description: no entry\n");
		if (*gmd2->equipment_description)
			UTF8_printf_m11("Equipment Description: %s\n", gmd2->equipment_description);
		else
			printf_m11("Equipment Description: no entry\n");
		if (gmd2->acquisition_channel_number == METADATA_ACQUISITION_CHANNEL_NUMBER_NO_ENTRY_m11)
			printf_m11("Acquisition Channel Number: no entry\n");
		else
			printf_m11("Acquisition Channel Number: %d\n", gmd2->acquisition_channel_number);
		
		// type-specific fields
		if (tmd2 != NULL) {
			if (*tmd2->reference_description)
				UTF8_printf_m11("Reference Description: %s\n", tmd2->reference_description);
			else
				printf_m11("Reference Description: no entry\n");
			if (tmd2->sampling_frequency == TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m11)
				printf_m11("Sampling Frequency: no entry\n");
			else if (tmd2->sampling_frequency == TIME_SERIES_METADATA_FREQUENCY_VARIABLE_m11)
				printf_m11("Sampling Frequency: variable\n");
			else
				printf_m11("Sampling Frequency: %lf\n", tmd2->sampling_frequency);
			if (tmd2->low_frequency_filter_setting == TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m11)
				printf_m11("Low Frequency Filter Setting: no entry\n");
			else
				printf_m11("Low Frequency Filter Setting (Hz): %lf\n", tmd2->low_frequency_filter_setting);
			if (tmd2->high_frequency_filter_setting == TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m11)
				printf_m11("High Frequency Filter Setting: no entry\n");
			else
				printf_m11("High Frequency Filter Setting (Hz): %lf\n", tmd2->high_frequency_filter_setting);
			if (tmd2->notch_filter_frequency_setting == TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m11)
				printf_m11("Notch Filter Frequency Setting: no entry\n");
			else
				printf_m11("Notch Filter Frequency Setting (Hz): %lf\n", tmd2->notch_filter_frequency_setting);
			if (tmd2->AC_line_frequency == TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m11)
				printf_m11("AC Line Frequency: no entry\n");
			else
				printf_m11("AC Line Frequency (Hz): %lf\n", tmd2->AC_line_frequency);
			if (tmd2->amplitude_units_conversion_factor == TIME_SERIES_METADATA_AMPLITUDE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m11)
				printf_m11("Amplitiude Units Conversion Factor: no entry\n");
			else
				printf_m11("Amplitude Units Conversion Factor: %lf\n", tmd2->amplitude_units_conversion_factor);
			if (*tmd2->amplitude_units_description)
				UTF8_printf_m11("Amplitude Units Description: %s\n", tmd2->amplitude_units_description);
			else
				printf_m11("Amplitude Units Description: no entry\n");
			if (tmd2->time_base_units_conversion_factor == TIME_SERIES_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m11)
				printf_m11("Time Base Units Conversion Factor: no entry\n");
			else
				printf_m11("Time Base Units Conversion Factor: %lf\n", tmd2->time_base_units_conversion_factor);
			if (*tmd2->time_base_units_description)
				UTF8_printf_m11("Time Base Units Description: %s\n", tmd2->time_base_units_description);
			else
				printf_m11("Time Base Units Description: no entry\n");
			if (tmd2->absolute_start_sample_number == TIME_SERIES_METADATA_ABSOLUTE_START_SAMPLE_NUMBER_NO_ENTRY_m11)
				printf_m11("Absolute Start Sample Number: no entry\n");
			else
				printf_m11("Absolute Start Sample Number: %ld\n", tmd2->absolute_start_sample_number);
			if (tmd2->number_of_samples == TIME_SERIES_METADATA_NUMBER_OF_SAMPLES_NO_ENTRY_m11)
				printf_m11("Number of Samples: no entry\n");
			else
				printf_m11("Number of Samples: %ld\n", tmd2->number_of_samples);
			if (tmd2->number_of_blocks == TIME_SERIES_METADATA_NUMBER_OF_BLOCKS_NO_ENTRY_m11)
				printf_m11("Number of Blocks: no entry\n");
			else
				printf_m11("Number of Blocks: %ld\n", tmd2->number_of_blocks);
			if (tmd2->maximum_block_bytes == TIME_SERIES_METADATA_MAXIMUM_BLOCK_BYTES_NO_ENTRY_m11)
				printf_m11("Maximum Block Bytes: no entry\n");
			else
				printf_m11("Maximum Block Bytes: %ld\n", tmd2->maximum_block_bytes);
			if (tmd2->maximum_block_samples == TIME_SERIES_METADATA_MAXIMUM_BLOCK_SAMPLES_NO_ENTRY_m11)
				printf_m11("Maximum Block Samples: no entry\n");
			else
				printf_m11("Maximum Block Samples: %u\n", tmd2->maximum_block_samples);
			if (tmd2->maximum_block_keysample_bytes == TIME_SERIES_METADATA_MAXIMUM_BLOCK_KEYSAMPLE_BYTES_NO_ENTRY_m11)
				printf_m11("Maximum Block Difference Bytes: no entry\n");
			else
				printf_m11("Maximum Block Keysample Bytes: %u\n", tmd2->maximum_block_keysample_bytes);
			if (tmd2->maximum_block_duration == TIME_SERIES_METADATA_MAXIMUM_BLOCK_DURATION_NO_ENTRY_m11)
				printf_m11("Maximum Block Duration: no entry\n");
			else
				UTF8_printf_m11("Maximum Block Duration: %lf %s\n", tmd2->maximum_block_duration, tmd2->time_base_units_description);
			if (tmd2->number_of_discontinuities == TIME_SERIES_METADATA_NUMBER_OF_DISCONTINUITIES_NO_ENTRY_m11)
				printf_m11("Number of Discontinuities: no entry\n");
			else
				printf_m11("Number of Discontinuities: %ld\n", tmd2->number_of_discontinuities);
			if (tmd2->maximum_contiguous_blocks == TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_BLOCKS_NO_ENTRY_m11)
				printf_m11("Maximum Contiguous Blocks: no entry\n");
			else
				printf_m11("Maximum Contiguous Blocks: %ld\n", tmd2->maximum_contiguous_blocks);
			if (tmd2->maximum_contiguous_block_bytes == TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_BLOCK_BYTES_NO_ENTRY_m11)
				printf_m11("Maximum Contiguous Block Bytes: no entry\n");
			else
				printf_m11("Maximum Contiguous Block Bytes: %ld\n", tmd2->maximum_contiguous_block_bytes);
			if (tmd2->maximum_contiguous_samples == TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_SAMPLES_NO_ENTRY_m11)
				printf_m11("Maximum Contiguous Samples: no entry\n");
			else
				printf_m11("Maximum Contiguous Samples: %ld\n", tmd2->maximum_contiguous_samples);
		} else if (vmd2 != NULL) {
			if (vmd2->time_base_units_conversion_factor == VIDEO_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m11)
				printf_m11("Time Base Units Conversion Factor: no entry\n");
			else
				printf_m11("Time Base Units Conversion Factor: %lf\n", vmd2->time_base_units_conversion_factor);
			if (*vmd2->time_base_units_description)
				UTF8_printf_m11("Time Base Units Description: %s\n", vmd2->time_base_units_description);
			else
				printf_m11("Time Base Units Description: no entry\n");
			if (vmd2->absolute_start_frame_number == VIDEO_METADATA_ABSOLUTE_START_FRAME_NUMBER_NO_ENTRY_m11)
				printf_m11("Absolute Start Frame Number: no entry\n");
			else
				printf_m11("Absolute Start Frame Number: %ld\n", vmd2->absolute_start_frame_number);
			if (vmd2->number_of_frames == VIDEO_METADATA_NUMBER_OF_FRAMES_NO_ENTRY_m11)
				printf_m11("Number of Frames: no entry\n");
			else
				printf_m11("Number of Frames: %ld\n", vmd2->number_of_frames);
			if (vmd2->frame_rate == VIDEO_METADATA_FRAME_RATE_NO_ENTRY_m11)
				printf_m11("Frame Rate: no entry\n");
			else if (vmd2->frame_rate == VIDEO_METADATA_FRAME_RATE_VARIABLE_m11)
				printf_m11("Frame Rate: variable\n");
			else
				printf_m11("Frame Rate: %lf (frames per second)\n", vmd2->frame_rate);
			if (vmd2->number_of_clips == VIDEO_METADATA_NUMBER_OF_CLIPS_NO_ENTRY_m11)
				printf_m11("Number of Clips: no entry\n");
			else
				printf_m11("Number of Clips: %ld (~= number of video indices)\n", vmd2->number_of_clips);
			if (vmd2->maximum_clip_bytes == VIDEO_METADATA_MAXIMUM_CLIP_BYTES_NO_ENTRY_m11)
				printf_m11("Maximum Clip Bytes: no entry\n");
			else
				printf_m11("Maximum Clip Bytes: %ld\n", vmd2->maximum_clip_bytes);
			if (vmd2->maximum_clip_frames == VIDEO_METADATA_MAXIMUM_CLIP_FRAMES_NO_ENTRY_m11)
				printf_m11("Maximum Clip Frames: no entry\n");
			else
				printf_m11("Maximum Clip Frames: %ld\n", vmd2->maximum_clip_frames);
			if (vmd2->number_of_video_files == VIDEO_METADATA_NUMBER_OF_VIDEO_FILES_NO_ENTRY_m11)
				printf_m11("Number of Video Files: no entry\n");
			else
				printf_m11("Number of Video Files: %d\n", vmd2->number_of_video_files);
			if (vmd2->maximum_clip_duration == VIDEO_METADATA_MAXIMUM_CLIP_DURATION_NO_ENTRY_m11)
				printf_m11("Maximum Clip Duration: no entry\n");
			else
				UTF8_printf_m11("Maximum Clip Duration: %lf %s\n", vmd2->maximum_clip_duration, vmd2->time_base_units_description);
			if (vmd2->number_of_discontinuities == VIDEO_METADATA_NUMBER_OF_DISCONTINUITIES_NO_ENTRY_m11)
				printf_m11("Number of Discontinuities: no entry\n");
			else
				printf_m11("Number of Discontinuities: %ld\n", vmd2->number_of_discontinuities);
			if (vmd2->maximum_contiguous_clips == VIDEO_METADATA_MAXIMUM_CONTIGUOUS_CLIPS_NO_ENTRY_m11)
				printf_m11("Maximum Contiguous Clips: no entry\n");
			else
				printf_m11("Maximum Contiguous Clips: %ld\n", vmd2->maximum_contiguous_clips);
			if (vmd2->maximum_contiguous_clip_bytes == VIDEO_METADATA_MAXIMUM_CONTIGUOUS_CLIP_BYTES_NO_ENTRY_m11)
				printf_m11("Maximum Contiguous Clip Bytes: no entry\n");
			else
				printf_m11("Maximum Contiguous Clip Bytes: %ld\n", vmd2->maximum_contiguous_clip_bytes);
			if (vmd2->maximum_contiguous_frames == VIDEO_METADATA_MAXIMUM_CONTIGUOUS_FRAMES_NO_ENTRY_m11)
				printf_m11("Maximum Contiguous Frames: no entry\n");
			else
				printf_m11("Maximum Contiguous Frames: %ld\n", vmd2->maximum_contiguous_frames);
			if (vmd2->horizontal_pixels == VIDEO_METADATA_HORIZONTAL_PIXELS_NO_ENTRY_m11)
				printf_m11("Horizontal Pixels: no entry\n");
			else
				printf_m11("Horizontal Pixels: %u\n", vmd2->horizontal_pixels);
			if (vmd2->vertical_pixels == VIDEO_METADATA_VERTICAL_PIXELS_NO_ENTRY_m11)
				printf_m11("Vertical Pixels: no entry\n");
			else
				printf_m11("Vertical Pixels: %u\n", vmd2->vertical_pixels);
			if (*vmd2->video_format)
				UTF8_printf_m11("Video Format: %s\n", vmd2->video_format);
			else
				printf_m11("Video Format: no entry\n");
		} else {
			printf_m11("(unrecognized metadata section 2 type)\n");
		}
	} else {
		printf_m11("No access to section 2\n");
	}
	printf_m11("------------------- Section 2 - END --------------------\n");
	printf_m11("------------------ Section 3 - START -------------------\n");
	if (md1->section_3_encryption_level <= NO_ENCRYPTION_m11) {
		if (md3->recording_time_offset == UUTC_NO_ENTRY_m11)
			printf_m11("Recording Time Offset: no entry\n");
		else
			printf_m11("Recording Time Offset: %ld\n", md3->recording_time_offset);
		if (md3->daylight_time_start_code.value == DTCC_VALUE_NO_ENTRY_m11) {
			printf_m11("Daylight Time Start Code: no entry\n");
		} else {
			generate_hex_string_m11((ui1 *) &md3->daylight_time_start_code.value, 8, hex_str);
			printf_m11("Daylight Time Start Code: %s\n", hex_str);
		}
		if (md3->daylight_time_end_code.value == DTCC_VALUE_NO_ENTRY_m11) {
			printf_m11("Daylight Time End Code: no entry\n");
		} else {
			generate_hex_string_m11((ui1 *) &md3->daylight_time_end_code.value, 8, hex_str);
			printf_m11("Daylight Time End Code: %s\n", hex_str);
		}
		if (*md3->standard_timezone_acronym)
			printf_m11("Standard Timezone Acronym: %s\n", md3->standard_timezone_acronym);
		else
			printf_m11("Standard Timezone Acronym: no entry\n");
		if (*md3->standard_timezone_string)
			printf_m11("Standard Timezone String: %s\n", md3->standard_timezone_string);
		else
			printf_m11("Standard Timezone String: no entry\n");
		if (*md3->daylight_timezone_acronym)
			printf_m11("Daylight Timezone Acronym: %s\n", md3->daylight_timezone_acronym);
		else
			printf_m11("Daylight Timezone Acronym: no entry\n");
		if (*md3->daylight_timezone_string)
			printf_m11("Daylight Timezone String: %s\n", md3->daylight_timezone_string);
		else
			printf_m11("Daylight Timezone String: no entry\n");
		if (*md3->subject_name_1)
			UTF8_printf_m11("Subject Name 1: %s\n", md3->subject_name_1);
		else
			printf_m11("Subject Name 1: no entry\n");
		if (*md3->subject_name_2)
			UTF8_printf_m11("Subject Name 2: %s\n", md3->subject_name_2);
		else
			printf_m11("Subject Name 2: no entry\n");
		if (*md3->subject_name_3)
			UTF8_printf_m11("Subject Name 3: %s\n", md3->subject_name_3);
		else
			printf_m11("Subject Name 3: no entry\n");
		if (*md3->subject_ID)
			UTF8_printf_m11("Subject ID: %s\n", md3->subject_ID);
		else
			printf_m11("Subject ID: no entry\n");
		if (*md3->recording_country)
			UTF8_printf_m11("Recording Country: %s\n", md3->recording_country);
		else
			printf_m11("Recording Country: no entry\n");
		if (*md3->recording_territory)
			UTF8_printf_m11("Recording Territory: %s\n", md3->recording_territory);
		else
			printf_m11("Recording Territory: no entry\n");
		if (*md3->recording_locality)
			UTF8_printf_m11("Recording Locality: %s\n", md3->recording_locality);
		else
			printf_m11("Recording Locality: no entry\n");
		if (*md3->recording_institution)
			UTF8_printf_m11("Recording Institution: %s\n", md3->recording_institution);
		else
			printf_m11("Recording Institution: no entry\n");
		if (*md3->geotag_format)
			UTF8_printf_m11("GeoTag Format: %s\n", md3->geotag_format);
		else
			printf_m11("GeoTag Format: no entry\n");
		if (*md3->geotag_data)
			UTF8_printf_m11("GeoTag Data: %s\n", md3->geotag_data);
		else
			printf_m11("GeoTag Data: no entry\n");
		if (md3->standard_UTC_offset == STANDARD_UTC_OFFSET_NO_ENTRY_m11)
			printf_m11("Standard UTC Offset: no entry\n");
		else
			printf_m11("Standard UTC Offset: %d\n", md3->standard_UTC_offset);
	} else {
		printf_m11("No access to section 3\n");
	}
	printf_m11("------------------- Section 3 - END --------------------\n");
	printf_m11("-------------------- Metadata - END --------------------\n\n");
	
	return;
}


void	show_password_data_m11(PASSWORD_DATA_m11 *pwd)
{
	si1	hex_str[HEX_STRING_BYTES_m11(ENCRYPTION_KEY_BYTES_m11)];
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// use message_m11() because show_password_data_m11() is used in normal (no programming) functions => so allow output to be suppressed easily
	if (pwd == NULL) {
		message_m11("\n-------------- Global Password Data - START --------------\n");
		pwd = &globals_m11->password_data;
	}
	else {
		message_m11("\n------------------ Password Data - START -----------------\n");
	}
	if (pwd->access_level >= LEVEL_1_ACCESS_m11) {
		generate_hex_string_m11(pwd->level_1_encryption_key, ENCRYPTION_KEY_BYTES_m11, hex_str);
		message_m11("Level 1 Encryption Key: %s\n", hex_str);
	}
	if (pwd->access_level == LEVEL_2_ACCESS_m11) {
		generate_hex_string_m11(pwd->level_2_encryption_key, ENCRYPTION_KEY_BYTES_m11, hex_str);
		message_m11("Level 2 Encryption Key: %s\n", hex_str);
	}
	show_password_hints_m11(pwd);
	message_m11("Access Level: %hhu\n", pwd->access_level);
	message_m11("Processed: %hhd\n", pwd->processed);
	message_m11("------------------- Password Data - END ------------------\n\n");
	
	return;
}


void	show_password_hints_m11(PASSWORD_DATA_m11 *pwd)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// use message_m11() because show_password_data_m11() is used in normal (not programming) functions => so allow output to be suppressed easily
	
	if (pwd == NULL)
		pwd = &globals_m11->password_data;
	if (*pwd->level_1_password_hint)
		message_m11("Level 1 Password Hint: %s\n", pwd->level_1_password_hint);
	if (*pwd->level_2_password_hint)
		message_m11("Level 2 Password Hint: %s\n", pwd->level_2_password_hint);
	
	return;
}


void	show_records_m11(FILE_PROCESSING_STRUCT_m11 *record_data_fps, si4 *record_filters)
{
	ui1			*ui1_p;
	si8			i, n_recs, r_cnt;
	RECORD_HEADER_m11	*rh;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// record_filters is a signed, "NULL terminated" array version of MED record type codes to include or exclude when reading records.
	// The terminal entry is NO_TYPE_CODE_m11 (== zero). NULL or no filter codes includes all records (== no filters).
	// filter modes: match positive: include
	//		 match negative: exclude
	//		 no match:
	//			all filters positive: exclude
	//			else: include
	// Note: as type codes are composed of ascii bytes values (< 0x80), it is always possible to make them negative without promotion.
	//
	// Example usage: si4	my_rec_filters[] = { REC_Sgmt_TYPE_CODE_m11, REC_Note_TYPE_CODE_m11, NO_TYPE_CODE_m11 };
	//
	// If the passed record_filters is NULL, the global record_filters will be used.
	// If the global record_filters are NULL, all records will be accepted.
	// If record_filters is a "zero-length" array (i.e. record_filters = { NO_TYPE_CODE_m11 }), all records will be accepted.
	// record_filters
	
	
	if (record_filters == NULL)
		record_filters = globals_m11->record_filters;	// if these too are NULL, no filters applied
	else if (*record_filters == NO_TYPE_CODE_m11)		// Note: if read_record_data_m11() with these filters was used to get records, filtering is already done
		record_filters = NULL;				// show all types, even if global filters are not NULL
	
	// show records
	n_recs = record_data_fps->number_of_items;
	ui1_p = (void *) record_data_fps->record_data;
	for (i = r_cnt = 0; i < n_recs; ++i) {
		rh = (RECORD_HEADER_m11 *) ui1_p;
		if (include_record_m11(rh->type_code, record_filters) == TRUE_m11)
			show_record_m11(record_data_fps, rh, ++r_cnt);
		ui1_p += rh->total_record_bytes;
	}
	
	return;
}


void	show_Sgmt_records_array_m11(LEVEL_HEADER_m11 *level_header, Sgmt_RECORD_m11 *Sgmt)
{
	si1	                time_str[TIME_STRING_BYTES_m11], hex_str[HEX_STRING_BYTES_m11(8)];
	si4			n_segs;
	si8			i;
	CHANNEL_m11		*chan;
	SESSION_m11		*sess;
	
	
	if (level_header != NULL) {
		switch (level_header->type_code) {
			case LH_TIME_SERIES_CHANNEL_m11:
			case LH_VIDEO_CHANNEL_m11:
				chan = (CHANNEL_m11 *) level_header;
				Sgmt = chan->Sgmt_records;
				break;
			case LH_SESSION_m11:
				sess = (SESSION_m11 *) level_header;
				Sgmt = sess->Sgmt_records;
				break;
			default:
				warning_message_m11("%s(): invalid level type\n", __FUNCTION__);
				return;
		}
	} else if (Sgmt == NULL) {
		warning_message_m11("%s(): both arguments are NULL\n", __FUNCTION__);
		return;
	}
	
	if (Sgmt == NULL) {
		warning_message_m11("%s(): NULL Sgmt records array\n", __FUNCTION__);
		return;
	}
	
	n_segs = globals_m11->number_of_session_segments;
	if (n_segs == 0) {
		warning_message_m11("%s(): empty Sgmt records array\n", __FUNCTION__);
		return;
	}
	
	for (i = 0; i < n_segs; ++i, ++Sgmt) {
		printf_m11("%sRecord number: %ld%s\n", TC_RED_m11, i + 1, TC_RESET_m11);
		if (Sgmt->start_time == RECORD_HEADER_START_TIME_NO_ENTRY_m11)
			printf_m11("Record Start Time: no entry\n");
		else {
			time_string_m11(Sgmt->start_time, time_str, TRUE_m11, FALSE_m11, FALSE_m11);
			printf_m11("Record Start Time: %ld (oUTC), %s\n", Sgmt->start_time, time_str);
		}
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
			printf_m11("Sampling Frequency: %lf\n\n", Sgmt->sampling_frequency);
	}

	return;
}


void    show_time_slice_m11(TIME_SLICE_m11 *slice)
{	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	printf_m11("Conditioned: ");
	if (slice->conditioned == TRUE_m11)
		printf_m11("true\n");
	else if (slice->conditioned == FALSE_m11)
		printf_m11("false\n");
	else if (slice->conditioned == UNKNOWN_m11)
		printf_m11("unknown\n");
	else
		printf_m11("invalid value (%hhd)\n", slice->conditioned);
	
	if (slice->number_of_segments == UNKNOWN_m11)
		printf_m11("Number of Segments: unknown\n");
	else if (slice->number_of_segments == EMPTY_SLICE_m11)
		printf_m11("Number of Segments: empty slice (segments missing)\n");
	else
		printf_m11("Number of Segments: %d\n", slice->number_of_segments);

	printf_m11("Start Time: ");
	if (slice->start_time == UUTC_NO_ENTRY_m11)
		printf_m11("no entry\n");
	else if (slice->start_time == BEGINNING_OF_TIME_m11)
		printf_m11("beginning of time\n");
	else if (slice->start_time == END_OF_TIME_m11)
		printf_m11("end of time\n");
	else
		printf_m11("%ld\n", slice->start_time);
	
	printf_m11("End Time: ");
	if (slice->end_time == UUTC_NO_ENTRY_m11)
		printf_m11("no entry\n");
	else if (slice->end_time == BEGINNING_OF_TIME_m11)
		printf_m11("beginning of time\n");
	else if (slice->end_time == END_OF_TIME_m11)
		printf_m11("end of time\n");
	else
		printf_m11("%ld\n", slice->end_time);
	
	printf_m11("Start Sample/Frame Number: ");
	if (slice->start_sample_number == SAMPLE_NUMBER_NO_ENTRY_m11)
		printf_m11("no entry\n");
	else if (slice->start_sample_number == END_OF_SAMPLE_NUMBERS_m11)
		printf_m11("end of samples/frames\n");
	else
		printf_m11("%ld\n", slice->start_sample_number);
	
	printf_m11("End Sample/Frame Number: ");
	if (slice->end_sample_number == SAMPLE_NUMBER_NO_ENTRY_m11)
		printf_m11("no entry\n");
	else if (slice->end_sample_number == END_OF_SAMPLE_NUMBERS_m11)
		printf_m11("end of samples/frames\n");
	else
		printf_m11("%ld\n", slice->end_sample_number);
	
	printf_m11("Start Segment Number: ");
	if (slice->start_segment_number == SEGMENT_NUMBER_NO_ENTRY_m11)
		printf_m11("no entry\n");
	else
		printf_m11("%d\n", slice->start_segment_number);
	
	printf_m11("End Segment Number: ");
	if (slice->end_segment_number == SEGMENT_NUMBER_NO_ENTRY_m11)
		printf_m11("no entry\n");
	else
		printf_m11("%d\n", slice->end_segment_number);
		
	printf_m11("\n");
	
	return;
}


void    show_timezone_info_m11(TIMEZONE_INFO_m11 *timezone_entry, TERN_m11 show_DST_detail)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	printf_m11("Country: %s\n", timezone_entry->country);
	printf_m11("Country Acronym (2 letter): %s\n", timezone_entry->country_acronym_2_letter);
	printf_m11("Country Acronym (3 letter): %s\n", timezone_entry->country_acronym_3_letter);
	if (*timezone_entry->territory)
		printf_m11("Territory: %s\n", timezone_entry->territory);
	if (*timezone_entry->territory_acronym)
		printf_m11("Territory Acronym: %s\n", timezone_entry->territory_acronym);
	printf_m11("Standard Timezone: %s\n", timezone_entry->standard_timezone);
	printf_m11("Standard Timezone Acronym: %s\n", timezone_entry->standard_timezone_acronym);
	printf_m11("Standard UTC Offset (secs): %d\n", timezone_entry->standard_UTC_offset);
	
	if (timezone_entry->daylight_time_start_code) {
		printf_m11("Daylight Timezone: %s\n", timezone_entry->daylight_timezone);
		printf_m11("Daylight Timezone Acronym: %s\n", timezone_entry->daylight_timezone_acronym);
		if (timezone_entry->daylight_time_start_code == DTCC_VALUE_NO_ENTRY_m11) {
			printf_m11("Daylight Time data is not available\n");
		} else if (show_DST_detail == TRUE_m11) {
			printf_m11("Daylight Time Start Code: 0x%lX\n", timezone_entry->daylight_time_start_code);
			show_daylight_change_code_m11((DAYLIGHT_TIME_CHANGE_CODE_m11 *) &timezone_entry->daylight_time_start_code, "\t");
			printf_m11("Daylight Time End Code: 0x%lX\n", timezone_entry->daylight_time_end_code);
			show_daylight_change_code_m11((DAYLIGHT_TIME_CHANGE_CODE_m11 *) &timezone_entry->daylight_time_end_code, "\t");
		}
	} else {
		printf_m11("Daylight Time is not observed\n");
	}
	return;
}


void	show_universal_header_m11(FILE_PROCESSING_STRUCT_m11 *fps, UNIVERSAL_HEADER_m11 *uh)
{
	TERN_m11        ephemeral_flag;
	si1             hex_str[HEX_STRING_BYTES_m11(PASSWORD_VALIDATION_FIELD_BYTES_m11)], time_str[TIME_STRING_BYTES_m11];
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// assign
	if (fps != NULL) {
		uh = fps->universal_header;
		if (fps->parameters.fd == FPS_FD_EPHEMERAL_m11)
			ephemeral_flag = TRUE_m11;
		else
			ephemeral_flag = FALSE_m11;
	} else {
		if (uh == NULL) {
			error_message_m11("%s(): invalid input\n", __FUNCTION__);
			return;
		}
		ephemeral_flag = UNKNOWN_m11;
	}
	
	printf_m11("---------------- Universal Header - START ----------------\n");
	if (uh->header_CRC == CRC_NO_ENTRY_m11)
		printf_m11("Header CRC: no entry\n");
	else {
		generate_hex_string_m11((ui1 *)&uh->header_CRC, CRC_BYTES_m11, hex_str);
		printf_m11("Header CRC: %s\n", hex_str);
	}
	if (uh->body_CRC == CRC_NO_ENTRY_m11)
		printf_m11("Body CRC: no entry\n");
	else {
		generate_hex_string_m11((ui1 *)&uh->body_CRC, CRC_BYTES_m11, hex_str);
		printf_m11("Body CRC: %s\n", hex_str);
	}
	if (uh->segment_end_time == UUTC_NO_ENTRY_m11)
		printf_m11("Segement End Time: no entry\n");
	else {
		time_string_m11(uh->segment_end_time, time_str, TRUE_m11, FALSE_m11, FALSE_m11);
		printf_m11("Segment End Time: %ld (oUTC), %s\n", uh->segment_end_time, time_str);
	}
	if (uh->number_of_entries == UNIVERSAL_HEADER_NUMBER_OF_ENTRIES_NO_ENTRY_m11)
		printf_m11("Number of Entries: no entry\n");
	else {
		printf_m11("Number of Entries: %ld  ", uh->number_of_entries);
		switch (uh->type_code) {
			case RECORD_DATA_FILE_TYPE_CODE_m11:
				printf_m11("(number of records in the file)\n");
				break;
			case RECORD_INDICES_FILE_TYPE_CODE_m11:
				printf_m11("(number of record indices in the file)\n");
				break;
			case TIME_SERIES_METADATA_FILE_TYPE_CODE_m11:
			case VIDEO_METADATA_FILE_TYPE_CODE_m11:
				if (ephemeral_flag == TRUE_m11)
					printf_m11("(maximum number of records in records files at this level and below)\n");
				else if (ephemeral_flag == FALSE_m11)
					printf_m11("(one metadata entry per metadata file)\n");
				else // UNKNOWN
					printf_m11("(one metadata entry, or maximum number of records in a records file at this level and below)\n");
				break;
			case VIDEO_INDICES_FILE_TYPE_CODE_m11:
				printf_m11("(number of video indices in the file)\n");
				break;
			case TIME_SERIES_DATA_FILE_TYPE_CODE_m11:
				printf_m11("(number of CMP blocks in the file)\n");
				break;
			case TIME_SERIES_INDICES_FILE_TYPE_CODE_m11:
				printf_m11("(number of time series indices in the file)\n");
				break;
			default:
				printf_m11("\n");
				break;
		}
	}
	if (uh->maximum_entry_size == UNIVERSAL_HEADER_MAXIMUM_ENTRY_SIZE_NO_ENTRY_m11)
		printf_m11("Maximum Entry Size: no entry\n");
	else {
		printf_m11("Maximum Entry Size: %u  ", uh->maximum_entry_size);
		switch (uh->type_code) {
			case RECORD_DATA_FILE_TYPE_CODE_m11:
				printf_m11("(number of bytes in the largest record in the file)\n");
				break;
			case RECORD_INDICES_FILE_TYPE_CODE_m11:
				printf_m11("(number of bytes in a record index)\n");
				break;
			case TIME_SERIES_METADATA_FILE_TYPE_CODE_m11:
			case VIDEO_METADATA_FILE_TYPE_CODE_m11:
				if (ephemeral_flag == TRUE_m11)
					printf_m11("(maximum number of bytes in a record at this level and below)\n");
				else if (ephemeral_flag == FALSE_m11)
					printf_m11("(number of bytes in a metadata structure)\n");
				else // UNKNOWN
					printf_m11("(metadata bytes, or maximum number of bytes in a record at this level and below)\n");
				break;
			case VIDEO_INDICES_FILE_TYPE_CODE_m11:
				printf_m11("(number of bytes in a video index)\n");
				break;
			case TIME_SERIES_DATA_FILE_TYPE_CODE_m11:
				printf_m11("(number of bytes in the largest CMP block in the file)\n");
				break;
			case TIME_SERIES_INDICES_FILE_TYPE_CODE_m11:
				printf_m11("(number of bytes in a time series index)\n");
				break;
			default:
				printf_m11("\n");
				break;
				
		}
	}
	if (uh->segment_number == UNIVERSAL_HEADER_SEGMENT_NUMBER_NO_ENTRY_m11)
		printf_m11("Segment Number: no entry\n");
	else if (uh->segment_number == UNIVERSAL_HEADER_CHANNEL_LEVEL_CODE_m11)
		printf_m11("Segment Number: channel level\n");
	else if (uh->segment_number == UNIVERSAL_HEADER_SESSION_LEVEL_CODE_m11)
		printf_m11("Segment Number: session level\n");
	else
		printf_m11("Segment Number: %d\n", uh->segment_number);
	if (*uh->type_string)
		printf_m11("File Type String: %s\n", uh->type_string);
	else
		printf_m11("File Type String: no entry\n");
	if (uh->MED_version_major == UNIVERSAL_HEADER_MED_VERSION_MAJOR_NO_ENTRY_m11 || uh->MED_version_minor == UNIVERSAL_HEADER_MED_VERSION_MINOR_NO_ENTRY_m11) {
		if (uh->MED_version_major == UNIVERSAL_HEADER_MED_VERSION_MAJOR_NO_ENTRY_m11)
			printf_m11("MED Version Major: no entry\n");
		else
			printf_m11("MED Version Major: %u\n", uh->MED_version_major);
		if (uh->MED_version_minor == UNIVERSAL_HEADER_MED_VERSION_MINOR_NO_ENTRY_m11)
			printf_m11("MED Version Minor: no entry\n");
		else
			printf_m11("MED Version Minor: %u\n", uh->MED_version_minor);
	}
	else
		printf_m11("MED Version: %u.%u\n", uh->MED_version_major, uh->MED_version_minor);
	if (uh->byte_order_code == UNIVERSAL_HEADER_BYTE_ORDER_CODE_NO_ENTRY_m11)
		printf_m11("Byte Order Code: no entry ");
	else {
		printf_m11("Byte Order Code: %u ", uh->byte_order_code);
		if (uh->byte_order_code == LITTLE_ENDIAN_m11)
			printf_m11("(little endian)\n");
		else if (uh->byte_order_code == BIG_ENDIAN_m11)
			printf_m11("(big endian)\n");
		else
			printf_m11("(unrecognized code)\n");
	}
	if (uh->session_start_time == UUTC_NO_ENTRY_m11)
		printf_m11("Session Start Time: no entry\n");
	else {
		time_string_m11(uh->session_start_time, time_str, TRUE_m11, FALSE_m11, FALSE_m11);
		printf_m11("Session Start Time: %ld (oUTC), %s\n", uh->session_start_time, time_str);
	}
	if (uh->segment_start_time == UUTC_NO_ENTRY_m11)
		printf_m11("Segment Start Time: no entry\n");
	else {
		time_string_m11(uh->segment_start_time, time_str, TRUE_m11, FALSE_m11, FALSE_m11);
		printf_m11("Segment Start Time: %ld (oUTC), %s\n", uh->segment_start_time, time_str);
	}
	if (*uh->session_name)
		UTF8_printf_m11("Session Name: %s\n", uh->session_name);
	else
		printf_m11("Session Name: no entry\n");
	if (*uh->channel_name)
		UTF8_printf_m11("Channel Name: %s\n", uh->channel_name);
	else
		printf_m11("Channel Name: no entry\n");
	if (*uh->anonymized_subject_ID)
		UTF8_printf_m11("Anonymized Subject ID: %s\n", uh->anonymized_subject_ID);
	else
		printf_m11("Anonymized Subject ID: no entry\n");
	if (uh->session_UID == UID_NO_ENTRY_m11)
		printf_m11("Session UID: no entry\n");
	else {
		generate_hex_string_m11((ui1 *)&uh->session_UID, UID_BYTES_m11, hex_str);
		printf_m11("Session UID: %s\n", hex_str);
	}
	if (uh->channel_UID == UID_NO_ENTRY_m11)
		printf_m11("Channel UID: no entry\n");
	else {
		generate_hex_string_m11((ui1 *)&uh->channel_UID, UID_BYTES_m11, hex_str);
		printf_m11("Channel UID: %s\n", hex_str);
	}
	if (uh->segment_UID == UID_NO_ENTRY_m11)
		printf_m11("Segment UID: no entry\n");
	else {
		generate_hex_string_m11((ui1 *)&uh->segment_UID, UID_BYTES_m11, hex_str);
		printf_m11("Segment UID: %s\n", hex_str);
	}
	if (uh->file_UID == UID_NO_ENTRY_m11)
		printf_m11("File UID: no entry\n");
	else {
		generate_hex_string_m11((ui1 *)&uh->file_UID, UID_BYTES_m11, hex_str);
		printf_m11("File UID: %s\n", hex_str);
	}
	if (uh->provenance_UID == UID_NO_ENTRY_m11)
		printf_m11("Provenance UID: no entry\n");
	else {
		generate_hex_string_m11((ui1 *)&uh->provenance_UID, UID_BYTES_m11, hex_str);
		printf_m11("Provenance UID: %s  ", hex_str);
		if (uh->provenance_UID == uh->file_UID)
			printf_m11("(original data)\n");
		else
			printf_m11("(derived data)\n");
	}
	if (all_zeros_m11(uh->level_1_password_validation_field, PASSWORD_VALIDATION_FIELD_BYTES_m11) == TRUE_m11)
		printf_m11("Level 1 Password Validation_Field: no entry\n");
	else {
		generate_hex_string_m11(uh->level_1_password_validation_field, PASSWORD_VALIDATION_FIELD_BYTES_m11, hex_str);
		printf_m11("Level 1 Password Validation_Field: %s\n", hex_str);
	}
	if (all_zeros_m11(uh->level_2_password_validation_field, PASSWORD_VALIDATION_FIELD_BYTES_m11) == TRUE_m11)
		printf_m11("Level 2 Password Validation_Field: no entry\n");
	else {
		generate_hex_string_m11(uh->level_2_password_validation_field, PASSWORD_VALIDATION_FIELD_BYTES_m11, hex_str);
		printf_m11("Level 2 Password Validation_Field: %s\n", hex_str);
	}
	if (all_zeros_m11(uh->level_3_password_validation_field, PASSWORD_VALIDATION_FIELD_BYTES_m11) == TRUE_m11)
		printf_m11("Level 3 Password Validation_Field: no entry\n");
	else {
		generate_hex_string_m11(uh->level_3_password_validation_field, PASSWORD_VALIDATION_FIELD_BYTES_m11, hex_str);
		printf_m11("Level 3 Password Validation_Field: %s\n", hex_str);
	}
	printf_m11("---------------- Universal Header - END ----------------\n\n");
	
	return;
}


TERN_m11	sort_channels_by_acq_num_m11(SESSION_m11 *sess)
{
	TERN_m11			read_metadata;
	si1				seg_dir[FULL_FILE_NAME_BYTES_m11], md_file[FULL_FILE_NAME_BYTES_m11], num_str[FILE_NUMBERING_DIGITS_m11 + 1];
	si4				i, n_chans, seg_idx;
	CHANNEL_m11			*chan;
	SEGMENT_m11			*seg;
	FILE_PROCESSING_STRUCT_m11	*md_fps;
	ACQ_NUM_SORT_m11		*acq_idxs;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif

	// Currently function only sort time series channels
	// Returns TRUE if sorted, FALSE if duplicate numbers exist, or other error condition
	
	n_chans = sess->number_of_time_series_channels;
	if (n_chans == 0) {
		warning_message_m11("%s(): no time series channels allocated\n", __FUNCTION__);
		return(FALSE_m11);
	}
	
	// build ACQ_NUM_SORT_m11 array
	acq_idxs = (ACQ_NUM_SORT_m11 *) malloc(n_chans * sizeof(ACQ_NUM_SORT_m11));
	seg_idx = get_segment_index_m11(FIRST_OPEN_SEGMENT_m11);
	*num_str = 0;
	for (i = 0; i < n_chans; ++i) {
		chan = sess->time_series_channels[i];
		read_metadata = FALSE_m11;
		if (chan->segments == NULL) {
			read_metadata = TRUE_m11;
		} else {
			seg = chan->segments[seg_idx];
			if (seg == NULL)
				read_metadata = TRUE_m11;
			else if (seg->metadata_fps == NULL)
				read_metadata = TRUE_m11;
		}
		if (read_metadata == TRUE_m11) {
			if (*num_str == 0)
				numerical_fixed_width_string_m11(num_str, FILE_NUMBERING_DIGITS_m11, seg_idx + 1);
			sprintf_m11(seg_dir, "%s/%s_s%s.%s", chan->path, chan->name, num_str, TIME_SERIES_SEGMENT_DIRECTORY_TYPE_STRING_m11);
			sprintf_m11(md_file, "%s/%s_s%s.%s", seg_dir, chan->name, num_str, TIME_SERIES_METADATA_FILE_TYPE_STRING_m11);
			if (file_exists_m11(md_file) == FILE_EXISTS_m11) {
				md_fps = read_file_m11(NULL, md_file, 0, 0, FPS_FULL_FILE_m11, NULL, NULL, USE_GLOBAL_BEHAVIOR_m11);
				if (md_fps == NULL) {
					warning_message_m11("%s(): error reading metadata file \"%s\"\n", __FUNCTION__, md_file);
					free((void *) acq_idxs);
					return(FALSE_m11);
				}
			} else {
				warning_message_m11("%s(): metadata file \"%s\" is missing\n", __FUNCTION__, md_file);
				free((void *) acq_idxs);
				return(FALSE_m11);
			}
			acq_idxs[i].acq_num = md_fps->metadata->time_series_section_2.acquisition_channel_number;
			FPS_free_processing_struct_m11(md_fps, TRUE_m11);
		} else {
			seg = chan->segments[seg_idx];
			acq_idxs[i].acq_num = seg->metadata_fps->metadata->time_series_section_2.acquisition_channel_number;
		}
		acq_idxs[i].chan = chan;
	}
	
	// sort it
	qsort((void *) acq_idxs, (size_t) n_chans, sizeof(ACQ_NUM_SORT_m11), compare_acq_nums_m11);
	
	// check for duplicates
	for (i = 1; i < n_chans; ++i)
		if (acq_idxs[i].acq_num == acq_idxs[i - 1].acq_num)
			break;
	if (i < n_chans) {
		warning_message_m11("%s(): duplicate acquisition channel numbers => not sorting\n", __FUNCTION__);
		free((void *) acq_idxs);
		return(FALSE_m11);
	}

	// move channel pointers
	for (i = 0; i < n_chans; ++i)
		sess->time_series_channels[i] = acq_idxs[i].chan;
	free((void *) acq_idxs);

	return(TRUE_m11);
}


si1	*time_string_m11(si8 uutc, si1 *time_str, TERN_m11 fixed_width, TERN_m11 relative_days, si4 colored_text, ...)  // time_str buffer should be of length TIME_STRING_BYTES_m11
{
	si1			*standard_timezone_acronym, *standard_timezone_string, *date_color, *time_color, *color_reset, *meridian;
	static si1      	private_time_str[TIME_STRING_BYTES_m11];
	static si1      	*mos[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	static si1      	*months[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
	static si1      	*wdays[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	static si1      	*mday_num_sufs[32] = {	"", "st", "nd", "rd", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", \
							"th", "th", "st", "nd", "rd", "th", "th", "th", "th", "th", "th", "th", "st" };
	static si1      	*weekdays[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
	TERN_m11		offset;
	si4             	microseconds, DST_offset, day_num;
	time_t             	local_time, test_time;
	sf8             	UTC_offset_hours;
	va_list         	arg_p;
	struct tm       	ti;
	LOCATION_INFO_m11	loc_info = {0};
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// Note if NULL is passed for time_str, this function is not thread-safe
	if (time_str == NULL)
		time_str = private_time_str;
	
	switch (uutc) {
		case UUTC_NO_ENTRY_m11:
			strcpy(time_str, "no entry");
			return(time_str);
		case BEGINNING_OF_TIME_m11:
			strcpy(time_str, "beginning of time");
			return(time_str);
		case END_OF_TIME_m11:
			strcpy(time_str, "end of time");
			return(time_str);
		case CURRENT_TIME_m11:
			uutc = current_uutc_m11();
			if (globals_m11->time_constants_set == FALSE_m11)  // set global time constants to location of machine
				if (get_location_info_m11(&loc_info, TRUE_m11, FALSE_m11) == NULL)
					warning_message_m11("%s(): daylight change data not available\n", __FUNCTION__);
			break;
	}
	
	if (globals_m11->RTO_known == FALSE_m11) {  // FALSE_m11 used to mean unknown and relevant.
		relative_days = offset = TRUE_m11;  // force relative days if using oUTC - nobody needs to know the 1970 date
	} else {  // use UNKNOWN_m11 (0) for cases in which recording time offset is irrelevant (e.g. times not associated with MED files)
		test_time = uutc - globals_m11->recording_time_offset;
		if (test_time < 0)  // time is offset
			uutc += globals_m11->recording_time_offset;
		offset = FALSE_m11;
	}
	DST_offset = DST_offset_m11(uutc);
	
	standard_timezone_acronym = globals_m11->standard_timezone_acronym;
	standard_timezone_string = globals_m11->standard_timezone_string;
	if (offset == FALSE_m11) {
		if (strncmp(standard_timezone_string, "offset", 6) == 0) {
			standard_timezone_acronym = "UTC";
			standard_timezone_string = "Coordinated Universal Time";
		}
	}
	local_time = (si8) (uutc / (si8) 1000000) + (si8) (globals_m11->standard_UTC_offset + DST_offset);
	microseconds = (si4) (uutc % (si8) 1000000);
#if defined MACOS_m11 || defined LINUX_m11
	gmtime_r(&local_time, &ti);
#endif
#ifdef WINDOWS_m11
	ti = *(gmtime(&local_time));
#endif
	ti.tm_year += 1900;
	
	if (colored_text == TRUE_m11) {
		va_start(arg_p, colored_text);
		date_color = va_arg(arg_p, si1 *);
		time_color = va_arg(arg_p, si1 *);
		va_end(arg_p);
		color_reset = TC_RESET_m11;
	} else {
		date_color = time_color = color_reset = "";
	}
	if (relative_days == TRUE_m11) {
		uutc -= globals_m11->recording_time_offset;
		day_num = (si4)(uutc / TWENTY_FOURS_HOURS_m11) + 1;
	}
	
	if (fixed_width == TRUE_m11) {
		UTC_offset_hours = (sf8)(DST_offset + globals_m11->standard_UTC_offset) / (sf8)3600.0;
		if (relative_days == TRUE_m11)
			sprintf_m11(time_str, "%sDay %04d  %s%02d:%02d:%02d.%06d", date_color, day_num, time_color, ti.tm_hour, ti.tm_min, ti.tm_sec, microseconds);
		else
			sprintf_m11(time_str, "%s%s %02d %s %d  %s%02d:%02d:%02d.%06d", date_color, wdays[ti.tm_wday], ti.tm_mday, mos[ti.tm_mon], ti.tm_year, time_color, ti.tm_hour, ti.tm_min, ti.tm_sec, microseconds);
		if (DST_offset) {
			if (UTC_offset_hours >= 0.0)
				sprintf_m11(time_str, "%s %s (UTC +%0.2lf)%s", time_str, globals_m11->daylight_timezone_acronym, UTC_offset_hours, color_reset);
			else
				sprintf_m11(time_str, "%s %s (UTC %0.2lf)%s", time_str, globals_m11->daylight_timezone_acronym, UTC_offset_hours, color_reset);
		}
		else {
			if (offset == TRUE_m11)  // no UTC offset displayed
				sprintf_m11(time_str, "%s %s%s", time_str, standard_timezone_acronym, color_reset);
			else if (UTC_offset_hours >= 0.0)
				sprintf_m11(time_str, "%s %s (UTC +%0.2lf)%s", time_str, standard_timezone_acronym, UTC_offset_hours, color_reset);
			else
				sprintf_m11(time_str, "%s %s (UTC %0.2lf)%s", time_str, standard_timezone_acronym, UTC_offset_hours, color_reset);
		}
	}
	else {
		ti.tm_sec += ((microseconds + 5e5) / 1e6);  // round to nearest second
		if (ti.tm_hour < 12) {
			meridian = "AM";
			if (ti.tm_hour == 0)
				ti.tm_hour = 12;
		}
		else {
			meridian = "PM";
			if (ti.tm_hour > 12)
				ti.tm_hour -= 12;
		}
		if (relative_days == TRUE_m11)
			sprintf_m11(time_str, "%sDay %04d  %s%d:%02d:%02d %s,", date_color, day_num, time_color, ti.tm_hour, ti.tm_min, ti.tm_sec, meridian);
		else
			sprintf_m11(time_str, "%s%s, %s %d%s, %d  %s%d:%02d:%02d %s,", date_color, weekdays[ti.tm_wday], months[ti.tm_mon], ti.tm_mday, mday_num_sufs[ti.tm_mday], ti.tm_year, time_color, ti.tm_hour, ti.tm_min, ti.tm_sec, meridian);
		if (DST_offset)
			sprintf_m11(time_str, "%s %s%s", time_str, globals_m11->daylight_timezone_string, color_reset);
		else
				sprintf_m11(time_str, "%s %s%s", time_str, standard_timezone_string, color_reset);
	}
	
	return(time_str);
}


si8     uutc_for_frame_number_m11(LEVEL_HEADER_m11 *level_header, si8 target_frame_number, ui4 mode, ...)  // varargs: si8 ref_frame_number, si8 ref_uutc, sf8 frame_rate
{
	si1			tmp_str[FULL_FILE_NAME_BYTES_m11], num_str[FILE_NUMBERING_DIGITS_m11 + 1];
	si4			seg_num, seg_idx;
	si8                     uutc, absolute_numbering_offset, ref_frame_number;
	si8			ref_uutc, i, n_inds, tmp_si8;
	sf8                     tmp_sf8, frame_rate;
	ui4			mask;
	va_list			args;
	SEGMENT_m11		*seg;
	CHANNEL_m11		*chan;
	SESSION_m11		*sess;
	VIDEO_INDEX_m11		*vi;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// uutc_for_frame_number_m11(NULL, si8 target_frame_number, ui4 mode, si8 ref_frame_number, si8 ref_uutc, sf8 frame_rate)
	// returns uutc extrapolated from ref_uutc
	// NOTE: target_frame_number must be session-relative (global indexing)
	
	// uutc_for_frame_number_m11(seg, target_uutc, mode)
	// returns uutc extrapolated from closest video index in frame specified by mode (this is typically more accurate, & takes discontinuities into account)
	
	// frame time is defined as the period from frame onset until the next frame
	// mode FIND_START_m11 (default): first uutc >= start of target frame period
	// mode FIND_END_m11: last uutc < start of next frame period
	// mode FIND_CENTER_m11: uutc closest to the center of the frame period
	
	if (level_header == NULL) {  // reference points passed
		va_start(args, mode);
		ref_frame_number = va_arg(args, si8);
		ref_uutc = va_arg(args, si8);
		frame_rate = va_arg(args, sf8);
		va_end(args);
		vi = NULL;
	} else {  // level header passed
		switch (level_header->type_code) {
			case LH_VIDEO_SEGMENT_m11:
				seg = (SEGMENT_m11 *) level_header;
				break;
			case LH_VIDEO_CHANNEL_m11:
			case LH_SESSION_m11:
				seg_num = segment_for_frame_number_m11(level_header, target_frame_number);
				seg_idx = get_segment_index_m11(seg_num);
				if (seg_idx == FALSE_m11)
					return(UUTC_NO_ENTRY_m11);
				if (level_header->type_code == LH_VIDEO_CHANNEL_m11) {
					chan = (CHANNEL_m11 *) level_header;
				} else {
					chan = globals_m11->reference_channel;
					if (chan->type_code != LH_VIDEO_CHANNEL_m11) {
						sess = (SESSION_m11 *) level_header;
						chan = sess->video_channels[0];
					}
				}
				seg = chan->segments[seg_idx];
				break;
			case LH_TIME_SERIES_CHANNEL_m11:
			case LH_TIME_SERIES_SEGMENT_m11:
				return(uutc_for_sample_number_m11(level_header, target_frame_number, mode));
			default:
				error_message_m11("%s(): invalid level type\n", __FUNCTION__);
				return(UUTC_NO_ENTRY_m11);
		}
		// open segment
		if (seg == NULL) {
			numerical_fixed_width_string_m11(num_str, FILE_NUMBERING_DIGITS_m11, seg_num);
			sprintf_m11(tmp_str, "%s/%s_s%s.%s", chan->path, chan->name, num_str, VIDEO_SEGMENT_DIRECTORY_TYPE_STRING_m11);
			seg = chan->segments[seg_idx] = open_segment_m11(NULL, NULL, tmp_str, chan->flags, NULL);
		} else if (!(seg->flags & LH_OPEN_m11)) {
			open_segment_m11(seg, NULL, NULL, seg->flags, NULL);
		}
		if (seg == NULL) {
			warning_message_m11("%s(): can't open segment\n", __FUNCTION__);
			return(UUTC_NO_ENTRY_m11);
		}

		vi = seg->video_indices_fps->video_indices;
		if (vi == NULL) {
			warning_message_m11("%s(): video indices are NULL => returning UUTC_NO_ENTRY_m11\n", __FUNCTION__);
			return(UUTC_NO_ENTRY_m11);
		}
		n_inds = seg->video_indices_fps->universal_header->number_of_entries - 1;  // account for terminal index here - cleaner code below
		
		i = find_index_m11(seg, target_frame_number, SAMPLE_SEARCH_m11);
		if (i == -1) {  // target frame earlier than segment start => return segment start time
			i = vi->start_time;
			return(i);
		}
		vi += i;
		if (i == n_inds) {  // target frame later than segment end => return segment end uutc
			i = vi->start_time - 1;
			return(i);
		}
		
		// make target_frame_number relative
		absolute_numbering_offset = seg->metadata_fps->metadata->video_section_2.absolute_start_frame_number;
		target_frame_number -= absolute_numbering_offset;

		ref_uutc = vi->start_time;
		ref_frame_number = vi->start_frame_number;
		++vi;  // advance to next index
		if (vi->file_offset > 0) {  // get local frame rate, unless discontinuity
			frame_rate = (sf8) (vi->start_frame_number - ref_frame_number);
			frame_rate /= ((sf8) (vi->start_time - ref_uutc) / (sf8) 1e6);
		} else {
			frame_rate = seg->metadata_fps->metadata->video_section_2.frame_rate;
		}
	}
	
	tmp_sf8 = (sf8) (target_frame_number - ref_frame_number) * (sf8) 1e6;
	mask = (ui4) (FIND_END_m11 | FIND_CENTER_m11 | FIND_START_m11);
	switch (mode & mask) {
		case FIND_END_m11:
			tmp_sf8 = (tmp_sf8 + (sf8) 1e6) / frame_rate;
			tmp_si8 = (si8) tmp_sf8;
			if (tmp_sf8 == (sf8) tmp_si8)
				--tmp_si8;
			break;
		case FIND_CENTER_m11:
			tmp_si8 = (si8) (((tmp_sf8 + (sf8) 5e5) / frame_rate) + (sf8) 0.5);
			break;
		case FIND_START_m11:
		default:
			tmp_sf8 = tmp_sf8 / frame_rate;
			tmp_si8 = (si8) tmp_sf8;
			if (tmp_sf8 != (sf8) tmp_si8)
				++tmp_si8;
			break;
	}
	
	uutc = ref_uutc + tmp_si8;
	if (vi != NULL)
		if (uutc >= vi->start_time)
			uutc = vi->start_time - 1;

	return(uutc);
}


si8     uutc_for_sample_number_m11(LEVEL_HEADER_m11 *level_header, si8 target_sample_number, ui4 mode, ...)  // varargs: si8 ref_sample_number, si8 ref_uutc, sf8 sampling_frequency
{
	si1			tmp_str[FULL_FILE_NAME_BYTES_m11], num_str[FILE_NUMBERING_DIGITS_m11 + 1];
	si4			seg_num, seg_idx;
	si8                     uutc, absolute_numbering_offset, ref_sample_number;
	si8			ref_uutc, n_inds, i, tmp_si8;
	sf8                     tmp_sf8, sampling_frequency;
	ui4			mask;
	va_list			args;
	SEGMENT_m11		*seg;
	CHANNEL_m11		*chan;
	SESSION_m11		*sess;
	TIME_SERIES_INDEX_m11	*tsi;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// uutc_for_sample_number_m11(NULL, si8 target_sample_number, ui4 mode, si8 ref_sample_number, si8 ref_uutc, sf8 sampling_frequency)
	// returns uutc extrapolated from ref_uutc
	// NOTE: target_sample_number must be session-relative (global indexing)

	// uutc_for_sample_number_m11(seg, target_uutc, mode)
	// returns uutc extrapolated from closest time series index in frame specified by mode (this is typically more accurate, & takes discontinuities into account)
	
	// sample time is defined as the period from sample onset until the next sample
	// mode FIND_START_m11 (default): first uutc >= start of target sample period
	// mode FIND_END_m11: last uutc < start of next sample period
	// mode FIND_CENTER_m11: uutc closest to the center of the sample period
	
	if (level_header == NULL) {  // reference points passed
		va_start(args, mode);
		ref_sample_number = va_arg(args, si8);
		ref_uutc = va_arg(args, si8);
		sampling_frequency = va_arg(args, sf8);
		va_end(args);
		tsi = NULL;
	} else {  // level header passed
		switch (level_header->type_code) {
			case LH_TIME_SERIES_SEGMENT_m11:
				seg = (SEGMENT_m11 *) level_header;
				break;
			case LH_TIME_SERIES_CHANNEL_m11:
			case LH_SESSION_m11:
				seg_num = segment_for_sample_number_m11(level_header, target_sample_number);
				seg_idx = get_segment_index_m11(seg_num);
				if (seg_idx == FALSE_m11)
					return(UUTC_NO_ENTRY_m11);
				if (level_header->type_code == LH_TIME_SERIES_CHANNEL_m11) {
					chan = (CHANNEL_m11 *) level_header;
				} else {
					chan = globals_m11->reference_channel;
					if (chan->type_code != LH_TIME_SERIES_CHANNEL_m11) {
						sess = (SESSION_m11 *) level_header;
						chan = sess->time_series_channels[0];
					}
				}
				seg = chan->segments[seg_idx];
				break;
			case LH_VIDEO_CHANNEL_m11:
			case LH_VIDEO_SEGMENT_m11:
				return(uutc_for_frame_number_m11(level_header, target_sample_number, mode));
			default:
				warning_message_m11("%s(): invalid level type\n", __FUNCTION__);
				return(UUTC_NO_ENTRY_m11);
		}
		// open segment
		if (seg == NULL) {
			numerical_fixed_width_string_m11(num_str, FILE_NUMBERING_DIGITS_m11, seg_num);
			sprintf_m11(tmp_str, "%s/%s_s%s.%s", chan->path, chan->name, num_str, TIME_SERIES_SEGMENT_DIRECTORY_TYPE_STRING_m11);
			seg = chan->segments[seg_idx] = open_segment_m11(NULL, NULL, tmp_str, chan->flags, NULL);
		} else if (!(seg->flags & LH_OPEN_m11)) {
			open_segment_m11(seg, NULL, NULL, seg->flags, NULL);
		}
		if (seg == NULL) {
			warning_message_m11("%s(): can't open segment\n", __FUNCTION__);
			return(UUTC_NO_ENTRY_m11);
		}

		tsi = seg->time_series_indices_fps->time_series_indices;
		if (tsi == NULL) {
			warning_message_m11("%s(): time series indices are NULL => returning UUTC_NO_ENTRY_m11\n", __FUNCTION__);
			return(UUTC_NO_ENTRY_m11);
		}
		n_inds = seg->time_series_indices_fps->universal_header->number_of_entries - 1;  // account for terminal index here - cleaner code below
		
		i = find_index_m11(seg, target_sample_number, SAMPLE_SEARCH_m11);
		if (i == -1) {  // target sample earlier than segment start => return segment start time
			i = tsi->start_time;
			return(i);
		}
		tsi += i;
		if (i == n_inds) {  // target sample later than segment end => return segment end uutc
			i = tsi->start_time - 1;
			return(i);
		}
		
		// make target_sample_number relative
		absolute_numbering_offset = seg->metadata_fps->metadata->time_series_section_2.absolute_start_sample_number;
		target_sample_number -= absolute_numbering_offset;

		ref_uutc = tsi->start_time;
		ref_sample_number = tsi->start_sample_number;
		++tsi;  // advance to next index
		if (tsi->file_offset > 0) {  // get local sampling frequency, unless discontinuity
			sampling_frequency = (sf8) (tsi->start_sample_number - ref_sample_number);
			sampling_frequency /= ((sf8) (tsi->start_time - ref_uutc) / (sf8) 1e6);
		} else {
			sampling_frequency = seg->metadata_fps->metadata->time_series_section_2.sampling_frequency;
		}
	}
	
	tmp_sf8 = (sf8) (target_sample_number - ref_sample_number) * (sf8) 1e6;
	mask = (ui4) (FIND_END_m11 | FIND_CENTER_m11 | FIND_START_m11);
	switch (mode & mask) {
		case FIND_END_m11:
			tmp_sf8 = (tmp_sf8 + (sf8) 1e6) / sampling_frequency;
			tmp_si8 = (si8) tmp_sf8;
			if (tmp_sf8 == (sf8) tmp_si8)
				--tmp_si8;
			break;
		case FIND_CENTER_m11:
			tmp_si8 = (si8) (((tmp_sf8 + (sf8) 5e5) / sampling_frequency) + (sf8) 0.5);
			break;
		case FIND_START_m11:
		default:
			tmp_sf8 = tmp_sf8 / sampling_frequency;
			tmp_si8 = (si8) tmp_sf8;
			if (tmp_sf8 != (sf8) tmp_si8)
				++tmp_si8;
			break;
	}
	
	uutc = ref_uutc + tmp_si8;
	if (tsi != NULL)
		if (uutc >= tsi->start_time)
			uutc = tsi->start_time - 1;
	
	return(uutc);
}


void    unescape_chars_m11(si1 *string, si1 target_char)
{
	si1	*c1, *c2, backslash;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	backslash = (si1) 0x5c;
	c1 = c2 = string;
	while (*c1) {
		if (*c1 == backslash) {
			if (*(c1 + 1) == target_char) {
				++c1;
				continue;
			}
		}
		*c2++ = *c1++;
	}
	*c2 = 0;
	
	return;
}


#ifdef WINDOWS_m11
FILETIME	uutc_to_win_time_m11(si8 uutc)
{
	FILETIME ft;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	uutc += WIN_USECS_TO_EPOCH_m11;
	uutc *= WIN_TICKS_PER_USEC_m11;
	
	ft.dwLowDateTime = (ui4) ((ui8) uutc & 0x00000000ffffffff);
	ft.dwHighDateTime = (ui4) ((ui8) uutc >> 32);
	
	return(ft);
}
#endif


TERN_m11        validate_record_data_CRCs_m11(FILE_PROCESSING_STRUCT_m11 *fps)
{
	TERN_m11        	valid;
	si8             	i;
	RECORD_HEADER_m11	*rh;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	valid = TRUE_m11;
	rh = (RECORD_HEADER_m11	*) fps->record_data;
	for (i = fps->number_of_items; i--;) {
		
		valid = CRC_validate_m11((ui1 *) rh + RECORD_HEADER_RECORD_CRC_START_OFFSET_m11, rh->total_record_bytes - RECORD_HEADER_RECORD_CRC_START_OFFSET_m11, rh->record_CRC);
		if (valid == FALSE_m11)
			return(valid);
		
		rh = (RECORD_HEADER_m11 *) ((ui1 *) rh + rh->total_record_bytes);
	}
	
	return(valid);
}


TERN_m11        validate_time_series_data_CRCs_m11(FILE_PROCESSING_STRUCT_m11 *fps)
{
	TERN_m11        		valid;
	si8             		i;
	CMP_BLOCK_FIXED_HEADER_m11	*bh;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	valid = TRUE_m11;
	bh = fps->parameters.cps->block_header;
	for (i = fps->number_of_items; i--;) {
		
		valid = CRC_validate_m11((ui1 *) bh + CMP_BLOCK_CRC_START_OFFSET_m11, bh->total_block_bytes - CMP_BLOCK_CRC_START_OFFSET_m11, bh->block_CRC);
		if (valid == FALSE_m11)
			return(valid);
		
		bh = (CMP_BLOCK_FIXED_HEADER_m11 *) ((ui1 *) bh + bh->total_block_bytes);
	}
	
	return(valid);
}


void    warning_message_m11(si1 *fmt, ...)
{
	va_list		args;
	

	// GREEN suppressible text to stderr
	if (!(globals_m11->behavior_on_fail & SUPPRESS_WARNING_OUTPUT_m11)) {
#ifndef MATLAB_m11
		fprintf(stderr, TC_GREEN_m11);
#endif
		va_start(args, fmt);
		UTF8_vfprintf_m11(stderr, fmt, args);
		va_end(args);
#ifndef MATLAB_m11
		fprintf(stderr, TC_RESET_m11);
		fflush(stderr);
#endif
	}
	
	return;
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si1	*wchar2char_m11(si1 *target, wchar_t *source)
{
	si1	*c, *c2;
	si8	len, wsz;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// if source == target, done in place
	// if not actually ascii, results may be weird
	
	wsz = sizeof(wchar_t);  // 2 or 4 => varies by OS & compiler
	c = target;
	c2 = (si1 *) source - wsz;
	len = wcslen(source);
	
	while (len--)
		*c++ = *(c2 += wsz);  // little endian version
	*c = 0;
	
	return(target);
}


void    win_cleanup_m11(void)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
#ifdef WINDOWS_m11
	#ifdef NEED_WIN_SOCKETS_m11
		WSACleanup();
	#endif
	
	#ifndef MATLAB_m11
		win_reset_terminal_m11();
	#endif
#endif
	return;
}
	

void win_clear_m11(void)
{
#ifdef WINDOWS_m11
	HANDLE				hStdout;
	CONSOLE_SCREEN_BUFFER_INFO	csbi;
	SMALL_RECT			scrollRect;
	COORD				scrollTarget;
	CHAR_INFO			fill;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

	// Get the number of character cells in the current buffer.
	if (!GetConsoleScreenBufferInfo(hStdout, &csbi))
		return;
	
	// Scroll the rectangle of the entire buffer.
	scrollRect.Left = 0;
	scrollRect.Top = 0;
	scrollRect.Right = csbi.dwSize.X;
	scrollRect.Bottom = csbi.dwSize.Y;
	
	// Scroll it upwards off the top of the buffer with a magnitude of the entire height.
	scrollTarget.X = 0;
	scrollTarget.Y = (SHORT)(0 - csbi.dwSize.Y);
	
	// Fill with empty spaces with the buffer's default text attribute.
	fill.Char.UnicodeChar = TEXT(' ');
	fill.Attributes = csbi.wAttributes;
	
	// Do the scroll
	ScrollConsoleScreenBuffer(hStdout, &scrollRect, NULL, scrollTarget, &fill);
	
	// Move the cursor to the top left corner too.
	csbi.dwCursorPosition.X = 0;
	csbi.dwCursorPosition.Y = 0;
	
	SetConsoleCursorPosition(hStdout, csbi.dwCursorPosition);
#endif
	return;
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si8	win_DATE_to_uutc_m11(sf8 date)
{
	sf8	secs, uutc;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// DATE == days since 29 Dec 1899 00:00:00 UTC
	secs = (date * (sf8) 86400.0) - (sf8) 2209161600.0;
	uutc = (si8) round(secs * (sf8) 1e6);
	
	return(uutc);
}



TERN_m11	win_initialize_terminal_m11(void)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
#ifdef  WINDOWS_m11
	HANDLE	hOut;
	DWORD	dwOriginalOutMode, dwRequestedOutModes, dwOutMode;
	
	
	// Set output mode to handle virtual terminal sequences
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE)
		return(FALSE_m11);

	dwOriginalOutMode = 0;
	if (!GetConsoleMode(hOut, &dwOriginalOutMode))
		return(FALSE_m11);

	dwRequestedOutModes = ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;

	dwOutMode = dwOriginalOutMode | dwRequestedOutModes;
	if (!SetConsoleMode(hOut, dwOutMode)) {  // failed to set both modes, try to step down mode gracefully.
	    dwRequestedOutModes = ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	    dwOutMode = dwOriginalOutMode | dwRequestedOutModes;
	    if (!SetConsoleMode(hOut, dwOutMode))  // Failed to set any VT mode, can't do anything here.
		    return(FALSE_m11);
	}
#endif
	return(TRUE_m11);
}


si4    win_ls_1d_to_tmp_m11(si1 **dir_strs, si4 n_dirs, TERN_m11 full_path)  // replacement for unix "ls -1d > temp_file (on a directory list)"
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
#ifdef WINDOWS_m11
	si1			*file_name, *dir_name, enclosing_directory[FULL_FILE_NAME_BYTES_m11], tmp_dir[FULL_FILE_NAME_BYTES_m11];
	ui4			fe;
	si4			i, n_files;
	WIN32_FIND_DATAA 	ffd;
	HANDLE 		        find_h;
	FILE			*fp;
	
	
	// returns number of files or -1 for error
	// dir_strs can include "*" & "?" regex characters
	
	if (dir_strs == NULL)
		return(-1);
	if (dir_strs[0] == NULL)
		return(-1);
	if (n_dirs < 1)
		return(-1);
	
	// open temp file
	fp = fopen_m11(globals_m11->temp_file, "w", __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	
	find_h = INVALID_HANDLE_VALUE;
	n_files = 0;
	for (i = 0; i < n_dirs; ++i) {
		dir_name = dir_strs[i];
		if (STR_contains_regex_m11(dir_name) == FALSE_m11) {
			fe = file_exists_m11(dir_name);
			// a plain directory name will not list it's contents => must append "\*"
			if (fe == DIR_EXISTS_m11) {
				sprintf(tmp_dir, "%s\\*", dir_name);
				dir_name = tmp_dir;
			} else if (fe == DOES_NOT_EXIST_m11) {
				continue;
			}
			// regular files will list
		}
		find_h = FindFirstFileA((LPCSTR) dir_name, &ffd);
		if (find_h == INVALID_HANDLE_VALUE)
			continue;
		if (full_path == TRUE_m11)
			extract_path_parts_m11(dir_name, enclosing_directory, NULL, NULL);
		do {
			file_name = ffd.cFileName;
			// exclude files/directories starting with "$"
			if (*file_name == '$')
				continue;
			// exclude ".", "..", & files/directories starting with "._"
			// invisible files (".<file_name>") are not excluded
			if (*file_name == '.') {
				if (file_name[1] == 0 || file_name[1] == '.' || file_name[1] == '_')
					continue;
			}
			++n_files;
			if (full_path == TRUE_m11)
				fprintf_m11(fp, "%s\\%s\n", enclosing_directory, file_name);
			else
				fprintf_m11(fp, "%s\n", file_name);
		} while (FindNextFileA(find_h, &ffd));
		
		FindClose(find_h);
	}
	fclose(fp);
		
	if (find_h == INVALID_HANDLE_VALUE && n_files == 0)
		return(-1);
	
	return(n_files);
#endif
	
	return(-1);
}


TERN_m11	win_reset_terminal_m11(void)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
#ifdef  WINDOWS_m11
	HANDLE	hOut;
	DWORD	dwOriginalOutMode;
	
	
	// Set output mode to handle virtual terminal sequences
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE)
		return(FALSE_m11);

	dwOriginalOutMode = 3;
	if (!SetConsoleMode(hOut, dwOriginalOutMode))
		return(FALSE_m11);
#endif
	return(TRUE_m11);
}


TERN_m11	win_socket_startup_m11(void)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
#ifdef WINDOWS_m11
	WORD		wVersionRequested;
	WSADATA		wsaData;
	si4		err;
	
	
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err) {
		error_message_m11("%s(): WSAStartup failed with error: %d\n", __FUNCTION__, err);
		return(FALSE_m11);
	}
	
	// Confirm that the WinSock DLL supports 2.2.
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		error_message_m11("%s(): Could not find a usable version of Winsock.dll\n", __FUNCTION__);
		WSACleanup();
		return(FALSE_m11);
	}
	
#endif
	return(TRUE_m11);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4	win_system_m11(si1 *command)  // Windows has a system() function which works fine, but it opens a command prompt window.
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
#ifdef WINDOWS_m11
	si1			*tmp_command;
	si1			*cmd_exe_path;
	si4			ret_val;
	si8			len;
	PROCESS_INFORMATION	process_info = {0};
	STARTUPINFOA		startup_info = {0};

	
	len = strlen(command);
	tmp_command = malloc(len + 4);
	tmp_command[0] = 0x2F;  // '/'
	tmp_command[1] = 0x63;  // 'c'
	tmp_command[2] = 0x20;  // <space>;
	memcpy(tmp_command + 3, command, len + 1);
	
	startup_info.cb = sizeof(STARTUPINFOA);
	cmd_exe_path = getenv("COMSPEC");
	_flushall();  // required for Windows system() calls, probably a good idea here too

	if (CreateProcessA(cmd_exe_path, tmp_command, NULL, NULL, 0, CREATE_NO_WINDOW, NULL, NULL, &startup_info, &process_info)) {
		WaitForSingleObject(process_info.hProcess, INFINITE);
		GetExitCodeProcess(process_info.hProcess, &ret_val);
		CloseHandle(process_info.hProcess);
		CloseHandle(process_info.hThread);
	}
	
	free((void *) tmp_command);
	
	return(ret_val);
#endif
	
#if defined MACOS_m11 || defined LINUX_m11
	return(-1);
#endif
}


#ifdef WINDOWS_m11
si8	win_time_to_uutc_m11(FILETIME win_time)
{
	si8	uutc, leftovers;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// A Windows time is the number of 100-nanosecond intervals since 12:00 AM January 1, 1601 UTC (excluding leap seconds).
	uutc = ((si8) win_time.dwHighDateTime << 32) + (si8) win_time.dwLowDateTime;
	leftovers = uutc % (si8) WIN_TICKS_PER_USEC_m11;
	leftovers = ((2 * leftovers) + WIN_TICKS_PER_USEC_m11) / (2 * WIN_TICKS_PER_USEC_m11);
	uutc /= (si8) WIN_TICKS_PER_USEC_m11;
	uutc -= (WIN_USECS_TO_EPOCH_m11 - leftovers);
	
	return(uutc);
}
#endif


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
sf8	win_uutc_to_DATE_m11(si8 uutc)
{
	sf8	secs, days;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// DATE == days since 29 Dec 1899 00:00:00 UTC
	secs = ((sf8) uutc / (sf8) 1e6) + (sf8) 2209161600.0;
	days = secs / (sf8) 86400.0;
	
	return(days);
}


void	windify_file_paths_m11(si1 *target, si1 *source)
{
	TERN_m11	match_made = FALSE_m11;
	si1		*c1, *c2;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// if target == source, or target == NULL, conversion done in place
	if (source == NULL)
		return;
	if (target == NULL)
		target = source;
	else if (target != source)
		strcpy(target, source);

	// Replace all '/' in string except if preceded by "http or "HTTP", including those not part of a path (not common & not the end of the world)
	// Note: "<white space>\" == "C:\" so don't need to handle that separately

	// try with "http"
	c1 = target;
	while ((c2 = STR_match_start_m11("http", c1)) != NULL) {
		*c2 = 0;
		STR_replace_char_m11('/', '\\', c1);
		*c2 = 'h';
		while (*c2 && *c2 != ' ')
			++c2;
		c1 = c2;
		match_made = TRUE_m11;
	}
	if (match_made == TRUE_m11) {
		STR_replace_char_m11('/', '\\', c1);
		return;
	}
	
	// try with "HTTP"
	while ((c2 = STR_match_start_m11("HTTP", c1)) != NULL) {
		*c2 = 0;
		STR_replace_char_m11('/', '\\', c1);
		*c2 = 'H';
		while (*c2 && *c2 != ' ')
			++c2;
		c1 = c2;
	}
	STR_replace_char_m11('/', '\\', c1);

	return;
}


si1	*windify_format_string_m11(si1 *fmt)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
#ifdef WINDOWS_m11
	// changes ld, li, lo, lu, lx, lX to "ll" versions of the same
	si1	*c, *new_c, *new_fmt;
	si4	matches;
	si8	len;
	
	
	if (fmt == NULL)
		return(NULL);
	
	matches = 0;
	c = fmt;
	while (*c) {
		if (*c++ == '%') {
			// skip over numbers & periods
			while ((*c >= '0' && *c <= '9') || *c == '.')
				++c;
			if (*c == 'l') {
				switch (*++c) {
					case 'd':
					case 'i':
					case 'o':
					case 'u':
					case 'x':
					case 'X':
						++matches;
						break;
				}
			}
		}
	}
	if (!matches)
		return(fmt);
	
	len = (si8) (c - fmt) + matches + 1;  // extra byte for terminal zero
	new_fmt = (si1 *) calloc((size_t) len, sizeof(ui1));
	
	c = fmt;
	new_c = new_fmt;
	while (*c) {
		if (*c == '%') {
			*new_c++ = *c++;
			// copy numbers & periods
			while ((*c >= '0' && *c <= '9') || *c == '.')
				*new_c++ = *c++;
			if (*c == 'l') {
				*new_c++ = *c++;
				switch (*c) {
					case 'd':
					case 'i':
					case 'o':
					case 'u':
					case 'x':
					case 'X':
						*new_c++ = 'l';
						break;
				}
			}
		}
		*new_c++ = *c++;
	}
		
	return(new_fmt);
#endif
	return(fmt);
}


//***********************************************************************//
//**************************  AES-128 FUNCTIONS  ************************//
//***********************************************************************//


// ATTRIBUTION
//
// Advanced Encryption Standard implementation in C.
// By Niyaz PK
// E-mail: niyazlife@gmail.com
// Downloaded from Website: www.hoozi.com
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
// Minor modifications have been made for compatibility with the MED Library.


// This function adds the round key to state.
// The round key is added to the state by an XOR function.
void	AES_add_round_key_m11(si4 round, ui1 state[][4], ui1 *round_key)
{
	si4	i, j;
	
	
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			state[j][i] ^= round_key[(round * AES_NB_m11 * 4) + (i * AES_NB_m11) + j];
	
	return;
}


// "in" is buffer to be encrypted (16 bytes)
// "out" is encrypted buffer (16 bytes)
// "in" can equal "out", i.e. can be done in place
// pass in expanded key externally - this is more efficient than passing the password
// if encrypting multiple times with the same encryption key
void	AES_decrypt_m11(ui1 *in, ui1 *out, si1 *password, ui1 *expanded_key)
{
	si1	key[16] = {0};
	ui1	state[4][4]; // the array that holds the intermediate results during encryption
	ui1	round_key[240]; // The array that stores the round keys
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (globals_m11->AES_sbox_table == NULL)  // all tables initialized together
		AES_initialize_tables_m11();
	
	if (expanded_key != NULL) {
		AES_inv_cipher_m11(in, out, state, expanded_key);
	} else if (password != NULL) {
		// password becomes the key (16 bytes, zero-padded if shorter, truncated if longer)
		strncpy_m11(key, password, 16);
		
		//The Key-Expansion routine must be called before the decryption routine.
		AES_key_expansion_m11(round_key, key);
		
		// The next function call decrypts the CipherText with the Key using AES algorithm.
		AES_inv_cipher_m11(in, out, state, round_key);
	} else {
		error_message_m11("%s(): No password or expanded key\n", __FUNCTION__);
	}
	
	return;
}


// This function produces AES_NB * (AES_NR + 1) round keys. The round keys are used in each round to encrypt the states.
// NOTE: make sure any terminal unused bytes in key array (password) are zeroed
void	AES_key_expansion_m11(ui1 *expanded_key, si1 *key)
{
	si4	i, j;
	ui1	temp[4], k;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// The round constant word array, Rcon[i], contains the values given by
	// x to the power (i - 1) being powers of x (x is denoted as {02}) in the field GF(28)
	// Note that i starts at 1, not 0).

	if (globals_m11->AES_rcon_table == NULL)
		if (AES_initialize_tables_m11() == FALSE_m11) {
			error_message_m11("%s(): error\n", __FUNCTION__);
			return;
		}
	
	// The first round key is the key itself.
	for (i = j = 0; i < AES_NK_m11; i++, j += 4) {
		expanded_key[j] = key[j];
		expanded_key[j + 1] = key[j + 1];
		expanded_key[j + 2] = key[j + 2];
		expanded_key[j + 3] = key[j + 3];
	}
	
	// All other round keys are found from the previous round keys.
	while (i < (AES_NB_m11 * (AES_NR_m11 + 1))) {
		
		for (j = 0; j < 4; j++) {
			temp[j] = expanded_key[(i - 1) * 4 + j];
		}
		
		if (i % AES_NK_m11 == 0) {
			// This rotates the 4 bytes in a word to the left once.
			// [a0,a1,a2,a3] becomes [a1,a2,a3,a0]
			k = temp[0];
			temp[0] = temp[1];
			temp[1] = temp[2];
			temp[2] = temp[3];
			temp[3] = k;
			
			// This takes a four-byte input word and applies the S-box
			// to each of the four bytes to produce an output word.
			temp[0] = (ui1)AES_get_sbox_value_m11(temp[0]);
			temp[1] = (ui1)AES_get_sbox_value_m11(temp[1]);
			temp[2] = (ui1)AES_get_sbox_value_m11(temp[2]);
			temp[3] = (ui1)AES_get_sbox_value_m11(temp[3]);
			
			temp[0] = temp[0] ^ (ui1)globals_m11->AES_rcon_table[i / AES_NK_m11];
		}
		else if (AES_NK_m11 > 6 && i % AES_NK_m11 == 4) {
			// This takes a four-byte input word and applies the S-box
			// to each of the four bytes to produce an output word.
			temp[0] = (ui1)AES_get_sbox_value_m11(temp[0]);
			temp[1] = (ui1)AES_get_sbox_value_m11(temp[1]);
			temp[2] = (ui1)AES_get_sbox_value_m11(temp[2]);
			temp[3] = (ui1)AES_get_sbox_value_m11(temp[3]);
		}
		
		expanded_key[i * 4] = expanded_key[(i - AES_NK_m11) * 4] ^ temp[0];
		expanded_key[i * 4 + 1] = expanded_key[(i - AES_NK_m11) * 4 + 1] ^ temp[1];
		expanded_key[i * 4 + 2] = expanded_key[(i - AES_NK_m11) * 4 + 2] ^ temp[2];
		expanded_key[i * 4 + 3] = expanded_key[(i - AES_NK_m11) * 4 + 3] ^ temp[3];
		
		i++;
	}
	
	return;
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4	AES_get_sbox_invert_m11(si4 num)
{
	if (globals_m11->AES_rsbox_table == NULL) {
		if (AES_initialize_tables_m11() == FALSE_m11) {
			error_message_m11("%s(): error\n", __FUNCTION__);
			return(-1);
		}
	}
	
	return(globals_m11->AES_rsbox_table[num]);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4	AES_get_sbox_value_m11(si4 num)
{
	if (globals_m11->AES_sbox_table == NULL)
		if (AES_initialize_tables_m11() == FALSE_m11) {
			error_message_m11("%s(): error\n", __FUNCTION__);
			return(-1);
		}
	
	return(globals_m11->AES_sbox_table[num]);
}


TERN_m11	AES_initialize_tables_m11(void)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif

	pthread_mutex_lock_m11(&globals_m11->AES_mutex);

	// rcon table
	if (globals_m11->AES_rcon_table == NULL) {
		globals_m11->AES_rcon_table = (si4*) calloc_m11((size_t)AES_RCON_ENTRIES_m11, sizeof(si4), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
		{
			si4 temp[AES_RCON_ENTRIES_m11] = AES_RCON_m11;
			memcpy(globals_m11->AES_rcon_table, temp, AES_RCON_ENTRIES_m11 * sizeof(si4));
		}
	}
	
	// rsbox table
	if (globals_m11->AES_rsbox_table == NULL) {
		globals_m11->AES_rsbox_table = (si4*) calloc_m11((size_t)AES_RSBOX_ENTRIES_m11, sizeof(si4), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
		{
			si4 temp[AES_RSBOX_ENTRIES_m11] = AES_RSBOX_m11;
			memcpy(globals_m11->AES_rsbox_table, temp, AES_RSBOX_ENTRIES_m11 * sizeof(si4));
		}
	}
	
	// sbox table
	if (globals_m11->AES_sbox_table == NULL) {
		globals_m11->AES_sbox_table = (si4*) calloc_m11((size_t)AES_SBOX_ENTRIES_m11, sizeof(si4), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
		{
			si4 temp[AES_SBOX_ENTRIES_m11] = AES_SBOX_m11;
			memcpy(globals_m11->AES_sbox_table, temp, AES_SBOX_ENTRIES_m11 * sizeof(si4));
		}
	}
	
	pthread_mutex_unlock_m11(&globals_m11->AES_mutex);

	return(TRUE_m11);
}


// Inv Cipher is the main decryption function
void	AES_inv_cipher_m11(ui1 *in, ui1 *out, ui1 state[][4], ui1 *round_key)
{
	si4	i, j, round = 0;
	
	// Copy the input encrypted text to state array.
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			state[j][i] = in[i * 4 + j];
		}
	}
	
	// Add the First round key to the state before starting the rounds.
	AES_add_round_key_m11(AES_NR_m11, state, round_key);
	
	// There will be AES_NR rounds.
	// The first AES_NR - 1 rounds are identical.
	// These AES_NR - 1 rounds are executed in the loop below.
	for (round = AES_NR_m11 - 1; round > 0; round--) {
		AES_inv_shift_rows_m11(state);
		AES_inv_sub_bytes_m11(state);
		AES_add_round_key_m11(round, state, round_key);
		AES_inv_mix_columns_m11(state);
	}
	
	// The last round is given below.
	// The MixColumns function is not here in the last round.
	AES_inv_shift_rows_m11(state);
	AES_inv_sub_bytes_m11(state);
	AES_add_round_key_m11(0, state, round_key);
	
	// The decryption process is over.
	// Copy the state array to output array.
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			out[i * 4 + j] = state[j][i];
		}
	}
	
	return;
}


// The method used to multiply may be difficult to understand.
// Please use the references to gain more information.
void	AES_inv_mix_columns_m11(ui1 state[][4])
{
	si4	i;
	ui1	a, b, c, d;
	

	for (i = 0; i < 4; i++) {
		a = state[0][i];
		b = state[1][i];
		c = state[2][i];
		d = state[3][i];
		state[0][i] = AES_MULTIPLY_m11(a, 0x0e) ^ AES_MULTIPLY_m11(b, 0x0b) ^ AES_MULTIPLY_m11(c, 0x0d) ^ AES_MULTIPLY_m11(d, 0x09);
		state[1][i] = AES_MULTIPLY_m11(a, 0x09) ^ AES_MULTIPLY_m11(b, 0x0e) ^ AES_MULTIPLY_m11(c, 0x0b) ^ AES_MULTIPLY_m11(d, 0x0d);
		state[2][i] = AES_MULTIPLY_m11(a, 0x0d) ^ AES_MULTIPLY_m11(b, 0x09) ^ AES_MULTIPLY_m11(c, 0x0e) ^ AES_MULTIPLY_m11(d, 0x0b);
		state[3][i] = AES_MULTIPLY_m11(a, 0x0b) ^ AES_MULTIPLY_m11(b, 0x0d) ^ AES_MULTIPLY_m11(c, 0x09) ^ AES_MULTIPLY_m11(d, 0x0e);
	}
	
	return;
}


void	AES_inv_shift_rows_m11(ui1 state[][4])
{
	ui1	temp;
	
	
	// Rotate first row 1 columns to right
	temp = state[1][3];
	state[1][3] = state[1][2];
	state[1][2] = state[1][1];
	state[1][1] = state[1][0];
	state[1][0] = temp;
	
	// Rotate second row 2 columns to right
	temp = state[2][0];
	state[2][0] = state[2][2];
	state[2][2] = temp;
	
	temp = state[2][1];
	state[2][1] = state[2][3];
	state[2][3] = temp;
	
	// Rotate third row 3 columns to right
	temp = state[3][0];
	state[3][0] = state[3][1];
	state[3][1] = state[3][2];
	state[3][2] = state[3][3];
	state[3][3] = temp;
	
	return;
}


void	AES_inv_sub_bytes_m11(ui1 state[][4])
{
	si4	i, j;
	
	
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			state[i][j] = (ui1) AES_get_sbox_invert_m11(state[i][j]);
		}
	}
	
	return;
}



//***********************************************************************//
//*****************************  AT FUNCTIONS  **************************//
//***********************************************************************//


void	AT_add_entry_m11(void *address, const si1 *function)
{
	ui8		bytes;
	si8		i, prev_node_count;
	AT_NODE		*atn;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (address == NULL) {
		#ifdef AT_DEBUG_m11
		warning_message_m11("%s(): attempting to add NULL object, called from function %s()\n", __FUNCTION__, function);
		#endif
		return;
	}
	
	// get mutex
	AT_mutex_on();
	
	// check if address exists in list and was previously free
	#ifdef AT_DEBUG_m11
	atn = globals_m11->AT_nodes;
	for (i = globals_m11->AT_node_count; i--; ++atn)
		if (atn->address == address)
			break;
	if (i >= 0) {
		if (atn->free_function != NULL) {
			// replace existing entry (keeps addresses in list unique)
			atn->alloc_function = function;
			atn->free_function = NULL;
			#ifdef MACOS_m11
			atn->bytes = (ui8) malloc_size(address);
			#endif
			#ifdef LINUX_m11
			atn->bytes = (ui8) malloc_usable_size(address);
			#endif
			#ifdef WINDOWS_m11
			atn->bytes = (ui8) _msize(address);
			#endif
			AT_mutex_off();
			return;
		} else {
			AT_mutex_off();
			warning_message_m11("%s(): address is already allocated, called from function %s()\n", __FUNCTION__, function);
			AT_show_entry_m11(address);
			return;
		}
	}
	#endif
	
	// expand list if needed
	if (globals_m11->AT_used_node_count == globals_m11->AT_node_count) {
		prev_node_count = globals_m11->AT_node_count;
		globals_m11->AT_node_count += GLOBALS_AT_LIST_SIZE_INCREMENT_m11;
		globals_m11->AT_nodes = (AT_NODE *) realloc((void *) globals_m11->AT_nodes, globals_m11->AT_node_count * sizeof(AT_NODE));
		if (globals_m11->AT_nodes == NULL) {
			AT_mutex_off();
			error_message_m11("%s(): error expanding AT list => exiting\n", __FUNCTION__);
			exit_m11(-1);
		}
		// zero new memory
		memset((void *) (globals_m11->AT_nodes + prev_node_count), 0, (size_t) GLOBALS_AT_LIST_SIZE_INCREMENT_m11 * sizeof(AT_NODE));
		atn = globals_m11->AT_nodes + prev_node_count;
	} else {
		// find a free node
		#ifdef AT_DEBUG_m11
		atn = globals_m11->AT_nodes + globals_m11->AT_used_node_count;
		#else
		atn = globals_m11->AT_nodes;
		for (i = globals_m11->AT_node_count; i--; ++atn)
			if (atn->address == NULL)
				break;
		#endif
	}
	
	// get true allocated bytes
#ifdef MACOS_m11
	bytes = (ui8) malloc_size(address);
#endif
#ifdef LINUX_m11
	bytes = (ui8) malloc_usable_size(address);
#endif
#ifdef WINDOWS_m11
	bytes = (ui8) _msize(address);
#endif
			
	// fill in
	++globals_m11->AT_used_node_count;
	atn->address = address;
	atn->bytes = bytes;
#ifdef AT_DEBUG_m11
	atn->alloc_function = function;
#endif
	
	// return mutex
	AT_mutex_off();
	
	return;
}


ui8	AT_alloc_size_m11(void *address)
{
	si8		i;
	ui8		bytes;
	AT_NODE		*atn;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (address == NULL) {
		#ifdef AT_DEBUG_m11
		warning_message_m11("%s(): attempting find a NULL object\n", __FUNCTION__);
		#endif
		return(0);
	}
	
	AT_mutex_on();

	atn = globals_m11->AT_nodes;
	for (i = globals_m11->AT_node_count; i--; ++atn) {
		if (atn->address == address) {
			bytes = atn->bytes;
			AT_mutex_off();
			return(bytes);
		}
	}
	
	#ifdef AT_DEBUG_m11
	message_m11("%s(): no entry for address %lu\n", __FUNCTION__, (ui8) address);
	#endif
	AT_mutex_off();

	return(0);
}


void	AT_free_all_m11(void)
{
	si8		i;
	AT_NODE		*atn;
	
#ifdef AT_DEBUG_m11
	si8		alloced_entries = 0;
#endif

#ifdef FN_DEBUG_m11
	#ifdef MATLAB_m11
	mexPrintf("%s()\n", __FUNCTION__);
	#else
	printf("%s()\n", __FUNCTION__);
	#endif
#endif
		
	AT_mutex_on();

	atn = globals_m11->AT_nodes;
	for (i = globals_m11->AT_node_count; i--; ++atn) {
		if (atn->address == NULL)
			continue;
		#ifdef AT_DEBUG_m11
		if (atn->free_function == NULL) {
			++alloced_entries;
			atn->free_function = __FUNCTION__;
			AT_mutex_off();  // release mutex for AT_show_entry_m11()
			AT_show_entry_m11(atn->address);
			AT_mutex_on();  // reclaim mutex
			#ifdef MATLAB_PERSISTENT_m11
			mxFree(atn->address);
			#else
			free(atn->address);
			#endif
		}
		#else
			#ifdef MATLAB_PERSISTENT_m11
			mxFree(atn->address);
			#else
			free(atn->address);
			#endif
			atn->address = NULL;
		#endif
	}

#ifdef AT_DEBUG_m11
	if (alloced_entries) {
		#ifdef MATLAB_m11
		mexPrintf("%s(): freed %ld AT entries:\n", __FUNCTION__, alloced_entries);
		#else
		printf("%s(): freed %ld AT entries:\n", __FUNCTION__, alloced_entries);
		#endif
	}
#else
	globals_m11->AT_used_node_count = 0;
#endif

	AT_mutex_off();

	return;
}


TERN_m11	AT_freeable_m11(void *address)
{
	si8		i;
	AT_NODE		*atn;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// silent function - just to tell whether an address is in the AT list
	
	if (address == NULL)
		return(FALSE_m11);
	
	// get mutex
	AT_mutex_on();
	
	// look for match entry
	atn = globals_m11->AT_nodes;
	for (i = globals_m11->AT_node_count; i--; ++atn)
		if (atn->address == address)
			break;

	// no entry
	if (i < 0) {
		AT_mutex_off();
		return(FALSE_m11);
	}

	// already freed
	#ifdef AT_DEBUG_m11
	if (atn->free_function != NULL) {
		AT_mutex_off();
		return(FALSE_m11);
	}
	#endif

	// return mutex
	AT_mutex_off();
	
	return(TRUE_m11);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void	AT_mutex_off(void)
{
	pthread_mutex_unlock_m11(&globals_m11->AT_mutex);

	return;
}



#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void	AT_mutex_on(void)
{
	pthread_mutex_lock_m11(&globals_m11->AT_mutex);

	return;
}


TERN_m11	AT_remove_entry_m11(void *address, const si1 *function)
{
	si8		i;
	AT_NODE		*atn;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	
	// Note this function does not free the accociated memory, just removes it from AT list
	
	if (address == NULL) {
		#ifdef AT_DEBUG_m11
		warning_message_m11("%s(): attempting to free NULL object, called from function %s()\n", __FUNCTION__, function);
		#endif
		return(FALSE_m11);
	}

	// get mutex
	AT_mutex_on();
	
	// look for match entry
	atn = globals_m11->AT_nodes;
	for (i = globals_m11->AT_node_count; i--; ++atn)
		if (atn->address == address)
			break;

	// no entry
	if (i < 0) {
		AT_mutex_off();
		#ifdef AT_DEBUG_m11
		warning_message_m11("%s(): address %lu is not allocated, called from function %s()\n", __FUNCTION__, (ui8) address, function);
		#endif
		return(FALSE_m11);
	}

	// already freed
	#ifdef AT_DEBUG_m11
	if (atn->free_function != NULL) {
		AT_mutex_off();
		warning_message_m11("%s(): address was already freed, called from function %s():", __FUNCTION__, function);
		AT_show_entry_m11(address);
		return(FALSE_m11);
	}
	#endif

	// remove
	#ifdef AT_DEBUG_m11
	atn->free_function = function;
	#else
	--globals_m11->AT_used_node_count;
	atn->address = NULL;
	#endif

	// return mutex
	AT_mutex_off();
	
	return(TRUE_m11);
}


void	AT_show_entries_m11(ui4	entry_type)
{
	si8		i;
	AT_NODE		*atn;
	#ifdef AT_DEBUG_m11
	si8		alloced_entries = 0;
	si8		freed_entries = 0;
	#endif

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	AT_mutex_on();
	
	atn = globals_m11->AT_nodes;
	for (i = globals_m11->AT_node_count; i--; ++atn) {
		if (atn->address == NULL)
			continue;
		#ifdef AT_DEBUG_m11
		if (atn->free_function == NULL) {
			if (entry_type & AT_CURRENTLY_ALLOCATED_m11) {
				message_m11("\naddress: %lu\n", (ui8) atn->address);
				message_m11("bytes: %lu\n", atn->bytes);
				message_m11("allocated by: %s()\n", atn->alloc_function);
			}
			++alloced_entries;
		} else {
			if (entry_type & AT_PREVIOUSLY_FREED_m11) {
				message_m11("\naddress: %lu\n", (ui8) atn->address);
				message_m11("bytes: %lu\n", atn->bytes);
				message_m11("allocated by: %s()\n", atn->alloc_function);
				message_m11("freed by: %s()\n", atn->free_function);
			}
			++freed_entries;
		}
		#else
		message_m11("\naddress: %lu\n", (ui8) atn->address);
		message_m11("bytes: %lu\n", atn->bytes);
		#endif
	}
#ifdef AT_DEBUG_m11
	message_m11("\ncurrently allocated AT entries: %lu\n", alloced_entries);
	message_m11("previously freed AT entries: %lu\n", freed_entries);
#else
	message_m11("\ncurrently allocated AT entries: %lu\n", globals_m11->AT_used_node_count);
#endif

	AT_mutex_off();

	return;
}


void	AT_show_entry_m11(void *address)
{
	si8		i;
	AT_NODE		*atn;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (address == NULL) {
		#ifdef AT_DEBUG_m11
		warning_message_m11("%s(): attempting to show a NULL object\n", __FUNCTION__);
		#endif
		return;
	}
	
	AT_mutex_on();

	atn = globals_m11->AT_nodes;
	for (i = globals_m11->AT_node_count; i--; ++atn) {
		if (atn->address == address) {
			message_m11("\naddress: %lu\n", (ui8) atn->address);
			message_m11("bytes: %lu\n", atn->bytes);
			#ifdef AT_DEBUG_m11
			message_m11("allocated by: %s()\n", atn->alloc_function);
			if (atn->free_function != NULL)
				message_m11("freed by: %s()\n", atn->free_function);
			#endif
			AT_mutex_off();
			return;
		}
	}
	message_m11("%s(): no entry for address %lu\n", __FUNCTION__, (ui8) address);
	
	AT_mutex_off();

	return;
}


TERN_m11	AT_update_entry_m11(void *orig_address, void *new_address, const si1 *function)
{
	si8		i;
	AT_NODE		*atn;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (orig_address == NULL) {
		if (new_address != NULL) {
			AT_add_entry_m11(new_address, function);
			return(TRUE_m11);
		}
	}
	if (new_address == NULL) {
		#ifdef AT_DEBUG_m11
		warning_message_m11("%s(): attempting to reassign to NULL object, called from function %s()\n", __FUNCTION__, function);
		#endif
		return(FALSE_m11);
	}
	
	// get mutex
	AT_mutex_on();

	// look for match entry
	atn = globals_m11->AT_nodes;
	for (i = globals_m11->AT_node_count; i--; ++atn)
		if (atn->address == orig_address)
			break;
	
	// no entry
	if (i < 0) {
		AT_mutex_off();
		#ifdef AT_DEBUG_m11
		warning_message_m11("%s(): address %lu is not in the list, called from function %s()\n", __FUNCTION__, (ui8) orig_address, function);
		#endif
		return(FALSE_m11);
	}
	
	#ifdef AT_DEBUG_m11
	if (atn->free_function != NULL) {
		warning_message_m11("%s(): original address was already freed, called from function %s():", __FUNCTION__, function);
		AT_show_entry_m11(orig_address);
		warning_message_m11("=> replacing with new data\n");
		atn->free_function = NULL;
	}
	#endif

	// update
	atn->address = new_address;
#ifdef MACOS_m11
	atn->bytes = (ui8) malloc_size(new_address);
#endif
#ifdef LINUX_m11
	atn->bytes = (ui8) malloc_usable_size(new_address);
#endif
#ifdef WINDOWS_m11
	atn->bytes = (ui8) _msize(new_address);
#endif

#ifdef AT_DEBUG_m11
	atn->alloc_function = function;
#endif

	// return mutex
	AT_mutex_off();

	return(TRUE_m11);
}



//***********************************************************************//
//****************************  CMP FUNCTIONS  **************************//
//***********************************************************************//


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
CMP_BUFFERS_m11    *CMP_allocate_buffers_m11(CMP_BUFFERS_m11 *buffers, si8 n_buffers, si8 n_elements, si8 element_size, TERN_m11 zero_data, TERN_m11 lock_memory)
{
	ui1	*array_base;
	ui8	i, pointer_bytes, array_bytes, total_requested_bytes, mod;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// all buffers are 8-byte aligned
	// also use this function to re-allocate (data not preserved)
	// cast buffer pointers to desired type (as long as element_size <= allocated type size)
	// e.g.  sf8_array = (sf8 *) buffer[0]; si4_array = (si4 *) buffer[1];
	
	if (buffers == NULL)
		buffers = (CMP_BUFFERS_m11 *) calloc_m11((size_t) 1, sizeof(CMP_BUFFERS_m11), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	else if (buffers->n_buffers >= n_buffers && buffers->n_elements >= n_elements && buffers->element_size >= element_size)
		return(buffers);
	
	// buffer pointers
	pointer_bytes = (ui8) (n_buffers * sizeof(void *));
	if ((mod = (pointer_bytes & (ui8) 7)))
		pointer_bytes += ((ui8) 8 - mod);
	
	// array bytes (pass sizeof(x) for element size so any pad bytes of structures are included)
	array_bytes = (ui8) (n_elements * element_size);
	if ((mod = (array_bytes & (ui8) 7)))
		array_bytes += ((ui8) 8 - mod);
	
	// allocate
	total_requested_bytes = pointer_bytes + (n_buffers * array_bytes);
	if (total_requested_bytes > buffers->total_allocated_bytes) {
		if (buffers->buffer != NULL) {
			if (buffers->locked == TRUE_m11)
				buffers->locked = munlock_m11((void *) buffers->buffer, (size_t) buffers->total_allocated_bytes, __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
			free_m11((void *) buffers->buffer, __FUNCTION__);
		}
		if (zero_data == TRUE_m11)
			buffers->buffer = (void **) calloc_m11((size_t) total_requested_bytes, sizeof(ui1), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
		else
			buffers->buffer = (void **) malloc_m11((size_t) total_requested_bytes, __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
		buffers->total_allocated_bytes = total_requested_bytes;
	} else if (zero_data == TRUE_m11) {
		memset((void *) buffers->buffer, 0, (size_t) total_requested_bytes);
	}
	buffers->n_buffers = n_buffers;
	buffers->n_elements = n_elements;
	buffers->element_size = element_size;
	
	// assign pointers
	array_base = (ui1 *) buffers->buffer + pointer_bytes;
	for (i = 0; i < n_buffers; ++i) {
		buffers->buffer[i] = (void *) array_base;
		array_base += array_bytes;
	}
	
	// lock
	buffers->locked = FALSE_m11;
	if (lock_memory == TRUE_m11)
		buffers->locked = mlock_m11((void *) buffers->buffer, (size_t) buffers->total_allocated_bytes, FALSE_m11, __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	
	return(buffers);
}


CMP_PROCESSING_STRUCT_m11	*CMP_allocate_processing_struct_m11(FILE_PROCESSING_STRUCT_m11 *fps, ui4 mode, si8 data_samples, si8 compressed_data_bytes, si8 keysample_bytes, ui4 block_samples, CMP_DIRECTIVES_m11 *directives, CMP_PARAMETERS_m11 *parameters)
{
	TERN_m11	need_compressed_data = FALSE_m11;
	TERN_m11	need_decompressed_data = FALSE_m11;
	TERN_m11	need_original_data = FALSE_m11;
	TERN_m11	need_keysample_buffer = FALSE_m11;
	TERN_m11	need_detrended_buffer = FALSE_m11;
	TERN_m11	need_derivative_buffer = FALSE_m11;
	TERN_m11	need_scrap_buffer = FALSE_m11;
	TERN_m11	need_scaled_amplitude_buffer = FALSE_m11;
	TERN_m11	need_scaled_frequency_buffer = FALSE_m11;
	TERN_m11	need_VDS_buffers = FALSE_m11;
	si8		pad_samples;
	CMP_PROCESSING_STRUCT_m11	*cps;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// pass CMP_SELF_MANAGED_MEMORY_m11 for data_samples to prevent automatic re-allocation

	if (fps->universal_header->type_code != TIME_SERIES_DATA_FILE_TYPE_CODE_m11) {
		error_message_m11("%s(): FPS must be time series data\n", __FUNCTION__);
		return(NULL);
	}
	
	if (fps->parameters.cps == NULL)
		fps->parameters.cps = (CMP_PROCESSING_STRUCT_m11 *) calloc_m11((size_t) 1, sizeof(CMP_PROCESSING_STRUCT_m11), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	cps = fps->parameters.cps;
	pthread_mutex_init_m11(&cps->parameters.mutex,  NULL);
	
	// set up directives
	if (directives != NULL)
		cps->directives = *directives;
	else // set defaults
		CMP_initialize_directives_m11(&cps->directives, (ui1) mode);
	if (fps->parent != NULL) {  // set level header directives
		if (((LEVEL_HEADER_m11 *) fps->parent)->flags & LH_NO_CPS_PTR_RESET_m11)
			cps->directives.cps_pointer_reset = FALSE_m11;  // default is TRUE_m11
		if (((LEVEL_HEADER_m11 *) fps->parent)->flags & LH_NO_CPS_CACHING_m11)
			cps->directives.cps_caching = FALSE_m11;  // default is TRUE_m11
	}
	
	// set up parameters
	if (parameters != NULL)
		cps->parameters = *parameters;
	else  // set defaults
		CMP_initialize_parameters_m11(&cps->parameters);
		
	if (mode == CMP_COMPRESSION_MODE_NO_ENTRY_m11) {
		warning_message_m11("%s(): No compression mode specified\n", __FUNCTION__);
		return(cps);
	}
	
	// allocate RED/PRED buffers
	if (cps->directives.algorithm == CMP_RED_COMPRESSION_m11) {  // VDS uses RED & PRED, but allocated for PRED
		if (cps->directives.mode == CMP_COMPRESSION_MODE_m11) {
			cps->parameters.count = calloc_m11(CMP_RED_MAX_STATS_BINS_m11, sizeof(ui4), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
			cps->parameters.sorted_count = calloc_m11(CMP_RED_MAX_STATS_BINS_m11, sizeof(CMP_STATISTICS_BIN_m11), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
			cps->parameters.symbol_map = calloc_m11(CMP_RED_MAX_STATS_BINS_m11, sizeof(ui1), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
			need_derivative_buffer = TRUE_m11;
		} else {
			cps->parameters.count = NULL;
			cps->parameters.sorted_count = NULL;
			cps->parameters.symbol_map = NULL;
		}
		cps->parameters.cumulative_count = calloc_m11(CMP_RED_MAX_STATS_BINS_m11 + 1, sizeof(ui8), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
		cps->parameters.minimum_range = calloc_m11(CMP_RED_MAX_STATS_BINS_m11, sizeof(ui8), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	} else if (cps->directives.algorithm == CMP_PRED_COMPRESSION_m11 || cps->directives.algorithm == CMP_VDS_COMPRESSION_m11) {  // VDS uses RED & PRED, but buffers allocated for PRED
		if (cps->directives.mode == CMP_COMPRESSION_MODE_m11) {
			cps->parameters.count = (void *) calloc_2D_m11((size_t) CMP_PRED_CATS_m11, CMP_RED_MAX_STATS_BINS_m11, sizeof(ui4), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
			cps->parameters.sorted_count = (void *) calloc_2D_m11((size_t) CMP_PRED_CATS_m11, CMP_RED_MAX_STATS_BINS_m11, sizeof(CMP_STATISTICS_BIN_m11), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
			cps->parameters.symbol_map = (void *) calloc_2D_m11((size_t) CMP_PRED_CATS_m11, CMP_RED_MAX_STATS_BINS_m11, sizeof(ui1), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
			need_derivative_buffer = TRUE_m11;
		} else {
			cps->parameters.count = NULL;
			cps->parameters.sorted_count = NULL;
			cps->parameters.symbol_map = NULL;
		}
		cps->parameters.cumulative_count = (void *) calloc_2D_m11((size_t) CMP_PRED_CATS_m11, CMP_RED_MAX_STATS_BINS_m11 + 1, sizeof(ui8), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
		cps->parameters.minimum_range = (void *) calloc_2D_m11((size_t) CMP_PRED_CATS_m11, CMP_RED_MAX_STATS_BINS_m11, sizeof(ui8), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	} else {
		cps->parameters.count = NULL;
		cps->parameters.sorted_count = NULL;
		cps->parameters.symbol_map = NULL;
		cps->parameters.cumulative_count = NULL;
		cps->parameters.minimum_range = NULL;
	}
	
	// VDS
	if (cps->directives.algorithm == CMP_VDS_COMPRESSION_m11)
		need_VDS_buffers = TRUE_m11;
	
	// decompression
	if (cps->directives.mode == CMP_DECOMPRESSION_MODE_m11) {
		need_compressed_data = TRUE_m11;
		need_decompressed_data = TRUE_m11;
		need_keysample_buffer = TRUE_m11;
	} else {  // compression
		need_compressed_data = TRUE_m11;
		need_original_data = TRUE_m11;
		need_keysample_buffer = TRUE_m11;
		
		if (cps->directives.detrend_data == TRUE_m11)
			need_detrended_buffer = TRUE_m11;
		if (cps->directives.find_derivative_level == TRUE_m11)
			need_scrap_buffer = TRUE_m11;
		if (cps->directives.set_amplitude_scale == TRUE_m11 || cps->directives.find_amplitude_scale == TRUE_m11)
			need_scaled_amplitude_buffer = TRUE_m11;
		if (cps->directives.set_frequency_scale == TRUE_m11 || cps->directives.find_frequency_scale == TRUE_m11)
			need_scaled_frequency_buffer = TRUE_m11;
		if (cps->directives.find_amplitude_scale == TRUE_m11 || cps->directives.find_frequency_scale == TRUE_m11)
			need_decompressed_data = TRUE_m11;
	}
	
	// original_data - caller specified array size
	if (need_original_data == TRUE_m11 && data_samples > 0)
		cps->input_buffer = cps->original_ptr = cps->original_data = (si4 *) calloc_m11((size_t) data_samples, sizeof(si4), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	else
		cps->input_buffer = cps->original_ptr = cps->original_data = NULL;
	
	// allocated_block_samples applies to any array whose size depends only on number of block samples
	cps->parameters.allocated_block_samples = block_samples;
	
	// compressed_data - caller specified array size
	if (need_compressed_data == TRUE_m11) {
		if (fps->parameters.raw_data_bytes < (compressed_data_bytes + UNIVERSAL_HEADER_BYTES_m11)) {
			FPS_reallocate_processing_struct_m11(fps, compressed_data_bytes + UNIVERSAL_HEADER_BYTES_m11);
		} else {
			cps->parameters.allocated_compressed_bytes = fps->parameters.raw_data_bytes - UNIVERSAL_HEADER_BYTES_m11;
			cps->block_header = (CMP_BLOCK_FIXED_HEADER_m11 *) fps->data_pointers;
		}
	} else {
		cps->parameters.allocated_compressed_bytes = 0;
	}
	
	// keysample_buffer - caller specified or maximum bytes required for specified block size
	if (keysample_bytes == 0)
		keysample_bytes = CMP_MAX_KEYSAMPLE_BYTES_m11(block_samples);
	if (need_keysample_buffer == TRUE_m11) {
		cps->parameters.keysample_buffer = (si1 *) calloc_m11((size_t) keysample_bytes, sizeof(ui1), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
		cps->parameters.allocated_keysample_bytes = keysample_bytes;
	} else {
		cps->parameters.keysample_buffer = NULL;
		cps->parameters.allocated_keysample_bytes = 0;
	}

	// decompressed_data - caller specified array size
	if (need_decompressed_data == TRUE_m11) {
		if (cps->directives.mode == CMP_DECOMPRESSION_MODE_m11) {
			if (data_samples > 0) {
				cps->parameters.cache = cps->decompressed_data = cps->decompressed_ptr = (si4 *) calloc_m11((size_t) data_samples, sizeof(si4), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
			} else {
				cps->parameters.cache = cps->decompressed_data = cps->decompressed_ptr = NULL;
			}
			cps->parameters.allocated_decompressed_samples = data_samples;
		} else { // cps->directives.mode == CMP_COMPRESSION_MODE_m11  (decompressed_ptr used to calculate mean residual ratio for each block)
			cps->parameters.cache = cps->decompressed_data = cps->decompressed_ptr = (si4 *) calloc_m11((size_t) block_samples, sizeof(si4), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
			cps->parameters.allocated_decompressed_samples = block_samples;
		}
	} else {
		cps->parameters.cache = cps->decompressed_data = cps->decompressed_ptr = NULL;
		cps->parameters.allocated_decompressed_samples = 0;
	}
	
	// detrended_buffer - maximum bytes required for caller specified block size
	if (need_detrended_buffer == TRUE_m11)
		cps->parameters.detrended_buffer = (si4 *) calloc_m11((size_t) block_samples, sizeof(si4), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	else
		cps->parameters.detrended_buffer = NULL;
	
	// derivative_buffer - maximum bytes required for caller specified block size
	if (need_derivative_buffer == TRUE_m11) {
		cps->parameters.derivative_buffer = (si4 *) malloc_m11((size_t) (block_samples << 2), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	} else {
		cps->parameters.derivative_buffer = NULL;
	}
	
	// scrap_buffers: here maximum bytes required for caller specified block size (other routines may expand)
	if (need_scrap_buffer == TRUE_m11) {
		cps->parameters.scrap_buffers = CMP_allocate_buffers_m11(NULL, 1, block_samples, sizeof(si4), FALSE_m11, FALSE_m11);
	} else {
		cps->parameters.scrap_buffers = NULL;
	}

	// scaled_amplitude_buffer - maximum bytes required for caller specified block size
	if (need_scaled_amplitude_buffer == TRUE_m11)
		cps->parameters.scaled_amplitude_buffer = (si4 *) calloc_m11((size_t) block_samples, sizeof(si4), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	else
		cps->parameters.scaled_amplitude_buffer = NULL;
	
	// scaled_frequency_buffer - maximum bytes required for caller specified block size
	if (need_scaled_frequency_buffer == TRUE_m11)
		cps->parameters.scaled_frequency_buffer = (si4 *) calloc_m11((size_t) block_samples, sizeof(si4), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	else
		cps->parameters.scaled_frequency_buffer = NULL;
	
	// VDS buffers - maximum bytes required for caller specified block size
	if (need_VDS_buffers == TRUE_m11) {
		if (mode == CMP_COMPRESSION_MODE_m11)
			pad_samples = CMP_VDS_LOWPASS_ORDER_m11 * 6;
		else
			pad_samples = CMP_MAK_PAD_SAMPLES_m11;
		cps->parameters.VDS_input_buffers = CMP_allocate_buffers_m11(NULL, CMP_VDS_INPUT_BUFFERS_m11, (si8) block_samples + pad_samples, sizeof(sf8), FALSE_m11, FALSE_m11);
		cps->parameters.VDS_output_buffers = CMP_allocate_buffers_m11(NULL, CMP_VDS_OUTPUT_BUFFERS_m11, (si8) block_samples, sizeof(sf8), FALSE_m11, FALSE_m11);
	} else {
		cps->parameters.VDS_input_buffers = cps->parameters.VDS_output_buffers = NULL;
	}

	return(cps);
}


TERN_m11     CMP_check_CPS_allocation_m11(FILE_PROCESSING_STRUCT_m11 *fps)
{
	TERN_m11	ret_val = TRUE_m11;
	si1		need_compressed_data = FALSE_m11;
	si1		need_decompressed_data = FALSE_m11;
	si1		need_original_data = FALSE_m11;
	si1		need_detrended_buffer = FALSE_m11;
	si1		need_derivative_buffer = FALSE_m11;
	si1		need_scaled_amplitude_buffer = FALSE_m11;
	si1		need_scaled_frequency_buffer = FALSE_m11;
	si1		need_keysample_buffer = FALSE_m11;
	si1		need_VDS_buffers = FALSE_m11;
	CMP_PROCESSING_STRUCT_m11	*cps;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (fps->universal_header->type_code != TIME_SERIES_DATA_FILE_TYPE_CODE_m11) {
		error_message_m11("%s(): FPS must be time series data\n", __FUNCTION__);
		return(FALSE_m11);
	}
	
	cps = fps->parameters.cps;
	if (cps == NULL) {
		warning_message_m11("%s(): cps is not allocated\n", __FUNCTION__);
		return(FALSE_m11);
	}
	
	if (cps->directives.algorithm == CMP_VDS_COMPRESSION_m11)
		need_VDS_buffers = TRUE_m11;

	// decompression
	if (cps->directives.mode == CMP_DECOMPRESSION_MODE_m11) {
		need_compressed_data = TRUE_m11;
		need_decompressed_data = TRUE_m11;
		need_keysample_buffer = TRUE_m11;
	}
	
	// compression
	else {
		need_compressed_data = TRUE_m11;
		need_original_data = TRUE_m11;
		need_keysample_buffer = TRUE_m11;
		
		if (cps->directives.detrend_data == TRUE_m11)
			need_detrended_buffer = TRUE_m11;
		if (cps->directives.set_derivative_level == TRUE_m11 || cps->directives.find_derivative_level == TRUE_m11)
			need_derivative_buffer = TRUE_m11;
		if (cps->directives.set_amplitude_scale == TRUE_m11 || cps->directives.find_amplitude_scale == TRUE_m11)
			need_scaled_amplitude_buffer = TRUE_m11;
		if (cps->directives.set_frequency_scale == TRUE_m11 || cps->directives.find_frequency_scale == TRUE_m11)
			need_scaled_frequency_buffer = TRUE_m11;
		if (cps->directives.find_amplitude_scale == TRUE_m11 || cps->directives.find_frequency_scale == TRUE_m11)
			need_decompressed_data = TRUE_m11;
	}
	
	// check compressed_data
	if (need_compressed_data == TRUE_m11 && fps->time_series_data == NULL) {
		error_message_m11("%s(): \"compressed_data\" is not allocated in the FILE_PROCESSING_STRUCT\n", __FUNCTION__);
		ret_val = FALSE_m11;
	}
	
	// check keysample_buffer
	if (need_keysample_buffer == TRUE_m11 && cps->parameters.keysample_buffer == NULL) {
		error_message_m11("%s(): \"keysample_buffer\" is not allocated in the CMP_PROCESSING_STRUCT\n", __FUNCTION__);
		ret_val = FALSE_m11;
	}
	
	// check original_data
	if (need_original_data == TRUE_m11 && cps->original_data == NULL) {
		error_message_m11("%s(): \"original_data\" is not allocated in the CMP_PROCESSING_STRUCT\n", __FUNCTION__);
		ret_val = FALSE_m11;
	}
	if (need_original_data == FALSE_m11 && cps->original_data != NULL) {
		warning_message_m11("%s(): \"original_data\" is needlessly allocated in the CMP_PROCESSING_STRUCT => freeing\n", __FUNCTION__);
		free_m11((void *) cps->original_data, __FUNCTION__);
		cps->original_ptr = cps->original_data = NULL;
		ret_val = FALSE_m11;
	}
	
	// check decompressed_data
	if (need_decompressed_data == TRUE_m11 && cps->decompressed_data == NULL) {
		error_message_m11("%s(): \"decompressed_data\" is not allocated in the CMP_PROCESSING_STRUCT\n", __FUNCTION__);
		ret_val = FALSE_m11;
	}
	if (need_decompressed_data == FALSE_m11 && cps->decompressed_data != NULL) {
		warning_message_m11("%s(): \"decompressed_data\" is needlessly allocated in the CMP_PROCESSING_STRUCT => freeing\n", __FUNCTION__);
		free_m11((void *) cps->decompressed_data, __FUNCTION__);
		cps->decompressed_ptr = cps->decompressed_data = NULL;
		ret_val = FALSE_m11;
	}
	
	// check detrended_buffer
	if (need_detrended_buffer == TRUE_m11 && cps->parameters.detrended_buffer == NULL) {
		error_message_m11("%s(): \"detrended_buffer\" is not allocated in the CMP_PROCESSING_STRUCT\n", __FUNCTION__);
		ret_val = FALSE_m11;
	}
	if (need_detrended_buffer == FALSE_m11 && cps->parameters.detrended_buffer != NULL) {
		warning_message_m11("%s(): \"detrended_buffer\" is needlessly allocated in the CMP_PROCESSING_STRUCT => freeing\n", __FUNCTION__);
		free_m11((void *) cps->parameters.detrended_buffer, __FUNCTION__);
		cps->parameters.detrended_buffer = NULL;
		ret_val = FALSE_m11;
	}
	
	// check derivative_buffer
	if (need_derivative_buffer == TRUE_m11 && cps->parameters.derivative_buffer == NULL) {
		error_message_m11("%s(): \"derivative_buffer\" is not allocated in the CMP_PROCESSING_STRUCT\n", __FUNCTION__);
		ret_val = FALSE_m11;
	}
	if (need_derivative_buffer == FALSE_m11 && cps->parameters.derivative_buffer != NULL) {
		warning_message_m11("%s(): \"derivative_buffer\" is needlessly allocated in the CMP_PROCESSING_STRUCT => freeing\n", __FUNCTION__);
		free_m11((void *) cps->parameters.derivative_buffer, __FUNCTION__);
		cps->parameters.derivative_buffer = NULL;
		ret_val = FALSE_m11;
	}
	
	// check scaled_amplitude_buffer
	if (need_scaled_amplitude_buffer == TRUE_m11 && cps->parameters.scaled_amplitude_buffer == NULL) {
		error_message_m11("%s(): \"scaled_amplitude_buffer\" is not allocated in the CMP_PROCESSING_STRUCT\n", __FUNCTION__);
		ret_val = FALSE_m11;
	}
	if (need_scaled_amplitude_buffer == FALSE_m11 && cps->parameters.scaled_amplitude_buffer != NULL) {
		warning_message_m11("%s(): \"scaled_amplitude_buffer\" is needlessly allocated in the CMP_PROCESSING_STRUCT => freeing\n", __FUNCTION__);
		free_m11((void *) cps->parameters.scaled_amplitude_buffer, __FUNCTION__);
		cps->parameters.scaled_amplitude_buffer = NULL;
		ret_val = FALSE_m11;
	}
	
	// check scaled_frequency_buffer
	if (need_scaled_frequency_buffer == TRUE_m11 && cps->parameters.scaled_frequency_buffer == NULL) {
		error_message_m11("%s(): \"scaled_frequency_buffer\" is not allocated in the CMP_PROCESSING_STRUCT\n", __FUNCTION__);
		ret_val = FALSE_m11;
	}
	if (need_scaled_frequency_buffer == FALSE_m11 && cps->parameters.scaled_frequency_buffer != NULL) {
		warning_message_m11("%s(): \"scaled_frequency_buffer\" is needlessly allocated in the CMP_PROCESSING_STRUCT => freeing\n", __FUNCTION__);
		free_m11((void *) cps->parameters.scaled_frequency_buffer, __FUNCTION__);
		cps->parameters.scaled_frequency_buffer = NULL;
		ret_val = FALSE_m11;
	}
	
	// check VDS buffers
	if (need_VDS_buffers == TRUE_m11 && (cps->parameters.VDS_input_buffers == NULL || cps->parameters.VDS_output_buffers == NULL)) {
		error_message_m11("%s(): \"VDS_buffers\" are not allocated in the CMP_PROCESSING_STRUCT\n", __FUNCTION__);
		ret_val = FALSE_m11;
	}
	if (need_VDS_buffers == FALSE_m11 && (cps->parameters.VDS_input_buffers != NULL || cps->parameters.VDS_output_buffers != NULL)) {
		warning_message_m11("%s(): \"VDS_buffers\" are needlessly allocated in the CMP_PROCESSING_STRUCT => freeing\n", __FUNCTION__);
		CMP_free_buffers_m11(cps->parameters.VDS_input_buffers, TRUE_m11);
		cps->parameters.VDS_input_buffers = NULL;
		CMP_free_buffers_m11(cps->parameters.VDS_output_buffers, TRUE_m11);
		cps->parameters.VDS_output_buffers = NULL;
		ret_val = FALSE_m11;
	}
	return(ret_val);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void	CMP_cps_mutex_off_m11(CMP_PROCESSING_STRUCT_m11 *cps)
{
	pthread_mutex_unlock_m11(&cps->parameters.mutex);
	
	return;
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void	CMP_cps_mutex_on_m11(CMP_PROCESSING_STRUCT_m11 *cps)
{
	pthread_mutex_lock_m11(&cps->parameters.mutex);

	return;
}


void    CMP_decode_m11(FILE_PROCESSING_STRUCT_m11 *fps)
{
	ui4				offset;
	si4				*si4_p;
	sf4				*sf4_p;
	sf8				intercept, gradient, amplitude_scale, frequency_scale;
	CMP_BLOCK_FIXED_HEADER_m11	*block_header;
	CMP_PROCESSING_STRUCT_m11	*cps;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (fps->universal_header->type_code != TIME_SERIES_DATA_FILE_TYPE_CODE_m11) {
		error_message_m11("%s(): FPS must be time series data\n", __FUNCTION__);
		return;
	}
	
	cps = fps->parameters.cps;
	block_header = cps->block_header;
	if (cps->parameters.allocated_block_samples < block_header->number_of_samples) {
		if (CMP_reallocate_processing_struct_m11(fps, CMP_DECOMPRESSION_MODE_m11, (si8) block_header->number_of_samples, block_header->number_of_samples) == NULL) {
			error_message_m11("%s(): reallocation error\n", __FUNCTION__);
			return;
		}
		block_header = cps->block_header;
	}

	// decrypt (probably done in read_file_m11(), but if not, do here)
	if (block_header->block_flags & CMP_BF_ENCRYPTION_MASK_m11)
		CMP_decrypt_m11(fps);
	
	// discontinuity
	if (block_header->block_flags & CMP_BF_DISCONTINUITY_MASK_m11)
		cps->parameters.discontinuity = TRUE_m11;
	else
		cps->parameters.discontinuity = FALSE_m11;
	
	// get variable region
	CMP_get_variable_region_m11(cps);
	
	// decompress
	switch (block_header->block_flags & CMP_BF_ALGORITHMS_MASK_m11) {
		case CMP_BF_RED_ENCODING_MASK_m11:
			cps->directives.algorithm = CMP_RED_COMPRESSION_m11;
			CMP_RED_decode_m11(cps);
			break;
		case CMP_BF_PRED_ENCODING_MASK_m11:
			cps->directives.algorithm = CMP_PRED_COMPRESSION_m11;
			CMP_PRED_decode_m11(cps);
			break;
		case CMP_BF_MBE_ENCODING_MASK_m11:
			cps->directives.algorithm = CMP_MBE_COMPRESSION_m11;
			CMP_MBE_decode_m11(cps);
			break;
		case CMP_BF_VDS_ENCODING_MASK_m11:
			cps->directives.algorithm = CMP_VDS_COMPRESSION_m11;
			CMP_VDS_decode_m11(cps);
			break;
		default:
			error_message_m11("%s(): unrecognized compression algorithm (%u)\n", __FUNCTION__, block_header->block_flags & CMP_BF_ALGORITHMS_MASK_m11);
			return;
	}

	if (cps->directives.algorithm != CMP_VDS_COMPRESSION_m11) {
		// unscale frequency-scaled decompressed_data if scaled (in place)
		// no blockwise frequency scaling in VDS encoded data
		if (block_header->parameter_flags & CMP_PF_FREQUENCY_SCALE_MASK_m11) {
			sf4_p = (sf4 *) cps->block_parameters;
			offset = cps->parameters.block_parameter_map[CMP_PF_FREQUENCY_SCALE_IDX_m11];
			frequency_scale = (sf8) *(sf4_p + offset);
			CMP_unscale_frequency_si4_m11(cps->decompressed_ptr, cps->decompressed_ptr, (si8) block_header->number_of_samples, frequency_scale);
		}
		
		// unscale amplitude-scaled decompressed_data if scaled (in place)
		// VDS_decode_m11() does amplitude scaling itself
		if (block_header->parameter_flags & CMP_PF_AMPLITUDE_SCALE_MASK_m11) {
			sf4_p = (sf4 *) cps->block_parameters;
			offset = cps->parameters.block_parameter_map[CMP_PF_AMPLITUDE_SCALE_IDX_m11];
			amplitude_scale = (sf8) *(sf4_p + offset);
			CMP_unscale_amplitude_si4_m11(cps->decompressed_ptr, cps->decompressed_ptr, (si8) block_header->number_of_samples, amplitude_scale);
		}
		
		// add trend to decompressed_data if detrended (in place)
		// VDS_decode_m11() does retrending itself
		if (CMP_IS_DETRENDED_m11(block_header)) {
			sf4_p = (sf4 *) cps->block_parameters;
			offset = cps->parameters.block_parameter_map[CMP_PF_GRADIENT_IDX_m11];
			gradient = (sf8) *(sf4_p + offset);
			si4_p = (si4 *) cps->block_parameters;
			offset = cps->parameters.block_parameter_map[CMP_PF_INTERCEPT_IDX_m11];
			intercept = (sf8) *(si4_p + offset);
			CMP_retrend_si4_m11(cps->decompressed_ptr, cps->decompressed_ptr, block_header->number_of_samples, gradient, intercept);
		}
	}
	
	// restrict returned samples
	cps->parameters.block_end_index = 0xFFFFFFFF;  // reset
	if (cps->parameters.block_start_index) {
		if (cps->directives.algorithm != CMP_VDS_COMPRESSION_m11 || cps->directives.cps_caching == TRUE_m11)
			cps->decompressed_data = cps->parameters.cache + cps->parameters.block_start_index;
		cps->parameters.block_start_index = 0;  // reset
	}

	return;
}


TERN_m11	CMP_decrypt_m11(FILE_PROCESSING_STRUCT_m11 *fps)
{
	ui1				*ui1_p, *key;
	si4				encryption_blocks, encryptable_blocks;
	si8				i, encryption_bytes;
	CMP_BLOCK_FIXED_HEADER_m11	*block_header;
	PASSWORD_DATA_m11		*pwd;
	CMP_PROCESSING_STRUCT_m11	*cps;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (fps->universal_header->type_code != TIME_SERIES_DATA_FILE_TYPE_CODE_m11) {
		error_message_m11("%s(): FPS must be time series data\n", __FUNCTION__);
		return(FALSE_m11);
	}
	cps = fps->parameters.cps;
	block_header = cps->block_header;

	// check if block is encrypted (already checked in CMP_decode() - just check here in case function being used independently)
	if (!(block_header->block_flags & CMP_BF_ENCRYPTION_MASK_m11))
		return(TRUE_m11);

	// get decryption key
	pwd = fps->parameters.password_data;
	if (block_header->block_flags & CMP_BF_LEVEL_1_ENCRYPTION_MASK_m11) {
		if (block_header->block_flags & CMP_BF_LEVEL_2_ENCRYPTION_MASK_m11) {
			error_message_m11("%s(): Cannot decrypt data: flags indicate both level 1 & level 2 encryption\n", __FUNCTION__);
			return(FALSE_m11);
		}
		if (pwd->access_level >= LEVEL_1_ENCRYPTION_m11) {
			key = pwd->level_1_encryption_key;
		} else {
			error_message_m11("%s(): Cannot decrypt data: insufficient access\n", __FUNCTION__);
			return(FALSE_m11);
		}
	} else {  // level 2 bit is set
		if (pwd->access_level == LEVEL_2_ENCRYPTION_m11) {
			key = pwd->level_2_encryption_key;
		} else {
			error_message_m11("%s(): Cannot decrypt data: insufficient access\n", __FUNCTION__);
			return(FALSE_m11);
		}
	}
	
	// calculated encryption blocks
	encryptable_blocks = (block_header->total_block_bytes - CMP_BLOCK_ENCRYPTION_START_OFFSET_m11) / ENCRYPTION_BLOCK_BYTES_m11;
	if (block_header->block_flags | CMP_BF_MBE_ENCODING_MASK_m11) {
		encryption_blocks = encryptable_blocks;
	} else {
		encryption_bytes = block_header->total_header_bytes - CMP_BLOCK_ENCRYPTION_START_OFFSET_m11 + ENCRYPTION_BLOCK_BYTES_m11;
		encryption_blocks = (si4)(((encryption_bytes - 1) / ENCRYPTION_BLOCK_BYTES_m11) + 1);
		if (encryptable_blocks < encryption_blocks)
			encryption_blocks = encryptable_blocks;
	}
	
	// decrypt
	ui1_p = (ui1 *) block_header + CMP_BLOCK_ENCRYPTION_START_OFFSET_m11;
	for (i = 0; i < encryption_blocks; ++i) {
		AES_decrypt_m11(ui1_p, ui1_p, NULL, key);
		ui1_p += ENCRYPTION_BLOCK_BYTES_m11;
	}
	
	// set block flags to decrypted
	block_header->block_flags &= ~CMP_BF_ENCRYPTION_MASK_m11;
	
	return(TRUE_m11);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void    CMP_free_buffers_m11(CMP_BUFFERS_m11 *buffers, TERN_m11 free_structure)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (buffers == NULL)
		return;
	
	if (buffers->locked == TRUE_m11) {
#if defined MACOS_m11 || defined LINUX_m11
		munlock((void *) buffers->buffer, (size_t) buffers->total_allocated_bytes);
#endif
#ifdef WINDOWS_m11
		VirtualUnlock((void *) buffers->buffer, (size_t) buffers->total_allocated_bytes);
#endif
	}
	free_m11((void *) buffers->buffer, __FUNCTION__);
	
	if (free_structure == TRUE_m11) {
		free_m11((void *) buffers, __FUNCTION__);
	} else {
		buffers->n_buffers = buffers->n_elements = buffers->element_size = 0;
		buffers->locked = FALSE_m11;
		buffers->buffer = NULL;
	}
	
	return;
}


void    CMP_free_processing_struct_m11(CMP_PROCESSING_STRUCT_m11 *cps, TERN_m11 free_cps_structure)
{
	CMP_DIRECTIVES_m11	saved_directives;
	CMP_PARAMETERS_m11	saved_parameters;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (cps == NULL) {
		warning_message_m11("%s(): trying to free a NULL CMP_PROCESSING_STRUCT_m11 => returning with no action\n", __FUNCTION__);
		return;
	}
	
	pthread_mutex_lock_m11(&cps->parameters.mutex);

	if (cps->original_data != NULL)
		free_m11((void *) cps->original_data, __FUNCTION__);
	
	if (cps->parameters.cache != NULL)  // decompressed_data is pointer into this
		free_m11((void *) cps->parameters.cache, __FUNCTION__);
	
	if (cps->parameters.keysample_buffer != NULL)
		free_m11((void *) cps->parameters.keysample_buffer, __FUNCTION__);
	
	if (cps->parameters.detrended_buffer != NULL)
		free_m11((void *) cps->parameters.detrended_buffer, __FUNCTION__);
	
	if (cps->parameters.scaled_amplitude_buffer != NULL)
		free_m11((void *) cps->parameters.scaled_amplitude_buffer, __FUNCTION__);
	
	if (cps->parameters.scaled_frequency_buffer != NULL)
		free_m11((void *) cps->parameters.scaled_frequency_buffer, __FUNCTION__);
	
	if (cps->parameters.scrap_buffers != NULL)
		CMP_free_buffers_m11(cps->parameters.scrap_buffers, TRUE_m11);
	
	if (cps->parameters.count != NULL)
		free_m11((void *) cps->parameters.count, __FUNCTION__);
	
	if (cps->parameters.cumulative_count != NULL)
		free_m11((void *) cps->parameters.cumulative_count, __FUNCTION__);
	
	if (cps->parameters.sorted_count != NULL)
		free_m11((void *) cps->parameters.sorted_count, __FUNCTION__);
	
	if (cps->parameters.minimum_range != NULL)
		free_m11((void *) cps->parameters.minimum_range, __FUNCTION__);
	
	if (cps->parameters.symbol_map != NULL)
		free_m11((void *) cps->parameters.symbol_map, __FUNCTION__);
	
	if (cps->parameters.VDS_input_buffers != NULL)
		CMP_free_buffers_m11(cps->parameters.VDS_input_buffers, TRUE_m11);
	if (cps->parameters.VDS_output_buffers != NULL)
		CMP_free_buffers_m11(cps->parameters.VDS_output_buffers, TRUE_m11);
	
	pthread_mutex_unlock_m11(&cps->parameters.mutex);
	pthread_mutex_destroy_m11(&cps->parameters.mutex);

	if (free_cps_structure == TRUE_m11) {
		free_m11((void *) cps, __FUNCTION__);
	} else {
		saved_directives = cps->directives;
		saved_parameters = cps->parameters;
		memset((void *) cps, 0, sizeof(CMP_PROCESSING_STRUCT_m11));
		cps->directives = saved_directives;
		cps->parameters = saved_parameters;
		cps->original_data = NULL;
		cps->parameters.cache = NULL;
		cps->decompressed_data = NULL;
		cps->parameters.keysample_buffer = NULL;
		cps->parameters.detrended_buffer = NULL;
		cps->parameters.scaled_amplitude_buffer = NULL;
		cps->parameters.scaled_frequency_buffer = NULL;
		cps->parameters.scrap_buffers = NULL;
		cps->parameters.count = NULL;
		cps->parameters.cumulative_count = NULL;
		cps->parameters.sorted_count = NULL;
		cps->parameters.minimum_range = NULL;
		cps->parameters.symbol_map = NULL;
		cps->parameters.VDS_input_buffers = NULL;
		cps->parameters.VDS_output_buffers = NULL;
	}


	return;
}


void	CMP_generate_parameter_map_m11(CMP_PROCESSING_STRUCT_m11 *cps)
{
	ui4				bit, flags, n_params, i, *p_map;
	CMP_BLOCK_FIXED_HEADER_m11	*bh;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// add up parameter bytes (4 bytes for each bit set)
	bh = cps->block_header;
	flags = bh->parameter_flags;
	p_map = cps->parameters.block_parameter_map;
	for (bit = 1, n_params = i = 0; i < CMP_PF_PARAMETER_FLAG_BITS_m11; ++i, bit <<= 1)
		if (flags & bit)
			p_map[i] = n_params++;
	
	cps->parameters.number_of_block_parameters = (si4) n_params;
	bh->parameter_region_bytes = (ui2) (n_params * 4);
	
	return;
}


ui1    CMP_get_overflow_bytes_m11(CMP_PROCESSING_STRUCT_m11 *cps, ui4 mode, ui4 algorithm)
{
	ui1					bits_per_samp;
	ui2					flags;
	si4					i, val, abs_min, abs_max;
	CMP_RED_MODEL_FIXED_HEADER_m11		*RED_header;
	CMP_PRED_MODEL_FIXED_HEADER_m11		*PRED_header;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (mode == CMP_COMPRESSION_MODE_m11) {  // assumes extrema are known & derivative level is set
		if (cps->directives.find_overflow_bytes == TRUE_m11) {
			if (cps->parameters.derivative_level) {
				abs_min = ABS_m11(cps->parameters.minimum_difference_value);
				abs_max = ABS_m11(cps->parameters.maximum_difference_value);
			} else {  // level zero => raw_data
				abs_min = ABS_m11(cps->parameters.minimum_sample_value);
				abs_max = ABS_m11(cps->parameters.maximum_sample_value);
			}
			val = (abs_min > abs_max) ? abs_min : abs_max;
			for (bits_per_samp = 1, i = val; i; i >>= 1)
				++bits_per_samp;
			if (algorithm == CMP_RED_COMPRESSION_m11) {
				RED_header = (CMP_RED_MODEL_FIXED_HEADER_m11 *) cps->parameters.model_region;
				if (RED_header->flags & CMP_RED_FLAGS_POSITIVE_DERIVATIVES_MASK_m11)
					--bits_per_samp;  // don't need a sign bit
			}
			cps->parameters.overflow_bytes = (bits_per_samp + 7) >> 3;
		} else if (cps->directives.set_overflow_bytes == TRUE_m11) {
			if (cps->parameters.goal_overflow_bytes != 2 && cps->parameters.goal_overflow_bytes != 3) {
				warning_message_m11("%s(): overflow bytes must be 2-4 => setting to 4\n", __FUNCTION__);
				cps->parameters.goal_overflow_bytes = CMP_PARAMETERS_OVERFLOW_BYTES_DEFAULT_m11;  // 4
				cps->parameters.overflow_bytes = CMP_PARAMETERS_OVERFLOW_BYTES_DEFAULT_m11;  // 4
			}
		} else {
			cps->parameters.goal_overflow_bytes = CMP_PARAMETERS_OVERFLOW_BYTES_DEFAULT_m11;  // 4
			cps->parameters.overflow_bytes = CMP_PARAMETERS_OVERFLOW_BYTES_DEFAULT_m11;  // 4
		}
		// set block flag
		if (algorithm == CMP_RED_COMPRESSION_m11) {
			RED_header->flags &= ~CMP_RED_OVERFLOW_BYTES_MASK_m11;
			if (cps->parameters.overflow_bytes == 2)
				RED_header->flags |= CMP_RED_2_BYTE_OVERFLOWS_MASK_m11;
			else if	(cps->parameters.overflow_bytes == 3)
				RED_header->flags |= CMP_RED_3_BYTE_OVERFLOWS_MASK_m11;
		} else if (algorithm == CMP_PRED_COMPRESSION_m11) {
			PRED_header = (CMP_PRED_MODEL_FIXED_HEADER_m11 *) cps->parameters.model_region;
			PRED_header->flags &= ~CMP_PRED_OVERFLOW_BYTES_MASK_m11;
			if (cps->parameters.overflow_bytes == 2)
				PRED_header->flags |= CMP_PRED_2_BYTE_OVERFLOWS_MASK_m11;
			else if	(cps->parameters.overflow_bytes == 3)
				PRED_header->flags |= CMP_PRED_3_BYTE_OVERFLOWS_MASK_m11;
		}
	} else {  // CMP_DECOMPRESSION_MODE_m11
		if (algorithm == CMP_RED_COMPRESSION_m11) {
			RED_header = (CMP_RED_MODEL_FIXED_HEADER_m11 *) cps->parameters.model_region;
			flags = RED_header->flags & CMP_RED_OVERFLOW_BYTES_MASK_m11;
			if (flags == CMP_RED_2_BYTE_OVERFLOWS_MASK_m11)
				cps->parameters.overflow_bytes =  2;
			else if (flags == CMP_RED_3_BYTE_OVERFLOWS_MASK_m11)
				cps->parameters.overflow_bytes =  3;
			else
				cps->parameters.overflow_bytes = CMP_PARAMETERS_OVERFLOW_BYTES_DEFAULT_m11;  // 4
		} else if (algorithm == CMP_PRED_COMPRESSION_m11) {
			PRED_header = (CMP_PRED_MODEL_FIXED_HEADER_m11 *) cps->parameters.model_region;
			flags = PRED_header->flags & CMP_PRED_OVERFLOW_BYTES_MASK_m11;
			if (flags == CMP_PRED_2_BYTE_OVERFLOWS_MASK_m11)
				cps->parameters.overflow_bytes =  2;
			else if (flags == CMP_PRED_3_BYTE_OVERFLOWS_MASK_m11)
				cps->parameters.overflow_bytes =  3;
			else
				cps->parameters.overflow_bytes = CMP_PARAMETERS_OVERFLOW_BYTES_DEFAULT_m11;  // 4
		}
	}
	
	return(cps->parameters.overflow_bytes);
}


void    CMP_get_variable_region_m11(CMP_PROCESSING_STRUCT_m11 *cps)
{
	ui1				*var_reg_ptr;
	CMP_BLOCK_FIXED_HEADER_m11	*block_header;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	block_header = cps->block_header;
	var_reg_ptr = (ui1 *) block_header + CMP_BLOCK_FIXED_HEADER_BYTES_m11;  // pointer to beginning of variable region
	
	// records region (user sets block_header->record_region_bytes)
	cps->block_records = var_reg_ptr;
	var_reg_ptr += block_header->record_region_bytes;
	
	// parameter region
	cps->block_parameters = (ui4 *) var_reg_ptr;
	CMP_generate_parameter_map_m11(cps);
	var_reg_ptr += block_header->parameter_region_bytes;
	
	// protected region
	// cps->protected_region = var_reg_ptr;  // no pointer to this in CPS
	var_reg_ptr += block_header->protected_region_bytes;
	
	// discretionary region (user sets block_header->discretionary_region_bytes)
	cps->discretionary_region = var_reg_ptr;
	var_reg_ptr += block_header->discretionary_region_bytes;
	
	// variable region bytes
	cps->parameters.variable_region_bytes = CMP_VARIABLE_REGION_BYTES_v1_m11(block_header);
	
	// model region (not part of variable region, but convenient to do this here)
	cps->parameters.model_region = var_reg_ptr;
	
	return;
}


void	CMP_initialize_directives_m11(CMP_DIRECTIVES_m11 *directives, ui1 mode)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	directives->mode = mode;
	directives->algorithm = CMP_DIRECTIVES_ALGORITHM_DEFAULT_m11;
	directives->encryption_level = CMP_DIRECTIVES_ENCRYPTION_LEVEL_DEFAULT_m11;
	directives->cps_pointer_reset = CMP_DIRECTIVES_CPS_POINTER_RESET_DEFAULT_m11;
	directives->cps_caching = CMP_DIRECTIVES_CPS_CACHING_DEFAULT_m11;
	directives->fall_through_to_best_encoding = CMP_DIRECTIVES_FALL_THROUGH_TO_BEST_ENCODING_DEFAULT_m11;
	directives->reset_discontinuity = CMP_DIRECTIVES_RESET_DISCONTINUITY_DEFAULT_m11;
	directives->include_noise_scores = CMP_DIRECTIVES_INCLUDE_NOISE_SCORES_DEFAULT_m11;
	directives->no_zero_counts = CMP_DIRECTIVES_NO_ZERO_COUNTS_DEFAULT_m11;
	directives->set_derivative_level = CMP_DIRECTIVES_SET_DERIVATIVE_LEVEL_DEFAULT_m11;
	directives->find_derivative_level = CMP_DIRECTIVES_FIND_DERIVATIVE_LEVEL_DEFAULT_m11;
	directives->set_overflow_bytes = CMP_DIRECTIVES_SET_OVERFLOW_BYTES_DEFAULT_m11;
	directives->find_overflow_bytes = CMP_DIRECTIVES_FIND_OVERFLOW_BYTES_DEFAULT_m11;
	directives->convert_to_native_units = CMP_DIRECTIVES_CONVERT_TO_NATIVE_UNITS_DEFAULT_m11;
	directives->detrend_data = CMP_DIRECTIVES_DETREND_DATA_DEFAULT_m11;
	directives->require_normality = CMP_DIRECTIVES_REQUIRE_NORMALITY_DEFAULT_m11;
	directives->use_compression_ratio = CMP_DIRECTIVES_USE_COMPRESSION_RATIO_DEFAULT_m11;
	directives->use_mean_residual_ratio = CMP_DIRECTIVES_USE_MEAN_RESIDUAL_RATIO_DEFAULT_m11;
	directives->use_relative_ratio = CMP_DIRECTIVES_USE_RELATIVE_RATIO_DEFAULT_m11;
	directives->set_amplitude_scale = CMP_DIRECTIVES_SET_AMPLITUDE_SCALE_DEFAULT_m11;
	directives->find_amplitude_scale = CMP_DIRECTIVES_FIND_AMPLITUDE_SCALE_DEFAULT_m11;
	directives->set_frequency_scale = CMP_DIRECTIVES_SET_FREQUENCY_SCALE_DEFAULT_m11;
	directives->find_frequency_scale = CMP_DIRECTIVES_FIND_FREQUENCY_SCALE_DEFAULT_m11;
	directives->set_overflow_bytes = CMP_DIRECTIVES_SET_OVERFLOW_BYTES_DEFAULT_m11;
	directives->find_overflow_bytes = CMP_DIRECTIVES_FIND_OVERFLOW_BYTES_DEFAULT_m11;
	directives->VDS_scale_by_baseline = CMP_DIRECTIVES_VDS_SCALE_BY_BASELINE_DEFAULT_m11;
	
	return;
}


void	CMP_initialize_parameters_m11(CMP_PARAMETERS_m11 *parameters)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	pthread_mutex_init_m11(&parameters->mutex, NULL);
	parameters->cache = NULL;
	parameters->cached_blocks = NULL;
	parameters->cached_block_cnt = 0;
	parameters->cached_block_list_len = 0;
	parameters->block_start_index = 0;
	parameters->block_end_index = 0xFFFFFFFF;
	parameters->allocated_block_samples = 0;
	parameters->allocated_keysample_bytes = 0;
	parameters->allocated_compressed_bytes = 0;
	parameters->allocated_decompressed_samples = 0;
	parameters->discontinuity = CMP_PARAMETERS_DISCONTINUITY_DEFAULT_m11;
	parameters->goal_derivative_level = CMP_PARAMETERS_DERIVATIVE_LEVEL_DEFAULT_m11;
	parameters->derivative_level = 0;
	parameters->goal_overflow_bytes = CMP_PARAMETERS_OVERFLOW_BYTES_DEFAULT_m11;
	parameters->overflow_bytes = 0;
	parameters->number_of_block_parameters = 0;
	parameters->minimum_sample_value = CMP_PARAMETERS_MINIMUM_SAMPLE_VALUE_DEFAULT_m11;
	parameters->maximum_sample_value = CMP_PARAMETERS_MAXIMUM_SAMPLE_VALUE_DEFAULT_m11;
	parameters->user_number_of_records = CMP_USER_NUMBER_OF_RECORDS_DEFAULT_m11;
	parameters->user_record_region_bytes = CMP_USER_RECORD_REGION_BYTES_DEFAULT_m11;
	parameters->user_parameter_flags = CMP_USER_PARAMETER_FLAGS_DEFAULT_m11;
	parameters->protected_region_bytes = CMP_PROTECTED_REGION_BYTES_DEFAULT_m11;
	parameters->user_discretionary_region_bytes = CMP_USER_DISCRETIONARY_REGION_BYTES_DEFAULT_m11;
	parameters->variable_region_bytes = 0;
	parameters->goal_ratio = CMP_PARAMETERS_GOAL_RATIO_DEFAULT_m11;
	parameters->goal_tolerance = CMP_PARAMETERS_GOAL_TOLERANCE_DEFAULT_m11;
	parameters->maximum_goal_attempts = CMP_PARAMETERS_MAXIMUM_GOAL_ATTEMPTS_DEFAULT_m11;
	parameters->minimum_normality = CMP_PARAMETERS_MINIMUM_NORMALITY_DEFAULT_m11;
	parameters->amplitude_scale = CMP_PARAMETERS_AMPLITUDE_SCALE_DEFAULT_m11;
	parameters->frequency_scale = CMP_PARAMETERS_FREQUENCY_SCALE_DEFAULT_m11;
	parameters->VDS_sampling_frequency = FREQUENCY_NO_ENTRY_m11;
	parameters->VDS_LFP_high_fc = FREQUENCY_NO_ENTRY_m11;
	parameters->VDS_threshold = CMP_PARAMETERS_VDS_THRESHOLD_DEFAULT_m11;

	parameters->count = NULL;
	parameters->sorted_count = NULL;
	parameters->cumulative_count = NULL;
	parameters->minimum_range = NULL;
	parameters->symbol_map = NULL;
	parameters->VDS_input_buffers = NULL;
	parameters->VDS_output_buffers = NULL;
	parameters->filtps = NULL;
	parameters->n_filtps = 0;

	return;
}


void	CMP_integrate_m11(CMP_PROCESSING_STRUCT_m11 *cps)
{
	ui1				deriv_level;
	ui4				n_samps;
	si4				*deriv_buffer, *si4_p1, *si4_p2;
	si8				i;
	CMP_BLOCK_FIXED_HEADER_m11	*block_header;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// integrates in place from/to decompressed_ptr
	deriv_level = cps->parameters.derivative_level;
	if (deriv_level == 0)
		return;

	block_header = cps->block_header;
	n_samps = block_header->number_of_samples;
	deriv_buffer = cps->decompressed_ptr;
	do {
		si4_p2 = deriv_buffer + deriv_level;
		si4_p1 = si4_p2 - 1;
		for (i = n_samps - deriv_level; i--;)
			*si4_p2++ += *si4_p1++;
	} while (--deriv_level);

	return;
}


sf8	*CMP_lin_interp_sf8_m11(sf8 *in_data, si8 in_len, sf8 *out_data, si8 out_len)
{
	sf8     x, inc, f_bot_x, bot_y, range;
	si8     i, bot_x, top_x, last_bot_x;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (out_data == NULL)
		out_data = (sf8 *) malloc_m11((size_t) (out_len << 3), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	
	if (in_len <= 1) {
		if (in_len == 0)
			return(NULL);
		for (i = 0; i < out_len; ++i)
			out_data[i] = in_data[0];
		return(out_data);
	}
	if (in_len == out_len) {
		memcpy((void *) out_data, (void *) in_data, (size_t) (in_len << 3));
		return(out_data);
	}
	
	// interpolate
	--in_len; --out_len;
	inc = (sf8) in_len / (sf8) out_len;
	
	out_data[0] = in_data[0];
	if (out_len <= (in_len << 1)) {  // downsample, or upsample ratio <= 2:1 (upsampling this way is faster than below)
		for (x = inc, i = 1; i < out_len; ++i, x += inc) {
			top_x = (bot_x = (si8) x) + 1;
			out_data[i] = (x - (sf8) bot_x) * (in_data[top_x] - in_data[bot_x]) + in_data[bot_x];
		}
	} else {  // upsample ratio > 2:1
		for (last_bot_x = -1, x = inc, i = 1; i < out_len; ++i, x += inc) {
			bot_x = (si8) x;
			if (bot_x != last_bot_x) {
				range = in_data[bot_x + 1] - in_data[bot_x];
				bot_y = in_data[bot_x];
				f_bot_x = (sf8) bot_x;
				last_bot_x = bot_x;
			}
			out_data[i] = ((x - f_bot_x) * range) + bot_y;
		}
	}
	out_data[out_len] = in_data[in_len];
	
	return(out_data);
}


si4	*CMP_lin_interp_si4_m11(si4 *in_data, si8 in_len, si4 *out_data, si8 out_len)
{
	sf8     x, inc, f_bot_x, bot_y, range;
	si8     i, bot_x, top_x, last_bot_x;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (out_data == NULL)
		out_data = (si4 *) malloc_m11((size_t) (out_len << 2), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	
	if (in_len <= 1) {
		if (in_len == 0)
			return(NULL);
		for (i = 0; i < out_len; ++i)
			out_data[i] = in_data[0];
		return(out_data);
	}
	if (in_len == out_len) {
		memcpy((void *) out_data, (void *) in_data, (size_t) (in_len << 2));
		return(out_data);
	}
	
	// interpolate
	--in_len; --out_len;
	inc = (sf8) in_len / (sf8) out_len;
	
	out_data[0] = in_data[0];
	if (out_len <= (in_len << 1)) {  // downsample, or upsample ratio <= 2:1
		for (x = inc, i = 1; i < out_len; ++i, x += inc) {
			top_x = (bot_x = (si8) x) + 1;
			out_data[i] = CMP_round_si4_m11((x - (sf8) bot_x) * ((sf8) in_data[top_x] - (sf8) in_data[bot_x]) + (sf8) in_data[bot_x]);
		}
	} else {  // upsample ratio > 2:1
		for (last_bot_x = -1, x = inc, i = 1; i < out_len; ++i, x += inc) {
			bot_x = (si8) x;
			if (bot_x != last_bot_x) {
				range = (sf8) in_data[bot_x + 1] - (sf8) in_data[bot_x];
				bot_y = (sf8) in_data[bot_x];
				f_bot_x = (sf8) bot_x;
				last_bot_x = bot_x;
			}
			out_data[i] = CMP_round_si4_m11(((x - f_bot_x) * range) + bot_y);
		}
	}
	out_data[out_len] = in_data[in_len];
	
	return(out_data);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void	CMP_lock_buffers_m11(CMP_BUFFERS_m11 *buffers)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// lock
	if (buffers->locked != TRUE_m11) {
		buffers->locked = mlock_m11((void *) buffers->buffer, buffers->total_allocated_bytes, FALSE_m11, __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
		buffers->locked = TRUE_m11;
	}

	return;
}


void    CMP_MBE_decode_m11(CMP_PROCESSING_STRUCT_m11 *cps)
{
	ui4				n_samps, total_header_bytes;
	si4				*si4_p, *init_val_p, bits_per_samp, n_derivs;
	si8				i, lmin;
	ui8				out_val, *in_word, mask, temp_mask, high_bits, in_word_val;
	ui1				in_bit;
	CMP_BLOCK_FIXED_HEADER_m11	*block_header;
	CMP_MBE_MODEL_FIXED_HEADER_m11	*MBE_header;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// read header
	block_header = cps->block_header;
	n_samps = block_header->number_of_samples;
	MBE_header = (CMP_MBE_MODEL_FIXED_HEADER_m11 *) cps->parameters.model_region;
	lmin = (si8) MBE_header->minimum_value;
	bits_per_samp = (si4) MBE_header->bits_per_sample;
	n_derivs = (si4) MBE_header->derivative_level;
	
	// copy initial derivative values to output buffer
	init_val_p = (si4 *) (cps->parameters.model_region + CMP_MBE_MODEL_FIXED_HEADER_BYTES_m11);
	for (i = 0; i < n_derivs; ++i)
		cps->decompressed_ptr[i] = *init_val_p++;
	
	// set parameters for return
	cps->parameters.derivative_level = n_derivs;

	// MBE decode
	// Note: can't use block_header->total_header_bytes in case input is VDS fall through
	total_header_bytes = (ui4) ((cps->parameters.model_region - (ui1 *) block_header) + CMP_MBE_MODEL_FIXED_HEADER_BYTES_m11 + (n_derivs * 4));
	in_word = (ui8 *) ((ui1 *) block_header + (total_header_bytes & ~((ui4) 7)));
	in_bit = (total_header_bytes & (ui4) 7) << 3;
	mask = ((ui8) 1 << bits_per_samp) - 1;
	si4_p = cps->decompressed_ptr + n_derivs;
	in_word_val = *in_word >> in_bit;
	for (i = n_samps - n_derivs; i--;) {
		out_val = in_word_val & mask;
		in_word_val >>= bits_per_samp;
		if ((in_bit += bits_per_samp) > 63) {
			in_word_val = *++in_word;
			if (in_bit &= 63) {
				temp_mask = ((ui8) 1 << in_bit) - 1;
				high_bits = in_word_val & temp_mask;
				out_val |= (high_bits << (bits_per_samp - in_bit));
				in_word_val >>= in_bit;
			}
		}
		*si4_p++ = (si4) ((si8) out_val + lmin);
	}
	
	// integrate derivatives
	CMP_integrate_m11(cps);

	return;
}


// Mofified Akima cubic interpolation
// Attribution: modifications based on Matlab's adjustments to weights of Akima function
// Note: input x's are integers, output x's are floats
sf8	*CMP_mak_interp_sf8_m11(CMP_BUFFERS_m11 *in_bufs, si8 in_len, CMP_BUFFERS_m11 *out_bufs, si8 out_len)
{
	si8		*index, *si8_p1, *si8_p2;
	si8     	i, j, filled_slopes, in_nm1, tmp_delta_len;
	si8		*in_x;
	sf8     	*delta, *tmp_delta, *weights, *slopes;
	sf8		delta_0, delta_m1, delta_n, delta_n1;
	sf8		*sf8_p1, *sf8_p2, *sf8_p3, *sf8_p4, *sf8_p5, *sf8_p6;
	sf8		sf8_v1, sf8_v2, sf8_v3, sf8_v4;
	sf8 		*in_y, *dx, *out_x, *tmp_out_x, *out_y, *coefs[4];

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// Note: This function strays from the usual medlib template in that it does not allocate its processing buffers if not passed.
	// This is just because this function inherently complex anyway, and because otherwise there would be too many arguments for my taste.

	// input buffers
	// allocate with in_bufs =  CMP_allocate_buffers_m11(NULL, CMP_MAK_INTERP_INPUT_BUFFERS_m11, in_len + CMP_MAK_INTERP_PAD_SAMPLES_m11, sizeof(sf8), FALSE_m11, FALSE_m11);
	in_y = (sf8 *) in_bufs->buffer[CMP_MAK_IN_Y_BUF];  // == 0
	in_x = (si8 *) in_bufs->buffer[CMP_MAK_IN_X_BUF];  // == 1
	dx = (sf8 *) in_bufs->buffer[2];
	tmp_delta = (sf8 *) in_bufs->buffer[3];
	slopes = (sf8 *) in_bufs->buffer[4];
	weights = (sf8 *) in_bufs->buffer[5];
	coefs[0] = (sf8 *) in_bufs->buffer[6];
	coefs[1] = (sf8 *) in_bufs->buffer[7];
	
	// output buffers
	// allocate with out_bufs = CMP_allocate_buffers_m11(NULL, CMP_MAK_INTERP_OUTPUT_BUFFERS_m11, out_len, sizeof(sf8), FALSE_m11, FALSE_m11);
	out_y = (sf8 *) out_bufs->buffer[CMP_MAK_OUT_Y_BUF];  // == 0
	out_x = (sf8 *) out_bufs->buffer[CMP_MAK_OUT_X_BUF];  // == 1
	tmp_out_x = (sf8 *) out_bufs->buffer[2];
	index = (si8 *) out_bufs->buffer[3];

	// special cases
	if (in_len <= 1) {
		if (in_len == 0)
			return(out_y);
		for (i = 0; i < out_len; ++i)
			out_y[i] = in_y[0];
		return(out_y);
	}
	if (in_len == out_len) {
		sf8_p1 = out_x;
		si8_p1 = in_x;
		for (i = in_len; i--;)  // check for non-linearly spaced interpolation
			if (*sf8_p1++ != (sf8) *si8_p1++)
				break;
		if (i == -1) {
			memcpy((void *) out_y, (void *) in_y, (size_t) (in_len << 3));
			return(out_y);
		}
	}

	in_nm1 = in_len - 1;
	tmp_delta_len = in_nm1 + 4;
	delta = tmp_delta + 2;
	si8_p1 = in_x;
	si8_p2 = in_x + 1;
	sf8_p1 = dx;
	sf8_p2 = delta;
	sf8_p3 = in_y;
	sf8_p4 = in_y + 1;
	for (i = in_nm1; i--;) {
		*sf8_p1 = (sf8) (*si8_p2++ - *si8_p1++); // dx
		*sf8_p2++ = (*sf8_p4++ - *sf8_p3++) / *sf8_p1++;  // delta = dy / dx
	}

	if (in_len > 2) {
		delta_0 = ((sf8) 2.0 * delta[0]) - delta[1];
		delta_m1 = ((sf8) 2.0 * delta_0) - delta[0];
		delta_n = ((sf8) 2.0 * delta[in_len - 2]) - delta[in_len - 3];
		delta_n1 = ((sf8) 2.0 * delta_n) - delta[in_len - 2];

		tmp_delta[0] = delta_m1;
		tmp_delta[1] = delta_0;
		tmp_delta[tmp_delta_len - 2] = delta_n;
		tmp_delta[tmp_delta_len - 1] = delta_n1;

		sf8_p1 = tmp_delta;
		sf8_p2 = tmp_delta + 1;
		sf8_p3 = weights;
		for (i = tmp_delta_len - 1; i--;) {
			sf8_v1 = *sf8_p1++;
			sf8_v2 = *sf8_p2++;
			sf8_v3 = (sf8_v1 + sf8_v2) / (sf8) 2.0;
			sf8_v3 = (sf8_v3 >= (sf8) 0.0) ? sf8_v3 : -sf8_v3;
			sf8_v4 = (sf8_v2 - sf8_v1);
			sf8_v4 = (sf8_v4 >= (sf8) 0.0) ? sf8_v4 : -sf8_v4;
			*sf8_p3++ = sf8_v3 + sf8_v4;
		}
		sf8_p1 = weights;
		sf8_p2 = weights + 2;
		sf8_p3 = tmp_delta + 1;
		sf8_p4 = tmp_delta + 2;
		sf8_p5 = slopes;
		for (i = in_len; i--;) {
			sf8_v1 = *sf8_p1 + *sf8_p2;
			if (sf8_v1 != 0.0)
				*sf8_p5++ = ((*sf8_p2++ * *sf8_p3++) + (*sf8_p1++ * *sf8_p4++)) / sf8_v1;
		}
		filled_slopes = (si8) (sf8_p5 - slopes);
	} else {
		slopes[0] = slopes[1] = delta[0];
		filled_slopes = (si8) 2;
	}
	// unfilled slopes need to be zeroed
	sf8_p1 = slopes + filled_slopes;
	for (i = in_len - filled_slopes; i--;)
		*sf8_p1++ = (sf8)  0.0;

	coefs[2] = slopes;
	coefs[3] = in_y;
	sf8_p1 = dx;
	sf8_p2 = slopes;
	sf8_p3 = slopes + 1;
	sf8_p4 = delta;
	sf8_p5 = coefs[0];
	sf8_p6 = coefs[1];
	for (i = in_nm1; i--;) {
		*sf8_p5++ = ((*sf8_p2 + *sf8_p3) - (*sf8_p4 * (sf8) 2.0)) / (*sf8_p1 * *sf8_p1);  // column 0
		*sf8_p6++ = ((*sf8_p4++ * (sf8) 3.0) - (*sf8_p2++ * (sf8) 2.0) - *sf8_p3++) / *sf8_p1++;  // column 1
	}

	for (i = 0, j = 0; i < out_len; ++i) {
		sf8_v1 = out_x[i];
		while (((sf8) in_x[j]) <= sf8_v1) {
			++j;
			if (j == in_nm1)
				break;
		}
		index[i] = j - 1;
		if (j == in_nm1) {
			for (++i, --j; i < out_len; ++i)
				index[i] = j;
			break;
		}
	}

	memcpy((void *) tmp_out_x, (void *) out_x, (size_t) (out_len << 3));  // don't destroy out_x (for repeat calls)
	sf8_p1 = tmp_out_x;
	si8_p1 = index;
	for (i = out_len; i--;)
		*sf8_p1++ -= in_x[*si8_p1++];
	sf8_p1 = out_y;
	sf8_p2 = coefs[0];
	si8_p1 = index;
	for (i = out_len; i--;)
		*sf8_p1++ = sf8_p2[*si8_p1++];

	for (i = 1; i < 4; ++i) {
		sf8_p1 = out_y;
		sf8_p2 = tmp_out_x;
		sf8_p3 = coefs[i];
		si8_p1 = index;
		for (j = out_len; j--; ++sf8_p1)
			*sf8_p1 = ((sf8) *sf8_p2++ * *sf8_p1) + sf8_p3[*si8_p1++];
	}

	return(out_y);
}


void    CMP_PRED_decode_m11(CMP_PROCESSING_STRUCT_m11 *cps)
{
	TERN_m11	no_zero_counts;
	ui1		*comp_p, *ui1_p, *low_bound_high_byte_p, *high_bound_high_byte_p;
	ui1		*goal_bound_high_byte_p, prev_cat, overflow_bytes;
	ui1		*symbol_map[CMP_PRED_CATS_m11], *symbols;
	si1		*si1_p1, *si1_p2, *key_p;
	ui2		*bin_counts, *stats_entries, *count[CMP_PRED_CATS_m11];
	ui4		n_samps, n_derivs, n_keysample_bytes, total_stats_entries, sign_bit, sign_bytes;
	si4		*si4_p, *init_val_p, overflow_val;
	ui8		**minimum_range, **cumulative_count;
	ui8		low_bound, high_bound, prev_high_bound, goal_bound, range;
	si8		i, j;
	CMP_BLOCK_FIXED_HEADER_m11		*block_header;
	CMP_PRED_MODEL_FIXED_HEADER_m11		*PRED_header;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// CMP decompress from block_header to decompressed_ptr
	block_header = cps->block_header;
	n_samps = block_header->number_of_samples;
	
	// zero samples, or only one value
	if (n_samps <= 1) {
		if (block_header->number_of_samples == 1)
			cps->decompressed_ptr[0] = *((si4 *) (cps->parameters.model_region + CMP_PRED_MODEL_FIXED_HEADER_BYTES_m11));
		cps->parameters.derivative_level = 0;
		return;
	}
	
	PRED_header = (CMP_PRED_MODEL_FIXED_HEADER_m11 *) cps->parameters.model_region;
	n_derivs = PRED_header->derivative_level;
	n_keysample_bytes = PRED_header->number_of_keysample_bytes;
	stats_entries = PRED_header->numbers_of_statistics_bins;
	for (total_stats_entries = i = 0; i < CMP_PRED_CATS_m11; ++i)
		total_stats_entries += (ui4) stats_entries[i];
	
	// set parameters for return
	cps->parameters.derivative_level = n_derivs;
	
	// get block flags
	no_zero_counts = FALSE_m11;
	if (PRED_header->flags & CMP_PRED_FLAGS_NO_ZERO_COUNTS_MASK_m11)
		no_zero_counts = TRUE_m11;
	overflow_bytes = CMP_get_overflow_bytes_m11(cps, CMP_DECOMPRESSION_MODE_m11, CMP_PRED_COMPRESSION_m11);
	sign_bit = (ui4) 1 << ((overflow_bytes << 3) - 1);
	if (overflow_bytes == 4)
		sign_bytes = (ui4) 0;
	else  // Windows: shift of 32 bits is equated to shift of 0, so have to do this
		sign_bytes = (ui4) 0xFFFFFFFF << (overflow_bytes << 3);

	// copy initial derivative values to output buffer
	init_val_p = (si4 *) (cps->parameters.model_region + CMP_PRED_MODEL_FIXED_HEADER_BYTES_m11);
	for (i = 0; i < n_derivs; ++i)
		cps->decompressed_ptr[i] = *init_val_p++;

	// build symbol map, count arrays, & minimum ranges
	bin_counts = (ui2 *) init_val_p;
	symbols = (ui1 *) (bin_counts + total_stats_entries);
	cumulative_count = (ui8 **) cps->parameters.cumulative_count;
	minimum_range = (ui8 **) cps->parameters.minimum_range;
	for (i = 0; i < CMP_PRED_CATS_m11; ++i) {
		count[i] = bin_counts; bin_counts += stats_entries[i];
		symbol_map[i] = symbols; symbols += stats_entries[i];
		if (no_zero_counts == TRUE_m11) {  // TO DO: decide mapping scheme for unmapped symbols in symbol map
			// TO DO: copy count & symbol map to arrays with 256 elements
			for (j = stats_entries[i]; i < 256; ++i)
				count[i][j] = 1;
		}
		for (cumulative_count[i][0] = j = 0; j < stats_entries[i]; ++j) {
			cumulative_count[i][j + 1] = cumulative_count[i][j] + (ui8) count[i][j];
			minimum_range[i][j] = CMP_RED_TOTAL_COUNTS_m11 / count[i][j];
			if (CMP_RED_TOTAL_COUNTS_m11 > (count[i][j] * minimum_range[i][j]))
				++minimum_range[i][j];
		}
	}
	
	// range decode
	key_p = cps->parameters.keysample_buffer;
	prev_high_bound = goal_bound = low_bound = 0;
	range = CMP_RED_MAXIMUM_RANGE_m11;
	comp_p = (ui1 *) block_header + block_header->total_header_bytes;
	low_bound_high_byte_p = ((ui1 *) &low_bound) + 5;
	high_bound_high_byte_p = ((ui1 *) &high_bound) + 5;
	goal_bound_high_byte_p = ((ui1 *) &goal_bound) + 5;
	ui1_p = goal_bound_high_byte_p;
	j = 6; do {
		*ui1_p-- = *comp_p++;
	} while (--j);
	prev_cat = CMP_PRED_NIL_m11;
	
	for (i = n_keysample_bytes; i;) {
		for (j = 0; range >= minimum_range[prev_cat][j];) {
			high_bound = low_bound + ((range * cumulative_count[prev_cat][j + 1]) >> 16);
			if (high_bound > goal_bound) {
				range = high_bound - (low_bound = prev_high_bound);
				*key_p = symbol_map[prev_cat][j];
				if (!--i)
					goto PRED_RANGE_DECODE_DONE_m11;
				prev_cat = CMP_PRED_CAT_m11(*key_p); ++key_p;
				j = 0;
			} else {
				prev_high_bound = high_bound;
				++j;
			}
		}
		high_bound = low_bound + range;
		if (*low_bound_high_byte_p != *high_bound_high_byte_p) {
			ui1_p = goal_bound_high_byte_p;
			j = 6; do {
				*ui1_p-- = *comp_p++;
			} while (--j);
			low_bound = 0;
			range = CMP_RED_MAXIMUM_RANGE_m11;
		} else {
			do {
				low_bound <<= 8;
				high_bound <<= 8;
				goal_bound = (goal_bound << 8) | (ui8) *comp_p++;
				range <<= 8;
			} while (*low_bound_high_byte_p == *high_bound_high_byte_p);
			low_bound &= CMP_RED_RANGE_MASK_m11;
			goal_bound &= CMP_RED_RANGE_MASK_m11;
		}
		prev_high_bound = low_bound;
	} PRED_RANGE_DECODE_DONE_m11:

	// generate output from keysample data
	si4_p = cps->decompressed_ptr + n_derivs;
	si1_p1 = (si1 *) cps->parameters.keysample_buffer;
	for (i = n_samps - n_derivs; i--;) {
		if (*si1_p1 == CMP_SI1_KEYSAMPLE_FLAG_m11) {
			overflow_val = 0;
			++si1_p1;
			si1_p2 = (si1 *) &overflow_val;
			j = overflow_bytes; do {
				*si1_p2++ = *si1_p1++;
			} while (--j);
			if (overflow_val & sign_bit)
				overflow_val |= sign_bytes;
			*si4_p++ = overflow_val;
		} else {
			*si4_p++ = (si4) *si1_p1++;
		}
	}

	// integrate derivatives
	CMP_integrate_m11(cps);

	return;
}


CMP_PROCESSING_STRUCT_m11	*CMP_reallocate_processing_struct_m11(FILE_PROCESSING_STRUCT_m11 *fps, ui4 mode, si8 data_samples, ui4 block_samples)
{
	TERN_m11			realloc_flag;
	ui4				new_val;
	si8				new_compressed_bytes, new_keysample_bytes, new_decompressed_samples;
	si8				mem_units_used, mem_units_avail, pad_samples;
	CMP_PROCESSING_STRUCT_m11 	*cps;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (fps->universal_header->type_code != TIME_SERIES_DATA_FILE_TYPE_CODE_m11) {
		error_message_m11("%s(): FPS must be time series data\n", __FUNCTION__);
		return(NULL);
	}
	cps = fps->parameters.cps;
	if (cps == NULL)
		return(NULL);
	
	realloc_flag = FALSE_m11;
	new_compressed_bytes = new_keysample_bytes = new_decompressed_samples = 0;
	
	if (cps->parameters.allocated_block_samples < block_samples)
		realloc_flag = TRUE_m11;
	
	switch (cps->directives.algorithm) {
		case CMP_RED_COMPRESSION_m11:
		case CMP_PRED_COMPRESSION_m11:
		case CMP_VDS_COMPRESSION_m11:
			new_val = CMP_MAX_KEYSAMPLE_BYTES_m11(block_samples);
			if (cps->parameters.allocated_keysample_bytes < new_val) {
				new_keysample_bytes = new_val;
				realloc_flag = TRUE_m11;
			}
			break;
	}
	
	switch (mode) {
		case CMP_COMPRESSION_MODE_m11:
			if (cps->parameters.allocated_compressed_bytes) {
				mem_units_used = (ui1 *) cps->block_header - fps->time_series_data;
				mem_units_avail = cps->parameters.allocated_compressed_bytes - mem_units_used;
				new_val = CMP_MAX_COMPRESSED_BYTES_m11(block_samples, 1);
				if (mem_units_avail < new_val) {
					new_compressed_bytes = new_val + mem_units_used;
					realloc_flag = TRUE_m11;
				}
			}
			if (cps->parameters.allocated_decompressed_samples) {
				mem_units_used = cps->decompressed_ptr - cps->decompressed_data;
				mem_units_avail = cps->parameters.allocated_decompressed_samples - mem_units_used;
				if (mem_units_avail < block_samples) {
					new_decompressed_samples = mem_units_used + block_samples;
					realloc_flag = TRUE_m11;
				}
			}
			break;
		case CMP_DECOMPRESSION_MODE_m11:
			if (cps->parameters.allocated_decompressed_samples != CMP_SELF_MANAGED_MEMORY_m11) {
				mem_units_used = cps->decompressed_ptr - cps->decompressed_data;
				mem_units_avail = cps->parameters.allocated_decompressed_samples - mem_units_used;
				if (mem_units_avail < data_samples) {
					new_decompressed_samples = mem_units_used + data_samples;
					realloc_flag = TRUE_m11;
				}
			}
			break;
		default:
			error_message_m11("%s(): No compression mode specified\n", __FUNCTION__);
			return(NULL);
	}
	
	if (realloc_flag == FALSE_m11)
		return(cps);

	// reallocate (free & alloc for speed - don't copy data)
	CMP_cps_mutex_on_m11(cps);
		
	if (new_compressed_bytes) // FPS_reallocate_processing_struct_m11() resets cps->block_header for time series data fps
		FPS_reallocate_processing_struct_m11(fps, new_compressed_bytes + UNIVERSAL_HEADER_BYTES_m11);
	
	if (new_keysample_bytes) {
		free_m11((void * ) cps->parameters.keysample_buffer, __FUNCTION__);
		if ((cps->parameters.keysample_buffer = (si1 *) calloc_m11((size_t) new_keysample_bytes, sizeof(ui1), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11)) == NULL)
			goto CMP_REALLOC_CPS_FAIL_m11;
		cps->parameters.allocated_keysample_bytes = new_keysample_bytes;
	}
	
	if (new_decompressed_samples) {
		if (cps->decompressed_data != NULL)
			free_m11((void * ) cps->parameters.cache, __FUNCTION__);
		if ((cps->decompressed_data = cps->decompressed_ptr = cps->parameters.cache = (si4 *) calloc_m11((size_t) new_decompressed_samples, sizeof(si4), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11)) == NULL)
			goto CMP_REALLOC_CPS_FAIL_m11;
		cps->parameters.allocated_decompressed_samples = new_decompressed_samples;
	}
		
	// reallocate the following if they were previously allocated
	if (cps->parameters.allocated_block_samples < block_samples) {
		if (cps->parameters.detrended_buffer != NULL) {
			free_m11((void * ) cps->parameters.detrended_buffer, __FUNCTION__);
			if ((cps->parameters.detrended_buffer = (si4 *) calloc_m11((size_t) block_samples, sizeof(si4), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11)) == NULL)
				goto CMP_REALLOC_CPS_FAIL_m11;
		}
		if (cps->parameters.derivative_buffer != NULL) {
			free_m11((void * ) cps->parameters.derivative_buffer, __FUNCTION__);
			if ((cps->parameters.derivative_buffer = (si4 *) calloc_m11((size_t) block_samples, sizeof(si4), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11)) == NULL)
				goto CMP_REALLOC_CPS_FAIL_m11;
		}
		if (cps->parameters.scaled_amplitude_buffer != NULL) {
			free_m11((void * ) cps->parameters.scaled_amplitude_buffer, __FUNCTION__);
			if ((cps->parameters.scaled_amplitude_buffer = (si4 *) calloc_m11((size_t) block_samples, sizeof(si4), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11)) == NULL)
				goto CMP_REALLOC_CPS_FAIL_m11;
		}
		if (cps->parameters.scaled_frequency_buffer != NULL) {
			free_m11((void * ) cps->parameters.scaled_frequency_buffer, __FUNCTION__);
			if ((cps->parameters.scaled_frequency_buffer = (si4 *) calloc_m11((size_t) block_samples, sizeof(si4), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11)) == NULL)
				goto CMP_REALLOC_CPS_FAIL_m11;
		}
		if (cps->parameters.VDS_input_buffers != NULL) {
			if (mode == CMP_COMPRESSION_MODE_m11)
				pad_samples = CMP_VDS_LOWPASS_ORDER_m11 * 6;
			else
				pad_samples = CMP_MAK_PAD_SAMPLES_m11;
			CMP_allocate_buffers_m11(cps->parameters.VDS_input_buffers, CMP_VDS_INPUT_BUFFERS_m11, (si8) (block_samples + pad_samples), sizeof(sf8), FALSE_m11, FALSE_m11);
			CMP_allocate_buffers_m11(cps->parameters.VDS_output_buffers, CMP_VDS_OUTPUT_BUFFERS_m11, (si8) block_samples, sizeof(sf8), FALSE_m11, FALSE_m11);
		}
		cps->parameters.allocated_block_samples = block_samples;
	}
	
	CMP_cps_mutex_off_m11(cps);

	return(cps);
	
CMP_REALLOC_CPS_FAIL_m11:
	
	CMP_cps_mutex_off_m11(cps);
	return(NULL);
}


void    CMP_RED_decode_m11(CMP_PROCESSING_STRUCT_m11 *cps)
{
	TERN_m11			pos_derivs, no_zero_counts;
	ui1				*comp_p, *low_bound_high_byte_p, *high_bound_high_byte_p, *goal_bound_high_byte_p;
	ui1				*ui1_p1, *ui1_p2, *symbol_map, n_derivs, overflow_bytes;
	si1				*si1_p1, *si1_p2, *key_p;
	ui2				*count;
	ui4				n_samps, n_keysample_bytes;
	si4				*si4_p, overflow_val, sign_bit, sign_bytes, *init_val_p;
	ui8				*minimum_range, *cumulative_count;
	ui8				low_bound, high_bound, prev_high_bound, goal_bound, range;
	si8				i, j, n_stats_entries;
	CMP_BLOCK_FIXED_HEADER_m11	*block_header;
	CMP_RED_MODEL_FIXED_HEADER_m11	*RED_header;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// CMP decompress from block_header to decompressed_ptr
	block_header = cps->block_header;
	n_samps = block_header->number_of_samples;
	
	// zero or one or samples
	if (n_samps <= 1) {
		if (block_header->number_of_samples == 1)
			cps->decompressed_ptr[0] = *((si4 *) (cps->parameters.model_region + CMP_RED_MODEL_FIXED_HEADER_BYTES_m11));
		cps->parameters.derivative_level = 0;
		return;
	}

	RED_header = (CMP_RED_MODEL_FIXED_HEADER_m11 *) cps->parameters.model_region;
	n_derivs = RED_header->derivative_level;
	n_keysample_bytes = RED_header->number_of_keysample_bytes;
	n_stats_entries = (si8) RED_header->number_of_statistics_bins;
	
	// set parameters for return
	cps->parameters.derivative_level = RED_header->derivative_level;
	
	// get block flags
	no_zero_counts = FALSE_m11;
	if (RED_header->flags & CMP_RED_FLAGS_NO_ZERO_COUNTS_MASK_m11)
		no_zero_counts = TRUE_m11;
	pos_derivs = FALSE_m11;
	if (RED_header->flags & CMP_RED_FLAGS_POSITIVE_DERIVATIVES_MASK_m11)
		pos_derivs = TRUE_m11;
	overflow_bytes = CMP_get_overflow_bytes_m11(cps, CMP_DECOMPRESSION_MODE_m11, CMP_RED_COMPRESSION_m11);
	sign_bit = (ui4) 1 << ((overflow_bytes << 3) - 1);
	if (overflow_bytes == 4)
		sign_bytes = (ui4) 0;
	else  // Windows: shift of 32 bits is equated to shift of 0, so have to do this
		sign_bytes = (ui4) 0xFFFFFFFF << (overflow_bytes << 3);
	
	// copy initial derivative values to output buffer
	init_val_p = (si4 *) (cps->parameters.model_region + CMP_RED_MODEL_FIXED_HEADER_BYTES_m11);
	for (i = 0; i < n_derivs; ++i)
		cps->decompressed_ptr[i] = *init_val_p++;

	// build symbol map, count array, & minimum ranges
	count = (ui2 *) init_val_p;
	symbol_map = (ui1 *) (count + n_stats_entries);
	if (no_zero_counts == TRUE_m11) {  // TO DO: decide mapping scheme for unmapped symbols in symbol map
		for (i = n_stats_entries; i < 256; ++i)  // TO DO: copy count & symbol map to arrays with 256 elements
			count[i] = 1;
	}
	cumulative_count = (ui8 *) cps->parameters.cumulative_count;
	minimum_range = (ui8 *) cps->parameters.minimum_range;
	for (cumulative_count[0] = i = 0; i < n_stats_entries; ++i) {
		cumulative_count[i + 1] = cumulative_count[i] + (ui8) count[i];
		minimum_range[i] = CMP_RED_TOTAL_COUNTS_m11 / count[i];
		if (CMP_RED_TOTAL_COUNTS_m11 > (count[i] * minimum_range[i]))
			++minimum_range[i];
	}

	// range decode
	key_p = cps->parameters.keysample_buffer;
	prev_high_bound = goal_bound = low_bound = 0;
	range = CMP_RED_MAXIMUM_RANGE_m11;
	comp_p = symbol_map + n_stats_entries;
	low_bound_high_byte_p = ((ui1 *) &low_bound) + 5;
	high_bound_high_byte_p = ((ui1 *) &high_bound) + 5;
	goal_bound_high_byte_p = ((ui1 *) &goal_bound) + 5;
	ui1_p1 = goal_bound_high_byte_p;
	j = 6; do {
		*ui1_p1-- = *comp_p++;
	} while (--j);
	
	for (i = n_keysample_bytes; i;) {
		for (j = 0; range >= minimum_range[j];) {
			high_bound = low_bound + ((range * cumulative_count[j + 1]) >> 16);
			if (high_bound > goal_bound) {
				range = high_bound - (low_bound = prev_high_bound);
				*key_p = symbol_map[j];
				if (!--i)
					goto RED_RANGE_DECODE_DONE_m11;
				++key_p;
				j = 0;
			} else {
				prev_high_bound = high_bound;
				++j;
			}
		}
		high_bound = low_bound + range;
		if (*low_bound_high_byte_p != *high_bound_high_byte_p) {
			ui1_p1 = goal_bound_high_byte_p;
			j = 6; do {
				*ui1_p1-- = *comp_p++;
			} while (--j);
			low_bound = 0;
			range = CMP_RED_MAXIMUM_RANGE_m11;
		} else {
			do {
				low_bound <<= 8;
				high_bound <<= 8;
				goal_bound = (goal_bound << 8) | (ui8) *comp_p++;
				range <<= 8;
			} while (*low_bound_high_byte_p == *high_bound_high_byte_p);
			low_bound &= CMP_RED_RANGE_MASK_m11;
			goal_bound &= CMP_RED_RANGE_MASK_m11;
		}
		prev_high_bound = low_bound;
	} RED_RANGE_DECODE_DONE_m11:
		
	// generate output from keysample data
	si4_p = cps->decompressed_ptr + n_derivs;
	if (pos_derivs == TRUE_m11) {
		ui1_p1 = (ui1 *) cps->parameters.keysample_buffer;
		for (i = n_samps - n_derivs; i--;) {
			if (*ui1_p1 == CMP_POS_DERIV_KEYSAMPLE_FLAG_m11) {
				++ui1_p1;
				overflow_val = 0;
				ui1_p2 = (ui1 *) &overflow_val;
				j = overflow_bytes; do {
					*ui1_p2++ = *ui1_p1++;
				} while (--j);
				*si4_p++ = overflow_val;
			} else {
				*si4_p++ = (si4) *ui1_p1++;
			}
		}
	} else {
		si1_p1 = (si1 *) cps->parameters.keysample_buffer;
		for (i = n_samps - n_derivs; i--;) {
			if (*si1_p1 == CMP_SI1_KEYSAMPLE_FLAG_m11) {
				overflow_val = 0;
				++si1_p1;
				si1_p2 = (si1 *) &overflow_val;
				j = overflow_bytes; do {
					*si1_p2++ = *si1_p1++;
				} while (--j);
				if (overflow_val & sign_bit)
					overflow_val |= sign_bytes;
				*si4_p++ = overflow_val;
			} else {
				*si4_p++ = (si4) *si1_p1++;
			}
		}
	}
	
	// integrate derivatives
	CMP_integrate_m11(cps);
	
	return;
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void    CMP_retrend_si4_m11(si4 *in_y, si4 *out_y, si8 len, sf8 m, sf8 b)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// retrend data from input_buffer to output_buffer
	// if input_buffer == output_buffer retrending data will be done in place
	
	while (len--)
		*out_y++ = CMP_round_si4_m11((sf8) *in_y++ + (b += m));
	
	return;
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void    CMP_retrend_2_sf8_m11(sf8 *in_x, sf8 *in_y, sf8 *out_y, si8 len, sf8 m, sf8 b)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// retrend data from in_y to out_y at specific x locations
	// if input_buffer == output_buffer retrending data will be done in place
	
	while (len--)
		*out_y++ = CMP_round_si4_m11((sf8) *in_y++ + (m * (sf8) *in_x++) + b);
	
	return;
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si2      CMP_round_si2_m11(sf8 val)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (isnan(val))
		return(NAN_SI2_m11);
	
	if (val >= (sf8) 0.0) {
		if ((val += (sf8) 0.5) > (sf8) POS_INF_SI2_m11)
			return(POS_INF_SI2_m11);
	} else {
		if ((val -= (sf8) 0.5) < (sf8) NEG_INF_SI2_m11)
			return(NEG_INF_SI2_m11);
	}
	
	return((si2) val);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4      CMP_round_si4_m11(sf8 val)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (isnan(val))
		return(NAN_SI4_m11);
	
	if (val >= (sf8) 0.0) {
		if ((val += (sf8) 0.5) > (sf8) POS_INF_SI4_m11)
			return(POS_INF_SI4_m11);
	} else {
		if ((val -= (sf8) 0.5) < (sf8) NEG_INF_SI4_m11)
			return(NEG_INF_SI4_m11);
	}
	
	return((si4) val);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void      CMP_sf8_to_si4_m11(sf8 *sf8_arr, si4 *si4_arr, si8 len)
{
	sf8	val;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	while (len--) {
		val = *sf8_arr++;
		if (isnan(val)) {
			*si4_arr++ = NAN_SI4_m11;
			continue;
		}
		if (val >= (sf8) 0.0) {
			if ((val += (sf8) 0.5) > (sf8) POS_INF_SI4_m11) {
				*si4_arr++ = POS_INF_SI4_m11;
				continue;
			}
		} else {
			if ((val -= (sf8) 0.5) < (sf8) NEG_INF_SI4_m11) {
				*si4_arr++ = NEG_INF_SI4_m11;
				continue;
			}
		}
		*si4_arr++ = (si4) val;
	}
	
	return;
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void      CMP_sf8_to_si4_and_scale_m11(sf8 *sf8_arr, si4 *si4_arr, si8 len, sf8 scale)
{
	sf8	val;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	while (len--) {
		val = *sf8_arr++ * scale;
		if (isnan(val)) {
			*si4_arr++ = NAN_SI4_m11;
			continue;
		}
		if (val >= (sf8) 0.0) {
			if ((val += (sf8) 0.5) > (sf8) POS_INF_SI4_m11) {
				*si4_arr++ = POS_INF_SI4_m11;
				continue;
			}
		} else {
			if ((val -= (sf8) 0.5) < (sf8) NEG_INF_SI4_m11) {
				*si4_arr++ = NEG_INF_SI4_m11;
				continue;
			}
		}
		*si4_arr++ = (si4) val;
	}
	
	return;
}


void    CMP_show_block_header_m11(CMP_BLOCK_FIXED_HEADER_m11 *block_header)
{
	si1     hex_str[HEX_STRING_BYTES_m11(CRC_BYTES_m11)], time_str[TIME_STRING_BYTES_m11];
	ui4     i, mask;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	printf_m11("--------------- CMP Fixed Block Header - START ---------------\n");
	printf_m11("Block Start UID: 0x%lx\n", block_header->block_start_UID);
	if (block_header->block_CRC == CRC_NO_ENTRY_m11)
		printf_m11("Block CRC: no entry\n");
	else {
		generate_hex_string_m11((ui1 *)&block_header->block_CRC, CRC_BYTES_m11, hex_str);
		printf_m11("Block CRC: %s\n", hex_str);
	}
	printf_m11("Block Flag Bits: ");
	for (i = 0, mask = 1; i < 32; ++i, mask <<= 1) {
		if (block_header->block_flags & mask)
			printf_m11("%d ", i);
	}
	printf_m11(" (value: 0x%08x)\n", block_header->block_flags);
	if (block_header->start_time == UUTC_NO_ENTRY_m11)
		printf_m11("Start Time: no entry\n");
	else {
		time_string_m11(block_header->start_time, time_str, TRUE_m11, FALSE_m11, FALSE_m11);
		printf_m11("Start Time: %ld (µUTC), %s\n", block_header->start_time, time_str);
	}
	printf_m11("Acquisition Channel Number: %u\n", block_header->acquisition_channel_number);
	printf_m11("Total Block Bytes: %u\n", block_header->total_block_bytes);
	printf_m11("Number of Samples: %u\n", block_header->number_of_samples);
	printf_m11("Number of Records: %hu\n", block_header->number_of_records);
	printf_m11("Record Region Bytes: %hu\n", block_header->record_region_bytes);
	printf_m11("Parameter Flag Bits: ");
	for (i = 0, mask = 1; i < 32; ++i, mask <<= 1) {
		if (block_header->parameter_flags & mask)
			printf_m11("%d ", i);
	}
	printf_m11(" (value: 0x%08x)\n", block_header->parameter_flags);
	printf_m11("Parameter Region Bytes: %hu\n", block_header->parameter_region_bytes);
	printf_m11("Protected Region Bytes: %hu\n", block_header->protected_region_bytes);
	printf_m11("Discretionary Region Bytes: %hu\n", block_header->discretionary_region_bytes);
	printf_m11("Model Region Bytes: %hu\n", block_header->model_region_bytes);
	printf_m11("Total Header Bytes: %u\n", block_header->total_header_bytes);
	printf_m11("---------------- CMP Fixed Block Header - END ----------------\n\n");
	
	return;
}


void    CMP_show_block_model_m11(CMP_PROCESSING_STRUCT_m11 *cps, TERN_m11 recursed_call)
{
	ui1					*VDS_model_region;
	si1					*symbols, *time_alg, *amp_alg, *indent;
	ui2     				*counts;
	ui4					algorithm, mask, amp_alg_flag, time_alg_flag;
	si4					*derivs;
	si8     				i, total_counts;
	CMP_BLOCK_FIXED_HEADER_m11		*block_header;
	CMP_RED_MODEL_FIXED_HEADER_m11		*RED_header;
	CMP_PRED_MODEL_FIXED_HEADER_m11		*PRED_header;
	CMP_MBE_MODEL_FIXED_HEADER_m11		*MBE_header;
	CMP_VDS_MODEL_FIXED_HEADER_m11		*VDS_header;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	block_header = cps->block_header;

	// "recursed_call" argument is used internally for VDS models => call with FALSE_m11
	if (recursed_call == TRUE_m11) {
		indent = "\t";
	} else {
		indent = "";
		printf_m11("------------------- CMP Block Model - START ------------------\n");
	}
	switch (block_header->block_flags & CMP_BF_ALGORITHMS_MASK_m11) {
		case CMP_BF_RED_ENCODING_MASK_m11:
			RED_header = (CMP_RED_MODEL_FIXED_HEADER_m11 *) cps->parameters.model_region;
			printf_m11("%sModel: Range Encoded Differences (RED)\n", indent);
			printf_m11("%sNumber of Keysample Bytes: %u\n", indent, RED_header->number_of_keysample_bytes);
			printf_m11("%sDerivative Level: %hhu\n", indent, RED_header->derivative_level);
			if (RED_header->derivative_level > 0) {
				derivs = (si4 *) (cps->parameters.model_region + CMP_RED_MODEL_FIXED_HEADER_BYTES_m11);
				printf_m11("%sDerivative Initial Values: %d", indent, derivs[0]);
				for (i = 1; i < RED_header->derivative_level; ++i)
					printf_m11(", %d", derivs[i]);
				printf_m11("\n");
			}
			printf_m11("%sRED Model Flag Bits: ", indent);
			for (i = 0, mask = 1; i < 16; ++i, mask <<= 1) {
				if (RED_header->flags & mask)
					printf_m11("%d ", i);
			}
			printf_m11(" (value: 0x%04x)\n", RED_header->flags);
			printf_m11("\n%sNumber of Statistics Bins: %hu  (counts are scaled)\n", indent, RED_header->number_of_statistics_bins);
			// end fixed RED model fields
			counts = (ui2 *) (cps->parameters.model_region + CMP_RED_MODEL_FIXED_HEADER_BYTES_m11 + (RED_header->derivative_level * 4));
			symbols = (si1 *) (counts + RED_header->number_of_statistics_bins);
			for (i = 0; i < RED_header->number_of_statistics_bins; ++i)
				printf_m11("%s\tsymbol: %hhd\tcount: %hu\n", indent, *symbols++, *counts++);
			break;
			
		case CMP_BF_PRED_ENCODING_MASK_m11:
			PRED_header = (CMP_PRED_MODEL_FIXED_HEADER_m11 *) cps->parameters.model_region;
			printf_m11("%sModel: Predictive Range Encoded Differences (PRED)\n", indent);
			printf_m11("%sNumber of Keysample Bytes: %u\n", indent, PRED_header->number_of_keysample_bytes);
			printf_m11("%sDerivative Level: %hhu\n", indent, PRED_header->derivative_level);
			if (PRED_header->derivative_level > 0) {
				derivs = (si4 *) (cps->parameters.model_region + CMP_PRED_MODEL_FIXED_HEADER_BYTES_m11);
				printf_m11("%sDerivative Initial Values: %d", indent, derivs[0]);
				for (i = 1; i < PRED_header->derivative_level; ++i)
					printf_m11(", %d", derivs[i]);
				printf_m11("\n");
			}
			printf_m11("%sPRED Model Flag Bits: ", indent);
			for (i = 0, mask = 1; i < 16; ++i, mask <<= 1) {
				if (PRED_header->flags & mask)
					printf_m11("%d ", i);
			}
			printf_m11(" (value: 0x%04x)\n", PRED_header->flags);
			// end fixed PRED model fields
			counts = (ui2 *) (cps->parameters.model_region + CMP_PRED_MODEL_FIXED_HEADER_BYTES_m11 + (PRED_header->derivative_level * 4));
			total_counts = (si8) (PRED_header->number_of_nil_statistics_bins + PRED_header->number_of_pos_statistics_bins + PRED_header->number_of_neg_statistics_bins);
			symbols = (si1 *) (counts + total_counts);
			printf_m11("\n%sNumber of NIL Statistics Bins: %hu  (counts are scaled)\n", indent, PRED_header->number_of_nil_statistics_bins);
			for (i = 0; i < PRED_header->number_of_nil_statistics_bins; ++i)
				printf_m11("%sbin %02d:    symbol: %hhd\tcount: %hu\n", indent, i, *symbols++, *counts++);
			printf_m11("\n%sNumber of POS Statistics Bins: %hu  (counts are scaled)\n", PRED_header->number_of_pos_statistics_bins);
			for (i = 0; i < PRED_header->number_of_pos_statistics_bins; ++i)
				printf_m11("%sbin %02d:    symbol: %hhd\tcount: %hu\n", indent, i, *symbols++, *counts++);
			printf_m11("\n%sNumber of NEG Statistics Bins: %hu  (counts are scaled)\n", indent, PRED_header->number_of_neg_statistics_bins);
			for (i = 0; i < PRED_header->number_of_neg_statistics_bins; ++i)
				printf_m11("%sbin %02d:    symbol: %hhd\tcount: %hu\n", indent, i, *symbols++, *counts++);
			break;
			
		case CMP_BF_MBE_ENCODING_MASK_m11:
			MBE_header = (CMP_MBE_MODEL_FIXED_HEADER_m11 *) cps->parameters.model_region;
			printf_m11("%sModel: Minimal Bit Encoding (MBE)\n", indent);
			printf_m11("%sMinimum Value: %d\n", indent, MBE_header->minimum_value);
			printf_m11("%sBits per Sample: %hhu\n", indent, MBE_header->bits_per_sample);
			printf_m11("%sDerivative Level: %hhu\n", indent, MBE_header->derivative_level);
			if (MBE_header->derivative_level > 0) {
				derivs = (si4 *) (cps->parameters.model_region + CMP_MBE_MODEL_FIXED_HEADER_BYTES_m11);
				printf_m11("%sDerivative Initial Values: %d", indent, derivs[0]);
				for (i = 1; i < MBE_header->derivative_level; ++i)
					printf_m11(", %d", derivs[i]);
				printf_m11("\n");
			}
			printf_m11("%sMBE Model Flag Bits: ", indent);
			for (i = 0, mask = 1; i < 16; ++i, mask <<= 1) {
				if (MBE_header->flags & mask)
					printf_m11("%d ", i);
			}
			printf_m11(" (value: 0x%04x)\n", MBE_header->flags);
			break;
			
		case CMP_BF_VDS_ENCODING_MASK_m11:
			VDS_model_region = cps->parameters.model_region;
			VDS_header = (CMP_VDS_MODEL_FIXED_HEADER_m11 *) VDS_model_region;
			algorithm = VDS_header->flags & CMP_VDS_AMPLITUDE_ALGORITHMS_MASK_m11;
			switch (algorithm) {
				case CMP_VDS_FLAGS_AMPLITUDE_RED_MASK_m11:
					amp_alg = "RED";
					amp_alg_flag = CMP_BF_RED_ENCODING_MASK_m11;
					break;
				case CMP_VDS_FLAGS_AMPLITUDE_PRED_MASK_m11:
					amp_alg = "PRED";
					amp_alg_flag = CMP_BF_PRED_ENCODING_MASK_m11;
					break;
				case CMP_VDS_FLAGS_AMPLITUDE_MBE_MASK_m11:
					amp_alg = "MBE";
					amp_alg_flag = CMP_BF_MBE_ENCODING_MASK_m11;
					break;
			}
			algorithm = VDS_header->flags & CMP_VDS_TIME_ALGORITHMS_MASK_m11;
			switch (algorithm) {
				case CMP_VDS_FLAGS_TIME_RED_MASK_m11:
					time_alg = "RED";
					time_alg_flag = CMP_BF_RED_ENCODING_MASK_m11;
					break;
				case CMP_VDS_FLAGS_TIME_PRED_MASK_m11:
					time_alg = "PRED";
					time_alg_flag = CMP_BF_PRED_ENCODING_MASK_m11;
					break;
				case CMP_VDS_FLAGS_TIME_MBE_MASK_m11:
					time_alg = "MBE";
					time_alg_flag = CMP_BF_MBE_ENCODING_MASK_m11;
					break;
			}
			printf_m11("Model: Vectorized Data Stream (VDS)\n");
			printf_m11("Number of VDS Samples: %u\n", VDS_header->number_of_VDS_samples);
			printf_m11("Amplitude Block Total Bytes: %u\n", VDS_header->amplitude_block_total_bytes);
			printf_m11("Amplitude Block Model: %s\n", amp_alg);
			printf_m11("Amplitude Block Model Bytes: %hu\n", VDS_header->amplitude_block_model_bytes);
			printf_m11("Time Block Model: %s\n", time_alg);
			printf_m11("Time Block Model Bytes: %hu\n", VDS_header->time_block_model_bytes);
			printf_m11("VDS Model Flag Bits: ");
			for (i = 0, mask = 1; i < 32; ++i, mask <<= 1) {
				if (VDS_header->flags & mask)
					printf_m11("%d ", i);
			}
			printf_m11(" (value: 0x%08x)\n", VDS_header->flags);
			// show amplitude model
			printf_m11("\t============== VDS Amplitude Block Model - START =============\n");
			cps->parameters.model_region = VDS_model_region + CMP_VDS_MODEL_FIXED_HEADER_BYTES_m11;
			block_header->block_flags &= ~CMP_BF_ALGORITHMS_MASK_m11;
			block_header->block_flags |= amp_alg_flag;
			CMP_show_block_model_m11(cps, TRUE_m11);
			printf_m11("\t=============== VDS Amplitude Block Model - END ==============\n");
			// show time model
			printf_m11("\t================ VDS Time Block Model - START ================\n");
			cps->parameters.model_region += VDS_header->amplitude_block_total_bytes;
			block_header->block_flags &= ~CMP_BF_ALGORITHMS_MASK_m11;
			block_header->block_flags |= time_alg_flag;
			CMP_show_block_model_m11(cps, TRUE_m11);
			printf_m11("\t================= VDS Time Block Model - END =================\n");
			// restore base VDS model
			cps->parameters.model_region = VDS_model_region;
			block_header->block_flags &= ~CMP_BF_ALGORITHMS_MASK_m11;
			block_header->block_flags |= CMP_BF_VDS_ENCODING_MASK_m11;
			break;
		default:
			error_message_m11("%s(): Unrecognized model (%u)\n", __FUNCTION__, block_header->block_flags & CMP_BF_ALGORITHMS_MASK_m11);
			break;
	}
	if (recursed_call != TRUE_m11)
		printf_m11("-------------------- CMP Block Model - END -------------------\n\n");
	
	return;
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void      CMP_si4_to_sf8_m11(si4 *si4_arr, sf8 *sf8_arr, si8 len)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	while (len--)
		*sf8_arr++ = (sf8) *si4_arr++;
	
	return;
}


// Code adapted from Numerical Recipes in C. Public domain.
sf8    *CMP_spline_interp_sf8_m11(sf8 *in_arr, si8 in_arr_len, sf8 *out_arr, si8 out_arr_len, CMP_BUFFERS_m11 *spline_bufs)
{
	TERN_m11	free_buffers;
	si8		i, lo_pt, hi_pt;
	sf8		*prev_y, *y, *next_y, *ty, out_x, out_x_inc, h, a, b;
	sf8		*d2y, *td2y, *prev_d2y, *next_d2y, *tu, *u, *prev_u, p, *tout;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// this version assumes input array is uniformly sampled; output array is uniformly sampled at new frequency
	// if out_arr is NULL, it is allocated and returned
	// if passing, allocate 3 buffers with (in_arr_len + CMP_SPLINE_TAIL_LEN_m11) elements of type sf8
	
	if (out_arr == NULL)
		out_arr = (sf8 *) malloc_m11((size_t) (out_arr_len << 3), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);

	if (in_arr_len <= 1) {
		if (in_arr_len == 0)
			return(out_arr);
		for (i = 0; i < out_arr_len; ++i)
			out_arr[i] = in_arr[0];
		return(out_arr);
	}
	if (in_arr_len == out_arr_len) {
		memcpy(out_arr, in_arr, in_arr_len << 3);
		return(out_arr);
	}
	
	free_buffers = FALSE_m11;
	if (spline_bufs == NULL)
		free_buffers = TRUE_m11;
	spline_bufs = CMP_allocate_buffers_m11(spline_bufs, 3, in_arr_len + CMP_SPLINE_TAIL_LEN_m11, sizeof(sf8), FALSE_m11, FALSE_m11);  // also reallocates
	y = (sf8 *) spline_bufs->buffer[0];
	d2y = (sf8 *) spline_bufs->buffer[1];
	u = (sf8 *) spline_bufs->buffer[2];
	memcpy((void *) y, (void *) in_arr, (size_t) (in_arr_len << 3));

	for (h = 2.0 * y[in_arr_len - 1], i = 0; i < CMP_SPLINE_TAIL_LEN_m11; ++i)
		y[in_arr_len + i] = h - y[in_arr_len - 2 - i];
	
	// spline
	d2y[0] = u[0] = 0.0;
	prev_d2y = d2y;
	td2y = prev_d2y + 1;
	prev_y = y;
	ty = prev_y + 1;
	next_y = ty + 1;
	prev_u = u;
	tu = prev_u + 1;
	
	in_arr_len += CMP_SPLINE_TAIL_LEN_m11;
	for (i = in_arr_len; i--;) {
		p = (*prev_d2y++ * 0.5) + 2.0;
		*td2y++ = -0.5 / p;
		*tu = (*next_y++ - *ty) - (*ty - *prev_y++);
		++ty;
		*tu = (3.0 * *tu - 0.5 * *prev_u++) / p;
		++tu;
	}
	*td2y = 0.0;
	
	next_d2y = d2y + in_arr_len - 1;
	td2y = next_d2y - 1;
	tu = u + in_arr_len - 2;
	for (i = in_arr_len - 1; i--;) {
		*td2y = (*td2y * *next_d2y--) + *tu--;
		--td2y;
	}
	in_arr_len -= CMP_SPLINE_TAIL_LEN_m11;
	
	// splint
	out_x_inc = (sf8) in_arr_len / (sf8) out_arr_len;
	out_x = -out_x_inc;
	tout = out_arr;
	for (i = out_arr_len; i--;) {
		hi_pt = (lo_pt = (si4) (out_x += out_x_inc)) + 1;
		b = 1.0 - (a = (sf8) hi_pt - out_x);
		*tout++ = (a * y[lo_pt] + b * y[hi_pt] + ((a * a * a - a) * d2y[lo_pt] + (b * b * b - b) * d2y[hi_pt]) / 6.0);
	}
	
	// clean up
	if (free_buffers == TRUE_m11)
		CMP_free_buffers_m11(spline_bufs, TRUE_m11);
	
	return(out_arr);
}


// Code adapted from Numerical Recipes in C. Public domain.
si4    *CMP_spline_interp_si4_m11(si4 *in_arr, si8 in_arr_len, si4 *out_arr, si8 out_arr_len, CMP_BUFFERS_m11 *spline_bufs)
{
	TERN_m11	free_buffers;
	si4	*tin, *tout;
	si8	i, lo_pt, hi_pt;
	sf8	*prev_y, *y, *next_y, *ty, out_x, out_x_inc, h, a, b;
	sf8	*d2y, *td2y, *prev_d2y, *next_d2y, *tu, *u, *prev_u, p;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// this version assumes input array is uniformly sampled; output array is uniformly sampled at new frequency
	// if out_arr is NULL, it is allocated and returned
	
	if (out_arr == NULL)
		out_arr = (si4 *) malloc_m11((size_t) (out_arr_len << 2), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);

	if (in_arr_len <= 1) {
		if (in_arr_len == 0)
			return(out_arr);
		for (i = 0; i < out_arr_len; ++i)
			out_arr[i] = in_arr[0];
		return(out_arr);
	}
	if (in_arr_len == out_arr_len) {
		memcpy(out_arr, in_arr, in_arr_len << 2);
		return(out_arr);
	}

	free_buffers = FALSE_m11;
	if (spline_bufs == NULL)
		free_buffers = TRUE_m11;
	spline_bufs = CMP_allocate_buffers_m11(spline_bufs, 3, in_arr_len, sizeof(sf8), FALSE_m11, FALSE_m11);  // also reallocates
	y = (sf8 *) spline_bufs->buffer[0];
	d2y = (sf8 *) spline_bufs->buffer[1];
	u = (sf8 *) spline_bufs->buffer[2];
	memcpy((void *) y, (void *) in_arr, (size_t) (in_arr_len << 3));

	for (tin = in_arr, ty = y, i = in_arr_len; i--;)
		*ty++ = (sf8) *tin++;
	
	for (h = 2.0 * y[in_arr_len - 1], i = 0; i < CMP_SPLINE_TAIL_LEN_m11; ++i)
		y[in_arr_len + i] = h - y[in_arr_len - 2 - i];
	
	// spline
	d2y[0] = u[0] = 0.0;
	prev_d2y = d2y;
	td2y = prev_d2y + 1;
	prev_y = y;
	ty = prev_y + 1;
	next_y = ty + 1;
	prev_u = u;
	tu = prev_u + 1;
	
	in_arr_len += CMP_SPLINE_TAIL_LEN_m11;
	for (i = 1; i <= in_arr_len - 2; i++) {
		p = (*prev_d2y++ * 0.5) + 2.0;
		*td2y++ = -0.5 / p;
		*tu = (*next_y++ - *ty) - (*ty - *prev_y++);
		++ty;
		*tu = (3.0 * *tu - 0.5 * *prev_u++) / p;
		++tu;
	}
	*td2y = 0.0;
	
	next_d2y = d2y + in_arr_len - 1;
	td2y = next_d2y - 1;
	tu = u + in_arr_len - 2;
	for (i = in_arr_len - 2; i >= 0; i--) {
		*td2y = (*td2y * *next_d2y--) + *tu--;
		--td2y;
	}
	in_arr_len -= CMP_SPLINE_TAIL_LEN_m11;
	
	// splint
	out_x_inc = (sf8) in_arr_len / (sf8) out_arr_len;
	out_x = -out_x_inc;
	tout = out_arr;
	for (i = 0; i < out_arr_len; i++) {
		hi_pt = (lo_pt = (si4) (out_x += out_x_inc)) + 1;
		b = 1.0 - (a = (sf8) hi_pt - out_x);
		*tout++ = (si4) ((a * y[lo_pt] + b * y[hi_pt] + ((a * a * a - a) * d2y[lo_pt] + (b * b * b - b) * d2y[hi_pt]) / 6.0) + 0.5);
	}
	
	// clean up
	if (free_buffers == TRUE_m11)
		CMP_free_buffers_m11(spline_bufs, TRUE_m11);

	return(out_arr);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void	CMP_unlock_buffers_m11(CMP_BUFFERS_m11 *buffers)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// unlock
	if (buffers->locked != FALSE_m11) {
		buffers->locked = munlock_m11((void *) buffers->buffer, buffers->total_allocated_bytes, __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
		buffers->locked = FALSE_m11;
	}

	return;
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void    CMP_unscale_amplitude_si4_m11(si4 *input_buffer, si4 *output_buffer, si8 len, sf8 scale_factor)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// unscale from input_buffer to output_buffer
	// if input_buffer == output_buffer unscaling will be done in place
	
	while (len--)
		*output_buffer++ = CMP_round_si4_m11((sf8) *input_buffer++ * scale_factor);

	return;
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void    CMP_unscale_amplitude_sf8_m11(sf8 *input_buffer, sf8 *output_buffer, si8 len, sf8 scale_factor)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// unscale from input_buffer to output_buffer
	// if input_buffer == output_buffer unscaling will be done in place
	
	while (len--)
		*output_buffer++ = *input_buffer++ * scale_factor;

	return;
}


void    CMP_unscale_frequency_si4_m11(si4 *input_buffer, si4 *output_buffer, si8 len, sf8 scale_factor)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// not written yet
	return;
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
CMP_BLOCK_FIXED_HEADER_m11	*CMP_update_CPS_pointers_m11(FILE_PROCESSING_STRUCT_m11 *fps, ui1 flags)
{
	CMP_BLOCK_FIXED_HEADER_m11	*block_header;
	CMP_PROCESSING_STRUCT_m11	*cps;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (fps->universal_header->type_code != TIME_SERIES_DATA_FILE_TYPE_CODE_m11) {
		error_message_m11("%s(): FPS must be time series data\n", __FUNCTION__);
		return(NULL);
	}
	cps = fps->parameters.cps;
	
	block_header = cps->block_header;
	if (flags & CMP_UPDATE_ORIGINAL_PTR_m11)
		cps->original_ptr += block_header->number_of_samples;
	else if (flags & CMP_RESET_ORIGINAL_PTR_m11)
		cps->original_ptr = cps->original_data;
	
	if (flags & CMP_UPDATE_BLOCK_HEADER_PTR_m11)
		cps->block_header = (CMP_BLOCK_FIXED_HEADER_m11 *) ((ui1 *) cps->block_header + block_header->total_block_bytes);
	else if (flags & CMP_RESET_BLOCK_HEADER_PTR_m11)
		cps->block_header = (CMP_BLOCK_FIXED_HEADER_m11 *) fps->data_pointers;
	
	if (flags & CMP_UPDATE_DECOMPRESSED_PTR_m11)
		cps->decompressed_ptr += block_header->number_of_samples;
	else if (flags & CMP_RESET_DECOMPRESSED_PTR_m11)
		cps->decompressed_ptr = cps->decompressed_data = cps->parameters.cache;

	return(cps->block_header);
}


void	CMP_VDS_decode_m11(CMP_PROCESSING_STRUCT_m11 *cps)
{
	ui1				*VDS_model_region;
	ui4				VDS_total_header_bytes, number_of_samples, algorithm;
	ui4				start_sample;
	si4				*si4_p;
	sf4				*sf4_p;
	si8				i, *si8_p, offset, *in_x;
	sf8				amplitude_scale, *in_y, *out_x, *out_y, *sf8_p, val;
	void				*saved_cumulative_count_p, *saved_minimum_range_p;
	CMP_BLOCK_FIXED_HEADER_m11	*block_header;
	CMP_BUFFERS_m11			*VDS_in_bufs, *VDS_out_bufs;
	CMP_VDS_MODEL_FIXED_HEADER_m11	*VDS_header;
	
	
	// save values set by CMP_decode_m11();
	block_header = cps->block_header;
	number_of_samples = (si8) block_header->number_of_samples;
	VDS_model_region = cps->parameters.model_region;  // set by CMP_decode_m11()
	VDS_header = (CMP_VDS_MODEL_FIXED_HEADER_m11 *) VDS_model_region;
	VDS_total_header_bytes = block_header->total_header_bytes;
	
	// decode amplitude data
	block_header->number_of_samples = VDS_header->number_of_VDS_samples;
	cps->parameters.model_region = (ui1 *) block_header + (si8) VDS_total_header_bytes;
	block_header->total_header_bytes = VDS_total_header_bytes + VDS_header->amplitude_block_model_bytes;
	block_header->model_region_bytes = VDS_header->amplitude_block_model_bytes;
	algorithm = VDS_header->flags & CMP_VDS_AMPLITUDE_ALGORITHMS_MASK_m11;
	switch (algorithm) {
		case CMP_VDS_FLAGS_AMPLITUDE_RED_MASK_m11:  // older VDS used RED for amplitudes - this should go away eventually
			// change PRED buffers to RED
			saved_cumulative_count_p = cps->parameters.cumulative_count;
			saved_minimum_range_p = cps->parameters.minimum_range;
			cps->parameters.cumulative_count = *((void **) saved_cumulative_count_p);
			cps->parameters.minimum_range = *((void **) saved_minimum_range_p);
			// decode
			CMP_RED_decode_m11(cps);
			// restore PRED buffers
			cps->parameters.cumulative_count = saved_cumulative_count_p;
			cps->parameters.minimum_range = saved_minimum_range_p;
			break;
		case CMP_VDS_FLAGS_AMPLITUDE_PRED_MASK_m11:
			CMP_PRED_decode_m11(cps);
			break;
		case CMP_VDS_FLAGS_AMPLITUDE_MBE_MASK_m11:
			CMP_MBE_decode_m11(cps);
			break;
	}

	// set up VDS buffers
	cps->parameters.VDS_input_buffers = CMP_allocate_buffers_m11(cps->parameters.VDS_input_buffers, CMP_VDS_INPUT_BUFFERS_m11, (si8) (VDS_header->number_of_VDS_samples + CMP_MAK_PAD_SAMPLES_m11), sizeof(sf8), FALSE_m11, FALSE_m11);
	cps->parameters.VDS_output_buffers = CMP_allocate_buffers_m11(cps->parameters.VDS_output_buffers, CMP_VDS_OUTPUT_BUFFERS_m11, (si8) number_of_samples, sizeof(sf8), FALSE_m11, FALSE_m11);
	VDS_in_bufs = cps->parameters.VDS_input_buffers;
	in_y = (sf8 *) VDS_in_bufs->buffer[CMP_MAK_IN_Y_BUF];  // location specified by mak_interp()
	in_x = (si8 *) VDS_in_bufs->buffer[CMP_MAK_IN_X_BUF];  // location specified by mak_interp()
	VDS_out_bufs = cps->parameters.VDS_output_buffers;
	out_y = (sf8 *) VDS_out_bufs->buffer[CMP_MAK_OUT_Y_BUF];  // location specified by mak_interp()
	out_x = (sf8 *) VDS_out_bufs->buffer[CMP_MAK_OUT_X_BUF];  // location specified by mak_interp()

	// copy amplitudes to sf8 buffer
	CMP_si4_to_sf8_m11(cps->decompressed_ptr, in_y, (si8) VDS_header->number_of_VDS_samples);

	// apply amplitude scaling (if applied) here (b/c fewer samples)
	if (block_header->parameter_flags & CMP_PF_AMPLITUDE_SCALE_MASK_m11) {
		sf4_p = (sf4 *) cps->block_parameters;
		offset = (si8) cps->parameters.block_parameter_map[CMP_PF_AMPLITUDE_SCALE_IDX_m11];
		amplitude_scale = (sf8) *(sf4_p + offset);
		CMP_unscale_amplitude_sf8_m11(in_y, in_y, (si8) VDS_header->number_of_VDS_samples, amplitude_scale);
	}

	// decode time data
	block_header->total_header_bytes = VDS_total_header_bytes + VDS_header->amplitude_block_total_bytes + VDS_header->time_block_model_bytes;
	cps->parameters.model_region = (ui1 *) block_header + (si8) VDS_total_header_bytes + (si8) VDS_header->amplitude_block_total_bytes;
	block_header->model_region_bytes = VDS_header->time_block_model_bytes;
	algorithm = VDS_header->flags & CMP_VDS_TIME_ALGORITHMS_MASK_m11;
	switch (algorithm) {
		case CMP_VDS_FLAGS_TIME_RED_MASK_m11:
			// change PRED amplitude buffers to RED time buffers
			saved_cumulative_count_p = cps->parameters.cumulative_count;
			saved_minimum_range_p = cps->parameters.minimum_range;
			cps->parameters.cumulative_count = *((void **) saved_cumulative_count_p);
			cps->parameters.minimum_range = *((void **) saved_minimum_range_p);
			CMP_RED_decode_m11(cps);
			// restore PRED amplitude buffers
			cps->parameters.cumulative_count = saved_cumulative_count_p;
			cps->parameters.minimum_range = saved_minimum_range_p;
			break;
		case CMP_VDS_FLAGS_TIME_MBE_MASK_m11:
			CMP_MBE_decode_m11(cps);
			break;
	}
	
	// copy times to si8 buffer
	si8_p = in_x;
	si4_p = cps->decompressed_ptr;
	for (i = VDS_header->number_of_VDS_samples; i--;)
		*si8_p++ = (si8) *si4_p++;

	// reconstruct trace
	if (cps->directives.cps_caching == TRUE_m11) {
		start_sample = 0;
	} else {
		start_sample = cps->parameters.block_start_index;
		if (cps->parameters.block_end_index != 0xFFFFFFFF)
			number_of_samples = cps->parameters.block_end_index + 1;
		number_of_samples -= start_sample;
	}
	
	// build out array & interpolate
	sf8_p = out_x;  // create mak out_x array
	val = (sf8) start_sample - (sf8) 1.0;
	for (i = number_of_samples; i--;)
		*sf8_p++ = (val += (sf8) 1.0);
	CMP_mak_interp_sf8_m11(VDS_in_bufs, (si8) VDS_header->number_of_VDS_samples, VDS_out_bufs, (si8) number_of_samples);

	// copy interpolated data to decompressed buffer
	CMP_sf8_to_si4_m11(out_y, cps->decompressed_ptr, number_of_samples);

	// restore block_header
	block_header->number_of_samples = number_of_samples;
	block_header->total_header_bytes = VDS_total_header_bytes;
	block_header->model_region_bytes = (ui2) CMP_VDS_MODEL_FIXED_HEADER_BYTES_m11;
	cps->parameters.model_region = VDS_model_region;

	return;
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void    CMP_zero_buffers_m11(CMP_BUFFERS_m11 *buffers)
{
	ui1	*zero_start;
	ui8	pointer_bytes, bytes_to_zero;
	
	
	pointer_bytes = buffers->n_buffers * sizeof(void *);
	bytes_to_zero = buffers->total_allocated_bytes - pointer_bytes;
	zero_start = (ui1 *) buffers->buffer + pointer_bytes;
	
	memset((void *) zero_start, 0, (size_t) bytes_to_zero);
	
	return;
}



//***********************************************************************//
//***************************  CRC FUNCTIONS  ***************************//
//***********************************************************************//

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


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
ui4	CRC_calculate_m11(const ui1 *block_ptr, si8 block_bytes)
{
	ui4	crc;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	crc = CRC_update_m11(block_ptr, block_bytes, CRC_START_VALUE_m11);
	
	return(crc);
}


ui4     CRC_combine_m11(ui4 block_1_crc, ui4 block_2_crc, si8 block_2_bytes)
{
	ui4     n, col;
	ui4     even[32];    // even-power-of-two zeros operator
	ui4     odd[32];     // odd-power-of-two zeros operator
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// degenerate case (also disallow negative lengths)
	if (block_2_bytes <= 0)
		return(block_1_crc);
	
	// put operator for one zero bit in odd
	odd[0] = CRC_POLYNOMIAL_m11;
	col = 1;
	for (n = 1; n < 32; n++) {
		odd[n] = col;
		col <<= 1;
	}
	
	// put operator for two zero bits in even
	CRC_matrix_square_m11(even, odd);
	
	// put operator for four zero bits in odd
	CRC_matrix_square_m11(odd, even);
	
	// apply block_2_bytes zeros to crc1 (first square will put the operator for one zero byte, eight zero bits, in even)
	do {
		// apply zeros operator for this bit of block_2_bytes
		CRC_matrix_square_m11(even, odd);
		if (block_2_bytes & 1)
			block_1_crc = CRC_matrix_times_m11(even, block_1_crc);
		block_2_bytes >>= 1;
		
		// if no more bits set, then done
		if (block_2_bytes == 0)
			break;
		
		// another iteration of the loop with odd and even swapped
		CRC_matrix_square_m11(odd, even);
		if (block_2_bytes & 1)
			block_1_crc = CRC_matrix_times_m11(odd, block_1_crc);
		block_2_bytes >>= 1;
		
		// if no more bits set, then done
	} while (block_2_bytes != 0);
	
	return(block_1_crc ^ block_2_crc);
}


TERN_m11	CRC_initialize_tables_m11(void)
{
	ui4	**crc_table, c, n, k;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif

	pthread_mutex_lock_m11(&globals_m11->CRC_mutex);
	
	if (globals_m11->CRC_table == NULL) {
		globals_m11->CRC_table = (ui4 **) calloc_2D_m11((size_t) CRC_TABLES_m11, (size_t) CRC_TABLE_ENTRIES_m11, sizeof(ui4), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	
		// generate a crc for every 8-bit value
		crc_table = globals_m11->CRC_table;
		for (n = 0; n < CRC_TABLE_ENTRIES_m11; n++) {
			for (c = n, k = 0; k < 8; k++)
				c = c & 1 ? CRC_POLYNOMIAL_m11 ^ (c >> 1) : c >> 1;
			crc_table[0][n] = c;
		}
		
		// generate crc for each value followed by one, two, and three zeros, and then the byte reversal of those as well as the first table
		for (n = 0; n < CRC_TABLE_ENTRIES_m11; n++) {
			c = crc_table[0][n];
			crc_table[4][n] = CRC_SWAP32_m11(c);
			for (k = 1; k < 4; k++) {
				c = crc_table[0][c & 0xff] ^ (c >> 8);
				crc_table[k][n] = c;
				crc_table[k + 4][n] = CRC_SWAP32_m11(c);
			}
		}
	}
	
	pthread_mutex_unlock_m11(&globals_m11->CRC_mutex);

	return(TRUE_m11);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void    CRC_matrix_square_m11(ui4 *square, const ui4 *mat)
{
	ui4     n;
	
	
	for (n = 0; n < 32; n++)
		square[n] = CRC_matrix_times_m11(mat, mat[n]);
	
	return;
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
ui4      CRC_matrix_times_m11(const ui4 *mat, ui4 vec)
{
	ui4     sum;
	
	
	sum = 0;
	while (vec) {
		if (vec & 1)
			sum ^= *mat;
		vec >>= 1;
		mat++;
	}
	
	return(sum);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
ui4	CRC_update_m11(const ui1 *block_ptr, si8 block_bytes, ui4 current_crc)
{
	ui4			**crc_table;
	register ui4            c;
	register const ui4	*ui4_buf;
	
	
	if (globals_m11->CRC_table == NULL)  {
		if (CRC_initialize_tables_m11() == FALSE_m11) {
			error_message_m11("%s(): error\n", __FUNCTION__);
			return(0);
		}
	}
	
	crc_table = globals_m11->CRC_table;
	
	c = ~current_crc;
	
	// bring block_ptr to 4 byte alignment
	while (block_bytes && ((ui8) block_ptr & (ui8) 3)) {
		c = crc_table[0][(c ^ (ui4) *block_ptr++) & (ui4) 0xff] ^ (c >> 8);
		block_bytes--;
	}
	
	// calculate CRC in 32 byte chunks
	ui4_buf = (const ui4*) block_ptr;
	while (block_bytes >= 32) {
		c ^= *ui4_buf++;
		c = crc_table[3][c & 0xff] ^ crc_table[2][(c >> 8) & 0xff] ^ crc_table[1][(c >> 16) & 0xff] ^ crc_table[0][c >> 24];
		c ^= *ui4_buf++;
		c = crc_table[3][c & 0xff] ^ crc_table[2][(c >> 8) & 0xff] ^ crc_table[1][(c >> 16) & 0xff] ^ crc_table[0][c >> 24];
		c ^= *ui4_buf++;
		c = crc_table[3][c & 0xff] ^ crc_table[2][(c >> 8) & 0xff] ^ crc_table[1][(c >> 16) & 0xff] ^ crc_table[0][c >> 24];
		c ^= *ui4_buf++;
		c = crc_table[3][c & 0xff] ^ crc_table[2][(c >> 8) & 0xff] ^ crc_table[1][(c >> 16) & 0xff] ^ crc_table[0][c >> 24];
		c ^= *ui4_buf++;
		c = crc_table[3][c & 0xff] ^ crc_table[2][(c >> 8) & 0xff] ^ crc_table[1][(c >> 16) & 0xff] ^ crc_table[0][c >> 24];
		c ^= *ui4_buf++;
		c = crc_table[3][c & 0xff] ^ crc_table[2][(c >> 8) & 0xff] ^ crc_table[1][(c >> 16) & 0xff] ^ crc_table[0][c >> 24];
		c ^= *ui4_buf++;
		c = crc_table[3][c & 0xff] ^ crc_table[2][(c >> 8) & 0xff] ^ crc_table[1][(c >> 16) & 0xff] ^ crc_table[0][c >> 24];
		c ^= *ui4_buf++;
		c = crc_table[3][c & 0xff] ^ crc_table[2][(c >> 8) & 0xff] ^ crc_table[1][(c >> 16) & 0xff] ^ crc_table[0][c >> 24];
		block_bytes -= 32;
	}
	
	// process remaining bytes in 4 byte chunks
	while (block_bytes >= 4) {
		c ^= *ui4_buf++;
		c = crc_table[3][c & 0xff] ^ crc_table[2][(c >> 8) & 0xff] ^ crc_table[1][(c >> 16) & 0xff] ^ crc_table[0][c >> 24];
		block_bytes -= 4;
	}
	block_ptr = (const ui1 *) ui4_buf;
	
	// process remaining bytes as single bytes
	while (block_bytes--)
		c = crc_table[0][(c ^ (ui4) *block_ptr++) & (ui4) 0xff] ^ (c >> 8);
	
	return(~c);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
TERN_m11	CRC_validate_m11(const ui1 *block_ptr, si8 block_bytes, ui4 crc_to_validate)
{
	ui4	crc;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	crc = CRC_calculate_m11(block_ptr, block_bytes);
	
	if (crc == crc_to_validate)
		return(TRUE_m11);
	
	return(FALSE_m11);
}



//***********************************************************************//
//************  FILE PROCESSING STRUCT STANDARD FUNCTIONS  **************//
//***********************************************************************//


FILE_PROCESSING_STRUCT_m11	*FPS_allocate_processing_struct_m11(FILE_PROCESSING_STRUCT_m11 *fps, si1 *full_file_name, ui4 type_code, si8 raw_data_bytes, LEVEL_HEADER_m11 *parent, FILE_PROCESSING_STRUCT_m11 *proto_fps, si8 bytes_to_copy)
{
	TERN_m11			free_fps;
	UNIVERSAL_HEADER_m11		*uh;
	CMP_PROCESSING_STRUCT_m11	*cps;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// allocate FPS
	free_fps = FALSE_m11;
	if (fps == NULL) {
		fps = (FILE_PROCESSING_STRUCT_m11 *) calloc_m11((size_t) 1, sizeof(FILE_PROCESSING_STRUCT_m11), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
		free_fps = TRUE_m11;
	} else if (fps->parameters.raw_data != NULL) {
		free_m11((void *) fps->parameters.raw_data, __FUNCTION__);
		fps->parameters.raw_data = NULL;
	}
	if (parent != NULL)
		fps->parent = (void *) parent;
	if (full_file_name != NULL)
		if (*full_file_name)
			strncpy_m11(fps->full_file_name, full_file_name, FULL_FILE_NAME_BYTES_m11);
	if (*fps->full_file_name && type_code == UNKNOWN_TYPE_CODE_m11)
		type_code = MED_type_code_from_string_m11(fps->full_file_name);

	// allocate raw_data
	(void) FPS_initialize_parameters_m11(&fps->parameters);
	if (raw_data_bytes == FPS_FULL_FILE_m11) {  // use this to allocate a memory mapped file also
		fps->parameters.raw_data_bytes = raw_data_bytes = file_length_m11(NULL, fps->full_file_name);
	} else {  // all files start with universal header
		if (raw_data_bytes == FPS_UNIVERSAL_HEADER_ONLY_m11)
			fps->parameters.raw_data_bytes = raw_data_bytes = UNIVERSAL_HEADER_BYTES_m11;
		else
			fps->parameters.raw_data_bytes = (raw_data_bytes += UNIVERSAL_HEADER_BYTES_m11);
	}
	fps->parameters.raw_data = (ui1 *) calloc_m11((size_t) raw_data_bytes, sizeof(ui1), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	uh = fps->universal_header = (UNIVERSAL_HEADER_m11 *) fps->parameters.raw_data;
	fps->number_of_items = 0;

	// if a prototype FILE_PROCESSING_STRUCT_m11 was passed - copy its directives, password data, universal_header, and raw data
	if (proto_fps != NULL) {
		fps->directives = proto_fps->directives;
		fps->parameters.password_data = proto_fps->parameters.password_data;
		bytes_to_copy += UNIVERSAL_HEADER_BYTES_m11;  // all files start with universal header
		if ((bytes_to_copy > proto_fps->parameters.raw_data_bytes) || (bytes_to_copy > raw_data_bytes))
			error_message_m11("%s(): copy request size exceeds available data or space => no copying done\n", __FUNCTION__);
		else
			memcpy(fps->parameters.raw_data, proto_fps->parameters.raw_data, bytes_to_copy);
		uh->type_code = type_code;
		uh->header_CRC = uh->body_CRC = CRC_START_VALUE_m11;
		uh->number_of_entries = 0;
		uh->maximum_entry_size = 0;
	} else {
		(void) FPS_initialize_directives_m11(&fps->directives);  // set directives to defaults
		initialize_universal_header_m11(fps, type_code, FALSE_m11, FALSE_m11);
	}
	generate_UID_m11(&uh->file_UID);
	uh->provenance_UID = uh->file_UID;  // if not originating file, caller should change provenance_UID to file_UID of originating file
	if (fps->parameters.password_data == NULL)
		fps->parameters.password_data = &globals_m11->password_data;

	// set appropriate pointers (also set maximum entry size where it's a fixed value)
	fps->data_pointers = fps->parameters.raw_data + UNIVERSAL_HEADER_BYTES_m11;
	switch (type_code) {
		case TIME_SERIES_INDICES_FILE_TYPE_CODE_m11:
			uh->maximum_entry_size = TIME_SERIES_INDEX_BYTES_m11;
			break;
		case TIME_SERIES_DATA_FILE_TYPE_CODE_m11:
			cps = fps->parameters.cps;
			if (cps != NULL) {
				cps->block_header = (CMP_BLOCK_FIXED_HEADER_m11 *) fps->data_pointers;
				cps->parameters.allocated_compressed_bytes = raw_data_bytes - UNIVERSAL_HEADER_BYTES_m11;
			}
			break;
		case TIME_SERIES_METADATA_FILE_TYPE_CODE_m11:
		case VIDEO_METADATA_FILE_TYPE_CODE_m11:
			uh->maximum_entry_size = METADATA_BYTES_m11;
			uh->number_of_entries = 1;
			break;
		case VIDEO_INDICES_FILE_TYPE_CODE_m11:
			uh->maximum_entry_size = VIDEO_INDEX_BYTES_m11;
			break;
		case RECORD_DATA_FILE_TYPE_CODE_m11:
			break;
		case RECORD_INDICES_FILE_TYPE_CODE_m11:
			uh->maximum_entry_size = RECORD_INDEX_BYTES_m11;
			break;
		default:
			if (free_fps == TRUE_m11)
				FPS_free_processing_struct_m11(fps, TRUE_m11);
			error_message_m11("%s(): unrecognized type code (code = 0x%08x)\n", type_code, __FUNCTION__);
			return(NULL);
	}

	return(fps);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void	FPS_close_m11(FILE_PROCESSING_STRUCT_m11 *fps) {
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (fps->parameters.fp != NULL) {
		fclose(fps->parameters.fp);
		fps->parameters.fp = NULL;
	}
	fps->parameters.fd = FPS_FD_CLOSED_m11;
	fps->parameters.fpos = 0;
	
	// leave flen intact

	return;
}


si4	FPS_compare_start_times_m11(const void *a, const void *b)
{
	si8				a_start_time, b_start_time;
	FILE_PROCESSING_STRUCT_m11	*fps;
	
	
	fps = (FILE_PROCESSING_STRUCT_m11 *) *((FILE_PROCESSING_STRUCT_m11 **) a);
	a_start_time = fps->universal_header->segment_start_time;

	fps = (FILE_PROCESSING_STRUCT_m11 *)  *((FILE_PROCESSING_STRUCT_m11 **) a);
	b_start_time = fps->universal_header->segment_start_time;

	// qsort() requires an si4 return value, so can't just subtract
	if (a_start_time > b_start_time)
		return((si4) 1);
	if (a_start_time < b_start_time)
		return((si4) -1);
	return((si4) 0);
}


void	FPS_free_processing_struct_m11(FILE_PROCESSING_STRUCT_m11 *fps, TERN_m11 free_fps_structure)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (fps == NULL) {
		warning_message_m11("%s(): trying to free a NULL FILE_PROCESSING_STRUCT_m11 => returning with no action\n", __FUNCTION__);
		return;
	}
	
	if (fps->universal_header != NULL) {
		if (fps->universal_header->type_code == TIME_SERIES_DATA_FILE_TYPE_CODE_m11)  // CPS requires special freeing
			if (fps->parameters.cps != NULL)
				if (fps->directives.free_CMP_processing_struct == TRUE_m11)
					CMP_free_processing_struct_m11(fps->parameters.cps, TRUE_m11);
	}
	
	if (fps->parameters.raw_data != NULL)
		free_m11((void *) fps->parameters.raw_data, __FUNCTION__);
	
	if (fps->directives.free_password_data == TRUE_m11)
		if (fps->parameters.password_data != &globals_m11->password_data && fps->parameters.password_data != NULL)
			free_m11((void *) fps->parameters.password_data, __FUNCTION__);
	
	if (fps->parameters.mmap_block_bitmap != NULL)
		free_m11((void *) fps->parameters.mmap_block_bitmap, __FUNCTION__);
	
	// Note: always close when freeing; close_file directives used in reading / writing functions
	FPS_close_m11(fps);  // if already closed, this fails silently
	
	if (free_fps_structure == TRUE_m11) {
		free_m11((void *) fps, __FUNCTION__);
	} else {
		// leave full_file_name intact
		fps->parameters.last_access_time = UUTC_NO_ENTRY_m11;
		fps->parameters.cps = NULL;
		if (fps->parameters.password_data != &globals_m11->password_data)  // if points to global password data, leave intact for re-use
			fps->parameters.password_data = NULL;
		fps->universal_header = NULL;
		fps->data_pointers = NULL;  // Note: if free_CMP_processing_struct == FALSE_m11, this pointer is still set to NULL => assumes cps address is also stored elsewhere
		fps->parameters.raw_data_bytes = 0;
		fps->parameters.raw_data = NULL;
		fps->parameters.mmap_block_bytes = 0;
		fps->parameters.mmap_number_of_blocks = 0;
		fps->parameters.mmap_block_bitmap = NULL;
	}

	return;
}


FPS_DIRECTIVES_m11	*FPS_initialize_directives_m11(FPS_DIRECTIVES_m11 *directives)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (directives == NULL)
		directives = (FPS_DIRECTIVES_m11 *) calloc_m11((size_t) 1, sizeof(FPS_DIRECTIVES_m11), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	
	// set directives to defaults
	directives->close_file = FPS_DIRECTIVES_CLOSE_FILE_DEFAULT_m11;
	directives->flush_after_write = FPS_DIRECTIVES_FLUSH_AFTER_WRITE_DEFAULT_m11;
	directives->update_universal_header = FPS_DIRECTIVES_UPDATE_UNIVERSAL_HEADER_DEFAULT_m11;
	directives->leave_decrypted = FPS_DIRECTIVES_LEAVE_DECRYPTED_DEFAULT_m11;
	directives->free_password_data = FPS_DIRECTIVES_FREE_PASSWORD_DATA_DEFAULT_m11;
	directives->free_CMP_processing_struct = FPS_DIRECTIVES_FREE_CMP_PROCESSING_STRUCT_DEFAULT_m11;
	directives->lock_mode = FPS_DIRECTIVES_LOCK_MODE_DEFAULT_m11;
	directives->open_mode = FPS_DIRECTIVES_OPEN_MODE_DEFAULT_m11;
	directives->memory_map = FPS_DIRECTIVES_MEMORY_MAP_DEFAULT_m11;

	
	return(directives);
}


FPS_PARAMETERS_m11	*FPS_initialize_parameters_m11(FPS_PARAMETERS_m11 *parameters)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (parameters == NULL)
		parameters = (FPS_PARAMETERS_m11 *) calloc_m11((size_t) 1, sizeof(FPS_PARAMETERS_m11), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	
	// set parameters to defaults
	parameters->last_access_time = UUTC_NO_ENTRY_m11;
	parameters->full_file_read = FALSE_m11;
	parameters->raw_data_bytes = 0;
	parameters->raw_data = NULL;
	parameters->password_data = &globals_m11->password_data;
	parameters->cps = NULL;
	parameters->fd = FPS_FD_NO_ENTRY_m11;
	parameters->fp = NULL;
	parameters->fpos = 0;
	parameters->flen = 0;
	parameters->mmap_block_bytes = 0;
	parameters->mmap_number_of_blocks = 0;
	parameters->mmap_block_bitmap = NULL;
	pthread_mutex_init_m11(&parameters->mutex, NULL);

	return(parameters);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
TERN_m11	FPS_lock_m11(FILE_PROCESSING_STRUCT_m11 *fps, si4 lock_type, const si1 *function, ui4 behavior_on_fail)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
#if defined MACOS_m11 || defined LINUX_m11
	struct flock	fl;
	
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m11)
		behavior_on_fail = globals_m11->behavior_on_fail;
	
	fl.l_type = lock_type;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 0;
	fl.l_pid = getpid();
	if (fcntl(fps->parameters.fd, F_SETLKW, &fl) == -1) {
		error_message_m11("%s(): fcntl() failed to lock file\n\tsystem error: %s (# %d)\n\tcalled from function %s()\n", __FUNCTION__, strerror(errno), errno, function);
		return(-1);
	}
#endif
	
	return(TRUE_m11);
}


si8	FPS_memory_map_read_m11(FILE_PROCESSING_STRUCT_m11 *fps, si8 file_offset, si8 bytes_to_read, const si1 *function, ui4 behavior_on_fail)
{
	ui1		mode;
	const ui1	SKIP_MODE = 0, READ_MODE = 1;
	ui4		start_block, end_block;
	ui8		bit_mask, *bit_word, read_start, read_bytes;
	si8		i, remaining_bytes, block_bytes;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (bytes_to_read == 0)
		return(TRUE_m11);  // didn't fail, just nothing to do
	file_offset = REMOVE_DISCONTINUITY_m11(file_offset);

	if (bytes_to_read == FPS_UNIVERSAL_HEADER_ONLY_m11) {
		bytes_to_read = UNIVERSAL_HEADER_BYTES_m11;
		file_offset = 0;
	}

	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m11)
		behavior_on_fail = globals_m11->behavior_on_fail;
	
	remaining_bytes = fps->parameters.flen - file_offset;
	if (bytes_to_read > remaining_bytes)
		bytes_to_read = remaining_bytes;
		
	block_bytes = (si8) fps->parameters.mmap_block_bytes;
	start_block = file_offset / block_bytes;
	end_block = (file_offset + bytes_to_read - 1) / block_bytes;
	bit_mask = 1 << (start_block % 64);
	bit_word = fps->parameters.mmap_block_bitmap + (start_block >> 6);
	file_offset = start_block * block_bytes;
	
	if (*bit_word & bit_mask) {
		mode = SKIP_MODE;
	} else {
		mode = READ_MODE;
		read_start = file_offset;
	}
	for (i = start_block; i < end_block; ++i) {
		if (*bit_word & bit_mask) {  // block already read
			if (mode) {  // switch READ_MODE to SKIP_MODE: read unread blocks up to here
				read_bytes = file_offset - read_start;
				FPS_seek_m11(fps, read_start);
				fread_m11((void *) (fps->parameters.raw_data + read_start), (size_t) 1, (size_t) read_bytes, fps->parameters.fp, fps->full_file_name, function, behavior_on_fail);
				mode = SKIP_MODE;
			}
		} else {  // block not yet read
			if (!mode) {  // switch SKIP_MODE to READ_MODE: mark read start
				read_start = file_offset;
				mode = READ_MODE;
			}
			*bit_word |= bit_mask;  // block will be read, mark now
		}
		file_offset += block_bytes;
		if (!(bit_mask <<= 1)) {
			++bit_word;
			bit_mask = 1;
		}
	}
	
	// final read
	if (*bit_word & bit_mask) {  // block already read
		if (mode)  // READ_MODE
			read_bytes = file_offset - read_start;
		else  // SKIP_MODE
			read_bytes = 0;
	} else {  // block not yet read
		if (mode) {  // READ_MODE
			read_bytes = fps->parameters.flen - read_start;
		} else {  // SKIP_MODE
			read_start = file_offset;
			read_bytes = fps->parameters.flen - file_offset;
		}
		*bit_word |= bit_mask;  // mark block as read
	}
	if (read_bytes) {
		FPS_seek_m11(fps, read_start);
		fread_m11((void *) (fps->parameters.raw_data + read_start), (size_t) 1, (size_t) read_bytes, fps->parameters.fp, fps->full_file_name, function, behavior_on_fail);
	}
	fps->parameters.fpos = read_start + read_bytes;

	return(bytes_to_read);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void FPS_mutex_off_m11(FILE_PROCESSING_STRUCT_m11 *fps)
{
	pthread_mutex_unlock_m11(&fps->parameters.mutex);
	
	return;
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void FPS_mutex_on_m11(FILE_PROCESSING_STRUCT_m11 *fps)
{
	pthread_mutex_lock_m11(&fps->parameters.mutex);

	return;
}


TERN_m11	FPS_open_m11(FILE_PROCESSING_STRUCT_m11 *fps, const si1 *function, ui4 behavior_on_fail)
{
	TERN_m11	create_file = FALSE_m11;
	si1		*mode, path[FULL_FILE_NAME_BYTES_m11], command[FULL_FILE_NAME_BYTES_m11 + 16];
	si1		name[BASE_FILE_NAME_BYTES_m11], extension[TYPE_BYTES_m11];
	static ui4	create_modes = (ui4) (FPS_R_PLUS_OPEN_MODE_m11 | FPS_W_OPEN_MODE_m11 | FPS_W_PLUS_OPEN_MODE_m11 | FPS_A_OPEN_MODE_m11 | FPS_A_PLUS_OPEN_MODE_m11);
	si4		lock_type;
#if defined MACOS_m11 || defined LINUX_m11
	struct stat	sb;
#endif
#ifdef WINDOWS_m11
	struct _stat64	sb;
	HANDLE		file_h;
	DISK_GEOMETRY	disk_geom = { 0 };
	ui4		dg_result;
#endif

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m11)
		behavior_on_fail = globals_m11->behavior_on_fail;
	
	if (fps->directives.open_mode & create_modes)
		create_file = TRUE_m11;
	
	// open
	mode = NULL;
	switch (fps->directives.open_mode) {
		case FPS_R_OPEN_MODE_m11:
			mode = "r";
			break;
		case FPS_R_PLUS_OPEN_MODE_m11:
			mode = "r+";
			break;
		case FPS_W_OPEN_MODE_m11:
			mode = "w";
			break;
		case FPS_W_PLUS_OPEN_MODE_m11:
			mode = "w+";
			break;
		case FPS_A_OPEN_MODE_m11:
			mode = "a";
			break;
		case FPS_A_PLUS_OPEN_MODE_m11:
			mode = "a+";
			break;
		case FPS_NO_OPEN_MODE_m11:
		default:
			error_message_m11("%s(): invalid open mode (%u)\n\tcalled from function %s()\n", __FUNCTION__, fps->directives.open_mode, function);
			return(FALSE_m11);
	}
	
	fps->parameters.fp = fopen_m11(fps->full_file_name, mode, function, RETURN_ON_FAIL_m11 | SUPPRESS_ERROR_OUTPUT_m11);
	if (fps->parameters.fp == NULL && errno == ENOENT && create_file == TRUE_m11) {
		// A component of the required directory tree does not exist - build it & try again
		extract_path_parts_m11(fps->full_file_name, path, name, extension);
#if defined MACOS_m11 || defined LINUX_m11
		sprintf_m11(command, "mkdir -p \"%s\"", path);
#endif
#ifdef WINDOWS_m11
		sprintf_m11(command, "mkdir \"%s\"", path);
#endif
		system_m11(command, TRUE_m11, __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
		fps->parameters.fp = fopen_m11(fps->full_file_name, mode, function, behavior_on_fail);
	}
	if (fps->parameters.fp == NULL) {
		error_message_m11("%s(): failed to open file \"%s\"\n\tcalled from function %s()\n", __FUNCTION__, fps->full_file_name, function);
		return(-1);
	}
	
	// file descriptor & file length
#if defined MACOS_m11 || defined LINUX_m11
	fps->parameters.fd = fileno(fps->parameters.fp);
	fstat(fps->parameters.fd, &sb);
#endif
#ifdef WINDOWS_m11
	fps->parameters.fd = _fileno(fps->parameters.fp);
	_fstat64(fps->parameters.fd, &sb);
#endif
	fps->parameters.flen = sb.st_size;
	fps->parameters.fpos = 0;

	// memory mapping
	if (fps->directives.memory_map == TRUE_m11) {
		if (globals_m11->mmap_block_bytes == GLOBALS_MMAP_BLOCK_BYTES_NO_ENTRY_m11) {	// save value as global so don't do this for every mem_map open
#if defined MACOS_m11 || defined LINUX_m11							// (assumes session files all on same file system)
			fps->parameters.mmap_block_bytes = (ui4) sb.st_blksize;
#endif
#ifdef WINDOWS_m11
			if ((file_h = (HANDLE) _get_osfhandle(fps->parameters.fd)) != INVALID_HANDLE_VALUE) {
				dg_result = (ui4) DeviceIoControl(file_h, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &disk_geom, sizeof(DISK_GEOMETRY), &dg_result, (LPOVERLAPPED) NULL);
				if (dg_result == 1)
					fps->parameters.mmap_block_bytes = (ui4) disk_geom.BytesPerSector;
			}
#endif
			if (fps->parameters.mmap_block_bytes <= 0)
				fps->parameters.mmap_block_bytes = GLOBALS_MMAP_BLOCK_BYTES_DEFAULT_m11;
			globals_m11->mmap_block_bytes = fps->parameters.mmap_block_bytes;
			fps->parameters.mmap_number_of_blocks = (ui4) ((fps->parameters.flen + (si8) (fps->parameters.mmap_block_bytes - 1)) / (si8) fps->parameters.mmap_block_bytes);
			fps->parameters.mmap_block_bitmap = (ui8 *) calloc_m11((size_t) ((fps->parameters.mmap_number_of_blocks + 63) / 64), sizeof(ui8), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
		} else {
			fps->parameters.mmap_block_bytes = globals_m11->mmap_block_bytes;
		}
	}
	
	// lock
#if defined MACOS_m11 || defined LINUX_m11
	if (fps->directives.lock_mode != FPS_NO_LOCK_MODE_m11) {
		lock_type = FPS_NO_LOCK_TYPE_m11;
		if (fps->directives.open_mode == FPS_R_OPEN_MODE_m11) {
			if (fps->directives.lock_mode & FPS_READ_LOCK_ON_READ_OPEN_m11)
				lock_type = F_RDLCK;
			else if (fps->directives.lock_mode & FPS_WRITE_LOCK_ON_READ_OPEN_m11)
				lock_type = F_WRLCK;
		} else if (fps->directives.lock_mode & (FPS_WRITE_LOCK_ON_WRITE_OPEN_m11 | FPS_WRITE_LOCK_ON_READ_WRITE_OPEN_m11)) {
			lock_type = F_WRLCK;
		} else {
			error_message_m11("%s(): incompatible lock (%u) and open (%u) modes\n\tcalled from function %s()\n", __FUNCTION__, fps->directives.lock_mode, fps->directives.open_mode, function);
			return(-1);
		}
		FPS_lock_m11(fps, lock_type, function, behavior_on_fail);
	}
#endif

	fps->parameters.last_access_time = current_uutc_m11();
	
	return(TRUE_m11);
}


si8	FPS_read_m11(FILE_PROCESSING_STRUCT_m11 *fps, si8 file_offset, si8 bytes_to_read, const si1 *function, ui4 behavior_on_fail)
{
	void	*data_ptr;
	si8	bytes_read, bytes_remaining;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// mutex on
	FPS_mutex_on_m11(fps);
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m11)
		behavior_on_fail = globals_m11->behavior_on_fail;
	file_offset = REMOVE_DISCONTINUITY_m11(file_offset);

#if defined MACOS_m11 || defined LINUX_m11
	// lock
	if (fps->directives.lock_mode & FPS_READ_LOCK_ON_READ_m11)
		FPS_lock_m11(fps, F_RDLCK, function, behavior_on_fail);
#endif
	// read
	if (file_offset == 0)
		data_ptr = (void *) fps->universal_header;
	else
		data_ptr = (void *) fps->data_pointers;
	
	bytes_remaining = fps->parameters.flen - file_offset;
	if (bytes_to_read > bytes_remaining)
		bytes_to_read = bytes_remaining;
	
	if (fps->directives.memory_map == TRUE_m11) {
		bytes_read = FPS_memory_map_read_m11(fps, file_offset, bytes_to_read, function, behavior_on_fail);
	} else {
		FPS_seek_m11(fps, file_offset);
		bytes_read = fread_m11(data_ptr, sizeof(ui1), (size_t) bytes_to_read, fps->parameters.fp, fps->full_file_name, function, behavior_on_fail);
	}

#if defined MACOS_m11 || defined LINUX_m11
	// unlock
	if (fps->directives.lock_mode & FPS_READ_LOCK_ON_READ_m11)
		FPS_unlock_m11(fps, function, behavior_on_fail);
#endif
	
	// update parameters
	fps->parameters.fpos = file_offset + bytes_read;
	fps->parameters.last_access_time = current_uutc_m11();
	
	// mutex on
	FPS_mutex_off_m11(fps);
			
	return(bytes_read);
}


TERN_m11	FPS_reallocate_processing_struct_m11(FILE_PROCESSING_STRUCT_m11 *fps, si8 new_raw_data_bytes)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (new_raw_data_bytes <= fps->parameters.raw_data_bytes)
		return(TRUE_m11);
		
	// reallocate
	fps->parameters.raw_data = (ui1 *) realloc_m11((void *) fps->parameters.raw_data, (size_t) new_raw_data_bytes, __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	
	// zero additional memory (realloc() copies existing memory if necessary, but does not zero additional memory allocated)
	if (new_raw_data_bytes > fps->parameters.raw_data_bytes)
		memset(fps->parameters.raw_data + fps->parameters.raw_data_bytes, 0, new_raw_data_bytes - fps->parameters.raw_data_bytes);
	fps->parameters.raw_data_bytes = new_raw_data_bytes;
		
	// reset fps pointers
	fps->universal_header = (UNIVERSAL_HEADER_m11 *) fps->parameters.raw_data; // all files start with universal header
	fps->data_pointers = fps->parameters.raw_data + UNIVERSAL_HEADER_BYTES_m11;
	if (fps->universal_header->type_code == TIME_SERIES_DATA_FILE_TYPE_CODE_m11 && fps->parameters.cps != NULL) {
		fps->parameters.cps->block_header = (CMP_BLOCK_FIXED_HEADER_m11 *) fps->data_pointers;
		fps->parameters.cps->parameters.allocated_compressed_bytes = new_raw_data_bytes - UNIVERSAL_HEADER_BYTES_m11;
	}
	
	return(TRUE_m11);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void	FPS_seek_m11(FILE_PROCESSING_STRUCT_m11 *fps, si8 file_offset)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	file_offset = REMOVE_DISCONTINUITY_m11(file_offset);
	if (fps->parameters.fpos == file_offset)
		return;
	
	fseek_m11(fps->parameters.fp, file_offset, SEEK_SET, fps->full_file_name, __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	fps->parameters.fpos = file_offset;
	
	return;
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void	FPS_set_pointers_m11(FILE_PROCESSING_STRUCT_m11 *fps, si8 file_offset)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (fps->parameters.full_file_read == TRUE_m11 || fps->directives.memory_map == TRUE_m11)
		fps->data_pointers = fps->parameters.raw_data + REMOVE_DISCONTINUITY_m11(file_offset);
	else
		fps->data_pointers = fps->parameters.raw_data + UNIVERSAL_HEADER_BYTES_m11;  // file_offset irrelevant

	if (fps->parameters.cps != NULL)
		fps->parameters.cps->block_header = (CMP_BLOCK_FIXED_HEADER_m11 *) fps->data_pointers;
	
	return;
}


void	FPS_show_processing_struct_m11(FILE_PROCESSING_STRUCT_m11 *fps)
{
	si1	hex_str[HEX_STRING_BYTES_m11(TYPE_STRLEN_m11)], time_str[TIME_STRING_BYTES_m11], *s;
	si4	i;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	printf_m11("----------- File Processing Structure - START ----------\n");
	UTF8_printf_m11("Full File Name: %s\n", fps->full_file_name);
	printf_m11("Full File Read: ");
	if (fps->parameters.full_file_read == TRUE_m11)
		printf_m11("true\n");
	else if (fps->parameters.full_file_read == FALSE_m11)
		printf_m11("false\n");
	else
		printf_m11("unknown\n");
	if (fps->parameters.last_access_time == UUTC_NO_ENTRY_m11) {
		printf_m11("Last Access Time: no entry\n");
	} else {
		time_string_m11(fps->parameters.last_access_time, time_str, FALSE_m11, FALSE_m11, FALSE_m11);
		printf_m11("Last Access Time: %ld (µUTC), %s\n", fps->parameters.last_access_time, time_str);
	}
	if (fps->parameters.fd >= 3)
		printf_m11("File Descriptor: %d (open)\n", fps->parameters.fd);
	else if (fps->parameters.fd == -1)
		printf_m11("File Descriptor: %d (closed)\n", fps->parameters.fd);
	else if (fps->parameters.fd == FPS_FD_NO_ENTRY_m11)
		printf_m11("File Descriptor: %d (not yet opened)\n", fps->parameters.fd);
	else if (fps->parameters.fd == FPS_FD_EPHEMERAL_m11)
		printf_m11("File Descriptor: %d (ephemeral)\n", fps->parameters.fd);
	else    // stdin == 0, stdout == 1, stderr == 2
		printf_m11("File Descriptor: %d (standard stream: invalid)\n", fps->parameters.fd);
	printf_m11("File Length: ");
	if (fps->parameters.flen == FPS_FILE_LENGTH_UNKNOWN_m11)
		printf_m11("unknown\n");
	else
		printf_m11("%ld\n", fps->parameters.flen);
	s = (si1 *) &fps->universal_header->type_code;
	generate_hex_string_m11((ui1 *) s, TYPE_STRLEN_m11, hex_str);
	printf_m11("File Type Code: %s    (", hex_str);
	for (i = 0; i < 4; ++i)
		printf_m11(" %c ", *s++);
	printf_m11(")\n");
	printf_m11("Raw Data Bytes: %ld\n", fps->parameters.raw_data_bytes);
	show_universal_header_m11(fps, NULL);
	if (fps->parameters.raw_data_bytes > UNIVERSAL_HEADER_BYTES_m11) {
		switch (fps->universal_header->type_code) {
			case TIME_SERIES_METADATA_FILE_TYPE_CODE_m11:
			case VIDEO_METADATA_FILE_TYPE_CODE_m11:
				show_metadata_m11(fps, NULL, 0);
				break;
			case RECORD_DATA_FILE_TYPE_CODE_m11:
				show_records_m11(fps, NULL);
				break;
			default:
				break;
		}
	}
	if (fps->directives.memory_map == TRUE_m11) {
		printf_m11("Memory Mapping:\n");
		printf_m11("\tBlock Size: %u\n", fps->parameters.mmap_block_bytes);
		printf_m11("\tNumber of Blocks: %u\n", fps->parameters.mmap_number_of_blocks);
	}
	printf_m11("------------ File Processing Structure - END -----------\n\n");
	
	return;
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void	FPS_sort_m11(FILE_PROCESSING_STRUCT_m11 **fps_array, si4 n_fps)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// input must be 2D FPS array, such as allocated by calloc_2D_m11()
	// sorts the pointers by FPS file start time, does not move the FPSs
	
	qsort((void *) fps_array, (size_t) n_fps, sizeof(FILE_PROCESSING_STRUCT_m11 *), FPS_compare_start_times_m11);

	return;
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4	FPS_unlock_m11(FILE_PROCESSING_STRUCT_m11 *fps, const si1 *function, ui4 behavior_on_fail)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
#if defined MACOS_m11 || defined LINUX_m11
	struct flock	fl;
	
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m11)
		behavior_on_fail = globals_m11->behavior_on_fail;
	
	fl.l_type = F_UNLCK;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 0;
	fl.l_pid = getpid();
	if (fcntl(fps->parameters.fd, F_SETLKW, &fl) == -1) {
		error_message_m11("%s(): fcntl() failed to unlock file\n\tsystem error: %s (# %d)\n\tcalled from function %s()\n", __FUNCTION__, strerror(errno), errno, function);
		return(-1);
	}
#endif
	
	return(0);
}



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


void	SHA_finalize_m11(SHA_CTX_m11 *ctx, ui1 *hash)
{
	ui4	i;
	
	
	i = ctx->datalen;

	// Pad whatever data is left in the buffer.
	if (ctx->datalen < 56) {
		ctx->data[i++] = 0x80;
		while (i < 56)
			ctx->data[i++] = 0x00;
	}
	else {
		ctx->data[i++] = 0x80;
		while (i < 64)
			ctx->data[i++] = 0x00;
		SHA_transform_m11(ctx, ctx->data);
		memset(ctx->data, 0, 56);
	}

	// Append to the padding the total message's length in bits and transform.
	ctx->bitlen += ctx->datalen * 8;
	ctx->data[63] = ctx->bitlen;
	ctx->data[62] = ctx->bitlen >> 8;
	ctx->data[61] = ctx->bitlen >> 16;
	ctx->data[60] = ctx->bitlen >> 24;
	ctx->data[59] = ctx->bitlen >> 32;
	ctx->data[58] = ctx->bitlen >> 40;
	ctx->data[57] = ctx->bitlen >> 48;
	ctx->data[56] = ctx->bitlen >> 56;
	SHA_transform_m11(ctx, ctx->data);

	// Since this implementation uses little endian byte ordering and SHA uses big endian,
	// reverse all the bytes when copying the final state to the output hash.
	for (i = 0; i < 4; ++i) {
		hash[i]      = (ctx->state[0] >> (24 - i * 8)) & SHA_LOW_BYTE_MASK_m11;
		hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & SHA_LOW_BYTE_MASK_m11;
		hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & SHA_LOW_BYTE_MASK_m11;
		hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & SHA_LOW_BYTE_MASK_m11;
		hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & SHA_LOW_BYTE_MASK_m11;
		hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & SHA_LOW_BYTE_MASK_m11;
		hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & SHA_LOW_BYTE_MASK_m11;
		hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & SHA_LOW_BYTE_MASK_m11;
	}
	
	return;
}


ui1    *SHA_hash_m11(const ui1 *data, si8 len, ui1 *hash)
{
	SHA_CTX_m11         ctx;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (globals_m11->SHA_h0_table == NULL)  // all tables initialized together
		SHA_initialize_tables_m11();

	// if hash not passed, up to caller to free it
	if (hash == NULL)
		hash = (ui1 *) calloc_m11((size_t) SHA_HASH_BYTES_m11, sizeof(ui1), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	
	SHA_initialize_m11(&ctx);
	SHA_update_m11(&ctx, data, len);
	SHA_finalize_m11(&ctx, hash);
	
	return(hash);
}


void	SHA_initialize_m11(SHA_CTX_m11 *ctx)
{
	ui4	*SHA_h0;
	
	
	ctx->datalen = 0;
	ctx->bitlen = 0;
	
	SHA_h0 = globals_m11->SHA_h0_table;
	ctx->state[0] = SHA_h0[0];
	ctx->state[1] = SHA_h0[1];
	ctx->state[2] = SHA_h0[2];
	ctx->state[3] = SHA_h0[3];
	ctx->state[4] = SHA_h0[4];
	ctx->state[5] = SHA_h0[5];
	ctx->state[6] = SHA_h0[6];
	ctx->state[7] = SHA_h0[7];
	
	return;
}


TERN_m11	SHA_initialize_tables_m11(void)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif

	pthread_mutex_lock_m11(&globals_m11->SHA_mutex);

	// h0 table
	if (globals_m11->SHA_h0_table == NULL) {
		globals_m11->SHA_h0_table = (ui4 *) calloc_m11((size_t) SHA_H0_ENTRIES_m11, sizeof(ui4), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
		{
			ui4 temp[SHA_H0_ENTRIES_m11] = SHA_H0_m11;
			memcpy(globals_m11->SHA_h0_table, temp, SHA_H0_ENTRIES_m11 * sizeof(ui4));
		}
	}
	
	// k table
	if (globals_m11->SHA_k_table == NULL) {
		globals_m11->SHA_k_table = (ui4 *) calloc_m11((size_t) SHA_K_ENTRIES_m11, sizeof(ui4), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
		{
			ui4 temp[SHA_K_ENTRIES_m11] = SHA_K_m11;
			memcpy(globals_m11->SHA_k_table, temp, SHA_K_ENTRIES_m11 * sizeof(ui4));
		}
	}
	
	pthread_mutex_unlock_m11(&globals_m11->SHA_mutex);

	return(TRUE_m11);
}


void	SHA_transform_m11(SHA_CTX_m11 *ctx, const ui1 *data)
{
	ui4	a, b, c, d, e, f, g, h, i, j, t1, t2, m[64], *sha_k;

	
	for (i = 0, j = 0; i < 16; ++i, j += 4)
		m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
	for ( ; i < 64; ++i)
		m[i] = SHA_SIG1_m11(m[i - 2]) + m[i - 7] + SHA_SIG0_m11(m[i - 15]) + m[i - 16];

	a = ctx->state[0];
	b = ctx->state[1];
	c = ctx->state[2];
	d = ctx->state[3];
	e = ctx->state[4];
	f = ctx->state[5];
	g = ctx->state[6];
	h = ctx->state[7];
	
	sha_k = globals_m11->SHA_k_table;
	for (i = 0; i < 64; ++i) {
		t1 = h + SHA_EP1_m11(e) + SHA_CH_m11(e,f,g) + sha_k[i] + m[i];
		t2 = SHA_EP0_m11(a) + SHA_MAJ_m11(a,b,c);
		h = g;
		g = f;
		f = e;
		e = d + t1;
		d = c;
		c = b;
		b = a;
		a = t1 + t2;
	}

	ctx->state[0] += a;
	ctx->state[1] += b;
	ctx->state[2] += c;
	ctx->state[3] += d;
	ctx->state[4] += e;
	ctx->state[5] += f;
	ctx->state[6] += g;
	ctx->state[7] += h;
	
	return;
}


void	SHA_update_m11(SHA_CTX_m11 *ctx, const ui1 *data, si8 len)
{
	si8	i;
	
	
	for (i = 0; i < len; ++i) {
		ctx->data[ctx->datalen] = data[i];
		ctx->datalen++;
		if (ctx->datalen == 64) {
			SHA_transform_m11(ctx, ctx->data);
			ctx->bitlen += 512;
			ctx->datalen = 0;
		}
	}
	
	return;
}



//***********************************************************************//
//****************************  STR FUNCTIONS  **************************//
//***********************************************************************//

si4	STR_compare_m11(const void *a, const void *b)
{
	si1		*ap, *bp, ac, bc;
	
	
	// Sorting Rules:
	// ascii only
	// case insensitive, but in case of equivalence lower case precedes upper case (e.g. "abc.txt" before "Abc.txt"
	//  "." before <space> (e.g. "RawData.nrd" before "RawData 0001.nrd"
	
	ac = *(ap = *((si1 **) a));
	bc = *(bp = *((si1 **) b));

	while (ac && bc) {
		
		// letters
		if (ac >= 'A' && ac <= 'Z') {
			ac += ('a' - 'A');  // "promote" to lower case, so "_" precedes letters
		}
		if (bc >= 'A' && bc <= 'Z')
			bc += ('a' - 'A');  // "promote" to lower case, so "_" precedes letters
		
		// equal - go to next character
		if (ac == bc) {
			ac = *++ap;
			bc = *++bp;
			continue;
		}
		
		// change ascii <space> before "." to "." before <space>
		if (ac == 0x20 || bc == 0x20) {  // a or b is a space, not both (caught above) (0x20 == <space>)
			if (bc == '.')
				return(1);
			if (ac == '.')
				return(-1);
		}
		
		return((si4) ac - (si4) bc);
	}
	
	if (ac)  // b longer than a
		return(1);
	if (bc)  // a longer than b
		return(-1);
	
	// case-insensitive equal strings - sort by case
	--ap; --bp;
	while (*++ap && *++bp) {
		if (*ap != *bp)  // first case difference
			return((si4) *bp - (si4) *ap);  // lower before upper case
	}
	
	// identical strings
	return(0);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
TERN_m11	STR_contains_regex_m11(si1 *string)
{
	si1	c;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// NOT an exhaustive list of potential regex characters, just enough to know if regex is present
	
	if (string == NULL)
		return(FALSE_m11);
	
	while ((c = *string++)) {
		switch (c) {
			case '*':
			case '?':
			case '+':
			case '|':
			case '^':
			case '$':
			case '[':
			case '{':
				return(TRUE_m11);
		}
	}
	return(FALSE_m11);
}


si1	*STR_match_end_m11(si1 *pattern, si1 *buffer)
{
	// returns pointer to the character after the first match in the buffer, NULL if no match (assumes both pattern & buffer are zero-terminated)
	si4	pat_len, buf_len;
	si1	*pat_p, *buf_p;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	pat_len = strlen(pattern);
	buf_len = strlen(buffer);
	if (pat_len >= buf_len)
		return(NULL);
	
	do {
		pat_p = pattern;
		buf_p = buffer++;
		while (*buf_p++ == *pat_p++) {
			if (!*pat_p) {
				if (*buf_p)
					return(buf_p);
				else
					return(NULL);
			}
		}
	} while (*buf_p);
	
	return(NULL);
}


si1	*STR_match_line_end_m11(si1 *pattern, si1 *buffer)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// returns pointer to beginning of the line following the line with first match, NULL if no match (assumes both pattern & buffer are zero-terminated)
	
	buffer = STR_match_end_m11(pattern, buffer);
	if (buffer == NULL)
		return(NULL);
	
	while (*buffer != '\n' && *buffer != '\r' && *buffer != 0)
		++buffer;
	if (*buffer == 0)
		return(NULL);
	
	while (*buffer == '\n' || *buffer != '\r')
		++buffer;
	
	if (*buffer == 0)
		return(NULL);
	
	return(buffer);
}


si1	*STR_match_line_start_m11(si1 *pattern, si1 *buffer)
{
	si1	*buf_p;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// returns pointer to beginning of the line containing the first match, NULL if no match (assumes both pattern & buffer are zero-terminated)

	buf_p = STR_match_start_m11(pattern, buffer);
	if (buf_p == NULL)
		return(NULL);
	
	while (*buf_p != '\n' && buf_p != buffer)
		--buf_p;
	
	if (buf_p == buffer)
		return(buffer);
	
	return(++buf_p);
}


si1	*STR_match_start_m11(si1 *pattern, si1 *buffer)
{
	si4	pat_len, buf_len;
	si1	*pat_p, *buf_p;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// returns pointer to beginning of the first match in the buffer, NULL if no match (assumes both pattern & buffer are zero-terminated)

	pat_len = strlen(pattern);
	buf_len = strlen(buffer);
	if (pat_len > buf_len)
		return(NULL);
	
	do {
		pat_p = pattern;
		buf_p = buffer++;
		while (*buf_p++ == *pat_p++)
			if (!*pat_p)
				return(--buffer);
	} while (*buf_p);
	
	return(NULL);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void    STR_replace_char_m11(si1 c, si1 new_c, si1 *buffer)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// Note: does not handle UTF8 chars
	// Done in place
	
	if (buffer == NULL || c == 0)
		return;
	
	do {
		if (*buffer == c)
			*buffer = new_c;
	} while (*buffer++);
	
	return;
}


si1	*STR_replace_pattern_m11(si1 *pattern, si1 *new_pattern, si1 *buffer, TERN_m11 free_input_buffer)
{
	si1	*c, *last_c, *new_c, *c2, *new_buffer = NULL;
	si4	char_diff, extra_chars, matches;
	si8	len, pat_len, new_pat_len;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (pattern == NULL || new_pattern == NULL || buffer == NULL)
		return(buffer);
	if (*pattern == 0 || *buffer == 0)
		return(buffer);
	
	pat_len = strlen(pattern);
	new_pat_len = strlen(new_pattern);
	char_diff = new_pat_len - pat_len;
	
	matches = 0;
	c = buffer;
	while (1) {
		c = STR_match_end_m11(pattern, c);
		if (c == NULL)
			break;
		++matches;
	}
	if (!matches)
		return(buffer);
	
	extra_chars = matches * char_diff;
	len = strlen(buffer) + extra_chars + 1;  // extra byte for terminal zero
	new_buffer = (si1 *) calloc_m11((size_t)len, sizeof(ui1), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	strcpy(new_buffer, buffer);
	
	last_c = c = buffer;
	new_c = new_buffer;
	extra_chars = 0;
	while (1) {
		c = STR_match_start_m11(pattern, c);
		if (c == NULL)
			break;
		while (last_c < c)
			*new_c++ = *last_c++;
		c2 = new_pattern;
		while (*c2)
			*new_c++ = *c2++;
		last_c = (c += pat_len);
	}
	while (*last_c)
		*new_c++ = *last_c++;
	
	if (free_input_buffer == TRUE_m11)
		free_m11((void *) buffer, __FUNCTION__);
	
	return(new_buffer);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void	STR_sort_m11(si1 **string_array, si8 n_strings)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// input must be 2D string array, such as allocated by calloc_2D_m11()
	// sorts the pointers, does not move the strings
	
	qsort((void *) string_array, (size_t) n_strings, sizeof(si1 *), STR_compare_m11);
	
	return;
}


void    STR_strip_character_m11(si1 *s, si1 character)
{
	si1	*c1, *c2;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	c1 = c2 = s;
	while (*c2) {
		if (*c2 == character) {
			++c2;
			continue;
		}
		*c1++ = *c2++;
	}
	*c1 = 0;
	
	return;
}


void	STR_to_lower_m11(si1 *s)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	--s;
	while (*++s) {
		if (*s > 64 && *s < 91)
			*s += 32;
	}
	
	return;
}


void	STR_to_title_m11(si1 *s)
{
	TERN_m11	cap_mode;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// make all lower case
	STR_to_lower_m11(s);
	
	// capitalize first letter regardless of word
	if (*s > 96 && *s < 123)
		*s -= 32;
	
	cap_mode = FALSE_m11;
	while (*++s) {
		if (*s < 97 || *s > 122) {  // not a lower case letter
			if (*s == 32)  // space
				cap_mode = TRUE_m11;
			continue;
		}
		if (cap_mode == TRUE_m11) {
			switch (*s) {  // not exhaustive, but covers most cases
				case 'a':
					if (strncmp(s, "a ", 2) == 0) {
						++s;
						continue;
					}
					if (strncmp(s, "an ", 3) == 0) {
						s += 2;
						continue;
					}
					if (strncmp(s, "and ", 4) == 0) {
						s += 3;
						continue;
					}
					break;
				case 'b':
					if (strncmp(s, "but ", 4) == 0) {
						s += 3;
						continue;
					}
					if (strncmp(s, "by ", 3) == 0) {
						s += 2;
						continue;
					}
					break;
				case 'f':
					if (strncmp(s, "for ", 4) == 0) {
						s += 3;
						continue;
					}
					if (strncmp(s, "from ", 5) == 0) {
						s += 4;
						continue;
					}
					break;
				case 'i':
					if (strncmp(s, "if ", 3) == 0) {
						s += 2;
						continue;
					}
					break;
				case 'o':
					if (strncmp(s, "of ", 3) == 0) {
						s += 2;
						continue;
					}
					break;
				case 't':
					if (strncmp(s, "the ", 4) == 0) {
						s += 3;
						continue;
					}
					if (strncmp(s, "to ", 3) == 0) {
						s += 2;
						continue;
					}
					break;
				case 'w':
					if (strncmp(s, "with ", 5) == 0) {
						s += 4;
						continue;
					}
					if (strncmp(s, "within ", 7) == 0) {
						s += 6;
						continue;
					}
					if (strncmp(s, "without ", 8) == 0) {
						s += 7;
						continue;
					}
					break;
			}
			*s -= 32;
			cap_mode = FALSE_m11;
		}
	}
	
	return;
}


void	STR_to_upper_m11(si1 *s)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	--s;
	while (*++s) {
		if (*s > 96 && *s < 123)
			*s -= 32;
	}
	
	return;
}



//***********************************************************************//
//******************************  UTF-8 FUNCTIONS  **********************//
//***********************************************************************//

// ATTRIBUTION
//
// Basic UTF-8 manipulation routines
// by Jeff Bezanson
// placed in the public domain Fall 2005

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


// byte offset => char_num
#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4	UTF8_char_num_m11(si1 *s, si4 offset)
{
	si4	char_num = 0, offs = 0;
	
	
	while (offs < offset && s[offs]) {
		(void) (UTF8_ISUTF_m11(s[++offs]) || UTF8_ISUTF_m11(s[++offs]) || UTF8_ISUTF_m11(s[++offs]) || ++offs);
		char_num++;
	}
	
	return(char_num);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void	UTF8_dec_m11(si1 *s, si4 *i)
{
	(void) (UTF8_ISUTF_m11(s[--(*i)]) || UTF8_ISUTF_m11(s[--(*i)]) || UTF8_ISUTF_m11(s[--(*i)]) || --(*i));
	
	return;
}


si4	UTF8_escape_m11(si1 *buf, si4 sz, si1 *src, si4 escape_quotes)
{
	si4	c = 0, i = 0, amt;
	
	
	while (src[i] && c < sz) {
		if (escape_quotes && src[i] == '"') {
			amt = snprintf_m11(buf, sz - c, "\\\"");
			i++;
		} else {
			amt = UTF8_escape_wchar_m11(buf, sz - c, UTF8_next_char_m11(src, &i));
		}
		c += amt;
		buf += amt;
	}
	if (c < sz)
		*buf = '\0';
	
	return(c);
}


si4	UTF8_escape_wchar_m11(si1 *buf, si4 sz, ui4 ch)
{
	if (ch == L'\n')
		return(snprintf(buf, sz, "\\n"));
	else if (ch == L'\t')
		return(snprintf(buf, sz, "\\t"));
	else if (ch == L'\r')
		return(snprintf(buf, sz, "\\r"));
	else if (ch == L'\b')
		return(snprintf(buf, sz, "\\b"));
	else if (ch == L'\f')
		return(snprintf(buf, sz, "\\f"));
	else if (ch == L'\v')
		return(snprintf(buf, sz, "\\v"));
	else if (ch == L'\a')
		return(snprintf(buf, sz, "\\a"));
	else if (ch == L'\\')
		return(snprintf(buf, sz, "\\\\"));
	else if (ch < 32 || ch == 0x7f)
		return(snprintf(buf, sz, "\\x%hhX", (ui1) ch));
	else if (ch > 0xFFFF)
		return(snprintf(buf, sz, "\\U%.8X", (ui4) ch));
	else if (ch >= 0x80 && ch <= 0xFFFF)
		return(snprintf(buf, sz, "\\u%.4hX", (ui2) ch));
	
	return(snprintf(buf, sz, "%c", (si1) ch));
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4     UTF8_fprintf_m11(FILE *stream, si1 *fmt, ...)
{
	si4		sz;
	si1		*src;
	ui4		*w_cs;
	va_list		args;
	

	va_start(args, fmt);
	sz = vasprintf_m11(&src, fmt, args);
	va_end(args);
	
#ifdef MATLAB_m11
	if (stream == stderr || stream == stdout)
		mexPrintf("%s", src);
	else
		fprintf(stream, "%s", src);
	free((void *) src);
	return(sz);
#endif
	
#ifdef WINDOWS_m11
	fprintf(stream, "%s", src);
	free((void *) src);
	return(sz);
#endif

	w_cs = (ui4 *) calloc(sz + 1, sizeof(ui4));
	UTF8_to_ucs_m11(w_cs, sz + 1, src, sz);
	fprintf(stream, "%ls", (wchar_t *) w_cs);
	
	free((void *) src);
	free((void *) w_cs);
	
	return(sz);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4	UTF8_hex_digit_m11(si1 c)
{
	return((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void	UTF8_inc_m11(si1 *s, si4 *i)
{
	(void) (UTF8_ISUTF_m11(s[++(*i)]) || UTF8_ISUTF_m11(s[++(*i)]) || UTF8_ISUTF_m11(s[++(*i)]) || ++(*i));
}


TERN_m11	UTF8_initialize_tables_m11(void)
{
#ifdef FN_DEBUG_m11
	printf_m11("%s()\n", __FUNCTION__);
#endif

	pthread_mutex_lock_m11(&globals_m11->UTF8_mutex);

	// offsets table
	if (globals_m11->UTF8_offsets_table == NULL) {
		globals_m11->UTF8_offsets_table = (ui4 *) malloc((size_t) (UTF8_OFFSETS_TABLE_ENTRIES_m11 << 2));
		{
			ui4 temp[UTF8_OFFSETS_TABLE_ENTRIES_m11] = UTF8_OFFSETS_TABLE_m11;
			memcpy((void *) globals_m11->UTF8_offsets_table, (void *) temp, (size_t) (UTF8_OFFSETS_TABLE_ENTRIES_m11 << 2));
		}
	}
	
	// trailing bytes table
	if (globals_m11->UTF8_trailing_bytes_table == NULL) {
		globals_m11->UTF8_trailing_bytes_table = (si1 *) malloc((size_t) UTF8_TRAILING_BYTES_TABLE_ENTRIES_m11);
		{
			si1 temp[UTF8_TRAILING_BYTES_TABLE_ENTRIES_m11] = UTF8_TRAILING_BYTES_TABLE_m11;
			memcpy((void *) globals_m11->UTF8_trailing_bytes_table, (void *) temp, (size_t) UTF8_TRAILING_BYTES_TABLE_ENTRIES_m11);
		}
	}
	
	pthread_mutex_unlock_m11(&globals_m11->UTF8_mutex);

	return(TRUE_m11);
}


si4     UTF8_is_locale_utf8_m11(si1 *locale)
{
	// this code based on libutf8
	const si1	*cp = locale;
	

	for (; *cp != '\0' && *cp != '@' && *cp != '+' && *cp != ','; cp++) {
		if (*cp == '.') {
			const si1 *encoding = ++cp;
			for (; *cp != '\0' && *cp != '@' && *cp != '+' && *cp != ','; cp++);
			if ((cp - encoding == 5 && !strncmp(encoding, "UTF-8", 5)) || (cp - encoding == 4 && !strncmp(encoding, "utf8", 4)))
				return(1); // it's UTF-8
			break;
		}
	}
	
	return(0);
}


TERN_m11 UTF8_is_valid_m11(si1 *string)
{
	si4	i, cnt;
	ui1	x;

	
	for (cnt = 0, i = (si4) strlen(string); i--;) {
		x = (ui1) *string++;
		if (cnt) {
			if ((x & 0xC0) != 0x80)  // 0bxx000000 & 0b11000000 == 0b10000000
				return(FALSE_m11);
			--cnt;
		} else {
			if ((x & 0xE0) == 0xC0)  // 0bxxx00000 & 0b11100000 == 0b11000000
			    	cnt = 1;
			else if ((x & 0xF0) == 0xE0)  // 0bxxxx0000 & 0b11110000 == 0b11100000
				cnt = 2;
			else if ((x & 0xF8) == 0xF0)  // 0bxxxxx000 & 0b11111000 == 0b11110000
				cnt = 3;
			else if (x & 0x80)  // 0bx0000000 & 0b10000000 == 0b10000000
				return(FALSE_m11);
		}
	}
	if (cnt)
		return(FALSE_m11);

	return(TRUE_m11);
}


si1	*UTF8_memchr_m11(si1 *s, ui4 ch, size_t sz, si4 *char_num)
{
	si4	i = 0, last_i = 0;
	ui4	c;
	si4	csz;
	

	if (globals_m11->UTF8_offsets_table == NULL)
		UTF8_initialize_tables_m11();
	
	*char_num = 0;
	while (i < sz) {
		c = csz = 0;
		do {
			c <<= 6;
			c += (ui1) s[i++];
			csz++;
		} while (i < sz && !UTF8_ISUTF_m11(s[i]));
		c -= globals_m11->UTF8_offsets_table[csz - 1];
		
		if (c == ch) {
			return(&s[last_i]);
		}
		last_i = i;
		(*char_num)++;
	}
	
	return(NULL);
}


// reads the next utf-8 sequence out of a string, updating an index
#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
ui4     UTF8_next_char_m11(si1 *s, si4 *i)
{
	ui4	ch = 0;
	si4	sz = 0;
	

	if (s[*i] == 0)
		return(0);
	
	if (globals_m11->UTF8_offsets_table == NULL)
		UTF8_initialize_tables_m11();
	
	do {
		ch <<= 6;
		ch += (ui1) s[(*i)++];
		sz++;
	} while (s[*i] && !UTF8_ISUTF_m11(s[*i]));
	
	ch -= globals_m11->UTF8_offsets_table[sz - 1];

	return(ch);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4	UTF8_octal_digit_m11(si1 c)
{
	return(c >= '0' && c <= '7');
}


// char_num => byte offset
#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4     UTF8_offset_m11(si1 *str, si4 char_num)
{
	si4	offs = 0;
	
	
	while (char_num > 0 && str[offs]) {
		(void) (UTF8_ISUTF_m11(str[++offs]) || UTF8_ISUTF_m11(str[++offs]) || UTF8_ISUTF_m11(str[++offs]) || ++offs);
		char_num--;
	}
	
	return(offs);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4     UTF8_printf_m11(si1 *fmt, ...)
{
	si4		sz;
	si1		*src;
	ui4		*w_cs;
	va_list 	args;
	
	
	va_start(args, fmt);
	sz = vasprintf_m11(&src, fmt, args);
	va_end(args);
	
#ifdef MATLAB_m11
	mexPrintf("%s", src);
	free((void *) src);
	return(sz);
#endif
	
#ifdef WINDOWS_m11
	printf("%s", src);
	free((void *) src);
	return(sz);
#endif

	w_cs = (ui4 *) calloc(sz + 1, sizeof(ui4));
	UTF8_to_ucs_m11(w_cs, sz + 1, src, sz);
	printf("%ls", (wchar_t *) w_cs);
	
	free((void *) src);
	free((void *) w_cs);
	
	return(sz);
}


// str points to a backslash or character after a backslash
// returns number of input characters processed
si4     UTF8_read_escape_sequence_m11(si1 *str, ui4 *dest)
{
	ui4	ch;
	si1	digs[9] = "\0\0\0\0\0\0\0\0";
	si4	dno = 0, i = 1;
	
	
	if (*str == '\\')
		++str;
	
	ch = (ui4)str[0];    // take literal character
	if (str[0] == 'n')
		ch = L'\n';
	else if (str[0] == 't')
		ch = L'\t';
	else if (str[0] == 'r')
		ch = L'\r';
	else if (str[0] == 'b')
		ch = L'\b';
	else if (str[0] == 'f')
		ch = L'\f';
	else if (str[0] == 'v')
		ch = L'\v';
	else if (str[0] == 'a')
		ch = L'\a';
	else if (UTF8_octal_digit_m11(str[0])) {
		i = 0;
		do {
			digs[dno++] = str[i++];
		} while (UTF8_octal_digit_m11(str[i]) && dno < 3);
		ch = strtol(digs, NULL, 8);
	}
	else if (str[0] == 'x') {
		while (UTF8_hex_digit_m11(str[i]) && dno < 2) {
			digs[dno++] = str[i++];
		}
		if (dno > 0)
			ch = strtol(digs, NULL, 16);
	}
	else if (str[0] == 'u') {
		while (UTF8_hex_digit_m11(str[i]) && dno < 4) {
			digs[dno++] = str[i++];
		}
		if (dno > 0)
			ch = strtol(digs, NULL, 16);
	}
	else if (str[0] == 'U') {
		while (UTF8_hex_digit_m11(str[i]) && dno < 8) {
			digs[dno++] = str[i++];
		}
		if (dno > 0)
			ch = strtol(digs, NULL, 16);
	}
	*dest = ch;
	
	return(i);
}


// returns length of next utf-8 sequence
#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4      UTF8_seqlen_m11(si1 *s)
{
	if (globals_m11->UTF8_offsets_table == NULL)
		UTF8_initialize_tables_m11();
	
	return(globals_m11->UTF8_trailing_bytes_table[(si4) ((ui1) s[0])] + 1);
}


si1	*UTF8_strchr_m11(si1 *s, ui4 ch, si4 *char_num)
{
	si4	i = 0, last_i = 0;
	ui4	c;
	
	
	*char_num = 0;
	while (s[i]) {
		c = UTF8_next_char_m11(s, &i);
		if (c == ch)
			return(&s[last_i]);
		last_i = i;
		(*char_num)++;
	}
	
	return(NULL);
}


// number of characters
si4     UTF8_strlen_m11(si1 *s)
{
	si4	count = 0;
	si4	i = 0;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	while (UTF8_next_char_m11(s, &i))
		count++;
	
	return(count);
}


// conversions without error checking
// only works for valid UTF-8, i.e. no 5- or 6-byte sequences
// srcsz == source size in bytes, or -1 if 0-terminated
// sz = dest size in # of wide characters

// returns # characters converted
// dest will always be 0-terminated, even if there isn't enough room
// for all the characters.
// if sz == srcsz + 1 (i.e. 4 * srcsz + 4 bytes), there will always be enough space
si4     UTF8_to_ucs_m11(ui4 *dest, si4 sz, si1 *src, si4 srcsz)
{
	ui4	ch;
	si1	*src_end = src + srcsz;
	si4	nb;
	si4	i = 0;
	
	
	if (globals_m11->UTF8_offsets_table == NULL)
		UTF8_initialize_tables_m11();
	
	while (i < sz - 1) {
		nb = globals_m11->UTF8_trailing_bytes_table[(ui1) *src];
		if (srcsz == -1 && *src == 0)
			goto UTF8_DONE_TOUCS_m11;
		else if (src + nb >= src_end)
			goto UTF8_DONE_TOUCS_m11;

		ch = 0;
		switch (nb) {
			// these fall through deliberately
			case 3: ch += (ui1) *src++; ch <<= 6;
			case 2: ch += (ui1) *src++; ch <<= 6;
			case 1: ch += (ui1) *src++; ch <<= 6;
			case 0: ch += (ui1) *src++;
		}
		ch -= globals_m11->UTF8_offsets_table[nb];
		dest[i++] = ch;
	}
	
UTF8_DONE_TOUCS_m11:
	
	dest[i] = 0;
	
	return(i);
}


// srcsz == number of source characters, or -1 if 0-terminated
// sz == size of dest buffer in bytes

// returns # characters converted
// dest will only be 0-terminated if there is enough space. this is
// for consistency; imagine there are 2 bytes of space left, but the next
// character requires 3 bytes. in this case we could NUL-terminate, but in
// general we can't when there's insufficient space. therefore this function
// only NULL-terminates if all the characters fit, and there's space for
// the NULL as well.
// the destination string will never be bigger than the source string
si4     UTF8_to_utf8_m11(si1 *dest, si4 sz, ui4 *src, si4 srcsz)
{
	ui4	ch;
	si4	i = 0;
	si1	*dest_end = dest + sz;
	
	
	while (srcsz < 0 ? src[i] != 0 : i < srcsz) {
		ch = src[i];
		if (ch < 0x80) {
			if (dest >= dest_end)
				return(i);
			*dest++ = (si1) ch;
		}
		else if (ch < 0x800) {
			if (dest >= dest_end - 1)
				return(i);
			*dest++ = (ch >> 6) | 0xC0;
			*dest++ = (ch & 0x3F) | 0x80;
		}
		else if (ch < 0x10000) {
			if (dest >= dest_end - 2)
				return(i);
			*dest++ = (ch >> 12) | 0xE0;
			*dest++ = ((ch >> 6) & 0x3F) | 0x80;
			*dest++ = (ch & 0x3F) | 0x80;
		}
		else if (ch < 0x110000) {
			if (dest >= dest_end - 3)
				return(i);
			*dest++ = (ch >> 18) | 0xF0;
			*dest++ = ((ch >> 12) & 0x3F) | 0x80;
			*dest++ = ((ch >> 6) & 0x3F) | 0x80;
			*dest++ = (ch & 0x3F) | 0x80;
		}
		i++;
	}
	if (dest < dest_end)
		*dest = '\0';
	
	return(i);
}


// convert a string with literal \uxxxx or \Uxxxxxxxx characters to UTF-8
// example: UTF8_unescape(mybuf, 256, "hello\\u220e")
// note the double backslash is needed if called on a C string literal
si4     UTF8_unescape_m11(si1 *buf, si4 sz, si1 *src)
{
	si4	c = 0, amt;
	ui4	ch;
	si1	temp[4];
	
	
	while (*src && c < sz) {
		if (*src == '\\') {
			src++;
			amt = UTF8_read_escape_sequence_m11(src, &ch);
		} else {
			ch = (si4)*src;
			amt = 1;
		}
		src += amt;
		amt = UTF8_wc_to_utf8_m11(temp, ch);
		if (amt > sz - c)
			break;
		memcpy(&buf[c], temp, amt);
		c += amt;
	}
	if (c < sz)
		buf[c] = '\0';
	
	return(c);
}


si4     UTF8_vfprintf_m11(FILE *stream, si1 *fmt, va_list args)
{
	si4	sz;
	si1	*src;
	ui4	*w_cs;
	
	
	sz = vasprintf_m11(&src, fmt, args);

#ifdef MATLAB_m11
	if (stream == stderr || stream == stdout)
		mexPrintf("%s", src);
	else
		fprintf(stream, "%s", src);
	free((void *) src);
	return(sz);
#endif
	
#ifdef WINDOWS_m11
	fprintf(stream, "%s", src);
	free((void *) src);
	return(sz);
#endif
	w_cs = (ui4 *) calloc(sz + 1, sizeof(ui4));
	UTF8_to_ucs_m11(w_cs, sz + 1, src, sz);

	fprintf(stream, "%ls", (wchar_t *) w_cs);
	
	free((void *) src);
	free((void *) w_cs);
	
	return(sz);
}


si4     UTF8_vprintf_m11(si1 *fmt, va_list args)
{
	si4	sz;
	si1	*src;
	ui4	*w_cs;
	
	
	sz = vasprintf_m11(&src, fmt, args);
	
#ifdef MATLAB_m11
	mexPrintf("%s", src);
	free((void *) src);
	return(sz);
#endif
	
#ifdef WINDOWS_m11
	printf("%s", src);
	free((void *) src);
	return(sz);
#endif
	
	w_cs = (ui4 *) calloc(sz + 1, sizeof(ui4));
	UTF8_to_ucs_m11(w_cs, sz + 1, src, sz);
	printf("%ls", (wchar_t *) w_cs);
	
	free((void *) src);
	free((void *) w_cs);
	
	return(sz);
}


si4     UTF8_wc_to_utf8_m11(si1 *dest, ui4 ch)
{
	if (ch < 0x80) {
		dest[0] = (char)ch;
		return(1);
	}
	if (ch < 0x800) {
		dest[0] = (ch >> 6) | 0xC0;
		dest[1] = (ch & 0x3F) | 0x80;
		return(2);
	}
	if (ch < 0x10000) {
		dest[0] = (ch >> 12) | 0xE0;
		dest[1] = ((ch >> 6) & 0x3F) | 0x80;
		dest[2] = (ch & 0x3F) | 0x80;
		return(3);
	}
	if (ch < 0x110000) {
		dest[0] = (ch >> 18) | 0xF0;
		dest[1] = ((ch >> 12) & 0x3F) | 0x80;
		dest[2] = ((ch >> 6) & 0x3F) | 0x80;
		dest[3] = (ch & 0x3F) | 0x80;
		return(4);
	}
	
	return(0);
}



//***********************************************************************//
//*****************  MED VERSIONS OF STANDARD FUNCTIONS  ****************//
//***********************************************************************//


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4    asprintf_m11(si1 **target, si1 *fmt, ...)
{
	si4		ret_val;
	va_list		args;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	va_start(args, fmt);
	ret_val = vasprintf_m11(target, fmt, args);
	va_end(args);
	
	// this function returns the allocated string, so add it to the AT list
	AT_add_entry_m11(*target, __FUNCTION__);

	return(ret_val);
}


void	*calloc_m11(size_t n_members, size_t el_size, const si1 *function, ui4 behavior_on_fail)
{
	void	*ptr;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (n_members == 0 || el_size == 0)
		return((void *) NULL);
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m11)
		behavior_on_fail = globals_m11->behavior_on_fail;
	
#ifdef MATLAB_PERSISTENT_m11
	ptr = mxCalloc((mwSize) n_members, (mwSize) el_size);
#else
	ptr = calloc(n_members, el_size);
#endif
	if (ptr == NULL) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m11)) {
			(void) fprintf_m11(stderr, "%c\n\t%s() failed to allocate the requested array (%ld members of size %ld)\n", 7, __FUNCTION__, n_members, el_size);
			(void) fprintf_m11(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				(void)fprintf_m11(stderr, "\tcalled from function %s()\n", function);
			if (behavior_on_fail & RETURN_ON_FAIL_m11)
				(void)fprintf_m11(stderr, "\t=> returning NULL\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m11)
				(void)fprintf_m11(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m11)
			return(NULL);
		else if (behavior_on_fail & EXIT_ON_FAIL_m11)
			exit_m11(-1);
	}
	
	// alloc tracking
	AT_add_entry_m11(ptr, function);

#ifdef MATLAB_PERSISTENT_m11
	mexMakeMemoryPersistent(ptr);
#endif

	return(ptr);
}


// not a standard function, but closely related
void	**calloc_2D_m11(size_t dim1, size_t dim2, size_t el_size, const si1 *function, ui4 behavior_on_fail)
{
	si8     i;
	ui1	**ptr;
	size_t  dim1_bytes, dim2_bytes, content_bytes, total_bytes;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// Returns pointer to 2 dimensional zeroed array of dim1 by dim2 elements of size el_size
	// ptr[0] points to a one dimensional array of size (dim1 * dim2)
	// The whole block can be freed with free(ptr)
	
	if (dim1 == 0 || dim2 == 0 || el_size == 0)
		return((void **) NULL);
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m11)
		behavior_on_fail = globals_m11->behavior_on_fail;
	
	dim1_bytes = dim1 * sizeof(void *) ;
	dim2_bytes = dim2 * el_size;
	content_bytes = dim1 * dim2_bytes;
	total_bytes = dim1_bytes + content_bytes;
	ptr = (ui1 **) calloc_m11(total_bytes, sizeof(ui1), function, behavior_on_fail);
	ptr[0] = (ui1 *) (ptr + dim1);
	for (i = 1; i < dim1; ++i)
		ptr[i] = ptr[i - 1] + dim2_bytes;
		
	return((void **) ptr);
}


size_t	calloc_size_m11(void *address, size_t element_size)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	return (malloc_size_m11(address) / element_size);
}


void	exit_m11(si4 status)
{
#ifdef FN_DEBUG_m11
	printf_m11("%s()\n", __FUNCTION__);
#endif
	
#ifdef WINDOWS_m11
	win_cleanup_m11();
#endif

#ifdef MATLAB_m11
	const si1	tmp_str[32];
	
	sprintf((char *) tmp_str, "Exit status: %d\n", status);
	mexErrMsgTxt(tmp_str);
#else
	exit(status);
#endif
}


FILE	*fopen_m11(si1 *path, si1 *mode, const si1 *function, ui4 behavior_on_fail)
{
	FILE	*fp;

#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m11)
		behavior_on_fail = globals_m11->behavior_on_fail;

#if defined MACOS_m11 || defined LINUX_m11
	if ((fp = fopen(path, mode)) == NULL) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m11)) {
			UTF8_fprintf_m11(stderr, "%c\n\t%s() failed to open file \"%s\"\n", 7, __FUNCTION__, path);
			fprintf_m11(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				fprintf_m11(stderr, "\tcalled from function %s()\n", function);
			if (behavior_on_fail & RETURN_ON_FAIL_m11)
				fprintf_m11(stderr, "\t=> returning NULL\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m11)
				fprintf_m11(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m11)
			return(NULL);
		else if (behavior_on_fail & EXIT_ON_FAIL_m11)
			exit_m11(-1);
}
#endif
	
#ifdef WINDOWS_m11
	TERN_m11	binary_set = FALSE_m11;
	TERN_m11	write_mode = FALSE_m11;
	si1		tmp_mode[8], *c, *tc;
	
	
	// MED requires binary mode
	c = mode;
	tc = tmp_mode;
	while (*c) {
		if (*c == 't') {
			*tc++ = 'b';
			binary_set = TRUE_m11;
		} else {
			if (*c == 'w' || *c == 'a' || *c == '+')
				write_mode = TRUE_m11;
			*tc++ = *c++;
		}
	}
	if (binary_set == FALSE_m11)
		*tc++ = 'b';
	*tc = 0;
	
	if (write_mode == TRUE_m11)
		fp = _fsopen(path, tmp_mode, _SH_DENYNO);
	else
		fp = fopen(path, tmp_mode);
		
	if (fp == NULL) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m11)) {
			UTF8_fprintf_m11(stderr, "%c\n\t%s() failed to open file \"%s\"\n", 7, __FUNCTION__, path);
			fprintf_m11(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				fprintf_m11(stderr, "\tcalled from function %s()\n", function);
			if (behavior_on_fail & RETURN_ON_FAIL_m11)
				fprintf_m11(stderr, "\t=> returning NULL\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m11)
				fprintf_m11(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m11)
			return(NULL);
		else if (behavior_on_fail & EXIT_ON_FAIL_m11)
			exit_m11(-1);
	}
#endif

	return(fp);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4     fprintf_m11(FILE *stream, si1 *fmt, ...)
{
	si1		*temp_str;
	si4		ret_val;
	va_list		args;
	
	
	va_start(args, fmt);
	ret_val = vasprintf_m11(&temp_str, fmt, args);  // could just call vfprintf_m11() here & be done, but it's hardly any extra code, so duplicate & skip extra function call
	va_end(args);
	
	if (ret_val >= 0) {
#ifdef MATLAB_m11
		if (stream == stderr || stream == stdout)
			ret_val = mexPrintf("%s", temp_str);
		else
#endif
		ret_val = fprintf(stream, "%s", temp_str);
		free((void *) temp_str);
	}

	return(ret_val);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4	fputc_m11(si4 c, FILE *stream)
{
	si4	ret_val;

	
#ifdef MATLAB_m11
	if (stream == stderr || stream == stdout)
		ret_val = mexPrintf("%c", c);
	else
#endif
	ret_val = fputc(c, stream);
	
	return(ret_val);
}


size_t	fread_m11(void *ptr, size_t el_size, size_t n_members, FILE *stream, si1 *path, const si1 *function, ui4 behavior_on_fail)
{
	size_t	nr;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m11)
		behavior_on_fail = globals_m11->behavior_on_fail;
	
	if ((nr = fread(ptr, el_size, n_members, stream)) != n_members) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m11)) {
			UTF8_fprintf_m11(stderr, "%c\n\t%s() failed to read file \"%s\"\n", 7, __FUNCTION__, path);
			fprintf_m11(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				fprintf_m11(stderr, "\tcalled from function %s()\n", function);
			if (behavior_on_fail & RETURN_ON_FAIL_m11)
				fprintf_m11(stderr, "\t=> returning number of items read\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m11)
				fprintf_m11(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m11)
			return(nr);
		else if (behavior_on_fail & EXIT_ON_FAIL_m11)
			exit_m11(-1);
	}

	return(nr);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void    free_m11(void *ptr, const si1 *function)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (AT_remove_entry_m11(ptr, function) == TRUE_m11) {
		#ifdef MATLAB_PERSISTENT_m11
		mxFree(ptr);
		#else
		free(ptr);
		#endif
	}
	
	return;
}


// not a standard function, but closely related
void    free_2D_m11(void **ptr, size_t dim1, const si1 *function)
{
	si8     i;
	void	*base_address;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
		
	// dim1 == 0 indicates allocated en block per caller (caller could just use free_m11() in this case, as here)
	if (dim1 == 0) {
		free_m11((void *) ptr, function);
		return;
	}
		
	// allocated en block  (check all addresses because pointers may have been sorted)
	base_address = (void *) ((ui1 *) ptr + (dim1 * sizeof(void *)));
	for (i = 0; i < dim1; ++i) {
		if (ptr[i] == base_address) {
			free_m11((void *) ptr, function);
			return;
		}
	}

	// allocated separately
	for (i = 0; i < dim1; ++i)
		free_m11((void *) ptr[i], function);
	free_m11((void *) ptr, function);

	return;
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4     fscanf_m11(FILE *stream, si1 *fmt, ...)
{
	si4		ret_val;
	va_list		args;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
#ifdef WINDOWS_m11
	si1	*new_fmt = NULL;
	
	// convert format string
	new_fmt = windify_format_string_m11(fmt);
	
	va_start(args, fmt);
	ret_val = vfscanf(stream, new_fmt, args);
	va_end(args);
	
	if (new_fmt != fmt)
		free((void *) new_fmt);
#endif
	
#if defined MACOS_m11 || defined LINUX_m11
	va_start(args, fmt);
	ret_val = vfscanf(stream, fmt, args);
	va_end(args);
#endif
	
	return(ret_val);
}


si4	fseek_m11(FILE *stream, si8 offset, si4 whence, si1 *path, const si1 *function, ui4 behavior_on_fail)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m11)
		behavior_on_fail = globals_m11->behavior_on_fail;
	
#if defined MACOS_m11 || defined LINUX_m11
	if ((fseek(stream, offset, whence)) == -1) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m11)) {
			(void) fprintf_m11(stderr, "%c\n\t%s() failed to move the file pointer to requested location (offset %ld, whence %d)\n", 7, __FUNCTION__, offset, whence);
			(void) UTF8_fprintf_m11(stderr, "%\tin file \"%s\"\n", path);
			(void) fprintf_m11(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				(void) fprintf_m11(stderr, "\tcalled from function %s()\n", function);
			if (behavior_on_fail & RETURN_ON_FAIL_m11)
				(void) fprintf_m11(stderr, "\t=> returning -1\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m11)
				(void) fprintf_m11(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m11)
			return(-1);
		else if (behavior_on_fail & EXIT_ON_FAIL_m11)
			exit_m11(-1);
	}
#endif

#ifdef WINDOWS_m11
	if ((_fseeki64(stream, offset, whence)) == -1) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m11)) {
			(void) fprintf_m11(stderr, "%c\n\t%s() failed to move the file pointer to requested location (offset %ld, whence %d)\n", 7, __FUNCTION__, offset, whence);
			(void) UTF8_fprintf_m11(stderr, "%\tin file \"%s\"\n", path);
			(void) fprintf_m11(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				(void) fprintf_m11(stderr, "\tcalled from function %s()\n", function);
			if (behavior_on_fail & RETURN_ON_FAIL_m11)
				(void) fprintf_m11(stderr, "\t=> returning -1\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m11)
				(void) fprintf_m11(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m11)
			return(-1);
		else if (behavior_on_fail & EXIT_ON_FAIL_m11)
			exit_m11(-1);
	}
#endif

	return(0);
}
		
		
si8	ftell_m11(FILE *stream, const si1 *function, ui4 behavior_on_fail)
{
	si8	pos;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m11)
		behavior_on_fail = globals_m11->behavior_on_fail;
	
#if defined MACOS_m11 || defined LINUX_m11
	if ((pos = ftell(stream)) == -1) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m11)) {
			(void) fprintf_m11(stderr, "%c\n\t%s() failed obtain the current location\n", 7, __FUNCTION__);
			(void) fprintf_m11(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				(void)fprintf_m11(stderr, "\tcalled from function %s()\n", function);
			if (behavior_on_fail & RETURN_ON_FAIL_m11)
				(void)fprintf_m11(stderr, "\t=> returning -1\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m11)
				(void)fprintf_m11(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m11)
			return(-1);
		else if (behavior_on_fail & EXIT_ON_FAIL_m11)
			exit_m11(-1);
	}
#endif
#ifdef WINDOWS_m11
	if ((pos = _ftelli64(stream)) == -1) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m11)) {
			(void) fprintf_m11(stderr, "%c\n\t%s() failed obtain the current location\n", 7, __FUNCTION__);
			(void) fprintf_m11(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				(void) fprintf_m11(stderr, "\tcalled from function %s()\n", function);
			if (behavior_on_fail & RETURN_ON_FAIL_m11)
				(void) fprintf_m11(stderr, "\t=> returning -1\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m11)
				(void) fprintf_m11(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m11)
			return(-1);
		else if (behavior_on_fail & EXIT_ON_FAIL_m11)
			exit_m11(-1);
	}
#endif

	return(pos);
}
		
		
size_t	fwrite_m11(void *ptr, size_t el_size, size_t n_members, FILE *stream, si1 *path, const si1 *function, ui4 behavior_on_fail)
{
	size_t	nw;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m11)
		behavior_on_fail = globals_m11->behavior_on_fail;
	
	if ((nw = fwrite(ptr, el_size, n_members, stream)) != n_members) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m11)) {
			(void) UTF8_fprintf_m11(stderr, "%c\n\t%s() failed to write file \"%s\"\n", 7, __FUNCTION__, path);
			(void) fprintf_m11(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				(void) fprintf_m11(stderr, "\tcalled from function %s()\n", function);
			if (behavior_on_fail & RETURN_ON_FAIL_m11)
				(void) fprintf_m11(stderr, "\t=> returning number of items written\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m11)
				(void) fprintf_m11(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m11)
			return(nw);
		else if (behavior_on_fail & EXIT_ON_FAIL_m11)
			exit_m11(-1);
	}
	
	return(nw);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
char	*getcwd_m11(char *buf, size_t size)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
#ifdef MATLAB_m11
	error_message_m11("%s(): the current working directory is not defined for Matlab mex files => pass full path\n", __FUNCTION__);
	return(NULL);
#endif

#if defined MACOS_m11 || defined LINUX_m11
	return(getcwd(buf, size));
#endif
#ifdef WINDOWS_m11
	return(_getcwd(buf, size));
#endif
}


void	*malloc_m11(size_t n_bytes, const si1 *function, ui4 behavior_on_fail)
{
	void	*ptr;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m11)
		behavior_on_fail = globals_m11->behavior_on_fail;
	
	if (n_bytes == 0)
		return((void *) NULL);
	
#ifdef MATLAB_PERSISTENT_m11
	ptr = mxMalloc((mwSize) n_bytes);
#else
	ptr = malloc(n_bytes);
#endif
	if (ptr == NULL) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m11)) {
			(void)fprintf_m11(stderr, "%c\n\t%s() failed to allocate the requested array (%ld bytes)\n", 7, __FUNCTION__, n_bytes);
			(void)fprintf_m11(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				(void)fprintf_m11(stderr, "\tcalled from function %s()\n", function);
			if (behavior_on_fail & RETURN_ON_FAIL_m11)
				(void)fprintf_m11(stderr, "\t=> returning NULL\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m11)
				(void)fprintf_m11(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m11)
			return(NULL);
		else if (behavior_on_fail & EXIT_ON_FAIL_m11)
			exit_m11(-1);
	}
	
	// alloc tracking
	AT_add_entry_m11(ptr, function);

#ifdef MATLAB_PERSISTENT_m11
	mexMakeMemoryPersistent(ptr);
#endif
	
	return(ptr);
}
		

// not a standard function, but closely related
void	**malloc_2D_m11(size_t dim1, size_t dim2, size_t el_size, const si1 *function, ui4 behavior_on_fail)
{
	si8     i;
	ui1	**ptr;
	size_t  dim1_bytes, dim2_bytes, content_bytes, total_bytes;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// Returns pointer to 2 dimensional array (not zeroed) of dim1 by dim2 elements of size el_size
	// ptr[0] points to a one dimensional array of size (dim1 * dim2)
	// The whole block can be freed with free(ptr)
	
	if (dim1 == 0 || dim2 == 0 || el_size == 0)
		return((void **) NULL);
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m11)
		behavior_on_fail = globals_m11->behavior_on_fail;
	
	dim1_bytes = dim1 * sizeof(void *);
	dim2_bytes = dim2 * el_size;
	content_bytes = dim1 * dim2_bytes;
	total_bytes = dim1_bytes + content_bytes;
	ptr = (ui1 **) malloc_m11(total_bytes, function, behavior_on_fail);
	ptr[0] = (ui1 *) (ptr + dim1);
	
	for (i = 1; i < dim1; ++i)
		ptr[i] = ptr[i - 1] + dim2_bytes;
	
	return((void **) ptr);
}


size_t	malloc_size_m11(void *address)
{
	si8		i;
	AT_NODE		*atn;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	atn = globals_m11->AT_nodes;
	for (i = globals_m11->AT_node_count; i--; ++atn) {
		if (atn->address == address)
			return(atn->bytes);
	}
	
	return(0);
}


void	memset_m11(void *ptr, const void *pattern, size_t pat_len, size_t n_members)
{
	si8	i;
	si2	*si2_p, si2_pat;
	si4	*si4_p, si4_pat;
	si8	*si8_p, si8_pat;
	size_t	buf_len;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	buf_len = n_members * pat_len;
	
	// regular memset()
	if (pat_len == 1) {
		memset(ptr, (si4) *((si1 *) pattern), buf_len);
		return;
	}
	
#ifdef MACOS_m11
	switch (pat_len) {  // optimized versions currently only for MacOS
		case 4:
			memset_pattern4(ptr, pattern, buf_len);
			return;
		case 8:
			memset_pattern8(ptr, pattern, buf_len);
			return;
		case 16:
			memset_pattern16(ptr, pattern, buf_len);
			return;
	}
#endif
	
	switch (pat_len) {
		case 2:
			si2_p = (si2 *) ptr;
			si2_pat = *((si2 *) pattern);
			for (i = buf_len >> 1; i--;)
				*si2_p++ = si2_pat;
			return;
		case 4:
			si4_p = (si4 *) ptr;
			si4_pat = *((si4 *) pattern);
			for (i = buf_len >> 2; i--;)
				*si4_p++ = si4_pat;
			return;
		case 8:
			si8_p = (si8 *) ptr;
			si8_pat = *((si8 *) pattern);
			for (i = buf_len >> 3; i--;)
				*si8_p++ = si8_pat;
			return;
		// case 16:  removed because some OSs silently implement sf16 as sf8, which would be quite bad with this usage
		default:
			warning_message_m11("%s(): unsupported pattern length\n", __FUNCTION__);
			return;
	}
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
TERN_m11	mlock_m11(void *addr, size_t len, TERN_m11 zero_data, const si1 *function, ui4 behavior_on_fail)
{
	si1			*err_str;
	si4			ret_val;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	#if defined MACOS_m11 || defined LINUX_m11
	ret_val = mlock(addr, len);
	#endif
	
	#ifdef WINDOWS_m11
	if (VirtualLock(addr, len))
		ret_val = 0;
	else
		ret_val = -1;
	#endif
	
	if (ret_val == 0) {
		if (zero_data == TRUE_m11)
			memset(addr, 0, len);  // forces OS to give real memory before return (otherwise there may be a lag)
		return(TRUE_m11);
	}
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m11)
		behavior_on_fail = globals_m11->behavior_on_fail;
	
	if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m11)) {
		#if defined MACOS_m11 || defined LINUX_m11
		err_str = strerror(errno);
		#endif
		#ifdef WINDOWS_m11
		errno = (si4) GetLastError();
		if (errno = 1453)
			err_str = "insufficient quota to complete the requested service";
		else
			err_str = "unknown error";
		#endif
		fprintf_m11(stderr, "%c\n\t%s() failed to lock the requested array (%ld bytes)\n", 7, __FUNCTION__, len);
		fprintf_m11(stderr, "\tsystem error number %d (%s)\n", errno, err_str);
		if (function != NULL)
			fprintf_m11(stderr, "\tcalled from function %s()\n", function);
		if (behavior_on_fail & RETURN_ON_FAIL_m11)
			fprintf_m11(stderr, "\t=> returning FALSE\n\n");
		else if (behavior_on_fail & EXIT_ON_FAIL_m11)
			fprintf_m11(stderr, "\t=> exiting program\n\n");
		fflush(stderr);
	}
	if (behavior_on_fail & EXIT_ON_FAIL_m11)
		exit_m11(-1);
	
	return(FALSE_m11);
}

	    
#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
TERN_m11	munlock_m11(void *addr, size_t len, const si1 *function, ui4 behavior_on_fail)
{
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	#if defined MACOS_m11 || defined LINUX_m11
	if (munlock(addr, len) == 0)
		return(TRUE_m11);
	#endif
	
	#ifdef WINDOWS_m11
	if (VirtualUnlock(addr, len))  // returns non-zero on success
		return(TRUE_m11);
	#endif
		
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m11)
		behavior_on_fail = globals_m11->behavior_on_fail;

	if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m11)) {
		fprintf_m11(stderr, "%c\n\t%s() failed to unlock the requested array (%ld bytes)\n", 7, __FUNCTION__, len);
		fprintf_m11(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
		if (function != NULL)
			fprintf_m11(stderr, "\tcalled from function %s()\n", function);
		if (behavior_on_fail & RETURN_ON_FAIL_m11)
			fprintf_m11(stderr, "\t=> returning FALSE\n\n");
		else if (behavior_on_fail & EXIT_ON_FAIL_m11)
			fprintf_m11(stderr, "\t=> exiting program\n\n");
		fflush(stderr);
	}
	if (behavior_on_fail & EXIT_ON_FAIL_m11)
		exit_m11(-1);
	
	return(FALSE_m11);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4     printf_m11(si1 *fmt, ...)
{
	si1		*temp_str;
	si4		ret_val;
	va_list		args;
	
	
	va_start(args, fmt);
	ret_val = vasprintf_m11(&temp_str, fmt, args);  // could just call vprintf_m11() here & be done, but it's hardly any extra code, so duplicate & skip extra function call
	va_end(args);
	
	if (ret_val >= 0) {
#ifdef MATLAB_m11
		ret_val = mexPrintf("%s", temp_str);
#else
		ret_val = printf("%s", temp_str);
#endif
		free((void *) temp_str);
	}
		
	return(ret_val);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4	putc_m11(si4 c, FILE *stream)
{
	return(fputc_m11(c, stream));
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4	putch_m11(si4 c)
{
	si4	ret_val;


#ifdef MATLAB_m11
	ret_val = mexPrintf("%c", c);
#else
	#ifdef WINDOWS_m11
		ret_val = _putch(c);
	#else
		ret_val = fputc_m11(c, stdout);
	#endif
#endif
	
	return(ret_val);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4	putchar_m11(si4 c)
{
	return(fputc_m11(c, stdout));
}


void	*realloc_m11(void *orig_ptr, size_t n_bytes, const si1 *function, ui4 behavior_on_fail)
{
	void	*ptr;
	ui8	alloced_bytes;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m11)
		behavior_on_fail = globals_m11->behavior_on_fail;
	
	if (n_bytes == 0) {
		if (orig_ptr != NULL)
			free_m11((void *) orig_ptr, function);
		return((void *) NULL);
	}
	
	// see if already has enough memory
	if (orig_ptr != NULL)
		alloced_bytes = AT_alloc_size_m11(orig_ptr);
	else
		alloced_bytes = 0;
	if (alloced_bytes >= n_bytes)
		return(orig_ptr);
	
#ifdef MATLAB_PERSISTENT_m11
	ptr = mxRealloc(orig_ptr, (mwSize) n_bytes);
#else
	ptr = realloc(orig_ptr, n_bytes);
#endif
	if (ptr == NULL) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m11)) {
			fprintf_m11(stderr, "%c\n\t%s() failed to reallocate the requested array (%ld bytes)\n", 7, __FUNCTION__, n_bytes);
			fprintf_m11(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				fprintf_m11(stderr, "\tcalled from function %s()\n", function);
			if (behavior_on_fail & RETURN_ON_FAIL_m11)
				fprintf_m11(stderr, "\t=> returning unreallocated pointer\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m11)
				fprintf_m11(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m11)
			return(orig_ptr);
		else if (behavior_on_fail & EXIT_ON_FAIL_m11)
			exit_m11(-1);
	}
	
	// alloc tracking
	AT_update_entry_m11(orig_ptr, ptr, function);

#ifdef MATLAB_PERSISTENT_m11
	mexMakeMemoryPersistent(ptr);
#endif

	return(ptr);
}


// not a standard function, but closely related
void	**realloc_2D_m11(void **curr_ptr, size_t curr_dim1, size_t new_dim1, size_t curr_dim2, size_t new_dim2, size_t el_size, const si1 *function, ui4 behavior_on_fail)
{
	si8	i;
	void	**new_ptr;
	size_t	least_dim1, least_dim2;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// Returns pointer to a reallocated 2 dimensional array of new_dim1 by new_dim2 elements of size el_size (new unused elements are zeroed)
	// ptr[0] points to a one dimensional array of size (dim1 * dim2)
	// The whole block can be freed with free(ptr)
	// Assumes memory was allocated with malloc_2D_m11() or calloc_2D_m11()
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m11)
		behavior_on_fail = globals_m11->behavior_on_fail;
	
	if (new_dim1 == 0 || new_dim2 == 0 || el_size == 0) {
		if (curr_ptr != NULL)
			free_m11((void *) curr_ptr, function);
		return((void **) NULL);
	}
	
	if (curr_ptr == NULL) {
		error_message_m11("%s(): attempting to re-allocate NULL pointer, called from function %s()\n", __FUNCTION__, function);
		return(NULL);
	}
	
	if (new_dim1 < curr_dim1)
		warning_message_m11("%s(): re-allocating first dimension to smaller size, called from function %s()\n", __FUNCTION__, function);
	if (new_dim2 < curr_dim2)
		warning_message_m11("%s(): re-allocating second dimension to smaller size, called from function %s()\n", __FUNCTION__, function);
	
	new_ptr = calloc_2D_m11(new_dim1, new_dim2, el_size, function, behavior_on_fail);
	
	least_dim1 = (curr_dim1 <= new_dim1) ? curr_dim1 : new_dim1;
	least_dim2 = (curr_dim2 <= new_dim2) ? curr_dim2 : new_dim2;
	for (i = 0; i < least_dim1; ++i)
		memcpy((void *) new_ptr[i], curr_ptr[i], (size_t) (least_dim2 * el_size));
	
	free_m11((void *) curr_ptr, function);
	
	return((void **) new_ptr);
}
		
		
// not a standard function, but closely related
void	*recalloc_m11(void *orig_ptr, size_t curr_bytes, size_t new_bytes, const si1 *function, ui4 behavior_on_fail)
{
	void	*ptr;
	ui1	*ui1_p;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m11)
		behavior_on_fail = globals_m11->behavior_on_fail;
	
	if (new_bytes == 0) {
		if (orig_ptr != NULL)
			free_m11((void *) orig_ptr, function);
		return((void *) NULL);
	}
		
#ifdef MATLAB_PERSISTENT_m11
	ptr = mxRealloc(orig_ptr, (mwSize) new_bytes);
#else
	ptr = realloc(orig_ptr, new_bytes);
#endif
	if (ptr == NULL) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m11)) {
			fprintf_m11(stderr, "%c\n\t%s() failed to reallocate the requested array (%ld bytes)\n", 7, __FUNCTION__, new_bytes);
			fprintf_m11(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				fprintf_m11(stderr, "\tcalled from function %s()\n", function);
			if (behavior_on_fail & RETURN_ON_FAIL_m11)
				fprintf_m11(stderr, "\t=> returning unreallocated pointer\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m11)
				fprintf_m11(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m11)
			return(orig_ptr);
		else if (behavior_on_fail & EXIT_ON_FAIL_m11)
			exit_m11(-1);
	}
	
	// zero new bytes
	if (new_bytes > curr_bytes) {
		ui1_p = (ui1 *) ptr + curr_bytes;
		memset(ui1_p, 0, new_bytes - curr_bytes);
	}
	
	// alloc tracking
	AT_update_entry_m11(orig_ptr, ptr, function);
	
#ifdef MATLAB_PERSISTENT_m11
	mexMakeMemoryPersistent(ptr);
#endif
	
	return(ptr);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4     scanf_m11(si1 *fmt, ...)
{
	si4         ret_val;
	va_list     args;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
#ifdef WINDOWS_m11
	si1* new_fmt = NULL;
	
	// convert format string
	new_fmt = windify_format_string_m11(fmt);
	
	va_start(args, fmt);
	ret_val = vscanf(new_fmt, args);
	va_end(args);
	
	if (new_fmt != fmt)
		free((void *) new_fmt);
#endif
	
#if defined MACOS_m11 || defined LINUX_m11
	va_start(args, fmt);
	ret_val = vscanf(fmt, args);
	va_end(args);
#endif
	
	return(ret_val);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4    snprintf_m11(si1 *target, si4 target_field_bytes, si1 *fmt, ...)
{
	si4		ret_val;
	va_list		args;
	
	
	// as opposed to standard snprintf(), snprintf_m11() allows source & target strings to overlap
	
	va_start(args, fmt);
	ret_val = vsnprintf_m11(target, target_field_bytes, fmt, args);
	va_end(args);
	
	return(ret_val);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4    sprintf_m11(si1 *target, si1 *fmt, ...)
{
	si1		*tmp_str;
	si4		ret_val;
	va_list		args;
	
	
	// as opposed to standard sprintf(), sprintf_m11() allows source & target strings to overlap
		
	va_start(args, fmt);
	ret_val = vasprintf_m11(&tmp_str, fmt, args);  	// could just call vsprintf_m11() here & be done, but it's hardly any extra code, so duplicate & skip extra function call
	va_end(args);
	
	memcpy(target, tmp_str, ret_val + 1);
	free((void *) tmp_str);

	return(ret_val);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4     sscanf_m11(si1 *target, si1 *fmt, ...)
{
	si4		ret_val;
	va_list		args;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
#ifdef WINDOWS_m11
	si1* new_fmt = NULL;
	
	// convert format string
	new_fmt = windify_format_string_m11(fmt);
	
	va_start(args, fmt);
	ret_val = vsscanf(target, new_fmt, args);
	va_end(args);
	
	if (new_fmt != fmt)
		free((void *) new_fmt);
#endif
	
#if defined MACOS_m11 || defined LINUX_m11
	va_start(args, fmt);
	ret_val = vsscanf(target, fmt, args);
	va_end(args);
#endif
	
	return(ret_val);
}


si8     strcat_m11(si1 *target, si1 *source)
{
	si1	*c;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// returns final length (not including terminal zero)
	
	if (target == NULL || source == NULL)
		return(-1);
	
	c = target;
	while ((*c++));
	--c;
	while ((*c++ = *source++));
	
	return((si8)((c - target) - 1));
}


si8     strcpy_m11(si1 *target, si1 *source)
{
	si1	*c;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// returns length (not including terminal zero)
	
	if (target == NULL || source == NULL)
		return(-1);
	
	c = target;
	while ((*c++ = *source++));
	
	return((si8) ((c - target) - 1));
}


si8    strncat_m11(si1 *target, si1 *source, si4 target_field_bytes)
{
	si1	*c;
	si8	len = 0;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// returns final length (not including terminal zeros)
	
	if (target == NULL)
		return(-1);
	if (target_field_bytes < 1) {
		*target = 0;
		return(-1);
	}
	
	c = target;
	if (source == NULL) {
		--target_field_bytes;
	} else {
		while (--target_field_bytes)
			if (*c++ == '\0')
				break;
	}
	
	--c;
	++target_field_bytes;
	
	while (--target_field_bytes)
		if ((*c++ = *source++) == '\0')
			break;
	len = (si8)((c - target) - 1);
	if (target_field_bytes) {
		while (--target_field_bytes)
			*c++ = '\0';
	} else {
		warning_message_m11("%s(): target string truncated\n", __FUNCTION__);
	}
	
	*c = '\0';
	
	return(len);
}


si8    strncpy_m11(si1 *target, si1 *source, si4 target_field_bytes)
{
	si1	*c;
	si8	len = 0;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	// returns length (not including terminal zeros)
	
	if (target == NULL)
		return(-1);
	
	if (target_field_bytes < 1) {
		*target = 0;
		return(-1);
	}
	
	c = target;
	if (source == NULL) {
		--target_field_bytes;
	} else {
		while (--target_field_bytes) {
			if ((*c++ = *source++) == 0)
				break;
		}
		len = (si8)((c - target) - 1);
	}
	
	if (target_field_bytes) {
		while (--target_field_bytes)
			*c++ = '\0';
	} else {
		warning_message_m11("%s(): target string truncated\n", __FUNCTION__);
	}
	
	*c = '\0';
	
	return(len);
}


si4     system_m11(si1 *command, TERN_m11 null_std_streams, const si1 *function, ui4 behavior_on_fail)
{
	si1	*temp_command;
	si4	ret_val, len;
	
#ifdef FN_DEBUG_m11
	message_m11("%s()\n", __FUNCTION__);
#endif
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m11)
		behavior_on_fail = globals_m11->behavior_on_fail;
	
	if (null_std_streams == TRUE_m11) {
		len = strlen(command);
		temp_command = malloc(len + (FULL_FILE_NAME_BYTES_m11 * 2) + 9);
		sprintf_m11(temp_command, "%s 1> %s 2> %s", command, NULL_DEVICE_m11, NULL_DEVICE_m11);
		command = temp_command;
	}
	
#if defined MACOS_m11 || defined LINUX_m11
	ret_val = system(command);
#endif
#ifdef WINDOWS_m11
	ret_val = win_system_m11(command);
#endif

	if (ret_val) {
		if (behavior_on_fail & RETRY_ONCE_m11) {
			nap_m11("1 ms");  // wait 1 ms
#if defined MACOS_m11 || defined LINUX_m11
			ret_val = system(command);
#endif
#ifdef WINDOWS_m11
			ret_val = win_system_m11(command);
#endif
			if (ret_val == 0) {
				if (null_std_streams == TRUE_m11)
					free((void *) temp_command);
				return(0);
			}
		}
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m11)) {
			fprintf_m11(stderr, "%c\n%s() failed\n", 7, __FUNCTION__);
			fprintf_m11(stderr, "\tcommand: \"%s\"\n", command);
			fprintf_m11(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			fprintf_m11(stderr, "\tshell return value %d\n", ret_val);
			if (function != NULL)
				fprintf_m11(stderr, "\tcalled from function %s()\n", function);
			if (behavior_on_fail & RETURN_ON_FAIL_m11)
				fprintf_m11(stderr, "\t=> returning -1\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m11)
				fprintf_m11(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m11) {
			if (null_std_streams == TRUE_m11)
				free((void *) temp_command);
			return(-1);
		} else if (behavior_on_fail & EXIT_ON_FAIL_m11) {
			exit_m11(-1);
		}
	}
	
	if (null_std_streams == TRUE_m11)
		free((void *) temp_command);
	
	return(0);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4    vasprintf_m11(si1 **target, si1 *fmt, va_list args)
{
	si4	ret_val;
	

#ifdef WINDOWS_m11  // no vasprintf() in Windows
	va_list		args_copy;
	
	*target = (si1 *) calloc((size_t) PRINTF_BUF_LEN_m11, sizeof(si1));
	va_copy(args_copy, args);  // save a copy before use in case need to realloc
	push_behavior_m11(RETURN_ON_FAIL_m11 | SUPPRESS_OUTPUT_m11);
	ret_val = vsnprintf_m11(*target, PRINTF_BUF_LEN_m11, fmt, args);
	pop_behavior_m11();
	
	// expand memory to required size
	*target = (si1 *) realloc((void *) *target, (size_t) (ret_val + 1));
	if (ret_val >= PRINTF_BUF_LEN_m11)
		ret_val = vsnprintf_m11(*target, ret_val + 1, fmt, args_copy);
#endif
	
#if defined MACOS_m11 || defined LINUX_m11
	ret_val = vasprintf(target, fmt, args);
#endif
	
	return(ret_val);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4     vfprintf_m11(FILE *stream, si1 *fmt, va_list args)
{
	si1	*temp_str;
	si4	ret_val;
	
	
	ret_val = vasprintf_m11(&temp_str, fmt, args);
	
	if (ret_val >= 0) {
#ifdef MATLAB_m11
		if (stream == stderr || stream == stdout)
			ret_val = mexPrintf("%s", temp_str);
		else
#endif
		ret_val = fprintf(stream, "%s", temp_str);
		free((void *) temp_str);
	}
	

	return(ret_val);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4     vprintf_m11(si1 *fmt, va_list args)
{
	si1	*temp_str;
	si4	ret_val;
	
	
	ret_val = vasprintf_m11(&temp_str, fmt, args);
	
	if (ret_val >= 0) {
#ifdef MATLAB_m11
		ret_val = mexPrintf("%s", temp_str);
#else
		ret_val = printf("%s", temp_str);
#endif
		free((void *) temp_str);
	}
	
	return(ret_val);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4    vsnprintf_m11(si1 *target, si4 target_field_bytes, si1 *fmt, va_list args)
{
	si4	ret_val;
	si1	*temp_str;
	
	
	//******** vsnprintf_m11() CONTAINS THE WINDOWS FORMATTING FOR ALL MED PRINTF FUNCTIONS ********//
	
	// as opposed to standard vsnprintf(), vsnprintf_m11() allows source & target strings to overlap
	
	if (target_field_bytes <= 1) {
		if (target_field_bytes == 1) {
			*target = 0;
			return(0);
		}
		return(-1);
	}
	
#ifdef WINDOWS_m11
	TERN_m11	free_fmt = FALSE_m11;
	si1		*new_fmt;
	
	// convert format string
	new_fmt = windify_format_string_m11(fmt);
	
	if (new_fmt != fmt) {
		fmt = new_fmt;
		free_fmt = TRUE_m11;
	}
#endif
	// Guarantee zeros in unused bytes per MED requirements
	temp_str = (si1 *) calloc((size_t) target_field_bytes, sizeof(si1));
	ret_val = vsnprintf(temp_str, target_field_bytes, fmt, args);
	
	// Guarantee terminal zero on overflow (not done in Linux & Windows)
	if (ret_val >= target_field_bytes) {
		temp_str[target_field_bytes - 1] = 0;
		warning_message_m11("%s(): target string truncated\n", __FUNCTION__);
	}
	memcpy(target, temp_str, target_field_bytes);
	free((void *) temp_str);
	
#ifdef WINDOWS_m11
	// convert file system paths
	windify_file_paths_m11(NULL, target);

	// clean up
	if (free_fmt == TRUE_m11)
		free((void *) new_fmt);
#endif
	
	return(ret_val);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4    vsprintf_m11(si1 *target, si1 *fmt, va_list args)
{
	si1		*tmp_str;
	si4		ret_val;
	
	
	// as opposed to standard vsprintf(), vsprintf_m11() allows source & target strings to overlap
	
	ret_val = vasprintf_m11(&tmp_str, fmt, args);
	
	memcpy(target, tmp_str, ret_val + 1);
	free((void *) tmp_str);

	return(ret_val);
}



