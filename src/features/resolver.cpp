#include "features.h"
#include "../helpers/console.h"

namespace resolver
{
	void handle()
	{
		if (!settings::desync::resolver)
			return;

		for (int i = 1; i < interfaces::engine_client->GetMaxClients(); ++i)
		{
			auto player = c_base_player::GetPlayerByIndex(i);
			if (player && player->IsPlayer() && player != interfaces::local_player)
			{
				player->m_angEyeAngles().yaw = player->m_flLowerBodyYawTarget();
				player->m_angEyeAngles().NormalizeClamp();
			}
		}
	}
}