/*
 * ProcData.cpp
 *
 *      Author: Hideo Takahashi
 */

#include <MdProcData.h>
#include <FileReader.h>
#include <LJParams.h>
#include <Logger.h>
#include <iostream>
#include <cassert>

MdProcData::MdProcData() {
    cells_ = NULL;
}

MdProcData::~MdProcData() {
    if (cells_) {
        // 配列用の new で確保した領域なので、配列用の delete を使う
        delete[] cells_;
    }
}


void MdProcData::init(CaseData *caseData, MdCommData *commData) {
    caseData_ = caseData;
    commData_ = commData;
    // セルを割り当てる
    allocateCells();
    // ループに使うレンジオブジェクトを一通り作成する
    initRanges();
    // 各セルを初期化する
    initCells();
    // 初期状態ファイルを読み込む
    readInitialStateFile();
}

void MdProcData::allocateCells() {
    // 自身が計算を担当するカットオフセルの数を取得する
    int ncx = caseData_->ncx_;
    int ncy = caseData_->ncy_;
    int ncz = caseData_->ncz_;

    // セルオブジェクトとしてはローカルセルと周辺セルの合計分を割り当てる
    acx_ = ncx + 2;
    acy_ = ncy + 2;
    acz_ = ncz + 2;

    int num_cells = acx_ * acy_ * acz_;
    // セルのメモリを割り当てる。メモリ不足の場合は、（このnewに限らず） std::bad_alloc 例外が挙がる。
    cells_ = new Cell[num_cells];
    //Logger::out << "Number of cells allocated is " << num_cells << std::endl;
    //Logger::out << "sizeof(Cell) = " << sizeof(Cell) << std::endl;
    //Logger::out << "Array allocated. size is " << sizeof(Cell)*num_cells << " bytes." << std::endl;
}

/*
 * 以下の4つの関数は、26方位の周辺セル、表面セルに対応するセル座標の範囲を計算するための補助関数。
 *
 * 各カットオフセルは整数の三次元座標を持つ。座標の値は [0..cnx+1][0..cny+1][0..cnz+1]
 * 0は周辺セル、1..cnがローカルセル、cn+1が再び周辺セルの分である。
 *
 * 周辺セル、表面セルは面、辺、頂点に対応して存在する。面が6個、辺が12個、頂点が8個。
 * 面に対応する表面/周辺セルは2次元の広がりを持つ
 * 辺に対応する表面/周辺セルは1次元の広がりを持つ
 * 頂点に対応する表面/周辺セルは単一のセルである
 * いずれのケースもカットオフセルのx,y,z座標のレンジ（範囲）で表現できる。
 * そのレンジの下限と上限を、x,y,z座標別に事前に計算しておくための関数。
 *
 * x,y,zのどの座標軸に沿っても、同様の計算をする。
 * 26の方位を表す [0,0,0] - [2,2,2] の値は、座標ごとに見ると、次の意味を持つ
 * 0 : 自プロセスよりも、相手のプロセス座標が小さい
 * 1 : 自プロセスと相手のプロセス座標が等しい
 * 2 : 自プロセスよりも、相手のプロセス座標が大きい
 * 以下の関数では、この0,1,2の値を引数 i に受け取っている。
 * これが一つの座標だけ見たときの「方位」である。
 *
 * もう一つの引数nは、その座標軸に沿ったローカルセルの個数（自プロセスが計算を担当するセルの個数）である。
 */

/*
 * 方位 i に対する、「周辺」セルレンジの「下限」を求める。
 */
int surrounding_low(int i, int n) {
    switch (i) {
        case 0:
            /* 自身よりもプロセス座標が小さい方位の周辺セルの座標範囲は 0..0 なので、その下限は0 */
            return 0;
        case 1:
            /* 自身とプロセス座標が等しい方位の周辺セルの座標範囲は 1..n なので、その下限は1 */
            return 1;
        case 2:
            /* 自身よりもプロセス座標が大きい方位の周辺セルの座標範囲は n+1..n+1 なので、その下限はn+1 */
            return n + 1;
        default:
            /* iの値として0,1,2以外が渡された */
            assert(0);
            return -1;
    }
}

/*
 * 方位 i に対する、「周辺」セルレンジの「上限」を求める。
 */
int surrounding_high(int i, int n) {
    switch (i) {
        case 0:
            /* 自身よりもプロセス座標が小さい方位の周辺セルの座標範囲は 0..0 なので、その上限は0 */
            return 0;
        case 1:
            /* 自身とプロセス座標が等しい方位の周辺セルの座標範囲は 1..n なので、その上限はn */
            return n;
        case 2:
            /* 自身よりもプロセス座標が大きい方位の周辺セルの座標範囲は n+1..n+1 なので、その上限はn+1 */
            return n + 1;
        default:
            /* iの値として0,1,2以外が渡された */
            assert(0);
            return -1;
    }
}

/*
 * 方位 i に対する、「表面」セルレンジの「下限」を求める。
 */
int surface_low(int i, int n) {
    switch (i) {
        case 0:
            /* 自身よりもプロセス座標が小さい方位の表面セルの座標範囲は 1..1 なので、その下限は1 */
            return 1;
        case 1:
            /* 自身とプロセス座標が等しい方位の表面セルの座標範囲は 1..n なので、その下限は1 */
            return 1;
        case 2:
            /* 自身よりもプロセス座標が大きい方位の表面セルの座標範囲は n..n なので、その下限はn */
            return n;
        default:
            /* iの値として0,1,2以外が渡された */
            assert(0);
            return -1;
    }
}

/*
 * 方位 i に対する、「表面」セルレンジの「上限」を求める。
 */
int surface_high(int i, int n) {
    switch (i) {
        case 0:
            /* 自身よりもプロセス座標が小さい方位の表面セルの座標範囲は 1..1 なので、その上限は1 */
            return 1;
        case 1:
            /* 自身とプロセス座標が等しい方位の表面セルの座標範囲は 1..n なので、その上限はn */
            return n;
        case 2:
            /* 自身よりもプロセス座標が大きい方位の表面セルの座標範囲は n..n なので、その上限はn */
            return n;
        default:
            /* iの値として0,1,2以外が渡された */
            assert(0);
            return -1;
    }
}

void MdProcData::initRanges() {
    /*
     * 方向別のローカルセルの数
     */
    int ncx = caseData_->ncx_;
    int ncy = caseData_->ncy_;
    int ncz = caseData_->ncz_;

    /*
     * (1) ローカル/周辺合わせた全てのセルに対するレンジを設定する
     */
    allCellsRange_.setRange(0, 0, 0, ncx + 1, ncy + 1, ncz + 1);

    /*
     * (2) 全てのローカルセルに対するレンジを設定する
     */
    localCellsRange_.setRange(1, 1, 1, ncx, ncy, ncz);

    /*
     * (3) 26方位（使わない[1,1,1]も含めて27方位）の周辺セル、表面セルのレンジを設定する
     */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                // 周辺セルの各座標成分ごとに、下限と上限を指定する
                surroundingRanges_[i][j][k].setRange(
                        surrounding_low(i, ncx), surrounding_low(j, ncy), surrounding_low(k, ncz),
                        surrounding_high(i, ncx), surrounding_high(j, ncy), surrounding_high(k, ncz));
                // 表面セルの各座標成分ごとに、下限と上限を指定する
                surfaceRanges_[i][j][k].setRange(
                        surface_low(i, ncx), surface_low(j, ncy), surface_low(k, ncz),
                        surface_high(i, ncx), surface_high(j, ncy), surface_high(k, ncz));
            }
        }
    }
}

void MdProcData::initCells() {
    // set the box of all cells.
    GridIterator3d it(allCellsRange_);
    while (it.next()) {
        Cell *cell = cellFor(it);
        BoxXYZ cellBox;
        caseData_->setBoxForCell(&cellBox, it);
        cell->setBox(cellBox);
    }

    // set the neighbors of all local cells.
    it.set(localCellsRange_);
    while (it.next()) {
        Cell *cell = cellFor(it);
        GridDirIterator3d ofs;
        while (ofs.next()) {
            GridIndex3d it3(it + ofs);
            Cell *neighborCell = cellFor(it3);
            cell->setNeighborCell(1 + ofs.ix_, 1 + ofs.iy_, 1 + ofs.iz_, neighborCell);
        }
    }
}

void MdProcData::setCellIndexForPos(GridIndex3d *cellIdx, const VectorXYZ &pos) const {
    // 自身のプロセス分割セルにおける相対座標を求める
    VectorXYZ offset = pos - caseData_->localBox_.p1_;
    // floor は cmath で定義された関数
    cellIdx->ix_ = 1 + floor(offset.x_ / caseData_->clx_);
    cellIdx->iy_ = 1 + floor(offset.y_ / caseData_->cly_);
    cellIdx->iz_ = 1 + floor(offset.z_ / caseData_->clz_);
}

void MdProcData::readInitialStateFile() {
    FileReader rdr;
    // ファイルをオープンする
    rdr.open(caseData_->initial_state_file_path_);
    // 分子の通し番号
    int serial = 0;
    // 行がなくなると readLineは false を返す。
    while (rdr.readLine()) {
        std::string name;
        // 分子の名前を読み込む
        rdr.readString(name, "Molecule type");
        // 分子の名前をサポートしている分子の種類の名前の一覧から検索し、整数の分子種別番号を返す。
        // 見つからなければ DataException が挙がる。
        int kind = LJParams::nameToMoleculeKind(name.c_str());
        VectorXYZ pos, vel;
        // 座標と速度を読み込む
        rdr.readDouble(pos.x_, "x");
        rdr.readDouble(pos.y_, "y");
        rdr.readDouble(pos.z_, "z");
        rdr.readDouble(vel.x_, "u");
        rdr.readDouble(vel.y_, "v");
        rdr.readDouble(vel.z_, "w");
        // このファイルはシミュレーション対象の全分子を含んでいるので、
        // 座標の範囲が自身のプロセス分割セルに属する場合にだけ、取り込む。
        if (caseData_->localBox_.contains(pos)) {
            // この分子は当プロセスの担当範囲に含まれる。
            GridIndex3d cid;
            // 座標に基づいて、データを保持すべきカットオフセルのセル座標を求める
            setCellIndexForPos(&cid, pos);
            // セル座標から、 cell オブジェクトを取得する
            Cell *cell = cellFor(cid);
            // 分子を保持する Particleオブジェクトのメモリを割り当てる
            Particle *p = allocateParticle();
            // 分子の情報を書き込む
            p->kind_ = kind;                        // 種別
            p->serial_ = serial;                    // 通し番号
            p->pos_ = pos;                          // 初期座標
            p->vel_dt_ = vel * caseData_->delta_t_; // 初速度
            // cellの持つ粒子リストに追加する
            cell->addParticle(p);
        }
        // 粒子の通し番号をインクリメント
        serial++;
    }
    total_molecule_count_ = serial;
    // ファイルをクローズする
    rdr.close();
}

Particle *MdProcData::allocateParticle() {
    Particle *p;
    // 空きparticleリストに在庫があるか？
    if (freeParticleList_.isEmpty()) {
        // 在庫はないので新規に割り当てる。
        p = new Particle;
    } else {
        // 在庫があるので、空き particle を一つ取り出す。
        p = freeParticleList_.removeTail();
    }
    return p;
}

void MdProcData::stockAllParticlesInList(ParticleList *pList) {
    // 空きparticleリストに全てのparticleを引き渡す。
    // その結果として pList は空になる。
    pList->moveAllTo(&freeParticleList_);
}

void MdProcData::clearSurroundingCells() {
    // 26方位の配列座標[0,0,0]..[2,2,2]を発生するイテレータ
    GridPeerIterator3d pit;
    while (pit.next()) {
        // その方位の周辺セルを定義したレンジでイテレータを初期化する
        GridIterator3d cit(surroundingRangeFor(pit));
        // その方位の周辺セルに属するセルに関してループ
        while (cit.next()) {
            Cell *cell = cellFor(cit);
            // cellが保持している全particleを空きリストに移す
            cell->moveAllParticlesTo(&freeParticleList_);
        }
    }
}

void MdProcData::exportExitingMoleculeFullData() {
    //Logger::out << "MdProcData::exportExitingMoleculeFullData" << std::endl;
    // 26方位の配列座標[0,0,0]..[2,2,2]を発生するイテレータ
    GridPeerIterator3d peerIt;
    while (peerIt.next()) {
        //Logger::out << "Checking direction " << peerIt << std::endl;
        // この方位の peer buffer を取得
        MdCommPeerBuffer *peer = commData_->bufferFor(peerIt);
        assert(peer->send_count_per_cell_.empty());
        assert(peer->send_molecule_full_.empty());
        // その方位の周辺セルに関してループ
        GridIterator3d cellIt(surroundingRangeFor(peerIt));
        while (cellIt.next()) {
            Cell *cell = cellFor(cellIt);
            //Logger::out << "Checking cell " << cellIt << " box : " << cell->cellBox() << std::endl;
            // cellの保持分子をfull data送信バッファに登録
            peer->addMoleculeFullDataFrom(cell);
        }
    }
    //Logger::out << "MdProcData::exportExitingMoleculeFullData end" << std::endl;
}


void MdProcData::importEnteringMoleculeFullData() {
    // 26方位の配列座標[0,0,0]..[2,2,2]を発生するイテレータ
    GridPeerIterator3d peerIt;
    while (peerIt.next()) {
        // この方位の peer buffer を取得
        MdCommPeerBuffer *peer = commData_->bufferFor(peerIt);
        int count_index = 0;
        int data_index = 0;
        // この方位の表面セルに関してループ
        GridIterator3d cellIt(surfaceRangeFor(peerIt));
        while (cellIt.next()) {
            // 表面セルを一つ取得
            Cell *cell = cellFor(cellIt);
            // このセルに格納すべき受信粒子の個数を取得
            int count_for_cell = peer->recv_count_per_cell_[count_index];
            ++count_index;
            // 粒子の個数のループの終端を算出しておく
            int data_index_end = data_index + count_for_cell;
            for (; data_index < data_index_end; ++data_index) {
                // 受信した粒子データから一つ取得
                CommMoleculeFullData *full = &(peer->recv_molecule_full_[data_index]);
                // particle のメモリを割り当て、そこに情報を転記
                Particle *p = allocateParticle();
                p->kind_ = full->kind_;
                p->serial_ = full->serial_;
                p->pos_.set(full->rx_, full->ry_, full->rz_);
                p->vel_dt_.set(full->vdtx_, full->vdty_, full->vdtz_);
                // cellにparticleを追加する
                cell->addParticle(p);
            }
        }
        peer->recv_molecule_full_.clear();
        peer->recv_count_per_cell_.clear();
    }
}

void MdProcData::calcForce() {
    // 全ローカルセルについてループ
    //Logger::out << " calc Force:start" << std::endl;
    GridIterator3d cellIt(localCellsRange_);
    while (cellIt.next()) {
        // 力計算では、各粒子に働く力の変数に、次々に加えていくので、最初に0にする。
        cellFor(cellIt)->clearForces();
    }
    cellIt.reset();
    while (cellIt.next()) {
        Cell *cell = cellFor(cellIt);
        // cellの中の粒子同士の分子間力を計算する
        cell->calcForceWithinSelf();
        // HINT : there are more to calculate.
        GridDirIterator3d ofs; //offset
        while (ofs.next()) {
            GridIndex3d otherIdx = cellIt + ofs;
            Cell *otherCell = cellFor(otherIdx);
            if (isLocalCell(otherIdx)) {
                if (ofs.lessThan(0, 0, 0)) {
                    cell->calcForceWithLocalCell(otherCell);
                }
            } else {
                cell->calcForceWithSurroundingCell(otherCell);
            }
        }
    }
}


void MdProcData::calcForceAndUp() {
    // 全ローカルセルについてループ
    GridIterator3d cellIt(localCellsRange_);
    while (cellIt.next()) {
        // 力計算では、各粒子に働く力の変数に、次々に加えていくので、最初に0にする。
        cellFor(cellIt)->clearForces();
        cellFor(cellIt)->clearUp();
    }
    cellIt.reset();

    while (cellIt.next()) {
        Cell *cell = cellFor(cellIt);
        // cellの中の粒子同士の分子間力を計算する
        cell->calcForceWithinSelfAndUp();

        GridDirIterator3d ofs; //offset
        while (ofs.next()) {
            GridIndex3d otherIdx = cellIt + ofs;
            Cell *otherCell = cellFor(otherIdx);
            if (isLocalCell(otherIdx)) {
                if (ofs.lessThan(0, 0, 0)) {
                    cell->calcForceWithLocalCellAndUp(otherCell);
                }
            } else {
                cell->calcForceWithSurroundingCellAndUp(otherCell);
            }
        }
    }
}


void MdProcData::updatePosition() {
    //Logger::out << "updatePosition" << std::endl;
    // 全ローカルセルについてループ
    GridIterator3d cellIt(localCellsRange_);
    // 位置を更新する。その結果セルの範囲から逸脱する分子も生じる。
    while (cellIt.next()) {
        Cell *cell = cellFor(cellIt);
        cell->updatePosition();
    }
    // 同じ範囲に対してループ
    cellIt.reset();
    // セルから逸脱しているものを適切な隣接セルに移動させる
    while (cellIt.next()) {
        Cell *cell = cellFor(cellIt);
        cell->migrateToNeighbor();
    }
    //Logger::out << "updatePosition:end" << std::endl;
}


//added
void MdProcData::updateVelocityHalf() {
    //Logger::out << "updateVelocityHalf" << std::endl;
    // 全ローカルセルについてループ
    GridIterator3d cellIt(localCellsRange_);
    // 位置を更新する。その結果セルの範囲から逸脱する分子も生じる。
    while (cellIt.next()) {
        Cell *cell = cellFor(cellIt);
        cell->updateVelocityHalf();
    }
    //Logger::out << "updateVelocityHalf:end" << std::endl;
}

void MdProcData::updateVelocityHalfAndCalcUk() {
    //Logger::out << "updateVelocityHalf" << std::endl;
    // 全ローカルセルについてループ
    GridIterator3d cellIt(localCellsRange_);
    // 位置を更新する。その結果セルの範囲から逸脱する分子も生じる。
    while (cellIt.next()) {
        Cell *cell = cellFor(cellIt);
        cell->updateVelocityHalfAndCalcUk();
    }
    //Logger::out << "updateVelocityHalf:end" << std::endl;
}


void MdProcData::exportTrajectoryData() {
    //Logger::out << "exportTrajectoryData" << std::endl;
    // 全ローカルセルについてループ
    GridIterator3d cellIt(localCellsRange_);
    while (cellIt.next()) {
        Cell *cell = cellFor(cellIt);
        // cellが保持している全分子をトラジェクトリーの送信バッファに登録する
        commData_->addTrajectoryDataFrom(cell);
    }
    //Logger::out << "exportTrajectoryData:end" << std::endl;
}

void MdProcData::writeTrajectoryData() {
    assert(caseData_->isRootRank());
    // 受信したトラジェクトリーデータをファイルに出力する
    // commData_->orderRecvTrajectoryToAllTrajectory();
    commData_->writeTrajectory();
}

void MdProcData::exportEnergyData() {
    GridIterator3d cellIt(localCellsRange_);
    commData_->send_uk_ = 0;
    commData_->send_up_ = 0;
    while (cellIt.next()) {
        Cell *cell = cellFor(cellIt);
        commData_->send_uk_ += cell->get_uk();
        commData_->send_up_ += cell->get_up();
    }
    //Logger::out << "MdProcData::exportEnergyData: " << commData_->send_uk_ << "," << commData_->send_up_ << std::endl;
}

void MdProcData::writeEnergyData() {
    assert(caseData_->isRootRank());
    // 受信したエネルギーデータをファイルに出力する
    commData_->writeTotalEnergy();
}

void MdProcData::exportSurfacingMoleculePosData() {
    //Logger::out << "MdProcData::exportSurfacingMoleculePosData" << std::endl;
    // 26方位の配列座標[0,0,0]..[2,2,2]を発生するイテレータ
    GridPeerIterator3d peerIt;
    while (peerIt.next()) {
        //Logger::out << "Checking direction " << peerIt << std::endl;
        // この方位の peer buffer を取得
        MdCommPeerBuffer *peer = commData_->bufferFor(peerIt);
        assert(peer->send_count_per_cell_.empty());
        assert(peer->send_molecule_pos_.empty());
        // その方位の周辺セルに関してループ
        GridIterator3d cellIt(surfaceRangeFor(peerIt));
        while (cellIt.next()) {
            Cell *cell = cellFor(cellIt);
            //Logger::out << "Checking cell " << cellIt << " box : " << cell->cellBox() << std::endl;
            // cellの保持分子をpos data送信バッファに登録
            peer->addMoleculePosDataFrom(cell);
        }
    }
    //Logger::out << "MdProcData::exportSurfacingMoleculePosData end" << std::endl;
}


void MdProcData::importSurroundingMoleculePosData() {
    //Logger::out << "MdProcData::importSurroundingMoleculePosData" << std::endl;
    // 26方位の配列座標[0,0,0]..[2,2,2]を発生するイテレータ
    GridPeerIterator3d peerIt;
    while (peerIt.next()) {
        // この方位の peer buffer を取得
        MdCommPeerBuffer *peer = commData_->bufferFor(peerIt);
        int count_index = 0;
        int data_index = 0;
        // この方位の表面セルに関してループ
        GridIterator3d cellIt(surroundingRangeFor(peerIt));
        while (cellIt.next()) {
            // 表面セルを一つ取得
            Cell *cell = cellFor(cellIt);
            // このセルに格納すべき受信粒子の個数を取得
            int count_for_cell = peer->recv_count_per_cell_[count_index];
            ++count_index;
            // 粒子の個数のループの終端を算出しておく
            int data_index_end = data_index + count_for_cell;
            for (; data_index < data_index_end; ++data_index) {
                // 受信した粒子データから一つ取得
                CommMoleculePosData *pos = &(peer->recv_molecule_pos_[data_index]);
                // particle のメモリを割り当て、そこに情報を転記
                Particle *p = allocateParticle();
                p->kind_ = pos->kind_;
                p->pos_.set(pos->rx_, pos->ry_, pos->rz_);
                // cellにparticleを追加する
                cell->addParticle(p);
            }
        }
        peer->recv_molecule_pos_.clear();
        peer->recv_count_per_cell_.clear();
    }
    //Logger::out << "MdProcData::importSurroundingMoleculePosData end" << std::endl;
}
