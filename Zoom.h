
#ifndef CHEAT_ZOOM_H
#define CHEAT_ZOOM_H

class Zoom : public IModule {
public:
    Zoom() : IModule("Zoom", Category::VISUAL, "让自己看得更远更清楚", "望远镜") {}
    ~Zoom() {}
};

#endif //CHEAT_ZOOM_H
