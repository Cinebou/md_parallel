/*
 * CaseData.h
 *
 *      Author: Hideo Takahashi
 */

#ifndef CASEDATA_H_
#define CASEDATA_H_

#include <string>

#include <BoxXYZ.h>
#include <GridIterator3d.h>
#include <IoException.h>
#include <DataException.h>

/*
 * 計算条件ファイルで指定されたパラメタや、時間発展計算の進行度合いを
 * 保持するクラス
 */
class CaseData {
public:

    // simulation parameters
    int npx_, npy_, npz_;     // processes per dimension
    int ncx_, ncy_, ncz_;     // cells per dimension in one process.
    double lx_, ly_, lz_;     // total box size [Ang]
    double plx_, ply_, plz_;  // process box size [Ang]
    double clx_, cly_, clz_;  // cell box size [Ang]
    double cutoff_radius_;    // cutoff radius [Ang]
    double delta_t_;          // step time [fs]
    double duration_;         // time to continue simulation [fs]
    int output_interval_;     // trajectory is written once per output_interval steps

    // path names for data files
    std::string initial_state_file_path_;
    std::string restart_file_path_;
    std::string trajectory_file_path_;
    std::string energy_file_path_;

    // about this simulation run
    GridRange3d allProcessesRange_;  // range including all processes in the simulation

    // about this process
    int my_rank_;                  // rank number assigned by MPI
    int num_procs_;                // total number of processes
    BoxXYZ localBox_;              // the box that this process is assigned to. [Ang]
    GridIterator3d localProcess_;  // the process coordinate for this process.


    // current state
    double t_;       // time within simulation [fs]
    int step_count_; // number of steps simulated.

    /*
     * Read case file and initialize.
     * throws IoException, DataException
     */
    void init(const char *file_name, int my_rank, int num_procs);

    /*
     * Read case file. Called within init.
     * throws IoException, DataException.
     */
    void readCaseFile(const char *file_name);

    /*
     * Calculate the process coordinate for a given rank.
     */
    void setProcessIteratorForRank(GridIndex3d *procIdx, int rank) const;

    /*
     * Calculate the MPI rank for a given process coordinate.
     */
    int getRankForProcess(const GridIndex3d &procIdx) const;

    /*
     * Calculate the process cell box coordinates for a given process coordinate.
     */
    void setBoxForProcess(BoxXYZ *box, const GridIndex3d &procIdx) const;

    /*
     * Calculate the cell box coordinates for a local cell coordinate.
     */
    void setBoxForCell(BoxXYZ *box, const GridIndex3d &cellIdx) const;

    /*
     * test if the simulation time has not reached the specified end time.
     */
    bool shouldProceed() const {
        return t_ <= duration_;
    }

    /*
     * test if the current step is one of the steps that we should
     * write trajectory data.
     */
    bool isOutputRound() const {
        return (step_count_ % output_interval_) == 0;
    }

    /*
     * increment the recorded step count and simulated time.
     */
    void incrementStep() {
        t_ += delta_t_;
        ++step_count_;
    }

    /*
     * test if the current process is the root rank process.
     */
    bool isRootRank() const {
        return my_rank_ == 0;
    }

};

#endif /* CASEDATA_H_ */
