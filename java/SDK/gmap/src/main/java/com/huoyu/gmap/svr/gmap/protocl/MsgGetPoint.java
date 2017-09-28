package com.huoyu.gmap.svr.gmap.protocl;

import com.huoyu.client.ResultCode;
import com.huoyu.gmap.svr.gmap.datastruct.Point;
import com.huoyu.gmap.svr.gmap.protocl.tool.MsgPo;
import com.huoyu.gmap.svr.gsrc.constant.ServiceType;

public class MsgGetPoint extends MsgPo {
    //请求参数
    public long     pointId;//顶点id

    // 回应参数
    public Point    point;     //顶点
    
    public MsgGetPoint() {
        super(10240, true, (short)ServiceType.GMAP.value());
    }

    @Override
    public boolean Build(boolean isResult) {
        if ( isResult ) return false;//不创建回应报文

        SetId((short) ProtoclNo.GET_POINT.value(), isResult);
        defaultParam();
        if ( !addInt64(pointId) ) return false;
        
        return true;
    }

    @Override
    public boolean Parse() {
        if ( !super.Parse() ) return false;
        if ( !isResult() ) return false;//不应该收到请求报文

        if ( ResultCode.SUCCESS.value() != this.code ) return true;
        point = getPoint();
        if ( readError ) return false;
        
        return true;

    }
}
