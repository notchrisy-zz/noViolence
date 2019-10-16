#pragma once

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

			if (attacker->m_iTeamNum() == target->m_iTeamNum())
				globals::team_damage[context->GetInt("attacker")] += context->GetInt("dmg_health");

			if (attacker == interfaces::local_player && target != interfaces::local_player)
			{
				char buf[256];
				sprintf_s(
					buf, "Hit %s in the %s for %d damage (%d health remaining)", 
					target->GetPlayerInfo().szName, hitgroups[context->GetInt("hitgroup")], 
					context->GetInt("dmg_health"), context->GetInt("health")
				);

				notifies::push(buf, notify_state_s::debug_state);
			}
		}
		else if (name == FNV("item_purchase"))
		{
			auto enemy = c_base_player::GetPlayerByUserId(context->GetInt("userid"));
			if (!enemy || !interfaces::local_player || enemy->m_iTeamNum() == interfaces::local_player->m_iTeamNum())
				return;

			char buf[256];
			sprintf_s(buf, "%s bought %s", enemy->GetPlayerInfo().szName, context->GetString("weapon"));

			notifies::push(std::string(buf), notify_state_s::debug_state);
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