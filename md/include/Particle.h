#ifndef _PARTICLE_H
#define _PARTICLE_H

#include <VectorXYZ.h>
#include <cstdlib>
#include <cassert>

/*
 * 分子（粒子）を一つ保持するクラス。
 * 双方向リストの要素。
 */
class Particle {
public:

    /*
     * 双方向リストの前後の要素を指すポインタ
     * リストの端点では、NULLにする。
     */
    Particle *next_;
    Particle *prev_;

    /*
     * 粒子の種類（粒子種別番号。src-nompi/LJParams.cpp参照）
     */
    int kind_;

    /*
     * 粒子の通し番号
     */
    int serial_;

    /*
     * 位置
     */
    VectorXYZ pos_; // [Angstrom]
    /*
     * 速度×Δt
     */
    VectorXYZ vel_dt_; // vel * dt [Angstrom]

    /*
     * 加速度×Δt^2/2
     */
    VectorXYZ a_dt2_half_; // acc * dt^2 * 0.5 [Angstrom]

};

/*
 * 粒子の双方向リスト
 */
class ParticleList {

    friend class TestParticleList;

    /*
     * リストの先頭と末尾要素。
     * リストが空の場合は両方NULL。
     * リストが単一要素の場合、両方とも同じ要素を指す。
     */
    Particle *head_;
    Particle *tail_;
public:

    ParticleList() {
        head_ = tail_ = NULL;
    }

    /*
     * デストラクタでは、リストの全要素を削除する。
     */
    ~ParticleList() {
        Particle *p = head_;
        while (p != NULL) {
            Particle *q = p->next_;
            delete p;
            p = q;
        }
    }

    /*
     * リストの先頭要素を返す。
     * リストの要素を走査するループで使わせる。
     */
    Particle *head() {
        return head_;
    }

    /*
     * リストの末尾に要素を追加する。
     * 双方向リストの基本処理。
     */
    void add(Particle *p) {
        /*
         * リストが空か？
         */
        if (head_ == NULL) {
            // 空である。
            assert(head_ == tail_);
            head_ = tail_ = p;
            p->next_ = p->prev_ = NULL;
        } else {
            // 空でない。
            assert(head_->prev_ == NULL);
            assert(tail_->next_ == NULL);
            assert(head_ == tail_ || tail_->prev_->next_ == tail_);
            p->prev_ = tail_;
            p->next_ = NULL;
            tail_->next_ = p;
            tail_=p;
            assert(tail_->next_ == NULL);
            assert(tail_->prev_->next_ == tail_);
        }
    }

    /*
     * リストから要素を取り除く。
     * 追加の場合よりも場合分けが多いので注意を要する。
     * 場合分けを多少減らすテクニックも存在するが、分かりにくくなるので、
     * ここでは素直に４とおりに場合分けして実装する。
     */
    void remove(Particle *p) {
        assert(head_->prev_ == NULL);
        assert(tail_->next_ == NULL);
        assert(head_ == tail_ || tail_->prev_->next_ == tail_);
        if (head_ == p && tail_ == p) {
            /* 削除対象が先頭かつ末尾、つまり最後の一つである場合 */
            head_ = tail_ = NULL;
        } else if (tail_ == p) {
            /* 最後の一つではなくて、末尾である場合 */
            tail_ = tail_->prev_;
            tail_->next_ = NULL;
        } else if (head_ == p) {
            /* 最後の一つではなくて、先頭である場合 */
            head_ = head_->next_;
            head_->prev_ = NULL;
        } else {
            /* 末尾でも先頭でもない、つまり途中である場合 */
            assert(p->prev_ && p->next_);
            p->prev_->next_ = p->next_;
            p->next_->prev_ = p->prev_;
        }
    }

    /*
     * リストの末尾要素を外して、その要素を返す。
     * 空き要素リストとして使われているParticleListからParticleを一つ取得する場面で使う。
     */
    Particle *removeTail() {
        assert(tail_);
        Particle *p = tail_;
        remove(p);
        return p;
    }

    /*
     * 持っている全ての要素を、引数に渡された別のParticleListに
     * 追加する。自身の保持要素を空き要素リストに引き渡す時に使う。
     */
    void moveAllTo(ParticleList *otherList) {
        assert((head_ && tail_) || (!head_ && !tail_));
        /*
         * 自身が空である場合には、引き渡しは生じない。
         */
        if (head_ != NULL) {
            assert(tail_ != NULL);
            /*
             * 受け入れ側のリストに、自身が保有している要素リストの先頭と末尾を引き渡す。
             */
            otherList->insertList(head_, tail_);
            /*
             * 保有していた要素は、もはや自分のものではないので、
             * 要素を指していたポインタはNULLにする。
             */
            head_ = tail_ = NULL;
        }
    }

    /*
     * 要素のリストを受け入れ、自身のリストに追加する。
     */
    void insertList(Particle *head, Particle *tail) {
        /*
         * 追加するものがある時にだけ呼ばれることが前提である。
         */
        assert(head != NULL);
        assert(tail != NULL);
        /*
         * 渡された要素の両端は、すでに先頭、末尾にふさわしく
         * なっていることが前提である。
         */
        assert(head->prev_ == NULL);
        assert(tail->next_ == NULL);
        /*
         * 自身が空か否かで処理が変わる。
         */
        if (tail_ != NULL) {
            /*
             * 自身が空ではない場合には、追加処理となる
             */
            tail_->next_ = head;
            head->prev_ = tail_;
            tail_ = tail;
        } else {
            /*
             * 自身が空である場合には、渡されたリストがそのまま
             * 自身の要素一式となる。
             */
            head_ = head;
            tail_ = tail;
        }
    }

    /*
     * 要素を数える。
     * 全要素のtraverse(走査)が発生するため、双方向リストでは苦手な処理である。
     * ほんとうに個数が必要な時のみ使うこと。
     * 空か否かを調べるだけだったら、isEmptyを使うこと。
     */
    size_t count() const {
        size_t n = 0;
        Particle *p = head_;
        while (p != NULL) {
            n++;
            assert((p->next_ == NULL) || (p->next_->prev_ == p));
            p = p->next_;
        }
        return n;
    }

    /*
     * リストが空であるか調べるpredicate(述語関数)
     */
    bool isEmpty() const {
        return (head_ == NULL && tail_ == NULL);
    }

};

#endif
