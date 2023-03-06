#include "stdafx.h"
#include "Menu.h"
#include "imgui/imgui.h"
# include "globals.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"
#include "obfuscator.hpp"
#include "xor.hpp"
#include"memory.h"
#include "mem.h"
#include "xorstr.hpp"
#include "style.h"

#define INRANGE(x,a,b)    (x >= a && x <= b) 
#define getBits( x )    (INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))
#define getByte( x )    (getBits(x[0]) << 4 | getBits(x[1]))
bool b_menu_open = true;
bool b_debug_open = false;
bool boxcheck;

uintptr_t FindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets)
{
	if (ptr != 0)
	{
		uintptr_t addr = ptr;
		for (unsigned int i = 0; i < offsets.size(); ++i)
		{
			addr = *(uintptr_t*)addr;
			addr += offsets[i];
		}
		return addr;
	}
	else
		return 0;
}


uint64_t BASEIMAGE2 = reinterpret_cast<uint64_t>(GetModuleHandleA(NULL));

bool b_fov = false;
float f_fov = 1.20f;
float f_map = 1.0f;
bool b_map = false;
bool b_brightmax = false;
bool b_thirdperson = false;
bool b_heartcheat = false;
bool b_norecoil = false;
bool b_no_flashbang = false;
char DDL_SetInt(__int64 fstate, __int64 context, unsigned int value) {
	uintptr_t address = g_data::base + g_data::ddl_setint;
	return ((char (*)(__int64, __int64, unsigned int))address)(fstate, context, value);
}
struct unnamed_type_integer
{
	int min;
	int max;
};
struct unnamed_type_integer64
{
	__int64 min;
	__int64 max;
};
struct unnamed_type_enumeration
{
	int stringCount;
	const char* strings;
};
/* 433 */
struct unnamed_type_unsignedInt64
{
	unsigned __int64 min;
	unsigned __int64 max;
};

/* 434 */
struct unnamed_type_value
{
	float min;
	float max;
	float devguiStep;
};

/* 435 */
struct unnamed_type_vector
{
	float min;
	float max;
	float devguiStep;
};
union DvarLimits
{
	unnamed_type_enumeration enumeration;
	unnamed_type_integer integer;
	unnamed_type_integer64 integer64;
	unnamed_type_unsignedInt64 unsignedInt64;
	unnamed_type_value value;
	unnamed_type_vector vector;
};
typedef enum DvarType : uint8_t
{
	DVAR_TYPE_BOOL = 0x0,
	DVAR_TYPE_FLOAT = 0x1,
	DVAR_TYPE_FLOAT_2 = 0x2,
	DVAR_TYPE_FLOAT_3 = 0x3,
	DVAR_TYPE_FLOAT_4 = 0x4,
	DVAR_TYPE_INT = 0x5,
	DVAR_TYPE_INT64 = 0x6,
	DVAR_TYPE_UINT64 = 0x7,
	DVAR_TYPE_ENUM = 0x8,
	DVAR_TYPE_STRING = 0x9,
	DVAR_TYPE_COLOR = 0xA,
	DVAR_TYPE_FLOAT_3_COLOR = 0xB,
	DVAR_TYPE_COUNT = 0xC,
} DvarType;
union DvarValue
{
	bool enabled; //0x0000
	int32_t integer; //0x0000
	uint32_t unsignedInt; //0x0000
	float value; //0x0000
	//Vector4 vector; //0x0000
	const char* string; //0x0000
	unsigned __int8 color[4]; //0x0000
	uint64_t unsignedInt64; //0x0000
	int64_t integer64; //0x0000
};
struct dvar_s
{
	char name[4]; //0x0
	uint32_t flags; //0x4
	BYTE level[1]; //0x8
	DvarType type; //0x9
	bool modified; //0xA
	uint32_t checksum; //0xC
	char* description; //0x10
	char pad2[16]; //0x18
	unsigned __int16 hashNext; //0x28
	DvarValue current; //0x30
	DvarValue latched; //0x40
	DvarValue reset; //0x50
	DvarLimits domain; //0x60
	//BbConstUsageFlags BbConstUsageFlags;
};

template<typename T> inline void dvar_set2(const char* dvarName, T value)
{
	
}
void CopyWeapon(int Class)
{


}

uintptr_t cbuff1;
uintptr_t cbuff2;
char inputtext[50];
int C_TagMOde = 0;


__int64 find_pattern(__int64 range_start, __int64 range_end, const char* pattern) {
	const char* pat = pattern;
	__int64 firstMatch = NULL;
	__int64 pCur = range_start;
	__int64 region_end;
	MEMORY_BASIC_INFORMATION mbi{};
	while (sizeof(mbi) == VirtualQuery((LPCVOID)pCur, &mbi, sizeof(mbi))) {
		if (pCur >= range_end - strlen(pattern))
			break;
		if (!(mbi.Protect & (PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_READWRITE))) {
			pCur += mbi.RegionSize;
			continue;
		}
		region_end = pCur + mbi.RegionSize;
		while (pCur < region_end)
		{
			if (!*pat)
				return firstMatch;
			if (*(PBYTE)pat == '\?' || *(BYTE*)pCur == getByte(pat)) {
				if (!firstMatch)
					firstMatch = pCur;
				if (!pat[1] || !pat[2])
					return firstMatch;

				if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?')
					pat += 3;
				else
					pat += 2;
			}
			else {
				if (firstMatch)
					pCur = firstMatch;
				pat = pattern;
				firstMatch = 0;
			}
			pCur++;
		}
	}
	return NULL;
}


bool init_once = true;
char input[30];
bool Unlock_once = true;

void Visual()
{
	ImGui::Dummy(ImVec2(0.0f, 3.0f));
	ImGui::Spacing();
	ImGui::Checkbox(xorstr_("Check Visibility"), &globals::b_visible);
	ImGui::Checkbox(xorstr_("Show Box"), &globals::b_box);
	/*ImGui::SameLine();
	ImGui::Combo("##", &globals::box_index, globals::box_types, 2);*/

	//ImGui::Checkbox(xorstr_("Show HealthBar"), &globals::b_health);
	ImGui::Checkbox(xorstr_("Show Line"), &globals::b_line);
	ImGui::Checkbox(xorstr_("Show Bones "), &globals::b_skeleton);
	ImGui::Checkbox(xorstr_("Show Names"), &globals::b_names);
	ImGui::Checkbox(xorstr_("Show Distance"), &globals::b_distance);
	ImGui::Checkbox(xorstr_("Show Team"), &globals::b_friendly);
	ImGui::SliderInt(xorstr_("##MAXDISTANCE"), &globals::max_distance, 0, 1000, xorstr_("ESP Distance: %d"));
}
void KeyBindButton(int& key, int width, int height)
{
	static auto b_get = false;
	static std::string sz_text = xorstr_("Click to bind.");

	if (ImGui::Button(sz_text.c_str(), ImVec2(static_cast<float>(width), static_cast<float>(height))))
		b_get = true;

	if (b_get)
	{
		for (auto i = 1; i < 256; i++)
		{
			if (GetAsyncKeyState(i) & 0x8000)
			{
				if (i != 12)
				{
					key = i == VK_ESCAPE ? -1 : i;
					b_get = false;
				}
			}
		}
		sz_text = xorstr_("Press a Key.");
	}
	else if (!b_get && key == -1)
		sz_text = xorstr_("Click to bind.");
	else if (!b_get && key != -1)
	{
		sz_text = xorstr_("Key ~ ") + std::to_string(key);
	}
}
void Aimbot()
{

	ImGui::Dummy(ImVec2(0.0f, 3.0f));
	ImGui::Spacing();
	ImGui::Checkbox(xorstr_("Enable"), &globals::b_lock);
	if (globals::b_lock)
	{
		ImGui::SliderInt(xorstr_("##LOCKSMOOTH"), &globals::aim_smooth, 1, 30, xorstr_("Lock Smooth: %d"));
	}
	ImGui::Checkbox(xorstr_("Crosshair"), &globals::b_crosshair);
	ImGui::Checkbox(xorstr_("Show FOV"), &globals::b_fov);
	if (globals::b_fov)
	{
		ImGui::SliderFloat(xorstr_("##LOCKFOV"), &globals::f_fov_size, 10.f, 800.f, xorstr_("FOV Size: %0.0f"));
	}
	ImGui::Checkbox(xorstr_("Skip Knocked"), &globals::b_skip_knocked);

	
	
	/*ImGui::Checkbox(xorstr_("Prediction"), &globals::b_prediction);*/

	
	

	ImGui::Checkbox(xorstr_("Use Bones"), &globals::target_bone);
	if (globals::target_bone)
	{
		ImGui::Combo(xorstr_("Lock Bone"), &globals::bone_index, globals::aim_lock_point, 4);
	}

	KeyBindButton(globals::aim_key, 100, 30);
	ImGui::Dummy(ImVec2(0.0f, 1.0f));
	ImGui::Spacing();

	

}
void ColorPicker()
{
	ImGui::Dummy(ImVec2(0.0f, 1.0f));
	//ImGui::Text("Fov Color");
	//ImGui::ColorEdit4("##Fov Color7", (float*)&color::bfov);
	//ImGui::Text("cross hair  Color");
	//ImGui::ColorEdit4("##cross hair Color9", (float*)&color::draw_crosshair);
	ImGui::Text("Visible Team Color");
	ImGui::ColorEdit4("##esp Color1", (float*)&color::VisibleColorTeam);
	ImGui::Spacing();
	ImGui::Text("Not Visible Team Color");
	ImGui::ColorEdit4("##esp Color2", (float*)&color::NotVisibleColorTeam);
	ImGui::Spacing();
	ImGui::Text("Visible Enemy Color");
	ImGui::ColorEdit4("##esp Color3", (float*)&color::VisibleColorEnemy);
	ImGui::Spacing();
	ImGui::Text("Not Visible Enemy Color");
	//ImGui::ColorEdit4("##esp Color4", (float*)&color::NotVisibleColorEnemy);
	//ImGui::Text("loot Color");
	//ImGui::ColorEdit4("##esp lootcolor", (float*)&loot::lootcolor);
	//ImGui::EndTabItem();

}

void Misc()
{
	ImGui::Dummy(ImVec2(0.0f, 3.0f));
	ImGui::Checkbox(xorstr_("UAV"), &globals::b_UAV);

}
int i_MenuTab = 0;
namespace g_menu
{
	void menu()
	{
		if (GetAsyncKeyState(VK_INSERT) & 0x1)
		{
			b_menu_open = !b_menu_open;

		}
		
		EditorColorScheme::ApplyTheme2();
		if (b_menu_open && screenshot::visuals)
		{


			

			ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_Always);

			ImGui::Begin(xorstr_("tobiwobi#2242"), &b_menu_open, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize);
			//ImGui::Begin(xorstr_("MENU"), &b_menu_open, ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize);
			

			int dwWidth = GetSystemMetrics(SM_CXSCREEN) / 3;
			int dwHeight = GetSystemMetrics(SM_CYSCREEN) / 2;
			ImGui::Dummy(ImVec2(0.0f, 1.0f));
			for (int i = 0; i < 25; i++)
			{
				ImGui::Spacing();
				ImGui::SameLine();
			}


			ImGui::Dummy(ImVec2(0.0f, 3.0f));
			ImGui::SetWindowPos(ImVec2(dwWidth * 2.0f, dwHeight * 0.2f), ImGuiCond_Once);
			{
				ImGui::BeginChild(xorstr_("##TABCHILD"), ImVec2(110, -1), true);
				{

					if (ImGui::Button(xorstr_("Visual"), ImVec2(95, 30))) { i_MenuTab = 0; }
					for (int i = 0; i < 15; i++)
					{
						ImGui::Spacing();
					}
					if (ImGui::Button(xorstr_("Aimbot"), ImVec2(95, 30))) { i_MenuTab = 1; }
					if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
					{
						ImGui::SetTooltip(xorstr_("Use at your own Risk! High Ban Chance!"));
					}				
					for (int i = 0; i < 15; i++)
					{
						ImGui::Spacing();
					}
					if (ImGui::Button(xorstr_("Color Picker"), ImVec2(95, 30))) { i_MenuTab = 2; }
					for (int i = 0; i < 15; i++)
					{
						ImGui::Spacing();
					}

					if (ImGui::Button(xorstr_("MISC"), ImVec2(95, 30))) { i_MenuTab = 3; }
					for (int i = 0; i < 15; i++)
					{
						ImGui::Spacing();
					}
				
				}
				ImGui::EndChild();

				ImGui::SameLine();

				ImGui::BeginChild(xorstr_("##FEATURESCHILD"), ImVec2(-1, -1), false);
				{
					if (i_MenuTab == 0) Visual();
					if (i_MenuTab == 1) Aimbot();
					if (i_MenuTab == 2) ColorPicker();
					if (i_MenuTab == 3) Misc();

				}
				ImGui::EndChild();
			}

		}
	}
}