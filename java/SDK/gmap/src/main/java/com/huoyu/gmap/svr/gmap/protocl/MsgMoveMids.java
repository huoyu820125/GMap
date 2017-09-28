package com.huoyu.gmap.svr.gmap.protocl;

import java.util.ArrayList;
import java.util.List;

import com.huoyu.client.MsgDefaultParam;
import com.huoyu.gmap.svr.gsrc.constant.ServiceType;

//move移动操作时，当前节点无数据，需要到其它节点查找的对象id
public class MsgMoveMids extends MsgDefaultParam {
    public byte                lineIndex;//ids的对象是经过了多少条边移动
    public boolean             isLine;//true ids是边，false ids是顶点
    public List<Long>          ids;//对象id

    public MsgMoveMids(){
        super(10240, true, (short)ServiceType.GMAP.value());
        ids = new ArrayList<Long>();
    }

    @Override
    public boolean Build(boolean isResult) {
        return false;//不应该创建数据
    }
    
    @Override
    public boolean Parse()
    {
        if ( !super.Parse() ) return false;
        if ( !isResult() ) return false;//不存在请求报文

        lineIndex = getInt8();
        if ( readError ) return false;
        isLine = getBoolean();
        if ( readError ) return false;

        if ( pos >= size ) return false;

        long id;
        ids.clear();
        for ( ; pos < size ; )
        {
            id = getInt64();
            if ( readError ) return false;
            ids.add(id);
        }

        return true;
    }
}
