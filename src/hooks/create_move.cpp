#include "hooks.h"
#include "../globals.h"
#include "../options.hpp"
#include "../helpers/input.h"
#include "../helpers/console.h"
#include "../helpers/entities.h"
#include "../features/features.h"
#include "..//runtime_saver.h"
#include "..//Backtrack_new.h"
#pragma intrinsic(_ReturnAddress)  

float side = 1.0f;

float real_angle = 0.0f;
float view_angle = 0.0f;

static CCSGOPlayerAnimState g_AnimState;
static int max_choke_ticks = 14;

float AngleDiff(float destAngle, float srcAngle) {
	float delta;

	delta = fmodf(destAngle - srcAngle, 360.0f);
	if (destAngle > srcAngle) {
		if (delta >= 180)
			delta -= 360;
	}
	else {
		if (delta <= -180)
			delta += 360;
	}
	return delta;
}

namespace hooks
{
	bool __stdcall client_mode::create_move_shared::hooked(float input_sample_frametime, CUserCmd* cmd)
	{

		netchannel::setup();

		static auto original = client_mode::hook.get_original<fn>(index);

		if (!cmd || !cmd->command_number)
			return original(interfaces::client_mode, input_sample_frametime, cmd);

		const auto ebp = reinterpret_cast<uintptr_t*>(uintptr_t(_AddressOfReturnAddress()) - sizeof(void*));
		auto& send_packet = *reinterpret_cast<bool*>(*ebp - 0x1C);

		bool* sendpacket2 = reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(_AddressOfReturnAddress()) + 0x14);

		engine_prediction::start(cmd);

		visuals::fetch_entities();
		entities::fetch_targets(cmd);

		static int latency_ticks = 0;
		float fl_latency = g::engine_client->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING);
		int latency = TIME_TO_TICKS(fl_latency);
		if (g::client_state->chokedcommands <= 0) {
			latency_ticks = latency;
		}
		else {
			latency_ticks = std::max(latency, latency_ticks);
		}

		if (g::game_rules_proxy->m_bIsValveDS()) {
			if (fl_latency >= g::global_vars->interval_per_tick)
				max_choke_ticks = 11 - latency_ticks;
			else
				max_choke_ticks = 11;
		}
		else {
			max_choke_ticks = 13 - latency_ticks;
		}

		static float SpawnTime = 0.0f;
		if (g::local_player->m_flSpawnTime() != SpawnTime) {
			g_AnimState.pBaseEntity = g::local_player;
			g::local_player->ResetAnimationState(&g_AnimState);
			SpawnTime = g::local_player->m_flSpawnTime();
		}

		static float next_lby = 0.0f;

		QAngle OldAngles = cmd->viewangles;

		auto Desync = [OldAngles](CUserCmd* cmd, bool* send_packet)
		{
			if (cmd->buttons & (IN_ATTACK | IN_ATTACK2 | IN_USE) ||
				g::local_player->m_nMoveType() == MOVETYPE_LADDER || g::local_player->m_nMoveType() == MOVETYPE_NOCLIP
				|| !g::local_player->IsAlive())
				return;

			auto* channel_info = interfaces::engine_client->GetNetChannelInfo();
			if (channel_info && (channel_info->GetAvgLoss(1) > 0.f || channel_info->GetAvgLoss(0) > 0.f))
				return;

			if (interfaces::local_player->m_bGunGameImmunity() || interfaces::local_player->m_fFlags() & FL_FROZEN)
				return;


			auto weapon = g::local_player->m_hActiveWeapon().Get();
			if (!weapon)
				return;

			auto weapon_index = weapon->m_iItemDefinitionIndex();
			if ((weapon_index == WEAPON_GLOCK || weapon_index == WEAPON_FAMAS) && weapon->m_flNextPrimaryAttack() >= g::global_vars->curtime)
				return;

			auto weapon_data = weapon->get_weapon_data();

			if (weapon_data->WeaponType == WEAPONTYPE_GRENADE) {
				if (!weapon->m_bPinPulled()) {
					float throwTime = weapon->m_fThrowTime();
					if (throwTime > 0.f)
						return;
				}

				if ((cmd->buttons & IN_ATTACK) || (cmd->buttons & IN_ATTACK2)) {
					if (weapon->m_fThrowTime() > 0.f)
						return;
				}
			}

			static bool broke_lby = false;

			/*if (globals::binds::desync) {
				side = -side;
			} */

			if (settings::desync::desync_mode == 1) {

				/*
				if( do_fakeduck )
					return;
				*/

				float minimal_move = 2.0f;
				if (g::local_player->m_fFlags() & FL_DUCKING)
					minimal_move *= 3.f;

				if (cmd->buttons & IN_WALK)
					minimal_move *= 3.f;

				bool should_move = g::local_player->m_vecVelocity().Length2D() <= 0.0f
					|| std::fabsf(g::local_player->m_vecVelocity().z) <= 100.0f;

				if ((cmd->command_number % 2) == 1) {
					cmd->viewangles.yaw += 120.0f * side; //was 120.0f
					if (should_move)
						cmd->sidemove -= minimal_move;
					*send_packet = false;
				}
				else if (should_move) {
					cmd->sidemove += minimal_move;
				}
			}
			else {
				if (next_lby >= g::global_vars->curtime) {
					if (!broke_lby && *send_packet && g::client_state->chokedcommands > 0)
						return;

					broke_lby = false;
					*send_packet = false;
					cmd->viewangles.yaw += 120.0f * side; //was 120.f
				}
				else {
					broke_lby = true;
					*send_packet = false;
					cmd->viewangles.yaw += 120.0f * -side; //was 120.f
				}
			}

			math::FixAngles(cmd->viewangles);
			math::MovementFix(cmd, OldAngles, cmd->viewangles);
		};

		if (settings::visuals::grenade_prediction)
			grenade_prediction::fetch_points(cmd);

		if (cmd->weaponselect == 0 && !lighting_shots::need_switch_weapon)
		{
			aimbot::handle(cmd);
			zeusbot::handle(cmd);
			knife_bot::handle(cmd, *sendpacket2);
		}

		static int definition_index = 7;
		auto a_settings = &settings::aimbot::m_items[definition_index];

		//g_Backtrack.OnMove(cmd); //New backtrack - CAUSES FPS DROPS

		if (a_settings->recoil.enabled)
			aimbot::OnMove(cmd);

		if (g::local_player && g::local_player->IsAlive() && (cmd->buttons & IN_ATTACK || cmd->buttons & IN_ATTACK2))
			saver.LastShotEyePos = g::local_player->GetEyePos();

		if (settings::lightning_shot::enabled)
			lighting_shots::handle(cmd);

		if (settings::misc::bhop)
			features::bhop(cmd);

		if (settings::misc::human_bhop)
			features::human_bhop(cmd);

		if (settings::misc::auto_strafe)
			features::auto_strafe(cmd);

		if (settings::misc::moon_walk)
			features::moon_walk(cmd);

		if (settings::misc::selfnade)
			features::SelfNade(cmd);

		if (settings::desync::enabled2 && std::fabsf(g::local_player->m_flSpawnTime() - g::global_vars->curtime) > 1.0f)
			Desync(cmd, sendpacket2);

		math::FixAngles(cmd->viewangles);
		cmd->viewangles.yaw = std::remainderf(cmd->viewangles.yaw, 360.0f);

		if (settings::desync::enabled2 && g::client_state->chokedcommands >= max_choke_ticks) {
			*sendpacket2 = true;
			cmd->viewangles = g::client_state->viewangles;
		}

		slow_walk::handle(cmd);
		fake_lags::handle(cmd, *sendpacket2);
		fake_duck::handle(cmd, *sendpacket2);
		//desync::handle(cmd, send_packet);

		static ConVar* m_yaw = m_yaw = g::cvar->find("m_yaw");
		static ConVar* m_pitch = m_pitch = g::cvar->find("m_pitch");
		static ConVar* sensitivity = sensitivity = g::cvar->find("sensitivity");

		static QAngle m_angOldViewangles = g::client_state->viewangles;

		float delta_x = std::remainderf(cmd->viewangles.pitch - m_angOldViewangles.pitch, 360.0f);
		float delta_y = std::remainderf(cmd->viewangles.yaw - m_angOldViewangles.yaw, 360.0f);

		if (delta_x != 0.0f) {
			float mouse_y = -((delta_x / m_pitch->GetFloat()) / sensitivity->GetFloat());
			short mousedy;
			if (mouse_y <= 32767.0f) {
				if (mouse_y >= -32768.0f) {
					if (mouse_y >= 1.0f || mouse_y < 0.0f) {
						if (mouse_y <= -1.0f || mouse_y > 0.0f)
							mousedy = static_cast<short>(mouse_y);
						else
							mousedy = -1;
					}
					else {
						mousedy = 1;
					}
				}
				else {
					mousedy = 0x8000u;
				}
			}
			else {
				mousedy = 0x7FFF;
			}

			cmd->mousedy = mousedy;
		}

		if (delta_y != 0.0f) {
			float mouse_x = -((delta_y / m_yaw->GetFloat()) / sensitivity->GetFloat());
			short mousedx;
			if (mouse_x <= 32767.0f) {
				if (mouse_x >= -32768.0f) {
					if (mouse_x >= 1.0f || mouse_x < 0.0f) {
						if (mouse_x <= -1.0f || mouse_x > 0.0f)
							mousedx = static_cast<short>(mouse_x);
						else
							mousedx = -1;
					}
					else {
						mousedx = 1;
					}
				}
				else {
					mousedx = 0x8000u;
				}
			}
			else {
				mousedx = 0x7FFF;
			}

			cmd->mousedx = mousedx;
		}

		auto anim_state = g::local_player->GetPlayerAnimState2();
		if (anim_state) {
			CCSGOPlayerAnimState anim_state_backup = *anim_state;
			*anim_state = g_AnimState;
			*g::local_player->GetVAngles2() = cmd->viewangles;
			g::local_player->UpdateClientSideAnimation();

			if (anim_state->speed_2d > 0.1f || std::fabsf(anim_state->flUpVelocity)) {
				next_lby = g::global_vars->curtime + 0.22f;
			}
			else if (g::global_vars->curtime > next_lby) {
				if (std::fabsf(AngleDiff(anim_state->m_flGoalFeetYaw, anim_state->m_flEyeYaw)) > 35.0f) {
					next_lby = g::global_vars->curtime + 1.1f;
				}
			}

			g_AnimState = *anim_state;
			*anim_state = anim_state_backup;
		}

		if (*sendpacket2) {
			real_angle = g_AnimState.m_flGoalFeetYaw;
			view_angle = g_AnimState.m_flEyeYaw;
		}

		engine_prediction::finish(cmd);

		//if (cmd->buttons & IN_SCORE)
			//hooks::dispatch_user_message::hook.CallOriginal(std::forward<IBaseClientDLL*>(interfaces::base_client), 50, 0, 0, nullptr); // 50 = CS_UM_ServerRankRevealAll

		if (!(cmd->buttons & IN_BULLRUSH))
			cmd->buttons |= IN_BULLRUSH;

		cmd->viewangles.NormalizeClamp();

		return false;
	}
}