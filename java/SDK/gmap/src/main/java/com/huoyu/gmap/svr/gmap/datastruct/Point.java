package com.huoyu.gmap.svr.gmap.datastruct;

import java.util.HashMap;
import java.util.Map;

//图数据库对象-顶点
public class Point {
    public long id;//顶点id
    public Map<String, Field>   data;//字段
    
    public Point()
    {
        data = new HashMap<String, Field>();
    }
}
