/*
 * TestBase.cpp
 *
 *  Created on: 2014/06/27
 *      Author: hideo-t
 */

#include <TestBase.h>
#include <cmath>

TestBase::~TestBase()
{
}

TestBase::TestBase()
{
    test_count_ = 0;
    test_passed_count_ = 0;
    test_failed_count_ = 0;

    /* the tolerance threshold is arbitrarily chosen here. */
    tolerance_ = 1.0e-10; /* default tolerance */

    setOut(&std::cout);
}

void TestBase::setOut(std::ostream *pout)
{
    pout_ = pout;
}

void TestBase::setTolerance(double tolerance)
{
    tolerance_ = tolerance;
}

/*
 * Write a report.
 */
int TestBase::report()
{
    (*pout_) << "Test results : ";
    if (test_count_ == 0) {
        (*pout_) << "No tests were done." << std::endl;
        return 0;
    } else if (test_failed_count_ == 0) {
        (*pout_) << "All " << test_passed_count_ << " tests passed." << std::endl;
        return 0;
    } else {
        (*pout_) << test_count_ << " tests done, ";
        (*pout_) << test_passed_count_ << " passed, ";
        (*pout_) << test_failed_count_ << " failed." << std::endl;
        return 1;
    }
}

/*
 * Functions called from the above macros.
 * Do not call these functions directly.
 */
void TestBase::testTrueFunc(bool expression, const char *expression_source,
        const char *file, int line)
{
    test_count_++;
    (*pout_) << "[" << test_count_ <<"]";
    if (expression) {
        /* test passed */
        test_passed_count_++;
        (*pout_) << "PASSED : ";
    } else {
        /* test failed */
        test_failed_count_++;
        (*pout_) << "FAILED, true expected : ";
    }
    (*pout_) << "\"" << expression_source << "\" = " << expression << " at \"";
    (*pout_) << file << "\", (" << line << ")" << std::endl;
}

void TestBase::testFalseFunc(bool expression, const char *expression_source,
        const char *file, int line)
{
    test_count_++;
    (*pout_) << "[" << test_count_ <<"]";
    if (! expression) {
        /* test passed */
        test_passed_count_++;
        (*pout_) << "PASSED : ";
    } else {
        /* test failed */
        test_failed_count_++;
        (*pout_) << "FAILED, false expected : ";
    }
    (*pout_) << "\"" << expression_source << "\" = " << expression << " at \"";
    (*pout_) << file << "\", (" << line << ")" << std::endl;
}

void TestBase::testNotNullFunc(void *expression, const char *expression_source,
        const char *file, int line)
{
    test_count_++;
    (*pout_) << "[" << test_count_ <<"]";
    if (expression != NULL) {
        /* test passed */
        test_passed_count_++;
        (*pout_) << "PASSED : ";
    } else {
        /* test failed */
        test_failed_count_++;
        (*pout_) << "FAILED, non NULL expected : ";
    }
    (*pout_) << "\"" << expression_source << "\" = " << expression << " at \"";
    (*pout_) << file << "\", (" << line << ")" << std::endl;
}

void TestBase::testNullFunc(void *expression, const char *expression_source,
        const char *file, int line)
{
    test_count_++;
    (*pout_) << "[" << test_count_ <<"]";
    if (expression == NULL) {
        /* test passed */
        test_passed_count_++;
        (*pout_) << "PASSED : ";
    } else {
        /* test failed */
        test_failed_count_++;
        (*pout_) << "FAILED, NULL expected : ";
    }
    (*pout_) << "\"" << expression_source << "\" = " << expression << " at \"";
    (*pout_) << file << "\", (" << line << ")" << std::endl;
}

/* double version */
void TestBase::dblEqualsFunc(double calculated_value, const char *calculated_source,
        double expected_value, const char *file, int line)
{
    test_count_++;
    (*pout_) << "[" << test_count_ <<"]";

    double error = fabs(calculated_value - expected_value);

    if (error < tolerance_) {
        /* test passed */
        test_passed_count_++;
        (*pout_) << "PASSED : ";
    } else {
        /* test failed */
        test_failed_count_++;
        (*pout_) << "FAILED, " << expected_value << " expected : ";
    }
    (*pout_) << "\"" << calculated_source << "\" = " << calculated_value;
    (*pout_) << " at \"" << file << "\", (" << line << ")" << std::endl;
}

void TestBase::dbl3EqualsFunc(double v1, double v2, double v3,
        const char *s1, const char *s2, const char *s3,
        double e1, double e2, double e3, const char *file, int line)
{
    test_count_++;
    (*pout_) << "[" << test_count_ <<"]";

    double error = fabs(v1 - e1) + fabs(v2 - e2) + fabs(v3 - e3);

    if (error < tolerance_ * 3) {
        /* test passed */
        test_passed_count_++;
        (*pout_) << "PASSED : ";
    } else {
        /* test failed */
        test_failed_count_++;
        (*pout_) << "FAILED, [" << e1 << "," << e2 << "," << e3 << "] expected : ";
    }
    (*pout_) << "\"[" << s1 << "," << s2 << "," << s3 << "]\" = ";
    (*pout_) << "[" << v1 << "," << v2 << "," << v3 << "]";
    (*pout_) << " at \"" << file << "\", (" << line << ")" << std::endl;

}

/* int version */
void TestBase::intEqualsFunc(int calculated_value, const char *calculated_source,
        int expected_value, const char *file, int line)
{
    test_count_++;
    (*pout_) << "[" << test_count_ <<"]";

    if (calculated_value == expected_value) {
        /* test passed */
        test_passed_count_++;
        (*pout_) << "PASSED : ";
    } else {
        /* test failed */
        test_failed_count_++;
        (*pout_) << "FAILED, " << expected_value << " expected : ";
    }
    (*pout_) << "\"" << calculated_source << "\" = " << calculated_value;
    (*pout_) << " at \"" << file << "\", (" << line << ")" << std::endl;
}


void TestBase::int3EqualsFunc(int v1, int v2, int v3,
        const char *s1, const char *s2, const char *s3,
        int e1, int e2, int e3, const char *file, int line)
{
    test_count_++;
    (*pout_) << "[" << test_count_ <<"]";

    if (v1 == e1 && v2 == e2 && v3 == e3) {
        /* test passed */
        test_passed_count_++;
        (*pout_) << "PASSED : ";
    } else {
        /* test failed */
        test_failed_count_++;
        (*pout_) << "FAILED, [" << e1 << "," << e2 << "," << e3 << "] expected : ";
    }
    (*pout_) << "\"[" << s1 << "," << s2 << "," << s3 << "]\" = ";
    (*pout_) << "[" << v1 << "," << v2 << "," << v3 << "]";
    (*pout_) << " at \"" << file << "\", (" << line << ")" << std::endl;

}

/* size_t version */
void TestBase::sizeEqualsFunc(size_t calculated_value, const char *calculated_source,
        size_t expected_value, const char *file, int line)
{
    test_count_++;
    (*pout_) << "[" << test_count_ <<"]";

    if (calculated_value == expected_value) {
        /* test passed */
        test_passed_count_++;
        (*pout_) << "PASSED : ";
    } else {
        /* test failed */
        test_failed_count_++;
        (*pout_) << "FAILED, " << expected_value << " expected : ";
    }
    (*pout_) << "\"" << calculated_source << "\" = " << calculated_value;
    (*pout_) << " at \"" << file << "\", (" << line << ")" << std::endl;
}

/* pointer version */
void TestBase::ptrEqualsFunc(void *calculated_value, const char *calculated_source,
        void *expected_value, const char *file, int line)
{
    test_count_++;
    (*pout_) << "[" << test_count_ <<"]";

    if (calculated_value == expected_value) {
        /* test passed */
        test_passed_count_++;
        (*pout_) << "PASSED : ";
    } else {
        /* test failed */
        test_failed_count_++;
        (*pout_) << "FAILED, " << std::hex << (unsigned long) expected_value << " expected : ";
    }
    (*pout_) << "\"" << calculated_source << "\" = ";
    (*pout_) << std::hex << (unsigned long) calculated_value << std::dec;
    (*pout_) << " at \"" << file << "\", (" << line << ")" << std::endl;
}

void TestBase::xyzEqualsFunc(const VectorXYZ &calculated_value, const char *calculated_source,
        const VectorXYZ &expected_value, const char *file, int line)
{
    test_count_++;
    (*pout_) << "[" << test_count_ <<"]";

    VectorXYZ disp = (calculated_value - expected_value);
    double r2 = disp.square();
    if (r2 < tolerance_*tolerance_) {
        /* test passed */
        test_passed_count_++;
        (*pout_) << "PASSED : ";
    } else {
        /* test failed */
        test_failed_count_++;
        (*pout_) << "FAILED, " << expected_value << " expected : ";
    }
    (*pout_) << "\"" << calculated_source << "\" = " << calculated_value;
    (*pout_) << " at \"" << file << "\", (" << line << ")" << std::endl;
}

void TestBase::i3dEqualsFunc(const GridIndex3d &calculated_value, const char *calculated_source,
        const GridIndex3d &expected_value, const char *file, int line)
{
    test_count_++;
    (*pout_) << "[" << test_count_ <<"]";

    if (calculated_value == expected_value) {
        /* test passed */
        test_passed_count_++;
        (*pout_) << "PASSED : ";
    } else {
        /* test failed */
        test_failed_count_++;
        (*pout_) << "FAILED, " << expected_value << " expected : ";
    }
    (*pout_) << "\"" << calculated_source << "\" = " << calculated_value;
    (*pout_) << " at \"" << file << "\", (" << line << ")" << std::endl;
}
