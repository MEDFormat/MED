
// Make this file the first include
#ifndef TARGETS_IN_m10
#define TARGETS_IN_m10

//**********************************************************************************//
//************************************  Target OS  *********************************//
//**********************************************************************************//

// Target OS Options: LINUX_m10, MACOS_m10, or WINDOWS_m10
// Define one of these here
// #define MACOS_m10
#define LINUX_m10
// #define WINDOWS_m10

#ifdef LINUX_m10
	#ifndef _GNU_SOURCE
		#define _GNU_SOURCE
	#endif
#endif

//**********************************************************************************//
//*******************************  Target Application  *****************************//
//**********************************************************************************//

// #define MATLAB_m10  // for mexPrintf() (Note: no UTF8 functionality)

#endif  // TARGETS_IN_m10
