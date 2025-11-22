#include "MainScene.h"
#include "AxmolRive.h"
#include "rive/file.hpp"
#include "rive/artboard.hpp"
#include "rive/animation/linear_animation_instance.hpp"
#include "rive/scene.hpp"
#include "rive/math/mat2d.hpp"

using namespace ax;

MainScene::MainScene()
{
}

MainScene::~MainScene()
{
    if (_touchListener)
        _eventDispatcher->removeEventListener(_touchListener);
}

bool MainScene::init()
{
    if (!Scene::init())
    {
        return false;
    }

    auto visibleSize = _director->getVisibleSize();
    auto origin = _director->getVisibleOrigin();
    auto safeArea = _director->getSafeAreaRect();

    // 1. Background
    auto background = LayerColor::create(Color32(40, 40, 40, 255));
    this->addChild(background, 0);

    // 2. DrawNode for Rive
    _riveDrawNode = DrawNode::create();
    this->addChild(_riveDrawNode, 1);
    
    // Fix coordinate system: Rive is Top-Left (Y down), Axmol is Bottom-Left (Y up).
    // We flip the DrawNode vertically and move it to the top of the screen.
    _riveDrawNode->setScaleY(-1.0f);
    _riveDrawNode->setPositionY(visibleSize.height);

    // 3. Initialize Rive
    _riveFactory = std::make_unique<AxmolFactory>();
    _riveRenderer = std::make_unique<AxmolRenderer>(_riveDrawNode);

    // 4. Load .riv File
    auto fileUtils = FileUtils::getInstance();
    std::string fullPath = fileUtils->fullPathForFilename("marty.riv");
    
    if (fullPath.empty()) {
        AXLOGD("Error: marty.riv not found!");
    } else {
        auto data = fileUtils->getDataFromFile(fullPath);
        if (!data.isNull()) {
            rive::Span<const uint8_t> bytes(data.getBytes(), data.getSize());
            
            rive::ImportResult result;
            _riveFile = rive::File::import(bytes, _riveFactory.get(), &result);
            
            if (_riveFile) {
                _artboard = _riveFile->artboardDefault();
                if (_artboard) {
                    _artboard->advance(0.0f);
                    
                    // Play the first animation if available
                    // auto anim = _artboard->animationAt(0);
                    // if (anim) {
                    //    _riveScene = _artboard->instance(); // Wait, Scene/StateMachine logic
                    // }
                    // Simple animation playback using LinearAnimationInstance directly?
                    // Or just advance artboard if it has a state machine?
                    // For now just show the artboard.
                    
                    AXLOGD("Rive file loaded successfully!");
                }
            } else {
                AXLOGD("Failed to import Rive file.");
            }
        }
    }

    // 5. Close Button
    auto closeItem = MenuItemImage::create("CloseNormal.png", "CloseSelected.png",
                                           AX_CALLBACK_1(MainScene::menuCloseCallback, this));
    if (closeItem) {
        float x = safeArea.origin.x + safeArea.size.width - closeItem->getContentSize().width / 2;
        float y = safeArea.origin.y + closeItem->getContentSize().height / 2;
        closeItem->setPosition(Vec2(x, y));
        
        auto menu = Menu::create(closeItem, NULL);
        menu->setPosition(Vec2::ZERO);
        this->addChild(menu, 2);
    }

    scheduleUpdate();
    return true;
}

void MainScene::update(float delta)
{
    if (_artboard && _riveRenderer) {
        // Clear previous frame
        _riveDrawNode->clear();
        
        // Advance animation
        // Note: In a real app you would advance the StateMachine or AnimationInstance here.
        // For the artboard itself, advance updates components.
        _artboard->advance(delta);
        
        // Center and scale the artboard to fit the screen
        auto visibleSize = _director->getVisibleSize();
        
        _riveRenderer->save();
        _riveRenderer->align(rive::Fit::contain, rive::Alignment::center,
                             rive::AABB(0, 0, visibleSize.width, visibleSize.height),
                             _artboard->bounds());
        
        _artboard->draw(_riveRenderer.get());
        _riveRenderer->restore();
    }
}

void MainScene::menuCloseCallback(ax::Object* sender)
{
    _director->end();
}

void MainScene::onTouchesBegan(const std::vector<ax::Touch*>& touches, ax::Event* event) {}
void MainScene::onTouchesMoved(const std::vector<ax::Touch*>& touches, ax::Event* event) {}
void MainScene::onTouchesEnded(const std::vector<ax::Touch*>& touches, ax::Event* event) {}
