#include "features.h"
#include "../helpers/console.h"

namespace lobby_inviter
{
	uint32_t max_count = 0;
	uint32_t* collection;

	struct item_t
	{
		uint16_t idx_next_0, unk_idx_2, idx_prev_4, unk_val_6;
		uint64_t steam_id_8;
		uint32_t* unk_ptr_16;
	}; //Size: 0x0014

	void inviteUser(const uint64_t& id)
	{
		using Fn = void(__thiscall*)(uintptr_t, uint64_t);
		static const auto this_ = *(uintptr_t * *)(utils::pattern_scan("client.dll", "83 3D ? ? ? ? ? 74 5E 8B 0D ? ? ? ? 68") + 2);
		static const auto fn = (Fn)utils::pattern_scan("client.dll", "55 8B EC 83 E4 F8 83 EC 1C 53 56 57 8B F1 FF 15 ? ? ? ? 8B");
		if (*this_)
			fn(*this_, id);
	}

	void inviteAll()
	{
		if (!collection)
		{
			//fetch players
			return;
		}

		for (uint16_t i = 0; i <= max_count; ++i)
		{
			auto item = &((item_t*)* collection)[i];
			inviteUser(item->steam_id_8);
		}
	}

	void update()
	{
		if (!collection)
			collection = *(uint32_t**)(utils::pattern_scan(LOBBIES_COLLECTION) + 2);

		if (!collection)
		{
			console::print("collection is nullptr");
			return;
		}

		max_count = ((uint16_t*)collection)[9];
	}
}