using System;

namespace Neon.Scene
{
    public struct EntityHandle
    {
        private ulong m_Handle;

        EntityHandle(
            ulong Handle)
        {
            m_Handle = Handle;
        }

        /// <summary>
        /// Returns an invalid entity id.
        /// </summary>
        public static EntityHandle Invalid
        {
            get
            {
                return new EntityHandle(0);
            }
        }

        /// <summary>
        /// Gets the entity id.
        /// </summary>
        public ulong GetId()
        {
            return m_Handle;
        }
    }
}