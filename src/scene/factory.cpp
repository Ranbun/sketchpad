// THIS SOURCE FILE IS AUTO GENERATED BY LUA SCRIPT, DO NOT MODIFY.
// THIS IS THE SIMPLE FACTORY PATTERN TO LOAD NEW SCENE AT RUNTIME.

// once you have created a new scene, rebuild the solution on the command line,
// the content of this file will be update automatically to reflect your changes.

#include "pch.h"

#include "core/log.h"
#include "scene/factory.h"
#include "scene/scene.h"
#include "examples/scene_01.h"

namespace scene::factory {

    const std::vector<std::string> titles {
        "Welcome Screen",
        "Example Scene"
    };

    Scene* LoadScene(const std::string& title) {
        if (title == "Welcome Screen") return new Scene(title);
        if (title == "Example Scene") return new Scene01(title);

        CORE_ERROR("Scene \"{0}\" is not registered in the factory ...", title);
        std::cin.get();
        exit(EXIT_FAILURE);
    }
}
