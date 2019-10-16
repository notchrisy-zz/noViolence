#include "features.h"
#include "../globals.h"
#include "../helpers/input.h"
#include "../helpers/console.h"

namespace desync
{
	float yaw_offset;

	bool flip_yaw = false;
	bool flip_packet = false;

	CCSPlayerAnimState* anim_state;

	bool is_enabled(CUserCmd* cmd)
	{
		if (!interfaces::local_player || !interfaces::local_player->IsAlive())
			return false;

		if (!settings::desync::enabled || (cmd->buttons & IN_USE))
			return false;

		auto* channel_info = interfaces::engine_client->GetNetChannelInfo();
		if (channel_info && (channel_info->GetAvgLoss(1) > 0.f || channel_info->GetAvgLoss(0) > 0.f))
			return false;

		if (interfaces::local_player->m_bGunGameImmunity() || interfaces::local_player->m_fFlags() & FL_FROZEN)
			return false;

		if (interfaces::local_player->m_nMoveType() == MOVETYPE_LADDER || interfaces::local_player->m_nMoveType() == MOVETYPE_NOCLIP)
			return false;

		return true;
	}

	bool is_firing(CUserCmd* cmd)
	{
		auto weapon = interfaces::local_player->m_hActiveWeapon();
		if (!weapon)
			return false;

		const auto weapon_type = weapon->get_weapon_data()->WeaponType;
		if (weapon_type == WEAPONTYPE_GRENADE)
		{
			return true;

			//if (!weapon->m_bPinPulled() && (cmd->buttons & IN_ATTACK || cmd->buttons & IN_ATTACK2))
				//return false;
		}
		else if (weapon_type == WEAPONTYPE_KNIFE)
		{
			if (cmd->buttons & IN_ATTACK || cmd->buttons & IN_ATTACK2)
				return true;
		}
		else if (weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER)
		{
			if (cmd->buttons & IN_ATTACK && weapon->CanFire())
				return true;

			if (cmd->buttons & IN_ATTACK2)
				return true;
		}
		else if (cmd->buttons & IN_ATTACK && weapon->CanFire() && weapon_type != WEAPONTYPE_C4)
			return true;

		return false;
	}

	float get_max_desync_delta()
	{
		const auto speed_factor = std::max<float>(0.f, std::min<float>(1, anim_state->m_flSpeedFactor));
		const auto speed_fraction = std::max<float>(0.f, std::min<float>(anim_state->m_flSpeedFraction, 1.f));

		const auto unk1 = (anim_state->m_flLandingRatio * -0.30000001 - 0.19999999) * speed_fraction;
		float unk2 = unk1 + 1.f;

		if (anim_state->m_flDuckAmount > 0)
			unk2 += anim_state->m_flDuckAmount * speed_factor * (0.5f - unk2);

		return anim_state->m_flMaxBodyYawDegrees * unk2;
	}

	void handle(CUserCmd* cmd, bool& send_packet)
	{
		if (!send_packet)
			return;

		if (!is_enabled(cmd))
		{
			yaw_offset = 0.f;

			return;
		}

		anim_state = interfaces::local_player->GetPlayerAnimState();

		assert(anim_state);

		yaw_offset = get_max_desync_delta();

		const auto old_angles = cmd->viewangles;

		flip_packet = !flip_packet;
		send_packet = flip_packet;
		if (!flip_packet)
		{
			if (settings::desync::yaw_flip)
				flip_yaw = !flip_yaw;
		}

#ifdef _DEBUG
		//console::print("=== desync ===");
		//console::print("anim state: %p", anim_state);
		//console::print("max desync delta: %.2f", yaw_offset);
#endif

		static float last_lby = 0.f;
		static float last_update = 0.f;

		const auto current_lby = interfaces::local_player->m_flLowerBodyYawTarget();
		const float current_time = interfaces::local_player->m_nTickBase() * interfaces::global_vars->interval_per_tick;

		const float delta = ceilf((current_time - last_update) * 100) / 100;
		const auto next_delta = ceilf((delta + interfaces::global_vars->interval_per_tick) * 100) / 100;

		if (interfaces::local_player->m_vecVelocity().Length2D() <= 0.f)
		{
			if (current_lby != 180.f && last_lby != current_lby)
			{
				//console::print("lby updated after %.4f", delta);

				last_lby = current_lby;
				last_update = current_time - interfaces::global_vars->interval_per_tick;
			} 
			else if (next_delta >= 1.1f)
			{
				//console::print("curr: %.4f; next: %.4f", delta, next_delta);

				send_packet = flip_packet = true;

				last_update = current_time;
			} 
		}
		else
		{
			last_lby = current_lby;
			last_update = current_time;
		}
		
		const auto low_fps = interfaces::global_vars->interval_per_tick * 0.9f < interfaces::global_vars->absoluteframetime;
		if (low_fps || is_firing(cmd))
			send_packet = flip_packet = true;

		if (send_packet)
			anim_state->m_flGoalFeetYaw += flip_yaw ? yaw_offset : -yaw_offset;
		else
		{
			cmd->buttons &= ~(IN_FORWARD | IN_BACK | IN_MOVERIGHT | IN_MOVELEFT);

			cmd->viewangles.yaw += 180.f;
			cmd->viewangles.NormalizeClamp();
			math::correct_movement(cmd, old_angles);
		}
	}
}