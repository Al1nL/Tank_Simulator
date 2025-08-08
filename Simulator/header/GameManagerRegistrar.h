#ifndef GAMEMANAGERREGISTRAR_H
#define GAMEMANAGERREGISTRAR_H

#include <vector>
#include <string>
#include <memory>
//#include <mutex>
#include "../../common/GameManagerRegistration.h"

class GameManagerRegistrar {
    struct GameManagerEntry {
        std::string so_name;
        GameManagerFactory factory;
        void* library_handle;

        GameManagerEntry(const std::string& name, void* handle = nullptr)
            : so_name(name), library_handle(handle) {}
    };

    std::vector<GameManagerEntry> game_managers;
    //mutable std::mutex mutex;

public:
    static GameManagerRegistrar& getInstance() {
        static GameManagerRegistrar instance;
        return instance;
    }

    void registerGameManager(const std::string& name, GameManagerFactory&& factory, void* handle = nullptr) {
        //std::lock_guard<std::mutex> lock(mutex);
        game_managers.emplace_back(name, handle);
        game_managers.back().factory = std::move(factory);
    }

    std::unique_ptr<AbstractGameManager> create(bool verbose) {
        //std::lock_guard<std::mutex> lock(mutex);
        if (game_managers.empty()) return nullptr;
        return game_managers.back().factory(verbose);
    }

    void unregisterAll() {
        //std::lock_guard<std::mutex> lock(mutex);
        game_managers.clear();
    }
};

#endif // GAMEMANAGERREGISTRAR_H