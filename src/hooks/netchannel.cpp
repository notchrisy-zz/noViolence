#include "hooks.h"
#include "../settings.h"
#include "../helpers/console.h"

namespace hooks
{
	void* last_ptr = nullptr;
	vfunc_hook netchannel::hook;

	void netchannel::setup()
	{
		return;
		if (!interfaces::client_state->m_NetChannel)
			return;

		if (!last_ptr || last_ptr != interfaces::client_state->m_NetChannel)
		{
			last_ptr = interfaces::client_state->m_NetChannel;
			if (hook.setup(interfaces::client_state->m_NetChannel))
			{
				//hook.hook_index(send_datagram::index, send_datagram::hooked);
				hook.hook_index(send_net_message::index, send_net_message::hooked);
			}
		}
	}

	bool __stdcall send_net_message::hooked(INetMessage& msg, bool bForceReliable, bool bVoice)
	{
		const auto original = netchannel::hook.get_original<fn>(index);

		if (settings::fake_lags::enabled && msg.GetGroup() == 9)
			bVoice = true;
		
		return original(interfaces::client_state->m_NetChannel, msg, bForceReliable, bVoice);
	}

	int __stdcall send_datagram::hooked(bf_write* datagram)
	{
		const auto original = netchannel::hook.get_original<fn>(index);

		return original(interfaces::client_state->m_NetChannel, datagram);
	}
}