package com.huoyu.gmap.svr.gmap.protocl.tool;

import com.huoyu.client.MsgDefaultParam;
import com.huoyu.client.ResultCode;
import com.huoyu.gmap.svr.gmap.protocl.ProtoclNo;
import com.huoyu.gmap.svr.gsrc.constant.ServiceType;

public class MsgDelLine extends MsgDefaultParam {
    //请求参数
    public long     lineId;//边id

    //回应参数
    public long     startId;//起点id
    public long     endId;//终点id
    
    public MsgDelLine() {
        super(10240, true, (short)ServiceType.GMAP.value());
    }

    @Override
    public boolean Build(boolean isResult) {
        if ( isResult ) return false;//不创建回应报文
    
        SetId((short) ProtoclNo.DEL_LINE.value(), isResult);
        defaultParam();
        
        //请求参数
        if ( !addInt64(lineId) ) return false;

        return true;
    }

    @Override
    public boolean Parse() {
        if ( !super.Parse() ) return false;
        if ( !isResult() ) return false;//不应该收到请求报文
        
        if ( ResultCode.SUCCESS.value() != this.code ) return true;
        startId = getInt64();
        if ( readError ) return false;
        endId = getInt64();
        if ( readError ) return false;
        
        return true;
    }
}
