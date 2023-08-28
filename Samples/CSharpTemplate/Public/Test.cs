using System;

namespace Tests
{
    class MonoTest
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
        ~MonoTest()
        {
            Console.WriteLine("FROM C# : MonoppTest destroyed.");
        }

        void Method1()
        {
            Console.WriteLine("FROM C# : Hello Mono World from instance.");
        }

        void Method2(string s)
        {
            Console.WriteLine("FROM C# : WithParam string: " + s);
        }
        void Method3(int s)
        {
            Console.WriteLine("FROM C# : WithParam int: " + s);
        }
        void Method4(int s, int s1)
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

        public static string Function4(string str)
        {
            return "The string value was: " + str;
        }

        public static void Function5()
        {
            throw new Exception("Hello!");
        }
    }
}