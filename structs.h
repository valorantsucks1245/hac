#pragma once

const std::vector<std::string> GUN_NAMES = {
"Melee", "Classic", "Shorty", "Frenzy", "Ghost", "Sheriff", "Stinger", "Spectre", "Bucky", "Judge",
"Bulldog", "Guardian", "Phantom", "Vandal", "Marshal", "Operator", "Ares", "Odin", "Outlaw"
};

enum VALO_GUNS {
    MELEE,
    CLASSIC,
    SHORTY,
    FRENZY,
    GHOST,
    SHERIFF,
    STINGER,
    SPECTRE,
    BUCKY,
    JUDGE,
    BULLDOG,
    GUARDIAN,
    PHANTOM,
    VANDAL,
    MARSHAL,
    OPERATOR,
    ARES,
    ODIN,
    OUTLAW,
    UNKNOWN
};

struct LauncherPointers {
    uintptr_t check_pointer;
    uintptr_t end_time;
    uintptr_t config;
};


struct TriggerConfig {
    int mode = 0;
    int key = 0;
};

struct AimerConfig {
    int mode = 0;
    int key = 0;
    int min_speed = 0;
    int max_speed = 0;
    int speed_distance = 0;
};

struct FickBotConfig {
    int mode = 0;
    int key = 0;
    int min_speed = 0;
    int max_speed = 0;
    int speed_distance = 0;
};

struct WeaponConfig {
    int GUNS_SHOT_DELAYS[19] = { 0 }; // Indexed by VALO_GUNS enum
    int GUNS_RCS[19] = { 0 };         // Indexed by VALO_GUNS enum
};

struct Config {
    // general
    int enemy_color = 0;
    int debug_display = 0;
    TriggerConfig trigger_config;
    AimerConfig aimer_config;
    FickBotConfig flickbot_config;
    WeaponConfig weapon_config;
};