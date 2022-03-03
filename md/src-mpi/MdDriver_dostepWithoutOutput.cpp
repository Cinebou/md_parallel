#include <MdDriver.h>
#include <Logger.h>


void MdDriver::doStepWithoutOutput() {
    Logger::out << "MdDriver::doStep  t = " << caseData_->t_ << std::endl;

    //a(t)とv(t)からv(t+1/2Δt)を計算
    procData_.updateVelocityHalf(); //done
    // 位置を更新する
    procData_.updatePosition(); //done

    // 周辺セルに移動した粒子を、プロセスの外に転出する粒子として送信バッファに転記する
    procData_.exportExitingMoleculeFullData(); //done
    // 転記が終わったので、全ての周辺セルを空にする
    procData_.clearSurroundingCells();//done
    // 周囲のプロセスとバッファ上のデータを送受信する
    communicator_.exchangeMoleculeFullData();
    // 受信バッファに受け取ったデータを表面セルに分配する
    procData_.importEnteringMoleculeFullData(); //done


    // HINT: some steps are skipped here. add them.
    procData_.exportSurfacingMoleculePosData();
    communicator_.exchangeMoleculePosData();
    procData_.importSurroundingMoleculePosData();

    // 分子間力を計算する
    procData_.calcForce(); //done

    // HINT: some steps are skipped here. add them.
    procData_.clearSurroundingCells();//done

    //a(t+Δt)とv(t+1/2Δt)からv(t+Δt)を計算
    procData_.updateVelocityHalf(); //done

    // 時間発展の回が１ステップ進んだことを記録する
    caseData_->incrementStep(); //done

    Logger::out << "MdDriver::doStep:end" << std::endl;
}
