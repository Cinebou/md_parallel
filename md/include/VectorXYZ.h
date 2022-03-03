/*
 * VectorXYZ.h
 *
 *      Author: Hideo Takahashi
 */

#ifndef _VECTORXYZ_H
#define _VECTORXYZ_H

#include <cmath>
#include <iostream>

/*
 * 3次元の実数ベクトルクラス
 *
 * MDで使う必要となる範囲で演算子や計算メソッドを設けている。
 *
 * やろうと思えば内積や外積なども定義できるが、MDでは使う場面がない。
 * また、ノルムも使いそうだが、LJポテンシャルでは、実は使う必要がないので設けていない。
 */
class VectorXYZ {
public:
    double x_;
    double y_;
    double z_;

    VectorXYZ() {  }

    VectorXYZ(double r) {
        x_ = y_ = z_ = r;
    }

    VectorXYZ(double x, double y, double z) {
        this->x_ = x;
        this->y_ = y;
        this->z_ = z;
    }

    VectorXYZ(VectorXYZ const &v) {
        x_ = v.x_;
        y_ = v.y_;
        z_ = v.z_;
    }

    void set(double x, double y, double z) {
        x_ = x;
        y_ = y;
        z_ = z;
    }

    void set(VectorXYZ const &other) {
        x_ = other.x_;
        y_ = other.y_;
        z_ = other.z_;
    }

    void clear() {
        x_ = y_ = z_ = 0;
    }

    /*
     * Assignment operators
     */
    void operator=(VectorXYZ const &v) {
        x_ = v.x_;
        y_ = v.y_;
        z_ = v.z_;
    }

    void operator+=(VectorXYZ const &v) {
        x_ += v.x_;
        y_ += v.y_;
        z_ += v.z_;
    }

    /*
     * binary operators
     */
    VectorXYZ operator-(VectorXYZ const &o) const {
        return VectorXYZ(x_-o.x_, y_-o.y_, z_-o.z_);
    }

    VectorXYZ operator+(VectorXYZ const &o) const {
        return VectorXYZ(x_+o.x_, y_+o.y_, z_+o.z_);
    }

    /*
     * スカラー倍。
     * この演算子は、 VectorXYZ * double の記述順に対応する。
     * もしも double * VectorXYZ の順序も使いたければ、
     * class VectorXYZの外で、VectorXYZ operator(double r, const VectorXYZ &vect); という
     * 関数（メソッドではない）を設ける。
     */
    VectorXYZ operator*(double r) const {
        return VectorXYZ(r*x_, r*y_, r*z_);
    }

    /*
     * 絶対値の二乗
     */
    double square() const {
        return x_*x_ + y_*y_ + z_*z_;
    }

};

/*
 * デバッグや表示用に、ostreamにVectorを出力する。
 */
static inline std::ostream& operator<<(std::ostream &os, VectorXYZ const &v) {
    os << "(" << v.x_ << "," << v.y_ << "," << v.z_ << ")";
    return os;
}

#endif /* VECTORXYZ_H_ */
