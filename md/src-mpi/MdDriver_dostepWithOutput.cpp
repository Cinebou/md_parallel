#include <MdDriver.h>
#include <Logger.h>


void MdDriver::doStepWithOutput() {
    Logger::out << "MdDriver::doStep  t = " << caseData_->t_ << std::endl;

    //a(t)とv(t)からv(t+1/2Δt)を計算
    procData_.updateVelocityHalf(); //done
    // 位置を更新する
    procData_.updatePosition(); //done

    // 周辺セルに移動した粒子を、プロセスの外に転出する粒子として送信バッファに転記する
    procData_.exportExitingMoleculeFullData();
    // 転記が終わったので、全ての周辺セルを空にする
    procData_.clearSurroundingCells();
    // 周囲のプロセスとバッファ上のデータを送受信する
    communicator_.exchangeMoleculeFullData();
    // 受信バッファに受け取ったデータを表面セルに分配する
    procData_.importEnteringMoleculeFullData();


    // HINT: some steps are skipped here. add them.
    procData_.exportSurfacingMoleculePosData();
    communicator_.exchangeMoleculePosData();
    procData_.importSurroundingMoleculePosData();

    // 分子間力を計算する
    procData_.calcForceAndUp();

    // HINT: some steps are skipped here. add them.
    procData_.clearSurroundingCells();

    //a(t+Δt)とv(t+1/2Δt)からv(t+Δt)を計算
    procData_.updateVelocityHalfAndCalcUk();

    // 保有している全粒子の座標データをトラジェクトリー送信バッファに転記する
    procData_.exportTrajectoryData();

    procData_.exportEnergyData();

    communicator_.calcEnergy();

    if (!caseData_->isRootRank()) {
        // 自身がルートでなかったら、そのデータをルートに送る
        communicator_.sendTrajectroyDataToRoot();
    } else {
        // 自身がrootだったら、各プロセスから送られてくるデータを全て受け取る
        communicator_.recvTrajectoryDataAtRoot();
        // 全粒子分のデータをファイルに書く。
        procData_.writeTrajectoryData();

        procData_.writeEnergyData();
    }

    // 時間発展の回が１ステップ進んだことを記録する
    caseData_->incrementStep();

    Logger::out << "MdDriver::doStep:end" << std::endl;
}
