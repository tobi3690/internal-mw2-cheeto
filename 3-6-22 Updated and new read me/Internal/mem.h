#include <cstddef>
#include <Windows.h>
#include <process.h>
#include "lazyimporter.h"
enum info { modbase = 1, imagesize, };
#define is_bad_ptr(p)    (!is_valid_ptr(p))
#define is_valid_ptr(p) ((uintptr_t)(p) <= 0x7FFFFFFEFFFF && (uintptr_t)(p) >= 0x1000) 
namespace mem
{

	enum info { modbase = 1, imagesize, };

	extern uintptr_t modinfo(info); //gets module information when needed

	extern void memcopy(BYTE* dst, BYTE* src, size_t size);

	extern __int64 find_pattern(__int64 range_start, __int64 range_end, const char* pattern);

	template<typename T> inline auto readMemory(uintptr_t ptr) noexcept -> T {
		if (is_bad_ptr(ptr)) {
			//DEBUG_INFO("Attempted to read invalid memory at {:#x}", ptr);
			return {};
		}
		return *reinterpret_cast<T*>(ptr);
	}

	template<typename T> inline auto writeMemory(uintptr_t ptr, T value) noexcept -> T {
		if (is_bad_ptr(ptr)) {
			//DEBUG_INFO("Attempted to read invalid memory at {:#x}", ptr);
			return {};
		}
		return *reinterpret_cast<T*>(ptr) = value;
	}
}

class Scanner
{
private:
	std::ptrdiff_t _skip = 0x500000;
	std::ptrdiff_t  searchStart = (std::ptrdiff_t)mem::modinfo(mem::info::modbase) + _skip;
	std::ptrdiff_t searchEnd = searchStart + ((std::ptrdiff_t)mem::modinfo(mem::info::imagesize) - _skip);

	bool debugflag;
	const char* name = nullptr;

public:
	explicit Scanner(const char* pattern, const char* name, bool debug = false, std::ptrdiff_t offset = 0x0);

	~Scanner() {};
	std::ptrdiff_t addr = 0;

	//Scanner at(std::ptrdiff_t offset);

	auto getaddr()->std::ptrdiff_t;
	auto validaddr() -> bool;

	auto resolve_call()->std::ptrdiff_t;

	auto resolve_lea()->std::ptrdiff_t;

	auto resolve_cmp()->std::ptrdiff_t;
};

static uintptr_t __cdecl I_beginthreadex(void* _Security, unsigned _StackSize, _beginthreadex_proc_type _StartAddress, void* _ArgList, unsigned _InitFlag, unsigned* _ThrdAddr) {
	return iat(_beginthreadex).get()(_Security, _StackSize, _StartAddress, _ArgList, _InitFlag, _ThrdAddr);
}
