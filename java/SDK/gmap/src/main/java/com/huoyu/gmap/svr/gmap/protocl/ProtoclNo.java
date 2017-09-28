package com.huoyu.gmap.svr.gmap.protocl;

/**
 * 协议号（报文类型，消息类型，消息Id）
 * 由于报文有moudleId做区分（服务类型ServiceType），
 * 所以每个服务提供的接口报文的协议号可以独自定义，不担心冲突
 * @author 孙尚天
 * @version $Id: ProtoclNo.java, v 0.1 2017年7月6日 下午15:27:23 孙尚天 Exp $
 */
public enum ProtoclNo {
    UNKNOW(-99, "未知消息"),
    NEW_ID(1001, "产生一个唯一id"),
    SET_POINT(1002, "设置顶点"),
    ADD_LINE_FOR_POINT(1003, "给顶点添加边"),
    DEL_LINE_FOR_POINT(1004, "删除顶点的边"),
    GET_POINT(1005, "取得顶点"),
    ADD_LINE(1006, "创建边"),
    DEL_LINE(1007, "删除边"),
    GET_LINE(1008, "取得边"),
    DEL_POINT(1009, "删除顶点"),
    SET_MOVE_OPT(1010, "设置move参数，move2 move1 filterPoint共用"),
    MOVE(1011, "多级移动"),
    POINTS(1012, "找到的顶点"),
    MOVE_MIDS(1013, "move操作中当前节点无数据的对象id"),
    //批量操作
    SET_POINTS(1014, "批量设置顶点"),
    LINK_POINTS(1015, "批量连接顶点"),
    DEL_POINTS(1016, "批量删除顶点");


    
    private final int    value;
    private final String descript;

    //构造器默认也只能是private, 从而保证构造函数只能在内部使用
    ProtoclNo(int value, String descript) {
        this.value = value;
        this.descript = descript;
    }

    public static ProtoclNo toEnum(int value) {
        if (ProtoclNo.NEW_ID.equals(value)) {
            return ProtoclNo.NEW_ID;
        }
        else if (ProtoclNo.SET_POINT.equals(value)) {
            return ProtoclNo.SET_POINT;
        }
        else if (ProtoclNo.ADD_LINE_FOR_POINT.equals(value)) {
            return ProtoclNo.ADD_LINE_FOR_POINT;
        }
        else if (ProtoclNo.DEL_LINE_FOR_POINT.equals(value)) {
            return ProtoclNo.DEL_LINE_FOR_POINT;
        }
        else if (ProtoclNo.GET_POINT.equals(value)) {
            return ProtoclNo.GET_POINT;
        }
        else if (ProtoclNo.ADD_LINE.equals(value)) {
            return ProtoclNo.ADD_LINE;
        }
        else if (ProtoclNo.DEL_LINE.equals(value)) {
            return ProtoclNo.DEL_LINE;
        }
        else if (ProtoclNo.GET_LINE.equals(value)) {
            return ProtoclNo.GET_LINE;
        }
        else if (ProtoclNo.DEL_POINT.equals(value)) {
            return ProtoclNo.DEL_POINT;
        }
        else if (ProtoclNo.SET_MOVE_OPT.equals(value)) {
            return ProtoclNo.SET_MOVE_OPT;
        }
        else if (ProtoclNo.MOVE.equals(value)) {
            return ProtoclNo.MOVE;
        }
        else if (ProtoclNo.POINTS.equals(value)) {
            return ProtoclNo.POINTS;
        }
        else if (ProtoclNo.MOVE_MIDS.equals(value)) {
            return ProtoclNo.MOVE_MIDS;
        }
        else if (ProtoclNo.SET_POINTS.equals(value)) {
            return ProtoclNo.SET_POINTS;
        }
        else if (ProtoclNo.LINK_POINTS.equals(value)) {
            return ProtoclNo.LINK_POINTS;
        }
        else if (ProtoclNo.DEL_POINTS.equals(value)) {
            return ProtoclNo.DEL_POINTS;
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
