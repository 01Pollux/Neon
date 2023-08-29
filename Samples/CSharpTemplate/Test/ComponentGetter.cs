using Neon;
using System;

namespace Test
{
    public class ComponentGetter : NeonObject
    {
        public class Nested
        {
            public class SubNested
            {

            }
        }

        public new void OnCreate()
        {
            Console.WriteLine("ComponentGetter.OnCreate");
        }

        public new void OnDestroy()
        {
            Console.WriteLine("ComponentGetter.OnDestroy");
        }
    }
}