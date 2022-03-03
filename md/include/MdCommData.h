/*
 * CommData.h
 *
 *      Author: Hideo Takahashi
 */

#ifndef _MDCOMMDATA_H
#define _MDCOMMDATA_H

#include <GridIterator3d.h>
#include <Cell.h>
#include <LJParams.h>
#include <Logger.h>
#include <vector>
#include <iostream>
#include <fstream>

/*
 * トラジェクトリー出力用に分子のデータを授受するための構造体
 */
struct CommMoleculeTrajData {
    /*
     * 分子種別番号 (See LJParams.cpp)
     */
    int kind_;
    /*
     * 分子の通し番号
     */
    int serial_;
    /*
     * 座標 [Angstrom]
     */
    double rx_, ry_, rz_;
    /*
     * 速度 [Angstrom*fs^-1]
     */
    double vx_, vy_, vz_;
};

/*
 * デバッグ用に上記構造体を印字するためのオペレータ
 */
std::ostream &operator<<(std::ostream &os, const CommMoleculeTrajData &data);

/*
 * プロセス間の移転のために分子のデータを授受するための構造体
 */
struct CommMoleculeFullData {
    /*
     * 分子種別番号 (See LJParams.cpp)
     */
    int kind_;
    /*
     * 分子の通し番号
     */
    int serial_;
    /*
     * 座標 [Angstrom]
     */
    double rx_, ry_, rz_;
    /*
     * 速度×Δt [Angstrom]
     */
    double vdtx_, vdty_, vdtz_;
    /*
     * 加速度×Δt^2
     */
    double adt2x_, adt2y_, adt2z_;
};

/*
 * デバッグ用に上記構造体を印字するためのオペレータ
 */
std::ostream &operator<<(std::ostream &os, const CommMoleculeFullData &data);

struct CommMoleculePosData {
    /*
     * 分子種別番号 (See LJParams.cpp)
     */
    int kind_;
    /*
     * 座標 [Angstrom]
     */
    double rx_, ry_, rz_;
};

/*
 * デバッグ用に上記構造体を印字するためのオペレータ
 */
std::ostream &operator<<(std::ostream &os, const CommMoleculePosData &data);

/*
 * 通信バッファクラス
 */
class MdCommPeerBuffer {
public:
    /*
     * 通信相手のrank
     */
    int rank_;

    /*
     * こちらから送信する分子数
     */
    size_t send_count_;
    /*
     * 相手から受信する分子数
     */
    size_t recv_count_;
    /*
     * 送信データにつけるMPIのタグ値
     * 動作テストなどにおいて、MPIのプロセス総数が少ない場合、例えば2x2x2=8プロセスのような場合、
     * 左隣のプロセスと、右隣のプロセスが、周期境界条件によって同一のプロセスになってしまう。
     * そのような場合に、左向けに送出したデータと、右向けに送出したデータを、受信側で正しく区別
     * できるようにするためには、データを送り出す26の方位ごとに別々のタグ値を割り振る必要がある。
     */
    int tag_for_send_;
    /*
     * 受信データとして受け入れるタグ値
     */
    int tag_for_recv_;

    /*
     * 送信データの個数の配列
     * 受信した側で、受信した分子のデータを簡単にセルに分配できるように、送信側と受信側で全く
     * 同じ順序で境界に位置するセルについてループを回すものとして、一つ一つのセルごとに、
     * セルに属していた分子数をsend_count_per_cel に格納し、その個数の分子のデータをsend_atom_full_
     * 配列に加えていく。
     */
    std::vector<int> send_count_per_cell_;
    /*
     * 受信データの個数の配列
     */
    std::vector<int> recv_count_per_cell_;
    /*
     * プロセス間の分子の移転のための送信分子のデータ
     */
    std::vector <CommMoleculeFullData> send_molecule_full_;
    /*
     * プロセス間の分子の移転のための受信分子のデータ
     */
    std::vector <CommMoleculeFullData> recv_molecule_full_;

    std::vector <CommMoleculePosData> send_molecule_pos_;

    std::vector <CommMoleculePosData> recv_molecule_pos_;

    VectorXYZ offset;

    /*
     * 当バッファの相手先rankと送受信用のtag値を設定する。
     * 初期化の過程で呼ばれる。
     */
    void setRankAndTags(int rank, int tag_for_send, int tag_for_recv);

    /*
     * 送信用のタグ値を取得する
     */
    int tagForSend() const {
        return tag_for_send_;
    }

    /*
     * 受信用のタグ値を取得する
     */
    int tagForRecv() const {
        return tag_for_recv_;
    }

    /*
     * 分子の移転用の送信バッファをクリアする。
     */
    void clearSendMoleculeFullBuffer();

    void clearSendMoleculePosBuffer();

    /*
     * 引数のcellに属する全分子のデータを分子の移転用のバッファに追加する。
     */
    void addMoleculeFullDataFrom(Cell *cell);

    void addMoleculePosDataFrom(Cell *cell);

    /*
     * 送信分子数をベクターから取得して個数送信用の変数に格納する
     */
    void setMoleculeFullDataSendCount();

    void setMoleculePosDataSendCount();

    /*
     * セル別の受信分子数を合計して、これから受信する分子データの総数を算出し、
     * それに合わせて受信バッファのサイズを設定する。
     */
    void setMoleculeFullDataRecvBuffer();

    void setMoleculePosDataRecvBuffer();

    /*
     * プロセスをまたがる際の座標の補正量を設定する。
     */
    void setOffsetForSending(double x, double y, double z);
};

/*
 * 通信データクラス
 */
class MdCommData {
public:
    CaseData *caseData_;

    int my_rank_;

    int num_procs_;

    double total_uk_;

    double total_up_;

    double send_uk_;

    double send_up_;

    double recv_uk_;

    double recv_up_;

    // トラジェクトリーファイル
    std::fstream tfile_;

    // エネルギーファイル
    std::fstream efile_;

    // 各プロセスからrank=0プロセスに向けて分子の情報をトラジェクトリー出力用に送るためのベクター
    std::vector <CommMoleculeTrajData> send_molecule_traj_;

    // rank=0において、他のプロセスから送られてくる1プロセス分の分子の情報を受信するためのベクター
    std::vector <CommMoleculeTrajData> recv_molecule_traj_;

    // rank=0において、系の全分子の情報を保持するためのベクター
    std::vector <CommMoleculeTrajData> all_molecule_traj_;

    // 26方位の隣接プロセスに向けた送受信バッファ
    MdCommPeerBuffer peerBuffers_[3][3][3];

    // 初期化
    void init(CaseData *caseData);

    // 指定方位の隣接プロセスに向けた通信バッファオブジェクトを取得する
    MdCommPeerBuffer *bufferFor(const GridIndex3d &idx);

    // 引数のローカルセル内の全分子のデータをトラジェクトリー用の送信バッファに転記する
    void addTrajectoryDataFrom(Cell *cell);

    // rank=0において、自プロセスで送信バッファに格納したデータを、自プロセスの受信バッファに転記する
    void appendSendTrajectoryDataToRecvTrajectoryData();

    // rank=0において、トラジェクトリーデータ用の受信バッファを十分な量割り当てる
    void setRecvTrajectoryBufferSize(size_t count);

    // トラジェクトリー用の送信バッファを空にする
    //（カウンタをゼロにするだけでメモリの開放はしない）
    void clearSendTrajectory();

    // トラジェクトリー用の受信バッファを空にする
    //（カウンタをゼロにするだけでメモリの開放はしない）
    void clearRecvTrajectory();

    // 全分子のトラジェクトリー用データを保持するベクターの長さを設定する
    // この中でメモリ領域の割り当てが行われる。
    void setAllMoleculeCount(int all_atom_count);

    // root=0において、他のいずれかのプロセスから受信した、１プロセス分の分子データを
    // 個々の分子の通し番号に基づいて、全分子用の配列の該当箇所に転記する。
    void orderRecvTrajectoryToAllTrajectory();

    // 各種出力ファイルをオープンする。
    void openOutputFiles();

    // 各種出力ファイルをクローズする。
    void closeOutputFiles();

    // 全分子のトラジェクトリーをトラジェクトリーファイルに追記する。
    void writeTrajectory();

    // 総エネルギーをエネルギーファイルに追記する
    void writeTotalEnergy();
};


#endif /* MDCOMMDATA_H_ */
