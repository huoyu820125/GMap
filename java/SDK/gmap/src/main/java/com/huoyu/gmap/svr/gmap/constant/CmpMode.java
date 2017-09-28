package com.huoyu.gmap.svr.gmap.constant;

//比较模式-2<= -1<，0=，1>，2!=，3>=
public enum CmpMode {
    unknow((byte)99),//未定义
    left((byte)-2), //<
    leftEquals((byte)-1), //<=
    equals((byte)0),//=
    rightEquals((byte)1),//>
    right((byte)2),//>=
    unequals((byte)3);//!=

    private final byte    value;

    //构造器默认也只能是private, 从而保证构造函数只能在内部使用
    CmpMode(byte value) {
        this.value = value;
    }

    public boolean equals(byte v) {
        if (v == value) {
            return true;
        }

        return false;
    }

    public boolean equals(Byte v) {
        return equals(v.byteValue());
    }

    /*
     * 枚举byte类型值
     */
    public byte value() {
        return value;
    }

}
