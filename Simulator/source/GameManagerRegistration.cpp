#include "../header/GameManagerRegistrar.h"

GameManagerRegistration::GameManagerRegistration(GameManagerFactory factory) {
    GameManagerRegistrar::getInstance()
        .registerGameManager("", std::move(factory));
}