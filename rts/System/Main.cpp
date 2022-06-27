/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */

/**
	\mainpage
	This is the documentation of the Spring RTS Engine.
	https://springrts.com/
*/

#ifdef USE_MIMALLOC
	#undef new
	#undef delete
	#undef malloc
	#undef free
	#undef _aligned_malloc
	#undef posix_memalign
	#undef _aligned_free
	#include "mimalloc/include/mimalloc.h"
	#include "mimalloc/include/mimalloc-new-delete.h"
#endif

#include "System/ExportDefines.h"
#include "System/SpringApp.h"
#include "System/Exceptions.h"
#include "System/FileSystem/FileSystem.h"
#include "System/Platform/errorhandler.h"
#include "System/Platform/Threading.h"
#include "System/Platform/Misc.h"
#include "System/Log/ILog.h"

#include <clocale>
#include <cstdlib>
#include <cstdint>

#ifdef _WIN32
	#include "lib/SOP/SOP.hpp" // NvOptimus
#endif

// https://stackoverflow.com/a/27881472/9819318
EXTERNALIZER_B EXPORT_CLAUSE uint32_t NvOptimusEnablement =                  1; EXTERNALIZER_E //Optimus/NV use discrete GPU hint
EXTERNALIZER_B EXPORT_CLAUSE uint32_t AmdPowerXpressRequestHighPerformance = 1; EXTERNALIZER_E // AMD use discrete GPU hint

int Run(int argc, char* argv[])
{
#ifdef USE_MIMALLOC
	#ifdef _DEBUG
		mi_option_enable(mi_option_show_errors);
		mi_option_enable(mi_option_show_stats);
		mi_option_enable(mi_option_verbose);
	#else
		mi_option_disable(mi_option_show_errors);
		mi_option_disable(mi_option_show_stats);
		mi_option_disable(mi_option_verbose);
	#endif
	mi_option_enable(mi_option_large_os_pages);
	//mi_option_set(mi_option_reserve_huge_os_pages, 4);
	//mi_option_set(mi_option_eager_commit_delay, 16);
#endif

#ifdef __MINGW32__
	// For the MinGW backtrace() implementation we need to know the stack end.
	{
		extern void* stack_end;
		char here;
		stack_end = (void*) &here;
	}
#endif

	// already the default, but be explicit for locale-dependent functions (atof,strtof,...)
	setlocale(LC_ALL, "C");

	Threading::DetectCores();
	Threading::SetMainThread();

	SpringApp app(argc, argv);
	return (app.Run());
}


/**
 * Always run on dedicated GPU
 * @return true when restart is required with new env vars
 */
#if !defined(PROFILE) && !defined(HEADLESS)
static bool SetNvOptimusProfile(const std::string& processFileName)
{
#ifdef _WIN32
	if (SOP_CheckProfile("Spring"))
		return false;

	// sic; on Windows execvp spawns a new process which breaks lobby state-tracking by PID
	return (SOP_SetProfile("Spring", processFileName) == SOP_RESULT_CHANGE, false);
#endif
	return false;
}
#endif



/**
 * @brief main
 * @return exit code
 * @param argc argument count
 * @param argv array of argument strings
 *
 * Main entry point function
 */
int main(int argc, char* argv[])
{
// PROFILE builds exit on execv, HEADLESS does not use the GPU
#if !defined(PROFILE) && !defined(HEADLESS)
#define MAX_ARGS 32

	if (SetNvOptimusProfile(FileSystem::GetFilename(argv[0]))) {
		// prepare for restart
		std::array<std::string, MAX_ARGS> args;

		for (int i = 0, n = std::min(argc, MAX_ARGS); i < n; i++)
			args[i] = argv[i];

		// ExecProc normally does not return; if it does the retval is an error-string
		ErrorMessageBox(Platform::ExecuteProcess(args), "Execv error:", MBF_OK | MBF_EXCL);
	}
#undef MAX_ARGS
#endif

	return (Run(argc, argv));
}



#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstanceIn, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	return main(__argc, __argv);
}
#endif

