#pragma once

#include "../settings.h"
#include "../helpers/math.h"
#include "../valve_sdk/csgostructs.hpp"

namespace features
{
	void bhop(CUserCmd* cmd);
	void moon_walk(CUserCmd* cmd);
	void auto_strafe(CUserCmd* cmd);
	void thirdperson();
}

namespace color_modulation
{
	void event();
	void handle();
}

namespace fake_lags
{
	void handle(CUserCmd* cmd, bool& send_packet);
}

namespace visuals
{
	void fetch_entities();
	void render(ImDrawList* draw_list);
}

namespace esp
{
	void render(ImDrawList* draw_list);
}

namespace aimbot 
{
	void handle(CUserCmd *pCmd);
}

namespace glows
{
	void handle();
	void shutdown();
}

namespace skins
{
	extern std::map<int, item_setting> m_items;
	extern std::unordered_map<std::string, std::string> m_icon_overrides;

	extern std::vector<paint_kit_t> skin_kits;

	extern std::vector<weapon_name_t> knife_names;
	extern std::vector<weapon_name_t> glove_names;

	void save();
	void load();
	void initialize_kits();

	void handle();

	const weapon_info_t* get_weapon_info(const int& defindex);
	const char* get_icon_override(const std::string& original);
}

namespace lighting_shots
{
	extern bool need_switch_weapon;

	void handle(CUserCmd* cmd);
}

namespace zeusbot
{
	void handle(CUserCmd* cmd);
}

namespace grenade_prediction
{
	void fetch_points(CUserCmd* cmd);
	void render(ImDrawList* draw_list);

	int PhysicsClipVelocity(const Vector& in, const Vector& normal, Vector& out, float overbounce);
	void PushEntity(Vector& src, const Vector& move, trace_t& tr);
	void TraceHull(Vector& src, Vector& end, trace_t& tr);
	void AddGravityMove(Vector& move, Vector& vel, float frametime, bool onground);
	void ResolveFlyCollisionCustom(trace_t& tr, Vector& vecVelocity, float interval);
	void Setup(Vector& vecSrc, Vector& vecThrow, QAngle viewangles);
}

namespace offscreen_entities
{ 
	extern ImU32 sound_color;
	extern ImU32 origin_color;

	void dot(const Vector& origin, const Vector& pos, const ImU32& color);
	void render(ImDrawList* draw_list);
}

namespace engine_prediction
{
	void start(CUserCmd* cmd);
	void finish(CUserCmd* cmd);
}

namespace clantag
{
	void set(const char* tag);
	void set(const char* tag, const char* label);

	void restore();
	void animate();
}

namespace desync
{
	extern bool flip_yaw;
	void handle(CUserCmd* cmd, bool& send_packet);
}

namespace fake_duck
{
	extern bool current_state;

	void handle(CUserCmd* cmd, bool& send_packet);
}

namespace lobby_inviter
{
	extern 	uint32_t max_count;

	void update();
	void inviteAll();
}

namespace slow_walk
{
	void handle(CUserCmd* cmd);
}

namespace knife_bot
{
	void handle(CUserCmd* cmd, bool& send_packet);
}

namespace resolver
{
	void handle();
}

namespace no_flash
{
	void handle();
}

namespace no_smoke
{
	void event();
	void handle();
}