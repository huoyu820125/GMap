package com.huoyu.gmap.svr.gmap.protocl;

import java.util.List;

import com.huoyu.gmap.svr.gmap.constant.Action;
import com.huoyu.gmap.svr.gmap.constant.GridLimit;
import com.huoyu.gmap.svr.gmap.datastruct.Filter;
import com.huoyu.gmap.svr.gmap.datastruct.SelectLine;
import com.huoyu.gmap.svr.gmap.protocl.tool.MsgPo;
import com.huoyu.gmap.svr.gsrc.constant.ServiceType;

//move查找条件，操作参数
public class MsgSetMoveOpt extends MsgPo {
    public List<SelectLine>     selectLines; //移动路线
    public List<Filter>         filters;//过滤条件
    public boolean              selectAll;//选取所有字段=true时selectFields无效
    public List<String>         selectFields;//选取的字段
    public Action               act; //目标动作

    public MsgSetMoveOpt(){
        super(10240, true, (short)ServiceType.GMAP.value());
    }

    @Override
    public boolean Build(boolean isResult) {
        if ( isResult ) return false;//不创建回应报文

        SetId((short) ProtoclNo.SET_MOVE_OPT.value(), isResult);
        defaultParam();

        //移动路线
        short lineCount = (short)selectLines.size();
        if ( GridLimit.maxMoveCount.value() < lineCount ) return false;
        if ( !addUInt8(lineCount) ) return false;
        int j = 0;
        int i = 0;
        short count;
        for ( j = 0; j < lineCount; j++ )
        {
            SelectLine lineFilter = selectLines.get(j);
            //移动方向是出边/入边
            if ( !addBoolean(lineFilter.moveOut) ) return false;
            //是否从该移动方向所有边移动
            if ( !addBoolean(lineFilter.moveAllLine) ) return false;
            //不从所有的边移动，根据条件选择移动的边
            if ( !lineFilter.moveAllLine ){
                count = (short)lineFilter.selectField.size();
                if ( GridLimit.maxFilter.value() < count || 0 == count ) return false;
                if ( !addUInt8(count) ) return false;
                for ( i = 0; i < count; i++ )
                {
                    if ( !addInt8(lineFilter.selectField.get(i).cmpMode) ) return false;
                    if ( !addField(lineFilter.selectField.get(i).field) ) return false;
                }
            }
        }
        
        //过滤条件
        count = (short)filters.size();
        if ( GridLimit.maxFilter.value() < count ) return false;
        if ( !addUInt8(count) ) return false;
        for ( i = 0; i < count; i++ )
        {
            if ( !addInt8(filters.get(i).cmpMode) ) return false;
            if ( !addField(filters.get(i).field)) return false;
        }

        if ( !addBoolean(selectAll) ) return false;//选取所有字段=true时selectFields无效
        
        //选取字段
        count = (short)selectFields.size();//2048+2
        if ( GridLimit.maxFieldCount.value() < count ) return false;
        if ( !addUInt8(count) ) return false;
        for ( i = 0; i < count; i++ )
        {
            if ( selectFields.get(i).length() > GridLimit.maxFieldNameSize.value() ) return false;
            if ( !addString(selectFields.get(i)) ) return false;
        }

        //目标动作
        if ( !addInt8((byte)act.value()) ) return false;
        
        return true;
    }

    @Override
    public boolean Parse() {
        return false;//不存在回应报文
    }
}
