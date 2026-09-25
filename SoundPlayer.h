
#ifndef CHEAT_SOUNDPLAYER_H
#define CHEAT_SOUNDPLAYER_H

#include "../../ModuleManager.h"
#include "../../Module.h"

class SoundPlayer : public IModule {
private:
    int sound = 1;

public:
    SoundPlayer() : IModule("SoundPlayer", Category::PLAYER, "在每个玩家周围播放音效", "音效播放器") {
        IntValue("Sound", &sound, sound, 0, 422, "音效");
    } ~SoundPlayer() {}

    virtual void update() override {
        shouldDisplayMode = true;
        modeString = "Sound: " + std::to_string(sound);
    }

    virtual void onTick(GameMode *gm) override {
        LocalPlayer *lp = ClientInstance::get()->getLocalPlayer();

        if (lp == nullptr) return;
        if (lp->level == nullptr) return;

        lp->dimension->forEachPlayer([&](Player& object) {
            if (&object != lp && object.getRuntimeId() != lp->getRuntimeId() && object.getUniqueId() != lp->getUniqueId()) {
                if (TargetUtils::isValidTarget(&object)) {
                    if (ClientInstance::get()->getLocalPlayer()->distanceTo(&object) <= 300) {
                        Vector3f objPos = object.getPos();
                        LevelSoundEvent levelSoundEvent(sound, objPos, -1, "minecraft:player", false, false);
                        ClientInstance::get()->getPacketSender()->sendData(levelSoundEvent);
                    }
                }
            }
            return true;
        });
    }
};

#endif //CHEAT_SOUNDPLAYER_H