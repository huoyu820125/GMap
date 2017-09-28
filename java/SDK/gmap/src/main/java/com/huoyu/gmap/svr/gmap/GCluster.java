package com.huoyu.gmap.svr.gmap;

import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.huoyu.client.CallResult;
import com.huoyu.client.Cluster;
import com.huoyu.client.MsgDefaultParam;
import com.huoyu.client.NetConnect;
import com.huoyu.client.ResultCode;
import com.huoyu.gmap.svr.gmap.constant.Action;
import com.huoyu.gmap.svr.gmap.constant.GridLimit;
import com.huoyu.gmap.svr.gmap.datastruct.Filter;
import com.huoyu.gmap.svr.gmap.datastruct.Line;
import com.huoyu.gmap.svr.gmap.datastruct.MoveMidData;
import com.huoyu.gmap.svr.gmap.datastruct.Point;
import com.huoyu.gmap.svr.gmap.datastruct.SelectLine;
import com.huoyu.gmap.svr.gmap.protocl.MsgDelPoint;
import com.huoyu.gmap.svr.gmap.protocl.MsgGetPoint;
import com.huoyu.gmap.svr.gmap.protocl.MsgMove;
import com.huoyu.gmap.svr.gmap.protocl.MsgMoveMids;
import com.huoyu.gmap.svr.gmap.protocl.MsgPoints;
import com.huoyu.gmap.svr.gmap.protocl.MsgSetMoveOpt;
import com.huoyu.gmap.svr.gmap.protocl.MsgSetPoint;
import com.huoyu.gmap.svr.gmap.protocl.MsgSetPoints;
import com.huoyu.gmap.svr.gmap.protocl.ProtoclNo;
import com.huoyu.gmap.svr.gmap.protocl.tool.MsgAddLine;
import com.huoyu.gmap.svr.gmap.protocl.tool.MsgAddLineForPoint;
import com.huoyu.gmap.svr.gmap.protocl.tool.MsgDelLine;
import com.huoyu.gmap.svr.gmap.protocl.tool.MsgDelLineForPoint;
import com.huoyu.gmap.svr.gmap.protocl.tool.MsgGetLine;
import com.huoyu.gmap.svr.gmap.protocl.tool.MsgNewId;
import com.huoyu.gmap.svr.gsrc.GSrc;
import com.huoyu.gmap.svr.gsrc.constant.ServiceType;

//访问图数据库数据集群的SDK对象
public class GCluster extends Cluster {
    private GSrc cfgSvr;
    private long    clientId;//客户端id
    private long    moveIndex;//move操作序号
    
//    long m_sendTime = 0;
//    long m_recvTime = 0;
//    long m_parseTime = 0;

    public GCluster(Long clientId, GSrc svr) {
        cfgSvr = svr;
        moveIndex = 0;
        this.clientId = clientId;
    }
    
    //统一错误
    @SuppressWarnings("rawtypes")
    private CallResult Error(ResultCode ret, String reason, NetConnect dataSvr, int millSecond) {
        if (ResultCode.TIME_OUT == ret) {
            return new CallResult(ret.value(), "图数据库节点(" + dataSvr.getNodeId() + ")：" + dataSvr.getServiceIP() + ":" + dataSvr.getServicePort() + "返回超时" + millSecond + "毫秒");
        }
        if (ResultCode.NO_SERVER == ret) {
            return new CallResult(ret.value(), "图数据库节点(" + dataSvr.getNodeId() + ")：" + dataSvr.getServiceIP() + ":" + dataSvr.getServicePort() + "无服务");
        }
        if ( null != reason && !reason.equals("") ) return new CallResult(ret, ret.descript() + ":" + reason);
        return new CallResult(ret);
    }

    /*
     * 字符串时间转换成，距离1970年1月1日0点0分0秒的秒数
     * 时间格式：yyyy-MM-dd HH:mm:ss
     */
    public static long millSecond()
    {
        Date curDate = new Date();
        return curDate.getTime();//距离1970年1月1日0点0分0秒的毫秒数
    }

    //创建一个全局唯一的新id
    private CallResult<Long> NewId(int millSecond) {
        CallResult<List<Long>> ids = NewIds(1, millSecond);
        if ( ResultCode.SUCCESS.value() != ids.getCode() ) return new CallResult<Long>(ids);
        
        return new CallResult<Long>(ids.getData().get(0));
    }

    //创建一个顶点
    public CallResult<Void> AddPoint(Point point, int millSecond) {
        CallResult<Long> id = NewId(millSecond);
        if (ResultCode.SUCCESS.value() != id.getCode())
            return new CallResult<Void>(id.getCode(), "无法生成数据id：" + id.getReason());
        point.id = id.getData();
        return SetPoint(point, millSecond);
    }

    //修改一个顶点
    @SuppressWarnings("unchecked")
    public CallResult<Void> SetPoint(Point point, int millSecond) {
        if (null == point || point.id <= 0)
            return new CallResult<Void>(ResultCode.PARAM_ERROR, "顶点id必须 > 0");

        //从集群中选取一个服务节点
        NetConnect dataSvr = findNode(point.id);
        //执行请求
        MsgSetPoint msg = new MsgSetPoint();
        msg.point = point;
        ResultCode ret = dataSvr.query(msg, millSecond);
        if (ResultCode.SUCCESS != ret) return Error(ret, msg.reason, dataSvr, millSecond);

        //返回数据
        return new CallResult<Void>();
    }

    //取得顶点
    @SuppressWarnings("unchecked")
    public CallResult<Point> GetPoint(long pointId, int millSecond) {
        if (pointId <= 0)
            return new CallResult<Point>(ResultCode.PARAM_ERROR, "顶点id必须 > 0");

        //从集群中选取一个服务节点
        NetConnect dataSvr = findNode(pointId);
        //执行请求
        MsgGetPoint msg = new MsgGetPoint();
        msg.pointId = pointId;
        ResultCode ret = dataSvr.query(msg, millSecond);
        if (ResultCode.SUCCESS != ret) return Error(ret, msg.reason, dataSvr, millSecond);

        //返回数据
        return new CallResult<Point>(msg.point);
    }

    //创建一条边
    @SuppressWarnings("unchecked")
    private CallResult<Void> AddLine(Line line, int millSecond) {
        if (null == line || line.id <=0 || line.startId <= 0 || line.endId <= 0)
            return new CallResult<Void>(ResultCode.PARAM_ERROR, "数据id必须 > 0");

        //从集群中选取一个服务节点
        NetConnect dataSvr = findNode(line.id);
        //执行请求
        MsgAddLine msg = new MsgAddLine();
        msg.line = line;
        ResultCode ret = dataSvr.query(msg, millSecond);
        if (ResultCode.SUCCESS != ret) return Error(ret, msg.reason, dataSvr, millSecond);

        //返回数据
        return new CallResult<Void>();
    }

    //给顶点添加一条边
    @SuppressWarnings("unchecked")
    private CallResult<Void> AddLineForPoint(long pointId, long lineId, boolean isOut, int millSecond) {
        if (pointId <= 0 || lineId <= 0)
            return new CallResult<Void>(ResultCode.PARAM_ERROR, "顶点和边id必须 > 0");

        //从集群中选取一个服务节点
        NetConnect dataSvr = findNode(pointId);
        //执行请求
        MsgAddLineForPoint msg = new MsgAddLineForPoint();
        msg.pointId = pointId;
        msg.lineId = lineId;
        msg.isOut = isOut;

        ResultCode ret = dataSvr.query(msg, millSecond);
        if (ResultCode.SUCCESS != ret) return Error(ret, msg.reason, dataSvr, millSecond);

        //返回数据
        return new CallResult<Void>();
    }

    //删除顶点一条边
    @SuppressWarnings({ "unchecked", "unused" })
    private CallResult<Void> DelLineForPoint(long pointId, long lineId, boolean isOut, int millSecond) {
        if (pointId <= 0 || lineId <= 0)
            return new CallResult<Void>(ResultCode.PARAM_ERROR, "顶点和边id必须 > 0");

        //从集群中选取一个服务节点
        NetConnect dataSvr = findNode(pointId);
        //执行请求
        MsgDelLineForPoint msg = new MsgDelLineForPoint();
        msg.pointId = pointId;
        msg.lineId = lineId;
        msg.isOut = isOut;

        ResultCode ret = dataSvr.query(msg, millSecond);
        if (ResultCode.SUCCESS != ret) return Error(ret, msg.reason, dataSvr, millSecond);

        //返回数据
        return new CallResult<Void>();
    }

    //删除边，并返回删除前连接的顶点
    @SuppressWarnings({ "unchecked", "unused" })
    private CallResult<Line> DelLine(long lineId, int millSecond) {
        if (lineId <= 0)
            return new CallResult<Line>(ResultCode.PARAM_ERROR, "边id必须 > 0");

        //从集群中选取一个服务节点
        NetConnect dataSvr = findNode(lineId);
        //执行请求
        MsgDelLine msg = new MsgDelLine();
        msg.lineId = lineId;

        ResultCode ret = dataSvr.query(msg, millSecond);
        if (ResultCode.SUCCESS != ret) return Error(ret, msg.reason, dataSvr, millSecond);

        //返回数据
        return new CallResult<Line>();
    }

    /*
     * 将2个顶点A,B连接起来
     * 必须给被连接的A,B添加边，最后创建边
     * 否则如果先创建边，然后A,B只有一个添加成功，假设A添加失败
     * 就会造成move out 从A出发找不到B，但从B出发可以找到A
     * 连接存在，就应该move out和move in查询全部可以找到对方顶点
     * 只要有一方找不到对方，就是连接不存在，未创建成功。
     * 
     * 最后创建边，可以保证，成功则out in查询都可以找到对方，失败则out in查询都找不到对方
     */
    public CallResult<Void> Link(Line line, int millSecond) {
        //生成id
        CallResult<Long> lineId = NewId(millSecond);
        if (ResultCode.SUCCESS.value() != lineId.getCode())
            return new CallResult<Void>(lineId.getCode(), "无法生成数据id：" + lineId.getReason());
        line.id = lineId.getData();
        
        //添加边
        CallResult<Void> ret = AddLineForPoint(line.startId, line.id, true, millSecond);
        if (ResultCode.SUCCESS.value() != ret.getCode()) return ret;

        //添加边
        ret = AddLineForPoint(line.endId, line.id, false, millSecond);
        if (ResultCode.SUCCESS.value() != ret.getCode()) return ret;

        //建立连接
        return AddLine(line, millSecond);
    }

    //断开out连接
    public CallResult<Void> Unlink(long startId, List<Filter> filters, int millSecond) {
        if ( startId <= 0 )
            return new CallResult<Void>(ResultCode.PARAM_ERROR, "数据id必须 > 0");
        if ( null == filters || filters.isEmpty() )
            return new CallResult<Void>(ResultCode.PARAM_ERROR, "没有选择删除的边");

        List<Long> startPoints = new ArrayList<Long>();
        startPoints.add(startId);
        SelectLine delLine = new SelectLine();
        delLine.moveOut = true;
        delLine.selectField = filters;
        List<SelectLine> selectLines = new ArrayList<SelectLine>();
        selectLines.add(delLine);
        return new CallResult<Void>(Move(startPoints, selectLines, null, false, null, Action.unlink, millSecond));
    }

    //查询删除边，不对用户暴露边数据
    @SuppressWarnings({ "unchecked", "unused" })
    private CallResult<Line> GetLine(long lineId, int millSecond) {
        if (lineId <= 0)
            return new CallResult<Line>(ResultCode.PARAM_ERROR, "边id必须 > 0");

        //从集群中选取一个服务节点
        NetConnect dataSvr = findNode(lineId);
        //执行请求
        MsgGetLine msg = new MsgGetLine();
        msg.lineId = lineId;

        ResultCode ret = dataSvr.query(msg, millSecond);
        if (ResultCode.SUCCESS != ret) return Error(ret, msg.reason, dataSvr, millSecond);

        //返回数据
        return new CallResult<Line>(msg.line);
    }

    //删除一个顶点
    @SuppressWarnings("unchecked")
    public CallResult<Void> DelPoint(long pointId, int millSecond) {
        if (pointId <= 0)
            return new CallResult<Void>(ResultCode.PARAM_ERROR, "顶点id必须 > 0");


        //如果先断开所有边，然后顶点删除失败，则会造成删除没有成功，但起点和终点却互相查询不到
        //只要顶点删除成功，则终点反向查找起点一定失败，起点查找终点由于没有起点数据也一定失败
        //只要顶点删除失败，则所有数据无修改。
        //所以边数据留着清理机制来清理
        
        //从集群中选取一个服务节点
        NetConnect dataSvr = findNode(pointId);
        //执行请求
        MsgDelPoint msg = new MsgDelPoint();
        msg.pointId = pointId;

        ResultCode ret = dataSvr.query(msg, millSecond);
        if (ResultCode.SUCCESS != ret) return Error(ret, msg.reason, dataSvr, millSecond);

        //返回数据
        return new CallResult<Void>();
    }

    //从顶点开始执行move
    public CallResult<List<Point>> Move(
        List<Long> startPoints, //move操作起点
        List<SelectLine> selectLines, //选择移动的边匹配条件， = null表示不移动，查询顶点startPoints数据
        List<Filter> filters, //选择顶点匹配条件
        boolean selectAll, //取得所有字段 = true时selectFields无效
        List<String> selectFields, //终点要获取的字段
        Action act,  //move的目的（要执行的动作）
        int millSecond) {
//        m_sendTime = 0;
//        m_recvTime = 0;
//        m_parseTime = 0;


        //参数检查
        if ( null == startPoints || 0 == startPoints.size() ) {
            return new CallResult<List<Point>>(ResultCode.PARAM_ERROR, "参数错误：没有起点"); 
        }
        if ( Action.unlink == act ){
            if ( null == selectLines || selectLines.isEmpty() ) {
                return new CallResult<List<Point>>(ResultCode.PARAM_ERROR, "参数错误：没有断开的边"); 
            }
            if ( null != filters && !filters.isEmpty() ){
                return new CallResult<List<Point>>(ResultCode.PARAM_ERROR, "参数太多：多余的顶点匹配条件"); 
            }
            if ( null != selectFields && !selectFields.isEmpty() ){
                return new CallResult<List<Point>>(ResultCode.PARAM_ERROR, "参数太多：多余的顶点字段选择参数"); 
            }
        }
        if ( null == selectLines ) selectLines = new ArrayList<SelectLine>();

        if ( 0 == selectLines.size() ){
            if (Action.count == act||Action.unlink == act){
                return new CallResult<List<Point>>(ResultCode.PARAM_ERROR, "缺少参数：count与unlink操作必须移动"); 
            }
        }
        if ( Action.del == act || Action.unlink == act ){
            if ( 1 < selectLines.size() ){
                return new CallResult<List<Point>>(ResultCode.PARAM_ERROR, "参数太多：unlink与del操作不支持多级移动"); 
            }
        }
        //检查起点id
        for ( Long pid : startPoints ){
            if ( 0 >= pid ){
                return new CallResult<List<Point>>(ResultCode.PARAM_ERROR, "参数错误：起点id必须>0"); 
            }
        }
        //参数检查完成
        
        //获取分布式唯一查询id
        if ( 0x3FFFFF == moveIndex ) {
            CallResult<Long> clientId = cfgSvr.NewClientId(1000);//1秒必须拿到clientId
            if ( ResultCode.SUCCESS.value() != clientId.getCode() ) return new CallResult<List<Point>>(clientId);
            this.clientId = clientId.getData();
            moveIndex = 0;
        }
        else moveIndex++;
        
        //在第一条路上移动
        List<Point> endPoint = new ArrayList<Point>();//移动得到的终点
        Map<Long, Boolean> repeat = new HashMap<Long, Boolean>();//终点排重
        Map<Byte, MoveMidData> midData = new HashMap<Byte, MoveMidData>();//移动中间数据，需要到其它节点查询的边和顶点
        boolean startFilter = false;
        if ( 0 == selectLines.size() ) startFilter = true;
        CallResult<Void> suc = this.Move(endPoint, repeat, midData, startFilter, true, startPoints, selectLines, filters, selectAll, selectFields, act, millSecond);
        if ( ResultCode.SUCCESS.value() != suc.getCode() ) {
            return new CallResult<List<Point>>(suc);
        }

        //从路过的边或者顶点继续移动
        byte i = 0;//从剩余的第一条路开始
        Map<Byte, MoveMidData> midDataFix = null;
        for ( ; 0 < selectLines.size(); ){
            if ( !midData.containsKey(i) ) {//检查第一条路是否完成移动
                i++;
                selectLines.remove(0);//完成移动的一定是第一条路，第一条路被删除后，第二条路自动成为第一条路，并且一定首先完成移动
                continue;
            }
            //修正剩余路线的路编号
            midDataFix = new HashMap<Byte, MoveMidData>();
            MoveMidData startObj = null;
            for ( Map.Entry<Byte, MoveMidData> item : midData.entrySet() ) {
                startObj = item.getValue();
                startObj.lineIndex -= i;
                if ( 0 > startObj.lineIndex ) continue;
                midDataFix.put(startObj.lineIndex, startObj);
            }
            midData = midDataFix;

            //完成当前路的移动
            startObj = midData.get((byte)0);
            if ( 0 < startObj.lineIds.size() ){
                List<Long> valueList = new ArrayList<Long>(startObj.lineIds.keySet());
                suc = Move(endPoint, repeat, midData, true, false, valueList, selectLines, filters, selectAll, selectFields, act, millSecond);
                if ( ResultCode.SUCCESS.value() != suc.getCode() ) return new CallResult<List<Point>>(suc);
            }
            selectLines.remove(0);//完成移动的一定是第一条路，第一条路被删除后，第二条路自动成为第一条路，并且一定首先完成移动
            startObj = midData.get((byte)0);
            List<Long> valueList = new ArrayList<Long>(startObj.pointIds.keySet());

            //修正剩余路线的路编号
            midDataFix = new HashMap<Byte, MoveMidData>();
            for ( Map.Entry<Byte, MoveMidData> item : midData.entrySet() ) {
                startObj = item.getValue();
                startObj.lineIndex--;
                if ( 0 > startObj.lineIndex ) continue;
                midDataFix.put(startObj.lineIndex, startObj);
            }
            midData = midDataFix;
            //当前路连接的终点作为新起点开始移动
            if ( 0 < valueList.size() ){//从新起点开始移动
                suc = Move(endPoint, repeat, midData, true, true, valueList, selectLines, filters, selectAll, selectFields, act, millSecond);
                if ( ResultCode.SUCCESS.value() != suc.getCode() ) {
                    return new CallResult<List<Point>>(suc);
                }
            }
            
            i = 0;//从剩余的第一条路开始
        }
        
//        System.out.println( "打包（纯本地代码）+发送（服务端代码+网络传输）用时:" + m_sendTime + "毫秒");
//        System.out.println( "接收（服务端代码+网络传输）用时:" + m_recvTime + "毫秒");
//        System.out.println( "解包（纯本地代码）用时:" + m_parseTime + "毫秒");
        

        return new CallResult<List<Point>>(endPoint);
    }

    //从顶点or边开始move
    @SuppressWarnings("unchecked")
    private CallResult<Void> Move(
        List<Point> endPoint, //移动得到的终点
        Map<Long, Boolean> repeat, //终点排重
        Map<Byte, MoveMidData> midData, //移动中间数据，需要到其它节点查询的边和顶点
        boolean startFilter, //过滤起点
        boolean fromPoint, //ids是顶点id,move从顶点出发，否则move从边出发
        List<Long> ids, //move操作开始对象id，可能是顶点，也可能是边
        List<SelectLine> selectLines, //选择移动的边匹配条件
        List<Filter> filters, //选择顶点匹配条件
        boolean selectAll, //取得所有字段 = true时selectFields无效
        List<String> selectFields, //终点要获取的字段
        Action act,  //move的目的（要执行的动作）
        int millSecond
        ){
        if ( null == filters ) filters = new ArrayList<Filter>();//过滤条件
        if ( null == selectFields ) selectFields = new ArrayList<String>();//选取的字段

        //将起点分组
        Map<Integer, List<Long>> queryIds = new HashMap<Integer, List<Long>>();//map<节点id,起点id列表>
        Integer nodeId;
        for ( Long id: ids){
            nodeId = findNodeId(id.longValue());
            if ( !queryIds.containsKey(nodeId) ) queryIds.put(nodeId, new ArrayList<Long>());
            queryIds.get(nodeId).add(id);
        }

        //向集群发送请求，分布式计算
        MsgSetMoveOpt queryOpt = new MsgSetMoveOpt();
        queryOpt.selectLines = selectLines; //选择要走的边
        queryOpt.filters = filters;//过滤条件
        queryOpt.selectAll = selectAll;
        queryOpt.selectFields = selectFields;//选取的字段
        queryOpt.act = act;
        ResultCode suc = null;
        for ( Map.Entry<Integer, List<Long>> item : queryIds.entrySet() ){
            //init方法确保了findNodeId()return的id,用GetNode()一定可以找到连接，不用检查返回结果
            nodeId = item.getKey();
            NetConnect dataSvr = GetNode(nodeId);
//            long sendTime = GCluster.millSecond();
            suc = dataSvr.sendMsg(queryOpt, 5000);//5秒必须发送完成
//            sendTime = GCluster.millSecond() - sendTime;
//            m_sendTime += sendTime;

            if ( ResultCode.SUCCESS != suc ) return new CallResult<Void>(suc);
            MsgMove queryMove = new MsgMove();
            queryMove.searchId = clientId+moveIndex;//搜索唯一id
            queryMove.fromPoint = fromPoint;//从顶点开始移动
            queryMove.startFilter = startFilter;//过滤起点
            queryMove.ids = item.getValue();//起点id列表
            queryMove.startPos = 0;//id列表打包开始位置
            //起点数量可能>1000，但每次发送的报文最多容纳1000个起点，所以需要多次发送
            while ( queryMove.startPos < queryMove.ids.size() ){
//                sendTime = GCluster.millSecond();
                suc = dataSvr.sendMsg(queryMove, 5000);//5秒必须发送完成
//                sendTime = GCluster.millSecond() - sendTime;
//                m_sendTime += sendTime;
                if ( ResultCode.SUCCESS != suc ) return new CallResult<Void>(suc);
            }
        }
        
        //接收分布式结果
        for ( Map.Entry<Integer, List<Long>> item : queryIds.entrySet() ){
            //init方法确保了findNodeId()return的id,用GetNode()一定可以找到连接，不用检查返回结果
            nodeId = item.getKey();
            NetConnect dataSvr = GetNode(nodeId);
//            long parseTime = GCluster.millSecond();
            while ( true ) {
//                parseTime = GCluster.millSecond() - parseTime;
//                m_parseTime += parseTime;
                MsgDefaultParam msgData = new MsgDefaultParam(10240, true, (short)ServiceType.GMAP.value());
//                long recvTime = GCluster.millSecond();
                suc = dataSvr.recvMsg(msgData, 5000);//5秒必须收到结果
//                recvTime = GCluster.millSecond() - recvTime;
//                m_recvTime += recvTime;
                if ( ResultCode.SUCCESS != suc ) {
                    return Error(suc, msgData.reason, dataSvr, 5000);
                }

//                parseTime = GCluster.millSecond();
                if ( ProtoclNo.MOVE.value() == msgData.id ){//1次Move请求结果传输完成
                    MsgMove msg = new MsgMove();
                    msg.memcpy(msgData.Bytes(), msgData.Size());
                    if ( !msg.Parse() ) return new CallResult<Void>(ResultCode.MSG_FORMAT_ERROR);
                    if ( ResultCode.SUCCESS.value() != msg.code ) return new CallResult<Void>(msg.code, msg.reason);

                    break;//请求完成
                }
                if ( ProtoclNo.POINTS.value() == msgData.id ){//终点数据
                    MsgPoints msg = new MsgPoints();
                    msg.memcpy(msgData.Bytes(), msgData.Size());
                    if ( !msg.Parse() ) return new CallResult<Void>(ResultCode.MSG_FORMAT_ERROR);
                    if ( ResultCode.SUCCESS.value() != msg.code ) return new CallResult<Void>(msg.code, msg.reason);
                    for ( Point point : msg.points ){
                        if ( repeat.containsKey(point.id) ) continue;//排重
                        repeat.put(point.id, true);
                        endPoint.add(point);
                    }
                }
                if ( ProtoclNo.MOVE_MIDS.value() == msgData.id ){//中间数据
                    MsgMoveMids msg = new MsgMoveMids();
                    msg.memcpy(msgData.Bytes(), msgData.Size());
                    if ( !msg.Parse() ) return new CallResult<Void>(ResultCode.MSG_FORMAT_ERROR);
                    if ( ResultCode.SUCCESS.value() != msg.code ) return new CallResult<Void>(msg.code, msg.reason);
                    if ( !midData.containsKey(msg.lineIndex) ) {
                        midData.put(msg.lineIndex, new MoveMidData(msg.lineIndex));
                    }
                    MoveMidData mid = midData.get(msg.lineIndex);
                    if ( msg.isLine ) for ( Long id : msg.ids) mid.lineIds.put(id, true);//保存需要到其它节点查找的边id
                    else for ( Long id : msg.ids) mid.pointIds.put(id, true);//保存需要到其它节点查找的顶点id
                }
            }
        }
        
        return new CallResult<Void>();
    }

    ////////////////////////////////////////////////////////////////////////////////
    //批量操作接口
    //创建count个全局唯一的新id
    @SuppressWarnings("unchecked")
    private CallResult<List<Long>> NewIds(int count, int millSecond) {
        if ( 0 >= count ) return new CallResult<List<Long>>(ResultCode.PARAM_ERROR, ResultCode.PARAM_ERROR.descript() + ":期望产生id数量必须 > 1");
        if ( GridLimit.maxObjectCount.value() < count ) return new CallResult<List<Long>>(ResultCode.PARAM_ERROR, ResultCode.PARAM_ERROR.descript() + ":1次批量产生id最多" + GridLimit.maxObjectCount.value() + "个");
        
        int average = count / nodeCount();//每个节点需要产生的id数量
        int remainder = count - average * nodeCount();//除不尽的余数
        
        //从集群中每个服务节点上产生一部分id
        List<Long> ids = new ArrayList<Long>();
        int i = 0;
        for ( ; i < nodeCount(); i++ ){
            NetConnect dataSvr = this.routeNode();
            if (null == dataSvr) return new CallResult<List<Long>>(ResultCode.NO_SERVER);

            //执行请求
            MsgNewId msg = new MsgNewId();
            msg.m_count = average + remainder;//需要产生平均数+余数个id
            if ( 0 != remainder ) remainder = 0;//余数只加1次

            ResultCode ret = dataSvr.query(msg, millSecond);
            if (ResultCode.SUCCESS != ret) return Error(ret, msg.reason, dataSvr, millSecond);
            long j = 0;
            for ( ; j < msg.m_count; j++ ){
                ids.add(msg.m_id + j);
            }
        }
        
        return new CallResult<List<Long>>(ids);
    }

    //批量创建顶点（分布式计算）
    @SuppressWarnings("unchecked")
    public CallResult<Void> AddPoints( List<Point> points, int millSecond )
    {
        if ( null == points || 1 >= points.size() ) return new CallResult<Void>(ResultCode.PARAM_ERROR, ResultCode.PARAM_ERROR.descript() + ":顶点数量必须>1");
        
        CallResult<List<Long>> ids = this.NewIds(points.size(), millSecond);
        if ( ResultCode.SUCCESS.value() != ids.getCode() ) return new CallResult<Void>(ids.getCode(), "不能生成数据id:" + ids.getReason() );

        //将顶点分组到归属的节点
        int i = 0;
        Map<Integer, List<Point>> queryPoints = new HashMap<Integer, List<Point>>();//map<节点id,顶点列表>
        Integer nodeId;
        for ( i = 0; i < ids.getData().size(); i++ ){
            points.get(i).id = ids.getData().get(i);
            nodeId = findNodeId(points.get(i).id);
            if ( !queryPoints.containsKey(nodeId) ) queryPoints.put(nodeId, new ArrayList<Point>());
            queryPoints.get(nodeId).add(points.get(i));
        }
        
        ResultCode code;
        //分布式计算
        for ( Map.Entry<Integer, List<Point>> item : queryPoints.entrySet() ){
            //init方法确保了findNodeId()return的id,用GetNode()一定可以找到连接，不用检查返回结果
            nodeId = item.getKey();
            NetConnect dataSvr = GetNode(nodeId);
            MsgSetPoints msg = new MsgSetPoints();
            msg.isCreate = true;
            msg.points = item.getValue();
            msg.startPos = 0;

            while ( !msg.isEnd ){
                code = dataSvr.sendMsg(msg, millSecond);
                if ( ResultCode.SUCCESS != code ) {
                    this.CloseConnect();//分布式计算一旦请求失败，应该断开集群，避免其它成功节点有剩余数据未接收，破坏之后的请求
                    return Error(code, msg.reason, dataSvr, millSecond);
                }
            }
        }
        //获取分布式计算结果
        for ( Map.Entry<Integer, List<Point>> item : queryPoints.entrySet() ){
            //init方法确保了findNodeId()return的id,用GetNode()一定可以找到连接，不用检查返回结果
            nodeId = item.getKey();
            NetConnect dataSvr = GetNode(nodeId);
            MsgSetPoints msg = new MsgSetPoints();
            code = dataSvr.recvMsg(msg, millSecond);
            if ( ResultCode.SUCCESS != code ) {
                this.CloseConnect();//分布式计算一旦请求失败，应该断开集群，避免其它成功节点有剩余数据未接收，破坏之后的请求
                return Error(code, msg.reason, dataSvr, millSecond);
            }
        }
        
        return new CallResult<Void>();
    }

    //批量连接顶点
    public CallResult<Void> LinkPoints( List<Line> lines, int millSecond )
    {
        return new CallResult<Void>(ResultCode.REFUSE, "尚未提供批量连接顶点的服务");
    }

    //批量修改顶点
    public CallResult<Void> SetPoints( List<Point> points, int millSecond )
    {
        return new CallResult<Void>(ResultCode.REFUSE, "尚未提供批量修改顶点的服务");
    }

    //批量删除顶点
    public CallResult<Void> DetPoints( List<Long> ids, int millSecond )
    {
        return new CallResult<Void>(this.Move(ids, null, null, false, null, Action.del, millSecond));
    }
}
