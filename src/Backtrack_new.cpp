#include "backtrack_new.h"
#include "features/features.h"
#include "helpers/math.h"
#include "settings.h"
#include "Backtrack_new.h"

void Backtrack::OnMove(CUserCmd* pCmd) {
	if (!g::engine_client->IsInGame() || !g::local_player || !g::local_player->IsAlive()) {
		data.clear();
		return;
	}

	auto pWeapon = g::local_player->m_hActiveWeapon();
	if (!pWeapon || !pWeapon->IsGun()) {
		data.clear();
		return;
	}

	auto weaponData = pWeapon->get_weapon_data();
	auto a_settings = settings::aimbot::m_items[pWeapon->m_iItemDefinitionIndex()];
	if (!a_settings.enabled) {
		data.clear();
		return;
	}

	static ConVar* sv_maxunlag = g::cvar->find("sv_maxunlag");
	static ConVar* sv_minupdaterate = g::cvar->find("sv_minupdaterate");
	static ConVar* sv_maxupdaterate = g::cvar->find("sv_maxupdaterate");

	static ConVar* sv_client_min_interp_ratio = g::cvar->find("sv_client_min_interp_ratio");
	static ConVar* sv_client_max_interp_ratio = g::cvar->find("sv_client_max_interp_ratio");

	static ConVar* cl_interp_ratio = g::cvar->find("cl_interp_ratio");
	static ConVar* cl_interp = g::cvar->find("cl_interp");
	static ConVar* cl_updaterate = g::cvar->find("cl_updaterate");

	float updaterate = cl_updaterate->GetFloat();

	float minupdaterate = sv_minupdaterate->GetFloat();
	float maxupdaterate = sv_maxupdaterate->GetFloat();

	float min_interp = sv_client_min_interp_ratio->GetFloat();
	float max_interp = sv_client_max_interp_ratio->GetFloat();

	float flLerpAmount = cl_interp->GetFloat();
	float flLerpRatio = cl_interp_ratio->GetFloat();
	flLerpRatio = std::clamp(flLerpRatio, min_interp, max_interp);
	if (flLerpRatio == 0.0f)
		flLerpRatio = 1.0f;

	float updateRate = std::clamp(updaterate, minupdaterate, maxupdaterate);
	lerp_time = std::fmaxf(flLerpAmount, flLerpRatio / updateRate);
	latency = g::engine_client->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING) + g::engine_client->GetNetChannelInfo()->GetLatency(FLOW_INCOMING);
	correct_time = latency + lerp_time;

	// setup records
	c_base_player* player;
	for (int i = 1; i <= g::global_vars->maxClients; ++i) {
		player = c_base_player::GetPlayerByIndex(i);
		if (player->IsNotTarget()) {
			if (data.count(i) > 0)
				data.erase(i);
			continue;
		}

		if (!settings::misc::deathmatch && player->m_iTeamNum() == g::local_player->m_iTeamNum()) {
			if (data.count(i) > 0)
				data.erase(i);
			continue;
		}

		auto& cur_data = data[i];
		if (!cur_data.empty()) {
			auto& front = cur_data.front();
			if (front.simTime == player->m_flSimulationTime())
				continue;

			while (!cur_data.empty()) {
				auto& back = cur_data.back();
				float deltaTime = correct_time - (g::global_vars->curtime - back.simTime);
				if (std::fabsf(deltaTime) <= 0.2f)
					break;

				cur_data.pop_back();
			}
		}

		auto model = player->GetModel();
		if (!model)
			continue;

		auto hdr = g::mdl_info->GetStudiomodel(model);
		if (!hdr)
			continue;

		auto hitbox_set = hdr->GetHitboxSet(player->m_nHitboxSet());
		auto hitbox_head = hitbox_set->GetHitbox(HITBOX_HEAD);
		auto hitbox_center = (hitbox_head->bbmin + hitbox_head->bbmax) * 0.5f;

		backtrack_data bd;
		bd.simTime = player->m_flSimulationTime();
		bd.origin = player->m_vecOrigin();
		bd.angle = player->GetAbsAngles();

		*(Vector*)((uintptr_t)player + 0xA0) = player->m_vecOrigin();
		*(int*)((uintptr_t)player + 0xA68) = 0;
		*(int*)((uintptr_t)player + 0xA30) = 0;
		player->InvalidateBoneCache();
		player->SetupBones(bd.boneMatrix, 128, BONE_USED_BY_ANYTHING, g::global_vars->curtime);

		math::VectorTransform(hitbox_center, bd.boneMatrix[hitbox_head->bone], bd.hitboxPos);

		data[i].push_front(bd);
	}

	Vector localEyePos = g::local_player->GetEyePos();
	QAngle angles;
	int tick_count = -1;
	float best_fov = 180.0f;
	for (auto& node : data) {
		auto& cur_data = node.second;
		if (cur_data.empty())
			continue;

		for (auto& bd : cur_data) {
			float deltaTime = correct_time - (g::global_vars->curtime - bd.simTime);
			if (std::fabsf(deltaTime) > a_settings.backtrack.time)
				continue;

			math::vector2angles(bd.hitboxPos - localEyePos, angles);
			math::FixAngles(angles);
			float fov = math::GetFovToPlayer(pCmd->viewangles, angles);
			if (best_fov > fov) {
				best_fov = fov;
				tick_count = TIME_TO_TICKS(bd.simTime + lerp_time);
			}
		}
	}

	if (tick_count != -1) {
		pCmd->tick_count = tick_count;
	}
}

Backtrack g_Backtrack;
