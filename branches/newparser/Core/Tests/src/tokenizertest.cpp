#include <portability.h>

#include <cppunit/extensions/HelperMacros.h>

#include "cli_Tokenizer.h"
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

class TokenizerTest : public CPPUNIT_NS::TestCase, public cli::TokenizerCallback
{
	CPPUNIT_TEST_SUITE( TokenizerTest );	// The name of this class

	CPPUNIT_TEST( testTokenizer01 );
	CPPUNIT_TEST( testTokenizer02 );
	CPPUNIT_TEST( testTokenizer03 );
	CPPUNIT_TEST( testTokenizer04 );
	CPPUNIT_TEST( testTokenizer05 );
	CPPUNIT_TEST( testTokenizer06 );
	CPPUNIT_TEST( testTokenizer07 );
	CPPUNIT_TEST( testTokenizer08 );
	CPPUNIT_TEST( testTokenizer09 );
	CPPUNIT_TEST( testTokenizer10 );
	CPPUNIT_TEST( testTokenizer11 );
	CPPUNIT_TEST( testTokenizer12 );
	CPPUNIT_TEST( testTokenizer13 );
	CPPUNIT_TEST( testTokenizer14 );
	CPPUNIT_TEST( testTokenizer15 );
	CPPUNIT_TEST( testTokenizer16 );
	CPPUNIT_TEST( testTokenizer17 );
	CPPUNIT_TEST( testTokenizer18 );
	CPPUNIT_TEST( testTokenizer19 );
	CPPUNIT_TEST( testTokenizer20 );
	CPPUNIT_TEST( testTokenizer21 );
	CPPUNIT_TEST( testTokenizer22 );
	CPPUNIT_TEST( testTokenizer23 );
	CPPUNIT_TEST( testTokenizer24 );
	CPPUNIT_TEST( testTokenizer25 );
	CPPUNIT_TEST( testTokenizer26 );
	CPPUNIT_TEST( testTokenizer27 );
	CPPUNIT_TEST( testTokenizer28 );
	CPPUNIT_TEST( testTokenizer29 );
	CPPUNIT_TEST( testTokenizer30 );
	CPPUNIT_TEST( testTokenizer31 );
	CPPUNIT_TEST( testTokenizer32 );
	CPPUNIT_TEST( testTokenizer33 );
	CPPUNIT_TEST( testTokenizer34 );
	CPPUNIT_TEST( testTokenizer35 );

	CPPUNIT_TEST_SUITE_END();

public:
    TokenizerTest() 
        : cd(0) 
    {}
    virtual ~TokenizerTest() {}
    virtual bool HandleCommand(std::vector<std::string>& argv);

	void setUp();		// Called before each function outlined by CPPUNIT_TEST
	void tearDown();	// Called after each function outlined by CPPUNIT_TEST

protected:
	void testTokenizer01();
	void testTokenizer02();
	void testTokenizer03();
	void testTokenizer04();
	void testTokenizer05();
	void testTokenizer06();
	void testTokenizer07();
	void testTokenizer08();
	void testTokenizer09();
	void testTokenizer10();
	void testTokenizer11();
	void testTokenizer12();
	void testTokenizer13();
	void testTokenizer14();
	void testTokenizer15();
	void testTokenizer16();
	void testTokenizer17();
	void testTokenizer18();
	void testTokenizer19();
	void testTokenizer20();
	void testTokenizer21();
	void testTokenizer22();
	void testTokenizer23();
	void testTokenizer24();
	void testTokenizer25();
	void testTokenizer26();
	void testTokenizer27();
	void testTokenizer28();
	void testTokenizer29();
	void testTokenizer30();
	void testTokenizer31();
	void testTokenizer32();
	void testTokenizer33();
	void testTokenizer34();
	void testTokenizer35();

    void evaluate(CallData* cd);

    cli::Tokenizer* tokenizer;
    CallData* cd;
};

CPPUNIT_TEST_SUITE_REGISTRATION( TokenizerTest ); // Registers the test so it will be used

void TokenizerTest::setUp()
{
    tokenizer = new cli::Tokenizer();
    tokenizer->SetHandler(this);
}

void TokenizerTest::tearDown()
{
    delete tokenizer;
}

bool TokenizerTest::HandleCommand(std::vector<std::string>& argv)
{
    CPPUNIT_ASSERT_MESSAGE(cd->input, argv.size() == cd->q.front().size());
    for (int i=0; i < argv.size(); i++)
        CPPUNIT_ASSERT(argv[i] == cd->q.front()[i]);
    cd->q.pop();
    return true;
}

void TokenizerTest::evaluate(CallData* cd)
{
    this->cd = cd;
    CPPUNIT_ASSERT_MESSAGE(cd->input, tokenizer->Evaluate(cd->input));
}

void TokenizerTest::testTokenizer01() { evaluate(&CallData("seek", 1, "seek")); }
void TokenizerTest::testTokenizer02() { evaluate(&CallData("\nseek", 1, "seek")); }
void TokenizerTest::testTokenizer03() { evaluate(&CallData(" \nseek", 1, "seek")); }
void TokenizerTest::testTokenizer04() { evaluate(&CallData(" \n seek", 1, "seek")); }
void TokenizerTest::testTokenizer05() { evaluate(&CallData("seek\n", 1, "seek")); }
void TokenizerTest::testTokenizer06() { evaluate(&CallData("seek\n ", 1, "seek")); }
void TokenizerTest::testTokenizer07() { evaluate(&CallData("seek \n", 1, "seek")); }
void TokenizerTest::testTokenizer08() { evaluate(&CallData("seek \n ", 1, "seek")); }
void TokenizerTest::testTokenizer09() { evaluate(&CallData("s eek", 2, "s", "eek")); }
void TokenizerTest::testTokenizer10() { evaluate(&CallData(" s eek", 2, "s", "eek")); }
void TokenizerTest::testTokenizer11() { evaluate(&CallData("s eek ", 2, "s", "eek")); }
void TokenizerTest::testTokenizer12() { evaluate(&CallData("s  eek", 2, "s", "eek")); }
void TokenizerTest::testTokenizer13() { evaluate(&CallData("s  eek", 2, "s", "eek")); }
void TokenizerTest::testTokenizer14() { evaluate(&CallData("s   eek", 2, "s", "eek")); }
void TokenizerTest::testTokenizer15() { evaluate(&CallData(" s ee k", 3, "s", "ee", "k")); }
void TokenizerTest::testTokenizer16() { evaluate(&CallData("s ee k", 3, "s", "ee", "k")); }
void TokenizerTest::testTokenizer17() { evaluate(&CallData("s ee k ", 3, "s", "ee", "k")); }
void TokenizerTest::testTokenizer18() { evaluate(&CallData(" s ee k ", 3, "s", "ee", "k")); }
void TokenizerTest::testTokenizer19() { evaluate(&CallData(" s ee k ", 3, "s", "ee", "k")); }
void TokenizerTest::testTokenizer20() { evaluate(&CallData("s \"ee\" k", 3, "s", "ee", "k")); }
void TokenizerTest::testTokenizer21() { evaluate(&CallData("s \"e e\" k", 3, "s", "e e", "k")); }
void TokenizerTest::testTokenizer22() { evaluate(&CallData("s \" e e\" k", 3, "s", " e e", "k")); }
void TokenizerTest::testTokenizer23() { evaluate(&CallData("s \"e e \" k", 3, "s", "e e ", "k")); }
void TokenizerTest::testTokenizer24() 
{
        CallData a("seek\nseek");
        a.addResult(1, "seek");
        a.addResult(1, "seek");
        evaluate(&a);
}
void TokenizerTest::testTokenizer25()
{
        CallData a("\nseek\nseek");
        a.addResult(1, "seek");
        a.addResult(1, "seek");
        evaluate(&a);
}
void TokenizerTest::testTokenizer26()
{
        CallData a("seek\nseek\n");
        a.addResult(1, "seek");
        a.addResult(1, "seek");
        evaluate(&a);
}
void TokenizerTest::testTokenizer27()
{
        CallData a("seek\nse\nek");
        a.addResult(1, "seek");
        a.addResult(1, "se");
        a.addResult(1, "ek");
        evaluate(&a);
}
void TokenizerTest::testTokenizer28()
{
        CallData a("seek\n se  \n ek");
        a.addResult(1, "seek");
        a.addResult(1, "se");
        a.addResult(1, "ek");
        evaluate(&a);
}
void TokenizerTest::testTokenizer29() { evaluate(&CallData("\"se\nek\"", 1, "se\nek")); }
void TokenizerTest::testTokenizer30() { evaluate(&CallData("{{seek}}", 1, "{seek}")); }
void TokenizerTest::testTokenizer31()
{
    const char* body =
        "water-jug*propose*pour\n"
        "   (state <s> ^name water-jug\n"
        "              ^jug <i> { <> <i> <j> })\n"
        "   (<i> ^contents > 0 )\n"
        "   (<j> ^empty > 0)\n"
        "-->\n"
        "   (<s> ^operator <o> + =)\n"
        "   (<o> ^name pour\n"
        "        ^empty-jug <i>\n"
        "        ^fill-jug <j>)";
    std::string rule("sp {");
    rule.append(body);
    rule.append("}");
    evaluate(&CallData(rule.c_str(), 2, "sp", body));
}
void TokenizerTest::testTokenizer32() { evaluate(&CallData("print -i (s1 ^* *)", 5, "print", "-i", "(s1", "^*", "*)")); }
void TokenizerTest::testTokenizer33() { evaluate(&CallData("{[seek]}", 1, "[seek]")); }
void TokenizerTest::testTokenizer34()
{
    const char* body =
        "gp*test3\n"
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
        "   (<s> ^operator <o> = 15)\n";
    std::string rule("gp {");
    rule.append(body);
    rule.append("}");
    evaluate(&CallData(rule.c_str(), 2, "gp", body));
}

void TokenizerTest::testTokenizer35()
{
    const char* body =
        "add*propose*toggle-to-b\n"
        "   (state <s> ^superstate.operator.name add\n"
        "              ^tss <tss>\n"
        "             #-^sum\n"
        "              ^superstate.toggle a)\n"
        "            # -^superstate.toggle b)\n"
        "   (<tss> -^sum)\n"
        "-->\n"
        "   (<s> ^operator <o> +)\n"
        "   (<o> ^name toggle-to-b)\n";
    std::string rule("sp {");
    rule.append(body);
    rule.append("}");
    evaluate(&CallData(rule.c_str(), 2, "sp", body));
}
