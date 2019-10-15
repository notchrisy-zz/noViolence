#include "features.h"
#include "../options.hpp"

namespace no_flash
{
	void handle()
	{
		if (settings::misc::no_flash && interfaces::local_player)
			interfaces::local_player->m_flFlashDuration() = 0.f;
	}
}