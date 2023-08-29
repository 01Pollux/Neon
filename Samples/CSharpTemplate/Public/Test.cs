using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Neon
{
    public class SomeClass
    {
        public int Function4(string str, float y)
        {
            Console.WriteLine(str);
            return 1 + (int)y;
        }
    }

    public class MonoTest : SomeClass
    {
        public int SomeField = 12;

        public int SomeProperty
        {
            get
            {
                return SomeField;
            }
            set
            {
                Console.WriteLine("FROM C# : Setting property value to {0}", value);
                SomeField = value;
            }
        }

        public static int SomeFieldStatic = 12;

        public static int SomePropertyStatic
        {
            get
            {
                return SomeFieldStatic;
            }
            set
            {
                Console.WriteLine("FROM C# : Setting static property value to {0}", value);
                SomeFieldStatic = value;
            }
        }

        static MonoTest()
        {
            Console.WriteLine("FROM C# : STATIC CONSTRUCTOR.");
        }
        public MonoTest()
        {
            Console.WriteLine("FROM C# : MonoppTest created.");
        }
        public MonoTest(int x, float y)
        {
            Console.WriteLine("FROM C# : MonoppTest created with params: {0}, {1}", x, y);
        }
        ~MonoTest()
        {
            Console.WriteLine("FROM C# : MonoppTest destroyed.");
        }

        public void Method1()
        {
            Console.WriteLine("FROM C# : Hello Mono World from instance.");
        }

        public void Method2(string s)
        {
            Console.WriteLine("FROM C# : WithParam string: " + s);
        }
        public void Method3(int s)
        {
            Console.WriteLine("FROM C# : WithParam int: " + s);
        }
        public void Method4(int s, int s1)
        {
            Console.WriteLine("FROM C# : WithParam int, int: {0}, {1}", s, s1);
        }

        public string Method5(string s, int b)
        {
            Console.WriteLine("FROM C# : WithParam: {0}, {1}", s, b);
            return "Return Value: " + s;
        }

        public static int Function1(int a)
        {
            Console.WriteLine("FROM C# : Int value: " + a);
            return a + 1337;
        }

        public static void Function2(float a, int b, float c)
        {
            Console.WriteLine("FROM C# : VoidMethod: {0}, {1}, {2}", a, b, c);
        }

        public static void Function3(string a)
        {
            Console.WriteLine("FROM C# : String value: {0}", a);
        }

        //public new int Function4(string str, float y)
        //{
        //    Console.WriteLine(str);
        //    return 10 + (int)y;
        //}

        public static void Function5()
        {
            throw new Exception("Hello!");
        }
    }
}