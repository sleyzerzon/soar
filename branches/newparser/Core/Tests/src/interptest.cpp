#include <portability.h>

#include <cppunit/extensions/HelperMacros.h>

#include "cli_Interp.h"
#include <stdarg.h>
#include <queue>

struct CallData
{
    CallData(const char* input)
        : input(input)
    {
    }

    CallData(const char* input, int expected, ...)
        : input(input)
    {
        va_list args;
        va_start(args, expected);
        std::vector<std::string> argv;
        for (int i = 0; i < expected; ++i)
            argv.push_back(va_arg(args, const char*));
        va_end(args);

        q.push(argv);
    }

    void addResult(int expected, ...)
    {
        va_list args;
        va_start(args, expected);
        std::vector<std::string> argv;
        for (int i = 0; i < expected; ++i)
            argv.push_back(va_arg(args, const char*));
        va_end(args);
        q.push(argv);
    }

    const char* input;
    std::queue< std::vector<std::string> > q;
};

class InterpTest : public CPPUNIT_NS::TestCase
{
	CPPUNIT_TEST_SUITE( InterpTest );	// The name of this class

	CPPUNIT_TEST( testInterp01 );
	CPPUNIT_TEST( testInterp02 );
	CPPUNIT_TEST( testInterp03 );
	CPPUNIT_TEST( testInterp04 );
	CPPUNIT_TEST( testInterp05 );
	CPPUNIT_TEST( testInterp06 );
	CPPUNIT_TEST( testInterp07 );
	CPPUNIT_TEST( testInterp08 );
	CPPUNIT_TEST( testInterp09 );
	CPPUNIT_TEST( testInterp10 );
	CPPUNIT_TEST( testInterp11 );
	CPPUNIT_TEST( testInterp12 );
	CPPUNIT_TEST( testInterp13 );
	CPPUNIT_TEST( testInterp14 );
	CPPUNIT_TEST( testInterp15 );
	CPPUNIT_TEST( testInterp16 );
	CPPUNIT_TEST( testInterp17 );
	CPPUNIT_TEST( testInterp18 );
	CPPUNIT_TEST( testInterp19 );
	CPPUNIT_TEST( testInterp20 );
	CPPUNIT_TEST( testInterp21 );
	CPPUNIT_TEST( testInterp22 );
	CPPUNIT_TEST( testInterp23 );
	CPPUNIT_TEST( testInterp24 );
	CPPUNIT_TEST( testInterp25 );
	CPPUNIT_TEST( testInterp26 );
	CPPUNIT_TEST( testInterp27 );
	CPPUNIT_TEST( testInterp28 );
	CPPUNIT_TEST( testInterp29 );
	CPPUNIT_TEST( testInterp30 );
	CPPUNIT_TEST( testInterp31 );
	CPPUNIT_TEST( testInterp32 );
	CPPUNIT_TEST( testInterp33 );
	CPPUNIT_TEST( testInterp34 );
	CPPUNIT_TEST( testInterp35 );

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();		// Called before each function outlined by CPPUNIT_TEST
	void tearDown();	// Called after each function outlined by CPPUNIT_TEST

protected:
	void testInterp01();
	void testInterp02();
	void testInterp03();
	void testInterp04();
	void testInterp05();
	void testInterp06();
	void testInterp07();
	void testInterp08();
	void testInterp09();
	void testInterp10();
	void testInterp11();
	void testInterp12();
	void testInterp13();
	void testInterp14();
	void testInterp15();
	void testInterp16();
	void testInterp17();
	void testInterp18();
	void testInterp19();
	void testInterp20();
	void testInterp21();
	void testInterp22();
	void testInterp23();
	void testInterp24();
	void testInterp25();
	void testInterp26();
	void testInterp27();
	void testInterp28();
	void testInterp29();
	void testInterp30();
	void testInterp31();
	void testInterp32();
	void testInterp33();
	void testInterp34();
	void testInterp35();

    void evaluate(CallData cd);

    cli::Interp* interp;
};

CPPUNIT_TEST_SUITE_REGISTRATION( InterpTest ); // Registers the test so it will be used

void InterpTest::setUp()
{
    interp = new cli::Interp();
}

void InterpTest::tearDown()
{
    delete interp;
}

bool CommandHandler(std::vector<std::string>& argv, uintptr_t userData)
{
    CallData* cd = reinterpret_cast<CallData*>(userData);
    std::vector<std::string> eargv = cd->q.front();
    cd->q.pop();
    CPPUNIT_ASSERT_MESSAGE(cd->input, argv.size() == eargv.size());
    for (int i=0; i < argv.size(); i++)
        CPPUNIT_ASSERT(argv[i] == eargv[i]);
    return true;
}

void InterpTest::evaluate(CallData cd)
{
    interp->SetHandler(&CommandHandler, reinterpret_cast<uintptr_t>(&cd));
    CPPUNIT_ASSERT_MESSAGE(cd.input, interp->Evaluate(cd.input));
}

void InterpTest::testInterp01() { evaluate(CallData("seek", 1, "seek")); }
void InterpTest::testInterp02() { evaluate(CallData("\nseek", 1, "seek")); }
void InterpTest::testInterp03() { evaluate(CallData(" \nseek", 1, "seek")); }
void InterpTest::testInterp04() { evaluate(CallData(" \n seek", 1, "seek")); }
void InterpTest::testInterp05() { evaluate(CallData("seek\n", 1, "seek")); }
void InterpTest::testInterp06() { evaluate(CallData("seek\n ", 1, "seek")); }
void InterpTest::testInterp07() { evaluate(CallData("seek \n", 1, "seek")); }
void InterpTest::testInterp08() { evaluate(CallData("seek \n ", 1, "seek")); }
void InterpTest::testInterp09() { evaluate(CallData("s eek", 2, "s", "eek")); }
void InterpTest::testInterp10() { evaluate(CallData(" s eek", 2, "s", "eek")); }
void InterpTest::testInterp11() { evaluate(CallData("s eek ", 2, "s", "eek")); }
void InterpTest::testInterp12() { evaluate(CallData("s  eek", 2, "s", "eek")); }
void InterpTest::testInterp13() { evaluate(CallData("s  eek", 2, "s", "eek")); }
void InterpTest::testInterp14() { evaluate(CallData("s   eek", 2, "s", "eek")); }
void InterpTest::testInterp15() { evaluate(CallData(" s ee k", 3, "s", "ee", "k")); }
void InterpTest::testInterp16() { evaluate(CallData("s ee k", 3, "s", "ee", "k")); }
void InterpTest::testInterp17() { evaluate(CallData("s ee k ", 3, "s", "ee", "k")); }
void InterpTest::testInterp18() { evaluate(CallData(" s ee k ", 3, "s", "ee", "k")); }
void InterpTest::testInterp19() { evaluate(CallData(" s ee k ", 3, "s", "ee", "k")); }
void InterpTest::testInterp20() { evaluate(CallData("s \"ee\" k", 3, "s", "\"ee\"", "k")); }
void InterpTest::testInterp21() { evaluate(CallData("s \"e e\" k", 3, "s", "\"e e\"", "k")); }
void InterpTest::testInterp22() { evaluate(CallData("s \" e e\" k", 3, "s", "\" e e\"", "k")); }
void InterpTest::testInterp23() { evaluate(CallData("s \"e e \" k", 3, "s", "\"e e \"", "k")); }
void InterpTest::testInterp24() 
{
        CallData a("seek\nseek");
        a.addResult(1, "seek");
        a.addResult(1, "seek");
        evaluate(a);
}
void InterpTest::testInterp25()
{
        CallData a("\nseek\nseek");
        a.addResult(1, "seek");
        a.addResult(1, "seek");
        evaluate(a);
}
void InterpTest::testInterp26()
{
        CallData a("seek\nseek\n");
        a.addResult(1, "seek");
        a.addResult(1, "seek");
        evaluate(a);
}
void InterpTest::testInterp27()
{
        CallData a("seek\nse\nek");
        a.addResult(1, "seek");
        a.addResult(1, "se");
        a.addResult(1, "ek");
        evaluate(a);
}
void InterpTest::testInterp28()
{
        CallData a("seek\n se  \n ek");
        a.addResult(1, "seek");
        a.addResult(1, "se");
        a.addResult(1, "ek");
        evaluate(a);
}
void InterpTest::testInterp29() { evaluate(CallData("\"se\nek\"", 1, "\"se\nek\"")); }
void InterpTest::testInterp30() { evaluate(CallData("{{seek}}", 1, "{{seek}}")); }
void InterpTest::testInterp31()
{
    const char* rule =
        "sp {water-jug*propose*pour\n"
        "   (state <s> ^name water-jug\n"
        "              ^jug <i> { <> <i> <j> })\n"
        "   (<i> ^contents > 0 )\n"
        "   (<j> ^empty > 0)\n"
        "-->\n"
        "   (<s> ^operator <o> + =)\n"
        "   (<o> ^name pour\n"
        "        ^empty-jug <i>\n"
        "        ^fill-jug <j>)}";
    const char* body = rule + 3;
    evaluate(CallData(rule, 2, "sp", body));
}
void InterpTest::testInterp32() { evaluate(CallData("print -i (s1 ^* *)", 3, "print", "-i", "(s1 ^* *)")); }
void InterpTest::testInterp33() { evaluate(CallData("{[seek]}", 1, "{[seek]}")); }
void InterpTest::testInterp34()
{
    const char* rule =
        "gp {gp*test3\n"
        "   (state <s> ^operator <o> +\n"
        "              ^someflag [ true false ])\n"
        "   (<o> ^name foo\n"
        "        ^att [\n"
        "               val1 # a string value\n"
        "               1.3  # a numeric value\n"
        "               |another val[]][[[]]]][|  # a value with spaces and brackets in it\n"
        "               |\\|another val\\||  # a value with escaped pipes in it\n"
        "               ])\n"
        "-->\n"
        "   (<s> ^operator <o> = 15)\n"
        "}";
    const char* body = rule + 3;
    evaluate(CallData(rule, 2, "gp", body));
}

void InterpTest::testInterp35()
{
    const char* rule =
        "sp {add*propose*toggle-to-b\n"
        "   (state <s> ^superstate.operator.name add\n"
        "              ^tss <tss>\n"
        "             #-^sum\n"
        "              ^superstate.toggle a)\n"
        "            # -^superstate.toggle b)\n"
        "   (<tss> -^sum)\n"
        "-->\n"
        "   (<s> ^operator <o> +)\n"
        "   (<o> ^name toggle-to-b)\n"
        "}";
    const char* body = rule + 3;
    evaluate(CallData(rule, 2, "sp", body));
}
