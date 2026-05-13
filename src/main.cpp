/* 
    Copyright (C) 2026, Float314 and contributors

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#ifdef _WIN32
    #define NOMINMAX
    #include <windows.h>
    #include <shellapi.h>
#endif
#include "toml.hpp"

enum class Action {
    Create,
    Config,
    Help
};

constexpr auto ONLINE_GIT_REPO = "https://github.com/Float314/cxx_repo.git";

/// @brief Parses Arguments and returns action
/// @param argc int argc
/// @param argv char* argv[]
/// @return one of the options in enum class Action
Action parseAction(int argc, char* argv[]) {
    if (argc < 2)
        return Action::Help;

    std::string_view act = argv[1];

    if (act == "new") return Action::Create;
    if (act == "config") return Action::Config;
    if (act == "help" || act == "--help" || act == "-h") return Action::Help;

    return Action::Help;
}

/// @brief Clones the Repo provided
/// @param repoPath clone to the original repo
/// @param destPath path that repo to be placed (rip my englis)
/// @return 0 if command successful
int gitClone(const std::string& repoPath, const std::string& destPath) {
    std::string cmd = "git clone " + repoPath + " " + destPath;
    return std::system(cmd.c_str());
}

/// @brief Opens Configuration file
/// @param filename Filename of config file ¯\_(ツ)_/¯
/// @return 0 if successful
int openConfigFile(const std::string& filename) {
#ifdef _WIN32
    auto ret = reinterpret_cast<INT_PTR>(ShellExecuteA(nullptr, "open", filename.c_str(), nullptr, nullptr, SW_SHOWNORMAL));
    return ret > 32 ? 0 : 1;
#elif defined(__APPLE__)
    std::string cmd = "open \"" + filename + "\"";
    return std::system(cmd.c_str());
#else
    std::string cmd = "xdg-open \"" + filename + "\"";
    return std::system(cmd.c_str());
#endif // _WIN32 or __APPLE__
}

/// @brief Tries to git clone repository. If failed, tries to copy local templates
/// @param repo Path to online repo
/// @param localRepo Path to local repo
/// @param destPath destination path
/// @return 0 if successful.
int tryGitClone(const std::string& repo, const std::string& localRepo, const std::string& destPath) {
    if (gitClone(repo, destPath) == 0)
        return 0;

    if (gitClone(localRepo, destPath) == 0)
        return 0;

    std::cerr << "Git command failed! Make sure Git is installed and the local\n"
              << "repository path is correct.\n";
    return 1;
}

/// @brief Shows help for the program ¯\_(ツ)_/¯
void showHelp(const char* progName) {
    std::cout << "Usage: " << progName << " <command> [args]\n\n"
              << "Commands:\n"
              << "  new   <name>   Create a new C/C++ project\n"
              << "  config         Open the configuration file\n"
              << "  help           Show this help message\n"
              << "\n\n Licensed under the Apache License 2.0 \n"
              << "See <https://github.com/Float314/cproj-create> \n"
              << "For more details.";
}

int main(int argc, char* argv[]) {

    auto action = parseAction(argc, argv);

    switch (action) {
        case Action::Config: {
            if (openConfigFile("config_cproj_create.toml") != 0) {
                std::cerr << "Unable to open config file!\n";
                return 1;
            }
            break;
        }
        case Action::Create: {
            if (argc < 3) {
                std::cerr << "Error: missing project name.\n";
                showHelp(argv[0]);
                return 1;
            }

            auto toml = toml::parse_file("config_cproj_create.toml");
            auto repoDir = toml["config"]["templatesDirectory"].value_or("null");
            std::string localRepo(repoDir);

            if (tryGitClone(ONLINE_GIT_REPO, localRepo, argv[2]) != 0)
                return 1;
            break;
        }
        case Action::Help: {
            showHelp("cproj-create");
            return 0;
        }
    }

    return 0;
} 