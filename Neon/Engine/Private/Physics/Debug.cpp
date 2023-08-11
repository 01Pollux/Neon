#include <EnginePCH.hpp>

#ifndef NEON_DIST
#include <Physics/Debug.hpp>
#include <Physics/World.hpp>
#include <Runtime/DebugOverlay.hpp>

#include <Log/Logger.hpp>

namespace Neon::Physics
{
    void BulletPhysicsDebugDraw::drawLine(
        const btVector3& From,
        const btVector3& To,
        const btVector3& Color)
    {
        Color4 RgbaColor(Color.getX(), Color.getY(), Color.getZ(), 1.0f);
        Runtime::DebugOverlay::DrawLine(
            Runtime::DebugOverlay::LineArgs{
                .StartPosition = FromBullet3<true>(From),
                .StartColor    = RgbaColor,
                .EndPosition   = FromBullet3<true>(To),
                .EndColor      = RgbaColor });
    }

    void BulletPhysicsDebugDraw::drawContactPoint(
        const btVector3& PointOnB,
        const btVector3& NormalOnB,
        btScalar         Distance,
        int              LifeTime,
        const btVector3& Color)
    {
        Color4 RgbaColor(Color.getX(), Color.getY(), Color.getZ(), 1.0f);
        Runtime::DebugOverlay::DrawLine(
            float(LifeTime),
            Runtime::DebugOverlay::LineArgs{
                .StartPosition = FromBullet3<true>(PointOnB),
                .StartColor    = RgbaColor,
                .EndPosition   = FromBullet3<true>(PointOnB + NormalOnB * Distance),
                .EndColor      = RgbaColor });
    }

    void BulletPhysicsDebugDraw::reportErrorWarning(
        const char* WarningString)
    {
        NEON_ERROR_TAG("Physics", WarningString);
    }

    void BulletPhysicsDebugDraw::draw3dText(
        const btVector3& Location,
        const char*      TextString)
    {
    }

    void BulletPhysicsDebugDraw::setDebugMode(
        int DebugMode)
    {
        m_DebugDrawModes = DebugDrawModes(DebugMode);
    }

    int BulletPhysicsDebugDraw::getDebugMode() const
    {
        return m_DebugDrawModes;
    }
} // namespace Neon::Physics
#endif