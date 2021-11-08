#pragma once
#include <windows.h>
#include <Psapi.h>
#include <vector>

namespace Mem {
	void Patch(BYTE* dst, BYTE* src, unsigned int size);
	void Nop(BYTE* dst, unsigned int size);
	uintptr_t findSig(const char* sig);
	uintptr_t ResolveMultiLvlPtr(uintptr_t ptr, std::vector<unsigned int> offsets);
}