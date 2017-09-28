package com.huoyu;

import java.util.ArrayList;
import java.util.List;

import com.huoyu.client.CallResult;
import com.huoyu.client.NodeAddress;
import com.huoyu.client.ResultCode;
import com.huoyu.gmap.svr.SqlParser;
import com.huoyu.gmap.svr.gmap.GCluster;
import com.huoyu.gmap.svr.gmap.constant.Action;
import com.huoyu.gmap.svr.gmap.datastruct.Filter;
import com.huoyu.gmap.svr.gmap.datastruct.Line;
import com.huoyu.gmap.svr.gmap.datastruct.Point;
import com.huoyu.gmap.svr.gmap.datastruct.SelectLine;
import com.huoyu.gmap.svr.gsrc.GSrc;

//访问图数据库的SDK对象
public class GClient {
    private int timeOut;//所有调用默认超时时间
    
    public GClient()
    {
        timeOut = 30000;//所有调用默认超时时间30秒
    }
    
    //初始化服务信息，配置服务ip和端口，不用关心图数据库节点和ip端口,所有接口默认超时时间
    public CallResult<Void> Init(String ip, int port)
    {
        return Init(ip, port, 30000);
    }
    
    //初始化服务信息，配置服务ip和端口，不用关心图数据库节点和ip端口,所有接口默认超时时间
    public CallResult<Void> Init(String ip, int port, int timeOut)
    {
        this.timeOut = timeOut;
        config = new GSrc();
        ResultCode rc = config.setService(1, ip, port);
        if ( ResultCode.SUCCESS != rc ) return new CallResult<Void>(rc, "集群配置服务连接参数错误：" + rc.descript());
   
        //获取所有节点连接
        CallResult<List<NodeAddress>> cluster = config.GetCluster(1000);
        if ( ResultCode.SUCCESS.value() != cluster.getCode() ) return new CallResult<Void>(cluster.getCode(), "取不到集群信息：" + cluster.getReason());
        //申请唯一client id
        CallResult<Long> clientId = config.NewClientId(1000);
        if ( ResultCode.SUCCESS.value() != clientId.getCode() ) return new CallResult<Void>(clientId.getCode(), "申请client id失败：" + clientId.getReason());
        //连接所有图数据库节点
        database = new GCluster(clientId.getData(), config);
        return database.init(cluster.getData());
    }
    
    /*
     *结构化操作语句 
     ********************************************************************************************************************      
     * 字段支持的类型:
     *      根据字段值自动匹配，int64,string(小数以字符串存储)
     *      
     * 字段申明:
     *      [字段名]=[字段值]
     *      
     * 条件申明:
     *      [字段名]比较运算符[字段值]
     *      
     *      检查重复条件会报错
     *          比如：a > 1 and a > 2
     *          但是未对a > 1 and a >= 2做重复检查，出现这类重复条件，服务端会按照【严条件】a > 2给查询结果
     *      检查矛盾条件会报错
     *          比如：存在条件a = 1则a > x a < y a >= z a <= n 都是矛盾条件
     *          但是未对 a > 5 and a < 4做矛盾检查，出现这类矛盾条，件服务端查询结果=null
     *      
     * 支持的比较运算符:
     *      > >= = <= < !=
     * 
     * 长度限制:
     *      参考com.huoyu.gmap.svr.grid.constant.GridLimit
     * 
     ********************************************************************************************************************      
     * 基本操作
     * 创建顶点:return CallResult<Long> Long = 顶点id
     *      sql:add 字段1,字段2,字段n;
     *      
     * 编辑顶点:return CallResult<Void>
     *      sql:edit 顶点id 字段1,字段2,字段n;
     *      
     * 删除顶点:return CallResult<Void>
     *      sql:del 顶点id;
     *      
     * 创建连接:return CallResult<Void>
     *      sql:link 起点id to 终点id by 字段1,字段2,字段n;
     *      
     * 断开顶点:return CallResult<Void>
     *      参考图移动操作【unlink】语法
     *      
     * 
     ********************************************************************************************************************      
     * 图移动操作 
     * 移动参数:
     *      from 起点id1,起点id2,起点idn 空格
     *      out all 条件1 and 条件2 and 条件n 空格
     *      out all 
     *      ... 空格
     *      in 条件1 and 条件2 and 条件n 空格
     *      说明
     *          from后面是起点列表，表示从哪些顶点开始移动
     *          out in后面是1条路线的属性，out(出路，从起点到终点)，in(回路,从终点回起点)
     *      
     * 统计可到达的顶点数量：return CallResult<Long> Long = 数量
     *      sql:count 移动参数 select 条件1 and 条件2;
     *      
     * 查找可到达的顶点:return CallResult<List<Point>>
     *      sql:get [要获取的数据字段名],[要获取的数据字段名] 移动参数 select 条件1 and 条件2; 取得到达顶点指定的字段
     *      sql:get all 移动参数 select 条件1 and 条件2; 取得到达的顶点的所有字段
     *      sql:get id 移动参数 select 条件1 and 条件2; 仅仅取得到达顶点的id
     *      
     * 删除可到达的顶点(不支持多级移动):return CallResult<Void>
     *      sql:del 移动参数 select 条件1 and 条件2;
     *      
     * 断开可到达的顶点语法(不支持多级移动):return CallResult<Void>
     *      sql:unlink 移动参数
     *           
     ********************************************************************************************************************      
     * 批量操作(暂不支持)：
     * 批量创建顶点:return CallResult<List<Long>> Long = 顶点id
     *      adds 
     *      字段1,字段2,字段n 空格
     *      &字段1,字段2,字段n 空格
     *      &字段1,字段2,字段n;
     * 批量连接顶点:return CallResult<Void>
     *      links 字段1,字段2,字段n
     *      起点1id,终点1id 起点2id,终点2id 起点nid,终点nid;
     * 批量编辑顶点:return CallResult<Void>
     *      edits 
     *      &id1 字段1,字段2,字段n
     *      &id2 字段1,字段2,字段n
     *      &idn 字段1,字段2,字段n;
     * 批量删除：return CallResult<Void>
     *      使用删除可到达的顶点，不设置移动路线即可
     */
    @SuppressWarnings("rawtypes")
    public CallResult Sql(String sql){
        return Sql(sql, timeOut);
    }

    //带超时设置的，执行sql，millSecond <= 0不超时
    @SuppressWarnings("rawtypes")
    public CallResult Sql(String sql, int millSecond){
        SqlParser sqlParser = new SqlParser(sql);
        String act = sqlParser.getAction();
        if ( null == act ) return new CallResult(ResultCode.REFUSE, sqlParser.reason);
        
        /********************************************************************************************************************      
        * 基本操作
        */
        if ( act.equalsIgnoreCase("add") ) return SqlAdd(sqlParser, millSecond);
        if ( act.equals("edit") ) return SqlEdit(sqlParser, millSecond);
        if ( act.equals("del") ) return SqlDel(sqlParser, millSecond);
        if ( act.equals("link") ) return SqlLink(sqlParser, millSecond);
        
        /********************************************************************************************************************      
        * 移动操作
        */
        if ( act.equals("get") ) return SqlGet(sqlParser, millSecond);
        if ( act.equals("count") ) return SqlCount(sqlParser, millSecond);
        if ( act.equals("unlink") ) return SqlUnlink(sqlParser, millSecond);

        return new CallResult<Void>();
    }

    //创建顶点
    public CallResult<Void> AddPoint(Point point)
    {
        return AddPoint(point, timeOut);
    }
    
    //创建顶点，millSecond <= 0不超时
    public CallResult<Void> AddPoint(Point point, int millSecond)
    {
        return database.AddPoint(point, millSecond);
    }

    //编辑顶点
    public CallResult<Void> SetPoint(Point point)
    {
        return SetPoint(point, timeOut);
    }
    
    //编辑顶点，millSecond <= 0不超时
    public CallResult<Void> SetPoint(Point point, int millSecond)
    {
        return database.SetPoint(point, millSecond);
    }
    
    //取顶点
    public CallResult<Point> GetPoint(long pointId)
    {
        return GetPoint(pointId, timeOut);
    }

    //取顶点，millSecond <= 0不超时
    public CallResult<Point> GetPoint(long pointId, int millSecond)
    {
        return database.GetPoint(pointId, millSecond);
    }

    //连接顶点
    public CallResult<Void> Link(Line line)
    {
        return Link(line, timeOut);
    }
    
    //连接顶点，millSecond <= 0不超时
    public CallResult<Void> Link(Line line, int millSecond)
    {
        return database.Link(line, millSecond);
    }
    
    //移动可到达的顶点，进行get count del unlink等操作
    public CallResult<List<Point>> Move(List<Long> startPoints,
        List<SelectLine> selectLines, List<Filter> filters, 
        boolean selectAll, //取得所有字段 = true时selectFields无效
        List<String> selectFields, Action act)
    {
        return Move(startPoints, selectLines, filters, selectAll, selectFields, act, timeOut);
    }

    //移动可到达的顶点，进行get count del unlink等操作，millSecond <= 0不超时
    public CallResult<List<Point>> Move(List<Long> startPoints,
        List<SelectLine> selectLines, List<Filter> filters, 
        boolean selectAll, //取得所有字段 = true时selectFields无效
        List<String> selectFields, Action act, int millSecond)
    {
        CallResult<List<Point>> data = database.Move(startPoints, selectLines, filters, selectAll, selectFields, act, millSecond);
        if ( ResultCode.SUCCESS.value() != data.getCode() ) database.CloseConnect();
        return data;
    }

    //断开连接
    public CallResult<Void> Unlink(List<Long> startPoints, List<SelectLine> selectLines){
        return Unlink(startPoints, selectLines, timeOut);
    }
    
    //断开连接，millSecond <= 0不超时
    public CallResult<Void> Unlink(List<Long> startPoints, List<SelectLine> selectLines, int millSecond){
        return new CallResult<Void>(Move(startPoints, selectLines, null, false, null, Action.unlink, millSecond));
    }
    
    //删除顶点
    public CallResult<Void> DelPoint(long pointId)
    {
        return DelPoint(pointId, timeOut);
    }
    
    //删除顶点，millSecond <= 0不超时
    public CallResult<Void> DelPoint(long pointId, int millSecond)
    {
        return database.DelPoint(pointId, millSecond);
    }
    
    ////////////////////////////////////////////////////////////////////////////////
    //批量操作接口
    //批量设置顶点
    public CallResult<Void> AddPoints( List<Point> points ){
        int timeOut = (points.size() / 1000 + 1) * 1000;//1000个顶点/秒
        return AddPoints(points, timeOut);
    }
    
    //批量设置顶点，millSecond <= 0不超时
    public CallResult<Void> AddPoints( List<Point> points, int millSecond ){
        return database.AddPoints(points, millSecond);
    }
    
    //批量删除顶点(未测试，测试后开放)
    @SuppressWarnings("unused")
    private CallResult<Void> DetPoints( List<Long> ids )
    {
        return DetPoints(ids, timeOut);
    }
    
    //批量删除顶点(未测试，测试后开放)，millSecond <= 0不超时
    private CallResult<Void> DetPoints( List<Long> ids, int millSecond )
    {
        return database.DetPoints(ids, millSecond);
    }
    
    ////////////////////////////////////////////////////////////////////////////////
    //工具方法
    //使用sql添加1个顶点：add 字段1,字段2,字段n;
    private CallResult<Long> SqlAdd(SqlParser sqlParser, int millSecond){
        //解析sql
        Point point = new Point();
        point.data = sqlParser.getFields();
        if ( null == point.data ) return new CallResult<Long>(ResultCode.REFUSE, "sql错误：" + sqlParser.reason);
        //检查sql完成
        if ( !sqlParser.isEnd() ) return new CallResult<Long>(ResultCode.REFUSE, "sql错误，缺少结束符or存在非法语句：" + sqlParser.unreadedSql());
        //调用接口
        CallResult<Void> ret = this.AddPoint(point, millSecond);
        if ( ResultCode.SUCCESS.value() != ret.getCode() ) return new CallResult<Long>(ret);
        return new CallResult<Long>(point.id);
    }
    
    //使用sql修改1个顶点：edit 顶点id 字段1,字段2,字段n;
    private CallResult<Void> SqlEdit(SqlParser sqlParser, int millSecond){
        //解析sql
        Point point = new Point();
        List<Long> ids = sqlParser.getIds();
        if ( null == ids ) return new CallResult<Void>(ResultCode.REFUSE, "sql错误：" + sqlParser.reason);
        if ( 1 < ids.size() ) return new CallResult<Void>(ResultCode.REFUSE, "sql错误：id个数超过1" );
        point.data = sqlParser.getFields();
        if ( null == point.data ) return new CallResult<Void>(ResultCode.REFUSE, "sql错误：" + sqlParser.reason);
        point.id = ids.get(0);
        //检查sql完成
        if ( !sqlParser.isEnd() ) return new CallResult<Void>(ResultCode.REFUSE, "sql错误，缺少结束符or存在非法语句：" + sqlParser.unreadedSql());
        //调用接口
        return new CallResult<Void>(this.SetPoint(point, millSecond));
    }

    //使用sql删除顶点
    private CallResult<Void> SqlDel(SqlParser sqlParser, int millSecond ){
        if ( !sqlParser.isMove() ){//删除1个顶点语法：del 顶点id;
            //解析sql
            List<Long> ids = sqlParser.getIds();
            if ( null == ids ) return new CallResult<Void>(ResultCode.REFUSE, "sql错误：" + sqlParser.reason);
            if ( 1 < ids.size() ) return new CallResult<Void>(ResultCode.REFUSE, "sql错误：id个数超过1" );
            
            //检查sql完成
            if ( !sqlParser.isEnd() ) return new CallResult<Void>(ResultCode.REFUSE, "sql错误，缺少结束符or存在非法语句：" + sqlParser.unreadedSql());
            //调用接口
            return new CallResult<Void>(this.DelPoint(ids.get(0), millSecond));
        }
        
        //删除可到达的顶点语法(不支持多级移动)：del 移动参数 select 条件1 and 条件2;
        //解析sql
        //取起点列表
        List<Long> startPoints = sqlParser.getIds();
        if ( null == startPoints ) return new CallResult<Void>(ResultCode.REFUSE, "sql错误：缺少起点列表：" + sqlParser.reason);
        //取移动路线
        List<SelectLine> selectLines = new ArrayList<SelectLine>();
        SelectLine line;
        while ( true ){
            line = new SelectLine();
            line.moveAllLine = true;
            if ( sqlParser.isMoveOut() ) {
                line.moveOut = true;
                if ( !sqlParser.isMoveAll() ){
                    line.moveAllLine = false;
                    line.selectField = sqlParser.getFilters();
                    if ( null == line.selectField ) return new CallResult<Void>(ResultCode.REFUSE, "sql错误：缺少路线申明：" + sqlParser.reason);
                }
                selectLines.add(line);
                continue;
            }
            else if ( sqlParser.isMoveIn() ) {
                line.moveOut = false;
                if ( !sqlParser.isMoveAll() ){
                    line.moveAllLine = false;
                    line.selectField = sqlParser.getFilters();
                    if ( null == line.selectField ) return new CallResult<Void>(ResultCode.REFUSE, "sql错误：缺少路线申明：" + sqlParser.reason);
                }
                selectLines.add(line);
                continue;
            }
            break;
        }
        if ( 1 < selectLines.size() ) return new CallResult<Void>(ResultCode.REFUSE, "sql错误：del不支持多级路线");
        //取过滤条件
        List<Filter> filters = null;
        if ( sqlParser.isSelect() ){
            filters = sqlParser.getFilters();
            if ( null == filters ) return new CallResult<Void>(ResultCode.REFUSE, "sql过滤条件错误：" + sqlParser.reason);
        }
        //检查sql完成
        if ( !sqlParser.isEnd() ) return new CallResult<Void>(ResultCode.REFUSE, "sql错误，缺少结束符or存在非法语句：" + sqlParser.unreadedSql());
        //调用接口
        CallResult<List<Point>> points = this.Move(startPoints, selectLines, filters, false, null, Action.del, millSecond);
        if ( ResultCode.SUCCESS.value() != points.getCode() ) return new CallResult<Void>(points);
        
        return new CallResult<Void>();
    }

    //使用sql创建1条连接：link 起点id to 终点id by 字段1,字段2,字段n;
    private CallResult<Void> SqlLink(SqlParser sqlParser, int millSecond){
        //解析sql
        Line line = new Line();
        List<Long> ids = sqlParser.getIds();
        if ( null == ids ) return new CallResult<Void>(ResultCode.REFUSE, "sql错误：" + sqlParser.reason);
        if ( 1 < ids.size() ) return new CallResult<Void>(ResultCode.REFUSE, "sql错误：起点id个数超过1" );
        line.startId = ids.get(0);
        String keyWord = sqlParser.getKeyWord();
        if ( null == keyWord || !keyWord.equalsIgnoreCase("to") ) return new CallResult<Void>(ResultCode.REFUSE, "sql错误 缺少关键词to：" + sqlParser.reason );
        ids = sqlParser.getIds();
        if ( null == ids ) return new CallResult<Void>(ResultCode.REFUSE, "sql错误：" + sqlParser.reason);
        if ( 1 < ids.size() ) return new CallResult<Void>(ResultCode.REFUSE, "sql错误：终点id个数超过1" );
        line.endId = ids.get(0);
        keyWord = sqlParser.getKeyWord();
        if ( null == keyWord || !keyWord.equalsIgnoreCase("by") ) return new CallResult<Void>(ResultCode.REFUSE, "sql错误 缺少关键词by：" + sqlParser.reason );
        line.data = sqlParser.getFields();
        if ( null == line.data ) return new CallResult<Void>(ResultCode.REFUSE, "sql错误：" + sqlParser.reason);
        //检查sql完成
        if ( !sqlParser.isEnd() ) return new CallResult<Void>(ResultCode.REFUSE, "sql错误，缺少结束符or存在非法语句：" + sqlParser.unreadedSql());
        //调用接口
        return this.Link(line, millSecond);
    }
    
    //使用sql断开可到达的顶点语法，语法(不支持多级移动)：unlink 移动参数
    private CallResult<Void> SqlUnlink( SqlParser sqlParser, int millSecond ){
        //解析sql
        //语法检查，需要关键字from
        if ( !sqlParser.isMove() ) return new CallResult<Void>(ResultCode.REFUSE, "sql错误 缺少关键词from：" + sqlParser.reason );

        //取起点列表
        List<Long> startPoints = sqlParser.getIds();
        if ( null == startPoints ) return new CallResult<Void>(ResultCode.REFUSE, "sql错误：缺少起点列表：" + sqlParser.reason);

        //取移动路线(不支持多级移动)
        List<SelectLine> selectLines = new ArrayList<SelectLine>();
        SelectLine line;
        while ( true ){
            line = new SelectLine();
            line.moveAllLine = true;
            if ( sqlParser.isMoveOut() ) {
                line.moveOut = true;
                if ( !sqlParser.isMoveAll() ){
                    line.moveAllLine = false;
                    line.selectField = sqlParser.getFilters();
                    if ( null == line.selectField ) return new CallResult<Void>(ResultCode.REFUSE, "sql错误：缺少路线申明：" + sqlParser.reason);
                }
                selectLines.add(line);
                continue;
            }
            else if ( sqlParser.isMoveIn() ) {
                line.moveOut = false;
                if ( !sqlParser.isMoveAll() ){
                    line.moveAllLine = false;
                    line.selectField = sqlParser.getFilters();
                    if ( null == line.selectField ) return new CallResult<Void>(ResultCode.REFUSE, "sql错误：缺少路线申明：" + sqlParser.reason);
                }
                selectLines.add(line);
                continue;
            }
            break;
        }
        if ( 1 < selectLines.size() ) return new CallResult<Void>(ResultCode.REFUSE, "sql错误：unlink不支持多级路线");
        if ( 0 == selectLines.size() ) return new CallResult<Void>(ResultCode.REFUSE, "sql错误：unlink未指定被删除的路线");
        //检查sql完成
        if ( !sqlParser.isEnd() ) return new CallResult<Void>(ResultCode.REFUSE, "sql错误，缺少结束符or存在非法语句：" + sqlParser.unreadedSql());
        //调用接口
        return this.Unlink(startPoints, selectLines, millSecond);
    }

    /*
     * 使用sql获取移动可到达的顶点
     * 查找可到达的顶点:
     *      sql:get [要获取的数据字段名],[要获取的数据字段名] 移动参数 select 条件1 and 条件2; 取得到达顶点指定的字段
     *      sql:get all 移动参数 select 条件1 and 条件2; 取得到达的顶点的所有字段
     *      sql:get id 移动参数 select 条件1 and 条件2; 仅仅取得到达顶点的id
     */
    private CallResult<List<Point>> SqlGet(SqlParser sqlParser, int millSecond){
        //解析sql
        //取得选取的字段列表
        boolean selectAll = false;
        String selectMode = sqlParser.getSelectMode();
        if ( null == selectMode ) return new CallResult<List<Point>>(ResultCode.REFUSE, "sql错误：" + sqlParser.reason);
        List<String> selectField = null;
        if ( selectMode.equalsIgnoreCase("all") ) selectAll = true;
        else if ( selectMode.equalsIgnoreCase("select") ){
            selectField = sqlParser.getFieldNames();
            if ( null == selectField ) return new CallResult<List<Point>>(ResultCode.REFUSE, "sql错误：" + sqlParser.reason);
        }
        
        //语法检查，需要关键字from
        if ( !sqlParser.isMove() ) return new CallResult<List<Point>>(ResultCode.REFUSE, "sql错误 缺少关键词from：" + sqlParser.reason );

        //取起点列表
        List<Long> startPoints = sqlParser.getIds();
        if ( null == startPoints ) return new CallResult<List<Point>>(ResultCode.REFUSE, "sql错误：缺少起点列表：" + sqlParser.reason);

        //取移动路线
        List<SelectLine> selectLines = new ArrayList<SelectLine>();
        SelectLine line;
        while ( true ){
            line = new SelectLine();
            line.moveAllLine = true;
            if ( sqlParser.isMoveOut() ) {
                line.moveOut = true;
                if ( !sqlParser.isMoveAll() ){
                    line.moveAllLine = false;
                    line.selectField = sqlParser.getFilters();
                    if ( null == line.selectField ) return new CallResult<List<Point>>(ResultCode.REFUSE, "sql错误：缺少路线申明：" + sqlParser.reason);
                }
                selectLines.add(line);
                continue;
            }
            else if ( sqlParser.isMoveIn() ) {
                line.moveOut = false;
                if ( !sqlParser.isMoveAll() ){
                    line.moveAllLine = false;
                    line.selectField = sqlParser.getFilters();
                    if ( null == line.selectField ) return new CallResult<List<Point>>(ResultCode.REFUSE, "sql错误：缺少路线申明：" + sqlParser.reason);
                }
                selectLines.add(line);
                continue;
            }
            break;
        }

        //过滤条件
        List<Filter> filters = null;
        if ( sqlParser.isSelect() ){
            filters = sqlParser.getFilters();
            if ( null == filters ) return new CallResult<List<Point>>(ResultCode.REFUSE, "sql过滤条件错误：" + sqlParser.reason);
        }
        //检查sql完成
        if ( !sqlParser.isEnd() ) return new CallResult<List<Point>>(ResultCode.REFUSE, "sql错误，缺少结束符or存在非法语句：" + sqlParser.unreadedSql());
        //调用接口
        CallResult<List<Point>> ret = this.Move(startPoints, selectLines, filters, selectAll, selectField, Action.get, millSecond);
        if ( ResultCode.SUCCESS.value() == ret.getCode() && 0 == ret.getData().size() ) return new CallResult<List<Point>>(ResultCode.NO_DATA, ResultCode.NO_DATA.descript());

        return ret;
    }

    //使用sql统计可到达的顶点数量：count 移动参数 select 条件1 and 条件2;
    private CallResult<Long> SqlCount(SqlParser sqlParser, int millSecond){
        //解析sql
        //语法检查，需要关键字from
        if ( !sqlParser.isMove() ) return new CallResult<Long>(ResultCode.REFUSE, "sql错误 缺少关键词from：" + sqlParser.reason );

        //取起点列表
        List<Long> startPoints = sqlParser.getIds();
        if ( null == startPoints ) return new CallResult<Long>(ResultCode.REFUSE, "sql错误：缺少起点列表：" + sqlParser.reason);

        //取移动路线
        List<SelectLine> selectLines = new ArrayList<SelectLine>();
        SelectLine line;
        while ( true ){
            line = new SelectLine();
            line.moveAllLine = true;
            if ( sqlParser.isMoveOut() ) {
                line.moveOut = true;
                if ( !sqlParser.isMoveAll() ){
                    line.moveAllLine = false;
                    line.selectField = sqlParser.getFilters();
                    if ( null == line.selectField ) return new CallResult<Long>(ResultCode.REFUSE, "sql错误：缺少路线申明：" + sqlParser.reason);
                }
                selectLines.add(line);
                continue;
            }
            else if ( sqlParser.isMoveIn() ) {
                line.moveOut = false;
                if ( !sqlParser.isMoveAll() ){
                    line.moveAllLine = false;
                    line.selectField = sqlParser.getFilters();
                    if ( null == line.selectField ) return new CallResult<Long>(ResultCode.REFUSE, "sql错误：缺少路线申明：" + sqlParser.reason);
                }
                selectLines.add(line);
                continue;
            }
            break;
        }

        //过滤条件
        List<Filter> filters = null;
        if ( sqlParser.isSelect() ){
            filters = sqlParser.getFilters();
            if ( null == filters ) return new CallResult<Long>(ResultCode.REFUSE, "sql过滤条件错误：" + sqlParser.reason);
        }
        if ( 0 == selectLines.size() ) return new CallResult<Long>(ResultCode.REFUSE, "sql错误：count必须移动至少1次路线 or 设置过滤条件");
        //检查sql完成
        if ( !sqlParser.isEnd() ) return new CallResult<Long>(ResultCode.REFUSE, "sql错误，缺少结束符or存在非法语句：" + sqlParser.unreadedSql());
        //调用接口
        CallResult<List<Point>> points = this.Move(startPoints, selectLines, filters, false, null, Action.count, millSecond);
        if ( ResultCode.SUCCESS.value() != points.getCode() ) return new CallResult<Long>(points);
        
        return new CallResult<Long>((long)(points.getData().size()));
    }

    private GSrc   config;//集群配置操作client
    private GCluster     database;//数据集群操作client
}
