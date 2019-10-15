#include "features/features.h"
#include "settings.h"

void features::human_bhop(CUserCmd* cmd) {

	static int hops_restricted = 0;
	static int hops_hit = 0;

	auto local = interfaces::local_player;

	if (!(cmd->buttons & IN_JUMP)
		|| (local->m_nMoveType() & MOVETYPE_LADDER))
		return;
	if (!(local->m_fFlags() & FL_ONGROUND))
	{
	cmd->buttons &= ~IN_JUMP;
	hops_restricted = 0;
	}
	else if ((rand() % 100 > settings::misc::bhop_hit_chance			//chance of hitting first hop is always the same, the 2nd part is that so it always doesn't rape your speed
		&& hops_restricted < settings::misc::hops_restricted_limit)	//the same amount, it can be made a constant if you want to or can be removed, up to you
		|| (settings::misc::max_hops_hit > 0							//force fuck up after certain amount of hops to look more legit, you could add variance to this and
		&& hops_hit > settings::misc::max_hops_hit))				//everything but fuck off that's too much customisation in my opinion, i only added this one because prof told me to
		{
	cmd->buttons &= ~IN_JUMP;
	hops_restricted++;
	hops_hit = 0;
		}
		else
			hops_hit++;
}

