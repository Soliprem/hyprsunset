#include "ConfigManager.hpp"
#include "src/helpers/Log.hpp"

static void printHelp() {
    Debug::log(NONE, "┣ --gamma             -g  →  Set the display gamma (default 100%)");
    Debug::log(NONE, "┣ --gamma_max             →  Set the maximum display gamma (default 100%, maximum 200%)");
    Debug::log(NONE, "┣ --temperature       -t  →  Set the temperature in K (default 6000)");
    Debug::log(NONE, "┣ --identity          -i  →  Use the identity matrix (no color change)");
    Debug::log(NONE, "┣ --verbose               →  Print more logging");
    Debug::log(NONE, "┣ --version           -v  →  Print the version");
    Debug::log(NONE, "┣ --help              -h  →  Print this info");
    Debug::log(NONE, "╹");
}

int main(int argc, char** argv, char** envp) {
    std::string configPath;

    int         kelvin   = -1;
    float       gamma    = -1;
    float       maxGamma = -1;
    bool        identity = false;

    g_pHyprsunset = std::make_unique<CHyprsunset>();

    for (int i = 1; i < argc; ++i) {
        if (argv[i] == std::string{"-t"} || argv[i] == std::string{"--temperature"}) {
            if (i + 1 >= argc) {
                Debug::log(NONE, "✖ No temperature provided for {}", argv[i]);
                return 1;
            }

            try {
                kelvin = std::stoull(argv[i + 1]);
            } catch (std::exception& e) {
                Debug::log(NONE, "✖ Temperature {} is not valid", argv[i + 1]);
                return 1;
            }

            ++i;
        } else if (argv[i] == std::string{"-g"} || argv[i] == std::string{"--gamma"}) {
            if (i + 1 >= argc) {
                Debug::log(NONE, "✖ No gamma provided for {}", argv[i]);
                return 1;
            }

            try {
                gamma = std::stof(argv[i + 1]) / 100;
            } catch (std::exception& e) {
                Debug::log(NONE, "✖ Gamma {} is not valid", argv[i + 1]);
                return 1;
            }

            ++i;
        } else if (argv[i] == std::string{"--gamma_max"}) {
            if (i + 1 >= argc) {
                Debug::log(NONE, "✖ No gamma provided for {}", argv[i]);
                return 1;
            }

            try {
                maxGamma = std::stof(argv[i + 1]) / 100;
            } catch (std::exception& e) {
                Debug::log(NONE, "✖ Maximum gamma {} is not valid", argv[i + 1]);
                return 1;
            }

            ++i;
        } else if (argv[i] == std::string{"-i"} || argv[i] == std::string{"--identity"}) {
            identity = true;
        } else if (argv[i] == std::string{"-c"} || argv[i] == std::string{"--config"}) {
            if (i + 1 >= argc) {
                Debug::log(NONE, "✖ No config path provided for {}", argv[i]);
                return 1;
            }

            configPath = argv[i + 1];
        } else if (argv[i] == std::string{"-h"} || argv[i] == std::string{"--help"}) {
            printHelp();
            return 0;
        } else if (argv[i] == std::string{"-v"} || argv[i] == std::string{"--version"}) {
            Debug::log(NONE, "hyprsunset v{}", HYPRSUNSET_VERSION);
            return 0;
        } else if (argv[i] == std::string{"--verbose"}) {
            Debug::trace = true;
        } else {
            Debug::log(NONE, "✖ Argument not recognized: {}", argv[i]);
            printHelp();
            return 1;
        }
    }

    Debug::log(NONE, "┏ hyprsunset v{} ━━╸\n┃", HYPRSUNSET_VERSION);

    g_pConfigManager = makeUnique<CConfigManager>(configPath);
    g_pConfigManager->init();

    g_pHyprsunset->loadCurrentProfile();

    if (kelvin != -1) {
        g_pHyprsunset->KELVIN    = kelvin;
        g_pHyprsunset->kelvinSet = true;
        g_pHyprsunset->identity  = false;
    }

    if (gamma != -1)
        g_pHyprsunset->GAMMA = gamma;

    if (maxGamma != -1)
        g_pHyprsunset->MAX_GAMMA = maxGamma;

    if (identity)
        g_pHyprsunset->identity = true;

    if (!g_pHyprsunset->calculateMatrix())
        return 1;
    if (!g_pHyprsunset->init())
        return 1;

    return 0;
}
