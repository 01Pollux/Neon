#pragma once

#include <Core/String.hpp>

namespace Neon::Material
{
    class IMaterialNode
    {
    public:
        virtual ~IMaterialNode() = default;

        /// <summary>
        /// Returns true if the node can be evaluated as shader output.
        /// </summary>
        [[nodiscard]] virtual bool CanEvaluate()
        {
            return true;
        }

        /// <summary>
        /// Evaluates the node and returns the result as a string.
        /// </summary>
        [[nodiscard]] virtual StringU8 Evalute() = 0;
    };
} // namespace Neon::Material