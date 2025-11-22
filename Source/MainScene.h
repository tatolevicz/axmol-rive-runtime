#pragma once

#include "axmol/axmol.h"
#include "rive/refcnt.hpp" // Include for rive::rcp
#include "rive/animation/state_machine_instance.hpp"
#include "rive/animation/linear_animation_instance.hpp" // Added
#include <memory>
#include <vector>

// Forward declarations
namespace rive {
    class File;
    class ArtboardInstance;
    class Scene;
    class StateMachineInstance;
    class LinearAnimationInstance; // Added
}
class AxmolRenderer;
class AxmolFactory;

class MainScene : public ax::Scene
{
public:
    bool init() override;
    void update(float delta) override;

    // Input callbacks
    void onTouchesBegan(const std::vector<ax::Touch*>& touches, ax::Event* event);
    void onTouchesMoved(const std::vector<ax::Touch*>& touches, ax::Event* event);
    void onTouchesEnded(const std::vector<ax::Touch*>& touches, ax::Event* event);
    void menuCloseCallback(ax::Object* sender);

    // Helper to load artboard by index
    void loadArtboard(int index);

    MainScene();
    ~MainScene() override;

private:
    int _currentArtboardIndex = 0;
    ax::Node* _riveContainer = nullptr;
    
    std::unique_ptr<rive::ArtboardInstance> _artboard;
    std::unique_ptr<rive::Scene> _riveScene; // For state machines or animations
    std::unique_ptr<rive::StateMachineInstance> _stateMachine;
    std::unique_ptr<rive::LinearAnimationInstance> _animInstance; // Fallback animation
    std::unique_ptr<AxmolRenderer> _riveRenderer;
    std::unique_ptr<AxmolFactory> _riveFactory;
    
    // Rive File - use rcp
    rive::rcp<rive::File> _riveFile;
    
    ax::EventListenerTouchAllAtOnce* _touchListener = nullptr;
};
