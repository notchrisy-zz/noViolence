#include "hooks.h"
#include "../globals.h"
#include "../options.hpp"
#include "../helpers/input.h"
#include "../helpers/console.h"
#include "../helpers/entities.h"
#include "../features/features.h"

namespace hooks
{
	bool __stdcall client_mode::create_move_shared::hooked(float input_sample_frametime, CUserCmd* cmd)
	{
		netchannel::setup();

		static auto original = client_mode::hook.get_original<fn>(index);

		if (!cmd || !cmd->command_number)
			return original(interfaces::client_mode, input_sample_frametime, cmd);

		const auto ebp = reinterpret_cast<uintptr_t*>(uintptr_t(_AddressOfReturnAddress()) - sizeof(void*));
		auto & send_packet = *reinterpret_cast<bool*>(*ebp - 0x1C);

		engine_prediction::start(cmd);

		visuals::fetch_entities();
		entities::fetch_targets(cmd);

		if (settings::visuals::grenade_prediction)
			grenade_prediction::fetch_points(cmd);

		if (cmd->weaponselect == 0 && !lighting_shots::need_switch_weapon)
		{
			aimbot::handle(cmd);
			zeusbot::handle(cmd);
			knife_bot::handle(cmd, send_packet);
		}

		if (settings::lightning_shot::enabled)
			lighting_shots::handle(cmd);

		if (settings::misc::bhop)
			features::bhop(cmd);

		if (settings::misc::auto_strafe)
			features::auto_strafe(cmd);

		if (settings::misc::moon_walk)
			features::moon_walk(cmd);

		slow_walk::handle(cmd);
		fake_lags::handle(cmd, send_packet);
		fake_duck::handle(cmd, send_packet);
		desync::handle(cmd, send_packet);

		engine_prediction::finish(cmd);

#ifndef _DEBUG
		if (cmd->buttons & IN_SCORE)
			hooks::dispatch_user_message::hook.CallOriginal(std::forward<IBaseClientDLL*>(interfaces::base_client), 50, 0, 0, nullptr); // 50 = CS_UM_ServerRankRevealAll
#endif

		if (!(cmd->buttons & IN_BULLRUSH))
			cmd->buttons |= IN_BULLRUSH;

		cmd->viewangles.NormalizeClamp();

		return false;
	}
}