#include "utils.h"
#include "math.h"
#include "input.h"
#include "console.h"
#include "../globals.h"
#include "../security/importer.h"
#include "../features/features.h"
#include "../imgui/imgui_internal.h"
#include "../valve_sdk/csgostructs.hpp"

#define NOMINMAX
#include <Windows.h>
#include <string>
#include <vector>

namespace utils 
{
	float get_interpolation_compensation()
	{
		static const auto cl_interp = interfaces::cvar->find("cl_interp");
		static const auto max_ud_rate = interfaces::cvar->find("sv_maxupdaterate");
		static const auto cl_interp_ratio = interfaces::cvar->find("cl_interp_ratio");
		static const auto c_min_ratio = interfaces::cvar->find("sv_client_min_interp_ratio");
		static const auto c_max_ratio = interfaces::cvar->find("sv_client_max_interp_ratio");

		float ratio = cl_interp_ratio->GetFloat();
		if (ratio == 0)
			ratio = 1.0f;

		if (c_min_ratio && c_max_ratio && c_min_ratio->GetFloat() != 1)
			ratio = std::clamp(ratio, c_min_ratio->GetFloat(), c_max_ratio->GetFloat());

		const auto ud_rate = max_ud_rate->GetInt();

		return std::max(cl_interp->GetFloat(), (ratio / ud_rate));
	}

	void create_beam(const int& user_id, const Vector& end_pos)
	{
		if (!end_pos.IsValid())
			return;

		c_base_player* player = c_base_player::GetPlayerByUserId(user_id);
		if (!player || !player->IsPlayer() || player == interfaces::local_player)
			return;

		if (player->m_iTeamNum() == interfaces::local_player->m_iTeamNum() && !settings::misc::deathmatch)
			return;

		if (player->GetEyePos().DistTo(end_pos) < 0.1f)
			return;

		BeamInfo_t beamInfo;
		beamInfo.m_nType = TE_BEAMPOINTS;
		beamInfo.m_pszModelName = "sprites/physbeam.vmt";
		beamInfo.m_nModelIndex = -1;
		beamInfo.m_flHaloScale = 0.0f;
		beamInfo.m_flLife = 1.f;
		beamInfo.m_flWidth = 2.0f;
		beamInfo.m_flEndWidth = 2.0f;
		beamInfo.m_flFadeLength = 0.0f;
		beamInfo.m_flAmplitude = 2.0f;
		beamInfo.m_flBrightness = 255.f;
		beamInfo.m_flSpeed = 0.2f;
		beamInfo.m_nStartFrame = 0;
		beamInfo.m_flFrameRate = 0.f;
		beamInfo.m_flRed = player->m_iTeamNum() == team::team_ct ? 0.f : 240.f;
		beamInfo.m_flGreen = 50.f;
		beamInfo.m_flBlue = player->m_iTeamNum() == team::team_ct ? 240.f : 0.f;
		beamInfo.m_nSegments = 2;
		beamInfo.m_bRenderable = true;
		beamInfo.m_nFlags = FBEAM_FADEIN | FBEAM_FADEOUT;
		beamInfo.m_vecStart = player->GetEyePos();
		beamInfo.m_vecEnd = end_pos;

		Beam_t* beam = interfaces::view_render_beams->CreateBeamPoints(beamInfo);
		if (beam)
			interfaces::view_render_beams->DrawBeam(beam);
	}

	ImU32 to_im32(const Color& color, const float& alpha)
	{
		return ImGui::GetColorU32(ImVec4(color.r() / 255.f, color.g() / 255.f, color.b() / 255.f, alpha));
	}

	void* get_export(const char* module_name, const char* export_name)
	{
		HMODULE mod;
		while (!((mod = LI_FN(GetModuleHandleA).cached()(module_name))))
			LI_FN(Sleep).cached()(100);

		return reinterpret_cast<void*>(GetProcAddress(mod, export_name));
	}

	std::string get_weapon_name(void* weapon)
	{
		static const auto V_UCS2ToUTF8 = static_cast<int(*)(const wchar_t* ucs2, char* utf8, int len)>(get_export("vstdlib.dll", "V_UCS2ToUTF8"));

		if (!weapon)
			return "";

		const auto wide_name = interfaces::localize->Find(((c_base_combat_weapon*) weapon)->get_weapon_data()->szHudName);

		char weapon_name[256];
		V_UCS2ToUTF8(wide_name, weapon_name, sizeof(weapon_name));

		return weapon_name;
	}

	wchar_t* to_wstring(const char* str)
	{
		const auto size = strlen(str) + 1;

		auto* wc = new wchar_t[size];
		MultiByteToWideChar(CP_UTF8, 0, str, -1, wc, size);

		return wc;
	}

	bool hitchance(c_base_entity* entity, const QAngle& angles, const float& chance, const float& hit_count, const int& hit_group)
	{
		if (!interfaces::local_player)
			return false;

		auto weapon = interfaces::local_player->m_hActiveWeapon();
		if (!weapon || !weapon->IsWeapon())
			return false;

		Vector forward, right, up;
		Vector src = interfaces::local_player->GetEyePos();
		math::angle2vectors(angles, forward, right, up);

		int cHits = 0;
		int cNeededHits = static_cast<int>(hit_count * (chance / 100.f));

		weapon->UpdateAccuracyPenalty();
		float weap_spread = weapon->GetSpread();
		float weap_inaccuracy = weapon->GetInaccuracy();

		Ray_t ray;
		trace_t tr;
		CTraceFilterPlayersOnlySkipOne filter(interfaces::local_player);

		for (int i = 0; i < hit_count; i++)
		{
			float a = utils::random(0.f, 1.f);
			float b = utils::random(0.f, 2.f * M_PI);
			float c = utils::random(0.f, 1.f);
			float d = utils::random(0.f, 2.f * M_PI);

			float inaccuracy = a * weap_inaccuracy;
			float spread = c * weap_spread;

			if (weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER)
			{
				a = 1.f - a * a;
				a = 1.f - c * c;
			}

			Vector spreadView((cos(b) * inaccuracy) + (cos(d) * spread), (sin(b) * inaccuracy) + (sin(d) * spread), 0), direction;

			direction.x = forward.x + (spreadView.x * right.x) + (spreadView.y * up.x);
			direction.y = forward.y + (spreadView.x * right.y) + (spreadView.y * up.y);
			direction.z = forward.z + (spreadView.x * right.z) + (spreadView.y * up.z);
			direction.Normalized();

			QAngle viewAnglesSpread;
			math::vector2angles(direction, up, viewAnglesSpread);
			viewAnglesSpread.NormalizeClamp();

			Vector viewForward;
			math::angle2vectors(viewAnglesSpread, viewForward);
			viewForward.NormalizeInPlace();

			viewForward = src + (viewForward * weapon->get_weapon_data()->flRange);

			ray.Init(src, viewForward);
			interfaces::engine_trace->trace_ray(ray, MASK_SHOT | CONTENTS_GRATE, &filter , &tr);

			if (tr.hit_entity == entity && (hit_group == -1 || hit_group == tr.hitgroup))
				++cHits;

			const auto ch = static_cast<int>((static_cast<float>(cHits) / hit_count) * 100.f);
			if (ch >= chance)
				return true;

			if ((hit_count - i + cHits) < cNeededHits)
				return false;
		}

		return false;
	}

	bool can_lag(const bool& voice_check)
	{
		if (interfaces::global_vars->interval_per_tick * 0.9f < interfaces::global_vars->absoluteframetime)
			return false;

		if (voice_check && interfaces::engine_client->IsVoiceRecording())
			return false;

		auto* channel_info = interfaces::engine_client->GetNetChannelInfo();
		if (channel_info && (channel_info->GetAvgLoss(1) > 0.f || channel_info->GetAvgLoss(0) > 0.f))
			return false;

		return true;
	}

	int get_active_key()
	{
		ImGuiContext& g = *GImGui;
		ImGuiIO& io = g.IO;
		for (auto i = 0; i < 5; i++)
		{
			if (io.MouseDown[i])
			{
				switch (i)
				{
				case 0:
					return VK_LBUTTON;
				case 1:
					return VK_RBUTTON;
				case 2:
					return VK_MBUTTON;
				case 3:
					return VK_XBUTTON1;
				case 4:
					return VK_XBUTTON2;
				}
			}
		}

		for (auto i = VK_BACK; i <= VK_RMENU; i++)
		{
			if (io.KeysDown[i])
				return i;
		}

		if (input_system::is_key_down(ImGuiKey_Escape))
			return 0;

		return -1;
	}

	int random(const int& min, const int& max)
	{
		return rand() % (max - min + 1) + min;
	}

	float random(const float& min, const float& max)
	{
		return ((max - min) * ((float)rand() / RAND_MAX)) + min;
	}

	bool is_sniper(int iItemDefinitionIndex)
	{
		return iItemDefinitionIndex == WEAPON_AWP || iItemDefinitionIndex == WEAPON_SSG08 ||
			iItemDefinitionIndex == WEAPON_SCAR20 || iItemDefinitionIndex == WEAPON_G3SG1;
	}

	bool is_connected()
	{
		return interfaces::engine_client->IsInGame() && interfaces::local_player && interfaces::local_player->IsAlive();
	}

	struct hud_weapons_t 
	{
		std::int32_t* get_weapon_count() 
		{
			return reinterpret_cast<std::int32_t*>(std::uintptr_t(this) + 0x80);
		}
	};

	void force_full_update()
	{
		return;
		static const auto full_update_fn = reinterpret_cast<void(*)(void)>(pattern_scan(FORCE_FULL_UPDATE));
		full_update_fn();

		//interfaces::client_state->ForceFullUpdate();
		return;
		if (interfaces::local_player)
		{
			//interfaces::client_state->ForceFullUpdate();
			//interfaces::local_player->PostDataUpdate(0);
		}

		if (!interfaces::local_player || !interfaces::local_player->IsAlive())
			return;

		static auto clear_hud_weapon_icon_ptr = utils::pattern_scan(CLEAR_HUD_WEAPON_ICON);
		static auto clear_hud_weapon_icon_fn = reinterpret_cast<std::int32_t(__thiscall*)(void*, std::int32_t)>(clear_hud_weapon_icon_ptr);

		auto element = interfaces::Hud->FindHudElement<std::uintptr_t*>("CCSGO_HudWeaponSelection");
		auto hud_weapons = reinterpret_cast<hud_weapons_t*>(std::uintptr_t(element) - 0x9c);
		if (!hud_weapons || *hud_weapons->get_weapon_count() == 0)
			return;

		for (std::int32_t i = 0; i < *hud_weapons->get_weapon_count(); i++)
			i = clear_hud_weapon_icon_fn(hud_weapons, i);
	}

	bool is_line_goes_through_smoke(Vector vStartPos, Vector vEndPos)
	{
		static auto fn = reinterpret_cast<bool(*)(Vector, Vector)>(pattern_scan(IS_LINE_GOES_THROUGH_SMOKE_FN));

		return fn(vStartPos, vEndPos);
	}

	void set_player_name(const char* name)
	{
		globals::playername = name;

		static auto nameConvar = interfaces::cvar->find("name");
		nameConvar->m_fnChangeCallbacks.m_Size = 0;

		nameConvar->SetValue(name);
	}

	std::map<std::string, HMODULE> modules = {};

	HMODULE get_module(const std::string& name)
	{
		const auto module_name = name == "client.dll" ? "client_panorama.dll" : name;

		if (modules.count(module_name) == 0 || !modules[module_name])
			modules[module_name] = LI_FN(GetModuleHandleA).cached()(module_name.c_str());

		return modules[module_name];
	}

	std::uint8_t* pattern_scan(const char* moduleName, const char* signature)
	{
		return pattern_scan(get_module(moduleName), signature);
	}

    std::uint8_t* pattern_scan(void* module, const char* signature)
    {
        static auto pattern_to_byte = [](const char* pattern) {
            auto bytes = std::vector<int>{};
            auto start = const_cast<char*>(pattern);
            auto end = const_cast<char*>(pattern) + strlen(pattern);

            for(auto current = start; current < end; ++current) 
			{
                if(*current == '?') 
				{
                    ++current;
                    if(*current == '?')
                        ++current;
                    bytes.emplace_back(-1);
                } 
				else
                    bytes.emplace_back(strtoul(current, &current, 16));
            }
            return bytes;
        };

        auto dosHeader = (PIMAGE_DOS_HEADER)module;
        auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)module + dosHeader->e_lfanew);

        auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
        auto patternBytes = pattern_to_byte(signature);
        auto scanBytes = reinterpret_cast<std::uint8_t*>(module);

        auto s = patternBytes.size();
        auto d = patternBytes.data();

        for(auto i = 0ul; i < sizeOfImage - s; ++i) 
		{
            bool found = true;
            for(auto j = 0ul; j < s; ++j) 
			{
                if(scanBytes[i + j] != d[j] && d[j] != -1) {
                    found = false;
                    break;
                }
            }

            if(found) 
                return &scanBytes[i];
        }
        return nullptr;
    }
}