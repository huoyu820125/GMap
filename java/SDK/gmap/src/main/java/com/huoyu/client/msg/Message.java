/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.huoyu.client.msg;

/**
 * 报文类
 * 通信协议基础类，封装报文头的构造与解析
 * @author 孙尚天
 * @version $Id: Message.java, v 0.1 2016年11月21日 下午7:53:30 孙尚天 
 */
public class Message extends NetStream {
    private boolean headerIsReaded; //报文头已经解析,在数据流中是int8类型
    public short   moudleId;      //模块Id,在数据流中是uint8类型
    public short   id;            //报文id
    public boolean isResult;      //是回应

    /**
     * 构造报文
     * @param size  报文最大长度
     * @param smallOrder 使用小端网络字节序
     */
    public Message(int size, boolean smallOrder, short moudleId) {
        super(size, smallOrder);
        this.moudleId = moudleId;
        Init();
    }

    /**
     * 构造报文，最大长度1024，使用小端网络字节序
     */
    public Message(short moudleId) {
        super(1024, false);
        this.moudleId = moudleId;
        Init();
    }

    /**
     * @see client.msg.NetStream#Init()
     */
    @Override
    public void Init() {
        super.Init();
        headerIsReaded = false;
    }

    /**
     * @Title:设置报文id
     * @Description:
     * @param id
     * @param isResult 报文是请求，还是结果
     * @date:2016年11月22日 下午2:08:48
     * @author:孙尚天
     */
    public void SetId(short id, boolean isResult) {
        Init();
        this.id = id;
        this.isResult = isResult;
        addInt8((byte) moudleId);
        addInt16(this.id);
        addBoolean(isResult);
        ParamStart();
    }

    //报文头长度
    public int HeaderSize() {
        return 4 + 1 + 2 + 1;
    }

    public boolean Build(boolean isResult) {
        return false;
    }

    //解析报文
    public boolean Parse() {
        return ReadHeader();
    }

    //解析报文头
    protected boolean ReadHeader() {
        if (headerIsReaded) {
            ParamStart();
            return true;
        }
        if (0 > Size())
            return false;

        moudleId = getUInt8();
        if (readError)
            return false;

        id = getInt16();
        if (readError)
            return false;

        byte valueInt8 = getInt8();
        if (readError)
            return false;
        this.isResult = 1 == valueInt8 ? true : false;
        headerIsReaded = true;
        ParamStart();

        return true;
    }

    //报文是回应
    public boolean isResult() {
        return isResult;
    }

}
