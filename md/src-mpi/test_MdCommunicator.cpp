/*
 * test_MdCommunicator.cpp
 *
 *  Created on: 2014/06/28
 *      Author: hideo-t
 */

#include <MpiTestBase.h>
#include <MdCommunicator.h>
#include <GridIterator3d.h>
#include <Logger.h>
#include <IoException.h>
#include <DataException.h>

class TestMdCommunicator : public MpiTestBase {
    CaseData caseData_;
    MdCommData commData_;
    MdCommunicator comm_;

public:
    void setup();
    void testPeerRanks();
    void testExchangeMoleculeFull();
    void run();
};

void TestMdCommunicator::setup()
{
    caseData_.init("testdata/mdcommunicator/case1.txt", my_rank_, num_procs_);
    commData_.init(&caseData_);
    comm_.init(&caseData_, &commData_);
}

void TestMdCommunicator::testPeerRanks()
{
    // test in a specific rank 5 = [0,1,2]

    if (my_rank_ == 5) {
        // lower in x = [-1,1,2] => wrapped to [2,1,2] = 9*2+3*1+2 = 23
        // check peer rank
        int_equals(commData_.bufferFor(GridIndex3d(0,1,1))->rank_, 23);
        // check offset when sending to peer (see case.txt)
        xyz_equals(commData_.bufferFor(GridIndex3d(0,1,1))->offset, VectorXYZ(100, 0, 0));

        // higher in x = [1,1,2] = 9*1+3*1+2 = 14
        int_equals(commData_.bufferFor(GridIndex3d(2,1,1))->rank_, 14);
        xyz_equals(commData_.bufferFor(GridIndex3d(2,1,1))->offset, VectorXYZ(0, 0, 0));

        // lower in y = [0,0,2] = 9*0+3*0+2 = 2
        int_equals(commData_.bufferFor(GridIndex3d(1,0,1))->rank_, 2);
        xyz_equals(commData_.bufferFor(GridIndex3d(1,0,1))->offset, VectorXYZ(0, 0, 0));

        // higher in y = [0,2,2] = 9*0+3*2+2 = 8
        int_equals(commData_.bufferFor(GridIndex3d(1,2,1))->rank_, 8);
        xyz_equals(commData_.bufferFor(GridIndex3d(1,2,1))->offset, VectorXYZ(0, 0, 0));

        // lower in z = [0,1,1] = 9*0+3*1+1 = 4
        int_equals(commData_.bufferFor(GridIndex3d(1,1,0))->rank_, 4);
        xyz_equals(commData_.bufferFor(GridIndex3d(1,1,0))->offset, VectorXYZ(0, 0, 0));

        // higher in z = [0,1,2] => wrapped to [0,1,0] = 9*0+3*1+0 = 3
        int_equals(commData_.bufferFor(GridIndex3d(1,1,2))->rank_, 3);
        xyz_equals(commData_.bufferFor(GridIndex3d(1,1,2))->offset, VectorXYZ(0, 0, -300));
    }
}



void TestMdCommunicator::testExchangeMoleculeFull()
{
    //
    // set dummy molecule full data for all directions
    //
    GridPeerIterator3d it;
    while (it.next()) {
        MdCommPeerBuffer *buff = commData_.bufferFor(it);
        // dummy data spec:
        // sending cell count : 2
        // count per cell: 2
        // molecule kind : sender rank number * 2
        // molecule serial : sender rank number*10000 + cell index in array*100 + number in array
        buff->send_molecule_full_.clear();
        buff->send_count_per_cell_.clear();
        int cell_count = 2;
        // loop for cells facing this peer
        for (int ci = 0; ci < cell_count; ci++) {
            int molecule_count = 2;
            buff->send_count_per_cell_.push_back(molecule_count);
            for (int i = 0; i < molecule_count; i++) {
                CommMoleculeFullData full;
                full.kind_ = my_rank_ * 2;
                full.serial_ = my_rank_*10000 + ci * 100 + i;
                buff->send_molecule_full_.push_back(full);
            }
            buff->recv_molecule_full_.clear();
            buff->recv_count_per_cell_.clear();
        }
    }

    // exchange data
    comm_.exchangeMoleculeFullData();

    // examine dummy molecule full data from all directions
    it.reset();
    while (it.next()) {
        MdCommPeerBuffer *buff = commData_.bufferFor(it);
        int sender_rank = buff->rank_;
        int cell_count = 2; // expected

        int_equals(buff->recv_count_per_cell_.size(), cell_count);
        int k = 0;
        for (int ci = 0; ci < cell_count; ci++) {
            int molecule_count = 2; // expected
            int_equals(buff->recv_count_per_cell_[ci], molecule_count);
            for (int i = 0; i < molecule_count; i++) {
                CommMoleculeFullData &full = buff->recv_molecule_full_[k++];
                int_equals(full.kind_, sender_rank * 2);
                int_equals(full.serial_, sender_rank*10000 + ci * 100 + i);
            }
        }
    }
}

//
// Run this test under MPI with 27 processes
void TestMdCommunicator::run()
{
    setup();
    testPeerRanks();
    testExchangeMoleculeFull();
}

/*
 * Run this test under MPI, with process numbers 27.
 * It must be run from the parent directory of the "testdata" directory.
 */
int main(int argc, char *argv[])
{
    TestMdCommunicator test;
    test.initMpi(argc, argv);
    try {
        test.run();
    } catch (IoException &exp) {
        std::cout << exp << std::endl;
    } catch (DataException &exp) {
        std::cout << exp << std::endl;
    }
    return test.report();
}
