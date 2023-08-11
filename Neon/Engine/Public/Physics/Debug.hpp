#pragma once

#ifndef NEON_DIST

#include <Core/Neon.hpp>
#include <Bullet3/LinearMath/btIDebugDraw.h>

namespace Neon::Physics
{
    class BulletPhysicsDebugDraw : public btIDebugDraw
    {
    public:
        BulletPhysicsDebugDraw() = default;

        NEON_CLASS_NO_COPYMOVE(BulletPhysicsDebugDraw);

        virtual ~BulletPhysicsDebugDraw() = default;

        void drawLine(
            const btVector3& From,
            const btVector3& To,
            const btVector3& Color) override;

        void drawContactPoint(
            const btVector3& PointOnB,
            const btVector3& NormalOnB,
            btScalar         Distance,
            int              LifeTime,
            const btVector3& Color) override;

        void reportErrorWarning(
            const char* WarningString) override;

        void draw3dText(
            const btVector3& Location,
            const char*      TextString) override;

        void setDebugMode(
            int DebugMode) override;

        int getDebugMode() const override;

    private:
        DebugDrawModes m_DebugDrawModes = DebugDrawModes::DBG_NoDebug;
    };
} // namespace Neon::Physics
#endif