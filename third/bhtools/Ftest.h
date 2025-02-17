
#ifndef FTEST_H
#define FTEST_H

#include "Tlog.h"

namespace bhtools {


// 值是否相等
#define BHTEST_EQUAL(a,b)                                                           \
    if(a == b)  vlogd("[SUCC] [BHTEST_EQUAL] [A: "<<a<<"] == [B: "<<b<<"]");        \
    else        vloge("[FAIL] [BHTEST_EQUAL] [A: "<<a<<"] != [B: "<<b<<"]");        \


// 值是否为真
#define BHTEST_TRUE(ok)                                         \
    if(ok)  vlogd("[SUCC] [BHTEST_TRUE] "<<$(ok));              \
    else    vloge("[FAIL] [BHTEST_TRUE] "<<$(ok));              \

    


} // bhtools





#endif // FTEST_H
