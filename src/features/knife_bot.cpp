#include "features.h"
#include "../helpers/console.h"
#include "../helpers/entities.h"

namespace knife_bot
{
	const auto stab_dist = 32.f;
	const auto slash_dist = 48.f;

	void handle(CUserCmd* cmd, bool& send_packet)
	{
		if (!interfaces::local_player || !interfaces::local_player->IsAlive() || !settings::misc::knife_bot)
			return;

		if (cmd->buttons & IN_ATTACK || cmd->buttons & IN_ATTACK2)
			return;

		auto weapon = interfaces::local_player->m_hActiveWeapon();
		if (!weapon || !weapon->IsKnife())
			return;

		Ray_t ray;
		QAngle angles;
		CGameTrace tr;

		static const auto sv_maxunlag = interfaces::cvar->find("sv_maxunlag");

		auto nci = interfaces::engine_client->GetNetChannelInfo();
		if (!nci)
			return;

		const auto unlag = sv_maxunlag->GetFloat();
		const auto interpolation_comp = utils::get_interpolation_compensation();
		const auto correct_curtime = std::clamp(nci->GetLatency(FLOW_OUTGOING) + nci->GetLatency(FLOW_INCOMING) + interpolation_comp, 0.f, unlag);

		const auto eye_pos = interfaces::local_player->GetEyePos();
		for (auto& tick : entities::m_items)
		{
			for (auto& player : tick.players)
			{
				if (player.index == 0)
					continue;

				const auto is_valid = fabsf(correct_curtime - (interfaces::global_vars->curtime - player.m_flSimulationTime)) <= 0.2f;
				if (!is_valid)
					continue;

				auto entity = c_base_player::GetPlayerByIndex(player.index);
				if (!entity || !entity->IsAlive() || entity == interfaces::local_player)
					continue;

				CTraceFilterSkipTwoEntities filter = CTraceFilterSkipTwoEntities(interfaces::local_player, entity);

				for (size_t h = 0; h < sizeof(player.hitboxes); h++)
				{
					if (h == HITBOX_LEFT_FOOT || h == HITBOX_RIGHT_FOOT)
						continue;

					const auto hitbox = player.hitboxes[h][0];
					if (!hitbox.IsValid())
						continue;

					const auto distance = eye_pos.DistTo(hitbox);
					if (distance > slash_dist)
						continue;

					ray.Init(eye_pos, hitbox);
					interfaces::engine_trace->trace_ray(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr);
					if (eye_pos.DistTo(tr.endpos) != distance)
						continue;

					math::vector2angles(hitbox - eye_pos, angles);
					angles.NormalizeClamp();

					const auto old_angles = cmd->viewangles;
					cmd->buttons |= distance <= stab_dist ? IN_ATTACK2 : IN_ATTACK;
					cmd->viewangles = angles;
					cmd->tick_count = TIME_TO_TICKS(player.m_flSimulationTime + interpolation_comp);

					math::correct_movement(cmd, old_angles);

					send_packet = false;

					return;
				}
			}
		}
	}
}