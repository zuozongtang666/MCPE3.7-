
#ifndef CHEAT_SLOWSWING_H
#define CHEAT_SLOWSWING_H

#include "../../ModuleManager.h"
#include "../../Module.h"
#include "../Player/Swing.h"

class SlowSwing : public IModule {
public:
    float swingSpeed = 8.f;

    SlowSwing() : IModule("SlowSwing", Category::VISUAL, "改变你的挥手速度", "缓慢挥手") {
        FloatValue("Speed", &swingSpeed, swingSpeed, 0.1f, 20.f, "速度");
    } ~SlowSwing() {}

    void update() override {
        shouldDisplayMode = true;
        modeString = "Speed: " + Utils::floatToString(swingSpeed);
    }
};

#endif //CHEAT_SLOWSWING_H
