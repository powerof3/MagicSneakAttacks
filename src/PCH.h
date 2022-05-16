#pragma once

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <SimpleIni.h>
#include <spdlog/sinks/basic_file_sink.h>

#define DLLEXPORT __declspec(dllexport)

namespace logger = SKSE::log;
namespace string = SKSE::stl::string;

using namespace std::literals;

namespace stl
{
	using namespace SKSE::stl;

	template <class T>
	void write_thunk_call(std::uintptr_t a_src)
	{
		auto& trampoline = SKSE::GetTrampoline();
		SKSE::AllocTrampoline(14);

		T::func = trampoline.write_call<5>(a_src, T::thunk);
	}
}

#include "Version.h"

#ifdef SKYRIM_AE
#	define OFFSET(se, ae) ae
#	define OFFSET_3(se, ae, vr) ae
#elif SKYRIMVR
#	define OFFSET(se, ae) se
#	define OFFSET_3(se, ae, vr) vr
#else
#	define OFFSET(se, ae) se
#	define OFFSET_3(se, ae, vr) se
#endif
