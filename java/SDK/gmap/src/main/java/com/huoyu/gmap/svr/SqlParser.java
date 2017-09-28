package com.huoyu.gmap.svr;

import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.huoyu.gmap.svr.gmap.constant.CmpMode;
import com.huoyu.gmap.svr.gmap.constant.FieldType;
import com.huoyu.gmap.svr.gmap.datastruct.Field;
import com.huoyu.gmap.svr.gmap.datastruct.Filter;
import com.huoyu.gmap.svr.gmap.datastruct.Point;

//sql解析工具
public class SqlParser {
    public String reason;
    String utfSql;
    byte[] gbk;
    private int pos;//处理位置
    
    public SqlParser( String text ){
        reason = null;
        utfSql = text;
        try {
            gbk = text.getBytes("gbk");
        } catch (UnsupportedEncodingException e) {
            gbk = null;
            reason = "无法转为gbk编码";
            return;
        }
        pos = 0;
        
        return;
    }
    
    //绑定字符串
    public boolean BindString( String text ){
        reason = null;
        utfSql = text;
        try {
            gbk = text.getBytes("gbk");
        } catch (UnsupportedEncodingException e) {
            gbk = null;
            reason = "无法转为gbk编码";
            return false;
        }
        pos = 0;
        
        return true;
    }

    //处理位置到开头
    public void ToStart(){
        pos = 0;
        return;
    }
    
    //处理位置到开头
    public void ToEnd(){
        pos = gbk.length - 1;
        return;
    }
    
    //处理位置向后移动count字节
    public void Next(int count){
        if ( pos + count >= gbk.length ) return;
        pos += count;
        
        return;
    }
    
    //处理位置向回移动count字节
    public void Back(int count){
        if ( pos - count < 0 ) return;
        pos -= count;
        
        return;
    }
    
    //到达末尾
    public boolean isEnd(){
        int curPos = pos;
        moveSpace();
        if ( ';' != gbk[pos] ) {
            pos = curPos;
            return false;
        }
        pos++;
        moveSpace();
        if ( pos != gbk.length ) {
            pos = curPos;
            return false;
        }
        pos = curPos;
        return true;
    }
    
    //是空白符
    private boolean isSpace(byte gbk){
        if ( ' ' == gbk ) return true;
        if ( '\t' == gbk ) return true;
        if ( '\n' == gbk ) return true;
        if ( '\r' == gbk ) return true;
        
        return false;
    }
    
    //是数字
    private boolean isNum(byte gbk){
        if ( '0' <= gbk && '9' >= gbk ) return true;

        return false;
    }

    //是英文字母
    private boolean isLetter(byte gbk){
        if ( 'a' <= gbk && 'z' >= gbk ) return true;
        if ( 'A' <= gbk && 'Z' >= gbk ) return true;

        return false;
    }
    

    //当前已分析的sql
    private String readedSql(){
        String value = null;
        try {
            value = new String(gbk, 0, pos, "gbk");
        } catch (UnsupportedEncodingException e) {
            return "{}";
        }
        
        return "{" + value + "}";
    }
    
    //跳过空白符号
    private void moveSpace(){
        for ( ; pos < gbk.length; pos++ ){
            if ( isSpace(gbk[pos]) ) continue;
            break;
        }
        
        return;
    }
    
    //取单词
    private String getWord(){
        if ( null == gbk ) return null;
        moveSpace();//跳过空白符号
        if ( 0 > pos || pos >= gbk.length ) {
            reason = "null";
            return null;
        }
        
        
        int i = 0;
        byte[]item = new byte[gbk.length];
        for ( ; pos < gbk.length; pos++ ){
            if ( isSpace(gbk[pos]) || ';' == gbk[pos] ) {
                String value = null;
                try {
                    value = new String(item, 0, i, "gbk");
                } catch (UnsupportedEncodingException e) {
                    reason = "包含非法编码";
                    return null;
                }
                return value;
            }
            if ( !isLetter(gbk[pos]) ){
                reason = "包含非英文字符";
                return null;
            }
            item[i++] = gbk[pos];
        }
        reason = "null";
        
        return null;
    }

    //取动作
    public String getAction(){
        String op = getKeyWord(true);
        if ( null == op ){
            reason = "头部缺少操作申明：" + reason;
            return null;
        }
        if ( op.equals("adds") || op.equals("links") || op.equals("edits") ){
            reason = "尚未支持批操作" + op;
            return null;
        }
        if ( !op.equals("add") && !op.equals("edit") && !op.equals("del") 
                && !op.equals("link") && !op.equals("unlink") 
                && !op.equals("count") && !op.equals("get") 
                ){
            reason = "未定义的操作申明：" + op;
        }
        
        return op;
    }

    //取字段名
    private String getFieldName(){
        if ( null == gbk ) return null;
        moveSpace();//跳过空白符号
        if ( 0 > pos || pos >= gbk.length ) {
            reason = "字段名 = null";
            return null;
        }

        if ( pos >= gbk.length || '[' != gbk[pos] ) {
            reason = "字段名需要使用[]包围";
            return null;
        }
        pos++;
        
        int i = 0;
        byte[] item = new byte[gbk.length];
        for ( ; pos < gbk.length; pos++ ){
            if ( ']' != gbk[pos] ) {
                item[i++] = gbk[pos];
                continue;
            }
            if ( pos - 1 >= 0 && '\\' == gbk[pos - 1] ){
                item[i++] = gbk[pos];
                continue;
            }
            pos++;
            break;
        }
        String fieldName = null;
        try {
            fieldName = new String(item, 0, i, "gbk");
        } catch (UnsupportedEncodingException e) {
            reason = "字段名包含非法编码";
            return null;
        }
        
        return fieldName;
    }

    //取运算符
    private Byte getOperator(){
        if ( null == gbk ) return null;
        moveSpace();//跳过空白符号
        if ( 0 > pos || pos >= gbk.length ) {
            reason = "运算符 = null";
            return null;
        }

        if ( '<' == gbk[pos] ){
            pos++;
            if ( pos < gbk.length && '=' == gbk[pos] ){
                pos++;
                return CmpMode.leftEquals.value();//<=
            }
            return CmpMode.left.value();//<
        }
        if ( '=' == gbk[pos] ){
            pos++;
            return CmpMode.equals.value();//=
        }
        if ( '>' == gbk[pos] ){
            pos++;
            if ( pos < gbk.length && '=' == gbk[pos] ){
                pos++;
                return CmpMode.rightEquals.value();//>=
            }
            return CmpMode.right.value();//>
        }
        if ( '!' == gbk[pos] ){
            pos++;
            if ( pos > gbk.length || '=' != gbk[pos] ){
                reason = "运算符[!]之后缺少=";
                return null;
            }
            pos++;
            return CmpMode.unequals.value();//!=
        }
        
        reason = "运算符 = null";
        return null;
    }
    //取字段值
    private Boolean getFieldValue( Field field ){
        if ( null == gbk ) return false;
        if ( null == field ) return false;
        moveSpace();//跳过空白符号
        if ( 0 > pos || pos >= gbk.length ) {
            reason = "字段值 = null";
            return false;
        }

        if ( '[' != gbk[pos] ) {
            reason = "字段值需要使用[]包围";
            return false;
        }
        pos++;
        
        int i = 0;
        field.type = FieldType.int64;
        byte[] item = new byte[gbk.length];
        if ( '-' == gbk[pos] ) {//保存负号
            item[i++] = gbk[pos];
            pos++;
        }
        if ( pos >= gbk.length || '0' == gbk[pos] ) field.type = FieldType.str;//只有一个符号，或者0开头，为字符串
        
        //对于数值型字段，不再允许出现非数字字符
        for ( ; pos < gbk.length; pos++ ){
            if ( ']' != gbk[pos] ) {
                if (!isNum(gbk[pos]) ) field.type = FieldType.str;
                item[i++] = gbk[pos];
                continue;
            }
            if ( pos - 1 >= 0 && '\\' == gbk[pos - 1] ){
                item[i++] = gbk[pos];
                continue;
            }
            pos++;
            break;
        }
        
        String value = null;
        try {
            value = new String(item, 0, i, "gbk");
        } catch (UnsupportedEncodingException e) {
            reason = "字段值包含非法编码";
            return false;
        }
        if ( FieldType.str == field.type ){
            field.data = value;
            return true;
        }
        try {
            field.value = Long.valueOf(value);
        } catch (NumberFormatException e) {
            reason = "字段值" + value + "超出int64范围:" + Long.MIN_VALUE + "~" + Long.MAX_VALUE;
            return false;
        }

        return true;
    }

    //取得字段申明
    //[字段名1]=[字段值],
    private Field getField(){
        if ( null == gbk ) return null;
        moveSpace();//跳过空白符号
        if ( 0 > pos || pos >= gbk.length ) {
            reason = "字段 = null";
            return null;
        }
        
        Field field = new Field();
        //取字段名
        field.fieldName = getFieldName();
        if ( null == field.fieldName ) return null;

        //取运算符
        Byte op = getOperator();
        if ( null == op ) return null;
        if ( 0 != op ) {
            reason = "字段名与字段值之间缺少=连接";
            return null;
        }
        
        //取字段值
        if ( !getFieldValue(field) ) return null;
        
        return field;
    }

    //取得字段列表
    public Map<String,Field> getFields(){
        if ( pos >= gbk.length || !isSpace(gbk[pos] ) ) {
            reason = "缺少空格分割：" + readedSql();
            return null;
        }

        Map<String,Field> fields = new HashMap<String,Field>();
        Field field = null;
        while ( !isEnd() ){
            field = getField();
            if ( null == field ) {
                reason = "未找到字段申明：" + reason + ":" + readedSql();
                return null;
            }
            if ( fields.containsKey(field.fieldName) ){
                reason = "重复申明字段[" + field.fieldName + "]：" + readedSql();
                return null;
            }
            fields.put(field.fieldName, field);

            //准备解析下一个，或者列表结束
            int endPos = pos;
            moveSpace();
            if ( pos < gbk.length && ',' == gbk[pos] ) {
                pos++;
                continue;
            }
            pos = endPos;
            break;
        }
        if ( fields.isEmpty() ){
            reason = "字段列表 = null" + readedSql();
            return null;
        }
        
        return fields;
    }

    //取Long值
    private Long getLong(){
        if ( null == gbk ) return null;
        moveSpace();//跳过空白符号
        if ( 0 > pos || pos >= gbk.length ) {
            reason = "null";
            return null;
        }
        
        int i = 0;
        byte[] item = new byte[gbk.length - pos];
        for ( ; pos < gbk.length; pos++ ){
            if ( !isNum(gbk[pos]) ) break;
            item[i++] = gbk[pos];
        }
        String value = null;
        try {
            value = new String(item, 0, i, "gbk");
        } catch (UnsupportedEncodingException e) {
            reason = "包含非法编码";
            return null;
        }
        Long id = null;
        try {
            id = Long.valueOf(value);
        } catch (NumberFormatException e) {
            reason = value + "超出int64范围:" + Long.MIN_VALUE + "~" + Long.MAX_VALUE;
            return null;
        }
        
        return id;
    }
    
    //取id列表
    public List<Long> getIds(){
        if ( pos >= gbk.length || !isSpace(gbk[pos] ) ) {
            reason = "缺少空格分割：" + readedSql();
            return null;
        }
        
        Map<Long, Boolean> repeat = new HashMap<Long, Boolean>();
        List<Long> ids = new ArrayList<Long>();
        Long id = null;
        while ( !isEnd() ){
            id = getLong();
            if ( null == id ) {
                reason = "缺少id:" + reason + ":" + readedSql();
                return null;
            }
            if ( repeat.containsKey(id) ){
                reason = "重复申明id[" + id + "]：" + readedSql();
                return null;
            }
            ids.add(id);
            repeat.put(id, true);
            
            //准备解析下一个，或者列表结束
            int endPos = pos;
            moveSpace();
            if ( pos < gbk.length && ',' == gbk[pos] ) {
                pos++;
                continue;
            }
            pos = endPos;
            break;
        }
        if ( ids.isEmpty() ){
            reason = "id列表 = null" + readedSql();
            return null;
        }
        
        return ids;
    }
    

    //取得选取字段列表
    public List<String> getFieldNames(){
        if ( pos >= gbk.length || !isSpace(gbk[pos] ) ) {
            reason = "缺少空格分割：" + readedSql();
            return null;
        }
        
        List<String> selectFields = new ArrayList<String>();
        Map<String, Boolean> repeat = new HashMap<String, Boolean>();
        String name = null;
        while ( !isEnd() ){
            name = getFieldName();
            if ( null == name ) {
                reason = "缺少字段名:" + reason + ":" + readedSql();
                return null;
            }
            if ( repeat.containsKey(name) ){
                reason = "重复申明字段[" + name + "]：" + readedSql();
                return null;
            }
            selectFields.add(name);
            repeat.put(name, true);
            
            //准备解析下一个，或者列表结束
            int endPos = pos;
            moveSpace();
            if ( pos < gbk.length && ',' == gbk[pos] ) {
                pos++;
                continue;
            }
            pos = endPos;
            break;
        }
        if ( selectFields.isEmpty() ) {
            reason = "缺少申明[选取的字段]" + readedSql();
            return null;
        }
        
        return selectFields;
    }

    //取条件
    private Filter getFilter(){
        if ( null == gbk ) return null;
        moveSpace();//跳过空白符号
        if ( 0 > pos || pos >= gbk.length ) {
            reason = "条件 = null";
            return null;
        }
        
        Filter filter = new Filter();
        //取字段名
        filter.field.fieldName = getFieldName();
        if ( null == filter.field.fieldName ) return null;
        
        //取运算符
        Byte op = getOperator();
        if ( null == op ) return null;
        filter.cmpMode = op.byteValue();
        
        //取字段值
        if ( !getFieldValue(filter.field) ) return null;
        
        return filter;
    }

    //取条件列表
    public List<Filter> getFilters(){
        if ( pos >= gbk.length || !isSpace(gbk[pos] ) ) {
            reason = "缺少空格：" + readedSql();
            return null;
        }

        Map<String, Map<Byte, String>> repeat = new HashMap<String, Map<Byte, String>>();
        List<Filter> filters = new ArrayList<Filter>();
        Filter filter = null;
        while ( !isEnd() ){
            filter = getFilter();
            if ( null == filter ) {
                reason = "未找到条件申明：" + reason + ":" + readedSql();
                return null;
            }
            /*
             * 检查重复条件
             *      比如：a > 1 and a > 2
             *      但是未对a > 1 and a >= 2做重复检查，出现这类重复条件，服务端会按照【严条件】a > 2给查询结果
             * 检查矛盾条件
             *      比如：存在条件a = 1则a > x a < y a >= z a <= n 都是矛盾条件
             *      但是未对 a > 5 and a < 4做矛盾检查，出现这类矛盾条，件服务端查询结果=null
             */
            if ( repeat.containsKey(filter.field.fieldName) ){
                Map<Byte,String> conds = repeat.get(filter.field.fieldName);
                if ( conds.containsKey(filter.cmpMode) ){//同字段重复过滤方式
                    if ( CmpMode.unequals.value() != filter.cmpMode ){//不是!=，报告重复
                        reason = "重复申明条件[" + filter.field.fieldName + "]：" + readedSql();
                        return null;
                    }
                    if ( conds.get(filter.cmpMode).equals(filter.field.getValue()) ){
                        reason = "重复申明条件[" + filter.field.fieldName + "]：" + readedSql();
                        return null;
                    }
                }
                else if ( CmpMode.equals.value() == filter.cmpMode || conds.containsKey(CmpMode.equals.value()) ){//=矛盾
                    reason = "矛盾的条件申明[" + filter.field.fieldName + "]与[=]条件矛盾：" + readedSql();
                    return null;
                }
                conds.put(filter.cmpMode, filter.field.getValue());
            }
            else {
                Map<Byte,String> conds = new HashMap<Byte,String>();
                conds.put(filter.cmpMode, filter.field.getValue());
                repeat.put(filter.field.fieldName, conds);
            }
            filters.add(filter);
            
            //准备解析下一个，或者列表结束
            int endPos = pos;
            String key = getKeyWord();
            if ( null == key || !key.equals("and") ) {
                pos = endPos;
                break;
            }
            if ( !isSpace(gbk[pos]) ){//and后空格
                pos = endPos;
                break;
            }
        }
        if ( filters.isEmpty() ){
            reason = "条件列表 = null" + readedSql();
            return null;
        }
        
        return filters;
    }

    public Point getPoint(){
        Point point = new Point();
        point.data = this.getFields();
        
        return point;
    }

    //取关键字
    public String getKeyWord(){
        return getKeyWord(false);
    }
    
    //取关键字,isHead=true关键字在头部
    private String getKeyWord(boolean isHead){
        if ( pos >= gbk.length ) {
            reason = "关键字 = null" + readedSql();
            return null;
        }
        if ( !isHead ){
            if ( !isSpace(gbk[pos] ) ) {
                reason = "缺少空格分割：" + readedSql();
                return null;
            }
        }
        String value = getWord();
        if ( null == value ){
            reason = "关键字 = null" + readedSql();
            return null;
        }
        value = value.toLowerCase();

        if ( value.equals("add") ) return value;
        if ( value.equals("edit") ) return value;
        if ( value.equals("del") ) return value;
        if ( value.equals("link") ) return value;
        if ( value.equals("unlink") ) return value;
        if ( value.equals("count") ) return value;
        if ( value.equals("get") ) return value;
        
        if ( value.equals("adds") ) return value;
        if ( value.equals("links") ) return value;
        if ( value.equals("edits") ) return value;
        
        if ( value.equals("to") ) return value;
        if ( value.equals("by") ) return value;
        if ( value.equals("out") ) return value;
        if ( value.equals("in") ) return value;
        if ( value.equals("and") ) return value;
        if ( value.equals("id") ) return value;
        if ( value.equals("all") ) return value;
        if ( value.equals("from") ) return value;
        if ( value.equals("select") ) return value;

        reason = "非法关键字[" + value + "]" + readedSql();
        
        return null;
    }
    
    //取得get模式，取所有字段，只取id，取指定字段
    public String getSelectMode(){
        int curPos = pos;
        String keyWord = getKeyWord();
        if ( null == keyWord ) {
            pos = curPos;
            return "select";
        }
        
        if ( keyWord.equals("all") || keyWord.equals("id") ) return keyWord;
        reason = "缺少申明[选取的字段]" + readedSql();
        return null;
    }
    
    //检查是否开始移动参数
    public boolean isMove(){
        int curPos = pos;
        String keyWord = getKeyWord();
        if ( null != keyWord && keyWord.equals("from")) return true;
        pos = curPos;
        
        return false;
    }
    
    //检查移动路线是out
    public boolean isMoveOut(){
        int curPos = pos;
        String keyWord = getKeyWord();
        if ( null != keyWord && keyWord.equals("out")) return true;
        pos = curPos;
        
        return false;
    }

    //检查移动路线是in
    public boolean isMoveIn(){
        int curPos = pos;
        String keyWord = getKeyWord();
        if ( null != keyWord && keyWord.equals("in")) return true;
        pos = curPos;
        
        return false;
    }

    //检查移动路线，out/in之后是条件，还是all
    public boolean isMoveAll(){
        int curPos = pos;
        String keyWord = getKeyWord();
        if ( null != keyWord && keyWord.equals("all")) return true;
        pos = curPos;
        
        return false;
    }
    
    //检查是否是条件部分开始
    public boolean isSelect(){
        int curPos = pos;
        String keyWord = getKeyWord();
        if ( null != keyWord && keyWord.equals("select")) return true;
        pos = curPos;
        
        return false;
    }
    
    //剩余sql
    public String unreadedSql(){
        String value = null;
        try {
            value = new String(gbk, pos, gbk.length - pos,  "gbk");
        } catch (UnsupportedEncodingException e) {
            return "{}";
        }
        
        return "{" + value + "}";
    }
}
