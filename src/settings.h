#pragma once

#include "imgui/imgui.h"
#include "valve_sdk/csgostructs.hpp"

struct aimbot_group
{
	std::string name;
	std::vector<int> weapons = { };
};

enum settings_type_t : int
{
	separately = 0,
	subgroups,
	for_all,
	groups,
};

enum trigger_type
{
	in_fov = 0,
	in_crosshair,
};

enum fov_type_t : int
{
	statical = 0,
	dynamic,
	rage,
};

#pragma pack(push, 1)
struct weapon_setting 
{
	bool enabled = false;	
	bool autopistol = false;
	bool check_smoke = false;
	bool check_flash = false;
	bool check_zoom = true;
	bool check_air = true;
	bool by_damage = false;
	bool back_shot = false;

	int min_damage = 0;
	int min_hitchanse = 0;

	struct 
	{
		bool enabled = false;
		int ticks = 6;
		bool legit = false;
	} backtrack;

	bool autodelay = false;

	struct
	{
		bool enabled = false;
		bool in_radius = false;
		int delay = 0;
		int delay_btw_shots = 50;
		int hitchance = 1;
	} trigger;

	struct
	{
		bool enabled = false;
		bool always = false;
		float fov = 0.f;
		int chanse = 7;
		bool with_smooth = true;
	} silent;

	struct {
		bool enabled = false;
		bool first_bullet = false;
		bool humanize = false;
		bool standalone = true;

		float pitch = 2.0f;
		float yaw = 2.0f;
	} recoil;

	struct
	{
		bool head = true;
		bool neck = true;
		bool body = true;
		bool hands = true;
		bool legs = true;
	} hitboxes;

	struct
	{
		bool enabled = false;
		int min_damage = 1;
	} autowall;

	bool dynamic_fov = false;
	float fov = 0.f;
	float smooth = 1;
	int shot_delay = 0;
	int kill_delay = 600;
};
#pragma pack(pop)

enum fake_lag_types : int
{
	lag_always = 0,
	lag_only_in_air,
	lag_when_pick,
	lag_by_button,
};

namespace settings 
{
	void load(std::string name);
	void save(std::string name);

	namespace movement
	{
		extern bool engine_prediction;
	}

	namespace esp 
	{
		extern bool enabled;
		extern bool visible_only;
		extern bool boxes;
		extern int box_type;
		extern bool names;
		extern bool health;
		extern int health_position;
		extern bool armour;
		extern int armour_position;
		extern bool weapons;
		extern bool offscreen;
		extern bool sound;
		extern bool snaplines;
		extern bool is_scoped;
		extern bool dormant;
		extern bool beams;

		extern Color visible_color;
		extern Color occluded_color;
	}

	namespace chams
	{
		extern bool enabled;
		extern bool visible_only;
		extern bool wireframe;
		extern bool flat;

		extern Color visible_color;
		extern Color occluded_color;

		namespace arms
		{
			extern bool enabled;
			extern bool wireframe;

			extern ImVec4 color;
		}
	}

	namespace visuals 
	{
		extern bool grenade_prediction;
		extern bool world_grenades;
		extern bool sniper_crosshair;
		extern bool defuse_kit;
		extern bool planted_c4;
		extern bool dropped_weapons;
		extern bool night_mode;

		extern ImVec4 sky;
	}

	namespace desync
	{
		extern bool enabled;
		extern bool yaw_flip;
		extern bool resolver;
	}

	namespace aimbot 
	{
		extern int setting_type;

		extern std::vector<aimbot_group> m_groups;
		extern std::map<int, weapon_setting> m_items;
	}

	namespace fake_lags
	{
		extern int type;
		extern int factor;
		extern bool enabled;
	}

	namespace lightning_shot
	{
		extern bool enabled;
		extern int delay;
		extern bool check_zoom;
	}

	namespace misc 
	{
		extern bool bhop;
		extern bool auto_strafe;
		extern bool no_flash;
		extern bool no_smoke;
		extern float viewmodel_fov;
		extern bool moon_walk;
		extern bool deathmatch;
		extern bool knife_bot;
		extern bool radar;
	}
}

