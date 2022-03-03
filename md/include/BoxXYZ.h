/*
 * BoxXYZ.h
 *
 *      Author: Hideo Takahashi
 */

#ifndef BOXXYZ_H_
#define BOXXYZ_H_

#include <GridIterator3d.h>
#include <VectorXYZ.h>
#include <cassert>
#include <iostream>

/*
 * 座標系に沿った（回転されていない）直方体の位置を保持するクラス。
 * 点が直方体に入っているか否かの判定を主な機能として提供する。
 */
class BoxXYZ {
public:
    // x,y,zの値が最小の頂点と、最大の頂点のそれぞれの座標
    VectorXYZ p1_, p2_;
    // 対角線のベクトル
    VectorXYZ d_;

    BoxXYZ() {
        // leave uninitialized.
    }

    // コピーコンストラクタ
    BoxXYZ(const BoxXYZ &o) : p1_(o.p1_), p2_(o.p2_), d_(o.d_) {
    }

    // 頂点の座標を二組渡すコンストラクタ
    BoxXYZ(double x1, double y1, double z1, double x2, double y2, double z2) {
        set(x1,y1,z1,x2,y2,z2);
    }

    // 頂点の座標を二組設定するメソッド
    void set(double x1, double y1, double z1, double x2, double y2, double z2) {
        assert(x1 <= x2);
        assert(y1 <= y2);
        assert(z1 <= z2);

        p1_.set(x1, y1, z1);
        p2_.set(x2, y2, z2);
        d_ = p2_ - p1_;
    }

    // 代入演算子
    BoxXYZ &operator=(const BoxXYZ &other) {

        p1_.set(other.p1_);
        p2_.set(other.p2_);
        d_.set(other.d_);
        return *this;
    }

    // p1を起点とした、posの相対座標を返す
    VectorXYZ offset(VectorXYZ const &pos) const {
        return pos - p1_;
    }

    // 個々の座標用のアクセッサ(accessor)メソッド
    double x1() const {
        return p1_.x_;
    }
    double y1() const {
        return p1_.y_;
    }
    double z1() const {
        return p1_.z_;
    }

    double x2() const {
        return p2_.x_;
    }
    double y2() const {
        return p2_.y_;
    }
    double z2() const {
        return p2_.z_;
    }

    double dx() const {
        return d_.x_;
    }
    double dy() const {
        return d_.y_;
    }
    double dz() const {
        return d_.z_;
    }

    // 点が直方体に含まれるかいなかを判定する
    bool contains(const VectorXYZ &v) const {
        return ((p1_.x_ <= v.x_) && (v.x_ < p2_.x_) &&
                (p1_.y_ <= v.y_) && (v.y_ < p2_.y_) &&
                (p1_.z_ <= v.z_) && (v.z_ < p2_.z_));
    }

    // getRelativeIndexForの内部で使う判定関数。
    // privateメソッドにしてもよいが、そうするとテストがしにくくなるのでpublicのままにしておく。
    // 座標値、下限値、上限値を受け取り、以下を返す。戻り値は配列の添字に適するように0,1,2とする。
    // 0 : 下限未満
    // 1 : 下限以上、上限未満
    // 2 : 上限以上
    static int relativeIndex(double x, double xl, double xh) {
        return (x < xl) ? 0 : ((x >= xh) ? 2 : 1);
    }

    // 与えられた点が、直方体とどのような位置関係にあるのかを、整数のベクトルで返す
    // たとえば、直方体の中に入って入れば (1,1,1), x座標だけが直方体の範囲を超過していれば (2,1,1)
    // などのような値を返す。
    GridIndex3d getRelativeIndexFor(const VectorXYZ &pos) const {
        int ix, iy, iz;
        ix = relativeIndex(pos.x_, p1_.x_, p2_.x_);
        iy = relativeIndex(pos.y_, p1_.y_, p2_.y_);
        iz = relativeIndex(pos.z_, p1_.z_, p2_.z_);
        return GridIndex3d(ix, iy, iz);
    }

};

static inline std::ostream &operator<<(std::ostream &os, const BoxXYZ &box) {
    os << box.p1_ << "-" << box.p2_;
    return os;
}

#endif /* BOXs_H_ */
