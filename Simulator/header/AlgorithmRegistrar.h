#ifndef ALGORITHMREGISTRAR_H
#define ALGORITHMREGISTRAR_H
#include <dlfcn.h>
#include <vector>
#include <cassert>
#include "../../common/Player.h"
#include "../../common/TankAlgorithm.h"

class AlgorithmRegistrar
{
    class AlgorithmAndPlayerFactories
    {
        std::string so_name;
        TankAlgorithmFactory tankAlgorithmFactory = nullptr;
        PlayerFactory playerFactory;
        void *library_handle = nullptr;

    public:
        AlgorithmAndPlayerFactories(const std::string &so_name) : so_name(so_name) {}
        void setTankAlgorithmFactory(TankAlgorithmFactory &&factory)
        {
            tankAlgorithmFactory = std::move(factory);
        }
        void setPlayerFactory(PlayerFactory &&factory)
        {
            playerFactory = std::move(factory);
        }
        ~AlgorithmAndPlayerFactories() = default;
        void * getHandle() const { return library_handle; }
        void setHandle(void *handle)
        {
            if (!library_handle)
                library_handle = handle;
        }

        const std::string &name() const { return so_name; }
        
        std::unique_ptr<Player> createPlayer(int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells) const
        {
            return playerFactory(player_index, x, y, max_steps, num_shells);
        }

        std::unique_ptr<TankAlgorithm> createTankAlgorithm(int player_index, int tank_index) const
        {
            return tankAlgorithmFactory(player_index, tank_index);
        }

        bool hasPlayerFactory() const
        {
            return playerFactory != nullptr;
        }
        
        bool hasTankAlgorithmFactory() const
        {
            return tankAlgorithmFactory != nullptr;
        }

        TankAlgorithmFactory getTankAlgorithmFactory() const
        {
            return tankAlgorithmFactory;
        }

        PlayerFactory getPlayerFactory() const
        {
            return playerFactory;
        }
    };

    std::vector<AlgorithmAndPlayerFactories> algorithms;
    static AlgorithmRegistrar registrar;

public:
    static AlgorithmRegistrar &getAlgorithmRegistrar();
    void createAlgorithmFactoryEntry(const std::string &name)
    {
        algorithms.emplace_back(AlgorithmAndPlayerFactories(name));
    }

    void addPlayerFactoryToLastEntry(PlayerFactory &&factory)
    {
        algorithms.back().setPlayerFactory(std::move(factory));
    }
    void addTankAlgorithmFactoryToLastEntry(TankAlgorithmFactory &&factory)
    {
        assert(factory);
        algorithms.back().setTankAlgorithmFactory(std::move(factory));
    }
    void setHandleToLastEntry(void *handle)
    {
        algorithms.back().setHandle(handle);
    }
    struct BadRegistrationException
    {
        std::string name;
        bool hasName, hasPlayerFactory, hasTankAlgorithmFactory;
    };
    void validateLastRegistration()
    {
        const auto &last = algorithms.back();
        bool hasName = (last.name() != "");
        if (!hasName || !last.hasPlayerFactory() || !last.hasTankAlgorithmFactory())
        {
            throw BadRegistrationException{
                .name = last.name(),
                .hasName = hasName,
                .hasPlayerFactory = last.hasPlayerFactory(),
                .hasTankAlgorithmFactory = last.hasTankAlgorithmFactory()};
        }
    }
    void removeLast()
    {
        algorithms.pop_back();
    }
    auto begin() const
    {
        return algorithms.begin();
    }
    auto end() const
    {
        return algorithms.end();
    }
    std::size_t count() const { return algorithms.size(); }

    void cleanup(){
    // Phase 1: Clear all factories
        for (auto& entry : algorithms) {
            entry.setPlayerFactory(nullptr);
            entry.setTankAlgorithmFactory(nullptr);
        }
        algorithms.clear();

        for (auto algo : algorithms) {
            if (algo.getHandle() != nullptr) dlclose(algo.getHandle());
        }
        algorithms.clear();
    }

    AlgorithmAndPlayerFactories getAlgorithmAndPlayerFactory(int algo) const
    {
        return algorithms.at(algo);
    }
};
#endif // ALGORITHMREGISTRAR_H
