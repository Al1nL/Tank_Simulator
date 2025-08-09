#include "../header/GameManagerRegistrar.h"

GameManagerRegistration::GameManagerRegistration(GameManagerFactory factory) {
        auto& registrar = GameManagerRegistrar::getGameManagerRegistrar();
    registrar.addFactoryToLastEntry(std::move(factory));
}

