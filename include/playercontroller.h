#pragma once

#include "glm/glm.hpp"

struct PlayerController
{
    enum class ForwardControl {
        Idle,
        Forward,
        Backward
    };

    enum class SidewaysControl {
        Idle,
        Left,
        Right
    };

    ForwardControl forwardCtrl;
    SidewaysControl sidewaysCtrl;
    bool jumping;
};