
#ifndef CHEAT_TRACER_H
#define CHEAT_TRACER_H

class Tracer : public IModule {
public:
    Color color = Color(255, 255, 255);
    bool rainbow = true;

    Tracer() : IModule("Tracer", Category::VISUAL, "为你绘制指向目标的直线", "实体连线") {
        BoolValue("Rainbow", &rainbow, rainbow, "彩虹");
    }
    ~Tracer() {}

    void onRender() override {
        if (!ClientInstance::get()->getLocalPlayer()) return;
        if (!ClientInstance::get()->getLocalPlayer()->level) return;
        if (!ClientInstance::get()->getMatrix()) return;

        if (rainbow) {
            color = DrawUtils::getRainbowColor();
        } else {
            color = Color(255, 255, 255);
        }

        ClientInstance::get()->getLocalPlayer()->dimension->forEachPlayer([&](Player& player) {
            if (TargetUtils::isValidTarget(&player)) {
                Vector2f target;
                Vector2f screenSize = GuiData::get()->windowSize;
                DrawUtils::worldToScreen(player.eyePos, target);
                Vector2f mid((screenSize.x) / 2, (screenSize.y) / 2);
                if (target != Vector2f(0, 0)) {
                    DrawUtils::drawLine(mid.x, mid.y, target.x, target.y, 0.3f, 255, color.red, color.green, color.blue);
                }
            }
            return true;
        });
    }
};

#endif //CHEAT_TRACER_H
