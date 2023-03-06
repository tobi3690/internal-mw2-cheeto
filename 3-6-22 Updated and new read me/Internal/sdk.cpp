#include "stdafx.h"
#include "sdk.h"
#include "xor.hpp"
#include "lazyimporter.h"
#include "memory.h"
#include <map>
#include "defs.h"
#include "globals.h"
#include "xorstr.hpp"
#pragma comment(lib, "user32.lib")
#define DEBASE(a) ((size_t)a - (size_t)(unsigned long long)GetModuleHandleA(NULL))

uintptr_t dwProcessBase;
uint64_t backup = 0, Online_Loot__GetItemQuantity = 0, stackFix = 0;
NTSTATUS(*NtContinue)(PCONTEXT threadContext, BOOLEAN raiseAlert) = nullptr;

DWORD64 resolveRelativeAddress(DWORD64 instr, DWORD offset, DWORD instrSize) {
	return instr == 0ui64 ? 0ui64 : (instr + instrSize + *(int*)(instr + offset));
}

bool compareByte(const char* pData, const char* bMask, const char* szMask) {
	for (; *szMask; ++szMask, ++pData, ++bMask)
		if (*szMask == 'x' && *pData != *bMask)
			return false;
	return (*szMask) == NULL;
}

DWORD64 findPattern(DWORD64 dwAddress, DWORD64 dwLen, const char* bMask, const char* szMask) {
	DWORD length = (DWORD)strlen(szMask);
	for (DWORD i = 0; i < dwLen - length; i++)
		if (compareByte((const char*)(dwAddress + i), bMask, szMask))
			return (DWORD64)(dwAddress + i);
	return 0ui64;
}

LONG WINAPI TopLevelExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo)
{
	if (pExceptionInfo && pExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_ACCESS_VIOLATION)
	{
		if (pExceptionInfo->ContextRecord->R11 == 0xDEEDBEEF89898989)
		{
			pExceptionInfo->ContextRecord->R11 = backup;

			if (pExceptionInfo->ContextRecord->Rip > Online_Loot__GetItemQuantity && pExceptionInfo->ContextRecord->Rip < (Online_Loot__GetItemQuantity + 0x1000))
			{
				pExceptionInfo->ContextRecord->Rip = stackFix;
				pExceptionInfo->ContextRecord->Rax = 1;
			}
			NtContinue(pExceptionInfo->ContextRecord, 0);
		}
	}

	return EXCEPTION_CONTINUE_SEARCH;
}


namespace process
{
	HWND hwnd;

	BOOL CALLBACK EnumWindowCallBack(HWND hWnd, LPARAM lParam)
	{
		DWORD dwPid = 0;
		GetWindowThreadProcessId(hWnd, &dwPid);
		if (dwPid == lParam)
		{
			hwnd = hWnd;
			return FALSE;
		}
		return TRUE;
	}

	HWND get_process_window()
	{
		if (hwnd)
			return hwnd;

		EnumWindows(EnumWindowCallBack, GetCurrentProcessId());

		if (hwnd == NULL)
			Exit();

		return hwnd;
	}
}

namespace g_data
{
	uintptr_t base;
	uintptr_t peb;
	HWND hWind;
	uintptr_t unlocker;
	uintptr_t ddl_loadasset;
	uintptr_t ddl_getrootstate;
	uintptr_t ddl_getdllbuffer;
	uintptr_t ddl_movetoname;
	uintptr_t ddl_setint;
	uintptr_t Dvar_FindVarByName;
	uintptr_t Dvar_SetBoolInternal;
	uintptr_t Dvar_SetInt_Internal;
	uintptr_t Dvar_SetFloat_Internal;
	uintptr_t Camo_Offset_Auto_Test;
	

	uintptr_t Clantag_auto;

	uintptr_t a_parse;
	uintptr_t ddl_setstring;
	uintptr_t ddl_movetopath;
	uintptr_t ddlgetInth;
	uintptr_t visible_base;
	QWORD current_visible_offset;
	QWORD cached_visible_base;
	QWORD last_visible_offset;
	bool MemCompare(const BYTE* bData, const BYTE* bMask, const char* szMask) {
		for (; *szMask; ++szMask, ++bData, ++bMask) {
			if (*szMask == 'x' && *bData != *bMask) {
				return false;
			}
		}
		return (*szMask == NULL);
	}
	uintptr_t PatternScanEx(uintptr_t start, uintptr_t size, const char* sig, const char* mask)
	{
		BYTE* data = new BYTE[size];
		SIZE_T bytesRead;

		(iat(memcpy).get()(data, (LPVOID)start, size));
		//ReadProcessMemory(hProcess, (LPVOID)start, data, size, &bytesRead);

		for (uintptr_t i = 0; i < size; i++)
		{
			if (MemCompare((const BYTE*)(data + i), (const BYTE*)sig, mask)) {
				return start + i;
			}
		}
		delete[] data;
		return NULL;
	}

	uintptr_t FindOffset(uintptr_t start, uintptr_t size, const char* sig, const char* mask, uintptr_t base_offset, uintptr_t pre_base_offset, uintptr_t rindex, bool addRip)
	{
		auto address = PatternScanEx(start, size, sig, mask) + rindex;
		if (!address)
			return 0;
		auto ret = pre_base_offset + *reinterpret_cast<int32_t*>(address + base_offset);

		if (addRip)
		{
			ret = ret + address;
			if (ret)
				return (ret - base);
		}

		return ret;
	}

	void init()
	{
		base = (uintptr_t)(iat(GetModuleHandleA).get()("cod.exe"));
	
		
		hWind = process::get_process_window();
		
		peb = __readgsqword(0x60);
	}
}

namespace sdk
{
	const DWORD nTickTime = 64;//64 ms
	bool bUpdateTick = false;
	std::map<DWORD, velocityInfo_t> velocityMap;

	void enable_uav()
	{

		
		auto uavptr = *(uint64_t*)(g_data::base + globals::uavbase);
		if (uavptr != 0)
		{
			//*(bool*)(uavptr + auav) = g_Vars->mSettings.b_radar;
			*(int*)(uavptr + 0x41F) = 2;
			//*(int*)(uavptr + uav + 0x5) = 6;
		}

	
	}
	uintptr_t _get_player(int i)
	{
		auto cl_info_base = get_client_info_base();

		if (is_bad_ptr(cl_info_base))return 0;
		
		
			auto base_address = *(uintptr_t*)(cl_info_base);
			if (is_bad_ptr(base_address))return 0;

				return sdk::get_client_info_base() + (i * offsets::player::size);

	}
	bool in_game()
	{
		auto gameMode = *(int*)(g_data::base + offsets::game_mode);
		return  gameMode > 1;
	}

	int get_game_mode()
	{
		return *(int*)(g_data::base + offsets::game_mode);
	}

	int get_max_player_count()
	{
		return *(int*)(g_data::base + offsets::game_mode);
	}

	Vector3 _get_pos(uintptr_t address)
	{
		auto local_pos_ptr = *(uintptr_t*)((uintptr_t)address + offsets::player::pos);

		if (local_pos_ptr)
		{
			return *(Vector3*)(local_pos_ptr + 0x40);
		}
		return Vector3{};
	}

	uint32_t _get_index(uintptr_t address)
	{
		auto cl_info_base = get_client_info_base();

		if (is_bad_ptr(cl_info_base))return 0;

		return ((uintptr_t)address - cl_info_base) / offsets::player::size;
	
		
	}

	int _team_id(uintptr_t address)    {

		return *(int*)((uintptr_t)address + offsets::player::team);
	}

	int decrypt_visible_flag(int i, QWORD valid_list)
	{
		auto ptr = valid_list + ((i + i * 8) * 8) + 0xA83; //80 BF ? ? ? ? ? 74 20 80 BF ? ? ? ? ? 74 17
		DWORD dw1 = (*(DWORD*)(ptr + 4) ^ (DWORD)ptr);
		DWORD dw2 = ((dw1 + 2) * dw1);
		BYTE dec_visible_flag = *(BYTE*)(ptr) ^ BYTE1(dw2) ^ (BYTE)dw2;

		return (int)dec_visible_flag;
	}


	bool _is_visible(uintptr_t address)
	{
		if (IsValidPtr<uintptr_t>(&g_data::visible_base))
		{
			uint64_t VisibleList = *(uint64_t*)(g_data::visible_base + 0x108);
			if (is_bad_ptr( VisibleList))
				return false;

			uint64_t rdx = VisibleList + (_get_index(address) * 9 + 0x14E) * 8;
			if (is_bad_ptr(rdx))
				return false;

			DWORD VisibleFlags = (rdx + 0x10) ^ (*(DWORD*)(rdx + 0x14));
			if (is_bad_ptr(VisibleFlags))
				return false;

			DWORD v511 = VisibleFlags * (VisibleFlags + 2);
			if (!v511)
				return false;

			BYTE VisibleFlags1 = *(DWORD*)(rdx + 0x10) ^ v511 ^ BYTE1(v511);
			if (VisibleFlags1 == 3) {
				return true;
			}
		}
		return false;
	}


	uint64_t get_client_info()
	{
		auto mb = g_data::base; // = mb
		auto Peb = __readgsqword(0x60);
		uint64_t rax = mb, rbx = mb, rcx = mb, rdx = mb, rdi = mb, rsi = mb, r8 = mb, r9 = mb, r10 = mb, r11 = mb, r12 = mb, r13 = mb, r14 = mb, r15 = mb;
		rbx = *(uintptr_t*)(mb + 0x12B75B08);
		if (!rbx)
			return rbx;
		rcx = Peb;              //mov rcx, gs:[rax]
		rax = 0;                //and rax, 0xFFFFFFFFC0000000
		rax = _rotl64(rax, 0x10);               //rol rax, 0x10
		rax ^=*(uintptr_t*)(mb + 0xA6E00D1);             //xor rax, [0x00000000084C0A79]
		rax = _byteswap_uint64(rax);            //bswap rax
		rbx *=*(uintptr_t*)(rax + 0x13);             //imul rbx, [rax+0x13]
		rax = 0xBDE35F4CBE39A3FB;               //mov rax, 0xBDE35F4CBE39A3FB
		rbx *= rax;             //imul rbx, rax
		rax = mb + 0x28A;           //lea rax, [0xFFFFFFFFFDDE0C15]
		rbx ^= rcx;             //xor rbx, rcx
		rbx ^= rax;             //xor rbx, rax
		rcx = rbx;              //mov rcx, rbx
		rcx >>= 0x4;            //shr rcx, 0x04
		rcx ^= rbx;             //xor rcx, rbx
		rax = rcx;              //mov rax, rcx
		rax >>= 0x8;            //shr rax, 0x08
		rcx ^= rax;             //xor rcx, rax
		rax = rcx;              //mov rax, rcx
		rax >>= 0x10;           //shr rax, 0x10
		rcx ^= rax;             //xor rcx, rax
		rax = 0x52F40A1F170E1F2F;               //mov rax, 0x52F40A1F170E1F2F
		rbx = rcx;              //mov rbx, rcx
		rbx >>= 0x20;           //shr rbx, 0x20
		rbx ^= rcx;             //xor rbx, rcx
		rbx *= rax;             //imul rbx, rax
		rax = mb;           //lea rax, [0xFFFFFFFFFDDE0948]
		rbx += rax;             //add rbx, rax
		return rbx;
	}
	uint64_t get_client_info_base()
	{	// Updated 12/10/22
		auto mb = g_data::base;
		auto Peb = __readgsqword(0x60);
		
		uint64_t rax = mb, rbx = mb, rcx = mb, rdx = mb, rdi = mb, rsi = mb, r8 = mb, r9 = mb, r10 = mb, r11 = mb, r12 = mb, r13 = mb, r14 = mb, r15 = mb;
		r8 =*(uintptr_t*)(get_client_info() + 0x10ecb0);
		if (!r8)
			return r8;
		rbx = ~Peb;              //mov rbx, gs:[rax]
		rax = rbx;              //mov rax, rbx
		rax = _rotr64(rax, 0xE);                //ror rax, 0x0E
		rax &= 0xF;
		switch (rax) {
		case 0:
		{
			r11 = mb + 0x3D0AD3B8;              //lea r11, [0x000000003AEED7A0]
			r9 =*(uintptr_t*)(mb + 0xA6E010E);               //mov r9, [0x0000000008520496]
			rax = 0xCB3589F3B6909ABE;               //mov rax, 0xCB3589F3B6909ABE
			r8 ^= rax;              //xor r8, rax
			rax = mb + 0x5397;          //lea rax, [0xFFFFFFFFFDE4555C]
			rax = ~rax;             //not rax
			rax ^= rbx;             //xor rax, rbx
			rax += rbx;             //add rax, rbx
			r8 += rax;              //add r8, rax
			rax = rbx;              //mov rax, rbx
			rax *= r11;             //imul rax, r11
			r8 += rax;              //add r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x16;           //shr rax, 0x16
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x2C;           //shr rax, 0x2C
			r8 ^= rax;              //xor r8, rax
			rax = 0x24998E5A0459D337;               //mov rax, 0x24998E5A0459D337
			r8 *= rax;              //imul r8, rax
			rax = 0x4B602E07925249F1;               //mov rax, 0x4B602E07925249F1
			r8 *= rax;              //imul r8, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r9;              //xor rax, r9
			rax = _byteswap_uint64(rax);            //bswap rax
			r8 *=*(uintptr_t*)(rax + 0x13);              //imul r8, [rax+0x13]
			return r8;
		}
		case 1:
		{
			r9 =*(uintptr_t*)(mb + 0xA6E010E);               //mov r9, [0x000000000851FFC0]
			rax = mb;           //lea rax, [0xFFFFFFFFFDE3FE34]
			r8 -= rax;              //sub r8, rax
			r8 -= rbx;              //sub r8, rbx
			rax = mb;           //lea rax, [0xFFFFFFFFFDE3FC00]
			r8 -= rax;              //sub r8, rax
			rax = 0xACE5F1EC91AF00A9;               //mov rax, 0xACE5F1EC91AF00A9
			r8 *= rax;              //imul r8, rax
			rax = 0x3AA9CF24309B1B56;               //mov rax, 0x3AA9CF24309B1B56
			r8 += rax;              //add r8, rax
			r8 ^= rbx;              //xor r8, rbx
			r8 += rbx;              //add r8, rbx
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r9;              //xor rax, r9
			rax = _byteswap_uint64(rax);            //bswap rax
			r8 *=*(uintptr_t*)(rax + 0x13);              //imul r8, [rax+0x13]
			rax = r8;               //mov rax, r8
			rax >>= 0x26;           //shr rax, 0x26
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x9;            //shr rax, 0x09
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x12;           //shr rax, 0x12
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x24;           //shr rax, 0x24
			r8 ^= rax;              //xor r8, rax
			return r8;
		}
		case 2:
		{
			r9 =*(uintptr_t*)(mb + 0xA6E010E);               //mov r9, [0x000000000851FB38]
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r9;              //xor rax, r9
			rax = _byteswap_uint64(rax);            //bswap rax
			r8 *=*(uintptr_t*)(rax + 0x13);              //imul r8, [rax+0x13]
			rax = r8;               //mov rax, r8
			rax >>= 0xD;            //shr rax, 0x0D
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x1A;           //shr rax, 0x1A
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x34;           //shr rax, 0x34
			r8 ^= rax;              //xor r8, rax
			rax = rbx;              //mov rax, rbx
			uintptr_t RSP_0x50;
			RSP_0x50 = mb + 0x7535;             //lea rax, [0xFFFFFFFFFDE46FDE] : RSP+0x50
			rax *= RSP_0x50;                //imul rax, [rsp+0x50]
			r8 += rax;              //add r8, rax
			r8 -= rbx;              //sub r8, rbx
			rax = r8;               //mov rax, r8
			rax >>= 0x1F;           //shr rax, 0x1F
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x3E;           //shr rax, 0x3E
			r8 ^= rax;              //xor r8, rax
			rax = 0xC2808E17980A84D5;               //mov rax, 0xC2808E17980A84D5
			r8 *= rax;              //imul r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x9;            //shr rax, 0x09
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x12;           //shr rax, 0x12
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x24;           //shr rax, 0x24
			r8 ^= rax;              //xor r8, rax
			rax = 0x5266478A1D37203D;               //mov rax, 0x5266478A1D37203D
			r8 *= rax;              //imul r8, rax
			return r8;
		}
		case 3:
		{
			r10 =*(uintptr_t*)(mb + 0xA6E010E);              //mov r10, [0x000000000851F651]
			rcx = mb + 0x74FB260B;              //lea rcx, [0x0000000072DF1AD7]
			rax = rbx;              //mov rax, rbx
			rax ^= rcx;             //xor rax, rcx
			r8 -= rax;              //sub r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x23;           //shr rax, 0x23
			r8 ^= rax;              //xor r8, rax
			rax = rbx;              //mov rax, rbx
			rax = ~rax;             //not rax
			r8 ^= rax;              //xor r8, rax
			rax = mb + 0xF8;            //lea rax, [0xFFFFFFFFFDE3F27E]
			r8 ^= rax;              //xor r8, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r10;             //xor rax, r10
			rax = _byteswap_uint64(rax);            //bswap rax
			r8 *=*(uintptr_t*)(rax + 0x13);              //imul r8, [rax+0x13]
			rax = 0xA5CC4ADBA85C0DD7;               //mov rax, 0xA5CC4ADBA85C0DD7
			r8 *= rax;              //imul r8, rax
			rax = 0x52DC2D05AED106C8;               //mov rax, 0x52DC2D05AED106C8
			r8 += rax;              //add r8, rax
			rax = mb;           //lea rax, [0xFFFFFFFFFDE3F20C]
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0xA;            //shr rax, 0x0A
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x14;           //shr rax, 0x14
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x28;           //shr rax, 0x28
			r8 ^= rax;              //xor r8, rax
			return r8;
		}
		case 4:
		{
			r11 =*(uintptr_t*)(mb + 0xA6E010E);              //mov r11, [0x000000000851F15E]
			rax = rbx;              //mov rax, rbx
			uintptr_t RSP_0x58;
			RSP_0x58 = mb + 0x4C782F92;                 //lea rax, [0x000000004A5C1FB7] : RSP+0x58
			rax *= RSP_0x58;                //imul rax, [rsp+0x58]
			r8 ^= rax;              //xor r8, rax
			r8 ^= rbx;              //xor r8, rbx
			rax = 0x191250F2E60FEBFB;               //mov rax, 0x191250F2E60FEBFB
			r8 *= rax;              //imul r8, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r11;             //xor rax, r11
			rax = _byteswap_uint64(rax);            //bswap rax
			r8 *=*(uintptr_t*)(rax + 0x13);              //imul r8, [rax+0x13]
			rax = rbx;              //mov rax, rbx
			uintptr_t RSP_0x50;
			RSP_0x50 = mb + 0xA33;              //lea rax, [0xFFFFFFFFFDE3FA13] : RSP+0x50
			rax ^= RSP_0x50;                //xor rax, [rsp+0x50]
			r8 -= rax;              //sub r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x17;           //shr rax, 0x17
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x2E;           //shr rax, 0x2E
			r8 ^= rax;              //xor r8, rax
			rax = 0x4B17ED5B5B631C6;                //mov rax, 0x4B17ED5B5B631C6
			r8 ^= rax;              //xor r8, rax
			return r8;
		}
		case 5:
		{
			r10 =*(uintptr_t*)(mb + 0xA6E010E);              //mov r10, [0x000000000851ECA2]
			rax = r8;               //mov rax, r8
			rax >>= 0x25;           //shr rax, 0x25
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x22;           //shr rax, 0x22
			r8 ^= rax;              //xor r8, rax
			r8 -= rbx;              //sub r8, rbx
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r10;             //xor rax, r10
			rax = _byteswap_uint64(rax);            //bswap rax
			r8 *=*(uintptr_t*)(rax + 0x13);              //imul r8, [rax+0x13]
			rax = 0xC169708AB23490A3;               //mov rax, 0xC169708AB23490A3
			r8 *= rax;              //imul r8, rax
			rax = 0x2E7119275475DE6F;               //mov rax, 0x2E7119275475DE6F
			r8 *= rax;              //imul r8, rax
			r8 ^= rbx;              //xor r8, rbx
			rax = 0x5732C550EF15CA6F;               //mov rax, 0x5732C550EF15CA6F
			r8 *= rax;              //imul r8, rax
			return r8;
		}
		case 6:
		{
			r11 = mb + 0x3EF050C1;              //lea r11, [0x000000003CD43725]
			r9 =*(uintptr_t*)(mb + 0xA6E010E);               //mov r9, [0x000000000851E717]
			rax = r8;               //mov rax, r8
			rax >>= 0x17;           //shr rax, 0x17
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x2E;           //shr rax, 0x2E
			r8 ^= rax;              //xor r8, rax
			r8 -= rbx;              //sub r8, rbx
			rax = 0x4D54041807EA848A;               //mov rax, 0x4D54041807EA848A
			r8 += rax;              //add r8, rax
			rax = mb;           //lea rax, [0xFFFFFFFFFDE3E440]
			r8 += rax;              //add r8, rax
			rax = 0x1834E8F309A4E725;               //mov rax, 0x1834E8F309A4E725
			r8 *= rax;              //imul r8, rax
			rax = 0x2EC43430C0E9D8A6;               //mov rax, 0x2EC43430C0E9D8A6
			r8 += rax;              //add r8, rax
			rax = rbx;              //mov rax, rbx
			rax = ~rax;             //not rax
			r8 ^= rax;              //xor r8, rax
			r8 ^= r11;              //xor r8, r11
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r9;              //xor rax, r9
			rax = _byteswap_uint64(rax);            //bswap rax
			r8 *=*(uintptr_t*)(rax + 0x13);              //imul r8, [rax+0x13]
			return r8;
		}
		case 7:
		{
			r9 =*(uintptr_t*)(mb + 0xA6E010E);               //mov r9, [0x000000000851E207]
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r9;              //xor rax, r9
			rax = _byteswap_uint64(rax);            //bswap rax
			r8 *=*(uintptr_t*)(rax + 0x13);              //imul r8, [rax+0x13]
			rax = 0xEB0EE89227112CCF;               //mov rax, 0xEB0EE89227112CCF
			r8 *= rax;              //imul r8, rax
			rax = mb;           //lea rax, [0xFFFFFFFFFDE3DF34]
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0xE;            //shr rax, 0x0E
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x1C;           //shr rax, 0x1C
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x38;           //shr rax, 0x38
			r8 ^= rax;              //xor r8, rax
			rax = mb;           //lea rax, [0xFFFFFFFFFDE3E068]
			r8 ^= rax;              //xor r8, rax
			rax = mb;           //lea rax, [0xFFFFFFFFFDE3DEF7]
			r8 ^= rax;              //xor r8, rax
			rax = mb;           //lea rax, [0xFFFFFFFFFDE3DFD4]
			r8 ^= rax;              //xor r8, rax
			r8 -= rbx;              //sub r8, rbx
			return r8;
		}
		case 8:
		{
			r10 =*(uintptr_t*)(mb + 0xA6E010E);              //mov r10, [0x000000000851DCE0]
			rax = mb + 0xCEE;           //lea rax, [0xFFFFFFFFFDE3E813]
			rcx = rbx + 0x1;                //lea rcx, [rbx+0x01]
			rcx *= rax;             //imul rcx, rax
			rax = rbx;              //mov rax, rbx
			rax = ~rax;             //not rax
			rcx += rax;             //add rcx, rax
			rax = mb;           //lea rax, [0xFFFFFFFFFDE3DB0D]
			rcx -= rax;             //sub rcx, rax
			rcx += 0xFFFFFFFFA2B008D0;              //add rcx, 0xFFFFFFFFA2B008D0
			r8 += rcx;              //add r8, rcx
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r10;             //xor rax, r10
			rax = _byteswap_uint64(rax);            //bswap rax
			r8 *=*(uintptr_t*)(rax + 0x13);              //imul r8, [rax+0x13]
			rax = mb + 0x34B463EE;              //lea rax, [0x0000000032983E43]
			rax = ~rax;             //not rax
			rax -= rbx;             //sub rax, rbx
			r8 += rax;              //add r8, rax
			rax = 0x1210AC6B8798FB79;               //mov rax, 0x1210AC6B8798FB79
			r8 *= rax;              //imul r8, rax
			rcx = rbx;              //mov rcx, rbx
			rcx = ~rcx;             //not rcx
			rax = mb + 0x39E0C9A2;              //lea rax, [0x0000000037C4A130]
			rax = ~rax;             //not rax
			rcx += rax;             //add rcx, rax
			r8 ^= rcx;              //xor r8, rcx
			rax = r8;               //mov rax, r8
			rax >>= 0x24;           //shr rax, 0x24
			r8 ^= rax;              //xor r8, rax
			rax = rbx;              //mov rax, rbx
			rax = ~rax;             //not rax
			uintptr_t RSP_0xFFFFFFFFFFFFFF80;
			RSP_0xFFFFFFFFFFFFFF80 = mb + 0xEB70;               //lea rax, [0xFFFFFFFFFDE4C798] : RBP+0xFFFFFFFFFFFFFF80
			rax ^= RSP_0xFFFFFFFFFFFFFF80;          //xor rax, [rbp-0x80]
			r8 -= rax;              //sub r8, rax
			return r8;
		}
		case 9:
		{
			r9 =*(uintptr_t*)(mb + 0xA6E010E);               //mov r9, [0x000000000851D87A]
			rax = 0xFFFFFFFFCAC57E77;               //mov rax, 0xFFFFFFFFCAC57E77
			rax -= rbx;             //sub rax, rbx
			rax -= mb;          //sub rax, [rsp+0x50] -- didn't find trace -> use base
			r8 += rax;              //add r8, rax
			rax = mb;           //lea rax, [0xFFFFFFFFFDE3D440]
			rax += 0xAF88;          //add rax, 0xAF88
			r8 += rax;              //add r8, rax
			rax = 0xE1ACFAB93491129E;               //mov rax, 0xE1ACFAB93491129E
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x6;            //shr rax, 0x06
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0xC;            //shr rax, 0x0C
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x18;           //shr rax, 0x18
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x30;           //shr rax, 0x30
			r8 ^= rax;              //xor r8, rax
			rax = 0x15B4A934853F82C5;               //mov rax, 0x15B4A934853F82C5
			r8 += rax;              //add r8, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r9;              //xor rax, r9
			rax = _byteswap_uint64(rax);            //bswap rax
			rax =*(uintptr_t*)(rax + 0x13);              //mov rax, [rax+0x13]
			uintptr_t RSP_0x58;
			RSP_0x58 = 0x8B2C48358030B3D9;          //mov rax, 0x8B2C48358030B3D9 : RSP+0x58
			rax *= RSP_0x58;                //imul rax, [rsp+0x58]
			r8 *= rax;              //imul r8, rax
			return r8;
		}
		case 10:
		{
			r10 =*(uintptr_t*)(mb + 0xA6E010E);              //mov r10, [0x000000000851D2F5]
			rcx = mb;           //lea rcx, [0xFFFFFFFFFDE3CF77]
			rax = 0x609D23BAD8D3B50D;               //mov rax, 0x609D23BAD8D3B50D
			rcx -= rbx;             //sub rcx, rbx
			r8 += rax;              //add r8, rax
			r8 += rcx;              //add r8, rcx
			rax = 0xF980EEAE131B7607;               //mov rax, 0xF980EEAE131B7607
			r8 *= rax;              //imul r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0xA;            //shr rax, 0x0A
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x14;           //shr rax, 0x14
			r8 ^= rax;              //xor r8, rax
			rcx = r8;               //mov rcx, r8
			rcx >>= 0x28;           //shr rcx, 0x28
			rcx ^= r8;              //xor rcx, r8
			r8 = mb + 0x1B4;            //lea r8, [0xFFFFFFFFFDE3CF6B]
			r8 = ~r8;               //not r8
			r8 *= rbx;              //imul r8, rbx
			r8 += rcx;              //add r8, rcx
			rax = 0xFF6FF9E6D6FB4711;               //mov rax, 0xFF6FF9E6D6FB4711
			rax += r8;              //add rax, r8
			r8 = mb;            //lea r8, [0xFFFFFFFFFDE3CFF0]
			r8 += rax;              //add r8, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r10;             //xor rax, r10
			rax = _byteswap_uint64(rax);            //bswap rax
			r8 *=*(uintptr_t*)(rax + 0x13);              //imul r8, [rax+0x13]
			return r8;
		}
		case 11:
		{
			r10 =*(uintptr_t*)(mb + 0xA6E010E);              //mov r10, [0x000000000851CD23]
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r10;             //xor rax, r10
			rax = _byteswap_uint64(rax);            //bswap rax
			rax =*(uintptr_t*)(rax + 0x13);              //mov rax, [rax+0x13]
			uintptr_t RSP_0xFFFFFFFFFFFFFFA0;
			RSP_0xFFFFFFFFFFFFFFA0 = 0xD7BA1614E582675D;            //mov rax, 0xD7BA1614E582675D : RBP+0xFFFFFFFFFFFFFFA0
			rax *= RSP_0xFFFFFFFFFFFFFFA0;          //imul rax, [rbp-0x60]
			r8 *= rax;              //imul r8, rax
			rcx = rbx;              //mov rcx, rbx
			rcx = ~rcx;             //not rcx
			rax = mb + 0x32B9;          //lea rax, [0xFFFFFFFFFDE3FC5B]
			rax = ~rax;             //not rax
			rcx += rax;             //add rcx, rax
			r8 ^= rcx;              //xor r8, rcx
			rax = 0xEB87BAA323C88987;               //mov rax, 0xEB87BAA323C88987
			r8 *= rax;              //imul r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x17;           //shr rax, 0x17
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x2E;           //shr rax, 0x2E
			r8 ^= rax;              //xor r8, rax
			rax = mb;           //lea rax, [0xFFFFFFFFFDE3C875]
			r8 += rax;              //add r8, rax
			rax = mb + 0xD5A2;          //lea rax, [0xFFFFFFFFFDE49DB6]
			rax = ~rax;             //not rax
			rax *= rbx;             //imul rax, rbx
			r8 ^= rax;              //xor r8, rax
			rax = 0x8E49AEB43EDD907D;               //mov rax, 0x8E49AEB43EDD907D
			r8 *= rax;              //imul r8, rax
			return r8;
		}
		case 12:
		{
			r10 =*(uintptr_t*)(mb + 0xA6E010E);              //mov r10, [0x000000000851C81B]
			rax = mb + 0x73B2;          //lea rax, [0xFFFFFFFFFDE439CE]
			rax -= rbx;             //sub rax, rbx
			r8 += rax;              //add r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x1;            //shr rax, 0x01
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x2;            //shr rax, 0x02
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x4;            //shr rax, 0x04
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x8;            //shr rax, 0x08
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x10;           //shr rax, 0x10
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x20;           //shr rax, 0x20
			r8 ^= rax;              //xor r8, rax
			rax = 0x6B91C8A9C9BE5F93;               //mov rax, 0x6B91C8A9C9BE5F93
			r8 ^= rax;              //xor r8, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r10;             //xor rax, r10
			rax = _byteswap_uint64(rax);            //bswap rax
			r8 *=*(uintptr_t*)(rax + 0x13);              //imul r8, [rax+0x13]
			rax = 0xDB022E3A14D36F19;               //mov rax, 0xDB022E3A14D36F19
			r8 *= rax;              //imul r8, rax
			rax = 0xB5AD6003DB0EFA26;               //mov rax, 0xB5AD6003DB0EFA26
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x21;           //shr rax, 0x21
			r8 ^= rax;              //xor r8, rax
			rax = rbx;              //mov rax, rbx
			uintptr_t RSP_0xFFFFFFFFFFFFFFA0;
			RSP_0xFFFFFFFFFFFFFFA0 = mb + 0x618D3CC6;           //lea rax, [0x000000005F71045A] : RBP+0xFFFFFFFFFFFFFFA0
			rax *= RSP_0xFFFFFFFFFFFFFFA0;          //imul rax, [rbp-0x60]
			r8 += rax;              //add r8, rax
			return r8;
		}
		case 13:
		{
			r9 =*(uintptr_t*)(mb + 0xA6E010E);               //mov r9, [0x000000000851C25A]
			r8 ^= rbx;              //xor r8, rbx
			rax = r8;               //mov rax, r8
			rax >>= 0x12;           //shr rax, 0x12
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x24;           //shr rax, 0x24
			r8 ^= rax;              //xor r8, rax
			r8 += rbx;              //add r8, rbx
			rax = 0x88A7DDB6DE4103B9;               //mov rax, 0x88A7DDB6DE4103B9
			r8 *= rax;              //imul r8, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r9;              //xor rax, r9
			rax = _byteswap_uint64(rax);            //bswap rax
			r8 *=*(uintptr_t*)(rax + 0x13);              //imul r8, [rax+0x13]
			rax = rbx;              //mov rax, rbx
			rax -= mb;          //sub rax, [rsp+0x50] -- didn't find trace -> use base
			rax += 0xFFFFFFFFFFFF87A0;              //add rax, 0xFFFFFFFFFFFF87A0
			r8 += rax;              //add r8, rax
			rax = 0xE69460A1C31CE237;               //mov rax, 0xE69460A1C31CE237
			r8 *= rax;              //imul r8, rax
			r8 += rbx;              //add r8, rbx
			return r8;
		}
		case 14:
		{
			r9 =*(uintptr_t*)(mb + 0xA6E010E);               //mov r9, [0x000000000851BE05]
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r9;              //xor rax, r9
			rax = _byteswap_uint64(rax);            //bswap rax
			r8 *=*(uintptr_t*)(rax + 0x13);              //imul r8, [rax+0x13]
			rax = rbx;              //mov rax, rbx
			uintptr_t RSP_0x50;
			RSP_0x50 = mb + 0x944A;             //lea rax, [0xFFFFFFFFFDE4516B] : RSP+0x50
			rax *= RSP_0x50;                //imul rax, [rsp+0x50]
			r8 -= rax;              //sub r8, rax
			rax = 0xD51D5FD568D85813;               //mov rax, 0xD51D5FD568D85813
			r8 *= rax;              //imul r8, rax
			rax = mb + 0x6F53;          //lea rax, [0xFFFFFFFFFDE42ADA]
			rax = ~rax;             //not rax
			rax ^= rbx;             //xor rax, rbx
			r8 -= rax;              //sub r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x23;           //shr rax, 0x23
			r8 ^= rax;              //xor r8, rax
			rax = rbx;              //mov rax, rbx
			uintptr_t RSP_0xFFFFFFFFFFFFFF88;
			RSP_0xFFFFFFFFFFFFFF88 = mb + 0x479826F2;           //lea rax, [0x00000000457BE44F] : RBP+0xFFFFFFFFFFFFFF88
			rax ^= RSP_0xFFFFFFFFFFFFFF88;          //xor rax, [rbp-0x78]
			r8 += rax;              //add r8, rax
			rax = 0xB0E981C17C84CDAE;               //mov rax, 0xB0E981C17C84CDAE
			r8 ^= rax;              //xor r8, rax
			return r8;
		}
		case 15:
		{
			r10 =*(uintptr_t*)(mb + 0xA6E010E);              //mov r10, [0x000000000851B9D8]
			r14 = mb + 0xBCE0;          //lea r14, [0xFFFFFFFFFDE4758B]
			rax = mb;           //lea rax, [0xFFFFFFFFFDE3B625]
			r8 -= rax;              //sub r8, rax
			rax = 0x1400117508C19F2E;               //mov rax, 0x1400117508C19F2E
			r8 += rax;              //add r8, rax
			rax = 0x68948AE0D9FB65AF;               //mov rax, 0x68948AE0D9FB65AF
			r8 *= rax;              //imul r8, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r10;             //xor rax, r10
			rax = _byteswap_uint64(rax);            //bswap rax
			r8 *=*(uintptr_t*)(rax + 0x13);              //imul r8, [rax+0x13]
			rax = rbx;              //mov rax, rbx
			rax ^= r14;             //xor rax, r14
			r8 += rax;              //add r8, rax
			rax = rbx;              //mov rax, rbx
			rax = ~rax;             //not rax
			uintptr_t RSP_0x60;
			RSP_0x60 = mb + 0x5601DBC3;                 //lea rax, [0x0000000053E59486] : RSP+0x60
			rax += RSP_0x60;                //add rax, [rsp+0x60]
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x15;           //shr rax, 0x15
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x2A;           //shr rax, 0x2A
			r8 ^= rax;              //xor r8, rax
			return r8;
		}
		}
	}
	uint64_t get_bone_ptr()
	{	// Updated 12/10/22
		auto mb = g_data::base;
		auto Peb = __readgsqword(0x60);
		uint64_t rax = mb, rbx = mb, rcx = mb, rdx = mb, rdi = mb, rsi = mb, r8 = mb, r9 = mb, r10 = mb, r11 = mb, r12 = mb, r13 = mb, r14 = mb, r15 = mb;
		rdx = *(uintptr_t*)(mb + 0xDC82C88);
		if (!rdx)
			return rdx;
		r10 = Peb;              //mov r10, gs:[rax]
		rax = r10;              //mov rax, r10
		rax = _rotl64(rax, 0x2F);               //rol rax, 0x2F
		rax &= 0xF;
		switch (rax) {
		case 0:
		{
			r9 = *(uintptr_t*)(mb + 0xA6E01F8);               //mov r9, [0x0000000007913FCF]
			rax = mb;           //lea rax, [0xFFFFFFFFFD233AA5]
			rax += 0x2BB36F2C;              //add rax, 0x2BB36F2C
			rax += r10;             //add rax, r10
			rdx += rax;             //add rdx, rax
			rax = 0xFE4167EB815E2917;               //mov rax, 0xFE4167EB815E2917
			rdx *= rax;             //imul rdx, rax
			rax = mb;           //lea rax, [0xFFFFFFFFFD2339C7]
			rdx -= rax;             //sub rdx, rax
			rdx += 0xFFFFFFFFFFFFF2A2;              //add rdx, 0xFFFFFFFFFFFFF2A2
			rdx += r10;             //add rdx, r10
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0xD;            //shr rax, 0x0D
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x1A;           //shr rax, 0x1A
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x34;           //shr rax, 0x34
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x21;           //shr rax, 0x21
			rdx ^= rax;             //xor rdx, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r9;              //xor rax, r9
			rax = _byteswap_uint64(rax);            //bswap rax
			rdx *= *(uintptr_t*)(rax + 0x9);              //imul rdx, [rax+0x09]
			rax = 0x6DE496D4DEF5703B;               //mov rax, 0x6DE496D4DEF5703B
			rdx += rax;             //add rdx, rax
			return rdx;
		}
		case 1:
		{
			r8 = *(uintptr_t*)(mb + 0xA6E01F8);               //mov r8, [0x0000000007913A5B]
			rdx -= r10;             //sub rdx, r10
			rax = 0x146C6FDFEFC941AB;               //mov rax, 0x146C6FDFEFC941AB
			rdx ^= rax;             //xor rdx, rax
			rax = 0xDD3F653C73686E25;               //mov rax, 0xDD3F653C73686E25
			rdx *= rax;             //imul rdx, rax
			rax = mb;           //lea rax, [0xFFFFFFFFFD233514]
			rdx ^= rax;             //xor rdx, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r8;              //xor rax, r8
			rax = _byteswap_uint64(rax);            //bswap rax
			rdx *= *(uintptr_t*)(rax + 0x9);              //imul rdx, [rax+0x09]
			rax = rdx;              //mov rax, rdx
			rax >>= 0x1F;           //shr rax, 0x1F
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x3E;           //shr rax, 0x3E
			rdx ^= rax;             //xor rdx, rax
			return rdx;
		}
		case 2:
		{
			r12 = mb + 0x558C6586;              //lea r12, [0x0000000052AF999C]
			r8 = *(uintptr_t*)(mb + 0xA6E01F8);               //mov r8, [0x0000000007913560]
			rax = rdx;              //mov rax, rdx
			rax >>= 0x25;           //shr rax, 0x25
			rdx ^= rax;             //xor rdx, rax
			rax = 0xCC3D143FEB7FE171;               //mov rax, 0xCC3D143FEB7FE171
			rdx *= rax;             //imul rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x2;            //shr rax, 0x02
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x4;            //shr rax, 0x04
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x8;            //shr rax, 0x08
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x10;           //shr rax, 0x10
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x20;           //shr rax, 0x20
			rdx ^= rax;             //xor rdx, rax
			rax = 0xFBC7BABBDDF495CF;               //mov rax, 0xFBC7BABBDDF495CF
			rdx *= rax;             //imul rdx, rax
			rax = r10;              //mov rax, r10
			rax = ~rax;             //not rax
			rax ^= r12;             //xor rax, r12
			rdx += rax;             //add rdx, rax
			rdx ^= r10;             //xor rdx, r10
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r8;              //xor rax, r8
			rax = _byteswap_uint64(rax);            //bswap rax
			rax = *(uintptr_t*)(rax + 0x9);               //mov rax, [rax+0x09]
			uintptr_t RSP_0x70;
			RSP_0x70 = 0xB867E42E5E6B124F;          //mov rax, 0xB867E42E5E6B124F : RSP+0x70
			rax *= RSP_0x70;                //imul rax, [rsp+0x70]
			rdx *= rax;             //imul rdx, rax
			return rdx;
		}
		case 3:
		{
			r13 = mb + 0x1252F371;              //lea r13, [0x000000000F762229]
			r12 = mb + 0x37C142F1;              //lea r12, [0x0000000034E4719A]
			r8 = *(uintptr_t*)(mb + 0xA6E01F8);               //mov r8, [0x0000000007913014]
			rax = 0x910204116763E85B;               //mov rax, 0x910204116763E85B
			rdx *= rax;             //imul rdx, rax
			rax = r10;              //mov rax, r10
			rax *= r12;             //imul rax, r12
			rdx ^= rax;             //xor rdx, rax
			rax = 0x7031022422DF0B00;               //mov rax, 0x7031022422DF0B00
			rdx ^= rax;             //xor rdx, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r8;              //xor rax, r8
			rax = _byteswap_uint64(rax);            //bswap rax
			rdx *= *(uintptr_t*)(rax + 0x9);              //imul rdx, [rax+0x09]
			rax = rdx;              //mov rax, rdx
			rax >>= 0x28;           //shr rax, 0x28
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x1B;           //shr rax, 0x1B
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x36;           //shr rax, 0x36
			rdx ^= rax;             //xor rdx, rax
			rax = r10;              //mov rax, r10
			rax ^= r13;             //xor rax, r13
			rdx -= rax;             //sub rdx, rax
			rax = mb;           //lea rax, [0xFFFFFFFFFD232DA1]
			rax += 0xE0C1;          //add rax, 0xE0C1
			rax += r10;             //add rax, r10
			rdx ^= rax;             //xor rdx, rax
			return rdx;
		}
		case 4:
		{
			r13 = mb + 0x6123B610;              //lea r13, [0x000000005E46E016]
			r9 = *(uintptr_t*)(mb + 0xA6E01F8);               //mov r9, [0x0000000007912BA4]
			rax = rdx;              //mov rax, rdx
			rax >>= 0x1A;           //shr rax, 0x1A
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x34;           //shr rax, 0x34
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x1F;           //shr rax, 0x1F
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x3E;           //shr rax, 0x3E
			rdx ^= rax;             //xor rdx, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r9;              //xor rax, r9
			rax = _byteswap_uint64(rax);            //bswap rax
			rdx *= *(uintptr_t*)(rax + 0x9);              //imul rdx, [rax+0x09]
			rax = 0x4542C3F7F908A81B;               //mov rax, 0x4542C3F7F908A81B
			rdx += rax;             //add rdx, rax
			rax = 0x23B9828AC79FD4CB;               //mov rax, 0x23B9828AC79FD4CB
			rdx -= rax;             //sub rdx, rax
			rax = 0xA1C16A3DCA038C15;               //mov rax, 0xA1C16A3DCA038C15
			rdx *= rax;             //imul rdx, rax
			rax = mb + 0xE6CE;          //lea rax, [0xFFFFFFFFFD240EDA]
			rdx += rax;             //add rdx, rax
			rcx = r10 + 0x1;                //lea rcx, [r10+0x01]
			rcx *= r13;             //imul rcx, r13
			rcx += r10;             //add rcx, r10
			rdx += rcx;             //add rdx, rcx
			return rdx;
		}
		case 5:
		{
			r13 = mb + 0x7D537E9C;              //lea r13, [0x000000007A76A36D]
			r8 = *(uintptr_t*)(mb + 0xA6E01F8);               //mov r8, [0x0000000007912612]
			rdx += r10;             //add rdx, r10
			rax = 0x5E3097B90351365D;               //mov rax, 0x5E3097B90351365D
			rdx *= rax;             //imul rdx, rax
			rdx ^= r10;             //xor rdx, r10
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r8;              //xor rax, r8
			rax = _byteswap_uint64(rax);            //bswap rax
			rdx *= *(uintptr_t*)(rax + 0x9);              //imul rdx, [rax+0x09]
			rax = rdx;              //mov rax, rdx
			rax >>= 0x9;            //shr rax, 0x09
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x12;           //shr rax, 0x12
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x24;           //shr rax, 0x24
			rdx ^= rax;             //xor rdx, rax
			rax = 0x6792AFC7E277CEBB;               //mov rax, 0x6792AFC7E277CEBB
			rdx *= rax;             //imul rdx, rax
			rax = r10;              //mov rax, r10
			rax *= r13;             //imul rax, r13
			rdx ^= rax;             //xor rdx, rax
			rax = r10;              //mov rax, r10
			rax -= mb;          //sub rax, [rbp-0x58] -- didn't find trace -> use base
			rax += 0xFFFFFFFFBC20342A;              //add rax, 0xFFFFFFFFBC20342A
			rdx += rax;             //add rdx, rax
			return rdx;
		}
		case 6:
		{
			r9 = *(uintptr_t*)(mb + 0xA6E01F8);               //mov r9, [0x0000000007912153]
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r9;              //xor rax, r9
			rax = _byteswap_uint64(rax);            //bswap rax
			rdx *= *(uintptr_t*)(rax + 0x9);              //imul rdx, [rax+0x09]
			rcx = r10;              //mov rcx, r10
			rcx = ~rcx;             //not rcx
			rax = mb + 0x14AE;          //lea rax, [0xFFFFFFFFFD23318C]
			rax = ~rax;             //not rax
			rcx += rax;             //add rcx, rax
			rdx ^= rcx;             //xor rdx, rcx
			rax = rdx;              //mov rax, rdx
			rax >>= 0x15;           //shr rax, 0x15
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x2A;           //shr rax, 0x2A
			rdx ^= rax;             //xor rdx, rax
			rax = 0x2F93AED43CEE423C;               //mov rax, 0x2F93AED43CEE423C
			rdx ^= rax;             //xor rdx, rax
			rdx ^= r10;             //xor rdx, r10
			rax = 0x1E736ECD9542AA11;               //mov rax, 0x1E736ECD9542AA11
			rdx *= rax;             //imul rdx, rax
			rax = 0x6FAC73171ED37ECC;               //mov rax, 0x6FAC73171ED37ECC
			rdx += rax;             //add rdx, rax
			rax = mb;           //lea rax, [0xFFFFFFFFFD231E82]
			rdx += rax;             //add rdx, rax
			return rdx;
		}
		case 7:
		{
			r9 = *(uintptr_t*)(mb + 0xA6E01F8);               //mov r9, [0x0000000007911C9A]
			rax = rdx;              //mov rax, rdx
			rax >>= 0x24;           //shr rax, 0x24
			rdx ^= rax;             //xor rdx, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r9;              //xor rax, r9
			rax = _byteswap_uint64(rax);            //bswap rax
			rdx *= *(uintptr_t*)(rax + 0x9);              //imul rdx, [rax+0x09]
			rcx = mb;           //lea rcx, [0xFFFFFFFFFD2318AF]
			rcx -= r10;             //sub rcx, r10
			rax = 0xC33263AEF789DF81;               //mov rax, 0xC33263AEF789DF81
			rdx += rax;             //add rdx, rax
			rdx += rcx;             //add rdx, rcx
			rax = 0xEADF08D37CF197F1;               //mov rax, 0xEADF08D37CF197F1
			rdx *= rax;             //imul rdx, rax
			rdx ^= r10;             //xor rdx, r10
			rdx += r10;             //add rdx, r10
			rax = mb + 0x5593D02D;              //lea rax, [0x0000000052B6E9AC]
			rdx += rax;             //add rdx, rax
			rax = 0xB0F13D4372CD2C25;               //mov rax, 0xB0F13D4372CD2C25
			rdx *= rax;             //imul rdx, rax
			return rdx;
		}
		case 8:
		{
			r14 = mb + 0xC5DD;          //lea r14, [0xFFFFFFFFFD23DCC0]
			r8 = *(uintptr_t*)(mb + 0xA6E01F8);               //mov r8, [0x0000000007911842]
			rax = r14;              //mov rax, r14
			rax = ~rax;             //not rax
			rax *= r10;             //imul rax, r10
			rdx += rax;             //add rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x9;            //shr rax, 0x09
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x12;           //shr rax, 0x12
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x24;           //shr rax, 0x24
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0xD;            //shr rax, 0x0D
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x1A;           //shr rax, 0x1A
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x34;           //shr rax, 0x34
			rdx ^= rax;             //xor rdx, rax
			rax = 0x55AFF10999C86D01;               //mov rax, 0x55AFF10999C86D01
			rdx ^= r10;             //xor rdx, r10
			rdx *= rax;             //imul rdx, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r8;              //xor rax, r8
			rax = _byteswap_uint64(rax);            //bswap rax
			rdx *= *(uintptr_t*)(rax + 0x9);              //imul rdx, [rax+0x09]
			rax = 0x91E58FC0906E516B;               //mov rax, 0x91E58FC0906E516B
			rdx *= rax;             //imul rdx, rax
			rax = 0x536FCC1881ABB3DD;               //mov rax, 0x536FCC1881ABB3DD
			rdx -= rax;             //sub rdx, rax
			return rdx;
		}
		case 9:
		{
			r9 = *(uintptr_t*)(mb + 0xA6E01F8);               //mov r9, [0x000000000791139D]
			rax = r10;              //mov rax, r10
			rax -= mb;          //sub rax, [rbp-0x58] -- didn't find trace -> use base
			rax += 0xFFFFFFFFA016355E;              //add rax, 0xFFFFFFFFA016355E
			rdx += rax;             //add rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x7;            //shr rax, 0x07
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0xE;            //shr rax, 0x0E
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x1C;           //shr rax, 0x1C
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x38;           //shr rax, 0x38
			rdx ^= rax;             //xor rdx, rax
			rax = mb;           //lea rax, [0xFFFFFFFFFD23104E]
			rdx -= rax;             //sub rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x13;           //shr rax, 0x13
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rax >>= 0x26;           //shr rax, 0x26
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rdx ^= rax;             //xor rdx, rax
			rcx ^= r9;              //xor rcx, r9
			rcx = _byteswap_uint64(rcx);            //bswap rcx
			rdx *= *(uintptr_t*)(rcx + 0x9);              //imul rdx, [rcx+0x09]
			rax = 0x717FE87E6285FB9A;               //mov rax, 0x717FE87E6285FB9A
			rdx ^= rax;             //xor rdx, rax
			rax = 0x81A910B660606CCB;               //mov rax, 0x81A910B660606CCB
			rdx *= rax;             //imul rdx, rax
			rax = mb;           //lea rax, [0xFFFFFFFFFD230E12]
			rdx -= r10;             //sub rdx, r10
			rdx -= rax;             //sub rdx, rax
			rdx -= 0x451FF009;              //sub rdx, 0x451FF009
			return rdx;
		}
		case 10:
		{
			r8 = *(uintptr_t*)(mb + 0xA6E01F8);               //mov r8, [0x0000000007910E52]
			rax = rdx;              //mov rax, rdx
			rax >>= 0x26;           //shr rax, 0x26
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x9;            //shr rax, 0x09
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x12;           //shr rax, 0x12
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x24;           //shr rax, 0x24
			rdx ^= rax;             //xor rdx, rax
			rax = 0x427BC6EBC9E9BD3D;               //mov rax, 0x427BC6EBC9E9BD3D
			rdx -= r10;             //sub rdx, r10
			rdx *= rax;             //imul rdx, rax
			rax = 0x3543C84ABCF4B383;               //mov rax, 0x3543C84ABCF4B383
			rdx -= rax;             //sub rdx, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r8;              //xor rax, r8
			rax = _byteswap_uint64(rax);            //bswap rax
			rdx *= *(uintptr_t*)(rax + 0x9);              //imul rdx, [rax+0x09]
			rdx += r10;             //add rdx, r10
			rax = 0xE0A33520B6FCDB76;               //mov rax, 0xE0A33520B6FCDB76
			rdx ^= rax;             //xor rdx, rax
			return rdx;
		}
		case 11:
		{
			r13 = mb + 0x6F4;           //lea r13, [0xFFFFFFFFFD230F5B]
			r9 = *(uintptr_t*)(mb + 0xA6E01F8);               //mov r9, [0x0000000007910A00]
			rax = rdx;              //mov rax, rdx
			rax >>= 0x1D;           //shr rax, 0x1D
			rdx ^= rax;             //xor rdx, rax
			rax = r10;              //mov rax, r10
			rcx = rdx;              //mov rcx, rdx
			rax ^= r13;             //xor rax, r13
			rcx >>= 0x3A;           //shr rcx, 0x3A
			rdx ^= rcx;             //xor rdx, rcx
			rdx -= rax;             //sub rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x18;           //shr rax, 0x18
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x30;           //shr rax, 0x30
			rdx ^= rax;             //xor rdx, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r9;              //xor rax, r9
			rax = _byteswap_uint64(rax);            //bswap rax
			rax = *(uintptr_t*)(rax + 0x9);               //mov rax, [rax+0x09]
			uintptr_t RSP_0x30;
			RSP_0x30 = 0xF66C834D2893EC35;          //mov rax, 0xF66C834D2893EC35 : RSP+0x30
			rax *= RSP_0x30;                //imul rax, [rsp+0x30]
			rdx *= rax;             //imul rdx, rax
			rdx += r10;             //add rdx, r10
			rax = 0xB9B78846D4529A7C;               //mov rax, 0xB9B78846D4529A7C
			rdx ^= rax;             //xor rdx, rax
			rax = 0x168AB370FA2F3E7F;               //mov rax, 0x168AB370FA2F3E7F
			rdx *= rax;             //imul rdx, rax
			return rdx;
		}
		case 12:
		{
			r12 = mb + 0xADC0;          //lea r12, [0xFFFFFFFFFD23B01D]
			r9 = *(uintptr_t*)(mb + 0xA6E01F8);               //mov r9, [0x00000000079103C6]
			rax = 0x54B20FF32B698B31;               //mov rax, 0x54B20FF32B698B31
			rdx += rax;             //add rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x9;            //shr rax, 0x09
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x12;           //shr rax, 0x12
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x24;           //shr rax, 0x24
			rdx ^= rax;             //xor rdx, rax
			rax = 0xD632377F01273E4B;               //mov rax, 0xD632377F01273E4B
			rdx *= rax;             //imul rdx, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r9;              //xor rax, r9
			rax = _byteswap_uint64(rax);            //bswap rax
			rdx *= *(uintptr_t*)(rax + 0x9);              //imul rdx, [rax+0x09]
			rdx ^= r10;             //xor rdx, r10
			rdx ^= r12;             //xor rdx, r12
			rax = 0x2F3243F77C00FF4A;               //mov rax, 0x2F3243F77C00FF4A
			rdx ^= rax;             //xor rdx, rax
			rdx ^= r10;             //xor rdx, r10
			rax = mb;           //lea rax, [0xFFFFFFFFFD2300AD]
			rdx ^= rax;             //xor rdx, rax
			return rdx;
		}
		case 13:
		{
			r9 = *(uintptr_t*)(mb + 0xA6E01F8);               //mov r9, [0x000000000790FEB5]
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rax = mb + 0x7E923E8D;              //lea rax, [0x000000007BB53833]
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rax -= r10;             //sub rax, r10
			rcx ^= r9;              //xor rcx, r9
			rdx ^= rax;             //xor rdx, rax
			rcx = _byteswap_uint64(rcx);            //bswap rcx
			rdx *= *(uintptr_t*)(rcx + 0x9);              //imul rdx, [rcx+0x09]
			rax = mb;           //lea rax, [0xFFFFFFFFFD22FB8D]
			rax += 0xE5BA;          //add rax, 0xE5BA
			rax += r10;             //add rax, r10
			rdx += rax;             //add rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x14;           //shr rax, 0x14
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x28;           //shr rax, 0x28
			rdx ^= rax;             //xor rdx, rax
			rax = 0x8C7EEAE39FF7123C;               //mov rax, 0x8C7EEAE39FF7123C
			rdx ^= rax;             //xor rdx, rax
			rax = 0xE2D3E3752453C2D;                //mov rax, 0xE2D3E3752453C2D
			rdx *= rax;             //imul rdx, rax
			rax = 0xFBFCB7F17047B6E;                //mov rax, 0xFBFCB7F17047B6E
			rdx ^= rax;             //xor rdx, rax
			rax = mb;           //lea rax, [0xFFFFFFFFFD22F9E0]
			rdx += rax;             //add rdx, rax
			return rdx;
		}
		case 14:
		{
			r8 = *(uintptr_t*)(mb + 0xA6E01F8);               //mov r8, [0x000000000790F92D]
			rax = rdx;              //mov rax, rdx
			rax >>= 0x1A;           //shr rax, 0x1A
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x34;           //shr rax, 0x34
			rdx ^= rax;             //xor rdx, rax
			rax = 0x77A29410920726B9;               //mov rax, 0x77A29410920726B9
			rdx *= rax;             //imul rdx, rax
			rax = 0x4F0903AB19AED8D5;               //mov rax, 0x4F0903AB19AED8D5
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x15;           //shr rax, 0x15
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x2A;           //shr rax, 0x2A
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x4;            //shr rax, 0x04
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x8;            //shr rax, 0x08
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x10;           //shr rax, 0x10
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x20;           //shr rax, 0x20
			rdx ^= rax;             //xor rdx, rax
			rdx -= r10;             //sub rdx, r10
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r8;              //xor rax, r8
			rax = _byteswap_uint64(rax);            //bswap rax
			rax = *(uintptr_t*)(rax + 0x9);               //mov rax, [rax+0x09]
			uintptr_t RSP_0x30;
			RSP_0x30 = 0x62039DF9B088B0F9;          //mov rax, 0x62039DF9B088B0F9 : RSP+0x30
			rax *= RSP_0x30;                //imul rax, [rsp+0x30]
			rdx *= rax;             //imul rdx, rax
			return rdx;
		}
		case 15:
		{
			r8 = *(uintptr_t*)(mb + 0xA6E01F8);               //mov r8, [0x000000000790F262]
			rax = 0x2A82B14EF77F5441;               //mov rax, 0x2A82B14EF77F5441
			rdx += rax;             //add rdx, rax
			rdx += r10;             //add rdx, r10
			rax = mb;           //lea rax, [0xFFFFFFFFFD22EF4C]
			rdx ^= rax;             //xor rdx, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r8;              //xor rax, r8
			rax = _byteswap_uint64(rax);            //bswap rax
			rdx *= *(uintptr_t*)(rax + 0x9);              //imul rdx, [rax+0x09]
			rax = rdx;              //mov rax, rdx
			rax >>= 0xC;            //shr rax, 0x0C
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x18;           //shr rax, 0x18
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x30;           //shr rax, 0x30
			rdx ^= rax;             //xor rdx, rax
			rdx += r10;             //add rdx, r10
			rax = 0x6C92B8E2E6EF1820;               //mov rax, 0x6C92B8E2E6EF1820
			rdx -= rax;             //sub rdx, rax
			rax = 0x52B74D7B1271CDDF;               //mov rax, 0x52B74D7B1271CDDF
			rdx *= rax;             //imul rdx, rax
			return rdx;
		}
		}
	}
	uint16_t get_bone_index(uint32_t bone_index)
	{	// Updated 12/10/22
		auto mb = g_data::base;
		auto Peb = __readgsqword(0x60);
		
		uint64_t rax = mb, rbx = mb, rcx = mb, rdx = mb, rdi = mb, rsi = mb, r8 = mb, r9 = mb, r10 = mb, r11 = mb, r12 = mb, r13 = mb, r14 = mb, r15 = mb;
		rdi = bone_index;
		rcx = rdi * 0x13C8;
		rax = 0xC860AAA2514E393D;               //mov rax, 0xC860AAA2514E393D
		rax = _umul128(rax, rcx, (uintptr_t*)&rdx);             //mul rcx
		r11 = mb;           //lea r11, [0xFFFFFFFFFD548138]
		r10 = 0xE4C59B2FAB8D0E37;               //mov r10, 0xE4C59B2FAB8D0E37
		rdx >>= 0xC;            //shr rdx, 0x0C
		rax = rdx * 0x1471;             //imul rax, rdx, 0x1471
		rcx -= rax;             //sub rcx, rax
		rax = 0x1F47F5E6785AE5C5;               //mov rax, 0x1F47F5E6785AE5C5
		r8 = rcx * 0x1471;              //imul r8, rcx, 0x1471
		rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
		rax = r8;               //mov rax, r8
		rax -= rdx;             //sub rax, rdx
		rax >>= 0x1;            //shr rax, 0x01
		rax += rdx;             //add rax, rdx
		rax >>= 0xC;            //shr rax, 0x0C
		rax = rax * 0x1C84;             //imul rax, rax, 0x1C84
		r8 -= rax;              //sub r8, rax
		rax = 0x97B425ED097B425F;               //mov rax, 0x97B425ED097B425F
		rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
		rax = 0x70381C0E070381C1;               //mov rax, 0x70381C0E070381C1
		rdx >>= 0x6;            //shr rdx, 0x06
		rcx = rdx * 0x6C;               //imul rcx, rdx, 0x6C
		rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
		rdx >>= 0x5;            //shr rdx, 0x05
		rcx += rdx;             //add rcx, rdx
		rax = rcx * 0x92;               //imul rax, rcx, 0x92
		rcx = r8 * 0x94;                //imul rcx, r8, 0x94
		rcx -= rax;             //sub rcx, rax
		rax = *(uintptr_t*)(rcx + r11 * 1 + 0xA79D790);                //movzx eax, word ptr [rcx+r11*1+0xA79D790]
		r8 = rax * 0x13C8;              //imul r8, rax, 0x13C8
		rax = r10;              //mov rax, r10
		rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
		rax = r10;              //mov rax, r10
		rdx >>= 0xD;            //shr rdx, 0x0D
		rcx = rdx * 0x23CF;             //imul rcx, rdx, 0x23CF
		r8 -= rcx;              //sub r8, rcx
		r9 = r8 * 0x2DB6;               //imul r9, r8, 0x2DB6
		rax = _umul128(rax, r9, (uintptr_t*)&rdx);              //mul r9
		rdx >>= 0xD;            //shr rdx, 0x0D
		rax = rdx * 0x23CF;             //imul rax, rdx, 0x23CF
		r9 -= rax;              //sub r9, rax
		rax = 0x661EC6A5122F9017;               //mov rax, 0x661EC6A5122F9017
		rax = _umul128(rax, r9, (uintptr_t*)&rdx);              //mul r9
		rax = r9;               //mov rax, r9
		rax -= rdx;             //sub rax, rdx
		rax >>= 0x1;            //shr rax, 0x01
		rax += rdx;             //add rax, rdx
		rax >>= 0x7;            //shr rax, 0x07
		rcx = rax * 0xB7;               //imul rcx, rax, 0xB7
		rax = 0x67B23A5440CF6475;               //mov rax, 0x67B23A5440CF6475
		rax = _umul128(rax, r9, (uintptr_t*)&rdx);              //mul r9
		rdx >>= 0x5;            //shr rdx, 0x05
		rcx += rdx;             //add rcx, rdx
		rax = rcx * 0x9E;               //imul rax, rcx, 0x9E
		rcx = r9 + r9 * 4;              //lea rcx, [r9+r9*4]
		rcx <<= 0x5;            //shl rcx, 0x05
		rcx -= rax;             //sub rcx, rax
		r15 = *(uintptr_t*)(rcx + r11 * 1 + 0xA7A1530);                //movsx r15d, word ptr [rcx+r11*1+0xA7A1530]
		return r15;
	}

	player_t get_player(int i)
	{
		uint64_t decryptedPtr = get_client_info();

		if (is_valid_ptr (decryptedPtr))
		{
			uint64_t client_info = get_client_info_base();

			if (is_valid_ptr(client_info))
			{
				return player_t(client_info + (i * offsets::player::size));
			}
		}
		return player_t(NULL);
	}

	//player_t player_t
	
	//player_t get_local_player()
	//{
	//	auto addr = sdk::get_client_info_base() + (get_local_index() * offsets::player::size);
	//	if (is_bad_ptr(addr)) return 0;
	//	return addr;


	//}

	player_t get_local_player()
	{
		uint64_t decryptedPtr = get_client_info();

		if (is_bad_ptr(decryptedPtr))return player_t(NULL);

			auto local_index = *(uintptr_t*)(decryptedPtr + offsets::local_index);
			if (is_bad_ptr(local_index))return player_t(NULL);
			auto index = *(int*)(local_index + offsets::local_index_pos);
			return get_player(index);
		
		
	}

	/*name_t* get_name_ptr(int i)
	{
		uint64_t bgs = *(uint64_t*)(g_data::base + client::name_array);

		if (bgs)
		{
			name_t* pClientInfo = (name_t*)(bgs + client::name_array_padding + ((i + i * 8) << 4));

			if (is_bad_ptr(pClientInfo))return 0;
			else
			return pClientInfo;
			
		}
		return 0;
	}*/

	refdef_t* get_refdef()
	{
		uint32_t crypt_0 = *(uint32_t*)(g_data::base + offsets::ref_def_ptr);
		uint32_t crypt_1 = *(uint32_t*)(g_data::base + offsets::ref_def_ptr + 0x4);
		uint32_t crypt_2 = *(uint32_t*)(g_data::base + offsets::ref_def_ptr + 0x8);
		// lower 32 bits
		uint32_t entry_1 = (uint32_t)(g_data::base + offsets::ref_def_ptr);
		uint32_t entry_2 = (uint32_t)(g_data::base + offsets::ref_def_ptr + 0x4);
		// decryption
		uint32_t _low = entry_1 ^ crypt_2;
		uint32_t _high = entry_2 ^ crypt_2;
		uint32_t low_bit = crypt_0 ^ _low * (_low + 2);
		uint32_t high_bit = crypt_1 ^ _high * (_high + 2);
		auto ret = (refdef_t*)(((QWORD)high_bit << 32) + low_bit);
		if (is_bad_ptr(ret)) return 0;
		else
			return ret;
	}

	Vector3 get_camera_pos()
	{
		Vector3 pos = Vector3{};

		auto camera_ptr = *(uint64_t*)(g_data::base + offsets::camera_base);

		if (is_bad_ptr(camera_ptr))return pos;
		
		
		pos = *(Vector3*)(camera_ptr + offsets::camera_pos);
		if (pos.IsZero())return {};
		else
		return pos;
	}

	/*std::string get_player_name(int i)
	{
		uint64_t bgs = *(uint64_t*)(g_data::base + client::name_array);

		if (is_bad_ptr(bgs))return NULL;


		if (bgs)
		{
			name_t* clientInfo_ptr = (name_t*)(bgs + client::name_array_padding + (i * 0xD0));
			if (is_bad_ptr(clientInfo_ptr))return NULL;

			int length = strlen(clientInfo_ptr->name);
			for (int j = 0; j < length; ++j)
			{
				char ch = clientInfo_ptr->name[j];
				bool is_english = ch >= 0 && ch <= 127;
				if (!is_english)
					return xorstr_("Player");
			}
			return clientInfo_ptr->name;
		}
		return xorstr_("Player");
	}*/

	
    bool bones_to_screen(Vector3* BonePosArray, Vector2* ScreenPosArray, const long Count)
    {
        for (long i = 0; i < Count; ++i)
        {
            if (!world(BonePosArray[i], &ScreenPosArray[i]))
                return false;
        }
        return true;
    }



	bool get_bone_by_player_index(int i, int bone_id, Vector3* Out_bone_pos)
	{
		uint64_t decrypted_ptr = get_bone_ptr();

		if (is_bad_ptr(decrypted_ptr))return false;
		
			unsigned short index = get_bone_index(i);
			if (index != 0)
			{
				uint64_t bone_ptr = *(uint64_t*)(decrypted_ptr + (index * offsets::bones::size) + 0xD8);

				if (is_bad_ptr(bone_ptr))return false;
				
					Vector3 bone_pos = *(Vector3*)(bone_ptr + (bone_id * 0x20) + 0x10);

					if (bone_pos.IsZero())return false;

					uint64_t client_info = get_client_info();

					if (is_bad_ptr(client_info))return false;

					
					
						Vector3 BasePos = *(Vector3*)(client_info + offsets::bones::bone_base);

						if (BasePos.IsZero())return false;

						bone_pos.x += BasePos.x;
						bone_pos.y += BasePos.y;
						bone_pos.z += BasePos.z;

						*Out_bone_pos = bone_pos;
						return true;
					
				
			}
		
		return false;
	}
	
	int get_player_health(int i)
	{
		uint64_t bgs = *(uint64_t*)(g_data::base + offsets::name_array);

		if (bgs)
		{
			name_t* pClientInfo = (name_t*)(bgs + offsets::name_array_pos  +(i * 0xD0));

			if (pClientInfo)
			{
				return pClientInfo->get_health();
			}
		}
		return 0;
	}

	std::string get_player_name(int entityNum)
	{
		uint64_t bgs = *(uint64_t*)(g_data::base + offsets::name_array);
		if (is_bad_ptr(bgs)) return "";

		if (bgs)
		{
			name_t* clientInfo_ptr = (name_t*)(bgs + offsets::name_array_pos + (entityNum * 0xD0));
			if (is_bad_ptr(clientInfo_ptr)) return "";

			int length = strlen(clientInfo_ptr->name);
			for (int j = 0; j < length; ++j)
			{
				char ch = clientInfo_ptr->name[j];
				bool is_english = ch >= 0 && ch <= 127;
				if (!is_english)
					return xorstr_("Player");
			}
			return clientInfo_ptr->name;
		}
		return xorstr_("Player");
	}

	void start_tick()
	{
		static DWORD lastTick = 0;
		DWORD t = GetTickCount();
		bUpdateTick = lastTick < t;

		if (bUpdateTick)
			lastTick = t + nTickTime;
	}

	void update_vel_map(int index, Vector3 vPos)
	{
		if (!bUpdateTick)
			return;

		velocityMap[index].delta = vPos - velocityMap[index].lastPos;
		velocityMap[index].lastPos = vPos;
	}

	void clear_map()
	{
		if (!velocityMap.empty()) { velocityMap.clear(); }
	}

	Vector3 get_speed(int index)
	{
		return velocityMap[index].delta;
	}

	Vector3 get_prediction(int index, Vector3 source, Vector3 destination)
	{
		auto local_velocity = get_speed(local_index());
		auto target_velocity = get_speed(index);

		const auto distance = source.distance_to(destination);
		const auto travel_time = distance / globals::bullet_speed;
		auto pred_destination = destination + (target_velocity - local_velocity) * travel_time;
		/*position.x += travel_time * final_speed.x;
		position.y += travel_time * final_speed.y;
		position.z += 0.5 * globals::bullet_gravity * travel_time * travel_time;
		return position;*/

		pred_destination.z += 0.5f * std::fabsf(globals::bullet_gravity) * travel_time;

		return pred_destination;
	}


	Result MidnightSolver(float a, float b, float c)
	{
		Result res;

		double subsquare = b * b - 4 * a * c;

		if (subsquare < 0)
		{
			res.hasResult = false;
			return res;
		}
		else
		{
			res.hasResult = true,
			res.a = (float)((-b + sqrt(subsquare)) / (2 * a));
			res.b = (float)((-b - sqrt(subsquare)) / (2 * a));
		}
		return res;
	}

	Vector3 prediction_solver(Vector3 local_pos, Vector3 position, int index, float bullet_speed)
	{
		Vector3 aimPosition = Vector3().Zero();
		auto target_speed = get_speed(index);

		local_pos -= position; 

		float a = (target_speed.x * target_speed.x) + (target_speed.y * target_speed.y) + (target_speed.z * target_speed.z) - ((bullet_speed * bullet_speed) * 100);
		float b = (-2 * local_pos.x * target_speed.x) + (-2 * local_pos.y * target_speed.y) + (-2 * local_pos.z * target_speed.z);
		float c = (local_pos.x * local_pos.x) + (local_pos.y * local_pos.y) + (local_pos.z * local_pos.z);

		local_pos += position; 

		Result r = MidnightSolver(a, b, c);

		if (r.a >= 0 && !(r.b >= 0 && r.b < r.a))
		{
			aimPosition = position + target_speed * r.a;
		}
		else if (r.b >= 0)
		{
			aimPosition = position + target_speed * r.b;
		}

		return aimPosition;
	
	}

	uint64_t get_visible_base()
	{

		for (int32_t j{}; j <= 0x1770; ++j)
		{
			
			uint64_t vis_base_ptr = *(uint64_t*)(g_data::base + offsets::distribute) + (j * 0x190);
			uint64_t cmp_function = *(uint64_t*)(vis_base_ptr + 0x38);

			if (!cmp_function)
				continue;

			//LOGS_ADDR(cmp_function);

			uint64_t about_visible = g_data::base + offsets::visible;

			if (cmp_function == about_visible)
			{
				g_data::current_visible_offset = vis_base_ptr;
				return g_data::current_visible_offset;
			}

		}
		return NULL;
	}
	
	bool is_visible(int entityNum) {

		if (!g_data::visible_base)
			return false;

		uint64_t VisibleList = *(uint64_t*)(g_data::last_visible_offset + 0x80);
		if (!VisibleList)
			return false;
		uint64_t v421 = VisibleList + (entityNum * 9 + 0x152) * 8;
		if (!v421)
			return false;
		DWORD VisibleFlags = (v421 + 0x10) ^ *(DWORD*)(v421 + 0x14);
		if (!VisibleFlags)
			return false;
		DWORD v1630 = VisibleFlags * (VisibleFlags + 2);
		if (!v1630)
			return false;
		BYTE VisibleFlags1 = *(DWORD*)(v421 + 0x10) ^ v1630 ^ BYTE1(v1630);
		if (VisibleFlags1 == 3) {
			return true;
		}
		return false;
	}

	void update_last_visible()
	{
		g_data::last_visible_offset = g_data::current_visible_offset;
	}

	// player class methods
	bool player_t::is_valid() {
		if (is_bad_ptr(address))return 0;

		return *(bool*)((uintptr_t)address + offsets::player::valid);
	}

	bool player_t::is_dead() {
		if (is_bad_ptr(address))return 0;

		auto dead1 = *(bool*)((uintptr_t)address + offsets::player::dead_1);
		auto dead2 = *(bool*)((uintptr_t)address + offsets::player::dead_2);
		return !(!dead1 && !dead2 && is_valid());
	}

	int player_t::team_id() {

		if (is_bad_ptr(address))return 0;
		return *(int*)((uintptr_t)address + offsets::player::team);
	}

	int player_t::get_stance() {
		
		if (is_bad_ptr(address))return 4;
		auto ret = *(int*)((uintptr_t)address + offsets::player::stance);
	

		return ret;
	}


	float player_t::get_rotation()
	{
		if (is_bad_ptr(address))return 0;
		auto local_pos_ptr = *(uintptr_t*)((uintptr_t)address + offsets::player::pos);

		if (is_bad_ptr(local_pos_ptr))return 0;

		auto rotation = *(float*)(local_pos_ptr + 0x58);

		if (rotation < 0)
			rotation = 360.0f - (rotation * -1);

		rotation += 90.0f;

		if (rotation >= 360.0f)
			rotation = rotation - 360.0f;

		return rotation;
	}

	Vector3 player_t::get_pos() 
	{
		if (is_bad_ptr(address))return {};
		auto local_pos_ptr = *(uintptr_t*)((uintptr_t)address + offsets::player::pos);

		if (is_bad_ptr(local_pos_ptr))return{};
		else
			return *(Vector3*)(local_pos_ptr + 0x48);
		return Vector3{}; 


	}

	uint32_t player_t::get_index()
	{
		if (is_bad_ptr(this->address))return 0;

		auto cl_info_base = get_client_info_base();
		if (is_bad_ptr(cl_info_base))return 0;
		
		
	return ((uintptr_t)this->address - cl_info_base) / offsets::player::size;
		
	
	}

	bool player_t::is_visible()
	{
		if (is_bad_ptr(g_data::visible_base))return false;

		if (is_bad_ptr(this->address))return false;
		
			uint64_t VisibleList =*(uint64_t*)(g_data::visible_base + 0x108);
			if (is_bad_ptr(VisibleList))
				return false;

			uint64_t rdx = VisibleList + (player_t::get_index() * 9 + 0x14E) * 8;
			if (is_bad_ptr(rdx))
				return false;

			DWORD VisibleFlags = (rdx + 0x10) ^ *(DWORD*)(rdx + 0x14);
			if (!VisibleFlags)
				return false;

			DWORD v511 = VisibleFlags * (VisibleFlags + 2);
			if (!v511)
				return false;

			BYTE VisibleFlags1 = *(DWORD*)(rdx + 0x10) ^ v511 ^ BYTE1(v511);
			if (VisibleFlags1 == 3) {
				return true;
			}
		
		return false;
	}
	



}

