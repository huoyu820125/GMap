package com.huoyu.client;


/**
 * 网络线路（运营商类型）
 * 
 * @author 孙尚天
 * @version $Id: NetLine.java, v 0.1 2016年11月30日 下午4:57:39 孙尚天 Exp $
 */
public enum NetLine {
    UNKNOW(-99, "未知线路"),
    CTCC(1, "中国电信  chinaTelecom"),
    CMCC(2, "中国移动通信 chinamobile"),
    CUCC(3, "中国联通通讯 chinaunicom"),
    LOCAL(4, "内网连接");

    private final int    value;
    private final String descript;

    //构造器默认也只能是private, 从而保证构造函数只能在内部使用
    NetLine(int value, String descript) {
        this.value = value;
        this.descript = descript;
    }

    public static NetLine toEnum(int value) {
        if (NetLine.CTCC.equals(value)) {
            return NetLine.CTCC;
        }
        else if (NetLine.CMCC.equals(value)) {
            return NetLine.CMCC;
        }
        else if (NetLine.CUCC.equals(value)) {
            return NetLine.CUCC;
        }
        else if (NetLine.LOCAL.equals(value)) {
            return NetLine.LOCAL;
        }
        
        return NetLine.UNKNOW;
    }

    public static NetLine toEnum(Integer value) {
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
