#import "InputHandlerImpl.hpp"

#include <storm/window/Event.hpp>
#include <storm/window/VideoSettings.hpp>
#include <storm/window/Window.hpp>

using namespace storm;
using namespace storm::window;

/////////////////////////////////////
/////////////////////////////////////
InputHandlerImpl::InputHandlerImpl() {
}

/////////////////////////////////////
/////////////////////////////////////
InputHandlerImpl::~InputHandlerImpl() = default;

/////////////////////////////////////
/////////////////////////////////////
InputHandlerImpl::InputHandlerImpl(InputHandlerImpl &&) = default;

/////////////////////////////////////
/////////////////////////////////////
InputHandlerImpl &InputHandlerImpl::operator=(InputHandlerImpl &&) = default;

/////////////////////////////////////
/////////////////////////////////////
bool InputHandlerImpl::isKeyPressed(Key key) {
    return false;
}

/////////////////////////////////////
/////////////////////////////////////
bool InputHandlerImpl::isMouseButtonPressed(MouseButton button) {
    return false;
}

/////////////////////////////////////
/////////////////////////////////////
void InputHandlerImpl::setMousePosition(core::math::Vector2U position) {
}

/////////////////////////////////////
/////////////////////////////////////
void InputHandlerImpl::setMousePosition(core::math::Vector2U position, const Window &relative_to) {
}

/////////////////////////////////////
/////////////////////////////////////
core::math::Vector2U InputHandlerImpl::getMousePosition() {
    return core::makeNamed<core::math::Vector2U>(core::math::Vector2U { 0u, 0u });
}

/////////////////////////////////////
/////////////////////////////////////
core::math::Vector2U InputHandlerImpl::getMousePosition(const Window &relative_to) {
    return core::makeNamed<core::math::Vector2U>(core::math::Vector2U { 0u, 0u });
}

/////////////////////////////////////
/////////////////////////////////////
void InputHandlerImpl::setVirtualKeyboardVisible(bool) {
    // not supported
}
