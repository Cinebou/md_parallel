/*
 * test_CaseData.cpp
 *
 *  Created on: 2014/06/28
 *      Author: hideo-t
 */


#include <TestBase.h>
#include <CaseData.h>

/*
 * Tester class for CaseData
 */
class TestCaseData : public TestBase {
    /*
     * test target
     */
    CaseData caseData_;

public:

    void setup();
    void testBox();
    void testRank();
    void run();
};

void TestCaseData::setup()
{
    // rank5 : ix,iy,iz = 0, 1, 2
    caseData_.init("testdata/casedata/case.txt", 5, 27);
}

void TestCaseData::testBox()
{
    int_equals(caseData_.my_rank_, 5);
    int_equals(caseData_.num_procs_, 27);
    GridIndex3d ind;
    i3d_equals(caseData_.localProcess_, GridIndex3d(0,1,2));
    xyz_equals(caseData_.localBox_.p1_, VectorXYZ(0, 200, 600));
    xyz_equals(caseData_.localBox_.p2_, VectorXYZ(100, 400, 900));
}

void TestCaseData::testRank()
{
    for (int ix = 0; ix < 3; ix++) {
        for (int iy = 0; iy < 3; iy++) {
            for (int iz = 0; iz < 3; iz++) {
                int rank = ix*9+iy*3+iz;
                GridIndex3d idx(ix,iy,iz);
                int_equals(caseData_.getRankForProcess(idx), rank);
                GridIndex3d res;
                caseData_.setProcessIteratorForRank(&res, rank);
                i3d_equals(res, idx);
            }
        }
    }
}

void TestCaseData::run()
{
    setup();
    testBox();
    testRank();
}

int main(int argc, char *argv[])
{
    TestCaseData test;
    try {
        test.run();
    } catch (IoException &exp) {
        std::cout << exp << std::endl;
    } catch (DataException &exp) {
        std::cout << exp << std::endl;
    }
    return test.report();
}
