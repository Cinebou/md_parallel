/*
 * test_MdProcData.cpp
 *
 *  Created on: 2014/06/28
 *      Author: hideo-t
 */


#include <TestBase.h>
#include <MdProcData.h>

/*
 * Tester class for MdProcData
 */
class TestMdProcData : public TestBase {
    /*
     * test target
     */
    CaseData caseData_;
    MdProcData procData_;
    MdCommData commData_;

public:

    void setup();
    void testRanges();
    void run();
};


void TestMdProcData::setup()
{
    caseData_.init("testdata/mdprocdata/case.txt", 5, 27);
    commData_.init(&caseData_);
    procData_.init(&caseData_, &commData_);
}

void TestMdProcData::testRanges()
{
    // inspect a surface range.
    const GridRange3d & r1 = procData_.surfaceRangeFor(GridIndex3d(0,1,1));
    int3_equals(r1.xmin_, r1.ymin_, r1.zmin_, 1, 1, 1);
    int3_equals(r1.xmax_, r1.ymax_, r1.zmax_, 1, 3, 3);

    // inspect a surrounding range.
    const GridRange3d &r2 = procData_.surroundingRangeFor(GridIndex3d(1,2,1));
    int3_equals(r2.xmin_, r2.ymin_, r2.zmin_, 1, 4, 1);
    int3_equals(r2.xmax_, r2.ymax_, r2.zmax_, 3, 4, 3);
}

void TestMdProcData::run()
{
    setup();
    testRanges();
}

int main(int argc, char *argv[])
{
    TestMdProcData test;
    test.run();
    return test.report();
}
