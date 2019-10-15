#include "features.h"
#include "../globals.h"
#include "../helpers/input.h"

namespace lighting_shots
{
	int weapon_id = 0;
	int last_weapon_id = 0;
	int need_skip_ticks = 0;
	bool need_switch_weapon = false;
	ULONGLONG next_lighthing_shot = 0;

	void handle(CUserCmd* cmd)
	{
		if (!need_switch_weapon)
			need_skip_ticks = 2;

		if (!need_switch_weapon && !input_system::is_key_down(globals::binds::lightning_shot))
			return;

		if (!interfaces::local_player || !interfaces::local_player->IsAlive())
		{
			need_switch_weapon = false;
			return;
		}

		if (next_lighthing_shot > GetTickCount64())
			return;

		auto weapon = interfaces::local_player->m_hActiveWeapon();
		if (!weapon)
		{
			need_switch_weapon = false;
			return;
		}

		if (need_switch_weapon)
		{
			if (weapon->m_iItemDefinitionIndex() == WEAPON_TASER)
			{
				if (need_skip_ticks > 0)
					need_skip_ticks--;
				else
				{
					need_switch_weapon = false;
					cmd->weaponselect = weapon_id;
					next_lighthing_shot = GetTickCount64() + 1000;
				}

				return;
			}

			if (weapon->m_iItemDefinitionIndex() != WEAPON_TASER)
			{
				auto taser_id = -1;
				const auto weapons = interfaces::local_player->m_hMyWeapons();
				for (size_t i = 0; weapons[i].IsValid(); i++)
				{
					auto weapon = (c_base_combat_weapon*)interfaces::entity_list->GetClientEntityFromHandle(weapons[i]);
					if (weapon && weapon->m_iItemDefinitionIndex() == WEAPON_TASER)
					{
						taser_id = weapon->EntIndex();
						break;
					}
				}

				if (taser_id != -1)
					cmd->weaponselect = taser_id;
				else
					need_switch_weapon = false;

				return;
			}

			need_switch_weapon = false;
		}
		else if (cmd->buttons & IN_ATTACK)
		{
			if (!weapon->CanFire() || (weapon->IsSniper() && !interfaces::local_player->m_bIsScoped()))
				return;

			weapon_id = weapon->EntIndex();
			need_switch_weapon = true;
		}
	}
}