
// Make this file the first include
#ifndef TARGETS_IN_m11
#define TARGETS_IN_m11

//**********************************************************************************//
//************************************  Target OS  *********************************//
//**********************************************************************************//

// Target OS Options: LINUX_m10, MACOS_m10, or WINDOWS_m10
// Define one of these here
#define MACOS_m11
// #define LINUX_m11
// #define WINDOWS_m11

#ifdef WINDOWS_m11
	#define _CRT_SECURE_NO_WARNINGS
	#define NEED_WIN_SOCKETS_m11  // define this if winsock2.h needed in application, otherwise comment it out
#endif

#ifdef LINUX_m11
	#ifndef _GNU_SOURCE
		#define _GNU_SOURCE
	#endif
#endif


//**********************************************************************************//
//******************************  Target Applications  *****************************//
//**********************************************************************************//

// Target Application Options: MATLAB_m11
// Define one of these here if appropriate
// #define MATLAB_m11  // for screen output & exit functions
// #define MATLAB_PERSISTENT_m11	// For persistent memory between mex calls.
				// NOTE: it may be more convenient to define MATLAB_PERSISTENT_m11
				// only within mex functions that use it, rather than here

	// If using persistent memory, do something like the following:
	//
	//	// Global session pointer
	//	SESSION_m11	*sess = NULL;
	//
	// 	#ifdef MATLAB_PERSISTENT_m11
	//	void mexExitFcn(void) {
	//
	//		// free session separately to close files
	//		free_session_m11(sess, TRUE_m11);
	//
	//		// free everything else
	//		AT_free_all_m11();
	//
	//		// free globals (most already freed by AT_free_all_m11)
	//		free_globals_m11(TRUE_m11);
	//	}
	// 	#endif
	//
	//	void mexFunction(...) {
	//		...
	// 		#ifdef MATLAB_PERSISTENT_m11
	//		globals_m11 = <passed_value>;  // possibly NULL
	//		sess = <passed_value>;  // possibly NULL
	//		mexAtExit(mexExitFcn);  // register on every entry into mex
	// 		#endif
	//
	//		if (globals_m11 == NULL)
	//			initialize_medlib_m11(FALSE_m11, FALSE_m11);  // always NULL until library initialized
	//		...
	//	}


#ifdef MATLAB_PERSISTENT_m11
	#define MATLAB_m11
#endif


//**********************************************************************************//
//***********************************  Debug Modes  ********************************//
//**********************************************************************************//

// #define FN_DEBUG_m11  // uncomment to show entry into functions (and recompile medlib_m11.c)
// #define AT_DEBUG_m11  // uncomment for debug behavior in allocation tracking (and recompile medlib_m11.c)


#endif  // TARGETS_IN_m11
