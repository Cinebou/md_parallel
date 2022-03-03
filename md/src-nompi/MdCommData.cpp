/*
 * CommData.cpp
 *
 *      Author: Hideo Takahashi
 */

#include <MdCommData.h>
#include <IoException.h>

std::ostream &operator<<(std::ostream &os, const CommMoleculeTrajData &data) {
    os << "[ kind : " << data.kind_;
    os << ", serial : " << data.serial_;
    os << ", r = (" << data.rx_ << "," << data.ry_ << "," << data.rz_ << ")";
    os << ", v = (" << data.vx_ << "," << data.vy_ << "," << data.vz_ << ") ]";
    return os;
}

std::ostream &operator<<(std::ostream &os, const CommMoleculeFullData &data) {
    os << "[ kind : " << data.kind_;
    os << ", serial : " << data.serial_;
    os << ", r = (" << data.rx_ << "," << data.ry_ << "," << data.rz_ << ")";
    os << ", vdt = (" << data.vdtx_ << "," << data.vdty_ << "," << data.vdtz_ << ") ]";
    return os;
}

std::ostream &operator<<(std::ostream &os, const CommMoleculePosData &data) {
    os << "[ kind : " << data.kind_;
    os << ", r = (" << data.rx_ << "," << data.ry_ << "," << data.rz_ << ") ]";
    return os;
}

void MdCommPeerBuffer::setRankAndTags(int rank, int tag_for_send, int tag_for_recv) {
    rank_ = rank;
    tag_for_send_ = tag_for_send;
    tag_for_recv_ = tag_for_recv;
}

void MdCommPeerBuffer::clearSendMoleculeFullBuffer() {
    send_molecule_full_.clear();
    send_count_per_cell_.clear();
    send_count_ = 0;
}

void MdCommPeerBuffer::clearSendMoleculePosBuffer() {
    send_molecule_pos_.clear();
    send_count_per_cell_.clear();
    send_count_ = 0;
}

void MdCommPeerBuffer::addMoleculeFullDataFrom(Cell *cell) {
    int count = 0;
    // cellに含まれる全粒子の情報をsend_molecule_full_ベクターに追加していく
    for (Particle *p = cell->getParticleListHead(); p != NULL; p = p->next_) {
        // ベクターの要素の型であるCommMoleculeFullData構造体の変数dataに一旦値を格納してから
        // push_backでベクターに追加する。
        CommMoleculeFullData data;
        data.kind_ = p->kind_;
        data.serial_ = p->serial_;
        // HINT: To implement periodic boundary condition behavior,
        // consider adjusting the position sent out here.
        // Make the position values suitable for the recipient.
        data.rx_ = p->pos_.x_ + offset.x_;
        data.ry_ = p->pos_.y_ + offset.y_;
        data.rz_ = p->pos_.z_ + offset.z_;
        //Logger::out << "Sending molecule " << p->serial_
        //            << " at " << p->pos_ << std::endl;
        data.vdtx_ = p->vel_dt_.x_;
        data.vdty_ = p->vel_dt_.y_;
        data.vdtz_ = p->vel_dt_.z_;
        send_molecule_full_.push_back(data);
        // 送り出す粒子の数を数える
        ++count;
    }
    // このセルに由来する粒子の数を、送出粒子数のベクターに書き込む
    send_count_per_cell_.push_back(count);
    // 送信する総粒子数のカウンタにも加える
    send_count_ += count;
    if (count > 0) {
        //Logger::out << "sending " << count << " molecules." << std::endl;
    }
}

void MdCommPeerBuffer::addMoleculePosDataFrom(Cell *cell) {
    int count = 0;
    // cellに含まれる全粒子の情報をsend_molecule_pos_ベクターに追加していく
    for (Particle *p = cell->getParticleListHead(); p != NULL; p = p->next_) {
        // ベクターの要素の型であるCommMoleculePosData構造体の変数dataに一旦値を格納してから
        // push_backでベクターに追加する。
        CommMoleculePosData data;
        data.kind_ = p->kind_;
        // HINT: To implement periodic boundary condition behavior,
        // consider adjusting the position sent out here.
        // Make the position values suitable for the recipient.
        //Logger::out <<"pos before modified"<< p->pos_ << std::endl;
        data.rx_ = p->pos_.x_ + offset.x_;
        data.ry_ = p->pos_.y_ + offset.y_;
        data.rz_ = p->pos_.z_ + offset.z_;
        //Logger::out << "Sending molecule " << p->serial_
        //            << " at " << p->pos_ << std::endl;
        send_molecule_pos_.push_back(data);
        // 送り出す粒子の数を数える
        ++count;
    }
    // このセルに由来する粒子の数を、送出粒子数のベクターに書き込む
    send_count_per_cell_.push_back(count);
    // 送信する総粒子数のカウンタにも加える
    send_count_ += count;
    if (count > 0) {
        //Logger::out << "sending " << count << " molecules." << std::endl;
    }
}

void MdCommPeerBuffer::setMoleculeFullDataSendCount() {
#ifdef DEBUG_MPI
  //  Logger::out << "send_count " << send_atom_full_.size() << std::endl;
#endif
    send_count_ = send_molecule_full_.size();
}

void MdCommPeerBuffer::setMoleculePosDataSendCount() {
#ifdef DEBUG_MPI
    //Logger::out << "send_count " << send_atom_pos_.size() << std::endl;
#endif
    send_count_ = send_molecule_pos_.size();
}

void MdCommPeerBuffer::setMoleculeFullDataRecvBuffer() {
    int count = 0;
    // STLの用意するイテレータを積極的に活用する。
    std::vector<int>::iterator it;
    for (it = recv_count_per_cell_.begin();
         it != recv_count_per_cell_.end();
         ++it) {
        count += *it;
    }
#ifdef DEBUG_MPI
    //Logger::out << "receiving molecule full count sum : " << count << std::endl;
#endif
    recv_molecule_full_.resize(count);
    recv_count_ = count;
}

void MdCommPeerBuffer::setMoleculePosDataRecvBuffer() {
    int count = 0;
    // STLの用意するイテレータを積極的に活用する。
    std::vector<int>::iterator it;
    for (it = recv_count_per_cell_.begin();
         it != recv_count_per_cell_.end();
         ++it) {
        count += *it;
    }
#ifdef DEBUG_MPI
    //Logger::out << "receiving molecule pos count sum : " << count << std::endl;
#endif
    recv_molecule_pos_.resize(count);
    recv_count_ = count;
}

void MdCommPeerBuffer::setOffsetForSending(double x, double y, double z) {
    offset = VectorXYZ(x, y, z);
}

void MdCommData::init(CaseData *caseData) {
    caseData_ = caseData;
    total_uk_ = 0;
    total_up_ = 0;
    send_uk_ = 0;
    send_up_ = 0;
    recv_uk_ = 0;
    recv_up_ = 0;
}

MdCommPeerBuffer *MdCommData::bufferFor(const GridIndex3d &idx) {
    assert(idx.ix_ >= 0 && idx.ix_ < 3);
    assert(idx.iy_ >= 0 && idx.iy_ < 3);
    assert(idx.iz_ >= 0 && idx.iz_ < 3);
    return &peerBuffers_[idx.ix_][idx.iy_][idx.iz_];
}

void MdCommData::addTrajectoryDataFrom(Cell *cell) {
    Particle *p;
    double inv_delta_t = 1.0 / caseData_->delta_t_;
    // cellに含まれる全粒子をトラジェクトリー用の送信バッファに書き込む
    for (p = cell->getParticleListHead(); p != NULL; p = p->next_) {
        // 一旦変数dataに格納してからベクターにpush_backで追記する。
        CommMoleculeTrajData data;
        data.kind_ = p->kind_;
        data.serial_ = p->serial_;
        data.rx_ = p->pos_.x_;
        data.ry_ = p->pos_.y_;
        data.rz_ = p->pos_.z_;
        data.vx_ = p->vel_dt_.x_ * inv_delta_t;
        data.vy_ = p->vel_dt_.y_ * inv_delta_t;
        data.vz_ = p->vel_dt_.z_ * inv_delta_t;
        send_molecule_traj_.push_back(data);
    }
}

void MdCommData::appendSendTrajectoryDataToRecvTrajectoryData() {
    // rank=0 において、自身の保有する全粒子をroot rankに送信する代わりに
    // root rank向けの送信バッファの内容を、自身の受信バッファに転記する。
    assert(caseData_->isRootRank());
    //Logger::out << "MdCommData::appendSendTrajectoryDataToRecvTrajectoryData" << std::endl;
    size_t i, n;
    n = send_molecule_traj_.size();
    ///Logger::out << "appending " << n << " molecules" << std::endl;
    for (i = 0; i < n; i++) {
        recv_molecule_traj_.push_back(send_molecule_traj_[i]);
    }
}

void MdCommData::setRecvTrajectoryBufferSize(size_t count) {
    recv_molecule_traj_.resize(count);
}

void MdCommData::clearSendTrajectory() {
    send_molecule_traj_.clear();
}

void MdCommData::clearRecvTrajectory() {
    recv_molecule_traj_.clear();
}

void MdCommData::setAllMoleculeCount(int all_molecule_count) {
    all_molecule_traj_.resize(all_molecule_count);
}


void MdCommData::orderRecvTrajectoryToAllTrajectory() {
    assert(caseData_->isRootRank());
    // トラジェクトリー用に1プロセスずつ粒子のデータを送ってもらうが、粒子のシリアル番号
    // としては、でたらめな順番でデータが到着する。それらのデータを、シリアル番号順の
    // 配列に移し替える。受信した粒子のデータにはそれぞれシリアル番号が書いてあるので
    // 結果を受け取る配列の、シリアル番号の位置に、個々の粒子のデータを転記すればよい。
    // 普通に整数変数でループを回してもよいが、ここではSTLのイテレータを使ってループを書いている
    //Logger::out << "MdCommData::orderRecvTrajectoryToAllTrajectory" << std::endl;
    std::vector<CommMoleculeTrajData>::iterator it;
    for (it = recv_molecule_traj_.begin(); it != recv_molecule_traj_.end(); ++it) {
        size_t serial = it->serial_;
        assert(serial < all_molecule_traj_.size());
        //Logger::out << "molecule[" << serial << "] = " << *it << std::endl;
        all_molecule_traj_[serial] = *it;
    }
}

void MdCommData::openOutputFiles() {
    const char *traj_file_name = caseData_->trajectory_file_path_.c_str();
    const char *energy_file_name = caseData_->energy_file_path_.c_str();
    tfile_.open(traj_file_name, std::ios::out);
    efile_.open(energy_file_name, std::ios::out);
    if (!tfile_.is_open()) {
        throw IoException(__FILE__, __LINE__, traj_file_name);
    }
    if (!efile_.is_open()) {
        throw IoException(__FILE__, __LINE__, energy_file_name);
    }
}

void MdCommData::closeOutputFiles() {
    tfile_.close();
    efile_.close();
}

void MdCommData::writeTrajectory() {
    tfile_ << all_molecule_traj_.size() << std::endl;
    tfile_ << "# Output of mdlj\n";
    std::vector<CommMoleculeTrajData>::iterator it;
    // 全粒子の位置・速度をシリアル番号順に、ファイルに出力する
    for (it = all_molecule_traj_.begin(); it != all_molecule_traj_.end(); ++it) {
        tfile_ << LJParams::SOURCE_PARAMS_[it->kind_].label_;
        tfile_ << " " << it->rx_ << " " << it->ry_ << " " << it->rz_;
        tfile_ << " " << it->vx_ << " " << it->vy_ << " " << it->vz_ << std::endl;
    }
}

void MdCommData::writeTotalEnergy() {
    efile_ << caseData_->t_ << " " << total_uk_ << " " << total_up_ << " " << total_uk_ + total_up_ << std::endl;
    total_uk_ = 0;
    total_up_ = 0;
}
