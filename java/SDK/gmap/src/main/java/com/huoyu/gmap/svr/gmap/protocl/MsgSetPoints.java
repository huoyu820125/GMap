package com.huoyu.gmap.svr.gmap.protocl;

import java.util.List;

import com.huoyu.gmap.svr.gmap.datastruct.Point;
import com.huoyu.gmap.svr.gmap.protocl.tool.MsgPo;
import com.huoyu.gmap.svr.gsrc.constant.ServiceType;

public class MsgSetPoints extends MsgPo {
    //请求参数
    public boolean          isEnd;     //顶点列表结束
    public boolean          isCreate; //是创建操作
    public List<Point>      points;    //顶点列表
    public int              startPos;  //从第几个顶点开始打包

    // 回应参数
    
    public MsgSetPoints() {
        super(10240, true, (short)ServiceType.GMAP.value());
    }

    @Override
    public boolean Build(boolean isResult) {
        if ( isResult ) return false;//不创建回应报文

        SetId((short) ProtoclNo.SET_POINTS.value(), isResult);
        defaultParam();
        
        int count = points.size();
        isEnd = startPos < count?false:true;
        if ( !addBoolean(isEnd) ) return false;//顶点列表结束
        if ( !addBoolean(isCreate) ) return false;//是创建操作
        if ( isEnd ) return true;
        
        int i = startPos;
        int size = this.size;
        for ( ; i < count; i++ ){
            size = this.size;
            if ( !addPoint(points.get(i)) ) {
                startPos = i;//下次开始打包位置
                this.size = size;//确保截止长度都是完整有效数据的，避免发送时将半打包的数据发送出去
                return true;
            }
        }
        startPos = i;//下次开始打包位置
        
        return true;
    }

    @Override
    public boolean Parse() {
        if ( !super.Parse() ) return false;
        if ( !isResult() ) return false;//不应该收到请求报文

        return true;

    }
}
