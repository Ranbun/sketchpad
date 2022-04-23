#pragma once

// THIS HEADER FILE IS AUTO GENERATED BY LUA SCRIPT, DO NOT TOUCH
// THIS IS A SIMPLE FACTORY PATTERN TO LOAD NEW SCENES AT RUNTIME

// once you have made a new scene, rebuild the solution on the command line
// the content of this file will be automatic updated to reflect the change

#include <iostream>
#include <string>
#include <vector>
#include "core/log.h"
#include "scene/scene.h"

#include "example/scene_01.h"
#include "example/scene_02.h"
#include "example/scene_03.h"
#include "example/scene_04.h"
#include "example/scene_05.h"
#include "example/scene_06.h"

namespace scene::factory {

    inline const std::vector<std::string> titles {
        "Welcome Screen",
        "Tiled Forward Renderer",
        "Environment Lighting (IBL)",
        "Disney Principled BSDF",
        "Compute Shader Cloth Simulation",
        "Animation and Realtime Shadows",
        "Bezier Area Lights with LTC"
    };

    inline Scene* LoadScene(const std::string& title) {
        if (title == "Welcome Screen") return new Scene(title);
        if (title == "Tiled Forward Renderer") return new Scene01(title);
        if (title == "Environment Lighting (IBL)") return new Scene02(title);
        if (title == "Disney Principled BSDF") return new Scene03(title);
        if (title == "Compute Shader Cloth Simulation") return new Scene04(title);
        if (title == "Animation and Realtime Shadows") return new Scene05(title);
        if (title == "Bezier Area Lights with LTC") return new Scene06(title);

        CORE_ERROR("Scene \"{0}\" is not registered in the factory ...", title);
        std::cin.get();
        exit(EXIT_FAILURE);
    }

}