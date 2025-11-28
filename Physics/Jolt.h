#pragma once

#include "../ThirdParty/JoltPhysics/Jolt/Jolt.h"

#include "../ThirdParty/JoltPhysics/Jolt/RegisterTypes.h"
#include "../ThirdParty/JoltPhysics/Jolt/Core/Factory.h"
#include "../ThirdParty/JoltPhysics/Jolt/Core/TempAllocator.h"
#include "../ThirdParty/JoltPhysics/Jolt/Core/JobSystemThreadPool.h"
#include "../ThirdParty/JoltPhysics/Jolt/Physics/PhysicsSettings.h"
#include "../ThirdParty/JoltPhysics/Jolt/Physics/PhysicsSystem.h"
#include "../ThirdParty/JoltPhysics/Jolt/Physics/Collision/Shape/BoxShape.h"
#include "../ThirdParty/JoltPhysics/Jolt/Physics/Collision/Shape/SphereShape.h"
#include "../ThirdParty/JoltPhysics/Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "../ThirdParty/JoltPhysics/Jolt/Physics/Collision/Shape/CylinderShape.h"
#include "../ThirdParty/JoltPhysics/Jolt/Physics/Collision/Shape/ConvexShape.h"
#include "../ThirdParty/JoltPhysics/Jolt/Physics/Collision/Shape/MeshShape.h"
#include "../ThirdParty/JoltPhysics/Jolt/Physics/Body/BodyCreationSettings.h"
#include "../ThirdParty/JoltPhysics/Jolt/Physics/Body/BodyActivationListener.h"

#include "../Renderer/OpenGL.h" // For GLM

JPH_SUPPRESS_WARNINGS

using namespace JPH;
using namespace JPH::literals;

inline Vec3 ToJolt(const glm::vec3& v) { return Vec3(v.x, v.y, v.z); }
inline glm::vec3 ToGLM(const Vec3& v) { return glm::vec3(v.GetX(), v.GetY(), v.GetZ()); }
