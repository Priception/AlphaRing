#include "../basic_widget.h"

#include "game/halo3/native/native.h"

class TabMap : public BasicWidget {
public:
    TabMap(const char* name) : BasicWidget(name) {}
    void render() override;
    void update() override{}

};

static TabMap s_instance("Map");
BasicWidget* tab_map = &s_instance;

void TabMap::render() {
    const char* map_types[4] = {"", "Campaign", "Multiplayer", "MainMenu"};
    char buffer[1024];

    auto map = NativeHalo3()->Map()->getMap();

    if (map == nullptr) return;

    {
        auto map_name = strrchr(map->map_info.name, '\\');

        if (map_name == nullptr) map_name = map->map_info.name;
        else map_name += 1;

        sprintf(buffer, "Map: %s (%s)", map_name, map_types[map->map_info.game_type]);

        ImGui::Text(buffer);
    }

    if (ImGui::BeginListBox("Skulls")) {
        for (int i = 0; i < sizeof(eSkullValue) / 4; ++i) {
            const char *skull_name = eSkullName[i];
            int skull_value = eSkullValue[i];

            bool status = map->get_skull((eSkull) skull_value);

            if (ImGui::Selectable(skull_name, status))
                map->set_skull((eSkull) skull_value, !status);
        }

        ImGui::EndListBox();
    }
}
