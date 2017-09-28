package com.huoyu.gmap.svr.gmap.protocl.tool;

import com.huoyu.client.MsgDefaultParam;
import com.huoyu.client.ResultCode;
import com.huoyu.gmap.svr.gmap.protocl.ProtoclNo;
import com.huoyu.gmap.svr.gsrc.constant.ServiceType;

public class MsgNewId extends MsgDefaultParam {
    //传递数据
    public Integer  m_count;//产生id数量

    //回应数据
    public Long m_id;//第一个唯一id
    
    public MsgNewId()
    {
        super(10240, true, (short)ServiceType.GMAP.value());
        m_count = 1;
    }

    @Override
    public boolean Build(boolean isResult) {
        if ( isResult ) return false;//不创建回应报文

        SetId((short) ProtoclNo.NEW_ID.value(), isResult);
        defaultParam();
        if ( !addInt32(m_count) ) return false;//产生id数量

        return true;
    }
    
    @Override
    public boolean Parse()
    {
        if ( !super.Parse() ) return false;
        if ( !isResult() ) return false;//不应该收到请求报文
        m_count = this.getInt32();//产生id数量
        if ( readError ) return false;
        
        if ( ResultCode.SUCCESS.value() != this.code ) return true;
        m_id = this.getInt64();
        if ( readError ) return false;
        
        return true;
    }
}
