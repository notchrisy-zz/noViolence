#include "features.h"
#include "../globals.h"
#include "../render/render.h"
#include "../helpers/imdraw.h"
#include "../helpers/console.h"
#include "../esp.hpp"
#include "../hooks/hooks.h"
#include "..//runtime_saver.h"
#include "..//render/render.h"
#include "..//Backtrack_new.h"

#include <mutex>

extern float side;

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

	RECT GetBBox(c_base_entity* ent)
	{
		RECT rect{};
		auto collideable = ent->GetCollideable();

		if (!collideable)
			return rect;

		auto min = collideable->OBBMins();
		auto max = collideable->OBBMaxs();

		const matrix3x4_t& trans = ent->m_rgflCoordinateFrame();

		Vector points[] =
		{
			Vector(min.x, min.y, min.z),
			Vector(min.x, max.y, min.z),
			Vector(max.x, max.y, min.z),
			Vector(max.x, min.y, min.z),
			Vector(max.x, max.y, max.z),
			Vector(min.x, max.y, max.z),
			Vector(min.x, min.y, max.z),
			Vector(max.x, min.y, max.z)
		};

		Vector pointsTransformed[8];

		for (int i = 0; i < 8; i++)
			math::VectorTransform(points[i], trans, pointsTransformed[i]);

		Vector screen_points[8] = {};

		for (int i = 0; i < 8; i++)
		{
			if (!math::world2screen(pointsTransformed[i], screen_points[i]))
				return rect;
		}

		auto left = screen_points[0].x;
		auto top = screen_points[0].y;
		auto right = screen_points[0].x;
		auto bottom = screen_points[0].y;

		for (int i = 1; i < 8; i++)
		{
			if (left > screen_points[i].x)
				left = screen_points[i].x;

			if (top < screen_points[i].y)
				top = screen_points[i].y;

			if (right < screen_points[i].x)
				right = screen_points[i].x;

			if (bottom > screen_points[i].y)
				bottom = screen_points[i].y;
		}

		return RECT{ (long)left, (long)top, (long)right, (long)bottom };
	}

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

		auto bbox = GetBBox(entity);

		grenade_info_t info;
		const auto model_name = fnv::hash_runtime(interfaces::mdl_info->GetModelName(entity->GetModel()));
		if (model_name == FNV("models/Weapons/w_eq_smokegrenade_thrown.mdl"))
			info = { "Smoke", Color::White };
		else if (model_name == FNV("models/Weapons/w_eq_flashbang_dropped.mdl"))
			info = { "Flash", Color::Yellow };
		else if (model_name == FNV("models/Weapons/w_eq_fraggrenade_dropped.mdl"))
			info = { "Grenade", Color::Red };
		else if (model_name == FNV("models/Weapons/w_eq_molotov_dropped.mdl") || model_name == FNV("models/Weapons/w_eq_incendiarygrenade_dropped.mdl"))
			info = { "Molly", Color::Orange };
		else if (model_name == FNV("models/Weapons/w_eq_decoy_dropped.mdl"))
			info = { "Decoy", Color::Green };

		if (!info.name.empty())
			push_entity(entity, info.name, info.color);
	}

	void RenderPunchCross()
	{

		int w, h;

		g::engine_client->GetScreenSize(w, h);

		if (!g::engine_client->IsInGame() || !g::engine_client->IsConnected())
			return;

		if (!g::local_player && !g::local_player->IsAlive())
			return;

		int x = w / 2;
		int y = h / 2;
		int dy = h / 97;
		int dx = w / 97; //97 or 85

		QAngle punchAngle = g::local_player->m_aimPunchAngle();
		x -= (dx * (punchAngle.yaw));
		y += (dy * (punchAngle.pitch));

		float radius = settings::visuals::radius;

		switch (settings::visuals::rcs_cross_mode)
		{
		case 0:
			VGSHelper::Get().DrawLine(x - 5, y, x + 5, y, settings::visuals::recoilcolor, 1.0f);
			VGSHelper::Get().DrawLine(x, y - 5, x, y + 5, settings::visuals::recoilcolor, 1.0f);
			break;
		case 1:
			VGSHelper::Get().DrawCircle(x, y, radius, 255, settings::visuals::recoilcolor);
			break;
		}

	}

	void Choke()
	{
		std::stringstream ss;
		ss << "choked: " << g::client_state->chokedcommands;

		if (!g::engine_client->IsInGame() || !g::engine_client->IsConnected())
			return;

		if (!g::local_player && !g::local_player->IsAlive())
			return;

		VGSHelper::Get().DrawTextW(ss.str(), 10.0f, 450.0f, Color::White, 14);
	}

	void DrawFov() //todo
	{
		auto pWeapon = g::local_player->m_hActiveWeapon();
		if (!pWeapon)
			return;

		auto settings = settings::aimbot::m_items[pWeapon->m_iItemDefinitionIndex()];

		if (settings.enabled) {

			float fov = static_cast<float>(g::local_player->GetFOV());

			if (!g::engine_client->IsInGame() || !g::engine_client->IsConnected())
				return;


			if (!g::local_player && !g::local_player->IsAlive())
				return;

			int w, h;
			g::engine_client->GetScreenSize(w, h);

			Vector2D screenSize = Vector2D(w, h);
			Vector2D center = screenSize * 0.5f;

			float ratio = screenSize.x / screenSize.y;
			float screenFov = atanf((ratio) * (0.75f) * tan(DEG2RAD(fov * 0.5f)));

			float radiusFOV = tanf(DEG2RAD(aimbot::get_fov())) / tanf(screenFov) * center.x;

			VGSHelper::Get().DrawCircle(center.x, center.y, radiusFOV, 32, Color(255, 255, 255, 100));
		}
	}

	void RenderHitmarker()
	{
		if (!g::engine_client->IsInGame() || !g::engine_client->IsConnected())
			return;

		static int cx;
		static int cy;
		static int w, h;

		g::engine_client->GetScreenSize(w, h);
		cx = w / 2;
		cy = h / 2;

		//g_Saver.HitmarkerInfo.HitTime
		if (g::global_vars->realtime - saver.HitmarkerInfo.HitTime > .5f)
			return;

		float percent = (g::global_vars->realtime - saver.HitmarkerInfo.HitTime) / .5f;
		float percent2 = percent;

		if (percent > 1.f)
		{
			percent = 1.f;
			percent2 = 1.f;
		}

		percent = 1.f - percent;
		float addsize = percent2 * 5.f;

		Color clr = Color(255, 255, 255, (int)(percent * 255.f));

		VGSHelper::Get().DrawLine(cx - 3.f - addsize, cy - 3.f - addsize, cx + 3.f + addsize, cy + 3.f + addsize, clr, 1.f);
		VGSHelper::Get().DrawLine(cx - 3.f - addsize, cy + 3.f + addsize, cx + 3.f + addsize, cy - 3.f - addsize, clr, 1.f);
	}

	void RenderNoScopeOverlay()
	{
		if (!g::engine_client->IsInGame() || !g::engine_client->IsConnected())
			return;

		static int cx;
		static int cy;
		static int w, h;

		g::engine_client->GetScreenSize(w, h);
		cx = w / 2;
		cy = h / 2;


		if (g::local_player->m_bIsScoped())
		{
			VGSHelper::Get().DrawLine(0, cy, w, cy, Color::Black);
			VGSHelper::Get().DrawLine(cx, 0, cx, h, Color::Black);
			VGSHelper::Get().DrawCircle(cx, cy, 255, 255, Color::Black);
		}
	}

	void chams_misc(const ModelRenderInfo_t& info) noexcept {

		auto model_name = interfaces::mdl_info->GetModelName((model_t*)info.pModel);

		if (!model_name)
			return;

		static IMaterial* mat = nullptr;

		auto flat = interfaces::mat_system->FindMaterial("debug/debugdrawflat", TEXTURE_GROUP_MODEL, true, nullptr);

		mat = flat;

		if (settings::chams::wepchams && strstr(model_name, "models/weapons/v_")
			&& !strstr(model_name, "arms") && !strstr(model_name, "sleeve")) {

			interfaces::render_view->SetColorModulation(settings::chams::clr_weapon_chams);
			mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
			interfaces::mdl_render->ForcedMaterialOverride(mat);
		}
	}

	void more_chams() noexcept
	{

		static IMaterial* mat = nullptr;

		auto flat = interfaces::mat_system->FindMaterial("debug/debugdrawflat", TEXTURE_GROUP_MODEL, true, nullptr);

		mat = flat;

		for (int i = 0; i < interfaces::entity_list->GetHighestEntityIndex(); i++) {
			auto entity = reinterpret_cast<c_base_player*>(interfaces::entity_list->GetClientEntity(i));

			if (entity && entity != g::local_player) {
				auto client_class = entity->GetClientClass();
				auto model_name = interfaces::mdl_info->GetModelName(entity->GetModel());

				switch (client_class->m_ClassID) {
				case EClassId::CPlantedC4:
				case EClassId::CBaseAnimating:
					if (settings::chams::plantedc4_chams_xqz) {
						interfaces::render_view->SetColorModulation(settings::chams::clr_plantedc4_chams_xqz);
						mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
						interfaces::mdl_render->ForcedMaterialOverride(mat);
						entity->DrawModel(1, 255);
					}
					if (settings::chams::plantedc4_chams) {
						interfaces::render_view->SetColorModulation(settings::chams::clr_plantedc4_chams);
						mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
						interfaces::mdl_render->ForcedMaterialOverride(mat);
						entity->DrawModel(1, 255);
					}
					break;
					//	grenedes
				case EClassId::CHEGrenade:
				case EClassId::CFlashbang:
				case EClassId::CMolotovGrenade:
				case EClassId::CMolotovProjectile:
				case EClassId::CIncendiaryGrenade:
				case EClassId::CDecoyGrenade:
				case EClassId::CDecoyProjectile:
				case EClassId::CSmokeGrenade:
				case EClassId::CSmokeGrenadeProjectile:
				case EClassId::ParticleSmokeGrenade:
				case EClassId::CBaseCSGrenade:
				case EClassId::CBaseCSGrenadeProjectile:
				case EClassId::CBaseGrenade:
				case EClassId::CBaseParticleEntity:
				case EClassId::CSensorGrenade:
				case EClassId::CSensorGrenadeProjectile:
					if (settings::chams::nade_chams_xqz) {
						interfaces::render_view->SetColorModulation(settings::chams::clr_nade_chams_xqz);
						mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
						interfaces::mdl_render->ForcedMaterialOverride(mat);
						entity->DrawModel(1, 255);
					}
					if (settings::chams::nade_chams) {
						interfaces::render_view->SetColorModulation(settings::chams::clr_nade_chams);
						mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
						interfaces::mdl_render->ForcedMaterialOverride(mat);
						entity->DrawModel(1, 255);
					}
					break;
				}


				if (client_class->m_ClassID == CAK47 || client_class->m_ClassID == CDEagle || client_class->m_ClassID == CC4 ||
					client_class->m_ClassID >= CWeaponAug && client_class->m_ClassID <= CWeaponXM1014) {
					if (settings::chams::wep_droppedchams_xqz) {
						interfaces::render_view->SetColorModulation(settings::chams::clr_weapon_dropped_chams_xqz);
						mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
						interfaces::mdl_render->ForcedMaterialOverride(mat);
						entity->DrawModel(1, 255);
					}
					if (settings::chams::wep_droppedchams) {
						interfaces::render_view->SetColorModulation(settings::chams::clr_weapon_dropped_chams);
						mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
						interfaces::mdl_render->ForcedMaterialOverride(mat);
						entity->DrawModel(1, 255);
					}
				}
				interfaces::mdl_render->ForcedMaterialOverride(nullptr);
			}
		}
	}

	void AAIndicator()
	{
		int x, y;

		g::engine_client->GetScreenSize(x, y);

		if (!g::local_player || !g::local_player->IsAlive())
			return;

		if (!g::engine_client->IsInGame() || !g::engine_client->IsConnected())
			return;

		int cx = x / 2;
		int cy = y / 2;


		VGSHelper::Get().DrawText(side > 0.0f ? "<" : ">", side > 0.0f ? cx - 50 : cx + 40, cy - 10, Color::White, 19);
	}

	void DesyncChams()
	{
		if (!g::engine_client->IsInGame() || !g::engine_client->IsConnected())
			return;

		Vector OrigAng;
		IMaterial* Dagtag = g::mat_system->FindMaterial("models/inventory_items/dogtags/dogtags_outline", "Other textures");
		IMaterial* flat = g::mat_system->FindMaterial("debug/debugdrawflat", "Other textures");
		IMaterial* regular = g::mat_system->FindMaterial("simple_regular", TEXTURE_GROUP_MODEL);
		IMaterial* Metallic = g::mat_system->FindMaterial("simple_reflective", TEXTURE_GROUP_MODEL);
		OrigAng = g::local_player->GetAbsAngles2();
		g::local_player->SetAngle2(Vector(0, g::local_player->GetPlayerAnimState2()->m_flEyeYaw, 0)); //around 90% accurate
		if (g::Input->m_fCameraInThirdPerson)
		{
			g::mdl_render->ForcedMaterialOverride(Metallic);
			g::render_view->SetColorModulation(1.0f, 1.0f, 1.0f);
		}
		g::local_player->GetClientRenderable()->DrawModel(0x1, 255);
		g::local_player->SetAngle2(OrigAng);
	}

	float csgo_armor(float damage, int armor_value) {
		float armor_ratio = 0.5f;
		float armor_bonus = 0.5f;
		if (armor_value > 0) {
			float armor_new = damage * armor_ratio;
			float armor = (damage - armor_new) * armor_bonus;

			if (armor > static_cast<float>(armor_value)) {
				armor = static_cast<float>(armor_value) * (1.f / armor_bonus);
				armor_new = damage - armor;
			}

			damage = armor_new;
		}
		return damage;
	}

	void bomb_esp(c_planted_c4* entity) noexcept {
		if (!settings::esp::bomb_esp)
			return;

		auto local_player = reinterpret_cast<c_base_player*>(interfaces::entity_list->GetClientEntity(interfaces::engine_client->GetLocalPlayer()));
		if (!local_player)
			return;

		auto explode_time = entity->m_flC4Blow();
		auto remaining_time = explode_time - (interfaces::global_vars->interval_per_tick * local_player->m_nTickBase());
		if (remaining_time < 0)
			return;

		int width, height;
		interfaces::engine_client->GetScreenSize(width, height);

		Vector bomb_origin, bomb_position;
		bomb_origin = entity->m_vecOrigin();

		explode_time -= interfaces::global_vars->interval_per_tick * local_player->m_nTickBase();
		if (explode_time <= 0)
			explode_time = 0;

		char buffer[64];
		sprintf_s(buffer, "%.2f", explode_time);

		auto c4_timer = interfaces::cvar->find("mp_c4timer")->GetInt();
		auto value = (explode_time * height) / c4_timer;

		//bomb damage indicator calculations, credits casual_hacker
		float damage;
		float hp_reimaing = g::local_player->m_iHealth();
		auto distance = local_player->GetEyePos().DistTo(entity->m_vecOrigin());
		auto a = 450.7f;
		auto b = 75.68f;
		auto c = 789.2f;
		auto d = ((distance - b) / c);
		auto fl_damage = a * exp(-d * d);
		damage = float((std::max)((int)ceilf(csgo_armor(fl_damage, g::local_player->m_ArmorValue())), 0));
		hp_reimaing -= damage;

		//convert damage to string
		//std::string damage_text;
		//damage_text += "-";
		//damage_text += std::to_string((int)(damage));
		//damage_text += "HP";

		std::string damage_text;
		damage_text += "HP LEFT: ";
		damage_text += std::to_string((int)(hp_reimaing));

		//render on screen bomb bar
		/*if (explode_time <= 10) {
			render.draw_filled_rect(0, 0, 10, value, color(255, 0, 0, 180));
		}
		else {
			render.draw_filled_rect(0, 0, 10, value, color(0, 255, 0, 180));
		} */

		c_planted_c4* bomb = nullptr;
		for (int i = 1; i < interfaces::entity_list->GetHighestEntityIndex(); i++) {

			if (entity->GetClientClass()->m_ClassID == EClassId::CPlantedC4) {
				bomb = (c_planted_c4*)entity;
				break;
			}
		}
		//render bomb timer
		//render.draw_text(12, value - 11, render.name_font_big, buffer, false, color(255, 255, 255));



		//render bomb damage
		if (g::local_player->IsAlive() && damage <= g::local_player->m_iHealth()) {
			VGSHelper::Get().DrawTextW(damage_text, width / 2 - 95, height / 2 + 255, Color::White, 50);
		}

		//render fatal check
		if (g::local_player->IsAlive() && damage >= g::local_player->m_iHealth()) {
			VGSHelper::Get().DrawTextW("HP LEFT: 0", width / 2 - 95, height / 2 + 255, Color::Red, 50);
			//VGSHelper::Get().DrawTextW("Fatal!", width / 2 - 50, height / 2 + 240, Color::Red, 50);
		}

		if (!math::world2screen(bomb_origin, bomb_position))
			return;
		VGSHelper::Get().DrawTextW(buffer, bomb_position.x - 13, bomb_position.y + 8, Color::White, 15);
		//VGSHelper::Get().DrawFilledBox(bomb_position.x - c4_timer / 2, bomb_position.y + 13, c4_timer, 3, Color::Black);  //wont draw 
		//VGSHelper::Get().DrawFilledBox(bomb_position.x - c4_timer / 2, bomb_position.y + 13, explode_time, 3, Color::Blue);

	}

	void SpreadCircle()
	{
		if (!g::local_player || !g::local_player->IsAlive())
			return;

		c_base_combat_weapon* weapon = g::local_player->m_hActiveWeapon().Get();

		if (!weapon)
			return;

		float spread = weapon->GetInaccuracy() * 1000;

		if (spread == 0.f)
			return;

		//Console.WriteLine(spread);
		int x, y;
		g::engine_client->GetScreenSize(x, y);
		float cx = x / 2.f;
		float cy = y / 2.f;
		VGSHelper::Get().DrawCircle(cx, cy, spread, 35, settings::visuals::spread_cross_color);
	}

	/*void DrawGrenade(c_base_entity* ent) // !!! CAUSES FPS DROPS !!!
	{
		auto id = ent->GetClientClass()->m_ClassID;
		Vector vGrenadePos2D;
		Vector vGrenadePos3D = ent->m_vecOrigin();

		auto bbox = GetBBox(ent);

		if (bbox.right == 0 || bbox.bottom == 0)
			return;

		if (!math::world2screen(vGrenadePos3D, vGrenadePos2D))
			return;

		static const auto white_color = ImGui::GetColorU32(ImVec4::White);

		switch (id)
		{
		case EClassId::CSmokeGrenadeProjectile:
			VGSHelper::Get().DrawBox(bbox.left, bbox.top, bbox.right, bbox.bottom, Color::White);
			break;

		case EClassId::CBaseCSGrenadeProjectile:
		{
			model_t* model = (model_t*)ent->GetModel();

			if (!model)
			{
				return;
			}

			studiohdr_t* hdr = g::mdl_info->GetStudiomodel(model);

			if (!hdr)
			{
				return;
			}

			std::string name = hdr->szName;

			if (name.find("incendiarygrenade") != std::string::npos || name.find("fraggrenade") != std::string::npos)
			{
				//VGSHelper::Get().DrawBox(bbox.left, bbox.top, bbox.right, bbox.bottom, Color::Red);
				return;
			}
			//VGSHelper::Get().DrawBox(bbox.left, bbox.top, bbox.right, bbox.bottom, Color::Yellow);
			break;
		}

		case EClassId::CMolotovProjectile:
			//VGSHelper::Get().DrawBox(bbox.left, bbox.top, bbox.right, bbox.bottom, Color::Red);
			break;

		case EClassId::CDecoyProjectile:
			//VGSHelper::Get().DrawBox(bbox.left, bbox.top, bbox.right, bbox.bottom, Color::White);
			break;
		}
	} */

	void glow() noexcept {

		auto local_player = reinterpret_cast<c_base_player*>(interfaces::entity_list->GetClientEntity(interfaces::engine_client->GetLocalPlayer()));
		if (!local_player)
			return;

		for (size_t i = 0; i < interfaces::glow_manager->size; i++) {
			auto& glow = interfaces::glow_manager->objects[i];
			if (glow.unused())
				continue;

			auto glow_entity = reinterpret_cast<c_base_player*>(glow.entity);
			auto client_class = glow_entity->GetClientClass();
			if (!glow_entity || glow_entity->IsDormant())
				continue;

			auto is_enemy = glow_entity->m_iTeamNum() != local_player->m_iTeamNum();
			auto is_teammate = glow_entity->m_iTeamNum() == local_player->m_iTeamNum();


			switch (client_class->m_ClassID) {
			case EClassId::CCSPlayer:
				if (is_enemy && settings::glow::GlowEnemyEnabled) {
					glow.set(settings::glow::GlowEnemy[0], settings::glow::GlowEnemy[1], settings::glow::GlowEnemy[2], settings::glow::GlowEnemy[3]);
				}
				else if (is_teammate && settings::glow::GlowTeamEnabled) {
					glow.set(settings::glow::GlowTeam[0], settings::glow::GlowTeam[1], settings::glow::GlowTeam[2], settings::glow::GlowTeam[3]);
				}
				break;
			case EClassId::CPlantedC4:
			case EClassId::CBaseAnimating:
				if (settings::glow::GlowC4PlantedEnabled) {
					glow.set(settings::glow::GlowC4Planted[0], settings::glow::GlowC4Planted[1], settings::glow::GlowC4Planted[2], settings::glow::GlowC4Planted[3]);
				}
				break;
				//	grenedes
			case EClassId::CHEGrenade:
			case EClassId::CFlashbang:
			case EClassId::CMolotovGrenade:
			case EClassId::CMolotovProjectile:
			case EClassId::CIncendiaryGrenade:
			case EClassId::CDecoyGrenade:
			case EClassId::CDecoyProjectile:
			case EClassId::CSmokeGrenade:
			case EClassId::CSmokeGrenadeProjectile:
			case EClassId::ParticleSmokeGrenade:
			case EClassId::CBaseCSGrenade:
			case EClassId::CBaseCSGrenadeProjectile:
			case EClassId::CBaseGrenade:
			case EClassId::CBaseParticleEntity:
			case EClassId::CSensorGrenade:
			case EClassId::CSensorGrenadeProjectile:
				if (settings::glow::GlowNadesEnabled) {
					glow.set(settings::glow::GlowNades[0], settings::glow::GlowNades[1], settings::glow::GlowNades[2], settings::glow::GlowNades[3]);
				}
				break;
			}

			if (client_class->m_ClassID == CAK47 || client_class->m_ClassID == CDEagle || client_class->m_ClassID == CC4 ||
				client_class->m_ClassID >= CWeaponAug && client_class->m_ClassID <= CWeaponXM1014) {
				if (settings::glow::GlowDroppedWeaponsEnabled) {
					glow.set(settings::glow::GlowDroppedWeapons[0], settings::glow::GlowDroppedWeapons[1], settings::glow::GlowDroppedWeapons[2], settings::glow::GlowDroppedWeapons[3]);
				}
			}
		}
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
			if (settings::visuals::world_grenades && (classid == 9 || classid == 134 || classid == 111 || classid == 113 || classid == 156 || classid == 48)) //9 = HE,113 = molly,156 = smoke,48 = decoy
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