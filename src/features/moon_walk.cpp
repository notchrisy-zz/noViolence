#include "features.h"

void features::moon_walk(CUserCmd* cmd)
{
	if (!(interfaces::local_player->m_fFlags() & FL_ONGROUND))
		return;

	if (interfaces::local_player->m_nMoveType() == MOVETYPE_NOCLIP)
		return;

	if (interfaces::local_player->m_nMoveType() == MOVETYPE_LADDER)
		return;

	if (cmd->buttons & IN_FORWARD)
	{
		cmd->forwardmove = 450;
		cmd->buttons &= ~IN_FORWARD;
		cmd->buttons |= IN_BACK;
	}
	else if (cmd->buttons & IN_BACK)
	{
		cmd->forwardmove = -450;
		cmd->buttons &= ~IN_BACK;
		cmd->buttons |= IN_FORWARD;
	}

	if (cmd->buttons & IN_MOVELEFT)
	{
		cmd->sidemove = -450;
		cmd->buttons &= ~IN_MOVELEFT;
		cmd->buttons |= IN_MOVERIGHT;
	}
	else if (cmd->buttons & IN_MOVERIGHT)
	{
		cmd->sidemove = 450;
		cmd->buttons &= ~IN_MOVERIGHT;
		cmd->buttons |= IN_MOVELEFT;
	}
}
