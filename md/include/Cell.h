/*
 * Cell.h
 *
 *      Author: Hideo Takahashi
 */

#ifndef _CELL_H
#define _CELL_H

#include <Particle.h>
#include <BoxXYZ.h>
#include <Logger.h>
#include <LJParams.h>

/*
 * ローカルセルクラス
 */
class Cell {
    /*
     * C++文法により、このセクションは private
     */

    friend class TestCell; // Cellのテストドライバクラスから、本クラスのprivateメンバにアクセスできるように宣言しておく

    // このセルの占有する直方体
    BoxXYZ cellBox_;

    // このセルに属している粒子のリスト
    ParticleList list_;

    // 隣接セルのオブジェクトへのポインタ。[1][1][1] は自身に相当し、未使用。
    Cell *neighborCells_[3][3][3];

    // セル全体としてのUp,Ukの値。エネルギーを計算する回次でのみ、使用する。
    // 単位系は原子レベルのスケールに沿ったものとし、外部に出力する場面で巨視的なスケールに直すものとする
    double up_, uk_; // [u*Angstrom*fs^-2]

public:

    // コンストラクタ
    // ここには何も書かないが、メンバ変数のコンストラクタが自動的に実行される
    Cell() {}

    ~Cell() {}

    // 隣接セルへのポインタを設定する。初期化処理用。
    void setNeighborCell(int ocx, int ocy, int ocz, Cell *cell) {
        assert(ocx >= 0 && ocx <= 2);
        assert(ocy >= 0 && ocy <= 2);
        assert(ocz >= 0 && ocz <= 2);
        assert(cell != NULL);
        neighborCells_[ocx][ocy][ocz] = cell;
    }

    // セルの座標範囲を指定する。初期化処理用。
    void setBox(const BoxXYZ &box) {
        cellBox_ = box;
    }

    // セルの座標範囲を取得する。
    const BoxXYZ &cellBox() const {
        return cellBox_;
    }

    // 本セルからの位置関係がindで表される隣接セルへのポインタを取得する
    Cell *neighborCellFor(const GridIndex3d &ind) {
        assert(ind.ix_ >= 0 && ind.ix_ <= 2);
        assert(ind.iy_ >= 0 && ind.iy_ <= 2);
        assert(ind.iz_ >= 0 && ind.iz_ <= 2);
        return neighborCells_[ind.ix_][ind.iy_][ind.iz_];
    }

    // セルに粒子を追加する。粒子オブジェクトのメモリ割り当て責任はcallerにあり、
    // ここで渡された粒子のメモリの所有権は当セルが譲り受ける。
    void addParticle(Particle *p) {
        // 例年、多くのグループが、次のassertで落ちるバグを作っている。
        // もしassertでひっかかるようならば、以下のLoggerの行のコメントを外して
        // 追加しようとしているpの座標をログに記録すると、原因究明の助けになる。

        //Logger::out << "Cell" << cellBox() << ".addParticle" << p->pos_ << std::endl;

        assert(cellBox_.contains(p->pos_)); // セルの範囲外の座標の粒子が渡されていないか確認する。
        list_.add(p);
    }

    // 当セルの粒子数をゼロにする。セルが周辺セルである場合に使うメソッド。
    // セルが保持している全粒子のリストをそっくりそのまま、渡された粒子リストにつなげ替える。
    void moveAllParticlesTo(ParticleList *freeParticleList) {
        list_.moveAllTo(freeParticleList);
    }

    // 当セルが保持する粒子リストの先頭の粒子オブジェクトを返す。
    // セルに属する粒子に関してループ処理をするには、このメソッドを使う。
    Particle *getParticleListHead() {
        return list_.head();
    }

    // セルが空かどうか判定する
    bool empty() const {
        return list_.isEmpty();
    }

    // 粒子に働く力の計算値を全てゼロにする
    void clearForces();
    void clearUp();

    // セルに属する粒子同士の間に働く力を計算する
    void calcForceWithinSelf();

    void calcForceWithinSelfAndUp();

    //ローカルセル内の隣接セルとの力計算
    void calcForceWithLocalCell(Cell *cell);

    void calcForceWithLocalCellAndUp(Cell *cell);

    //周辺セルとの間の（プロセスをまたぐ）力計算
    void calcForceWithSurroundingCell(Cell *cell);

    void calcForceWithSurroundingCellAndUp(Cell *cell);

    // 粒子の位置を更新する
    void updatePosition();

    //粒子の速度を半分更新する
    void updateVelocityHalf();

    void updateVelocityHalfAndCalcUk();

    //ポテンシャルの計算をする
    VectorXYZ calcLJforce(VectorXYZ const *dist, double r_2, LJScaledMoleculePairParam const *pair);

    // 位置を更新した結果、セルの範囲を逸脱してしまった粒子を隣接セルに移動させる。
    // シミュレーションの1ステップでそれ以上遠くのセルまで粒子が移動した場合はエラーとして
    // 扱う（assertで判定しているのでデバッグ版でのみチェックが働く）
    void migrateToNeighbor();

    double get_uk() {
        return uk_;
    }

    double get_up() {
        return up_;
    }
};


#endif /* CELL_H_ */
