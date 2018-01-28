/****************************************************************************
  FileName     [ unate.h ]
  PackageName  [ util ]
  Synopsis     [ Unateness class ]
  Author       [ music960633 ]
****************************************************************************/

#ifndef UTIL_UNATE_H_
#define UTIL_UNATE_H_

/**************************************
  Class Unateness definition
**************************************/
class Unateness{
 public:
  Unateness(): val_(0) {}
  explicit Unateness(unsigned u): val_(u) {}
  Unateness(const Unateness& u) { val_ = u.val_; }
  Unateness& operator = (const Unateness& u) { val_ = u.val_; return (*this); }
  ~Unateness() {}

  unsigned getVal() const { return val_; }

  // functions for checking phases
  bool isNull()   const { return val_ == 0; }
  bool isPos()    const { return val_ == 1; }
  bool isNeg()    const { return val_ == 2; }
  bool isUnate()  const { return isPos() || isNeg(); }
  bool isBinate() const { return val_ == 3; }

  // operators
  void addPos() { val_ |= 1; }
  void addNeg() { val_ |= 2; }
  Unateness operator ~() { return Unateness((0xd8 >> (val_ << 1)) & 3); }
  Unateness operator | (Unateness u) { return Unateness(val_ | u.val_); }
  bool operator == (const Unateness& u) const { return val_ == u.val_; }
  bool operator != (const Unateness& u) const { return val_ != u.val_; }

 private:
  // 1st bit: pos phase used
  // 2nd bit: neg phase used
  unsigned val_;
};

const Unateness null_unate(0u);
const Unateness pos_unate(1u);
const Unateness neg_unate(2u);
const Unateness bi_unate(3u);

#endif  // UTIL_UNATE_H_
