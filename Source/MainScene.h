#pragma once

#include "axmol/axmol.h"
#include "rive/refcnt.hpp" // Include for rive::rcp
#include <memory>
#include <vector>

// Forward declarations
namespace rive {
    class File;
    class ArtboardInstance;
    class Scene;
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

    MainScene();
    ~MainScene() override;

private:
    ax::DrawNode* _riveDrawNode = nullptr;
    
    std::unique_ptr<rive::ArtboardInstance> _artboard;
    std::unique_ptr<rive::Scene> _riveScene; // For state machines or animations
    std::unique_ptr<AxmolRenderer> _riveRenderer;
    std::unique_ptr<AxmolFactory> _riveFactory;
    
    // Rive File - use rcp
    rive::rcp<rive::File> _riveFile;
    
    ax::EventListenerTouchAllAtOnce* _touchListener = nullptr;
};
