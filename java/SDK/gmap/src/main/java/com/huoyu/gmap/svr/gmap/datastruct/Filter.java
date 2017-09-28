package com.huoyu.gmap.svr.gmap.datastruct;

//查询过滤条件
public class Filter {
    public byte         cmpMode;//比较方式-2<= -1<，0=，1>，2!=，3>=
    public Field        field;//字段
    public Filter(){
        field = new Field();
    }
}
