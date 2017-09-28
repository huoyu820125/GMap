package com.huoyu.gmap.svr.gmap.datastruct;

import java.util.HashMap;
import java.util.Map;

//move操作需要到其它节点完成查询的中间数据
public class MoveMidData {
    public byte lineIndex;//产生中间数据的移动操作，当前移动到的边的顺序号
    public Map<Long, Boolean> lineIds;//lineIndex这条路边上的移动，未完成的边id
    public Map<Long, Boolean> pointIds;//lineIndex这条路边上的移动，未完成的顶点id
    
    public MoveMidData(byte lineIndex){
        this.lineIndex = lineIndex;
        lineIds = new HashMap<Long, Boolean>();
        pointIds = new HashMap<Long, Boolean>();
    }
}
