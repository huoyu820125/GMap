package com.huoyu.gmap.svr.gmap.protocl;

import java.util.List;

import com.huoyu.client.MsgDefaultParam;
import com.huoyu.gmap.svr.gsrc.constant.ServiceType;

//从起顶点/边开始移动N次，并执行预设的行为
public class MsgMove extends MsgDefaultParam {
    //传递数据
    public long                searchId;//搜索唯一id
    public boolean             fromPoint;//从顶点开始移动
    public boolean             startFilter;//过滤起点
    public List<Long>          ids;//起点id列表
    public boolean             moveStart;//开始移动
    public int                 startPos;//id列表打包开始位置
    //回应数据

    public MsgMove(){
        super(10240, true, (short)ServiceType.GMAP.value());
    }
    
    @Override
    public boolean Build(boolean isResult) {
        if ( isResult ) return false;//不创建回应报文

        SetId((short) ProtoclNo.MOVE.value(), isResult);
        defaultParam();

        //请求
        if ( !addInt64(searchId) ) return false;//搜索唯一id
        int count = ids.size() - startPos;
        if ( 0 == count ) return false;

        if ( count > 1000 ) {
            count = 1000;
            moveStart = false;
        }
        else moveStart = true;
        if ( !addBoolean(moveStart) ) return false;
        if ( !addInt16((short)count) ) return false;
        int i = 0;
        for ( ; i < count; i++ )
        {
            if ( !addInt64(ids.get(startPos)) ) return false;
            startPos++;
        }
        
        if ( !moveStart ) return true;
        
        if ( !addBoolean(fromPoint) ) return false;//从顶点开始移动
        if ( !addBoolean(startFilter) ) return false;//之前move操作不在本地的远端对象


        return true;
    }

    @Override
    public boolean Parse() {
        if ( !super.Parse() ) return false;
        if ( !isResult() ) return false;//不应该收到请求报文
        
        return true;
    }
}
