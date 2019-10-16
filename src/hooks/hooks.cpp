#include "hooks.h"
#include "../globals.h"
#include "../render/render.h"
#include "../helpers/utils.h"
#include "../helpers/console.h"
#include "../helpers/entities.h"
#include "../helpers/notifies.h"
#include "../features/features.h"
#include "c_game_event_listener.h"

using namespace blackbone; 

namespace hooks
{
	vfunc_hook d3d9::hook;
	vfunc_hook client_mode::hook;

	c_game_event_listener* event_listener;

	template<typename Structure>
	bool set_hook(void* _interface, call_order call_order = call_order::HookFirst, return_method return_type = return_method::UseOriginal)
	{
		const auto address = (*reinterpret_cast<uintptr_t**>(_interface))[Structure::index];
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

		d3d9::hook.setup(interfaces::d3_device);
		d3d9::hook.hook_index(d3d9::reset::index, d3d9::reset::hooked);
		d3d9::hook.hook_index(d3d9::end_scene::index, d3d9::end_scene::hooked);
		d3d9::hook.hook_index(d3d9::draw_indexed_primitive::index, d3d9::draw_indexed_primitive::hooked);

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
	}

	void destroy()
	{
		interfaces::game_events->remove_listener(event_listener);
		retrieve_message::hook.unhook_all();
		fire_bullets::hook.unhook_all();
		d3d9::hook.unhook_all();
		client_mode::hook.unhook_all();

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

		if (!globals::view_matrix::has_offset)
		{
			globals::view_matrix::has_offset = true;
			globals::view_matrix::offset = (reinterpret_cast<DWORD>(&interfaces::engine_client->WorldToScreenMatrix()) + 0x40);
		}

		original(interfaces::client_mode, view);
	}
}