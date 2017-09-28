package com.huoyu.gmap.svr.gmap.constant;

public enum GridLimit {
    maxFieldCount(64), //顶点和边允许带有的最大字段数
    maxFieldNameSize(32),//字段名最大长度
    maxFieldSize(256),//字段最大长度
    maxFilter(16),//过滤条件最大个数
    maxMoveCount(10),//移动最大经过边数
    maxObjectCount(1000000);//批量处理最大对象数100万

    private final int    value;

    //构造器默认也只能是private, 从而保证构造函数只能在内部使用
    GridLimit(int value) {
        this.value = value;
    }

    public boolean equals(int v) {
        if (v == value) {
            return true;
        }

        return false;
    }

    public boolean equals(Integer v) {
        return equals(v.intValue());
    }

    /*
     * 枚举int类型值
     */
    public int value() {
        return value;
    }

}
