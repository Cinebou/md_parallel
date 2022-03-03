#include <MdDriver.h>
#include <Logger.h>


void MdDriver::doInitialStep() {
    Logger::out << "MdDriver::doStep  t = " << caseData_->t_ << std::endl;

    // HINT: some steps are skipped here. add them.
    procData_.exportSurfacingMoleculePosData();
    communicator_.exchangeMoleculePosData();
    procData_.importSurroundingMoleculePosData();

    // 分子間力を計算する
    procData_.calcForce();

    // HINT: some steps are skipped here. add them.
    procData_.clearSurroundingCells();

    //a(t+Δt)とv(t+1/2Δt)からv(t+Δt)を計算
    procData_.updateVelocityHalf();


    // 時間発展の回が１ステップ進んだことを記録する
    caseData_->incrementStep();

    Logger::out << "MdDriver::doStep:end" << std::endl;
}
