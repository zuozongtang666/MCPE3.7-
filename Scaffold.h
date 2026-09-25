
#ifndef CHEAT_SCAFFOLD_H
#define CHEAT_SCAFFOLD_H

#include "../../Module.h"

class Scaffold : public IModule {
private:
    ModuleMode rotMode;
    bool autoBlock = true;
    int launchY = 0;
    bool lockY = false;
    int extend = 1;

    Vector3i calc;
    bool foundBlock = false;
    int range = 4;
    int prevSlot = 0;

    bool shouldRot = false;

public:
    Scaffold() : IModule("Scaffold", Category::MOVEMENT, "在行走时自动放置方块", "自动搭路") {
        rotMode.addMode("None");
        rotMode.addMode("Client");
        rotMode.addMode("Server");
        rotMode.addMode("Both");
        ModeValue("Rotation", &rotMode, "Client", "转头模式");
        IntValue("Extend", &extend, extend, 0, 10, "延伸/伸展");
        BoolValue("LockY", &lockY, lockY, "锁定Y");
        BoolValue("AutoBlock", &autoBlock, autoBlock, "自动方块");
    }; ~Scaffold() {};

    void update() override {
        shouldDisplayMode = true;
        modeString = "Extend: " + std::to_string(extend);
    }

    void onEnable() override {
        launchY = ClientInstance::get()->getLocalPlayer()->aabbComp->lowerPos.y - 0.55f;
    }

    bool canPlace(Vector3f pos) {
        auto block = ClientInstance::get()->getLocalPlayer()->getRegion()->getBlock(pos.floor());
        return block->blockLegacy->isReplaceable();
    }

    bool buildBlock(Vector3f pos) {
        static std::vector<Vector3f> neighbours = {
                Vector3f(0, -1, 0), Vector3f(0, 1, 0),
                Vector3f(0, 0, -1), Vector3f(0, 0, 1),
                Vector3f(-1, 0, 0), Vector3f(1, 0, 0),
        };

        auto gm = ClientInstance::get()->getLocalPlayer()->getGameMode();

        for (int face = 0; face < neighbours.size(); face++) {
            Vector3f offset = Vector3f(pos.floor()).sub(neighbours[face]);
            if (!canPlace(offset)) {
                gm->useItem(ClientInstance::get()->getLocalPlayer()->getSelectedItem());
                gm->startBuildBlock(offset, face, false);
                gm->buildBlock(offset, face, true);
                return true;
            }
        }
        return false;
    }

    bool predictBlock(Vector3f pos) {
        static std::vector<Vector3f> blocks;
        static float distance = 5;
        if (blocks.empty()) {
            for (int y = -distance; y <= 0; y++) {
                for (int x = -distance; x <= distance; x++) {
                    for (int z = -distance; z <= distance; z++) {
                        blocks.push_back(Vector3f(x, y, z));
                    }
                }
            }
            std::sort(blocks.begin(), blocks.end(), [](Vector3f start, Vector3f end) {
                return sqrtf((start.x * start.x) + (start.y * start.y) + (start.z * start.z)) < sqrtf((end.x * end.x) + (end.y * end.y) + (end.z * end.z));
            });
        }

        for (const Vector3f& offset : blocks) {
            if (buildBlock(Vector3f(pos.floor()).add(offset))) {
                return true;
            }
        }
        return false;
    }

    void onTick(GameMode *gm) override {
        LocalPlayer* lp = ClientInstance::get()->getLocalPlayer();

        shouldRot = false;

        if (lp == nullptr) return;
        if (lp->getRegion() == nullptr) return;

        PlayerInventory * supplies = lp->getSupplies();
        if (autoBlock) {
            Inventory* inv = supplies->getInventory();
            prevSlot = supplies->selectedHotbarSlot;
            // 找到第一个可用的方块槽位
            int foundSlot = -1;
            for (int i = 0; i <= 8; i++) {
                ItemStack* stack = inv->getItemStack(i);
                if (HackUtils::ItemUtils::isBlock(stack)) {
                    foundSlot = i;
                    break;  // 找到第一个就停止
                }
            }
            // 如果找到了方块且不是当前槽位，则切换
            if (foundSlot != -1 && foundSlot != prevSlot) {
                supplies->selectedHotbarSlot = foundSlot;
            }
        }

        if (HackUtils::ItemUtils::isBlock(lp->getSelectedItem())) {
            Vector3f vel = lp->posComp->velocity;
            float speed = vel.magnitudexz();
            vel = vel.normalize();

            Vector3f blockBelow(lp->getPos().x, lockY ? launchY : lp->aabbComp->lowerPos.y - 0.5f, lp->getPos().z);
            blockBelow.x += vel.x * extend;
            blockBelow.z += vel.z * extend;

            shouldRot = true;

            if (HackUtils::BlockUtils::isBlockReplaceable(blockBelow)) {
                predictBlock(blockBelow);
            } else if (!buildBlock(blockBelow)) {
                float velocity_xz = ClientInstance::get()->getLocalPlayer()->posComp->velocity.magnitudexz();
                if (velocity_xz > 0.f) {  // Are we actually walking?
                    blockBelow.x -= vel.x;
                    blockBelow.z -= vel.z;
                }
            }
        }
        // 恢复之前的槽位
        supplies->selectedHotbarSlot = prevSlot;
    }

    void onActorRender(Actor *obj, Vector2f *rot, Vector3f *camera, Vector3f *pos, bool &needRot) override {
        if (obj == ClientInstance::get()->getLocalPlayer() && obj) {
            if (shouldRot) {
                if (ClientInstance::get()->getOptions()->getPlayerViewPerspective() != 0) {
                    LocalPlayer* lp = ClientInstance::get()->getLocalPlayer();

                    Vector2f angle = lp->getPos().CalcAngle(Vector3f(calc.x, calc.y, calc.z));

                    lp->rotComp->rot.x = angle.x;
                    lp->rotComp->rot.y = angle.y;
                    lp->rotComp->rotPrev.x = angle.x;
                    lp->rotComp->rotPrev.y = angle.y;

                    lp->setHeadRot(angle);
                    lp->setPitchSmooth(angle.x);
                    lp->setYBodyRotation(angle.y);

                    needRot = true;
                }
            }
        }
    }

    void onSendPacket(Packet *pkt, bool &cancel) override {
        if (pkt->getName() == "InteractPacket") {
            auto packet = (InteractPacket *) (pkt);
            if (packet->mAction == InteractPacket::Action::InteractUpdate)
                cancel = true;
        }
        if (rotMode.currentValue != 0 && rotMode.currentValue != 2) {
            if (shouldRot) {
                Vector2f angleNet = ClientInstance::get()->getLocalPlayer()->getPos().CalcAngle(Vector3f(calc.x, calc.y, calc.z));
                if (pkt->getName() == "MovePlayerPacket") {
                    auto movePkt = (MovePlayerPacket *) (pkt);
                    movePkt->mRot.x = 87;
                    movePkt->mRot.y = angleNet.y;
                    movePkt->mYHeadRot = angleNet.y;
                }
                if (pkt->getName() == "PlayerAuthInputPacket") {
                    auto authPkt = (PlayerAuthInputPacket*) (pkt);
                    authPkt->pitch = 87;
                    authPkt->yaw = angleNet.y;
                    authPkt->headYaw = angleNet.y;
                }
            }
        }
    }
};

#endif //CHEAT_SCAFFOLD_H