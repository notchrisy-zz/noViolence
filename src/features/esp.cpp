#include "features.h"
#include "../globals.h"
#include "../render/render.h"
#include "../helpers/imdraw.h"
#include "../helpers/console.h"
#include "../helpers/entities.h"
#include "../helpers/autowall.h"

namespace esp
{
	decltype(entities::m_local) m_local;
	entities::player_data_t m_entities[MAX_PLAYERS];
	
	bool is_enabled()
	{
		if (render::menu::is_visible() || !render::fonts::visuals)
			return false;

		return interfaces::engine_client->IsInGame() && interfaces::local_player && settings::esp::enabled;
	}

	void render(ImDrawList* draw_list)
	{
		if (!is_enabled())
			return;

		if (entities::locker.try_lock())
		{
			memcpy(m_entities, entities::m_items.front().players, sizeof(m_entities));
			entities::locker.unlock();
		}

		if (entities::local_mutex.try_lock())
		{
			m_local = entities::m_local;
			entities::local_mutex.unlock();
		}

		static const auto white_color = ImGui::GetColorU32(ImVec4::White);
		static const auto smoke_color = ImGui::GetColorU32(ImVec4(1.f, 1.f, 1.f, 0.4f));
		static const auto orange_color = ImGui::GetColorU32(ImVec4::Orange);
		static const auto green_color = ImGui::GetColorU32(ImVec4::Green);
		static const auto black_color = ImGui::GetColorU32(ImVec4::Black);

		ImGui::PushFont(render::fonts::visuals);

		RECT box;
		for (const auto& data : m_entities)
		{
			if (data.index == 0 || !data.hitboxes[0][0].IsValid())
				continue;

			if (settings::esp::offscreen)
				offscreen_entities::dot(m_local.world_pos, data.hitboxes[0][0], offscreen_entities::origin_color);

			if (settings::esp::visible_only && (!data.is_visible || data.in_smoke || m_local.is_flashed))
				continue;

			{
				bool at_screen = true;
				Vector screen_points[8] = {};
				for (int i = 0; i < 8; i++)
				{
					if (!math::world2screen(data.points[i], screen_points[i]))
					{
						at_screen = false;
						break;
					}
				}

				if (!at_screen)
					continue;

				auto left = screen_points[0].x;
				auto top = screen_points[0].y;
				auto right = screen_points[0].x;
				auto bottom = screen_points[0].y;

				for (int i = 1; i < 8; i++)
				{
					if (left > screen_points[i].x)
						left = screen_points[i].x;
					if (top > screen_points[i].y)
						top = screen_points[i].y;
					if (right < screen_points[i].x)
						right = screen_points[i].x;
					if (bottom < screen_points[i].y)
						bottom = screen_points[i].y;
				}

				box = RECT{ (long)left, (long)top, (long)right, (long)bottom };
				if (box.bottom == 0 || box.left == 0 || box.right == 0 || box.top == 0)
					continue;
			}
			
			const auto visible_color = utils::to_im32(settings::esp::visible_color);
			const auto occluded_color = utils::to_im32(settings::esp::occluded_color);

			const auto width = box.right - box.left;
			const auto height = box.bottom - box.top;

			const auto box_color = data.is_dormant ? smoke_color : data.is_visible && !data.in_smoke && !m_local.is_flashed ? visible_color : occluded_color;

			if (settings::esp::names)
			{
				const auto name_size = ImGui::CalcTextSize(data.name.c_str());
				auto x = box.left + width / 2.f - name_size.x / 2.f;

				imdraw::outlined_text(
					data.name.c_str(), ImVec2(x, box.top - name_size.y), data.is_dormant ? smoke_color : white_color
				);
			}

			if (settings::esp::is_scoped && data.is_scoped && !data.is_dormant)
			{
				static const auto scoped_text_size = ImGui::CalcTextSize("scoped");

				imdraw::outlined_text("scoped", ImVec2(box.right + 2.f, box.top - scoped_text_size.y - 2.f), smoke_color);
			}

			if (settings::esp::snaplines && !data.is_dormant)
			{
				draw_list->AddLine(ImVec2(m_local.local_pos.x, m_local.local_pos.y), ImVec2(data.origin.x, data.origin.y), green_color, 1.5f);
			}

			auto render_line = [box, draw_list](const Color& color, const int& value, const int& position, const bool& with_offset = false, const int& offset_position = 0)
			{
				const auto im_color = utils::to_im32(color);

				const auto width = (float(fabs(box.right - box.left))) / 100.f * (100.f - value);
				const auto height = (float(fabs(box.bottom - box.top))) / 100.f * (100.f - value);

				//black width + offset between lines
				const auto offset = with_offset && position == offset_position ? 7.f : 0.f;
				if (position == 0)
				{
					draw_list->AddRectFilled(ImVec2(box.left - 2.f - offset, box.top), ImVec2(box.left - 7.f - offset, box.bottom), black_color);
					draw_list->AddRectFilled(ImVec2(box.left - 3.f - offset, box.top + height), ImVec2(box.left - 6.f - offset, box.bottom), im_color);
				}
				else if (position == 1)
				{
					draw_list->AddRectFilled(ImVec2(box.right + 2.f + offset, box.top), ImVec2(box.right + 7.f + offset, box.bottom), black_color);
					draw_list->AddRectFilled(ImVec2(box.right + 3.f + offset, box.top + height), ImVec2(box.right + 6.f + offset, box.bottom), im_color);
				}
				else if (position == 2)
				{
					draw_list->AddRectFilled(ImVec2(box.left, box.bottom + 2.f + offset), ImVec2(box.right, box.bottom + 7.f + offset), black_color);
					draw_list->AddRectFilled(ImVec2(box.left, box.bottom + 3.f + offset), ImVec2(box.right, box.bottom + 6.f + offset), im_color);
				}
			};

			if (settings::esp::health)
			{
				const auto green = int(data.m_iHealth * 2.55f);
				const auto red = 255 - green;

				render_line(Color(red, green, 0, 255), data.m_iHealth, settings::esp::health_position, false);
			}

			if (settings::esp::armour)
			{
				render_line(Color(0, 50, 255, 255), data.m_ArmorValue, settings::esp::armour_position, settings::esp::health, settings::esp::health_position);
			}

			if (settings::esp::weapons && !data.weapon.empty())
			{
				const auto weapon_size = ImGui::CalcTextSize(data.weapon.c_str());

				auto y_pos = box.bottom + 2.f;
				if (settings::esp::health && settings::esp::health_position == 2)
					y_pos += 7.f;

				if (settings::esp::armour && settings::esp::armour_position == 2)
					y_pos += 7.f;

				imdraw::outlined_text(
					data.weapon.c_str(), ImVec2(box.left + width / 2.f - weapon_size.x / 2.f, y_pos), data.is_dormant ? smoke_color : orange_color
				);
			}

			if (settings::esp::boxes)
			{
				static const auto thickness = 1.f;
				if (settings::esp::box_type == EBoxType::Normal)
					draw_list->AddRect(ImVec2(box.left, box.top), ImVec2(box.right, box.bottom), box_color, thickness, 15, 1.5f);
				else
				{
					static const auto delta = 5.f;

					std::pair<ImVec2, ImVec2> points[] = 
					{
						{ImVec2(box.left, box.top), ImVec2(box.left + width / delta, box.top)}, //left top
						{ImVec2(box.left, box.bottom), ImVec2(box.left + width / delta, box.bottom)}, //left bottom

						{ImVec2(box.right, box.top), ImVec2(box.right - width / delta, box.top) }, //right top
						{ImVec2(box.right, box.bottom), ImVec2(box.right - width / delta, box.bottom)}, //right bottom

						{ImVec2(box.left, box.top), ImVec2(box.left, box.top + height / delta)}, //left top-bottom
						{ImVec2(box.left, box.bottom), ImVec2(box.left, box.bottom - height / delta)}, //left bottom-top

						{ImVec2(box.right, box.top), ImVec2(box.right, box.top + height / delta)}, //right top-bottom
						{ImVec2(box.right, box.bottom), ImVec2(box.right, box.bottom - height / delta)} //right bottom-top
					};

					for (const auto& point : points)
						draw_list->AddLine(point.first, point.second, box_color, thickness);
				}
			}
		}

		ImGui::PopFont();
	}
}