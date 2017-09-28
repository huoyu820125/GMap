package com.huoyu.client;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * 时间类
 * 
 * @author 孙尚天
 * @version $Id: Time.java, v 0.1 2016年12月1日 上午11:09:18 孙尚天 Exp $
 */
public class Time {
    Date curDate = null;
    boolean timeStarted = false;//开始计时
    long duration = 0;//从timeStarted开始，经历的时间毫秒
    
    
    public Time(){
        start();
    }

    //开始计时
    public void start(){
        timeStarted = true;
        curDate = new Date();
    }
    
    //开始计时后，经过的毫秒数，对象已创建就开始第一次计时
    public long after(){
        if ( timeStarted ){
            duration = new Date().getTime() - curDate.getTime();
        }
        
        return duration;
    }
    /*
     * 从startTime开始，到当前时间，经过的天数
     * startTime唯一格式：yyyy-MM-dd HH:mm:ss
     */
    public static int days( String startTime )
    {
        Date curDate = new Date();
        long curTime = curDate.getTime();//距离1970年1月1日0点0分0秒的毫秒数
        curTime = curTime/1000 - millSecond(startTime);

        return (int)(curTime/86400);
    }

    /*
     * 字符串时间转换成，距离1970年1月1日0点0分0秒的秒数
     * 时间格式：yyyy-MM-dd HH:mm:ss
     */
    public static long millSecond(String st)
    {
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        Date date = null;
        try {
            date = sdf.parse(st);
        } catch (ParseException e) {//认为只有字符串格式不对异常
            return -1;
        }
        return date.getTime() / 1000;
    }
    
    //睡眠
    public void sleep(long millSecond) {
        try {
            Thread.sleep(millSecond);
        } catch (InterruptedException ex) {}//认为不会发生异常
    }
}
