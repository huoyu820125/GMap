package com.huoyu.gmap.svr.gmap.datastruct;

import java.util.ArrayList;
import java.util.List;

//move操作1次移动选择的边，匹配条件
public class SelectLine {
    public boolean moveOut;//选出边move
    public boolean moveAllLine;//从moveOut指定的方向上所有路线移动，moveAllLine = true时，selectField无效
    public List<Filter> selectField;//选匹配字段的边
    public SelectLine(){
        moveOut = true;
        moveAllLine = false;
        selectField = new ArrayList<Filter>();
    }
}
