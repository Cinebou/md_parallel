/*
 * test_LJParams.cpp
 *
 *  Created on: 2013/02/28
 *      Author: hideo-t
 */

#include <TestBase.h>
#include <LJParams.h>

/*
 * Tester class for LJParams
 */
class TestLJParams : public TestBase {
    /*
     * test target
     */

public:

    void run();
};


void TestLJParams::run()
{
    Particle p1, p2;

    CaseData cdata;
    cdata.delta_t_ = 1.0;
    LJParams::initParams(&cdata);

    double sigma = LJParams::SOURCE_PARAMS_[0].sigma_;
    double root6_2 = pow(2.0, 1.0/6.0);
    double r0 = sigma * root6_2;

    std::cout << "sigma : " << sigma << std::endl;
    std::cout << "root6_2 : " << root6_2 << std::endl;
    std::cout << "r0 : " << r0 << std::endl;

}

int main(int argc, char *argv[])
{
    TestLJParams test;
    test.run();
    return test.report();
}
