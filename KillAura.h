
#ifndef CHEAT_KILLAURA_H
#define CHEAT_KILLAURA_H

#include "../../ModuleManager.h"
#include "../../Module.h"
#include "LegiFightBot.h"
#include "../Misc/Disabler.h"

class KillAura : public IModule {
private:
    int switchTarget = 0;
    TheTimer *cpsTime = new TheTimer;
    std::string ec_tap = "\x93\xC4\x0B" "ModEventC2S" "\x94\xC4\x11" "ECNukkitClientMod" "\xC4\x14" "ECNukkitClientSystem" "\xC4\x19" "InteractBeforeClientEvent" "\x81\xC4\x04" "type" "\xC4\x03" "TAP" "\xC0";
    
    int cpsValue = 14;
    float range = 6.f;
    bool attackMob = false;
    bool autoWeapon = true;
    ModuleMode moduleMode;
    bool told = false;
    bool needClick = false;

public:
    std::vector<Actor*> targetList;
    std::vector<Actor*> targetListCopy;
    int boosts = 0;
    bool targetListEmpty = true;
    ModuleMode rotationMode;
    Vector2f angle;

    KillAura() : IModule("KillAura", Category::COMBAT, "自动攻击范围内符合条件的实体.", "杀戮光环") {
        moduleMode.addMode("Single");
        moduleMode.addMode("Switch");
        moduleMode.addMode("Multi");
        ModeValue("Mode", &moduleMode, "Switch", "模式");
        rotationMode.addMode("None");
        rotationMode.addMode("Strafe");
        rotationMode.addMode("Client");
        ModeValue("RotMode", &rotationMode, "Client", "转头模式");
        IntValue("CPS", &cpsValue, cpsValue, 1, 20, "每秒攻击");
        FloatValue("Range", &range, range, 0.1f, 20.f, "距离");
        BoolValue("AttackMob", &attackMob, false, "攻击生物");
        //BoolValue("AutoWeapon", &autoWeapon, autoWeapon, "自动武器");
    } ~KillAura() {}

    /*bool findWeapon() {
        PlayerInventory* supplies = ClientInstance::get()->getLocalPlayer()->getSupplies();
        Inventory* inv = supplies->getInventory();
        auto prevSlot = supplies->selectedHotbarSlot;
        for (int i = 0; i < 8; i++) {
            ItemStack* stack = inv->getItemStack(i);
            if (HackUtils::ItemUtils::isWeapon(stack)) {
                if (prevSlot != i) {
                    supplies->selectedHotbarSlot = i;
                }
                return true;
            }
        }
        return false;
    }*/

    void attack(Actor *obj) {
        if (boosts != 0) {
            for (char i = 0; i <= boosts; i++) {
                ClientInstance::get()->getLocalPlayer()->getGameMode()->attack(obj);
            }
        } else {
            ClientInstance::get()->getLocalPlayer()->getGameMode()->attack(obj);
        }
    }

    virtual void update() override {
        shouldDisplayMode = true;
        modeString = moduleMode.getMode();
    }

    virtual void onDisable() override {
        told = false;
        needClick = false;
        targetListEmpty = true;
        targetListCopy.clear();
    }

    virtual void onTick(GameMode *gm) override {
        if (needClick) {
            //HackUtils::clickScreen();
            ClientInstance::get()->getPacketSender()->sendData(PyRpc(ec_tap).write().data);
            needClick = false;
        }
    }

    virtual void onPlayerTick(LocalPlayer *lp) override {
        if (moduleMgr->getModule<LegitFightBot>()->isEnabled()) {
            if (!told) {
                notificationF("LegitFightBot 已开启, 为避免冲突杀戮将不工作");
                told = true;
            }
            targetListEmpty = true;
            return;
        }

        targetListEmpty = targetList.empty();

        if (lp->level == nullptr) return;

        targetList.clear();
        if (attackMob) {
            mGameData.forEachEntity([&](Actor *object) -> void {
                if (object != lp && object->getRuntimeId() != lp->getRuntimeId() && object->getUniqueId() != lp->getUniqueId()) {
                    if (ClientInstance::get()->getLocalPlayer()->distanceTo(object) <= range) {
                        targetList.push_back(object);
                    }
                }
            });
        } else {
            lp->dimension->forEachPlayer([&](Player& object) {
                if (&object != lp && object.getRuntimeId() != lp->getRuntimeId() && object.getUniqueId() != lp->getUniqueId()) {
                    if (TargetUtils::isValidTarget(&object)) {
                        if (ClientInstance::get()->getLocalPlayer()->distanceTo(&object) <= range) {
                            targetList.push_back(&object);
                        }
                    }
                }
                return true;
            });
        }

        targetListCopy = targetList;

        if (!targetList.empty()) {
            std::sort(targetList.begin(), targetList.end(), CompareTargetEnArray());

            angle = ClientInstance::get()->getLocalPlayer()->getPos().CalcAngle(targetList[0]->getPos());
            if (rotationMode.currentValue == 1) {
                lp->rotComp->pitch = angle.x;
                lp->rotComp->yaw = angle.y;
                lp->setYHeadRot(angle.y);
                lp->setYBodyRotation(angle.y);
            } else if (rotationMode.currentValue == 2) {
                lp->rotComp->pitch = angle.x;
                lp->setYHeadRot(angle.y);
                lp->setYBodyRotation(angle.y);
            }

            if (cpsTime->hasTimePassed((int) 1000.f / cpsValue)) {
                //if (autoWeapon) findWeapon();
                switch (moduleMode.currentValue) {
                    case 0:
                        attack(targetList[0]);
                        break;
                    case 1:
                        if (switchTarget < targetList.size()) {
                            attack(targetList[switchTarget]);
                            switchTarget++;
                        } else switchTarget = 0;
                        break;
                    case 2:
                        for (auto i: targetList){
                            attack(i);
                        }
                        break;
                }
                needClick = true;
                lp->swing();
                cpsTime->reset();
            }
        } else {
            switchTarget = 0;
        }
    }
};

#endif //CHEAT_KILLAURA_H
