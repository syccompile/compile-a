#include <cassert>
#include <initializer_list>
#include <iostream>
#include<cmath>

using Uint32 = unsigned int;
using Uint64 = unsigned long long;
using Int32 = int;
using Int64 = long long;

//  __builtin_clz  返回前导的0的个数
inline int clz(Uint32 x) { return __builtin_clz(x); }
//  __builtin_ctz  返回后面的0的个数
inline int ctz(Uint32 x) { return __builtin_ctz(x); }
// 具体的mulsh还要根据选择的的ARM乘法指令来定
Int32 mulsh(Int32 a, Int32 b) { return (Int64(a) * b) >> 32; }

// 根据除数d可以算出来一些后续需要的参数
struct param{
    int l;
    int sh;
    int m;
};

constexpr int N = 32;

// 这些div函数是generate_Signed_Division的运行结果
// 可以根据这个结果，将div转化为 mul和srl sra  ---需要给x86形式ir增加srl sra等指令
// div运行需要20-100：cycle,   mul: 3-5cycle  其他算数运算： 1cycle
int div1(int n) {
    return n;
}
int div_3(int n) {
int n_sign = -(n >> 31);
return  -(((n + mulsh(-1431655765 ,n)) >> 1) - n_sign);
}
int div_4(int n) {
int x = n >> (2-1);
int y = (Uint32)x >> (32-2);
return -(n + y) >> 2;
}
int div7(int n) {
    int n_sign = -(n >> 31);
    return  (((n + mulsh(-1840700269 ,n)) >> 2) - n_sign);
}
int div14(int n) {
    int n_sign = -(n >> 31);
    return  (((n + mulsh(-1840700269 ,n)) >> 3) - n_sign);
}
int div_31(int n) {
int n_sign = -(n >> 31);
return  -(((n + mulsh(-2078209981 ,n)) >> 4) - n_sign);
}
int div32(int n) {
    int x = n >> (5-1);
    int y = (Uint32)x >> (32-5);
    return (n + y) >> 5;
}
int div641(int n) {
    int n_sign = -(n >> 31);
    return  (((n + mulsh(-864353792 ,n)) >> 9) - n_sign);
}

// 得到d相关参数
param get_param(int d){
    assert(d != 0);
    int l0 = N - clz(std::fabs(d) - 1);
    int l = std::max(l0, 1);
    int sh = l - 1;
    Uint64  m = 1 + ((Uint64(1) << (N+l-1)) / std::fabs(d));
    int m_ = m - (Uint64(1) << N);
    param tmp;
    tmp.l = l;
    tmp.m = m_;
    tmp.sh = sh;
    return  tmp;
}

// 除法转乘法+移位
int div_to_mul(int n, int d){
    param my_param = get_param(d);
    if(d == 1){
        return n;
    }
    else if(d == -1){
        return mulsh(n, -1);
    }
    else if(d == (Uint32(1) << my_param.l)) {   // 如果d是2的l次方，直接右移
        int x = n >> (my_param.l-1);
        int y = (Uint32)x >> (N-my_param.l);
        return (n + y) >> my_param.l;
    }
    else if(d == -(Uint32(1) << my_param.l)) {   // 如果d是2的l次方，直接右移
        int x = n >> (my_param.l-1);
        int y = (Uint32)x >> (N-my_param.l);
        return -((n + y) >> my_param.l);
    }
    else{
        int n_sign = n<0? -1 : 0;
        return d < 0? -(((n + mulsh(my_param.m, n)) >> my_param.sh) - n_sign) \
        : (((n + mulsh(my_param.m, n)) >> my_param.sh) - n_sign);
    }

}

// 将转换结果打印
void generate_Signed_Division(int d){
    assert(d != 0);
    std::cout << "int div" << d << "(int n) {\n";
    param param_ = get_param(d);
    int l = param_.l;
    int sh = param_.sh;
    int m_ = param_.m;
    int q;
    if(d == 1){
        std::cout << "    return n;\n";
    }
    else if(d == -1){
        std::cout << "    return mulsh(n, -1);\n";
    }
    else if(d == (Uint32(1) << l)) {   // 如果d是2的l次方，直接右移
        std::cout << "    int x = n >> (" << l << "-1);\n";
        std::cout << "    int y = (Uint32)x >> (" << N << "-" << l << ");\n";
        std::cout << "    return (n + y) >> " << l << ";\n";
    }
    else if(d == -(Uint32(1) << l))  {
        std::cout << "    int x = n >> (" << l << "-1);\n";
        std::cout << "    int y = (Uint32)x >> (" << N << "-" << l << ");\n";
        std::cout << "    return -(n + y) >> " << l << ";\n";
    }
    else if(d < 0){
        std::cout << "    int n_sign = -(n >> 31);\n";
        std::cout << "    return  -(((n + mulsh(" << m_ << " ,n)) >> " << sh << ") - n_sign);\n";
    }
    else{
        std::cout << "    int n_sign = -(n >> 31);\n";
        std::cout << "    return  (((n + mulsh(" << m_ << " ,n)) >> " << sh << ") - n_sign);\n";
    }
    std::cout << "}\n";

}


int main() {
//    for(int d : std::initializer_list<int>{1, -3, -4, 7, 14, -31, 32, 641})
//        generate_Signed_Division(d);

    for(int i = 0; i < 100; i = i+3){
        std::cout << "i / -3 = " << div_3(i) << "   standard answer: " << i / -3 << std::endl;
    }
// todo
// 当前ir需要新增 SRL SRA指令
// 把ir中的div语句，转化为mul和移位语句
}