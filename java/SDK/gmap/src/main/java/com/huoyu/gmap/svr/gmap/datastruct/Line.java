package com.huoyu.gmap.svr.gmap.datastruct;

import java.util.HashMap;
import java.util.Map;

//图数据库对象-边
public class Line {
    public long id;//边id
    public long startId;//起点id
    public long endId;//终点id
    public Map<String, Field>   data;//字段
    
    public Line()
    {
        data = new HashMap<String, Field>();
    }
}
