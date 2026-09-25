
#ifndef CHEAT_SHOULDERCAM_H
#define CHEAT_SHOULDERCAM_H

class ShoulderCam : public IModule {
private:
    float offset = 1.3;
public:
    ShoulderCam() : IModule("ShoulderCam", Category::VISUAL, "类似于电影中的越肩视角", "越肩视角") {
        FloatValue("Offset", &offset, offset, 0, 4, "偏移");
    } ~ShoulderCam() {}

    void update() override {
        shouldDisplayMode = true;
        modeString = "Offset: " + Utils::floatToString(offset);
    }

    void onTick(GameMode *gm) override {
        //Camera::getCamera()->setOffset(Vector3f(offset, offset, offset));
    }

    void onDisable() override {
        //Camera::getCamera()->setOffset(Vector3f(0, 0, 0));
    }
};

#endif //CHEAT_SHOULDERCAM_H