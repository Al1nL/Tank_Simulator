#ifndef GAMEMANAGERREGISTRAR_H
#define GAMEMANAGERREGISTRAR_H

#include <vector>
#include <string>
#include <memory>
#include <cassert>
#include <functional>
#include <dlfcn.h>
#include <mutex>
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
            assert(factory == nullptr);
            factory = std::move(f);
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
    };

    std::vector<GameManagerEntry> game_managers;
    static GameManagerRegistrar registrar;
    mutable std::mutex mutex;

public:
    static GameManagerRegistrar& getGameManagerRegistrar();

    void createGameManagerEntry(const std::string& name, void* handle = nullptr) {
        std::lock_guard<std::mutex> lock(mutex);
        game_managers.emplace_back(name, handle);
    }

    void addFactoryToLastEntry(GameManagerFactory&& factory) {
        std::lock_guard<std::mutex> lock(mutex);
        game_managers.back().setFactory(std::move(factory));
    }

    struct BadRegistrationException {
        std::string name;
        bool hasName, hasFactory;
    };

    void validateLastRegistration() {
        std::lock_guard<std::mutex> lock(mutex);
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
        std::lock_guard<std::mutex> lock(mutex);
        game_managers.pop_back();
    }

    auto begin() const {
        std::lock_guard<std::mutex> lock(mutex);
        return game_managers.begin();
    }

    auto end() const {
        std::lock_guard<std::mutex> lock(mutex);
        return game_managers.end();
    }

    std::size_t count() const {
        std::lock_guard<std::mutex> lock(mutex);
        return game_managers.size();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex);
        game_managers.clear();
    }

    void cleanup() {
        std::lock_guard<std::mutex> lock(mutex);
        for (auto& entry : game_managers) {
            if (entry.getHandle()) {
                dlclose(entry.getHandle());
            }
        }
        game_managers.clear();
    }
};

#endif // GAMEMANAGERREGISTRAR_H