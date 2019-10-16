#include "../render.h"
#include "../../globals.h"
#include "../../settings.h"
#include "../../helpers/notifies.h"

extern void bind_button(const char* eng, const char* rus, int& key);
extern bool hotkey(const char* label, int* k, const ImVec2& size_arg = ImVec2(0.f, 0.f));

namespace render
{
	namespace menu
	{
		extern std::map<int, weapon_type_t> get_weapons(bool need_knife);
		extern std::map<int, const char*> get_groups(bool need_knife = false, bool need_groups = false);
		extern bool selectable_weapons(
			int& selected_item,
			bool only_groups,
			std::string& weaponName,
			std::map<int, const char*> groups,
			std::map<int, weapon_type_t> k_item_names,
			std::vector<int> selected_weapons = { }
		);

		extern bool listbox_group_weapons(
			int& selected_item,
			std::map<int, const char*> groups,
			std::map<int, weapon_type_t> items,
			ImVec2 listbox_size,
			bool show_only_selected = false,
			std::vector<int> selected_weapons = { }
		);

		void aimbot_tab()
		{
			static int definition_index = 7;
			auto settings = &settings::aimbot::m_items[definition_index];

			child(___("Main", u8"Основные настройки"), [&settings]()
			{
				auto k_item_names = get_weapons(false);

				const char* setting_types[] =
				{
					___("Separately", u8"Раздельно"),
					___("Subgroups", u8"Подразделы"),
					___("For all", u8"Все оружие"),
					___("Groups", u8"Разделы")
				};

				static bool is_settings_visible = false;

				if (ImGui::Combo("##setting_type", &settings::aimbot::setting_type, setting_types, IM_ARRAYSIZE(setting_types)))
					is_settings_visible = settings::aimbot::setting_type == settings_type_t::groups ? false : true;

				if (settings::aimbot::setting_type != settings_type_t::groups)
					is_settings_visible = true;

				if (settings::aimbot::setting_type == settings_type_t::for_all)
				{
					definition_index = 0;
				}
				else if (settings::aimbot::setting_type == settings_type_t::separately || settings::aimbot::setting_type == settings_type_t::subgroups)
				{
					auto groups = get_groups(false, true);

					std::string weaponName;
					if (settings::aimbot::setting_type == settings_type_t::subgroups)
					{
						if (groups.count(definition_index) == 0)
							definition_index = WEAPONTYPE_PISTOL;

						weaponName = groups[definition_index];
					}
					else
					{
						if (k_item_names.count(definition_index) == 0)
							definition_index = WEAPON_AK47;

						weaponName = k_item_names[definition_index].name;
					}

					selectable_weapons(definition_index, settings::aimbot::setting_type == settings_type_t::subgroups, weaponName, groups, k_item_names);
				}
				else if (settings::aimbot::setting_type == settings_type_t::groups)
				{
					if (definition_index < 0 || definition_index >= settings::aimbot::m_groups.size())
					{
						definition_index = 0;
					}

					if (settings::aimbot::m_groups.size() > 0)
					{
						ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() - 70.f);
						ImGui::Combo("##aimbot.groups", &definition_index, [](void* data, int idx, const char** out_text)
						{
							*out_text = settings::aimbot::m_groups[idx].name.c_str();
							return true;
						}, nullptr, settings::aimbot::m_groups.size(), 10);
						ImGui::PopItemWidth();

						ImGui::SameLine();

						if (is_settings_visible)
						{
							if (ImGui::Button(___("Edit", u8"Изменить"), ImVec2(ImGui::GetContentRegionAvailWidth(), 0.f)))
								is_settings_visible = false;
						}
						else
						{
							if (ImGui::Button(___("Hide", u8"Скрыть"), ImVec2(ImGui::GetContentRegionAvailWidth(), 0.f)))
								is_settings_visible = true;
						}
					}

					if (!is_settings_visible)
					{
						separator(___("New group", u8"Новая группа"));

						ImGui::Text(___("Name", u8"Имя"));

						static char group_name[32];
						ImGui::InputText("##aimbot.group_name", group_name, sizeof(group_name));

						if (ImGui::Button(___("Create", u8"Добавить")))
						{
							if (strlen(group_name) == 0)
								notifies::push(___("Enter the group name", u8"Укажите имя группы"), notify_state_s::danger_state);
							else
							{
								settings::aimbot::m_groups.emplace_back(aimbot_group{ std::string(group_name), { } });

								memset(group_name, 0, sizeof(group_name));
								notifies::push(___("Group created", u8"Группа создана"));

								definition_index = settings::aimbot::m_groups.size() - 1;
							}
						}

						if (settings::aimbot::m_groups.empty())
							return;

						separator(___("Current group", u8"Текущая группа"));

						auto& current_group = settings::aimbot::m_groups[definition_index];

						static auto weapon_to_select = -1;
						std::string placeholder = ___("Select weapon", u8"Выберите предметы");

						const auto groups = get_groups(false, false);

						if (selectable_weapons(weapon_to_select, false, placeholder, groups, k_item_names, current_group.weapons))
						{
							if (std::find(current_group.weapons.begin(), current_group.weapons.end(), weapon_to_select) == current_group.weapons.end())
								current_group.weapons.emplace_back(weapon_to_select);

							weapon_to_select = -1;
						}

						//ImGui::Text(___("Press for remove weapon", u8"Нажмите для удаления"));

						static int weapon_to_remove = -1;
						//ImVec2(ImGui::GetContentRegionAvailWidth(), 150.f)
						if (listbox_group_weapons(weapon_to_remove, groups, k_item_names, ImVec2(0, 150.f), true, current_group.weapons))
						{
							current_group.weapons.erase(std::find(current_group.weapons.begin(), current_group.weapons.end(), weapon_to_remove));

							weapon_to_remove = -1;
						}

						if (ImGui::Button(___("Delete", u8"Удалить")))
						{
							notifies::push(___("Group removed", u8"Группа удалена"));

							settings::aimbot::m_groups.erase(settings::aimbot::m_groups.begin() + definition_index);

							definition_index = 0;
						}

						return;
					}
				}

				checkbox("Enabled", u8"Включено", &settings->enabled);
				checkbox("Humanize", u8"Смягчать", &settings->recoil.humanize);

				if (settings::aimbot::setting_type == settings_type_t::separately)
				{
					switch (definition_index)
					{
					case WEAPON_P250:
					case WEAPON_USP_SILENCER:
					case WEAPON_GLOCK:
					case WEAPON_FIVESEVEN:
					case WEAPON_TEC9:
					case WEAPON_DEAGLE:
					case WEAPON_ELITE:
					case WEAPON_HKP2000:
					case 201:
						checkbox("Auto Pistol", u8"Автом. пистолеты", &settings->autopistol);
					default:
						break;
					}
				}
				else if (settings::aimbot::setting_type == settings_type_t::subgroups && (definition_index == WEAPONTYPE_PISTOL || definition_index == 201))
					checkbox("Auto Pistol", u8"Автом. пистолеты", &settings->autopistol);
				else
					checkbox("Auto Pistol", u8"Автом. пистолеты", &settings->autopistol);

				checkbox("Air Check", u8"Не наводиться в прыжке", &settings->check_air);
				checkbox("Flash Check", u8"Не наводиться ослепленным", &settings->check_flash);
				checkbox("Smoke Check", u8"Не наводиться в дымовые", &settings->check_smoke);

				if (settings::aimbot::setting_type == settings_type_t::separately)
				{
					if (utils::is_sniper(definition_index))
						checkbox("Zoom Check", u8"Наводиться только в зуме", &settings->check_zoom);
				}
				else if (settings::aimbot::setting_type == settings_type_t::subgroups)
				{
					if (definition_index == 240 || definition_index == 209 || definition_index == WEAPONTYPE_SNIPER_RIFLE)
						checkbox("Zoom Check", u8"Наводиться только в зуме", &settings->check_zoom);
				}
				else if (settings::aimbot::setting_type == settings_type_t::groups)
					checkbox("Zoom Check", u8"Наводиться только в зуме", &settings->check_zoom);

				columns(2);
				{
					checkbox("By Damage", u8"По дамагу", &settings->by_damage);

					ImGui::NextColumn();

					ImGui::PushItemWidth(-1);
					ImGui::SliderIntLeftAligned(___("Min:##min_damage", u8"От:##min_damage"), &settings->min_damage, 1, 100, "%.0f HP");
					ImGui::PopItemWidth();
				}
				columns(1);

				columns(2);
				{
					checkbox("Auto Wall", u8"Игнор стен", &settings->autowall.enabled);

					ImGui::NextColumn();

					ImGui::PushItemWidth(-1);
					ImGui::SliderIntLeftAligned(___("Min:##autowall", u8"От:##autowall"), &settings->autowall.min_damage, 1, 100, "%.0f HP");
					ImGui::PopItemWidth();
				}
				columns(1);

				separator(___("Delays", u8"Задержки"));

				if (!settings->silent.enabled)
				{
					checkbox("Auto Delay", u8"Автоматическая задержка", &settings->autodelay);
					ImGui::SliderIntLeftAligned(___("Shot Delay:", u8"Перед выстрелом:"), &settings->shot_delay, 0, 250, "%.0f ms");
				}
				ImGui::SliderIntLeftAligned(___("Target Switch Delay:", u8"После убийства:"), &settings->kill_delay, 0, 1000, "%.0f ms");

				if (settings::aimbot::setting_type != 0)
					return;

				if (!utils::is_connected())
					return;

				auto weapon = interfaces::local_player->m_hActiveWeapon();
				if (!weapon || !weapon->IsWeapon())
					return;

				const auto item_definition_index = weapon->m_iItemDefinitionIndex();
				if (k_item_names.count(item_definition_index) == 0)
					return;

				ImGui::Separator();

				if (ImGui::Button(___("Current", u8"Текущее")))
					definition_index = item_definition_index;
			});

			ImGui::NextColumn();

			child(___("Silent", u8"Невидимая доводка"), [&settings]()
			{
				checkbox("Enabled##silent", u8"Включено##silent", &settings->silent.enabled);
				checkbox("Always (?)", u8"Всегда (?)", &settings->silent.always);
				tooltip("Silent will work on every shot.", u8"Будет работать на каждом выстреле.");
				checkbox("Use With Smooth", u8"Совмещать с доводкой", &settings->silent.with_smooth);

				if (settings->silent.with_smooth && settings->fov < settings->silent.fov)
					settings->silent.fov = settings->fov;

				ImGui::SliderFloatLeftAligned(___("FOV:", u8"Радиус:"), &settings->silent.fov, 0, settings->silent.with_smooth ? settings->fov : 15.f);
				//ImGui::SliderInt("##silent.chanse", &settings->silent.chanse, 1, 10, ___("Chanse: %.0f", u8"Шанс срабатывания: %.0f"));

				separator(___("Other", u8"Прочее"));

				ImGui::PushID("aimbot.other");
				{
					checkbox("Dynamic FOV", u8"Динамический радиус", &settings->dynamic_fov);
					checkbox("Legit Compensation", &settings->backtrack.legit);

					ImGui::SliderFloatLeftAligned(___("FOV:", u8"Радиус:"), &settings->fov, 0, 15.f);
					ImGui::SliderFloatLeftAligned(___("Smooth:", u8"Плавность:"), &settings->smooth, 1, 10.f);

					ImGui::SliderIntLeftAligned(___("Lag Compensation:", u8"Лаг компенсация:"), &settings->backtrack.ticks, 1, 12, ___("%.0f ticks", u8"%.0f тиков"));

					ImGui::SliderIntLeftAligned(___("Hit Chance:", u8"Мин шанс попадания:"), &settings->min_hitchanse, 0, 100, "%.0f%%");
				}
				ImGui::PopID();

				columns(3);
				{
					checkbox("Head", u8"Голова", &settings->hitboxes.head); 
					checkbox("Hands", u8"Руки", &settings->hitboxes.hands);

					ImGui::NextColumn();

					checkbox("Neck", u8"Шея", &settings->hitboxes.neck);
					checkbox("Legs", u8"Ноги", &settings->hitboxes.legs);

					ImGui::NextColumn();

					checkbox("Body", u8"Живот", &settings->hitboxes.body);
				}
				columns(1);
			});

			ImGui::NextColumn();

			child(___("Recoil Control System", u8"Спрей"), [&settings]()
			{
				checkbox("Enabled", u8"Включено", &settings->recoil.enabled);
				checkbox("Standalone", u8"Постоянный", &settings->recoil.standalone);
				checkbox("First Bullet", u8"С первой пули", &settings->recoil.first_bullet);

				ImGui::SliderFloatLeftAligned("Pitch:", &settings->recoil.pitch, 0, 2);
				ImGui::SliderFloatLeftAligned("Yaw:", &settings->recoil.yaw, 0, 2);
				
				separator(___("Trigger", u8"Авто выстрел"));

				columns(2);
				{
					checkbox("Enabled##trigger", u8"Включено##trigger", &settings->trigger.enabled);

					ImGui::NextColumn();

					ImGui::PushItemWidth(-1);
					hotkey("##binds.trigger", &globals::binds::trigger);
					ImGui::PopItemWidth();
				}
				columns(1);

				bind_button("Back Shot", "Back Shot", globals::binds::back_shot);

				ImGui::SliderIntLeftAligned(___("Reaction time:", u8"Перед выстрелом:"), &settings->trigger.delay, 0, 250, "%.0f ms");
				ImGui::SliderIntLeftAligned(___("Delay Between Shots:", u8"Между выстрелами:"), &settings->trigger.delay_btw_shots, 0, 250, "%.0f ms");
				ImGui::SliderIntLeftAligned(___("Hit Chance:", u8"Мин шанс попадания:"), &settings->trigger.hitchance, 1, 100, "%.0f%%");

			});
		}
	}
}