#include "hooks.h"
#include "../globals.h"
#include "../settings.h"
#include "../helpers/console.h"
#include "../helpers/entities.h"

namespace hooks
{
	vfunc_hook fire_bullets::hook;

	void __stdcall fire_bullets::hooked(int type)
	{
		static const auto original = hook.get_original<fn>(index);

		original(interfaces::fire_bullets, type);

		const auto index = interfaces::fire_bullets->m_iPlayer + 1;
		auto* entity = c_base_player::GetPlayerByIndex(index);
		if (!entity || !entity->IsPlayer())
			return;

		for (auto& tick : entities::m_items)
		{
			for (auto& player : tick.players)
			{
				if (index == player.index)
				{
					player.is_shooting = true;
					player.m_flShotTime = entity->m_flSimulationTime();
					player.shot_origin = interfaces::fire_bullets->m_vecOrigin;

					return;
				}
			}
		}
	}
}