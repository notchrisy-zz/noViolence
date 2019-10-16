#include "../hooks/hooks.h"
#include "../globals.h"
#include "../render/render.h"
#include "../helpers/utils.h"
#include "../helpers/console.h"
#include "../helpers/entities.h"
#include "../helpers/notifies.h"
#include "../features/features.h"
#include "c_game_event_listener.h"
#include "..//Chams.h"
#include "..//HitPossitionHelper.h"
#include "..//runtime_saver.h"
#include "..//jsoncpp/json.h"
#include "..//helpers/input.h"


using namespace blackbone;

static CCSGOPlayerAnimState g_AnimState;

std::map<int, item_setting> m_items = { };

namespace hooks
{
	vfunc_hook d3d9::hook;
	vfunc_hook client_mode::hook;
	vfunc_hook sound_hook::hook;
	vfunc_hook vgui_panel::hook;
	vfunc_hook mdlrender::hook;
	vfunc_hook renderview::hook;
	vfunc_hook events::hook;
	vfunc_hook SL::hook;

	c_game_event_listener* event_listener;

	template<typename Structure>
	bool set_hook(void* _interface, call_order call_order = call_order::HookFirst, return_method return_type = return_method::UseOriginal)
	{
		const auto address = (*reinterpret_cast<uintptr_t * *>(_interface))[Structure::index];
		auto ptr = brutal_cast<Structure::fn>(address);
		const auto res = Structure::hook.Hook(ptr, &Structure::hooked, call_order, return_type);

		return res;
	}

	template<typename Structure>
	void set_vmt_hook(void* ptr, const std::string& module_name = "")
	{
		if (module_name.empty())
			Structure::hook.setup(ptr);
		else
			Structure::hook.setup(ptr, module_name.c_str());

		Structure::hook.hook_index(Structure::index, Structure::hooked);
	}

	void initialize()
	{
		sequence::hook = new recv_prop_hook(c_base_view_model::m_nSequence(), sequence::hooked);

#ifndef _DEBUG
		set_hook<dispatch_user_message>(interfaces::base_client, call_order::HookLast);
#endif

		client_mode::hook.setup(interfaces::client_mode, xorstr_("client.dll"));
		client_mode::hook.hook_index(client_mode::override_view::index, client_mode::override_view::hooked);
		client_mode::hook.hook_index(client_mode::create_move_shared::index, client_mode::create_move_shared::hooked);
		client_mode::hook.hook_index(client_mode::post_screen_effects::index, client_mode::post_screen_effects::hooked);

		d3d9::hook.setup(interfaces::d3_device);
		d3d9::hook.hook_index(d3d9::reset::index, d3d9::reset::hooked);
		d3d9::hook.hook_index(d3d9::end_scene::index, d3d9::end_scene::hooked);
		d3d9::hook.hook_index(d3d9::draw_indexed_primitive::index, d3d9::draw_indexed_primitive::hooked);

		vgui_panel::hook.setup(interfaces::vgui_panel, xorstr_("vgui2.dll"));
		vgui_panel::hook.hook_index(vgui_panel::paint_traverse::index, vgui_panel::paint_traverse::hooked);

		sound_hook::hook.setup(interfaces::engine_sound, xorstr_("engine.dll"));
		sound_hook::hook.hook_index(sound_hook::emit_sound1::index, sound_hook::emit_sound1::hooked);

		//mdlrender::hook.setup(interfaces::mdl_render, xorstr_("client_panorama.dll"));
		mdlrender::hook.setup(interfaces::g_studiorender);
		mdlrender::hook.hook_index(mdlrender::draw_model_execute::index, mdlrender::draw_model_execute::hooked);

		renderview::hook.setup(interfaces::render_view, xorstr_("engine.dll"));
		renderview::hook.hook_index(renderview::scene_end::index, renderview::scene_end::hooked);

		events::hook.setup(interfaces::game_events, xorstr_("engine.dll"));
		events::hook.hook_index(events::fire_event::index, events::fire_event::hooked);

		SL::hook.setup(interfaces::g_SpatialPartition);
		SL::hook.hook_index(SL::SuppressList::index, SL::SuppressList::hooked);

		//set_vmt_hook<fire_bullets>(interfaces::fire_bullets, xorstr_("client.dll"));
		//set_vmt_hook<retrieve_message>(interfaces::game_coordinator);

		event_listener = new c_game_event_listener();
		interfaces::game_events->add_listener(event_listener, xorstr_("game_newmap"), false);
		interfaces::game_events->add_listener(event_listener, xorstr_("switch_team"), false);
		interfaces::game_events->add_listener(event_listener, xorstr_("player_hurt"), false);
		interfaces::game_events->add_listener(event_listener, xorstr_("bullet_impact"), false);
		interfaces::game_events->add_listener(event_listener, xorstr_("item_purchase"), false);
		interfaces::game_events->add_listener(event_listener, xorstr_("player_spawned"), false);
		interfaces::game_events->add_listener(event_listener, xorstr_("cs_pre_restart"), false);
		interfaces::game_events->add_listener(event_listener, xorstr_("round_freeze_end"), false);
		interfaces::game_events->add_listener(event_listener, xorstr_("announce_phase_end"), false);
		interfaces::game_events->add_listener(event_listener, xorstr_("round_start"), false);
	}

	void destroy()
	{
		interfaces::game_events->remove_listener(event_listener);
		retrieve_message::hook.unhook_all();
		fire_bullets::hook.unhook_all();
		d3d9::hook.unhook_all();
		client_mode::hook.unhook_all();
		vgui_panel::hook.unhook_all();
		sound_hook::hook.unhook_all();
		mdlrender::hook.unhook_all();
		renderview::hook.unhook_all();
		events::hook.unhook_all();
		SL::hook.unhook_all();

		delete sequence::hook;
	}

	void __stdcall client_mode::override_view::hooked(CViewSetup* view)
	{
		static const auto original = hook.get_original<fn>(index);

		no_flash::handle();
		no_smoke::handle();
		clantag::animate();
		color_modulation::handle();

		features::thirdperson();

		if (!interfaces::engine_client->IsConnected() || !interfaces::engine_client->IsInGame())
			return original(interfaces::client_mode, view);

		if (!globals::view_matrix::has_offset)
		{
			globals::view_matrix::has_offset = true;
			globals::view_matrix::offset = (reinterpret_cast<DWORD>(&interfaces::engine_client->WorldToScreenMatrix()) + 0x40);
		}

		if (!g::local_player->m_bIsScoped())
			view->fov = settings::misc::debug_fov;

		//view->fov = settings::misc::debug_fov;

		original(interfaces::client_mode, view);
	}

	void __stdcall SL::SuppressList::hooked(int a2, bool a3) {
		static auto ofunc = hook.get_original<fn>(index);

		static auto OnRenderStart_Return = utils::pattern_scan(("client_panorama.dll"), "FF 50 40 8B 1D ? ? ? ?") + 0x3;
		static auto FrameNetUpdateEnd_Return = utils::pattern_scan(("client_panorama.dll"), "5F 5E 5D C2 04 00 83 3D ? ? ? ? ?");

		if (g::local_player && g::local_player->IsAlive()) {
			if (_ReturnAddress() == OnRenderStart_Return) {
				static auto set_abs_angles = utils::pattern_scan(("client_panorama.dll"), "55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1 E8");
				reinterpret_cast<void(__thiscall*)(void*, const QAngle&)>(set_abs_angles)(g::local_player, QAngle(0.0f, g_AnimState.m_flGoalFeetYaw, 0.0f));
			}
		}

		ofunc(g::g_SpatialPartition, a2, a3);
	}

	void __stdcall vgui_panel::paint_traverse::hooked(vgui::VPANEL panel, bool forceRepaint, bool allowForce)
	{
		static auto panelId = vgui::VPANEL{ 0 };
		static const auto original = hook.get_original<fn>(index);

		if (g::engine_client->IsInGame() && g::engine_client->IsConnected() && settings::misc::noscope && !strcmp("HudZoom", interfaces::vgui_panel->GetName(panel)))
			return;

		if (settings::visuals::rcs_cross)
			visuals::RenderPunchCross();

		if (settings::visuals::hitmarker)
			visuals::RenderHitmarker();

		if (settings::misc::noscope)
			visuals::RenderNoScopeOverlay();

		if (settings::visuals::spread_cross)
			visuals::SpreadCircle();

		if (settings::visuals::choke)
			visuals::Choke();

		if (settings::desync::enabled2)
			visuals::AAIndicator();

		if (settings::esp::drawFov && g::engine_client->IsInGame() && g::engine_client->IsConnected())
			visuals::DrawFov();

		/*for (auto i = 1; i <= g::entity_list->GetHighestEntityIndex(); ++i)
		{
			auto entity = c_base_entity::GetEntityByIndex(i);

			if (!entity)
				continue;

			if (settings::visuals::world_grenades)  // !!! CAUSES FPS DROPS !!!
				visuals::DrawGrenade(entity);

		}*/

		for (int i = 1; i < interfaces::entity_list->GetHighestEntityIndex(); i++) {
			auto entity = reinterpret_cast<c_planted_c4*>(interfaces::entity_list->GetClientEntity(i));

			if (entity) {
				auto client_class = entity->GetClientClass();
				auto model_name = interfaces::mdl_info->GetModelName(entity->GetModel());

				if (client_class->m_ClassID == EClassId::CPlantedC4 && entity->m_bBombTicking() && !entity->m_bBombDefused()) {
					visuals::bomb_esp(entity);
				}
			}
		}

		original(interfaces::vgui_panel, panel, forceRepaint, allowForce);

		if (!panelId)
		{
			const auto panelName = interfaces::vgui_panel->GetName(panel);

			if (!strcmp(panelName, "FocusOverlayPanel"))
				panelId = panel;
		}
		else if (panelId == panel)
		{
			//Ignore 50% cuz it called very often
			static bool bSkip = false;
			bSkip = !bSkip;

			if (bSkip)
				return;

			if (g::local_player && GetAsyncKeyState(VK_TAB))
				utils::RankRevealAll();

		}
	}

	void __stdcall sound_hook::emit_sound1::hooked(IRecipientFilter& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char* pSample, float flVolume, int nSeed, float flAttenuation, int iFlags, int iPitch, const Vector* pOrigin, const Vector* pDirection, void* pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity, int unk)
	{
		static const auto original = hook.get_original<fn>(index);

		if (!strcmp(pSoundEntry, "UIPanorama.popup_accept_match_beep")) {
			static auto fnAccept = reinterpret_cast<bool(__stdcall*)(const char*)>(utils::pattern_scan(GetModuleHandleA("client_panorama.dll"), "55 8B EC 83 E4 F8 8B 4D 08 BA ? ? ? ? E8 ? ? ? ? 85 C0 75 12"));

			HWND window = FindWindow(NULL, L"Counter - Strike: Global Offensive");

			if (fnAccept) {

				fnAccept("");

				//This will flash the CSGO window on the taskbar
				//so we know a game was found (you cant hear the beep sometimes cause it auto-accepts too fast)
				FLASHWINFO fi;
				fi.cbSize = sizeof(FLASHWINFO);
				fi.hwnd = window;
				fi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
				fi.uCount = 0;
				fi.dwTimeout = 0;
				FlashWindowEx(&fi);
			}
		}

		original(interfaces::engine_sound, filter, iEntIndex, iChannel, pSoundEntry, nSoundEntryHash, pSample, flVolume, nSeed, flAttenuation, iFlags, iPitch, pOrigin, pDirection, pUtlVecOrigins, bUpdatePositions, soundtime, speakerentity, unk);
	}

	void __fastcall mdlrender::draw_model_execute::hooked(void* pEcx, void* pEdx, void* pResults, DrawModelInfo_t* pInfo, matrix3x4_t* pBoneToWorld, float* flpFlexWeights, float* flpFlexDelayedWeights, Vector& vrModelOrigin, int32_t iFlags)
	{
		static auto original = hook.get_original<DrawModelExecute>(index);

		//visuals::chams_misc(pInfo);
		bool forced_mat = !g::mdl_render->IsForcedMaterialOverride();
		if (forced_mat)
			Chams::Get().OnDrawModelExecute(pResults, pInfo, pBoneToWorld, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);

		original(pEcx, pResults, pInfo, pBoneToWorld, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);

		if (forced_mat)
			g::mdl_render->ForcedMaterialOverride(nullptr);
	}

	void __fastcall renderview::scene_end::hooked(IVRenderView*& view)
	{
		static const auto original = hook.get_original<fn>(index);
		Chams::Get().OnSceneEnd();
		visuals::more_chams();

		if (settings::chams::desync && settings::desync::enabled2)
			visuals::DesyncChams();

		original(view);

		if (!g::engine_client->IsInGame() || !g::engine_client->IsConnected())
			return;

	}

	int __stdcall client_mode::post_screen_effects::hooked(int value)
	{
		static auto original = hook.get_original<fn>(index);

		visuals::glow();

		return original(interfaces::client_mode, value);
	}

	bool __stdcall events::fire_event::hooked(IGameEvent* pEvent)
	{
		static auto original = hook.get_original<fn>(index);

		decltype(entities::m_local) m_local;

		if (!strcmp(pEvent->GetName(), "player_death") && g::engine_client->GetPlayerForUserID(pEvent->GetInt("attacker")) == g::engine_client->GetLocalPlayer()) {
			auto& weapon = g::local_player->m_hActiveWeapon();

			if (weapon && weapon->IsWeapon()) {
				auto& skin_data = skins::statrack_items[weapon->m_iItemDefinitionIndex()];
				auto& skin_data2 = skins::m_items[weapon->m_iItemDefinitionIndex()];
				if (skin_data2.enabled && skin_data2.stat_track.enabled) {
					skin_data.statrack_new.counter++;
					weapon->m_nFallbackStatTrak() = skin_data.statrack_new.counter;
					weapon->GetClientNetworkable()->PostDataUpdate(0);
					weapon->GetClientNetworkable()->OnDataChanged(0);
				}
			}
			skins::SaveStatrack();
		}

		if (!strcmp(pEvent->GetName(), "round_start"))
		{
			m_local.isBombPlantedStatus = false;
			m_local.AfterPlant = false;
		}


		/* if (Options::Misc::killTrashTalk)
		{
			if (!strcmp(pEvent->GetName(), "player_death"))
			{
				int attacker = g_EngineClient->GetPlayerForUserID(pEvent->GetInt("attacker"));
				int userid = g_EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"));
				if (attacker != userid)
				{
					if (attacker == g_EngineClient->GetLocalPlayer())
					{
						g_EngineClient->ExecuteClientCmd("say GOTIHM!");
						if (attacker = userid)
						{
							g_EngineClient->ExecuteClientCmd("say GOTIHM!");
						}
					}
				}
			}
		} */

		return original(g::game_events, pEvent);
	}
}