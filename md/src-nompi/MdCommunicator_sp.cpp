#include <MdCommunicator_sp.h>

void MdCommunicator_sp::init(CaseData *caseData, MdCommData *commData)
{
    caseData_ = caseData;
    commData_ = commData;
}

void MdCommunicator_sp::exchangeMoleculeFullData()
{
    /*
     * 26方位別の送信バッファに格納された分子のデータを、反対方向からの受信バッファに転記する
     * 
     * ただし、その際に座標が範囲内に収まるように更新する
     */
    GridPeerIterator3d peerIt;
    while (peerIt.next())
    {
        MdCommPeerBuffer *sender_peer = commData_->bufferFor(peerIt);
        MdCommPeerBuffer *receiver_peer = commData_->bufferFor(GridIndex3d(2, 2, 2) - peerIt);
         // ひとまずコピーし、その後にコピー元のバッファを空にしておく
        receiver_peer->recv_molecule_full_ = sender_peer->send_molecule_full_;
        receiver_peer->recv_count_per_cell_ = sender_peer->send_count_per_cell_;
        sender_peer->clearSendMoleculeFullBuffer();
        // 座標が逸脱していたら補正する
        double lx = caseData_->lx_;
        double ly = caseData_->ly_;
        double lz = caseData_->lz_;
        size_t n = receiver_peer->recv_molecule_full_.size();
        size_t i;
        for (i = 0; i < n; i++) {
            double rx = receiver_peer->recv_molecule_full_[i].rx_;
            double ry = receiver_peer->recv_molecule_full_[i].ry_;
            double rz = receiver_peer->recv_molecule_full_[i].rz_;
            if (rx < 0) {
                rx += lx;
            } else if (rx >= lx) {
                rx -= lx;
            }
            if (ry < 0) {
                ry += ly;
            } else if (ry >= ly) {
                ry -= ly;
            }
            if (rz < 0) {
                rz += lz;
            } else if (rz >= lz) {
                rz -= lz;
            }
            receiver_peer->recv_molecule_full_[i].rx_ = rx;
            receiver_peer->recv_molecule_full_[i].ry_ = ry;
            receiver_peer->recv_molecule_full_[i].rz_ = rz;
        }
    }
}

void MdCommunicator_sp::recvTrajectoryDataAtRoot()
{
    // root自身の分のデータは通信によらずに、単なるデータの転記で済ませる。
    // 送信データバッファにデータが入っているので、
    // それを受信データバッファに転記する。
    commData_->appendSendTrajectoryDataToRecvTrajectoryData();
    // 送信を終えた場合と同様に、送信データバッファをクリアする
    commData_->clearSendTrajectory();
    // 受信を終えた場合と同様に、受信データをトラジェクトリ配列に分配する（粒子の通し番号順に）
    commData_->orderRecvTrajectoryToAllTrajectory();
    // 分配を終えたので、受信バッファをクリアする。
    commData_->clearRecvTrajectory();
}
