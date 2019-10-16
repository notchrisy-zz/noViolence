#include "features.h"

namespace glows
{
	void handle()
	{
		for (auto i = 0; i < interfaces::glow_obj_manager->m_GlowObjectDefinitions.Count(); i++) {
			auto& glowObject = interfaces::glow_obj_manager->m_GlowObjectDefinitions[i];
			if (glowObject.IsUnused())
				continue;

			auto entity = reinterpret_cast<c_base_player*>(glowObject.m_pEntity);
			if (!entity || entity->is_dormant())
				continue;

			if (CCSPlayer != entity->GetClientClass()->m_ClassID)
				continue;

			if (!entity->IsAlive())
				continue;

			auto is_enemy = entity->m_iTeamNum() != interfaces::local_player->m_iTeamNum();
			if (!is_enemy && !settings::misc::deathmatch)
				continue;

			const auto color = entity->m_iTeamNum() == team::team_ct ? Color(35, 130, 230) : Color(235, 40, 40);

			glowObject.m_flRed = color.r() / 255.0f;
			glowObject.m_flGreen = color.g() / 255.0f;
			glowObject.m_flBlue = color.b() / 255.0f;
			glowObject.m_flAlpha = 0.6f;
			glowObject.m_nGlowStyle = 1;
			glowObject.m_bFullBloomRender = true;
			glowObject.m_bRenderWhenOccluded = true;
			glowObject.m_bRenderWhenUnoccluded = false;
		}
	}

	void shutdown()
	{
		for (auto i = 0; i < interfaces::glow_obj_manager->m_GlowObjectDefinitions.Count(); i++) 
		{
			auto& glowObject = interfaces::glow_obj_manager->m_GlowObjectDefinitions[i];
			auto entity = reinterpret_cast<c_base_player*>(glowObject.m_pEntity);

			if (glowObject.IsUnused())
				continue;

			if (!entity || entity->is_dormant())
				continue;

			glowObject.m_flAlpha = 0.0f;
		}
	}
}
