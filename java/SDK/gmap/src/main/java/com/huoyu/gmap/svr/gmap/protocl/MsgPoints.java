package com.huoyu.gmap.svr.gmap.protocl;

import java.util.ArrayList;
import java.util.List;

import com.huoyu.gmap.svr.gmap.datastruct.Point;
import com.huoyu.gmap.svr.gmap.protocl.tool.MsgPo;
import com.huoyu.gmap.svr.gsrc.constant.ServiceType;

//移动操作get/count动作，得到的终点
public class MsgPoints extends MsgPo {
    public List<Point>    points;//终点

    public MsgPoints(){
        super(10240, true, (short)ServiceType.GMAP.value());
        points = new ArrayList<Point>();
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

        points.clear();

        if ( pos >= size ) return false;

        for ( ; pos < size; )
        {
            Point point = getPoint();
            if ( readError ) return false;
            points.add(point);
        }

        return true;
    }
}
