#include "settings.h"
#include "config.h"
#include "options.hpp"
#include "jsoncpp/json.h"
#include "helpers/imdraw.h"

namespace settings
{
	namespace movement
	{
		bool engine_prediction = false;
	}

	namespace esp
	{
		bool enabled = false;
		bool visible_only = false;
		bool boxes = false;
		int box_type = 0;
		bool names = false;
		bool health = false;
		int health_position = 0;
		bool armour = false;
		int armour_position = 1;
		bool weapons = false;
		bool offscreen = false;
		bool sound = false;
		bool snaplines = false;
		bool is_scoped = false;
		bool dormant = false;
		bool beams = false;
		int esp_on_chance;
		int esp_off_chance;
		bool is_flashed = false;
		bool is_defusing = false;
		bool is_reloading = false;
		float mfts = 0.1f;
		bool mat_force = false;
		bool drawFov;
		bool bomb_esp = false;
		bool kevlarinfo = false;
		bool haskit = false;
		bool money = false;
		bool antiobs = false;

		Color visible_color = Color(0, 200, 80);
		Color occluded_color = Color(0, 0, 0);
	}

	namespace glow
	{
		bool GlowEnemyEnabled = false;
		bool GlowTeamEnabled = false;
		bool GlowC4PlantedEnabled = false;
		bool GlowDroppedWeaponsEnabled = false;
		bool GlowNadesEnabled = false;

		float GlowEnemy[4]{ 1.0f, 0.f, 0.f, 255.0f };
		float GlowTeam[4]{ 0.0f, 1.f, 0.f, 255.0f };

		float GlowC4Planted[4]{ 1.0f, 0.f, 0.f, 255.0f };
		float GlowDroppedWeapons[4]{ 1.0f, 0.f, 0.f, 255.0f };
		float GlowNades[4]{ 1.0f, 0.f, 0.f, 255.0f };

	}

	namespace chams
	{
		bool enabled = false;
		bool visible_only = true;
		bool wireframe = false;
		bool flat = false;
		bool desync = false;
		bool localnew = false;
		bool teamnew = false;
		bool enemynew = false;
		bool btchams = false;
		bool btflat = false;
		Color btcolor = Color(0, 255, 248, 147);
		bool xqz = false;
		int bttype = 0;
		float clr_weapon_chams[4]{ 1.0f, 1.0f, 1.0f, 255.0f };
		float clr_weapon_dropped_chams[4]{ 1.0f, 1.0f, 1.0f, 255.0f };
		float clr_weapon_dropped_chams_xqz[4]{ 1.0f, 1.0f, 1.0f, 255.0f };
		Color wepcolor = Color(0, 200, 80);
		bool wepchams = false;
		bool wep_droppedchams = false;
		bool wep_droppedchams_xqz = false;

		bool nade_chams = false;
		bool nade_chams_xqz = false;
		float clr_nade_chams_xqz[4]{ 1.0f, 1.0f, 1.0f, 255.0f };
		float clr_nade_chams[4]{ 1.0f, 1.0f, 1.0f, 255.0f };

		bool plantedc4_chams = false;
		bool plantedc4_chams_xqz = false;
		float clr_plantedc4_chams[4]{ 1.0f, 1.0f, 1.0f, 255.0f };
		float clr_plantedc4_chams_xqz[4]{ 1.0f, 1.0f, 1.0f, 255.0f };

		int matmode;


		int localmodenew = 0;
		int enemymodenew = 0;
		int teammodenew = 0;

		Color visible_color = Color(0, 200, 80);
		Color occluded_color = Color(0, 0, 0);

		Color LocalColor_vis = Color(0, 200, 80);
		Color LocalColor_invis = Color(0, 0, 0);

		Color TeamColor_vis = Color(0, 200, 80);
		Color TeamColor_invis = Color(0, 0, 0);

		Color EnemyColor_vis = Color(0, 200, 0);
		Color EnemyColor_invis = Color(0, 0, 0);

		Color LocalColor_XQZ = Color(0, 200, 0);
		Color TeamColor_XQZ = Color(0, 200, 0);
		Color EnemyColor_XQZ = Color(0, 200, 0);

		namespace arms
		{
			bool enabled = false;
			bool wireframe = true;

			ImVec4 color = ImVec4::Red;
		}
	}

	namespace visuals
	{
		bool grenade_prediction = false;
		bool world_grenades = false;
		bool sniper_crosshair = true;
		bool defuse_kit = false;
		bool planted_c4 = false;
		bool dropped_weapons = false;
		bool night_mode = false;
		bool pitch = false;
		bool hitmarker = false;
		int hitsound;
		bool skychanger = false;
		int skychanger_mode = 0;
		bool newstyle = true;
		bool rcs_cross = false;
		bool bullet_tracer = false;
		float radius = 12;
		int rcs_cross_mode;
		bool spread_cross = false;
		bool a = false;
		bool b = false;
		bool choke = false;
		bool skeletons = false;
		bool rankrevealall = false;

		ImVec4 sky = ImVec4(0.25, 0.25, 0.25, 1.0);
		Color recoilcolor = Color(0, 0, 0);
		Color floatc = Color(154.f, 0.f, 255.f, 255.f);
		Color clr_bullet_tracer = Color(154, 0, 255, 255);
		Color spread_cross_color = Color(255, 0, 0, 255);
		Color skeletoncolor = Color(255, 255, 255, 255);
	}

	namespace desync
	{
		bool enabled = false;
		bool yaw_flip = false;
		bool resolver = false;
		int desync_mode = 0;
		bool enabled2 = false;
	}

	namespace aimbot
	{
		int setting_type = settings_type_t::separately;

		std::vector<aimbot_group> m_groups = { };
		std::map<int, weapon_setting> m_items = { };
	}

	namespace fake_lags
	{
		bool enabled = false;
		int type = fake_lag_types::lag_always;
		int factor = 12;
	}

	namespace lightning_shot
	{
		bool enabled = false;
		int delay = 4;
		bool check_zoom = true;
	}

	namespace misc
	{
		bool radar = false;
		bool bhop = false;
		bool auto_strafe = false;
		bool no_smoke = false;
		bool no_flash = false;
		bool human_bhop = false;
		float viewmodel_fov = 68.f;
		bool moon_walk = false;
		bool deathmatch = false;
		bool knife_bot = false;
		int bhop_hit_chance;
		int hops_restricted_limit;
		int max_hops_hit;
		int debug_fov = 90;
		bool triggerbot_in_zoom_only = false;
		bool esp_random;
		bool lknife;
		bool noscope;
		bool selfnade;
		bool lefthandknife;
		bool legitpeek = false;
		bool on_detach;
	};

	void load(std::string name)
	{
		config::load(name, "settings", true, [](Json::Value root)
		{
			Option::Load(root["esp.enabled"], esp::enabled);
			Option::Load(root["esp.visible_only"], esp::visible_only);
			Option::Load(root["esp.boxes"], esp::boxes, true);
			Option::Load(root["esp.names"], esp::names);
			Option::Load(root["esp.box_type"], esp::box_type, EBoxType::Normal);
			Option::Load(root["esp.health"], esp::health);
			Option::Load(root["esp.health_position"], esp::health_position, 0);
			Option::Load(root["esp.armour"], esp::armour);
			Option::Load(root["esp.armour_position"], esp::armour_position, 1);
			Option::Load(root["esp.weapons"], esp::weapons);
			Option::Load(root["esp.offscreen"], esp::offscreen);
			Option::Load(root["esp.sound"], esp::sound);
			Option::Load(root["esp.dormant"], esp::dormant);
			Option::Load(root["esp.is_scoped"], esp::is_scoped);
			Option::Load(root["esp.snaplines"], esp::snaplines);
			Option::Load(root["esp.beams"], esp::beams);
			Option::Load(root["esp.random"], misc::esp_random);
			Option::Load(root["esp.esp_on_chance"], esp::esp_on_chance);
			Option::Load(root["esp.esp_off_chance"], esp::esp_off_chance);
			Option::Load(root["esp.is_flashed"], esp::is_flashed);
			Option::Load(root["esp.is_defusing"], esp::is_defusing);
			Option::Load(root["esp.mat_force_enabled"], esp::mat_force);
			Option::Load(root["esp.mat_force_value"], esp::mfts);
			Option::Load(root["esp.draw_aimbot_fov"], esp::drawFov);

			Option::Load(root["esp.visible_color"], esp::visible_color, Color(0, 200, 80));
			Option::Load(root["esp.occluded_color"], esp::occluded_color, Color::Black);

			Option::Load(root["visuals.world_grenades"], visuals::world_grenades, true);
			Option::Load(root["visuals.rankrevealall"], visuals::rankrevealall, true);

			Option::Load(root["visuals.sniper_crosshair"], visuals::sniper_crosshair, true);
			Option::Load(root["visuals.grenade_prediction"], visuals::grenade_prediction, true);
			Option::Load(root["visuals.defuse_kit"], visuals::defuse_kit);
			Option::Load(root["visuals.planted_c4"], visuals::planted_c4);
			Option::Load(root["visuals.dropped_weapons"], visuals::dropped_weapons);
			Option::Load(root["visuals.sky"], visuals::sky, ImVec4(0.25, 0.25, 0.25, 1.0));
			Option::Load(root["visuals.night_mode"], visuals::night_mode);

			imdraw::apply_style(visuals::night_mode);

			//Option::Load(root["chams.enabled"], chams::enabled, true); //old chams
			Option::Load(root["chams.visible_only"], chams::visible_only, true);
			Option::Load(root["chams.flat"], chams::flat, false);
			Option::Load(root["chams.wireframe"], chams::wireframe, false);

			Option::Load(root["chams.enemy"], chams::enemynew);
			Option::Load(root["chams.team"], chams::teamnew);
			Option::Load(root["chams.local"], chams::localnew);

			Option::Load(root["chams.enemy_mode"], chams::enemymodenew);
			Option::Load(root["chams.team_mode"], chams::teammodenew);
			Option::Load(root["chams.local_mode"], chams::localmodenew);

			Option::Load(root["chams.legit_aa"], chams::desync);
			Option::Load(root["chams.backtrack"], chams::bttype);
			Option::Load(root["chams.backtrack_flat"], chams::btflat);
			Option::Load(root["chams.backtrack_color"], chams::btcolor, Color(0, 255, 248, 147));

			Option::Load(root["chams.enemy_color_vis"], chams::EnemyColor_vis, Color::Black);
			Option::Load(root["chams.team_color_vis"], chams::TeamColor_vis, Color::Black);
			Option::Load(root["chams.local_color_vis"], chams::LocalColor_vis, Color::Black);

			Option::Load(root["chams.enemy_color_xqz"], chams::EnemyColor_XQZ, Color(0, 200, 80));
			Option::Load(root["chams.team_color_xqz"], chams::TeamColor_XQZ, Color(0, 200, 80));
			Option::Load(root["chams.local_color_xqz"], chams::LocalColor_XQZ, Color(0, 200, 80));

			Option::Load(root["chams.viewmodel_weapons"], chams::wepchams);
			//Option::Load(root["chams.viewmodel_weapons_color"], chams::clr_weapon_chams[3], (255.f, 255.f, 255.f, 255.f));

			Option::Load(root["chams.dropped_weapons"], chams::wep_droppedchams);
			//Option::Load(root["chams.dropped_weapons_color"], chams::clr_weapon_dropped_chams[3]); //here?

			Option::Load(root["chams.plantedc4"], chams::plantedc4_chams);
			//Option::Load(root["chams.plantedc4_color"], chams::clr_plantedc4_chams[3]);

			Option::Load(root["chams.nades"], chams::nade_chams);
			//Option::Load(root["chams.nades_color"], chams::clr_nade_chams[3]);

			Option::Load(root["chams.arms.enabled"], chams::arms::enabled, false);
			Option::Load(root["chams.arms.wireframe"], chams::arms::wireframe, true);
			Option::Load(root["chams.arms.color"], chams::arms::color, ImVec4::Red);

			Option::Load(root["chams.visible_color"], chams::visible_color, Color(0, 200, 80));
			Option::Load(root["chams.occluded_color"], chams::occluded_color, Color::Black);

			Option::Load(root["misc.bhop"], misc::bhop);
			Option::Load(root["misc.auto_strafe"], misc::auto_strafe);
			Option::Load(root["misc.viewmodel_fov"], misc::viewmodel_fov, 68);
			Option::Load(root["misc.moon_walk"], misc::moon_walk);
			Option::Load(root["misc.deathmatch"], misc::deathmatch);
			Option::Load(root["misc.no_flash"], misc::no_flash);
			Option::Load(root["misc.no_smoke"], misc::no_smoke);
			Option::Load(root["misc.knife_bot"], misc::knife_bot);
			Option::Load(root["misc.radar"], misc::radar);
			Option::Load(root["misc.human_bhop"], misc::human_bhop);
			Option::Load(root["misc.human_bhop_hopchance"], misc::bhop_hit_chance);
			Option::Load(root["misc.human_bhop_hopslimit"], misc::hops_restricted_limit);
			Option::Load(root["misc.human_bhop_maxhops_hit"], misc::max_hops_hit);
			Option::Load(root["misc.resolver"], desync::resolver);
			Option::Load(root["misc.engine_prediction"], movement::engine_prediction);
			Option::Load(root["misc.knife_bot"], misc::knife_bot);
			Option::Load(root["misc.debug_fov"], misc::debug_fov);
			Option::Load(root["misc.skychanger"], visuals::skychanger);
			Option::Load(root["misc.skychanger_mode"], visuals::skychanger_mode);
			Option::Load(root["misc.menu_style"], visuals::newstyle);
			Option::Load(root["misc.rcs_cross"], visuals::rcs_cross);
			Option::Load(root["misc.rcs_cross_mode"], visuals::rcs_cross_mode);
			Option::Load(root["misc.bullet_tracer"], visuals::bullet_tracer, false);
			//Option::Load(root["misc.bullet_tracer_color"], visuals::clr_bullet_tracer, Color(154, 0, 255, 255));
			Option::Load(root["misc.radius"], visuals::radius, 12.f);
			Option::Load(root["mics.rcs_cross_color"], visuals::recoilcolor);
			Option::Load(root["misc.no_scope_overlay"], misc::noscope);
			Option::Load(root["misc.self_nade"], misc::selfnade);
			Option::Load(root["misc.hitmarker"], visuals::hitmarker);
			Option::Load(root["misc.hitmarker_sound"], visuals::hitsound);
			Option::Load(root["misc.bomb_damage_esp"], esp::bomb_esp);
			Option::Load(root["misc_kevlar_info"], settings::esp::kevlarinfo);
			Option::Load(root["misc_spread_crosshair"], settings::visuals::spread_cross);
			Option::Load(root["misc_spread_crosshair_color"], settings::visuals::spread_cross_color);
			Option::Load(root["misc_esp_money"], settings::esp::money);
			Option::Load(root["misc_esp_kit"], settings::esp::haskit);
			Option::Load(root["misc.choke_indicator"], settings::visuals::choke);

			Option::Load(root["glow.enemy"], glow::GlowEnemyEnabled);
			Option::Load(root["glow.team"], glow::GlowTeamEnabled);
			Option::Load(root["glow.plantedc4"], glow::GlowC4PlantedEnabled);
			Option::Load(root["glow.nades"], glow::GlowNadesEnabled);
			Option::Load(root["glow.dropped_weapons"], glow::GlowDroppedWeaponsEnabled);

			Option::Load(root["fake_lags.enabled"], fake_lags::enabled);
			Option::Load(root["fake_lags.factor"], fake_lags::factor, 6);
			Option::Load(root["fake_lags.type"], fake_lags::type);

			Option::Load(root["desync.enabled"], desync::enabled);
			Option::Load(root["desync.yaw_flip"], desync::yaw_flip);
			Option::Load(root["desync.resolver"], desync::resolver);
			Option::Load(root["desync.enabled2"], desync::enabled2);
			Option::Load(root["desync_mode"], desync::desync_mode);

			Option::Load(root["legitbot.setting_type"], aimbot::setting_type);

			aimbot::m_items.clear();

			Json::Value aimbot_items = root["legitbot.items"];
			for (Json::Value::iterator it = aimbot_items.begin(); it != aimbot_items.end(); ++it)
			{
				Json::Value settings = *it;

				weapon_setting data;
				Option::Load(settings["enabled"], data.enabled);
				Option::Load(settings["back_shot"], data.back_shot);
				Option::Load(settings["autodelay"], data.autodelay);
				Option::Load(settings["autopistol"], data.autopistol);
				Option::Load(settings["check_smoke"], data.check_smoke);
				Option::Load(settings["check_flash"], data.check_flash);
				Option::Load(settings["check_air"], data.check_air, true);
				Option::Load(settings["backtrack.enabled"], data.backtrack.enabled);
				Option::Load(settings["backtrack.legit"], data.backtrack.legit);
				Option::Load(settings["backtrack.ticks"], data.backtrack.ticks, 6);
				Option::Load(settings["backtrack.time"], data.backtrack.time, 200);

				if (data.backtrack.ticks < 1 || data.backtrack.ticks > 12)
					data.backtrack.ticks = 6;

				if (data.backtrack.time < 0 || data.backtrack.time > 200)
					data.backtrack.time = 200;

				Option::Load(settings["check_zoom"], data.check_zoom, true);
				Option::Load(settings["silent.enabled"], data.silent.enabled, false);
				Option::Load(settings["silent.fov"], data.silent.fov, 0.f);
				Option::Load(settings["silent.always"], data.silent.always, false);
				Option::Load(settings["silent.chanse"], data.silent.chanse, 7);
				if (data.silent.chanse > 10)
					data.silent.chanse = std::clamp<int>(data.silent.chanse / 10, 0, 10);

				Option::Load(settings["silent.with_smooth"], data.silent.with_smooth, true);

				Option::Load(settings["trigger.enabled"], data.trigger.enabled);
				Option::Load(settings["trigger.in_radius"], data.trigger.in_radius);
				Option::Load(settings["trigger.delay"], data.trigger.delay);
				Option::Load(settings["trigger.delay_btw_shots"], data.trigger.delay_btw_shots, 50);
				Option::Load(settings["trigger.hitchance"], data.trigger.hitchance, 1);

				Option::Load(settings["hitboxes.head"], data.hitboxes.head, true);
				Option::Load(settings["hitboxes.neck"], data.hitboxes.neck, true);
				Option::Load(settings["hitboxes.body"], data.hitboxes.body, true);
				Option::Load(settings["hitboxes.hands"], data.hitboxes.hands);
				Option::Load(settings["hitboxes.legs"], data.hitboxes.legs);

				Option::Load(settings["dynamic_fov"], data.dynamic_fov);
				Option::Load(settings["fov"], data.fov);
				Option::Load(settings["smooth"], data.smooth, 1);
				Option::Load(settings["shot_delay"], data.shot_delay);
				Option::Load(settings["kill_delay"], data.kill_delay, 600);
				Option::Load(settings["by_damage"], data.by_damage);
				Option::Load(settings["min_damage"], data.min_damage, 0);
				Option::Load(settings["hitchanse"], data.min_hitchanse, 0);

				Option::Load(settings["autowall.enabled"], data.autowall.enabled);
				Option::Load(settings["autowall.min_damage"], data.autowall.min_damage, 1);

				Option::Load(settings["rcs"], data.recoil.enabled);
				Option::Load(settings["rcs.humanize"], data.recoil.humanize);
				Option::Load(settings["rcs.first_bullet"], data.recoil.first_bullet);
				Option::Load(settings["rcs.standalone"], data.recoil.standalone, true);
				Option::Load(settings["rcs_value_x"], data.recoil.pitch, 2.f);
				Option::Load(settings["rcs_value_y"], data.recoil.yaw, 2.f);

				aimbot::m_items[it.key().asInt()] = data;
			}

			settings::aimbot::m_groups.clear();

			for (auto group : root["aimbot.groups"])
			{
				std::vector<int> weapons = { };

				for (auto& weapon_index : group["weapons"])
				{
					weapons.emplace_back(weapon_index.asInt());
				}

				settings::aimbot::m_groups.emplace_back(aimbot_group{ group["name"].asString(), weapons });
			}
		});
	}

	void save(std::string name)
	{
		config::save(name, "settings", true, []()
		{
			Json::Value config;

			config["esp.enabled"] = esp::enabled;
			config["esp.visible_only"] = esp::visible_only;
			config["esp.boxes"] = esp::boxes;
			config["esp.names"] = esp::names;
			config["esp.box_type"] = esp::box_type;
			config["esp.health"] = esp::health;
			config["esp.health_position"] = esp::health_position;
			config["esp.armour"] = esp::armour;
			config["esp.armour_position"] = esp::armour_position;
			config["esp.weapons"] = esp::weapons;
			config["esp.offscreen"] = esp::offscreen;
			config["esp.sound"] = esp::sound;
			config["esp.dormant"] = esp::dormant;
			config["esp.is_scoped"] = esp::is_scoped;
			config["esp.snaplines"] = esp::snaplines;
			config["esp.beams"] = esp::beams;
			config["esp.random"] = misc::esp_random;
			config["esp.esp_on_chance"] = esp::esp_on_chance;
			config["esp.esp_off_chance"] = esp::esp_off_chance;
			config["esp.is_defusing"] = esp::is_defusing;
			config["esp.is_flashed"] = esp::is_flashed;
			config["esp.random"] = misc::esp_random;
			config["esp.mat_force_enabled"] = esp::mat_force;
			config["esp.mat_force_value"] = esp::mfts;
			config["esp.draw_aimbot_fov"] = esp::drawFov;

			Option::Save(config["esp.visible_color"], esp::visible_color);
			Option::Save(config["esp.occluded_color"], esp::occluded_color);

			config["visuals.world_grenades"] = visuals::world_grenades;
			config["visuals.rankrevealall"] = visuals::rankrevealall;
			config["visuals.sniper_crosshair"] = visuals::sniper_crosshair;
			config["visuals.grenade_prediction"] = visuals::grenade_prediction;
			config["visuals.defuse_kit"] = visuals::defuse_kit;
			config["visuals.planted_c4"] = visuals::planted_c4;
			config["visuals.dropped_weapons"] = visuals::dropped_weapons;
			config["visuals.night_mode"] = visuals::night_mode;

			Option::Save(config["visuals.sky"], visuals::sky);

			//config["chams.enabled"] = chams::enabled; //old chams
			config["chams.visible_only"] = chams::visible_only;
			config["chams.flat"] = chams::flat;
			config["chams.wireframe"] = chams::wireframe;
			config["chams.enemy_mode"] = chams::enemymodenew;
			config["chams.team_mode"] = chams::teammodenew;
			config["chams.local_mode"] = chams::localmodenew;
			config["chams.enemy"] = chams::enemynew;
			config["chams.team"] = chams::teamnew;
			config["chams.local"] = chams::localnew;
			config["chams.legit_aa"] = chams::desync;
			config["chams.backtrack"] = chams::bttype;
			config["chams.backtrack_flat"] = chams::btflat;

			Option::Save(config["chams.backtrack_color"], chams::btcolor);
			Option::Save(config["chams.enemy_color_vis"], chams::EnemyColor_vis);
			Option::Save(config["chams.team_color_vis"], chams::TeamColor_vis);
			Option::Save(config["chams.local_color_vis"], chams::LocalColor_vis);
			Option::Save(config["chams.enemy_color_xqz"], chams::EnemyColor_XQZ);
			Option::Save(config["chams.team_color_xqz"], chams::TeamColor_XQZ);
			Option::Save(config["chams.local_color_xqz"], chams::LocalColor_XQZ);

			config["chams.dropped_weapons"] = chams::wep_droppedchams;
			//Option::Save(config["chams.dropped_weapons_color"], chams::clr_weapon_dropped_chams[3]);
			config["chams.plantedc4"] = chams::plantedc4_chams;
			//Option::Save(config["chams.plantedc4_color"], chams::clr_plantedc4_chams[3]);
			config["chams.nades"] = chams::nade_chams;
			//Option::Save(config["chams.nades_color"], chams::clr_nade_chams[3]);
			//Option::Save(config["chams.viewmodel_weapons_color"], chams::clr_weapon_chams[3]);


			Option::Save(config["chams.visible_color"], chams::visible_color);
			Option::Save(config["chams.occluded_color"], chams::occluded_color);

			config["chams.arms.enabled"] = chams::arms::enabled;
			config["chams.arms.wireframe"] = chams::arms::wireframe;

			Option::Save(config["chams.arms.color"], chams::arms::color);

			config["misc.bhop"] = misc::bhop;
			config["misc.auto_strafe"] = misc::auto_strafe;
			config["misc.viewmodel_fov"] = misc::viewmodel_fov;
			config["misc.moon_walk"] = misc::moon_walk;
			config["misc.deathmatch"] = misc::deathmatch;
			config["misc.no_flash"] = misc::no_flash;
			config["misc.no_smoke"] = misc::no_smoke;
			config["misc.knife_bot"] = misc::knife_bot;
			config["misc.radar"] = misc::radar;
			config["misc.human_bhop"] = misc::human_bhop;
			config["misc.human_bhop_hopchance"] = misc::bhop_hit_chance;
			config["misc.human_bhop_hopslimit"] = misc::hops_restricted_limit;
			config["misc.human_bhop_maxhops_hit"] = misc::max_hops_hit;
			config["misc.resolver"] = desync::resolver;
			config["misc.engine_prediction"] = movement::engine_prediction;
			config["misc.knife_bot"] = misc::knife_bot;
			config["misc.debug_fov"] = misc::debug_fov;
			config["misc.skychanger"] = visuals::skychanger;
			config["misc.skychanger_mode"] = visuals::skychanger_mode;
			config["misc.menu_style"] = visuals::newstyle;
			config["misc.rcs_cross"] = visuals::rcs_cross;
			config["misc.rcs_cross_mode"] = visuals::rcs_cross_mode;
			config["misc.bullet_tracer"] = visuals::bullet_tracer;
			//Option::Save(config["misc.bullet_tracer_color"], visuals::clr_bullet_tracer); //Here the bug happens
			config["misc.radius"] = visuals::radius;
			config["misc.no_scope_overlay"] = misc::noscope;
			config["misc.self_nade"] = misc::selfnade;
			config["misc.hitmarker"] = visuals::hitmarker;
			config["misc.hitmarker_sound"] = visuals::hitsound;
			config["misc.bomb_damage_esp"] = esp::bomb_esp;
			config["misc_kevlar_info"] = settings::esp::kevlarinfo;
			config["misc_spread_crosshair"] = settings::visuals::spread_cross;
			Option::Save(config["misc_spread_crosshair_color"], settings::visuals::spread_cross_color);
			config["misc_esp_money"] = settings::esp::money;
			config["misc_esp_kit"] = settings::esp::haskit;
			config["misc.choke_indicator"] = settings::visuals::choke;
			Option::Save(config["mics.rcs_cross_color"], visuals::recoilcolor);

			config["glow.enemy"] = glow::GlowEnemyEnabled;
			config["glow.team"] = glow::GlowTeamEnabled;
			config["glow.dropped_weapons"] = glow::GlowDroppedWeaponsEnabled;
			config["glow.plantedc4"] = glow::GlowC4PlantedEnabled;
			config["glow.nades"] = glow::GlowNadesEnabled;

			config["fake_lags.enabled"] = fake_lags::enabled;
			config["fake_lags.factor"] = fake_lags::factor;
			config["fake_lags.type"] = fake_lags::type;

			config["desync.enabled"] = desync::enabled;
			config["desync.yaw_flip"] = desync::yaw_flip;
			config["desync.resolver"] = desync::resolver;
			config["desync.enabled2"] = desync::enabled2;
			config["desync_mode"] = desync::desync_mode;

			config["legitbot.setting_type"] = aimbot::setting_type;

			Json::Value aimbot_items;
			for (auto data : aimbot::m_items)
			{
				Json::Value act;
				const auto aimbot_data = data.second;
				act["back_shot"] = aimbot_data.back_shot;
				act["autodelay"] = aimbot_data.autodelay;
				act["autopistol"] = aimbot_data.autopistol;
				act["autowall.enabled"] = aimbot_data.autowall.enabled;
				act["autowall.min_damage"] = aimbot_data.autowall.min_damage;
				act["backtrack.enabled"] = aimbot_data.backtrack.enabled;
				act["backtrack.legit"] = aimbot_data.backtrack.legit;
				act["backtrack.ticks"] = aimbot_data.backtrack.ticks;
				act["check_flash"] = aimbot_data.check_flash;
				act["check_smoke"] = aimbot_data.check_smoke;
				act["check_air"] = aimbot_data.check_air;
				act["enabled"] = aimbot_data.enabled;
				act["fov"] = aimbot_data.fov;
				act["dynamic_fov"] = aimbot_data.dynamic_fov;
				act["silent.enabled"] = aimbot_data.silent.enabled;
				act["silent.fov"] = aimbot_data.silent.fov;
				act["silent.always"] = aimbot_data.silent.always;
				act["silent.chanse"] = aimbot_data.silent.chanse;
				act["silent.with_smooth"] = aimbot_data.silent.with_smooth;
				act["by_damage"] = aimbot_data.by_damage;
				act["min_damage"] = aimbot_data.min_damage;
				act["hitchanse"] = aimbot_data.min_hitchanse;
				act["backtrack.time"] = aimbot_data.backtrack.time;

				act["trigger.enabled"] = aimbot_data.trigger.enabled;
				act["trigger.in_radius"] = aimbot_data.trigger.in_radius;
				act["trigger.delay"] = aimbot_data.trigger.delay;
				act["trigger.delay_btw_shots"] = aimbot_data.trigger.delay_btw_shots;
				act["trigger.hitchance"] = aimbot_data.trigger.hitchance;

				act["hitboxes.head"] = aimbot_data.hitboxes.head;
				act["hitboxes.neck"] = aimbot_data.hitboxes.neck;
				act["hitboxes.body"] = aimbot_data.hitboxes.body;
				act["hitboxes.hands"] = aimbot_data.hitboxes.hands;
				act["hitboxes.legs"] = aimbot_data.hitboxes.legs;

				act["kill_delay"] = aimbot_data.kill_delay;
				act["check_zoom"] = aimbot_data.check_zoom;
				act["shot_delay"] = aimbot_data.shot_delay;
				act["smooth"] = aimbot_data.smooth;
				act["min_damage"] = aimbot_data.min_damage;

				act["rcs"] = aimbot_data.recoil.enabled;
				act["rcs.humanize"] = aimbot_data.recoil.humanize;
				act["rcs.first_bullet"] = aimbot_data.recoil.first_bullet;
				act["rcs.standalone"] = aimbot_data.recoil.standalone;
				act["rcs_value_x"] = aimbot_data.recoil.pitch;
				act["rcs_value_y"] = aimbot_data.recoil.yaw;

				aimbot_items[data.first] = act;
			}

			config["legitbot.items"] = aimbot_items;

			Json::Value aimbot_groups;
			for (auto group : aimbot::m_groups)
			{
				Json::Value act;
				act["name"] = group.name;

				Json::Value weapons;
				for (auto& weapon : group.weapons)
					weapons.append(weapon);

				act["weapons"] = weapons;

				aimbot_groups.append(act);
			}

			config["aimbot.groups"] = aimbot_groups;

			return config;
		});
	}
}
