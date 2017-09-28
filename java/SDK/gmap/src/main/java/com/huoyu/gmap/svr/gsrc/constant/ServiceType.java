package com.huoyu.gmap.svr.gsrc.constant;

/**
 * 服务类型
 * 必须所有服务公用，避免枚举值定义冲突
 * @author 孙尚天
 * @version $Id: ServiceType.java, v 0.1 2016年11月28日 下午9:30:02 孙尚天 Exp $
 */
public enum ServiceType {
    UNKNOW(-99, "未知类型服务"),
    GSRC(1, "资源服务"),
    GMAP(2, "图数据库服务");

    private final int    value;
    private final String descript;

    //构造器默认也只能是private, 从而保证构造函数只能在内部使用
    ServiceType(int value, String descript) {
        this.value = value;
        this.descript = descript;
    }

    public static ServiceType toEnum(int value) {
        if (ServiceType.GSRC.equals(value)) {
            return ServiceType.GSRC;
        }
        else if (ServiceType.GMAP.equals(value)) {
            return ServiceType.GMAP;
        }

        return ServiceType.UNKNOW;
    }

    public static ServiceType toEnum(Integer value) {
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
