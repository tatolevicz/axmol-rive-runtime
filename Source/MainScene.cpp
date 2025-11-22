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

    // 2. Rive Container
    _riveContainer = Node::create();
    this->addChild(_riveContainer, 1);

    // Fix coordinate system: Rive is Top-Left (Y down), Axmol is Bottom-Left (Y up).
    // We flip the Container vertically and move it to the top of the screen.
    _riveContainer->setScaleY(-1.0f);
    _riveContainer->setPositionY(visibleSize.height);

    // 3. Initialize Rive
    _riveFactory = std::make_unique<AxmolFactory>();
    _riveRenderer = std::make_unique<AxmolRenderer>(_riveContainer);

    // 4. Load .riv File
    auto fileUtils = FileUtils::getInstance();
    std::string fullPath = fileUtils->fullPathForFilename("emojis.riv");

    if (fullPath.empty()) {
        AXLOGD("Error: emojis.riv not found!");
    } else {
        auto data = fileUtils->getDataFromFile(fullPath);
        if (!data.isNull()) {
            rive::Span<const uint8_t> bytes(data.getBytes(), data.getSize());

            rive::ImportResult result;
            _riveFile = rive::File::import(bytes, _riveFactory.get(), &result);

            if (_riveFile) {
                _artboard = _riveFile->artboardDefault();
                if (_artboard) {
                    // Advance artboard to initial state
                    _artboard->advance(0.0f);

                    // Try to load the first State Machine
                    _stateMachine = _artboard->stateMachineAt(0);
                    if (_stateMachine) {
                        AXLOGD("State Machine loaded successfully!");
                    } else {
                        // Fallback: Try to load the first animation
                        _animInstance = _artboard->animationAt(0);
                        if (_animInstance) {
                            AXLOGD("No State Machine, playing animation: %s", _animInstance->name().c_str());
                        } else {
                            AXLOGD("No State Machine and no Animation found.");
                        }
                    }

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

    // 6. Touch Listener
    _touchListener = ax::EventListenerTouchAllAtOnce::create();
    _touchListener->onTouchesBegan = AX_CALLBACK_2(MainScene::onTouchesBegan, this);
    _touchListener->onTouchesMoved = AX_CALLBACK_2(MainScene::onTouchesMoved, this);
    _touchListener->onTouchesEnded = AX_CALLBACK_2(MainScene::onTouchesEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(_touchListener, this);

    scheduleUpdate();
    return true;
}

void MainScene::update(float delta)
{
    if (_artboard && _riveRenderer) {
        // Prepare for new frame
        _riveRenderer->startFrame();

        // Advance animation
        if (_stateMachine) {
            _stateMachine->advance(delta);
        } else if (_animInstance) {
            _animInstance->advance(delta);
            _animInstance->apply();
            _artboard->advance(delta); // Still needed to update components
        } else {
            _artboard->advance(delta);
        }

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

void MainScene::onTouchesBegan(const std::vector<ax::Touch*>& touches, ax::Event* event) {
    if (_stateMachine && !touches.empty()) {
        auto touch = touches[0];
        auto location = touch->getLocation();
        // Convert to Rive coordinates (which are rendered centered/scaled)
        // This is tricky because we used 'align' in draw.
        // Ideally, we should store the transform used in 'draw'.
        // For now, let's assume we can just pass screen coordinates if we didn't align?
        // But we DID align.
        // To do this properly, we need the inverse of the alignment transform.
        // Rive's 'computeAlignment' returns a Mat2D. We should store it.

        // Re-calculate alignment transform
        auto visibleSize = _director->getVisibleSize();
        rive::Mat2D transform = rive::computeAlignment(
            rive::Fit::contain, rive::Alignment::center,
            rive::AABB(0, 0, visibleSize.width, visibleSize.height),
            _artboard->bounds()
        );

        rive::Mat2D inverse;
        if (transform.invert(&inverse)) {
            rive::Vec2D localPos = inverse * rive::Vec2D(location.x, visibleSize.height - location.y); // Axmol Y is up, Rive Y is down
            _stateMachine->pointerDown(localPos);
        }
    }
}

void MainScene::onTouchesMoved(const std::vector<ax::Touch*>& touches, ax::Event* event) {
    if (_stateMachine && !touches.empty()) {
        auto touch = touches[0];
        auto location = touch->getLocation();

        auto visibleSize = _director->getVisibleSize();
        rive::Mat2D transform = rive::computeAlignment(
            rive::Fit::contain, rive::Alignment::center,
            rive::AABB(0, 0, visibleSize.width, visibleSize.height),
            _artboard->bounds()
        );

        rive::Mat2D inverse;
        if (transform.invert(&inverse)) {
            rive::Vec2D localPos = inverse * rive::Vec2D(location.x, visibleSize.height - location.y);
            _stateMachine->pointerMove(localPos);
        }
    }
}

void MainScene::onTouchesEnded(const std::vector<ax::Touch*>& touches, ax::Event* event) {
    if (_stateMachine && !touches.empty()) {
        auto touch = touches[0];
        auto location = touch->getLocation();

        auto visibleSize = _director->getVisibleSize();
        rive::Mat2D transform = rive::computeAlignment(
            rive::Fit::contain, rive::Alignment::center,
            rive::AABB(0, 0, visibleSize.width, visibleSize.height),
            _artboard->bounds()
        );

        rive::Mat2D inverse;
        if (transform.invert(&inverse)) {
            rive::Vec2D localPos = inverse * rive::Vec2D(location.x, visibleSize.height - location.y);
            _stateMachine->pointerUp(localPos);
        }
    }
}
