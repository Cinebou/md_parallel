/*
 * Communicator.cpp
 *
 *      Author: Hideo Takahashi
 */

#include <MdCommunicator.h>
#include <GridIterator3d.h>
#include <Logger.h>

#include <mpi.h>

/*
 * MPIにユーザ定義の型の構造を登録して、識別コード(MPI_Datatype型の値)を発行してもらう。
 * その値を覚えておくための変数
 */
MPI_Datatype MdCommunicator::MPI_MOLECULE_FULL_DATA_TYPE;
MPI_Datatype MdCommunicator::MPI_MOLECULE_POS_DATA_TYPE;
MPI_Datatype MdCommunicator::MPI_MOLECULE_TRAJECTORY_DATA_TYPE;

void MdCommunicator::init(CaseData *caseData, MdCommData *commData) {
    caseData_ = caseData;
    commData_ = commData;

    GridIndex3d myIndex;
    // 自身のプロセス座標を取得する
    caseData_->setProcessIteratorForRank(&myIndex, caseData_->my_rank_);
    // 実在するプロセス座標の範囲を表現するRangeを用意しておく
    GridRange3d procGrid(0, 0, 0, caseData_->npx_ - 1, caseData_->npy_ - 1, caseData_->npz_ - 1);

    GridPeerIterator3d peerIt; // [0,0,0] - [2,2,2] の範囲を走査するイテレータ
    while (peerIt.next()) {
        // peerItの方角にいるプロセスを、ラップ抜きに考える
        // ここでの計算値は、まだ周期境界条件を考慮しておらず、下限を下回ったり
        // 上限を越えたりしていることがある。
        GridIndex3d otherIndex = myIndex + peerIt + GridIndex3d(-1, -1, -1);

        // 周期境界条件を加味した場合の相手プロセスのプロセス座標を求める。
        GridIndex3d wrappedIndex = procGrid.wrapIndex(otherIndex);

        // 両者に差がある場合には、こちらからあちらに粒子を送りつける際に
        // 座標を補正する必要がある。補正の必要の有無によらずに補正量を
        // 事前に計算しておく。補正量は26とおりの方角ごとに定まるので、
        // 方角別の通信バッファである peerBufferに補正量を格納しておく。

        // 補正量の値は、考えてください。
        double offset_x = (wrappedIndex.ix_ - otherIndex.ix_) * caseData_->plx_;
        double offset_y = (wrappedIndex.iy_ - otherIndex.iy_) * caseData_->ply_;
        double offset_z = (wrappedIndex.iz_ - otherIndex.iz_) * caseData_->plz_;
        //Logger::out << "" << std::endl;
        //Logger::out << "offset  " << offset_x << " " << offset_y << " " << offset_z << std::endl;
        ///Logger::out << "" << std::endl;

        // 相手のrankを求める
        int pr = caseData_->getRankForProcess(wrappedIndex);
        // 考察中の方角に向けてのpeerBufferを取得する
        MdCommPeerBuffer *peer = commData_->bufferFor(peerIt);

        // MPIでは、同時期に複数のプロセスからデータが到着する時に、送り主のrank番号を
        // 指定して、自分が受け取りたいデータを指定することができるが、同時期に同一の
        // プロセスから意味合いの違う複数のデータが立て続けに送られてくる場合には、
        // rankでは識別できない。そのような場合ではrankに加えてtag番号を自由に決めて
        // 指定し、rankとtagの値の対でもってデータを識別することができる。
        // 本プログラムに即して言うと、例えばテスト段階で 2x2x2 プロセス
        // の分割を行った場合、あるプロセスから見て、左隣のプロセスと右隣のプロセスは、
        // 周期境界条件により、同一のプロセスになる。そこで、送信の方位ごとにタグ番号
        // を決めておくことにする。右に送り出すときのtagと、右から受信する時のtagは
        // 異なることに注意。右から受信するデータは、相手から見れば、左に送り出した
        // データなので、右側からは、「左向きのデータ」が到着する。
        int dir = peerIt.ix_ * 9 + peerIt.iy_ * 3 + peerIt.iz_;
        peer->setRankAndTags(pr, dir, 26 - dir); // 26 is the magic number. (27-1)

        // rank, tagに加えて、さきほどの「補正量」もpeerに設定しておく。
        peer->setOffsetForSending(offset_x, offset_y, offset_z);

      //  Logger::out << "For direction : ["
        //            << peerIt.ix_ << "," << peerIt.iy_ << "," << peerIt.iz_
        //            << "]" << std::endl;
      //  Logger::out << "Peer Rank = " << pr << std::endl;
    }

    initMpiTypes();
}

void MdCommunicator::initMpiTypes() {
    /*
     * 粒子データをMPIで授受する時に使うstructの構造をMPIに登録して
     * MPIで送受信する時の配列の「データ型」として使えるようにする。
     */
    //Logger::out << "initMpiTypes" << std::endl;

    /*
     * MPI_Type_create_struct, MPI_Type_commit 関数を利用する。
     */
    // Full
    int count_full = 11;
    int blocklengths_full[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    MPI_Aint displacements_full[11];
    MPI_Datatype types_full[11] = {MPI_INT, MPI_INT, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE,
                                   MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE};
    MPI_Aint base_address_full;
    displacements_full[0] = offsetof(CommMoleculeFullData, kind_);
    displacements_full[1] = offsetof(CommMoleculeFullData, serial_);
    displacements_full[2] = offsetof(CommMoleculeFullData, rx_);
    displacements_full[3] = offsetof(CommMoleculeFullData, ry_);
    displacements_full[4] = offsetof(CommMoleculeFullData, rz_);
    displacements_full[5] = offsetof(CommMoleculeFullData, vdtx_);
    displacements_full[6] = offsetof(CommMoleculeFullData, vdty_);
    displacements_full[7] = offsetof(CommMoleculeFullData, vdtz_);
    displacements_full[8] = offsetof(CommMoleculeFullData, adt2x_);
    displacements_full[9] = offsetof(CommMoleculeFullData, adt2y_);
    displacements_full[10] = offsetof(CommMoleculeFullData, adt2z_);
    MPI_Type_create_struct(count_full, blocklengths_full, displacements_full, types_full,
                           &MdCommunicator::MPI_MOLECULE_FULL_DATA_TYPE);
    MPI_Type_commit(&MdCommunicator::MPI_MOLECULE_FULL_DATA_TYPE);

    // Pos
    int count_pos = 4;
    MPI_Datatype types_pos[4] = {MPI_INT, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE};
    int blocklengths_pos[4] = {1, 1, 1, 1};
    MPI_Aint displacements_pos[4];
    displacements_pos[0] = offsetof(CommMoleculePosData, kind_);
    displacements_pos[1] = offsetof(CommMoleculePosData, rx_);
    displacements_pos[2] = offsetof(CommMoleculePosData, ry_);
    displacements_pos[3] = offsetof(CommMoleculePosData, rz_);
    MPI_Type_create_struct(count_pos, blocklengths_pos, displacements_pos, types_pos,
                           &MdCommunicator::MPI_MOLECULE_POS_DATA_TYPE);
    MPI_Type_commit(&MdCommunicator::MPI_MOLECULE_POS_DATA_TYPE);

    // Traj
    int count_traj = 8;
    MPI_Datatype types_traj[8] = {MPI_INT, MPI_INT, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE,
                                  MPI_DOUBLE};
    int blocklengths_traj[8] = {1, 1, 1, 1, 1, 1, 1, 1};
    MPI_Aint displacements_traj[8];
    displacements_traj[0] = offsetof(CommMoleculeTrajData, kind_);
    displacements_traj[1] = offsetof(CommMoleculeTrajData, serial_);
    displacements_traj[2] = offsetof(CommMoleculeTrajData, rx_);
    displacements_traj[3] = offsetof(CommMoleculeTrajData, ry_);
    displacements_traj[4] = offsetof(CommMoleculeTrajData, rz_);
    displacements_traj[5] = offsetof(CommMoleculeTrajData, vx_);
    displacements_traj[6] = offsetof(CommMoleculeTrajData, vy_);
    displacements_traj[7] = offsetof(CommMoleculeTrajData, vz_);

    MPI_Type_create_struct(count_traj, blocklengths_traj, displacements_traj, types_traj,
                           &MdCommunicator::MPI_MOLECULE_TRAJECTORY_DATA_TYPE);
    MPI_Type_commit(&MdCommunicator::MPI_MOLECULE_TRAJECTORY_DATA_TYPE);

    //Logger::out << "initMpiTypes:done" << std::endl;
}

void MdCommunicator::exchangeMoleculeFullData() {
    /*
     * プロセス間の粒子の転移のための送受信を行う
     */
    /*
     * 送信と受信の対を26方位に対して行う。
     */
    MPI_Request reqs[26 + 26];//個々のsend/recvのための構造体の配列
    MPI_Status stats[26 + 26];

    GridPeerIterator3d pidx; // このイテレータは [1,1,1]を飛ばして [0,0,0] - [2,2,2] の間の26方位を発生する。

    int reqi = 0; //構造体のためのカウンタ
    while (pidx.next()) {
        // この方位のpeer buffer を取得する
        MdCommPeerBuffer *peer = commData_->bufferFor(pidx);
        peer->setMoleculeFullDataSendCount(); //送信分子数をsend_count_に格納
        // 方位が決まると、その方位のpeerに面しているセルの数が決まる。
        // こちらから送信する粒子の数は、総合計ではなく、セルごとの粒子数の配列として送る
        // 向こうからも、同じ長さの配列で、セル別の粒子数を送ってくる。
        // つまり、同じ長さの整数配列を相互に送り合うことになる。
        size_t cell_count = peer->send_count_per_cell_.size(); // 相手に面しているセルの数
        // 本メソッドが呼ばれる時点では、送信用の配列にはすでに値がつまっているが、
        // 受信用の配列は、領域すら準備されていない（初回の時点では）
        // そこで、受信用のバッファの長さを割り当てる。
        // この長さは毎回同じなので、初期化処理に移してもよい（大きな性能向上は望めないが）
        peer->recv_count_per_cell_.resize(cell_count);
        // 送り出したいデータが始まるアドレス
        int *send_count_addr = &peer->send_count_per_cell_.front();
        // 受信データを受け取る配列が待ち受けているアドレス
        int *recv_count_addr = &peer->recv_count_per_cell_.front();

        // デバッグ用のログの例を示すが、この箇所は呼び出し回数が多いのでリリース版では
        // #ifdefによる抑止が必須である。
        //Logger::out << "Exchanging migrating particle counts for " << cell_count;
        //Logger::out << " boundary cells with rank " << peer->rank_;
        ///Logger::out << " with tag " << peer->tagForSend() << "/" << peer->tagForRecv() << std::endl;

        MPI_Isend(send_count_addr,
                  cell_count,
                  MPI_INT,
                  peer->rank_,
                  peer->tagForSend(),
                  MPI_COMM_WORLD,
                  &reqs[reqi]);

        MPI_Irecv(recv_count_addr,
                  cell_count,
                  MPI_INT,
                  peer->rank_,
                  peer->tagForRecv(),
                  MPI_COMM_WORLD,
                  &reqs[reqi + 1]);
        reqi += 2;
    }

    MPI_Waitall(reqi, reqs, stats);
    /*
     * 上記で、相手との境界に面したセルごとに、お互いに送信予定の粒子数を、相互に伝えることができた。
     * 続いて、実際の粒子の座標データの授受に移る。
     */

    pidx.reset(); // 先ほど使ったイテレータを初期状態に戻す。
    reqi = 0;
    while (pidx.next()) {
        MdCommPeerBuffer *peer = commData_->bufferFor(pidx);
        // 送信する粒子数の合計値を求める
        //peer->setMoleculeFullDataSendCount();
        // 先に受信した粒子数の配列から、受信予定の総粒子数を求め、その数に合わせて
        // 粒子の座標データ用の受信バッファを用意する。
        peer->setMoleculeFullDataRecvBuffer(); // Note 'FullData'
        //Logger::out << "Exchaning molecules with rank " << peer->rank_;
        //Logger::out << " with tag " << peer->tagForSend() << "/" << peer->tagForRecv();
        //Logger::out << " sending " << peer->send_count_ << ", receiving " << peer->recv_count_ << std::endl;
        // 送信データの先頭アドレス
        CommMoleculeFullData *send_addr = &peer->send_molecule_full_.front();
        // 受信データを受け取る配列の先頭アドレス
        CommMoleculeFullData *recv_addr = &peer->recv_molecule_full_.front();

        // 通信のための適切な関数を呼ぶ
        MPI_Isend(send_addr,
                  peer->send_count_,
                  MdCommunicator::MPI_MOLECULE_FULL_DATA_TYPE,
                  peer->rank_,
                  peer->tagForSend(),
                  MPI_COMM_WORLD,
                  &reqs[reqi]);
        MPI_Irecv(recv_addr,
                  peer->recv_count_,
                  MdCommunicator::MPI_MOLECULE_FULL_DATA_TYPE,
                  peer->rank_,
                  peer->tagForRecv(),
                  MPI_COMM_WORLD,
                  &reqs[reqi + 1]);
        reqi += 2;
    }
    MPI_Waitall(reqi, reqs, stats);
    /*
     * 送受信が終わったので、送信バッファの内容はもう必要ない。
     * 送信バッファの内容を空にしておく。
     * バッファ用のメモリは、次回も使うので解放するわけではない。
     * カウンタをゼロに戻すだけ。
     */
    pidx.reset();
    while (pidx.next()) {
        commData_->bufferFor(pidx)->clearSendMoleculeFullBuffer();
    }

}

void MdCommunicator::sendTrajectroyDataToRoot() {
    //Logger::out << "sendTrajectroyDataToRoot" << std::endl;

    /*
     * 自身がrootの場合は扱いが違うので、自身がroot以外の場合にだけ送信する。
     */
    if (!caseData_->isRootRank()) {
        int send_count = commData_->send_molecule_traj_.size();
        CommMoleculeTrajData *send_data = &commData_->send_molecule_traj_.front();
      //  Logger::out << "Sending " << send_count
        //            << " molecules from rank " << caseData_->my_rank_ << std::endl;

        /*
        for(int k = 0; k < send_count; ++k){
          std::cout << " send_mol = " << send_data[k] << std::endl;
        }
        */

        /* データをルートrankに送る。粒子数を表す整数を送り、それに続いて粒子の座標データを送る */
        MPI_Send(&send_count,
                 1,
                 MPI_INT,
                 0,
                 0,
                 MPI_COMM_WORLD);

        MPI_Send(send_data,
                 send_count,
                 MdCommunicator::MPI_MOLECULE_TRAJECTORY_DATA_TYPE,
                 0,
                 0,
                 MPI_COMM_WORLD);

        /* 送信が済んだので、送信バッファを空にする */
        commData_->clearSendTrajectory();
    }
    //Logger::out << "sendTrajectoryDataToRoot: done" << std::endl;
}

void MdCommunicator::recvTrajectoryDataAtRoot() {
    // このメソッドは、rootでだけ呼ばれる
    assert(caseData_->isRootRank());

    //Logger::out << "recvTrajectroyDataToRoot" << std::endl;

    // 全rankのそれぞれから順にデータを受信する。
    GridIterator3d procIt(caseData_->allProcessesRange_);
    while (procIt.next()) {
        //MdCommPeerBuffer *peer = commData_->bufferFor(procIt);いらない
        //peer->setMoleculeFullDataRecvBuffer();

        int rank = caseData_->getRankForProcess(procIt);
        // root自身の分のデータは通信によらずに、単なるデータの転記で済ませる。
        if (rank == 0) {
            // データをもらってくる相手は rank == 0 つまり、自身である。
            // rootであっても、他のrank同様に、送信データバッファにデータが入っているので、
            // それを受信データバッファに転記する。
            commData_->appendSendTrajectoryDataToRecvTrajectoryData();
            // 送信を終えた場合と同様に、送信データバッファをクリアする
            commData_->clearSendTrajectory();
        } else {
            // データをもらってくる相手はルート以外なので、実際に通信をする。
            int recv_count = 0;
            MPI_Status status;
            MPI_Recv(&recv_count,
                     1,
                     MPI_INT,
                     rank,
                     0,
                     MPI_COMM_WORLD,
                     &status);

            commData_->recv_molecule_traj_.resize(recv_count);

            //  Logger::out << "Receiving " << recv_count
            //            << " molecules from rank " << caseData_->my_rank_ << std::endl;

            // データ数に続いて、粒子のデータを受信する。
            CommMoleculeTrajData *recv_data = &commData_->recv_molecule_traj_.front();
            MPI_Recv(recv_data,
                     recv_count,
                     MdCommunicator::MPI_MOLECULE_TRAJECTORY_DATA_TYPE,
                     rank,
                     0,
                     MPI_COMM_WORLD,
                     &status);

            /*
             if(caseData_->isRootRank()){
                for(int k = 0; k < recv_count; ++k){
                  Logger::out << " recv_mol = " << recv_data[k] << std::endl;
                }
             }
             */

        }
        // 受信を終えた場合と同様に、受信データをトラジェクトリ配列に分配する（粒子の通し番号順に）
        commData_->orderRecvTrajectoryToAllTrajectory();
        // 受信データをトラジェクトリ配列に格納したら、受信バッファをクリアする
        commData_->clearRecvTrajectory();
    }
    //Logger::out << "recvTrajectoryDataToRoot: done" << std::endl;
}

void MdCommunicator::exchangeMoleculePosData() {
    MPI_Request reqs[26 + 26];
    MPI_Status stats[26 + 26];
    GridPeerIterator3d pidx; // このイテレータは [1,1,1]を飛ばして [0,0,0] - [2,2,2] の間の26方位を発生する。
    int reqi = 0;
    while (pidx.next()) {
        // この方位のpeer buffer を取得する
        MdCommPeerBuffer *peer = commData_->bufferFor(pidx);
        peer->setMoleculePosDataSendCount();
        // 方位が決まると、その方位のpeerに面しているセルの数が決まる。
        // こちらから送信する粒子の数は、総合計ではなく、セルごとの粒子数の配列として送る
        // 向こうからも、同じ長さの配列で、セル別の粒子数を送ってくる。
        // つまり、同じ長さの整数配列を相互に送り合うことになる。
        size_t cell_count = peer->send_count_per_cell_.size();
        // 本メソッドが呼ばれる時点では、送信用の配列にはすでに値がつまっているが、
        // 受信用の配列は、領域すら準備されていない（初回の時点では）
        // そこで、受信用のバッファの長さを割り当てる。
        // この長さは毎回同じなので、初期化処理に移してもよい（大きな性能向上は望めないが）
        peer->recv_count_per_cell_.resize(cell_count);
        // 送り出したいデータが始まるアドレス
        int *send_count_addr = &peer->send_count_per_cell_.front();
        // 受信データを受け取る配列が待ち受けているアドレス
        int *recv_count_addr = &peer->recv_count_per_cell_.front();

        // デバッグ用のログの例を示すが、この箇所は呼び出し回数が多いのでリリース版では
        // #ifdefによる抑止が必須である。
        //  Logger::out << "Exchanging migrating particle counts for " << cell_count;
        //  Logger::out << " boundary cells with rank " << peer->rank_;
        //  Logger::out << " with tag " << peer->tagForSend() << "/" << peer->tagForRecv()<<std::endl;

        // 必要な通信をここで行う。
        MPI_Isend(send_count_addr,
                  cell_count,
                  MPI_INT,
                  peer->rank_,
                  peer->tagForSend(),
                  MPI_COMM_WORLD,
                  &reqs[reqi]);
        MPI_Irecv(recv_count_addr,
                  cell_count,
                  MPI_INT,
                  peer->rank_,
                  peer->tagForRecv(),
                  MPI_COMM_WORLD,
                  &reqs[reqi + 1]);
        reqi += 2;
    }
    MPI_Waitall(reqi, reqs, stats);
    /*
     * 上記で、相手との境界に面したセルごとに、お互いに送信予定の粒子数を、相互に伝えることができた。
     * 続いて、実際の粒子の座標データの授受に移る。
     */

    pidx.reset(); // 先ほど使ったイテレータを初期状態に戻す。
    reqi = 0;
    while (pidx.next()) {
        MdCommPeerBuffer *peer = commData_->bufferFor(pidx);
        // 送信する粒子数の合計値を求める
        peer->setMoleculePosDataSendCount();
        // 先に受信した粒子数の配列から、受信予定の総粒子数を求め、その数に合わせて
        // 粒子の座標データ用の受信バッファを用意する。
        peer->setMoleculePosDataRecvBuffer(); // Note 'PosData'
        //  Logger::out << "Exchaning molecules with rank " << peer->rank_;
        //  Logger::out << " with tag " << peer->tagForSend() << "/" << peer->tagForRecv();
        //  Logger::out << " sending " << peer->send_count_ << ", receiving " << peer->recv_count_ << std::endl;
        // 送信データの先頭アドレス
        CommMoleculePosData *send_addr = &peer->send_molecule_pos_.front();
        // 受信データを受け取る配列の先頭アドレス
        CommMoleculePosData *recv_addr = &peer->recv_molecule_pos_.front();

        //  Logger::out <<"one direction"<<std::endl;
        //for (int k = 0; k < peer->send_count_; k++) {
        //    Logger::out << peer->send_molecule_pos_[k] << std::endl;
        //}

        // 通信のための適切な関数を呼ぶ
        MPI_Isend(send_addr,
                  peer->send_count_,
                  MPI_MOLECULE_POS_DATA_TYPE,
                  peer->rank_,
                  peer->tagForSend(),
                  MPI_COMM_WORLD,
                  &reqs[reqi]);

        MPI_Irecv(recv_addr,
                  peer->recv_count_,
                  MPI_MOLECULE_POS_DATA_TYPE,
                  peer->rank_,
                  peer->tagForRecv(),
                  MPI_COMM_WORLD,
                  &reqs[reqi + 1]);
        reqi += 2;
    }

    MPI_Waitall(reqi, reqs, stats);
    /*
     * 送受信が終わったので、送信バッファの内容はもう必要ない。
     * 送信バッファの内容を空にしておく。
     * バッファ用のメモリは、次回も使うので解放するわけではない。
     * カウンタをゼロに戻すだけ。
     */
    pidx.reset();
    while (pidx.next()) {
        commData_->bufferFor(pidx)->clearSendMoleculePosBuffer();
    }
    //Logger::out << "Pos comm finish" << std::endl;
}

void MdCommunicator::calcEnergy() {
    MPI_Reduce(&commData_->send_uk_, &commData_->recv_uk_, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&commData_->send_up_, &commData_->recv_up_, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    commData_->total_uk_ = commData_->recv_uk_;
    commData_->total_up_ = commData_->recv_up_;
    commData_->recv_up_ = 0;
    commData_->recv_uk_ = 0;
    //Logger::out << "recvEnergyDataAtRoot" << std::endl;
}
