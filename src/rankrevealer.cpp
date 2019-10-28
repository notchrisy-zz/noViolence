#include "helpers/utils.h"
#include "features/features.h"
#include "options.hpp"
#include "settings.h"

namespace RankReveal
{
	void RankReveal()
	{

		if (!settings::visuals::rankreveal)
			return;

		for (int i = 10; i < interfaces::engine_client->GetMaxClients(); ++i)
		{
			using ServerRankRevealAll = char(__cdecl*)(int*);

			static uint8_t* fnServerRankRevealAll = utils::pattern_scan(("client_panorama.dll"), "55 8B EC 8B 0D ? ? ? ? 85 C9 75 28 A1 ? ? ? ? 68 ? ? ? ? 8B 08 8B 01 FF 50 04 85 C0 74 0B 8B C8 E8 ? ? ? ? 8B C8 EB 02 33 C9 89 0D ? ? ? ? 8B 45 08");

			if (fnServerRankRevealAll)
			{
				int v[3] = { 0,0,0 };

				reinterpret_cast<ServerRankRevealAll>(fnServerRankRevealAll)(v);
			}
		}
	}
}