/*
 * Driver.cpp
 *
 *      Author: Hideo Takahashi
 */

#include <MdDriver.h>
#include <Logger.h>

MdDriver::~MdDriver() {

}

MdDriver::MdDriver() {

}

void MdDriver::init(CaseData *caseData) {
    // caseDataは初期化済みのものが渡ってくる
    caseData_ = caseData;
    // commData（通信バッファ）を初期化する
    commData_.init(caseData);
    // communicator（通信機能）を初期化する
    communicator_.init(caseData_, &commData_);
    // 本プロセスの保持する物理計算のデータを初期化する（データファイル読み込みはここで起きる）
    procData_.init(caseData_, &commData_);
    if (caseData_->isRootRank()) {
        // rootである場合はさらに、トラジェクトリーデータ受信用に
        // 全粒子数分の配列を割当てる
        commData_.setAllMoleculeCount(procData_.getMoleculeCount());
        // データ出力用ファイルを開く。
        commData_.openOutputFiles();
    }
}


void MdDriver::finalize() {
    // 出力用ファイルを一通りクローズする
    commData_.closeOutputFiles();
}
