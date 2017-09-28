package com.huoyu.gmap.svr.gmap.protocl.tool;

import com.huoyu.gmap.svr.gmap.datastruct.Line;
import com.huoyu.gmap.svr.gmap.protocl.ProtoclNo;
import com.huoyu.gmap.svr.gsrc.constant.ServiceType;

public class MsgAddLine extends MsgPo {
    public Line line;
    
    public MsgAddLine() {
        super(10240, true, (short)ServiceType.GMAP.value());
        line = new Line();
    }

    @Override
    public boolean Build(boolean isResult) {
        if ( isResult ) return false;//不创建回应报文

        SetId((short) ProtoclNo.ADD_LINE.value(), isResult);
        defaultParam();
        return addLine(line);
    }

    @Override
    public boolean Parse() {
        if ( !super.Parse() ) return false;
        if ( !isResult() ) return false;//不应该收到请求报文

        return true;
    }
}
