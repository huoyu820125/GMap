package com.huoyu.gmap.svr.gmap.constant;

public enum Action {
    unknow(99),//未定义行为
    get(0), //取得可到达的顶点(支持多级move)
    count(1), //统计可到达的顶点数量(支持多级move)
    del(2), //删除可到达的顶点(不支持多级move)
    unlink(3); //"断开可到达的顶点(不支持多级move)

    private final int    value;

    //构造器默认也只能是private, 从而保证构造函数只能在内部使用
    Action(int value) {
        this.value = value;
    }

    public static Action toEnum(int value) {
        if (Action.get.equals(value)) {
            return Action.get;
        }
        else if (Action.count.equals(value)) {
            return Action.count;
        }
        else if (Action.del.equals(value)) {
            return Action.del;
        }
        else if (Action.unlink.equals(value)) {
            return Action.unlink;
        }
        
        return Action.unknow;
    }

    public static Action toEnum(Integer value) {
        return toEnum(value.intValue());
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
