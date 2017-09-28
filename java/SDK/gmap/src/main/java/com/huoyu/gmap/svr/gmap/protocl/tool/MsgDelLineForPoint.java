package com.huoyu.gmap.svr.gmap.protocl.tool;

import com.huoyu.client.MsgDefaultParam;
import com.huoyu.gmap.svr.gmap.protocl.ProtoclNo;
import com.huoyu.gmap.svr.gsrc.constant.ServiceType;

public class MsgDelLineForPoint extends MsgDefaultParam {
    //请求参数
    public long        pointId;//顶点id
    public long        lineId;//边id
    public boolean     isOut;//边的方向是出去

    public MsgDelLineForPoint() {
        super(10240, true, (short)ServiceType.GMAP.value());
    }

    @Override
    public boolean Build(boolean isResult) {
        if ( isResult ) return false;//不创建回应报文

        SetId((short) ProtoclNo.DEL_LINE_FOR_POINT.value(), isResult);
        defaultParam();
        
        //请求参数
        if ( !addInt64(pointId) ) return false;
        if ( !addInt64(lineId) ) return false;
        if ( !addBoolean(isOut) ) return false;

        return true;
    }

    @Override
    public boolean Parse() {
        if ( !super.Parse() ) return false;
        if ( !isResult() ) return false;//不应该收到请求报文

        return true;
    }
}
