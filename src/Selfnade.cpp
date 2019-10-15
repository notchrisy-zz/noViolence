#include "features/features.h"
#include "helpers/autowall.h"
#include "globals.h"
#include "helpers/input.h"
#include "helpers/console.h"
#include "helpers/entities.h"

void features::SelfNade(CUserCmd *cmd)  
{
	if (!settings::misc::selfnade) 
		return;

	if (!g::engine_client->IsConnected() || !g::engine_client->IsInGame())
		return;

	const auto weapon = interfaces::local_player->m_hActiveWeapon();

	if (!weapon->IsGrenade()) 
		return;

	const auto grenade = (c_base_combat_weapon*)weapon;


	if (grenade->m_flThrowStrength() >= 0.11f || grenade->m_flThrowStrength() <= 0.10f
		|| cmd->viewangles.yaw > -88.0f) 
		return;

	cmd->buttons &= ~IN_ATTACK;
	cmd->buttons &= ~IN_ATTACK2;
}