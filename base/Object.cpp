#include "gpwe/util/Object.hpp"
#include "gpwe/sys.hpp"

using namespace gpwe;

SysManager *ObjectBase::sysManager() const noexcept{ return sys::manager(); }
InputManager *ObjectBase::inputManager() const noexcept{ return sys::inputManager(); }
RenderManager *ObjectBase::renderManager() const noexcept{ return sys::renderManager(); }
PhysicsManager *ObjectBase::physicsManager() const noexcept{ return sys::physicsManager(); }
WorldManager *ObjectBase::worldManager() const noexcept{ return sys::worldManager(); }
UiManager *ObjectBase::uiManager() const noexcept{ return sys::uiManager(); }
AppManager *ObjectBase::appManager() const noexcept{ return sys::appManager(); }
