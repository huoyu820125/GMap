package com.huoyu.client;

import com.huoyu.client.msg.Message;

/**
 * 包含默认参数的消息
 * @author 孙尚天
 * @version $Id: MsgDefaultParam.java, v 0.1 2016年11月23日 下午4:24:41 孙尚天 Exp $
 */
public class MsgDefaultParam extends Message {

    public MsgDefaultParam(int size, boolean smallOrder, short moudleId) {
        super(size, smallOrder, moudleId);
    }

    public long   connectId = 1;             //连接Id
    public int    objectId  = 2;             //对象Id
    public byte[] ip        = { 3, 4, 5, 6 }; //连接Ip
    public byte   code      = 0;             //错误码0成功，非0失败
    public String reason    = "";            //失败原因

    /**
     * @Title: 填充默认参数
     * @Description:
     * @return
     * @date:2016年11月21日 下午8:15:32
     * @author:孙尚天
     */
    public boolean defaultParam() {
        if (!addInt64(connectId))
            return false;
        if (!addInt32(objectId))
            return false;
        if (!addInt8(ip[0]))
            return false;
        if (!addInt8(ip[1]))
            return false;
        if (!addInt8(ip[2]))
            return false;
        if (!addInt8(ip[3]))
            return false;
        if (!addInt8(code))
            return false;
        if (0 != code) {
            if (!addString(reason))
                return false;
        }

        return true;
    }

    //解析默认参数
    @Override
    public boolean Parse() {
        if (!super.Parse())
            return false;
        connectId = getInt64();
        if (readError)
            return false;
        objectId = getInt32();
        if (readError)
            return false;
        ip[0] = (byte) getUInt8();
        if (readError)
            return false;
        ip[1] = (byte) getUInt8();
        if (readError)
            return false;
        ip[2] = (byte) getUInt8();
        if (readError)
            return false;
        ip[3] = (byte) getUInt8();
        if (readError)
            return false;
        code = (byte) getUInt8();
        if (readError)
            return false;
        if (0 != code) {
            reason = getString();
            if (readError)
                return false;
        }

        return true;
    }
}
