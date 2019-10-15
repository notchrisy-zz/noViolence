#include "features.h"
#include "../globals.h"
#include "../options.hpp"
#include "../helpers/console.h"

namespace color_modulation
{
	auto debug_crosshair = -1;
	auto view_model_fov = -1.f;
	auto debug_model_fov = -1;

	auto arms_state = false;
	auto post_processing = false;
	auto night_mode_state = false;
	auto arms_wireframe_state = true;

	ImVec4 last_sky;
	ImVec4 arms_color;

	ConVar* viewmodel_fov = nullptr;
	ConVar* debug_fov = nullptr;
	ConVar* r_3dsky = nullptr;
	ConVar* pitchup = nullptr;
	ConVar* pitchdown = nullptr;
	ConVar* noclip = nullptr;
	ConVar* r_DrawSpecificStaticProp = nullptr;

	const uint32_t model_textures = FNV("Model textures");
	const uint32_t skybox_textures = FNV("SkyBox textures");
	const uint32_t world_textures = FNV("World textures");
	const uint32_t static_prop_textures = FNV("StaticProp textures");
	const uint32_t particle_textures = FNV("Particle textures");


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

		if (!r_DrawSpecificStaticProp)
		{
			r_DrawSpecificStaticProp = interfaces::cvar->find("r_drawspecificstaticprop");
			r_DrawSpecificStaticProp->m_fnChangeCallbacks.m_Size = 0;
		}

		reinterpret_cast<void(__fastcall*)(const char*)>(sky_fn_offset)(xorstr_("vertigo"));

		static auto sv_skyname = interfaces::cvar->find(xorstr_("sv_skyname"));

		r_3dsky->SetValue(0);
		r_DrawSpecificStaticProp->SetValue(settings::visuals::night_mode ? 0 : 1);

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

		if (debug_model_fov != settings::misc::debug_fov)
			return true;

		return false;
	}

	void SkyChanger()
	{

		static auto sv_skyname = interfaces::cvar->find(xorstr_("sv_skyname"));

		switch (settings::visuals::skychanger_mode)
		{
		case 0: //Baggage
			sv_skyname->SetValue("cs_baggage_skybox");
			break;
		case 1: //Tibet
			sv_skyname->SetValue("cs_tibet");
			break;
		case 2: //Embassy
			sv_skyname->SetValue("embassy");
			break;
		case 3: //Italy
			sv_skyname->SetValue("italy");
			break;
		case 4: //Daylight 1
			sv_skyname->SetValue("sky_cs15_daylight01_hdr");
			break;
		case 5: //Daylight 2
			sv_skyname->SetValue("sky_cs15_daylight02_hdr");
			break;
		case 6: //Daylight 3
			sv_skyname->SetValue("sky_cs15_daylight03_hdr");
			break;
		case 7: //Daylight 4
			sv_skyname->SetValue("sky_cs15_daylight04_hdr");
			break;
		case 8: //Cloudy
			sv_skyname->SetValue("sky_csgo_cloudy01");
			break;
		case 9: //Night 1
			sv_skyname->SetValue("sky_csgo_night02");
			break;
		case 10: //Night 2
			sv_skyname->SetValue("sky_csgo_night02b");
			break;
		case 11: //Night Flat
			sv_skyname->SetValue("sky_csgo_night_flat");
			break;
		case 12: //Day HD
			sv_skyname->SetValue("sky_day02_05_hdr");
			break;
		case 13: //Day
			sv_skyname->SetValue("sky_day02_05");
			break;
		case 14: //Rural
			sv_skyname->SetValue("sky_l4d_rural02_ldr");
			break;
		case 15: //Vertigo HD
			sv_skyname->SetValue("vertigo_hdr");
			break;
		case 16: //Vertigo Blue HD
			sv_skyname->SetValue("vertigoblue_hdr");
			break;
		case 17: //Vertigo
			sv_skyname->SetValue("vertigo");
			break;
		case 18: //Vietnam
			sv_skyname->SetValue("vietnam");
			break;
		case 19: //Dusty Sky
			sv_skyname->SetValue("sky_dust");
			break;
		case 20: //Jungle
			sv_skyname->SetValue("jungle");
			break;
		case 21: //Nuke
			sv_skyname->SetValue("nukeblank");
			break;
		case 22: //Office
			sv_skyname->SetValue("office");
			break;
		}



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

		if (!debug_fov)
		{
			debug_fov = interfaces::cvar->find(xorstr_("fov_cs_debug"));
			debug_fov->m_fnChangeCallbacks.m_Size = 0;
		}

		if (!pitchdown)
		{
			pitchdown = interfaces::cvar->find(xorstr_("cl_pitchdown"));
			pitchdown->m_fnChangeCallbacks.m_Size = 0;
		}

		if (!pitchup)
		{
			pitchup = interfaces::cvar->find(xorstr_("cl_pitchup"));
			pitchup->m_fnChangeCallbacks.m_Size = 0;
		}

		r_modelAmbientMin->SetValue(settings::visuals::night_mode ? 1.f : 0.f);
		mat_force_tonemap_scale->SetValue(settings::visuals::night_mode ? 0.2f : 1.f);

		viewmodel_fov->SetValue(settings::misc::viewmodel_fov);
		//debug_fov->SetValue(settings::misc::debug_fov);
		mat_postprocess_enable->SetValue(post_processing ? 1 : 0);


		if (settings::esp::mat_force)
		{
			static auto mat_force_tonemap_scale = interfaces::cvar->find(xorstr_("mat_force_tonemap_scale"));

			mat_force_tonemap_scale->SetValue(settings::esp::mfts);
		}
	}

	void sniper_crosshair()
	{
		if (!interfaces::local_player)
			return;

		bool is_scoped = interfaces::local_player->m_bIsScoped();
		if (!interfaces::local_player->IsAlive() && interfaces::local_player->m_hObserverTarget())
		{
			auto observer = (c_base_player*)c_base_player::GetEntityFromHandle(interfaces::local_player->m_hObserverTarget());
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
		debug_model_fov = settings::misc::debug_fov;
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


			//if (_group == skybox_textures)
				//material->ColorModulate(last_sky.x, last_sky.y, last_sky.z); 

			if (_group == model_textures)
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

			if (_group == static_prop_textures && settings::visuals::night_mode) //Static props, aka Ticket box on A Mirage or boxes.
			{
				material->ColorModulate(0.5f, 0.5f, 0.5f);
				//material->SetMaterialVarFlag(MATERIAL_VAR_TRANSLUCENT, false);
				//material->AlphaModulate(1.0f);
			}

			/*if (_group == world_textures && settings::visuals::night_mode) //walls
			{
				//material->SetMaterialVarFlag(MATERIAL_VAR_TRANSLUCENT, false);
				material->ColorModulate(0.2f, 0.2f, 0.2f);
				//material->AlphaModulate(1.0f);
			} */

			if (_group == particle_textures)
				material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);

			static auto sv_skyname = interfaces::cvar->find(xorstr_("sv_skyname"));

			if (settings::visuals::night_mode)
				sv_skyname->SetValue("sky_csgo_night02");
		}
	}
}