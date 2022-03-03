/*
 * GridIterator3d.h
 *
 *      Author: Hideo Takahashi
 */

#ifndef GRIDITERATOR3D_H_
#define GRIDITERATOR3D_H_

#include <iostream>

/*
 * 三次元配列の中の位置（整数の３次元座標）を保持するクラス
 */
class GridIndex3d {
public:

    int ix_, iy_, iz_;

    // デフォルトコンストラクタ
    GridIndex3d() { }

    // コピーコンストラクタ
    GridIndex3d(const GridIndex3d &o) {
        ix_ = o.ix_;
        iy_ = o.iy_;
        iz_ = o.iz_;
    }

    // 個々の変数の初期値を指定するコンストラクタ
    GridIndex3d(int x, int y, int z) {
        ix_ = x;
        iy_ = y;
        iz_ = z;
    }

    // 一致比較メソッド
    bool equals(int ix, int iy, int iz) const {
        return (ix_ == ix) && (iy_ == iy) && (iz_ == iz);
    }

    // 一致比較演算子
    bool operator ==(const GridIndex3d &o) const {
        return (ix_ == o.ix_) && (iy_ == o.iy_) && (iz_ == o.iz_);
    }

    // 大小比較メソッド
    // x,y,z を辞書に登場する単語のようにみなして大小比較する
    // xに違いがあればy,zは不問、
    // xが等しく、yに違いがあればzは不問
    // x,yが等しければzの違いを吟味する。
    bool lessThan(int ix, int iy, int iz) const {
        return ((ix_ < ix) ||
                ((ix_ == ix) && ((iy_ < iy) ||
                        ((iy_ == iy) && (iz_ < iz)))));
    }

    // 加算演算子
    GridIndex3d operator+(const GridIndex3d &o) const {
        return GridIndex3d(ix_ + o.ix_, iy_ + o.iy_, iz_ + o.iz_);
    }


    // 減算演算子
    GridIndex3d operator-(const GridIndex3d &o) const {
        return GridIndex3d(ix_ - o.ix_, iy_ - o.iy_, iz_ - o.iz_);
    }

};

// ストリームへの出力演算子
inline static std::ostream &operator<<(std::ostream &os, const GridIndex3d &ind) {
    os << "[" << ind.ix_ << "," << ind.iy_ << "," << ind.iz_ << "]";
    return os;
}

// 三次元座標の、直方体状の範囲を保持するクラス
class GridRange3d {
public:
    int xmin_, ymin_, zmin_; // 下限値。境界含む
    int xmax_, ymax_, zmax_; // 上限値。境界含む

    GridRange3d() { }

    GridRange3d(int xmin, int ymin, int zmin, int xmax, int ymax, int zmax) {
        xmin_ = xmin;
        ymin_ = ymin;
        zmin_ = zmin;

        xmax_ = xmax;
        ymax_ = ymax;
        zmax_ = zmax;
    }

    void setRange(int xmin, int ymin, int zmin, int xmax, int ymax, int zmax) {
        xmin_ = xmin;
        ymin_ = ymin;
        zmin_ = zmin;

        xmax_ = xmax;
        ymax_ = ymax;
        zmax_ = zmax;
    }

    // 下限から上限までの間に収まっていない可能性のあるvに対して、
    // 範囲を逸脱していたら範囲を周期的に扱い、逸脱したのと反対側から
    // 範囲に入ってきた値に変換する。
    // 喩えて言えば、θ<0 の場合には2π-θを返し、θ>2πの場合にはθ-2πを返すようなもの。
    static int wrap(int v, int min, int max) {
        if (v < min) {
            return v + (max-min+1);
        } else if (v > max) {
            return v - (max-min+1);
        } else {
            return v;
        }
    }

    // 与えられた三次元座標値の各成分に対して、範囲を逸脱していたら
    // 反対側の境界の内側の座標となるように補正する。
    GridIndex3d wrapIndex(const GridIndex3d &ind) const {
        return GridIndex3d(wrap(ind.ix_, xmin_, xmax_),
                wrap(ind.iy_, ymin_, ymax_),
                wrap(ind.iz_, zmin_, zmax_));
    }

    // 範囲に属する整数座標の個数を返す
    size_t size() const {
        return (xmax_ - xmin_ + 1) * (ymax_ - ymin_ + 1) * (zmax_ - zmin_ + 1);
    }

};

/*
 * GridRange3dが保持している座標の範囲を走査するためのイテレータクラス
 * 座標の範囲を直接指定して用いることもできる。
 */
class GridIterator3d : public GridIndex3d {
public:

    int xmin_, ymin_, zmin_;
    int xmax_, ymax_, zmax_;

    GridIterator3d() { }

    /*
     * レンジから初期化するためのコンストラクタ
     */
    GridIterator3d(const GridRange3d &r) : GridIndex3d(r.xmin_, r.ymin_, r.zmin_ - 1){
        xmin_ = r.xmin_;
        ymin_ = r.ymin_;
        zmin_ = r.zmin_;

        xmax_ = r.xmax_;
        ymax_ = r.ymax_;
        zmax_ = r.zmax_;
    }

    /*
     * コピーコンストラクタ
     */
    GridIterator3d(const GridIterator3d &o) : GridIndex3d(o) {
        xmin_ = o.xmin_;
        ymin_ = o.ymin_;
        zmin_ = o.zmin_;

        xmax_ = o.xmax_;
        ymax_ = o.ymax_;
        zmax_ = o.zmax_;
    }

    /*
     * 座標のレンジと初期座標を直接していするコンストラクタ
     */
    GridIterator3d(int xmin, int ymin, int zmin,
            int xmax, int ymax, int zmax,
            int ix, int iy, int iz) : GridIndex3d(ix,iy,iz) {

        xmin_ = xmin;
        ymin_ = ymin;
        zmin_ = zmin;

        xmax_ = xmax;
        ymax_ = ymax;
        zmax_ = zmax;
    }

    /*
     * 座標のレンジのみを指定するコンストラクタ
     */
    GridIterator3d(int xmin, int ymin, int zmin,
            int xmax, int ymax, int zmax) : GridIndex3d(xmin, ymin, zmin - 1) {

        xmin_ = xmin;
        ymin_ = ymin;
        zmin_ = zmin;

        xmax_ = xmax;
        ymax_ = ymax;
        zmax_ = zmax;

    }

    /*
     * レンジオブジェクトを用いて状態を再設定する
     */
    void set(GridRange3d &r) {
        xmin_ = r.xmin_;
        ymin_ = r.ymin_;
        zmin_ = r.zmin_;

        xmax_ = r.xmax_;
        ymax_ = r.ymax_;
        zmax_ = r.zmax_;

        ix_ = r.xmin_;
        iy_ = r.ymin_;
        iz_ = r.zmin_ - 1;
    }

    /*
     * 同じレンジに対して再び走査するために座標を初期化する
     */
    void reset() {
        ix_ = xmin_;
        iy_ = ymin_;
        iz_ = zmin_ - 1;
    }

    /*
     * 走査順にしたがって次の座標に着目点を進める。
     * 進めた先の座標が走査範囲内にある場合には true、走査範囲を越えた場合には false を返す。
     */
    bool next() {
        if (++iz_ <= zmax_) {
            return true;
        }
        iz_ = zmin_;
        if (++iy_ <= ymax_) {
            return true;
        }
        iy_ = ymin_;
        if (++ix_ <= xmax_) {
            return true;
        }
        return false;
    }

};

/*
 * 隣接セルの範囲を走査するためのイテレータ
 * [0,0,0] -> [0,0,1] -> ... -> [2,2,1] -> [2,2,2] のように進む。
 * [1,1,1]はスキップする。
 */
class GridPeerIterator3d : public GridIndex3d {

public:

    GridPeerIterator3d() : GridIndex3d(0,0,-1) { }

    void reset() {
        ix_ = 0;
        iy_ = 0;
        iz_ = -1;
    }

    bool next() {
        ++iz_;
        if (iz_ == 1 && iy_ == 1 && ix_ == 1) {
            // [1,1,1] の場合はさらに[1,1,2]まで進める
            ++iz_;
            return true;
        } else if (iz_ <= 2) {
            return true;
        }
        iz_ = 0;
        if (++iy_ <= 2) {
            return true;
        }
        iy_ = 0;
        if (++ix_ <= 2) {
            return true;
        }
        return false;
    }
};

/*
 * 周囲の26方位のベクトルを発生させるイテレータ
 * [-1,-1,-1] -> [-1,-1,0] -> ... -> [1,1,0] -> [1,1,1] のように進む
 * [0,0,0]はスキップする
 */
class GridDirIterator3d : public GridIndex3d {

public:

    GridDirIterator3d() : GridIndex3d(-1,-1,-2) { }

    void reset() {
        ix_ = -1;
        iy_ = -1;
        iz_ = -2;
    }

    bool next() {
        ++iz_;
        if (iz_ == 0 && iy_ == 0 && ix_ == 0) {
            ++iz_;
            return true;
        } else if (iz_ <= 1) {
            return true;
        }
        iz_ = -1;
        if (++iy_ <= 1) {
            return true;
        }
        iy_ = -1;
        if (++ix_ <= 1) {
            return true;
        }
        return false;
    }
};

/*
GridIndex3d GridIndex3d::operator+(const GridPeerIterator3d &o) const {
    return GridIndex3d(ix_ + o.ix_, iy_ + o.iy_, iz_ + o.iz_);
}
*/


#endif /* GRIDITERATOR3D_H_ */
