#include "GameTrainer.hpp"
#include "Offsets.hpp"
#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <chrono>

namespace igi {
using namespace std::chrono_literals;
GameTrainer::GameTrainer() = default;
GameTrainer::~GameTrainer() { stop(); }
void GameTrainer::start() { if (running_.exchange(true)) return; worker_ = std::jthread([this]{ run(); }); }
void GameTrainer::stop() { running_ = false; if (worker_.joinable()) worker_.join(); }

void GameTrainer::toggleInvincible(){ std::scoped_lock l(mutex_); view_.invincible.enabled=!view_.invincible.enabled; view_.invincible.led=view_.invincible.enabled?LedState::Waiting:LedState::Off; }
void GameTrainer::toggleMagazine(){ std::scoped_lock l(mutex_); view_.magazine.enabled=!view_.magazine.enabled; view_.magazine.led=view_.magazine.enabled?LedState::Waiting:LedState::Off; }
void GameTrainer::toggleInventory(){ std::scoped_lock l(mutex_); view_.inventory.enabled=!view_.inventory.enabled; view_.inventory.led=view_.inventory.enabled?LedState::Waiting:LedState::Off; }
UiSnapshot GameTrainer::snapshot() const { std::scoped_lock l(mutex_); return view_; }
bool GameTrainer::validPointer(std::uint32_t p) const { return p >= 0x00010000u && p < 0x80000000u; }

std::optional<std::uintptr_t> GameTrainer::resolvePlayer() {
    auto p0=memory_.read<std::uint32_t>(memory_.moduleBase()+offsets::rootStatic); if(!p0||!validPointer(*p0)) return std::nullopt;
    auto p1=memory_.read<std::uint32_t>(*p0+offsets::playerStep1); if(!p1||!validPointer(*p1)) return std::nullopt;
    auto p2=memory_.read<std::uint32_t>(*p1+offsets::playerStep2); if(!p2||!validPointer(*p2)) return std::nullopt;
    auto p3=memory_.read<std::uint32_t>(*p2+offsets::playerStep3); if(!p3||!validPointer(*p3)) return std::nullopt;
    return static_cast<std::uintptr_t>(*p3);
}
std::optional<std::uintptr_t> GameTrainer::resolveMagazine() {
    auto m0=memory_.read<std::uint32_t>(memory_.moduleBase()+offsets::magazineStatic); if(!m0||!validPointer(*m0)) return std::nullopt;
    auto m1=memory_.read<std::uint32_t>(*m0+offsets::magazineStep1); if(!m1||!validPointer(*m1)) return std::nullopt;
    auto m2=memory_.read<std::uint32_t>(*m1+offsets::magazineStep2); if(!m2||!validPointer(*m2)) return std::nullopt;
    return static_cast<std::uintptr_t>(*m2)+offsets::magazineFinal;
}

void GameTrainer::updateHotkeys() {
    if (GetAsyncKeyState(VK_F1)&1) toggleInvincible();
    if (GetAsyncKeyState(VK_F2)&1) toggleMagazine();
    if (GetAsyncKeyState(VK_F3)&1) toggleInventory();
    if (GetAsyncKeyState(VK_F12)&1) { std::scoped_lock l(mutex_); view_.invincible.enabled=view_.magazine.enabled=view_.inventory.enabled=false; view_.invincible.led=view_.magazine.led=view_.inventory.led=LedState::Off; }
}

void GameTrainer::tick() {
    updateHotkeys();
    if (!memory_.attached()) {
        memory_.detach();
        if (!memory_.attach(offsets::processName)) { std::scoped_lock l(mutex_); view_.attached=false; view_.pid=0; view_.status=L"Waiting for igi.exe"; return; }
    }
    auto player=resolvePlayer();
    if(!player){ std::scoped_lock l(mutex_); view_.attached=true; view_.pid=memory_.pid(); view_.status=L"Attached â€¢ start or resume a mission"; return; }

    bool inv,mag,inventory;
    { std::scoped_lock l(mutex_); inv=view_.invincible.enabled; mag=view_.magazine.enabled; inventory=view_.inventory.enabled; view_.attached=true; view_.pid=memory_.pid(); view_.status=L"Connected â€¢ offline mission active"; }

    auto damage=memory_.read<float>(*player+offsets::accumulatedDamage);
    auto capacity=memory_.read<float>(*player+offsets::damageCapacity);
    if(damage&&capacity&&std::isfinite(*damage)&&std::isfinite(*capacity)&&*capacity>0.0f){
        float hp=std::clamp((*capacity-*damage)/(*capacity)*100.0f,0.0f,100.0f);
        std::scoped_lock l(mutex_); view_.healthPercent=hp;
    }
    if(inv){
        bool valid=damage&&capacity&&std::isfinite(*damage)&&std::isfinite(*capacity)&&*capacity>0.0f;
        bool wrote=false; if(valid&&*damage!=0.0f) wrote=memory_.write<float>(*player+offsets::accumulatedDamage,0.0f);
        std::scoped_lock l(mutex_); view_.invincible.led=valid?LedState::Active:LedState::Waiting; if(wrote) ++view_.invincible.writes;
    }
    if(mag){
        auto address=resolveMagazine(); bool plausible=false,wrote=false;
        if(address){ auto current=memory_.read<std::int32_t>(*address); plausible=current&&*current>=offsets::magazineMin&&*current<=offsets::magazineMax; if(plausible&&*current!=offsets::magazineTarget) wrote=memory_.write<std::int32_t>(*address,offsets::magazineTarget); }
        std::scoped_lock l(mutex_); view_.magazine.led=plausible?LedState::Active:LedState::Waiting; if(wrote) ++view_.magazine.writes;
    }
    if(inventory){
        auto count=memory_.read<std::int32_t>(*player+offsets::inventoryTable); bool valid=count&&*count>=0&&*count<=offsets::maxSafeRecords; std::uint64_t writes=0;
        if(valid){ for(std::int32_t i=0;i<*count;++i){ auto r=*player+offsets::inventoryFirst+static_cast<std::uintptr_t>(i)*offsets::inventoryRecordSize; auto current=memory_.read<std::int32_t>(r+4); auto maximum=memory_.read<std::int32_t>(r+8); if(!current||!maximum||*current<0) continue; auto target=*maximum>0?*maximum:offsets::unclampedTarget; if(target>=0&&target<=100000&&*current!=target&&memory_.write<std::int32_t>(r+4,target)) ++writes; } }
        std::scoped_lock l(mutex_); view_.inventory.led=valid?LedState::Active:LedState::Waiting; view_.inventory.writes+=writes;
    }
}

void GameTrainer::run(){ while(running_){ tick(); std::this_thread::sleep_for(std::chrono::milliseconds(offsets::pollIntervalMs)); } memory_.detach(); }
}
