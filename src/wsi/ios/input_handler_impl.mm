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
void InputHandlerImpl::set_mouse_position(core::math::vec2u position) {
}

/////////////////////////////////////
/////////////////////////////////////
void InputHandlerImpl::set_mouse_position(core::math::vec2u position,
                                          const Window &relative_to) {
}

/////////////////////////////////////
/////////////////////////////////////
core::math::vec2u InputHandlerImpl::getMousePosition() {
    return core::makeNamed<core::math::vec2u>(core::math::vec2u { 0u, 0u });
}

/////////////////////////////////////
/////////////////////////////////////
core::math::vec2u InputHandlerImpl::getMousePosition(const Window &relative_to) {
    return core::makeNamed<core::math::vec2u>(core::math::vec2u { 0u, 0u });
}

/////////////////////////////////////
/////////////////////////////////////
void InputHandlerImpl::toggle_virtual_keyboard_visibility(bool) {
    // not supported
}
