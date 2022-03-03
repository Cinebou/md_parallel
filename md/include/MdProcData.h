/*
 * ProcData.h
 *
 *      Author: Hideo Takahashi
 */

#ifndef _MDPROCDATA_H
#define _MDPROCDATA_H

#include <CaseData.h>
#include <MdCommData.h>
#include <Cell.h>
#include <GridIterator3d.h>
#include <vector>

/*
 * per process data クラス
 * 1プロセスがシミュレーションの範囲のデータを保持するクラス
 */
class MdProcData {

    /*
     * 計算条件
     */
    CaseData *caseData_;
    /*
     * 通信バッファ
     */
    MdCommData *commData_;
    /*
     * 未使用のParticle構造体のリスト
     */
    ParticleList freeParticleList_;
    /*
     * ローカルセルの一次元配列
     * 周辺セルも含む全てのローカルセル
     */
    Cell *cells_;
    /*
     * 周辺セルも含んだ全ローカルセルを覆うレンジ
     */
    GridRange3d allCellsRange_;
    /*
     * ローカルセルの部分だけを覆うレンジ
     */
    GridRange3d localCellsRange_;
    /*
     * 方位別の表面セルを覆うレンジ
     */
    GridRange3d surfaceRanges_[3][3][3];
    /*
     * 方位別の周辺セルを覆うレンジ
     */
    GridRange3d surroundingRanges_[3][3][3];
    /*
     * 周辺セルの分を含んだセルの個数
     */
    int acx_, acy_, acz_;

    /*
     * シミュレーション対象の分子の総数。
     * 全プロセスに分配される分子の総数。
     */
    int total_molecule_count_;

public:

    MdProcData();
    ~MdProcData();

    /*
     * 初期化
     */
    void init(CaseData *caseData_, MdCommData *commData_);

    /*
     * レンジオブジェクトを初期化する。init()から呼ばれる。
     */
    void initRanges();

    /*
     * セルオブジェクトの領域を確保する。init()から呼ばれる。
     */
    void allocateCells();

    /*
     * 各セルオブジェクトを初期化する。init()から呼ばれる。
     */
    void initCells();

    /*
     * 初期状態ファイルを読み込む
     */
    void readInitialStateFile();

    /*
     * Particle構造体のメモリを割り当てる。
     * 未使用のParticleのストック（在庫）がfreeParticleList_あればそれを優先的に再利用する。
     * freeParticleList_が空の場合には new でメモリを新規に割り当てる。
     */
    Particle *allocateParticle();

    /*
     * 引数のpが保有している全てのParticleをpから取り去ってfreeParticleList_につなげかえる。
     * pは空のリストとなり、pが保持していたParticleは再利用の対象になる。
     */
    void stockAllParticlesInList(ParticleList *pList);

    /*
     * 分子の座標posから、その分子が所属すべきセルの座標を算出する。
     */
    void setCellIndexForPos(GridIndex3d *cellIdx, const VectorXYZ &pos) const;

    /*
     * 引数で座標が指定されたセルを取得する
     */
    Cell *cellFor(const GridIndex3d &cellIdx) {
        assert(cellIdx.ix_ >= 0 && cellIdx.ix_ < acx_);
        assert(cellIdx.iy_ >= 0 && cellIdx.iy_ < acy_);
        assert(cellIdx.iz_ >= 0 && cellIdx.iz_ < acz_);
        // acy_*acz_の値は常に同一なので、その積を変数に保存しておけば、多少高速化できる。
        int index = cellIdx.ix_*acy_*acz_ + cellIdx.iy_*acz_ + cellIdx.iz_;
        return &cells_[index];
    }

    /*
     * 引数の座標がローカルセルに該当するか（周辺セルではないことを）判定する
     */
    bool isLocalCell(const GridIndex3d &i) const {
        return (i.ix_ > 0) && (i.ix_ <= caseData_->ncx_) &&
                (i.iy_ > 0) && (i.iy_ <= caseData_->ncy_) &&
                (i.iz_ > 0) && (i.iz_ <= caseData_->ncz_);
    }

    /*
     * 引数で示された方位の表面セルの集合を表したレンジを取得する
     * 戻り値にconstがあるので、この戻り値を変更することは許さない。
     * 引数にconstがあるので、引数のrangeをこのメソッドが変更しないことを約束する。
     * 引数のかっこの後にconstがあるので、このメソッドが、thisを変更しないこと
     * （メンバ変数を改変しないこと）を約束する。
     */
    const GridRange3d &surfaceRangeFor(const GridIndex3d &rangeIdx) const {
        return surfaceRanges_[rangeIdx.ix_][rangeIdx.iy_][rangeIdx.iz_];
    }

    /*
     * 引数で示された方位の周辺セルの集合を表したレンジを取得する
     */
    const GridRange3d &surroundingRangeFor(const GridIndex3d &rangeIdx) const {
        return surroundingRanges_[rangeIdx.ix_][rangeIdx.iy_][rangeIdx.iz_];
    }

    /*
     * 初期状態ファイルに記載されていた分子の総数を返す。
     */
    int getMoleculeCount() const {
        return total_molecule_count_;
    }

    /*
     * 本プロセスの担当領域から転出した分子のデータを、周辺セルから送信バッファに転記する
     */
    void exportExitingMoleculeFullData();

    /*
     * 本プロセスの担当領域から転出した分子のデータを、表面セルから送信バッファに転記する
     */
    void exportSurfacingMoleculePosData();

    /*
     * 本プロセスの担当領域から転出した分子のデータを、受信バッファから表面セルに転記する
     */
    void importSurroundingMoleculePosData();

    /*
     * 本プロセスの担当領域に転入した分子のデータを、受信バッファから、該当する表面セルに転記する
     */
    void importEnteringMoleculeFullData();

    /*
     * トラジェクトリー用のデータを送信バッファに転記する
     */
    void exportTrajectoryData();

    /*
     * 全周辺セルのParticleListの全Particleを、フリーリストにつなぎ替えることに
     * よって、全周辺セルを空にする。
     */
    void clearSurroundingCells();

    /*
     * 分子間力の計算をする
     */
    void calcForce();

    /*
     * 結果出力回で分子間力の計算とエネルギー計算をする
     */
    void calcForceAndUp();

    /*
     * 位置の更新計算をする
     */
    void updatePosition();

    /*
     * 速度の更新計算をする
     */
    void updateVelocityHalf();

    /*
     * 結果出力回で速度の更新計算とエネルギー計算をする
     */
    void updateVelocityHalfAndCalcUk();

    /*
     * rank=0において、受信した全粒子の位置・速度のデータをトラジェクトリーファイルに出力する。
     */
    void writeTrajectoryData();

    void exportEnergyData();

    void writeEnergyData();
};

#endif /* MDPROCDATA_H_ */
