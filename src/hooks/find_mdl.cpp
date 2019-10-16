#include "hooks.h"
#include "../options.hpp"
#include "../helpers/console.h"

namespace hooks
{
	blackbone::Detour<find_mdl::fn> find_mdl::hook;

	std::vector<std::string> game_models = 
	{
		//=== inferno sky ===

		"models/props/de_nuke/hr_nuke/nuke_skydome_001/nuke_skydome_001.mdl",
		"models/props/de_nuke/hr_nuke/nuke_skydome_001/nuke_skydome_002.mdl",
		"models/props/de_nuke/hr_nuke/nuke_skydome_001/nuke_skydome_003.mdl",

		//=== de dust 2 sky ===

		"models/props/de_dust/hr_dust/dust_skybox/dust_skydome.mdl",
		"models/props/de_dust/hr_dust/dust_skybox/dust_skybox_buildings_combined_01.mdl"

		//=== mirage ===
			
		//trees on ct
		"models/props_foliage/tree_palm_dust.mdl",
		"models/props_foliage/mall_small_palm01.mdl",
		"models/props_foliage/mall_small_palm01_medium.mdl",
		"models/props_foliage/mall_small_palm01_cluster_medium.mdl",
		"models/props_foliage/urban_palm01.mdl",
		"models/props_foliage/urban_small_palm01.mdl",
		"models/props_foliage/urban_palm01_medium.mdl",

		"models/props_vehicles/bus01_2.mdl", //bus on ct
		"models/props/de_piranesi/pi_apc.mdl", //tank on ct 

		"models/props/de_mirage/large_door_b.mdl",
		"models/props/de_mirage/large_door_c.mdl",

		"models/props_foliage/mall_grass_bush01.mdl",
	};

	std::string get_new_model(std::string relative_path)
	{
		for (auto& model : game_models)
		{
			if (relative_path == model)
				return "models/editor/spot.mdl";
		}

		return relative_path;
	}

	MDLHandle_t __stdcall find_mdl::hooked(IMDLCache*& thisptr, const char*& relative_path)
	{
		const auto path = get_new_model(relative_path);
	
		return hook.CallOriginal(std::forward<IMDLCache*>(thisptr), std::forward<const char*>(path.c_str()));
	}
}
