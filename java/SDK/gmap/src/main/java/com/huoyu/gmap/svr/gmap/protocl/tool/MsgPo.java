package com.huoyu.gmap.svr.gmap.protocl.tool;

import java.io.UnsupportedEncodingException;
import java.util.HashMap;
import java.util.Map;

import com.huoyu.client.MsgDefaultParam;
import com.huoyu.gmap.svr.gmap.constant.FieldType;
import com.huoyu.gmap.svr.gmap.constant.GridLimit;
import com.huoyu.gmap.svr.gmap.datastruct.Field;
import com.huoyu.gmap.svr.gmap.datastruct.Line;
import com.huoyu.gmap.svr.gmap.datastruct.Point;

public class MsgPo  extends MsgDefaultParam {

    public MsgPo(int size, boolean smallOrder, short moudleId) {
        super(size, smallOrder, moudleId);
    }
    
    protected boolean addPoint( Point point ){
        if (!addInt64(point.id)) return false;//顶点id
        return addFields(point.data);
    }
    
    protected Point getPoint()
    {
        Point point = new Point();
        point.id = getInt64();
        if ( readError ) return null;//顶点id
        
        point.data = getFields();
        if ( readError ) return null;
        
        return point;
    }
    
    protected boolean addLine( Line line ){
        if (!addInt64(line.id)) return false;//边id
        if (!addInt64(line.startId)) return false;//起点id
        if (!addInt64(line.endId)) return false;//终点id
        return addFields(line.data);
    }
    
    protected Line getLine()
    {
        Line line = new Line();
        line.id = getInt64();
        if ( readError ) return null;//顶点id
        line.startId = getInt64();
        if ( readError ) return null;//起点id
        line.endId = getInt64();
        if ( readError ) return null;//终点id

        line.data = getFields();
        if ( readError ) return null;
        
        return line;
    }
    
    protected boolean addFields( Map<String, Field> data ){
        //字段数
        int count = data.size();
        if ( GridLimit.maxFieldCount.value() < count ) return false;
        if (!addUInt8((short)count)) return false;
        if ( 0 == data.size() ) return true;
        
        for ( Map.Entry<String, Field> entry : data.entrySet() )
        {
            entry.getValue().fieldName = entry.getKey();
            if ( !addField(entry.getValue()) ) return false;
        }

        return true;
    }
    
    protected Map<String, Field> getFields()
    {
        //字段数
        short count = getUInt8();
        if ( GridLimit.maxFieldCount.value() < count ) 
        {
            readError = true;
            return null;
        }
        if ( readError ) return null;

        Map<String, Field> data = new HashMap<String, Field>();
        int i = 0;
        Field field = null;
        for ( ; i < count; i++ )
        {
            field = getField();//字段
            if ( readError ) return null;
            data.put(field.fieldName, field);
        }

        return data;
    }
    
    protected boolean addField( Field field )
    {
        int fieldSize = 0;
        try {
            fieldSize = field.fieldName.getBytes("gbk").length;
        } catch (UnsupportedEncodingException e) {
            return false;
        }
        if ( GridLimit.maxFieldNameSize.value() < fieldSize || 0 == fieldSize ) return false;
        if (!this.addStream(field.fieldName)) return false;//字段名
        if (!this.addInt8((byte)field.type.value())) return false;//字段类型
        if (!this.addInt64(field.value)) return false;//字段值
        if ( FieldType.str != field.type ) return true;
        
        try {
            fieldSize = field.data.getBytes("gbk").length;
        } catch (UnsupportedEncodingException e) {
            return false;
        }
        if ( GridLimit.maxFieldSize.value() < fieldSize || 0 == fieldSize ) return false;
        if (!this.addString(field.data)) return false;//属性流类型值

        return true;
    }

    protected Field getField()
    {
        Field field = new Field();

        //字段名
        field.fieldName = getStream();
        if ( readError ) return null;
        int fieldSize = 0;
        try {
            fieldSize = field.fieldName.getBytes("gbk").length;
        } catch (UnsupportedEncodingException e) {
            readError = true;
            return null;
        }
        if ( GridLimit.maxFieldNameSize.value() < fieldSize || 0 == fieldSize )
        {
            readError = true;
            return null;
        }

        //字段类型
        field.type = FieldType.toEnum(getInt8());
        if ( readError ) return null;
        //字段值
        field.value = getInt64();
        if ( readError ) return null;
        if ( FieldType.str != field.type ) return field;
        
        //属性流类型值
        field.data = getString();
        if ( readError ) return null;
        try {
            fieldSize = field.data.getBytes("gbk").length;
        } catch (UnsupportedEncodingException e) {
            readError = true;
            return null;
        }
        if ( GridLimit.maxFieldSize.value() < fieldSize || 0 == fieldSize )
        {
            readError = true;
            return null;
        }
        
        return field;
    }

}
