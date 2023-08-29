using Neon.Scene;

namespace Neon
{
    public class NeonObject
    {
        /// <summary>
        /// Self is a reference to the entity itself.
        /// </summary>
        public EntityHandle Self { get; private set; }

        /// <summary>
        /// Called when the component is created.
        /// </summary>
        public void OnCreate() { }

        /// <summary>
        /// Called when the component is destroyed.
        /// </summary>
        public void OnDestroy() { }
    }
}
