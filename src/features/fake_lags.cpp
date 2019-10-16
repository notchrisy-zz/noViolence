#include "features.h"
#include "../globals.h"
#include "../options.hpp"
#include "../helpers/entities.h"
#include "../helpers/console.h"
#include "../helpers/input.h"

namespace fake_lags
{
	struct target_t
	{
		float fov;
		Vector eye_pos;
		c_base_player* entity;

		bool operator<(const target_t& other) const
		{
			return fov < other.fov;
		}
	};

	const auto point_scale = 0.8f;
	matrix3x4_t bone_matrix[MAXSTUDIOBONES];

	int hitboxes[7] =
	{
		0, 1, 6, 10, 9, 13, 14
	};

	Vector local_points[27];
	std::vector<target_t> targets;

	Ray_t ray;
	CGameTrace tr;
	CTraceFilterWorldAndPropsOnly filter;

	bool has_target(const QAngle& angles, const bool& in_air)
	{
		if (interfaces::local_player->m_vecVelocity().Length2D() == 0.f && !in_air)
			return false;
	
		if (entities::m_items.empty())
			return false;

		bool has_any_enemies = false;
		for (const auto& player : entities::m_items.front().players)
		{
			if (player.index != 0)
			{
				has_any_enemies = true; 
				if (player.is_visible)
					return true;
			}
		}

		if (!has_any_enemies)
			return false;

		auto studio_model = interfaces::mdl_info->GetStudiomodel(interfaces::local_player->GetModel());
		if (!studio_model)
			return false;

		if (!interfaces::local_player->SetupBones(bone_matrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, 0.0f))
			return false;

		auto min = Vector(), max = Vector();
		const auto velocity = interfaces::local_player->m_vecVelocity() * interfaces::global_vars->interval_per_tick;

		Vector forward;
		math::angle2vectors(angles, forward);

		Vector right = forward.Cross(Vector(0, 0, 1));
		Vector left = Vector(-right.x, -right.y, right.z);

		Vector top = Vector(0, 0, 1.f);
		Vector bot = Vector(0, 0, -1.f);

		const auto eye_pos = interfaces::local_player->GetEyePos();
		
		int i = 0;
		for (const auto& index : hitboxes)
		{
			const auto hitbox = studio_model->GetHitboxSet(0)->GetHitbox(index);
			if (!hitbox)
				continue;

			const auto bone = bone_matrix[hitbox->bone];
			math::VectorTransform(hitbox->bbmax, bone, max);
			math::VectorTransform(hitbox->bbmin, bone, min);

			const auto center = (min + max) * 0.5f + velocity;
			if (index == HITBOX_HEAD)
			{
				local_points[i] = center + top * (hitbox->m_flRadius * point_scale);
				i++;
			}

			local_points[i] = center + right * (hitbox->m_flRadius * point_scale); i++;
			local_points[i] = center + left * (hitbox->m_flRadius * point_scale); i++;
		}

		targets.clear();

		QAngle aim_angles;
		c_base_player* entity;
		for (const auto& player : entities::m_items.front().players)
		{
			if (player.index == 0)
				continue; 

			entity = c_base_player::GetPlayerByIndex(player.index);
			if (entity && entity != interfaces::local_player)
			{
				const auto entity_pos = player.hitboxes[0][0];
				if (!entity_pos.IsValid())
					continue;

				math::vector2angles(entity_pos - eye_pos, aim_angles);
				aim_angles.NormalizeClamp();

				targets.push_back({ math::GetFovToPlayer(angles, aim_angles), entity_pos, entity });
			}
		}

		std::sort(targets.begin(), targets.end());

		for (const auto& item : targets)
		{
			for (const auto& point : local_points)
			{
				if (!point.IsValid())
					continue;

				ray.Init(item.eye_pos, point);
				interfaces::engine_trace->trace_ray(ray, CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTER, &filter, &tr);

				if (item.eye_pos.DistTo(point) == item.eye_pos.DistTo(tr.endpos))
					return true;
			}
		}

		return false;
	}

	bool is_enabled(CUserCmd* cmd)
	{
		if (!settings::fake_lags::enabled || settings::fake_lags::factor == 0)
			return false;

		if (!interfaces::local_player || !interfaces::local_player->IsAlive())
			return false;

		if (!utils::can_lag() || cmd->buttons & IN_USE)
			return false;

		auto weapon = interfaces::local_player->m_hActiveWeapon();
		if (weapon)
		{
			if (cmd->buttons & IN_ATTACK)
			{
				if (weapon->IsKnife())
					return false;

				if (weapon->CanFire())
					return false;
			}

			if (cmd->buttons & IN_ATTACK2 && weapon->IsSniper())
				return false;
		}

		const auto was_pressed = globals::binds::fake_lag != 0 && input_system::is_key_down(globals::binds::fake_lag);
		if (was_pressed)
			return true;

		if (settings::fake_lags::type == fake_lag_types::lag_by_button && was_pressed)
			return true;

		if (settings::fake_lags::type == fake_lag_types::lag_only_in_air && !(interfaces::local_player->m_fFlags() & FL_ONGROUND))
			return true;

		if (settings::fake_lags::type == fake_lag_types::lag_when_pick)
			return has_target(cmd->viewangles, !(interfaces::local_player->m_fFlags() & FL_ONGROUND));
		
		return settings::fake_lags::type == fake_lag_types::lag_always;
	}

	void handle(CUserCmd* cmd, bool& send_packet)
	{
		if (!is_enabled(cmd))
			return;

#ifdef _DEBUG
		//console::print("=== fake lags ===");
		//console::print("Length %.2f", interfaces::local_player->m_vecVelocity().Length());
		//console::print("Length2D %.2f", interfaces::local_player->m_vecVelocity().Length2D());
		//console::print("m_flSimulationTime %.2f", interfaces::local_player->m_flSimulationTime());
#endif

		const auto chocked = *reinterpret_cast<int*>(uintptr_t(interfaces::client_state->m_NetChannel) + 0x002C);
		if (chocked > settings::fake_lags::factor)
			return;

		send_packet = false;
	}
}