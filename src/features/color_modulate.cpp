#include "features.h"
#include "../globals.h"
#include "../options.hpp"
#include "../helpers/console.h"

namespace color_modulation
{
	auto debug_crosshair = -1;
	auto view_model_fov = -1.f;

	auto arms_state = false;
	auto post_processing = false;
	auto night_mode_state = false;
	auto arms_wireframe_state = true;

	ImVec4 last_sky;
	ImVec4 arms_color;

	ConVar* viewmodel_fov = nullptr;
	ConVar* r_3dsky = nullptr;

	const uint32_t model_textures = FNV("Model textures");
	const uint32_t skybox_textures = FNV("SkyBox textures");

	uint8_t* sky_fn_offset = nullptr;

	void event()
	{
		if (!sky_fn_offset)
			sky_fn_offset = utils::pattern_scan(SET_SKY);

		assert(sky_fn_offset);

		if (!r_3dsky)
		{
			r_3dsky = interfaces::cvar->find("r_3dsky");
			r_3dsky->m_fnChangeCallbacks.m_Size = 0;
		}

		reinterpret_cast<void(__fastcall*)(const char*)>(sky_fn_offset)(xorstr_("vertigo"));

		r_3dsky->SetValue(0);

		last_sky = ImVec4();
		debug_crosshair = -1;
		night_mode_state = !night_mode_state;
	}

	bool is_vars_changed()
	{
		if (last_sky != settings::visuals::sky)
			return true;

		if (night_mode_state != settings::visuals::night_mode)
			return true;

		if (arms_state != settings::chams::arms::enabled)
			return true;

		if (arms_wireframe_state != settings::chams::arms::wireframe)
			return true;

		if (arms_color != settings::chams::arms::color)
			return true;

		if (post_processing != globals::post_processing)
			return true;

		if (view_model_fov != settings::misc::viewmodel_fov)
			return true;
			
		return false;
	}

	void set_convars()
	{
		static auto r_modelAmbientMin = interfaces::cvar->find(xorstr_("r_modelAmbientMin"));
		static auto mat_postprocess_enable = interfaces::cvar->find(xorstr_("mat_postprocess_enable"));
		static auto mat_force_tonemap_scale = interfaces::cvar->find(xorstr_("mat_force_tonemap_scale"));

		if (!viewmodel_fov)
		{
			viewmodel_fov = interfaces::cvar->find(xorstr_("viewmodel_fov"));
			viewmodel_fov->m_fnChangeCallbacks.m_Size = 0;
		}

		r_modelAmbientMin->SetValue(settings::visuals::night_mode ? 1.f : 0.f);
		mat_force_tonemap_scale->SetValue(settings::visuals::night_mode ? 0.2f : 1.f);

		viewmodel_fov->SetValue(settings::misc::viewmodel_fov);
		mat_postprocess_enable->SetValue(post_processing ? 1 : 0);
	}

	void sniper_crosshair()
	{
		if (!interfaces::local_player)
			return;

		bool is_scoped = interfaces::local_player->m_bIsScoped();
		if (!interfaces::local_player->IsAlive() && interfaces::local_player->m_hObserverTarget())
		{
			auto observer = (c_base_player*) c_base_player::GetEntityFromHandle(interfaces::local_player->m_hObserverTarget());
			if (observer && observer->IsPlayer())
				is_scoped = observer->m_bIsScoped();
		}

		static auto weapon_debug_spread_show = interfaces::cvar->find(xorstr_("weapon_debug_spread_show"));
		
		if (settings::visuals::sniper_crosshair)
		{
			if (debug_crosshair != 0 && is_scoped)
			{
				debug_crosshair = 0;
				weapon_debug_spread_show->SetValue(0);
			}

			if (debug_crosshair != 3 && !is_scoped)
			{
				debug_crosshair = 3;
				weapon_debug_spread_show->SetValue(3);
			}
		}
		else if (debug_crosshair != 0)
		{
			debug_crosshair = 0;
			weapon_debug_spread_show->SetValue(0);
		}
	}

	void handle()
	{
		sniper_crosshair();

		if (!is_vars_changed())
			return;

		last_sky = settings::visuals::sky;

		post_processing = globals::post_processing;
		view_model_fov = settings::misc::viewmodel_fov;
		night_mode_state = settings::visuals::night_mode;

		arms_color = settings::chams::arms::color;
		arms_state = settings::chams::arms::enabled;
		arms_wireframe_state = settings::chams::arms::wireframe;

		set_convars();

		for (auto i = interfaces::mat_system->FirstMaterial(); i != interfaces::mat_system->InvalidMaterial(); i = interfaces::mat_system->NextMaterial(i))
		{
			auto* material = interfaces::mat_system->GetMaterial(i);
			if (!material)
				continue;

			const auto name = material->GetName();
			const auto group = material->GetTextureGroupName();

			const auto _name = fnv::hash_runtime(name);
			const auto _group = fnv::hash_runtime(group);
			
			if (_group == skybox_textures)
				material->ColorModulate(last_sky.x, last_sky.y, last_sky.z);
			else if (_group == model_textures)
			{
				if (std::string(name).substr(0, 28) != std::string(xorstr_("models/weapons/v_models/arms")))
					continue;

				if (std::string(name).substr(0, 34) == std::string(xorstr_("models/weapons/v_models/arms/glove")))
					continue;

				const auto color = settings::chams::arms::color;
				if (settings::chams::arms::enabled)
					material->ColorModulate(color.x, color.y, color.z);
				else
					material->ColorModulate(1.f, 1.f, 1.f);

				//material->AlphaModulate(0.4f);

				material->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, settings::chams::arms::enabled && settings::chams::arms::wireframe);
			}
		}
	}
}
