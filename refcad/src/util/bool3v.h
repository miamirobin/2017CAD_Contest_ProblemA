/****************************************************************************
  FileName     [ bool3v.h ]
  PackageName  [ util ]
  Synopsis     [ 3-value bool ]
  Author       [ music960633 ]
****************************************************************************/

#ifndef UTIL_BOOL3V_H_
#define UTIL_BOOL3V_H_

class bool_3v {
 public:
  bool_3v()                     : value(0) {}
  explicit bool_3v(unsigned v)  : value(v) {}
  explicit bool_3v(bool x)      : value(x) {}

  bool operator == (bool_3v b) const {
    return ((b.value&2) & (value&2)) | (!(b.value&2) & (value == b.value));
  }
  bool operator != (bool_3v b) const { return !(*this == b); }

  bool_3v operator && (bool_3v b) const {
    if (value == 0u || b.value == 0u) return bool_3v(0u);
    if (value == 2u || b.value == 2u) return bool_3v(2u);
    return bool_3v(1u);
  }
  bool_3v operator || (bool_3v b) const {
    if (value == 1u || b.value == 1u) return bool_3v(1u);
    if (value == 2u || b.value == 2u) return bool_3v(2u);
    return bool_3v(0u);
  }
  bool_3v operator ^ (bool_3v b) const {
    if (value == 2u || b.value == 2u) return bool_3v(2u);
    return bool_3v(value ^ b.value);
  }
  bool_3v operator !() const {
    if (value == 0u) return bool_3v(1u);
    if (value == 1u) return bool_3v(0u);
    return bool_3v(2u);
  }

 private:
  unsigned value;
};

const bool_3v false_3v(0u);
const bool_3v true_3v(1u);
const bool_3v undef_3v(2u);

#endif  // UTIL_BOOL3V_H_
