package com.huoyu.gmap.svr.gmap.protocl;

import com.huoyu.gmap.svr.gmap.protocl.tool.MsgPo;
import com.huoyu.gmap.svr.gsrc.constant.ServiceType;

public class MsgDelPoint extends MsgPo {
    public long pointId;//顶点id
    
    public MsgDelPoint()
    {
        super(10240, true, (short)ServiceType.GMAP.value());
    }

    @Override
    public boolean Build(boolean isResult) {
        if ( isResult ) return false;//不创建回应报文

        SetId((short) ProtoclNo.DEL_POINT.value(), isResult);
        defaultParam();
        if ( !addInt64(pointId) ) return false;
        
        return true;
    }

    @Override
    public boolean Parse() {
        if ( !super.Parse() ) return false;
        if ( !isResult() ) return false;//不应该收到请求报文

        return true;
    }
}
