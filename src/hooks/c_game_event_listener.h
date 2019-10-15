#pragma once
#include "../Sounds.h"
#include "..//HitPossitionHelper.h"
#include "..//runtime_saver.h"
#include "..//esp.hpp"
#include "../valve_sdk/interfaces/IVEngineClient.hpp"

#pragma comment(lib, "Winmm.lib") //PlaySoundA fix


ConVar* game_type = nullptr;
ConVar* game_mode = nullptr;

class c_game_event_listener final : public IGameEventListener2
{
	const char* hitgroups[10] =
	{
		"generic", "head", "chest", "stomach", "arm", "arm", "leg", "leg", "gear"
	};

	void FireGameEvent(IGameEvent* context) override
	{
		const auto name = fnv::hash_runtime(context->GetName());

		if (name == FNV("game_newmap"))
		{
			no_smoke::event();
			color_modulation::event();
			globals::team_damage.clear();
		}
		else if (name == FNV("player_hurt"))
		{
			auto attacker = c_base_player::GetPlayerByUserId(context->GetInt("attacker"));
			auto target = c_base_player::GetPlayerByUserId(context->GetInt("userid"));
			if (!attacker || !target)
				return;

			HitPossitionHelper::Get().OnFireEvent(context);

			if (attacker->m_iTeamNum() == target->m_iTeamNum())
				globals::team_damage[context->GetInt("attacker")] += context->GetInt("dmg_health");

			if (attacker == interfaces::local_player && target != interfaces::local_player)
			{
				char buf[256];
				sprintf_s(buf, "%s -%d (%d hp left)",target->GetPlayerInfo().szName, context->GetInt("dmg_health"), context->GetInt("health"));

				notifies::push(buf, notify_state_s::debug_state);

					/*if (!game_mode)
						game_mode = interfaces::cvar->find("game_mode");

					if (!game_type)
						game_type = interfaces::cvar->find("game_type");

					if (game_type->GetInt() == 0 && game_mode->GetInt() == 0) //casual
						return;

					if (game_type->GetInt() == 1 && game_mode->GetInt() == 1) //demolition
						return;

					if (game_type->GetInt() == 1 && game_mode->GetInt() == 0) //arms race
						return;

					if (game_type->GetInt() == 1 && game_mode->GetInt() == 2) //deathmatch
						return;  */
			}

			if (settings::misc::killsay)
			{
				if ("dmg_health" == 0)
					context->ClientCmd("say Your shit, try using noViolence");
			}

			if (settings::visuals::hitmarker)
			{

				int _attacker = context->GetInt("attacker");
				if (g::engine_client->GetPlayerForUserID(_attacker) == g::engine_client->GetLocalPlayer())
				{
					using namespace Sounds;
					saver.HitmarkerInfo = HitmarkerInfoStruct{ g::global_vars->realtime, 0.f };
					switch (settings::visuals::hitsound)
					{
					case 0:
						//PlaySoundA((g_Config.AppdataFolder + "hitsound0.wav").data(), NULL, SND_ASYNC | SND_NODEFAULT | SND_NOSTOP);
						PlaySoundA(cod, NULL, SND_ASYNC | SND_MEMORY); //cod sound
						break;
					case 1:
						//PlaySoundA((g_Config.AppdataFolder + "hitsound1.wav").data(), NULL, SND_ASYNC | SND_NODEFAULT | SND_NOSTOP);
						PlaySoundA(skeet, NULL, SND_ASYNC | SND_MEMORY); //skeet sound
						break;
					case 2:
						PlaySoundA(punch, NULL, SND_ASYNC | SND_MEMORY); //punch sound
						break;
					case 3:
						PlaySoundA(metal, NULL, SND_ASYNC | SND_MEMORY); //metal sound
						break;
					case 4:
						PlaySoundA(boom, NULL, SND_ASYNC | SND_MEMORY); //boom sound
						break;
					}
				}
			}
		}
		else if (name == FNV("bullet_impact"))
		{
			HitPossitionHelper hp;

			c_base_player* shooter = static_cast<c_base_player*>(g::entity_list->GetClientEntity(g::engine_client->GetPlayerForUserID(context->GetInt("userid"))));

			if (!shooter || shooter != g::local_player)
				return;

			Vector p = Vector(context->GetFloat("x"), context->GetFloat("y"), context->GetFloat("z"));
			hp.ShotTracer(saver.LastShotEyePos, p);
		}
		else if (name == FNV("item_purchase"))
		{
			auto enemy = c_base_player::GetPlayerByUserId(context->GetInt("userid"));
			if (!enemy || !interfaces::local_player || enemy->m_iTeamNum() == interfaces::local_player->m_iTeamNum())
				return;

			if (!game_mode)
				game_mode = interfaces::cvar->find("game_mode");

			if (!game_type)
				game_type = interfaces::cvar->find("game_type");

			if (game_type->GetInt() == 0 && game_mode->GetInt() == 0) //casual
				return;

			if (game_type->GetInt() == 1 && game_mode->GetInt() == 1) //demolition
				return;

			if (game_type->GetInt() == 1 && game_mode->GetInt() == 0) //arms race
				return;

			if (game_type->GetInt() == 1 && game_mode->GetInt() == 2) //deathmatch
				return;

			char buf[256];

			sprintf_s(buf, "%s - %s", enemy->GetPlayerInfo().szName, context->GetString("weapon"));

			notifies::push(std::string(buf), notify_state_s::debug_state);
		}
		else if (name == FNV("round_start") && settings::misc::esp_random)
		{
			int chance;
		
			chance = rand() % 100 + 1;

			char number[256];

			sprintf_s(number, "Chance: %i%%", chance);

			notifies::push(number, notify_state_s::debug_state);

			if (chance >= settings::esp::esp_on_chance)
			{
				settings::esp::visible_only = false;

				/* CHAMS MODES: */
				settings::chams::enemymodenew = 0; //normal
			}
			else if (chance <= settings::esp::esp_off_chance)
			{
				settings::esp::visible_only = true;

				/* CHAMS MODES: */
				settings::chams::enemymodenew = 5; //XQZ
			}
		}
		else if (name == FNV("cs_pre_restart") || name == FNV("switch_team") || name == FNV("announce_phase_end") || name == FNV("round_freeze_end"))
			clantag::restore();
		else if (name == FNV("bullet_impact") && settings::esp::beams)
			utils::create_beam(context->GetInt("userid"), Vector(context->GetFloat("x"), context->GetFloat("y"), context->GetFloat("z")));
	}

	int GetEventDebugID(void) override
	{
		return EVENT_DEBUG_ID_INIT;
	}
};