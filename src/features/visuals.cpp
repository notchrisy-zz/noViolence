#include "features.h"
#include "../globals.h"
#include "../render/render.h"
#include "../helpers/imdraw.h"
#include "../helpers/console.h"

#include <mutex>

namespace visuals
{
	std::mutex render_mutex;

	struct entity_data_t
	{
		std::string text;
		Vector origin;
		Color color;
	};

	struct grenade_info_t
	{
		std::string name;
		Color color;
	};

	std::vector<entity_data_t> entities;
	std::vector<entity_data_t> saved_entities;

	bool is_enabled()
	{
		return interfaces::engine_client->IsConnected() && interfaces::local_player && !render::menu::is_visible();
	}

	void push_entity(c_base_entity* entity, const std::string& text, const Color& color = Color::White)
	{
		entities.emplace_back(entity_data_t{ text, entity->m_vecOrigin(), color });
	}

	void world_grenades(c_base_player* entity)
	{
		if (!interfaces::local_player || !interfaces::local_player->IsAlive())
			return;

		if (interfaces::local_player->IsFlashed())
			return;

		if (!interfaces::local_player->CanSeePlayer(entity, entity->GetRenderOrigin()))
			return;

		if (utils::is_line_goes_through_smoke(interfaces::local_player->GetEyePos(), entity->GetRenderOrigin()))
			return;

		grenade_info_t info;
		const auto model_name = fnv::hash_runtime(interfaces::mdl_info->GetModelName(entity->GetModel()));
		if (model_name == FNV("models/Weapons/w_eq_smokegrenade_thrown.mdl"))
			info = { "Smoke", Color::White };
		 else if (model_name == FNV("models/Weapons/w_eq_flashbang_dropped.mdl"))
			info = { "Flash", Color::Yellow };
		else if (model_name == FNV("models/Weapons/w_eq_fraggrenade_dropped.mdl"))
			info = { "Frag", Color::Orange };
		else if (model_name == FNV("models/Weapons/w_eq_molotov_dropped.mdl") || model_name == FNV("models/Weapons/w_eq_incendiarygrenade_dropped.mdl"))
			info = { "Molotov", Color::Red };

		if (!info.name.empty())
			push_entity(entity, info.name, info.color);
	}

	void fetch_entities()
	{
		render_mutex.lock();

		entities.clear();

		if (!is_enabled())
		{
			render_mutex.unlock();
			return;
		}

		for (auto i = 1; i <= interfaces::entity_list->GetHighestEntityIndex(); ++i)
		{
			auto* entity = c_base_player::GetPlayerByIndex(i);
			if (!entity || entity->IsPlayer() || entity->is_dormant() || entity == interfaces::local_player)
				continue;
			
			const auto classid = entity->GetClientClass()->m_ClassID;
			if (settings::visuals::world_grenades && (classid == 9 || classid == 134 || classid == 111))
				world_grenades(entity);
			else if (settings::visuals::planted_c4 && entity->IsPlantedC4())
				push_entity(entity, "Bomb", Color::Yellow);
			else if (settings::visuals::defuse_kit && entity->IsDefuseKit() && !entity->m_hOwnerEntity().IsValid())
				push_entity(entity, "Defuse Kit", Color::Green);
			else if (settings::visuals::dropped_weapons && entity->IsWeapon() && !entity->m_hOwnerEntity().IsValid())
				push_entity(entity, utils::get_weapon_name(entity), Color::White);
		}

		render_mutex.unlock();
	}

	void render(ImDrawList* draw_list)
	{
		if (!is_enabled() || !render::fonts::visuals)
			return;
		
		if (render_mutex.try_lock())
		{
			saved_entities = entities;
			render_mutex.unlock();
		}
		
		ImGui::PushFont(render::fonts::visuals);

		Vector origin;
		for (const auto& entity : saved_entities)
		{
			if (math::world2screen(entity.origin, origin))
			{
				const auto text_size = ImGui::CalcTextSize(entity.text.c_str());
				imdraw::outlined_text(entity.text.c_str(), ImVec2(origin.x - text_size.x / 2.f, origin.y), utils::to_im32(entity.color));
			}
		}

		ImGui::PopFont();
	}
}
