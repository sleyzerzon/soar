#ifndef UNITTEST_H
#define UNITTEST_H

#include <algorithm>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>
#include <sstream>

#define CPPUNIT_STR_STR(x) #x
#define CPPUNIT_STR(x) CPPUNIT_STR_STR(x)

class CPPUnit_Assert_Failure : public std::runtime_error {
public:
  explicit CPPUnit_Assert_Failure(const std::string &msg = "CPPUnit Assertion Failure")
    : std::runtime_error(msg)
  {
  }
};

#define CPPUNIT_ASSERT(x) if(!(x)) { \
  std::ostringstream oss; \
  oss << "CPPUnit Assertion Failure " << " in file " << __FILE__ << ':' << __LINE__ << " : " <<  CPPUNIT_STR(x); \
  throw CPPUnit_Assert_Failure(oss.str()); \
}
#define CPPUNIT_ASSERT_MESSAGE(m, x) if(!(x)) { \
  std::ostringstream oss; \
  oss << "CPPUnit Assertion Failure " << " in file " << __FILE__ << ':' << __LINE__ << " : " << CPPUNIT_STR(x); \
  oss << std::endl << m; \
  throw CPPUnit_Assert_Failure(oss.str()); \
}

#define CPPUNIT_TEST_SUITE(x) typedef x test_t; \
  friend class Register_##x; \
  std::string get_class_name() const {return CPPUNIT_STR(x);} \
  std::list<std::pair<std::string, std::function<void ()>>> get_tests() { \
    std::list<std::pair<std::string, std::function<void ()>>> tests;
#define CPPUNIT_TEST(x) \
    tests.push_back(std::make_pair(CPPUNIT_STR(x), std::bind(&test_t::x, this)));
#define CPPUNIT_TEST_SUITE_END() \
    return tests; \
  }

#define CPPUNIT_TEST_SUITE_REGISTRATION(x) \
  static class Register_##x { \
  public: \
    Register_##x() { \
      CPPUNIT_NS::TestFactoryRegistry::getRegistry().giveTest(&m_test_class); \
    } \
    x m_test_class; \
  } g_registration_##x;

class TestFactoryRegistry;

class CPPUNIT_NS {
public:
  class TestListener {
  public:
    virtual void tell(const bool &) = 0;
  };

  class TestResult {
  public:
    void addListener(TestListener * const &listener) {
      m_listeners.push_back(listener);
    }

    void tell(const bool &result_) {
      const bool &result = result_;
      std::for_each(m_listeners.begin(), m_listeners.end(), [&result](TestListener * const listener) {listener->tell(result);});
    }

  private:
    std::list<TestListener *> m_listeners;
  };

  class TestCase {
    virtual std::string get_class_name() const = 0;
    virtual std::list<std::pair<std::string, std::function<void ()>>> get_tests() = 0;
    virtual void setUp() {}
    virtual void tearDown() {}

  public:
    virtual void run(TestResult &result_) {
      TestResult &result = result_;
      auto tests = get_tests();
      std::for_each(tests.begin(), tests.end(), [this, &result](const std::pair<std::string, std::function<void ()>> &sf) {
        std::cout << this->get_class_name() << "::" << sf.first;
        std::cout.flush();
        this->setUp();
        try {
          sf.second();
          result.tell(true);
        }
        catch(CPPUnit_Assert_Failure &caf) {
          result.tell(false);
          std::cerr << caf.what() << std::endl;
        }
        this->tearDown();
      });
    }
  };

  class TestResultCollector : public TestListener {
  public:
    TestResultCollector()
      : m_successes(0),
      m_failures(0)
    {
    }

    bool wasSuccessful() const {return !m_failures;}
    size_t successes() const {return m_successes;}
    size_t failures() const {return m_failures;}
    
  private:
    void tell(const bool &result) {
      ++(result ? m_successes : m_failures);
    }

    size_t m_successes;
    size_t m_failures;
  };

  class BriefTestProgressListener : public TestListener {
    void tell(const bool &result) {
      std::cout << " : " << (result ? "OK" : "FAILED") << std::endl;
    }
  };

  class TestRunner {
  public:
    void addTest(TestCase * const test) {
      tests.push_back(test);
    }

    void run(TestResult &result_) {
      TestResult &result = result_;
      std::for_each(tests.begin(), tests.end(), [&result](TestCase * const &test) {test->run(result);});
    }

  private:
    std::list<TestCase *> tests;
  };

  class CompilerOutputter {
  public:
    CompilerOutputter(const TestResultCollector * const trc, std::ostream &os)
      : m_trc(trc),
      m_os(&os)
    {
    }

    void write() const {
      if(m_trc->wasSuccessful())
        *m_os << "OK (" << m_trc->successes() << ')' << std::endl;
      else
        *m_os << "FAILURE (" << m_trc->successes() << " successful, " << m_trc->failures() << " failed)" << std::endl;
    }

  private:
    const TestResultCollector * m_trc;
    std::ostream * m_os;
  };

  class TestFactoryRegistry {
    TestFactoryRegistry(const TestFactoryRegistry &);
    TestFactoryRegistry operator=(const TestFactoryRegistry &);

    TestFactoryRegistry() {}
    ~TestFactoryRegistry() {}

    class TestCases : public TestCase {
      TestCases(const TestCases &);
      TestCases operator =(const TestCases &);
      
      TestCases() {}
      ~TestCases() {}

      std::string get_class_name() const {return "";}
      std::list<std::pair<std::string, std::function<void ()>>> get_tests() {return std::list<std::pair<std::string, std::function<void ()>>>();}

    public:
      static TestCases & get_TestCases() {
        static TestCases g_TestCases;
        return g_TestCases;
      }

      void run(TestResult &result_) {
        TestResult &result = result_;
        std::for_each(tests.begin(), tests.end(), [&result](TestCase * const &test) {test->run(result);});
      }

      std::list<TestCase *> tests;
    };

  public:
    static TestFactoryRegistry & getRegistry() {
      static TestFactoryRegistry g_TestFactoryRegistry;
      return g_TestFactoryRegistry;
    }

    TestCase * makeTest() const {
      return &TestCases::get_TestCases();
    }

    void giveTest(TestCase * const test) {
      TestCases::get_TestCases().tests.push_back(test);
    }

  private:
  };

};

#endif
