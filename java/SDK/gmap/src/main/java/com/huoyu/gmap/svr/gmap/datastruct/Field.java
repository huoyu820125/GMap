package com.huoyu.gmap.svr.gmap.datastruct;

import java.io.UnsupportedEncodingException;

import com.huoyu.gmap.svr.gmap.constant.FieldType;

//数据库对象属性
public class Field {
    public String       fieldName;//字段名不带\0
    public FieldType    type;//字段类型
    public long         value;//保存整型数据
    public String       data;//保存字符串，序列化类型数据
    
    public Field(){
    }
    
    public Field(String name, String value){
        fieldName = name;
        setValue(value);
    }
    
    //取字段值
    public String getValue(){
        if ( FieldType.str == type ) return data;
        return String.valueOf(value);
    }
    //设置值
    public void setValue(String value){
        if ( null == value || value.isEmpty() ) return;
        type = FieldType.str;
        this.data = value;
        
        byte[] gbk;
        try {
            gbk = value.getBytes("gbk");
        } catch (UnsupportedEncodingException e) {
            return;
        }
        
        int i = 0;
        if ( '-' == gbk[i] ) i++;//允许负号
        if ( i >= gbk.length || '0' == gbk[i] ) return;//只有一个负号，或者0开头，为字符串
        
        //对于数值型字段，不再允许出现非数字字符
        for ( ; i < gbk.length; i++ ){
            if ( !('0' <= gbk[i] && '9' >= gbk[i]) ) return;//存在非数字，为字符串
        }
        
        //转为数字
        try {
            this.value = Long.valueOf(value);
        } catch (NumberFormatException e) {
            return;//超出数字范围，使用字符串保存
        }
        type = FieldType.int64;//修正类型
        
        return;
    }
}
