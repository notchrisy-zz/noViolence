#include "features.h"

namespace engine_prediction
{
	int* m_pPredictionRandomSeed = nullptr;

	float m_flOldCurtime;
	float m_flOldFrametime;

	int m_fFlags;
	int m_nMoveType;

	auto was_predicted = false;

	bool is_enabled()
	{
		return settings::movement::engine_prediction && interfaces::local_player && interfaces::local_player->IsAlive();
	}

	void start(CUserCmd* cmd)
	{
		was_predicted = is_enabled();
		if (!was_predicted)
			return;

		if (!m_pPredictionRandomSeed)
			m_pPredictionRandomSeed = *(int**)(utils::pattern_scan(PREDICTION_RANDOM_SEED) + 2);

		*m_pPredictionRandomSeed = cmd->random_seed;

		m_flOldCurtime = interfaces::global_vars->curtime;
		m_flOldFrametime = interfaces::global_vars->frametime;
		m_fFlags = interfaces::local_player->m_fFlags();
		m_nMoveType = interfaces::local_player->m_nMoveType();

		interfaces::global_vars->curtime = interfaces::local_player->m_nTickBase() * interfaces::global_vars->interval_per_tick;
		interfaces::global_vars->frametime = interfaces::global_vars->interval_per_tick;

		interfaces::game_movement->StartTrackPredictionErrors(interfaces::local_player);

		void *m_MoveData = *(void**)((DWORD)interfaces::game_movement + 0x8);
		memset(m_MoveData, 0, sizeof(m_MoveData));

		interfaces::move_helper->SetHost(interfaces::local_player);
		interfaces::prediction->SetupMove(interfaces::local_player, cmd, interfaces::move_helper, m_MoveData);
		interfaces::game_movement->ProcessMovement(interfaces::local_player, m_MoveData);
		interfaces::prediction->FinishMove(interfaces::local_player, cmd, m_MoveData);

		interfaces::local_player->m_fFlags() = m_fFlags;
		interfaces::local_player->m_nMoveType() = m_nMoveType;
	}

	void finish(CUserCmd* cmd)
	{
		if (!was_predicted)
			return;

		interfaces::game_movement->FinishTrackPredictionErrors(interfaces::local_player);
		interfaces::move_helper->SetHost(nullptr);

		*m_pPredictionRandomSeed = -1;

		interfaces::global_vars->curtime = m_flOldCurtime;
		interfaces::global_vars->frametime = m_flOldFrametime;
	}
}