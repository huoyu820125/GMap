package com.huoyu.gmap.svr.gsrc.protocl;

import com.huoyu.client.MsgDefaultParam;
import com.huoyu.client.ResultCode;
import com.huoyu.gmap.svr.gsrc.constant.ServiceType;

public class MsgNewClientId extends MsgDefaultParam{

    //请求参数

    // 回应参数
    public long       m_clientId;//cient唯一id

    public MsgNewClientId() {
        super(10240, true, (short)ServiceType.GSRC.value());
    }

    @Override
    public boolean Build(boolean isResult) {
        if ( isResult ) return false;//不创建回应报文
    
        SetId((short) ProtoclNo.NEW_CLIENT_ID.value(), isResult);
        defaultParam();

        return true;
    }

    @Override
    public boolean Parse() {
        if ( !super.Parse() ) return false;
        if ( !isResult() ) return false;//不解析请求报文

        // 回应参数
        if (!isResult()) {
            return false;
        }
        if (ResultCode.SUCCESS.value() != code) {
            return true;
        }

        //服务列表
        m_clientId = getInt64();//readError
        if (readError) {
            return false;
        }

        return true;
    }
}
