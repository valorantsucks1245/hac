#pragma once
#include "headers.h"

class ConfigDebugRead {
public:
    int read_cfg(Config& config_rel) {
        std::ifstream file("config.ini");
        if (!file.is_open()) {
            std::cerr << "Error: Could not open config file." << std::endl;
            return -1;
        }

        std::string line;
        std::string section = "";

        while (std::getline(file, line)) {
            if (line.empty() || line[0] == ';' || line[0] == '#')
                continue;
            if (line[0] == '[' && line[line.length() - 1] == ']') {
                section = line.substr(1, line.length() - 2);
                continue;
            }

            size_t delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos) {
                std::string key = line.substr(0, delimiterPos);
                std::string value = line.substr(delimiterPos + 1);

                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);

                size_t commentPos = value.find(';');
                if (commentPos != std::string::npos) {
                    value = value.substr(0, commentPos);
                    value.erase(value.find_last_not_of(" \t") + 1);
                }

                try {
                    int intValue = std::stoi(value);

                    if (section == "general") {
                        if (key == "enemy_color") //not implemented yet
                            config_rel.enemy_color = intValue;
                        else if (key == "debug_display") //not implemented yet
                            config_rel.debug_display = intValue;
                    }
                    else if (section == "trigger") {
                        if (key == "mode")
                            config_rel.trigger_config.mode = intValue;
                        else if (key == "key")
                            config_rel.trigger_config.key = intValue;
                    }
                    else if (section == "aimer") {
                        if (key == "mode")
                            config_rel.aimer_config.mode = intValue;
                        else if (key == "key")
                            config_rel.aimer_config.key = intValue;
                        else if (key == "min_speed")
                            config_rel.aimer_config.min_speed = intValue;
                        else if (key == "max_speed")
                            config_rel.aimer_config.max_speed = intValue;
                        else if (key == "speed_distance")
                            config_rel.aimer_config.speed_distance = intValue;
                    }
                    else if (section == "flickbot") { //not implemented yet
                        if (key == "mode")
                            config_rel.flickbot_config.mode = intValue;
                        else if (key == "key")
                            config_rel.flickbot_config.key = intValue;
                        else if (key == "min_speed")
                            config_rel.flickbot_config.min_speed = intValue;
                        else if (key == "max_speed")
                            config_rel.flickbot_config.max_speed = intValue;
                        else if (key == "speed_distance")
                            config_rel.flickbot_config.speed_distance = intValue;
                    }
                    else if (section == "weapons") {
                        size_t typeDelimiter = key.find('_');
                        if (typeDelimiter != std::string::npos) {
                            std::string type = key.substr(0, typeDelimiter);
                            std::string weapon = key.substr(typeDelimiter + 1);

                            int weaponIndex = -1;
                            if (weapon == "MELEE") weaponIndex = MELEE;
                            else if (weapon == "CLASSIC") weaponIndex = CLASSIC;
                            else if (weapon == "SHORTY") weaponIndex = SHORTY;
                            else if (weapon == "FRENZY") weaponIndex = FRENZY;
                            else if (weapon == "GHOST") weaponIndex = GHOST;
                            else if (weapon == "SHERIFF") weaponIndex = SHERIFF;
                            else if (weapon == "STINGER") weaponIndex = STINGER;
                            else if (weapon == "SPECTRE") weaponIndex = SPECTRE;
                            else if (weapon == "BUCKY") weaponIndex = BUCKY;
                            else if (weapon == "JUDGE") weaponIndex = JUDGE;
                            else if (weapon == "BULLDOG") weaponIndex = BULLDOG;
                            else if (weapon == "GUARDIAN") weaponIndex = GUARDIAN;
                            else if (weapon == "PHANTOM") weaponIndex = PHANTOM;
                            else if (weapon == "VANDAL") weaponIndex = VANDAL;
                            else if (weapon == "MARSHAL") weaponIndex = MARSHAL;
                            else if (weapon == "OPERATOR") weaponIndex = OPERATOR;
                            else if (weapon == "ARES") weaponIndex = ARES;
                            else if (weapon == "ODIN") weaponIndex = ODIN;
                            else if (weapon == "OUTLAW") weaponIndex = OUTLAW;

                            if (weaponIndex != -1) {
                                if (type == "DELAY") {
                                    config_rel.weapon_config.GUNS_SHOT_DELAYS[weaponIndex] = intValue;
                                }
                                else if (type == "RCS") { //not imlemented yet
                                    config_rel.weapon_config.GUNS_RCS[weaponIndex] = intValue;
                                }
                            }
                        }
                    }
                }
                catch (const std::exception& e) {
                    std::cerr << "error parsing config value: " << value << " - " << e.what() << std::endl;
                }
            }
        }
        file.close();
        return 0;
    }

    void run_cfg_loop(Config& config_rel) {
        while (true) {
            read_cfg(config_rel);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }
};