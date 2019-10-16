#include "hooks.h"
#include "../globals.h"
#include "../options.hpp"
#include "../helpers/utils.h"
#include "../helpers/console.h"
#include "../features/features.h"

namespace hooks
{
	vfunc_hook post_data_update::hook;

	void post_data_update::setup()
	{
		static bool was_hooked = false;
		if (!interfaces::engine_client->IsInGame() || !interfaces::local_player)
		{
			was_hooked = false;
			return;
		}

		if (was_hooked)
			return;

		was_hooked = true;

		const auto network = static_cast<IClientNetworkable*>(interfaces::local_player);
		if (post_data_update::hook.setup(network))
			post_data_update::hook.hook_index(post_data_update::index, post_data_update::hooked);
	}

	void __stdcall post_data_update::hooked(int update_type)
	{
		skins::handle();
		resolver::handle();

		hook.get_original<fn>(index)(static_cast<IClientNetworkable*>(interfaces::local_player), update_type);
	}
}