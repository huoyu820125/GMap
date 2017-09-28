package com.huoyu.gmap;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;

import com.huoyu.GClient;
import com.huoyu.client.CallResult;
import com.huoyu.client.ResultCode;
import com.huoyu.gmap.svr.gmap.GCluster;
import com.huoyu.gmap.svr.gmap.constant.FieldType;
import com.huoyu.gmap.svr.gmap.datastruct.Field;
import com.huoyu.gmap.svr.gmap.datastruct.Line;
import com.huoyu.gmap.svr.gmap.datastruct.Point;

/**
 * Unit test for simple App.
 */
public class AppTest 
    extends TestCase
{
    /**
     * Create the test case
     *
     * @param testName name of the test case
     */
    public AppTest( String testName )
    {
        super( testName );
    }

    /**
     * @return the suite of tests being tested
     */
    public static Test suite()
    {
        return new TestSuite( AppTest.class );
    }

    /**
     * Rigourous Test :-)
     */
    @SuppressWarnings("unchecked")
    public void testApp()
    {
        getMore(412316860516L);
        getMore(412316860516L);
        getMore(412316860516L);
        getMore(412316860516L);
        getMore(412316860516L);
        getMore(412316860516L);
        getMore(412316860516L);
        getMore(412316860516L);
        getMore(412316860516L);
        getMore(412316860516L);
        getMore(412316860516L);
        getMore(412316860516L);
        getMore(412316860516L);
        speed();
        example();

        long startTime = GCluster.millSecond();
        List<Long> pidList = AddPoints(10000);
        startTime = GCluster.millSecond() - startTime;
        System.out.println( "批量创建1万个顶点用时:" + startTime + "毫秒");
        long p0 = pidList.get(0);
        long p1 = pidList.get(1);
        if ( !LinkPoints(p0, pidList, 1, 2000, LinkType.Friend) ) return;
        if ( !LinkPoints(p0, pidList, 2001, 2000, LinkType.Schoolmate) ) return;
        if ( !LinkPoints(p0, pidList, 4001, 2000, LinkType.Blacklist) ) return;
        if ( !LinkPoints(p1, pidList, 1001, 2000, LinkType.Friend) ) return;
        if ( !LinkPoints(p1, pidList, 3001, 2000, LinkType.Schoolmate) ) return;
        
        //2级连接
        int i = 0;
        for ( i = 1; i < 1001; i++ )
        {
            LinkPoints(pidList.get(i), pidList, 5000 + i, 1, LinkType.Schoolmate);
        }
        for ( i = 2001; i < 3001; i++ )
        {
            LinkPoints(pidList.get(i), pidList, 3500 + i, 1, LinkType.Schoolmate);
        }
        
        GClient svr = new GClient();
        CallResult<Void> suc = svr.Init("127.0.0.1", 8888);
        if ( ResultCode.SUCCESS.value() != suc.getCode() )
        {
            System.out.println( "初始化对象失败:" + suc.getReason() );
            return;
        }
        
        while ( true ){
            CallResult<Long> m2count = svr.Sql("count from " + p0 + "," + p1 
                + " out [关系id]=[" + LinkType.Friend.value() + "] "
                + " out [关系id]=[" + LinkType.Schoolmate.value() + "] ;"
                );
            if ( ResultCode.SUCCESS.value() !=  m2count.getCode() ){
                System.out.println( "move失败:" + m2count.getReason() );
                continue;
            }
            System.out.println( "move可到达" + m2count.getData() + "个顶点");
        
            CallResult<List<Point>> points = svr.Sql("get all from " + p0 + "," + p1 
                + " out [关系id]=[" + LinkType.Friend.value() + "] ;"
                );
            if ( ResultCode.SUCCESS.value() !=  points.getCode() ){
                System.out.println( "move失败:" + points.getReason() );
                continue;
            }
            System.out.println( "move可到达" + points.getData().size() + "个顶点");
        
            suc = svr.Sql("unlink from " 
            + points.getData().get(0).id 
            + "," + points.getData().get(1).id
                + " in [关系id]=[" + LinkType.Friend.value() + "];" );
        
            if ( ResultCode.SUCCESS.value() != suc.getCode() ){
                System.out.println( "unlink失败:" + suc.getReason() );
                continue;
            }
            System.out.println( "断开1个顶点");
        
        }
    }


    @SuppressWarnings("unchecked")
    public static List<Long> AddPoints(int count)
    {
        GClient svr = new GClient();
        CallResult<Void> suc = svr.Init("192.168.2.225", 8888);
        if ( ResultCode.SUCCESS.value() != suc.getCode() )
        {
            System.out.println( "初始化对象失败:" + suc.getReason() );
            return null;
        }
        
        Point point = new Point();
        int i = 0;
        List<Long> pidList = new ArrayList<Long>();
        for ( i = 0; i < count; i++ )
        {
            CallResult<Long> id = svr.Sql("add [姓名]=[孙尚天],[余额]=[1],[用户id]=[1];");
            if ( ResultCode.SUCCESS.value() != id.getCode() )
            {
                System.out.println( "创建顶点失败:" + id.getReason() );
                return null;
            }
            point.id = id.getData();

            CallResult<List<Point>> points = svr.Sql("get id from " + point.id + ";");
            if ( ResultCode.SUCCESS.value() != points.getCode() )
            {
                System.out.println( "查询顶点失败:" + points.getReason() );
                return null;
            }
            point = points.getData().get(0);
            
            suc = svr.Sql("edit " + point.id + " [姓名]=[孙尚天" + i + "],[余额]=[10" + i + "],[用户id]=[20" + i + "];");
            if ( ResultCode.SUCCESS.value() != suc.getCode() )
            {
                System.out.println( "修改顶点失败:" + suc.getReason() );
                return null;
            }
            
            points = svr.Sql("get id from " + point.id + ";");
            if ( ResultCode.SUCCESS.value() != points.getCode() )
            {
                System.out.println( "查询顶点失败:" + points.getReason() );
                return null;
            }
            point = points.getData().get(0);
            pidList.add(point.id);
        }
        
        return pidList;
    }
    
    @SuppressWarnings("unchecked")
    public static boolean LinkPoints(long startPointId, List<Long> endList, int startPos, int linkCount, LinkType linkType){
        GClient svr = new GClient();
        CallResult<Void> suc = svr.Init("127.0.0.1", 8888);
        if ( ResultCode.SUCCESS.value() != suc.getCode() )
        {
            System.out.println( "初始化对象失败:" + suc.getReason() );
            return false;
        }

        int i = 0;
        for ( ; i < linkCount; i++ ){
            CallResult<Long> id = svr.Sql("link " + startPointId + " to " + endList.get(startPos + i) 
                + " by [关系id]=[" + linkType.value() + "],[关系]=[" + linkType.descript() + "];");
            if ( ResultCode.SUCCESS.value() != id.getCode() )
            {
                System.out.println( "建立连接失败:" + id.getReason() );
                return false;
            }
        }
        
        return true;
    }

    @SuppressWarnings("unchecked")
    public static void example(){
        GClient svr = new GClient();
        CallResult<Void> suc = svr.Init("127.0.0.1", 8888);
        if ( ResultCode.SUCCESS.value() != suc.getCode() ){
            System.out.println( "初始化对象失败:" + suc.getReason() );
            return;
        }
        
        Point point = new Point();
        Line line = new Line();
        //创建2个顶点
        CallResult<Long> id = null;
        int i = 0;
        for ( i = 0; i < 2; i++ )
        {
            //创建1个顶点
            id = svr.Sql("add [用户id]=[" + (i + 1) + "], [姓名]=[孙钱], [余额]=[100];");
            if ( ResultCode.SUCCESS.value() != id.getCode() )
            {
                System.out.println( "创建顶点失败:" + id.getReason() );
                return;
            }
            //取得刚才创建的顶点
            CallResult<List<Point>> data = svr.Sql("get all from " + id.getData() + ";");
            if ( ResultCode.SUCCESS.value() != data.getCode() )
            {
                System.out.println( "查询顶点失败:" + data.getReason() );
                return;
            }
            point = data.getData().get(0);
            System.out.println( "新顶点" + point.id + ":" );
            for ( Map.Entry<String, Field> item : point.data.entrySet() )
            {
                if ( FieldType.str == item.getValue().type ) {
                    System.out.println( "[" + item.getKey() + "] " + item.getValue().type.descript() + " = " + item.getValue().data);
                }
                else {
                    System.out.println( "[" + item.getKey() + "] " + item.getValue().type.descript() + " = " + item.getValue().value);
                }
            }
            //修改刚才创建的顶点
            String name = "孙尚天";
            if ( 1 == i ) name = "孙炸天";
            suc = svr.Sql("edit " + point.id + " [姓名]=[" + name + "], [余额]=[100000000];");
            if ( ResultCode.SUCCESS.value() != suc.getCode() )
            {
                System.out.println( "修改顶点失败:" + suc.getReason() );
                return;
            }
            //取得刚才修改的顶点
            data = svr.Sql("get all from " + id.getData() + ";");
            if ( ResultCode.SUCCESS.value() != data.getCode() )
            {
                System.out.println( "查询顶点失败:" + data.getReason() );
                return;
            }
            point = data.getData().get(0);
            System.out.println( "修改后顶点" + point.id + ":" );
            for ( Map.Entry<String, Field> item : point.data.entrySet() )
            {
                if ( FieldType.str == item.getValue().type ) {
                    System.out.println( "[" + item.getKey() + "] " + item.getValue().type.descript() + " = " + item.getValue().data);
                }
                else {
                    System.out.println( "[" + item.getKey() + "] " + item.getValue().type.descript() + " = " + item.getValue().value);
                }
            }
            if ( 0 == i ) line.startId = point.id;
            else line.endId = point.id;
        }
        
        //连接刚才创建的2个顶点
        id = svr.Sql("Link " + line.startId + " to " + line.endId + " by [关系id]=[1],[关系]=[好友]; ");
        if ( ResultCode.SUCCESS.value() != id.getCode() )
        {
            System.out.println( "建立连接失败:" + id.getReason() );
            return;
        }
        CallResult<List<Point>> points = svr.Sql("get all from " + line.startId + " out [关系id]=[1]; ");
        if ( ResultCode.SUCCESS.value() != points.getCode() )
        {
            System.out.println( "不可到达顶点:" + points.getReason() );
            return;
        }
        point = points.getData().get(0);
        System.out.println( "找到顶点" + point.id + ":" );
        for ( Map.Entry<String, Field> item : point.data.entrySet() )
        {
            if ( FieldType.str == item.getValue().type ) {
                System.out.println( "[" + item.getKey() + "] " + item.getValue().type.descript() + " = " + item.getValue().data);
            }
            else {
                System.out.println( "[" + item.getKey() + "] " + item.getValue().type.descript() + " = " + item.getValue().value);
            }
        }
        
        //删除找到的顶点
        suc = svr.Sql("del " + point.id + ";");
        if ( ResultCode.SUCCESS.value() != suc.getCode() )
        {
            System.out.println( "删除顶点失败:" + suc.getReason() );
            return;
        }
        //移动到刚才删除的顶点，正确结果是无数据
        points = svr.Sql("get all from " + point.id + ";");
        if ( ResultCode.NO_DATA.value() == points.getCode() )
        {
            System.out.println( "顶点已删除" );
        }
        else if ( ResultCode.SUCCESS.value() != points.getCode() )
        {
            System.out.println( "查询顶点失败:" + points.getReason() );
        }
        else 
        {
            System.out.println( "图数据库存在bug，查询到已删除的顶点" );
            return;
        }

        //批量创建10万个顶点
        List<Point> pointList = new ArrayList<Point>();
        for ( i = 0; i < 100000; i++ ){
            point = new Point();
            point.id = i + 1;
            point.data.put("id", new Field("id", "" + i));
            point.data.put("姓名", new Field("姓名", "火总" + i));
            point.data.put("余额", new Field("余额", "" + i));
            pointList.add(point);
        }
        long startTime = GCluster.millSecond();
        suc = svr.AddPoints(pointList, 0);
        if ( ResultCode.SUCCESS.value() != suc.getCode() ){
            System.out.println( "批量创建10万个顶点失败:" + suc.getReason() );
        }
        startTime = GCluster.millSecond() - startTime;
        System.out.println( "批量创建10万个顶点用时:" + startTime + "毫秒");

        System.out.println( "success!" );
    }

    public static void speed(){
        GClient svr = new GClient();
        CallResult<Void> suc = svr.Init("192.168.2.225", 8888);
        if ( ResultCode.SUCCESS.value() != suc.getCode() ){
            System.out.println( "初始化对象失败:" + suc.getReason() );
            return;
        }

        List<Long> pids = new ArrayList<Long>();
        int i = 0;
        for ( i = 0; i < 10; i++ ){
            List<Long> pids0 = AddPoints(1000000);
            for ( Long id : pids0 ){
                pids.add(id);
            }
        }
        
        System.out.println( "起点id:" + pids.get(100) );
        if ( !LinkPoints(pids.get(100), pids, 101, 2000, LinkType.Friend) ) return;
        int startPos = 10000;
        for ( i = 101; i < 2100; i++ ){
            if ( !LinkPoints(pids.get(i), pids, startPos, 1000, LinkType.Friend) ) return;
            startPos += 1000;
        }
        System.out.println( "起点id:" + pids.get(100) );
    }

    @SuppressWarnings("unchecked")
    public static void getMore( long startPointId ){
        GClient svr = new GClient();
        CallResult<Void> suc = svr.Init("192.168.2.225", 8888);
        if ( ResultCode.SUCCESS.value() != suc.getCode() ){
            System.out.println( "初始化对象失败:" + suc.getReason() );
            return;
        }
        long startTime = GCluster.millSecond();
        CallResult<List<Point>> points = svr.Sql("get all from " + startPointId 
            + " out [关系id]=[" + LinkType.Friend.value() + "] "
            + " out [关系id]=[" + LinkType.Friend.value() + "] ;"
            );
        if ( ResultCode.SUCCESS.value() !=  points.getCode() ){
            System.out.println( "move失败:" + points.getReason() );
            return;
        }
        startTime = GCluster.millSecond() - startTime;
        System.out.println( "move可到达" + points.getData().size() + "个顶点,用时:" + startTime + "毫秒");
    }

}
