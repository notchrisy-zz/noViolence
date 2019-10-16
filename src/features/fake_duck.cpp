#include "features.h"
#include "../globals.h"
#include "../helpers/console.h"
#include "../helpers/input.h"

namespace fake_duck
{
	bool need_crouch = true;
	bool current_state = false;
	
	void handle(CUserCmd* cmd, bool& send_packet)
	{
		if (!interfaces::local_player || !interfaces::local_player->IsAlive() || !utils::can_lag())
			return;

		//console::print("=== fake duck ===");

		const auto chocked = *reinterpret_cast<int*>(uintptr_t(interfaces::client_state->m_NetChannel) + 0x002C);
	
		current_state = send_packet && globals::binds::fake_duck > 0 && input_system::is_key_down(globals::binds::fake_duck);
		auto weapon = interfaces::local_player->m_hActiveWeapon();
		const auto is_firing = weapon && weapon->CanFire() && cmd->buttons & IN_ATTACK;

		if (chocked > 15 || !current_state)
		{
			//console::print("something went wrong");
			//console::print("chocked: %d", chocked);

			need_crouch = true;

			return;
		}

		const auto anim_state = interfaces::local_player->GetPlayerAnimState();
		//console::print("m_flDuckAmount: %.2f", anim_state->m_flDuckAmount);

		//console::print("crouch state - %d", need_crouch ? 1 : 0);

		if (need_crouch && anim_state->m_flDuckAmount >= 0.8f)
		{
			//console::print("m_flDuckAmount over than 0.9f");

			need_crouch = false;
			return;
		}

		if (!need_crouch && anim_state->m_flDuckAmount <= 0.55f)
		{
			//console::print("m_flDuckAmount lower than 0.5f");
			need_crouch = true;
		}

		send_packet = false;

		if (need_crouch && !is_firing)
			cmd->buttons |= IN_DUCK;
	}
}