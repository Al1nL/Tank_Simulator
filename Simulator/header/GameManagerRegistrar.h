#ifndef GAMEMANAGERREGISTRAR_H
#define GAMEMANAGERREGISTRAR_H

#include <vector>
#include <string>
#include <memory>
#include <cassert>
#include <functional>
#include <dlfcn.h>
#include "../../common/GameManagerRegistration.h"
#include "../../common/AbstractGameManager.h"

class GameManagerRegistrar {
    class GameManagerEntry {
        std::string so_name;
        GameManagerFactory factory;
        void* library_handle;

    public:
        GameManagerEntry(const std::string& name, void* handle = nullptr)
            : so_name(name), library_handle(handle) {}

        void setFactory(GameManagerFactory&& f) {
            // assert(factory == nullptr);
            factory = std::move(f);
        }
        void setHandle(void* handle) {
            library_handle = handle;
        }
        const std::string& name() const { return so_name; }

        std::unique_ptr<AbstractGameManager> create(bool verbose) const {
            return factory(verbose);
        }

        bool hasFactory() const {
            return factory != nullptr;
        }

        void* getHandle() const {
            return library_handle;
        }

        GameManagerFactory getFactory() const
        {
            return factory;
        }

        string getName() const
        {
            return so_name;
        }
    };

    std::vector<GameManagerEntry> game_managers;
    static GameManagerRegistrar registrar;

public:
    static GameManagerRegistrar& getGameManagerRegistrar();

    void createGameManagerEntry(const std::string& name, void* handle = nullptr) {
        game_managers.emplace_back(name, handle);
    }
    void setHandleToLastEntry(void* handle) {
        game_managers.back().setHandle(handle);
    }
    void addFactoryToLastEntry(GameManagerFactory&& factory) {
        game_managers.back().setFactory(std::move(factory));
    }

    struct BadRegistrationException {
        std::string name;
        bool hasName, hasFactory;
    };

    void validateLastRegistration() {
        const auto& last = game_managers.back();
        bool hasName = (last.name() != "");
        if (!hasName || !last.hasFactory()) {
            throw BadRegistrationException{
                .name = last.name(),
                .hasName = hasName,
                .hasFactory = last.hasFactory()
            };
        }
    }

    void removeLast() {
        game_managers.pop_back();
    }

    auto begin() const {
        return game_managers.begin();
    }

    auto end() const {
        return game_managers.end();
    }

    std::size_t count() const {
        return game_managers.size();
    }

    // void clear() {
    //     game_managers.clear();
    // }

    void cleanup() {
        for (auto& entry : game_managers) {
            entry.setFactory(GameManagerFactory{}); // assign empty std::function

            if (entry.getHandle()) {
                dlclose(entry.getHandle());
                entry.setHandle(nullptr);
            }
        }
        game_managers.clear();
    }

    GameManagerEntry getGameManager(int i) const
    {
        return game_managers.at(i);
    }
};

#endif // GAMEMANAGERREGISTRAR_H