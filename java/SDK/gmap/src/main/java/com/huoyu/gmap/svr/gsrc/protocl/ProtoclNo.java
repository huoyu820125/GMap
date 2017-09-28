package com.huoyu.gmap.svr.gsrc.protocl;

/**
 * 协议号（报文类型，消息类型，消息Id）
 * 由于报文有moudleId做区分（服务类型ServiceType），
 * 所以每个服务提供的接口报文的协议号可以独自定义，不担心冲突
 * @author 孙尚天
 * @version $Id: ProtoclNo.java, v 0.1 2016年11月30日 下午12:20:23 孙尚天 Exp $
 */
public enum ProtoclNo {
    UNKNOW(-99, "未知消息"),
    GET_CLUSTER(1001, "取服务集群"),
    NEW_CLIENT_ID(1002, "创建调用端id");
    
    private final int    value;
    private final String descript;

    //构造器默认也只能是private, 从而保证构造函数只能在内部使用
    ProtoclNo(int value, String descript) {
        this.value = value;
        this.descript = descript;
    }

    public static ProtoclNo toEnum(int value) {
        if (ProtoclNo.GET_CLUSTER.equals(value)) {
            return ProtoclNo.GET_CLUSTER;
        }
        else if (ProtoclNo.NEW_CLIENT_ID.equals(value)) {
            return ProtoclNo.NEW_CLIENT_ID;
        }

        return ProtoclNo.UNKNOW;
    }

    public static ProtoclNo toEnum(Integer value) {
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

    /*
     * 枚举的字符串描述
     */
    public String descript() {
        return descript;
    }
}
