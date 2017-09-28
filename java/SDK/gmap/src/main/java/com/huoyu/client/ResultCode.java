package com.huoyu.client;

/**
 * 结果码
 * @author 孙尚天
 * @version $Id: ResultCode.java, v 0.1 2016年11月23日 下午9:11:44 孙尚天 Exp $
 */
public enum ResultCode {
    UNKNOW(-99, "未定义错误"),
    //普通订单状态
    SUCCESS(0, "成功"), 
    PARAM_ERROR(1, "参数错误"), 
    NO_SERVER(2, "服务无响应"), 
    MSG_FORMAT_ERROR(3, "报文格式错误"), 
    REFUSE(4, "拒绝"), 
    TIME_OUT(5, "超时"), 
    NO_DATA(6, "无数据"), 
    IP_ERROR(7, "服务配置错误"),
    SYS_ERROR(8, "操作系统异常"),
    UN_INIT(9, "未初始化");

 
    private final int    value;
    private final String descript;

    //构造器默认也只能是private, 从而保证构造函数只能在内部使用
    ResultCode(int value, String descript) {
        this.value = value;
        this.descript = descript;
    }

    public static ResultCode toEnum(int value) {
        if (ResultCode.SUCCESS.equals(value))
            return ResultCode.SUCCESS;
        else if (ResultCode.IP_ERROR.equals(value))
            return ResultCode.IP_ERROR;
        else if (ResultCode.PARAM_ERROR.equals(value))
            return ResultCode.PARAM_ERROR;
        else if (ResultCode.NO_SERVER.equals(value))
            return ResultCode.NO_SERVER;
        else if (ResultCode.TIME_OUT.equals(value))
            return ResultCode.TIME_OUT;
        else if (ResultCode.MSG_FORMAT_ERROR.equals(value))
            return ResultCode.MSG_FORMAT_ERROR;
        else if (ResultCode.REFUSE.equals(value))
            return ResultCode.REFUSE;
        else if (ResultCode.NO_DATA.equals(value))
            return ResultCode.NO_DATA;
        else if (ResultCode.SYS_ERROR.equals(value))
            return ResultCode.SYS_ERROR;
        else if (ResultCode.UN_INIT.equals(value))
            return ResultCode.UN_INIT;

        return ResultCode.UNKNOW;
    }

    public static ResultCode toEnum(Integer value) {
        return toEnum(value.intValue());
    }

    public boolean equals(int v) {
        if (v == value)
            return true;
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
