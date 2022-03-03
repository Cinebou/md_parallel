#include <MdDriver_sp.h>
#include <string.h>

MdDriver_sp::~MdDriver_sp()
{
    // empty
}

MdDriver_sp::MdDriver_sp()
{
    // empty
}

void MdDriver_sp::init(CaseData *caseData)
{
    // caseDataは初期化済みのものが渡ってくる
    caseData_ = caseData;
    // commData（通信バッファ）を初期化する
    commData_.init(caseData);
    // communicator（通信機能）を初期化する
    communicator_.init(caseData_, &commData_);
    // 本プロセスの保持する物理計算のデータを初期化する（データファイル読み込みはここで起きる）
    procData_.init(caseData_, &commData_);

    // SP版では常にroot rank.
    assert(caseData_->isRootRank());

    // rootである場合はさらに、トラジェクトリーデータ受信用に
    // 全粒子数分の配列を割当てる
    commData_.setAllMoleculeCount(procData_.getMoleculeCount());
    // データ出力用ファイルを開く。
    commData_.openOutputFiles();
}

void MdDriver_sp::doStep()
{
    Logger::out << "MdDriver_sp::doStep  t = " << caseData_->t_ << std::endl;

    //a(t)とv(t)からv(t+1/2Δt)を計算
    procData_.updateVelocityHalf();
    // 位置を更新する
    procData_.updatePosition();



    // HINT: some steps are skipped here. add them.

    // 分子間力を計算する
    procData_.calcForce();

    //a(t)とv(t)からv(t+1/2Δt)を計算
    procData_.updateVelocityHalf();

    // 保有している全粒子の座標データをトラジェクトリー送信バッファに転記する
    procData_.exportTrajectoryData();
    
    communicator_.recvTrajectoryDataAtRoot();
    // 全粒子分のデータをファイルに書く。
    procData_.writeTrajectoryData();

    // 時間発展の回が１ステップ進んだことを記録する
    caseData_->incrementStep();

    Logger::out << "MdDriver_sp::doStep:end" << std::endl;
}

void MdDriver_sp::finalize()
{
    // 出力用ファイルを一通りクローズする
    commData_.closeOutputFiles();
}
