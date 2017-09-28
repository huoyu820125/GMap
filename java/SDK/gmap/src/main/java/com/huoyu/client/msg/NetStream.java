/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.huoyu.client.msg;

import java.io.UnsupportedEncodingException;

/**
 * 网络数据流类
 * 封装普通类型压入流中和从流中取出
 * @author 孙尚天
 * @version $Id: NetStream.java, v 0.1 2016年11月22日 上午7:53:30 孙尚天
 */
public class NetStream {
    private byte[]  stream;       //网络数据流存储空间
    private int     space;        //存储空间的容量，最大能存储的数据长度
    protected int     size;         //数据实际长度
    protected int     pos;          //当前读取位置
    private boolean smallOrder;   //小端网络字节序
    private int     curParamIndex; //正在解析的参数编号

    public NetStream(int size, boolean smallOrder) {
        this.space = size;
        this.stream = new byte[this.space];
        this.smallOrder = smallOrder;
        Init();
    }

    public boolean memcpy(byte[] stream, int size) {
        if (size > space) {
            return false;
        }

        int i = 0;
        for (i = 0; i < size; i++) {
            this.stream[i] = stream[i];
        }
        if (0 > Size()) {
            return false;
        }

        return true;
    }

    public void Init() {
        size = 4;
        pos = 0;
        I2Stream(stream, 0, size, 4);
    }

    public boolean addBoolean(boolean v) {
        curParamIndex++;
        if (size + 1 > space) {
            return false;
        }

        stream[size] = (byte) (!v?0:1);
        size += 1;
        return true;
    }

    public boolean addInt8(byte v) {
        curParamIndex++;
        if (size + 1 > space) {
            return false;
        }

        stream[size] = v;
        size += 1;
        return true;
    }

    public boolean addInt16(short v) {
        curParamIndex++;
        if (size + 2 > space) {
            return false;
        }

        I2Stream(stream, size, v, 2);
        size += 2;
        return true;
    }

    public boolean addInt32(int v) {
        curParamIndex++;
        if (size + 4 > space) {
            return false;
        }

        I2Stream(stream, size, v, 4);
        size += 4;
        return true;
    }

    public boolean addInt64(long v) {
        curParamIndex++;
        if (size + 8 > space) {
            return false;
        }

        I2Stream(stream, size, v, 8);
        size += 8;
        return true;
    }

    public boolean addUInt8(short v) {
        return addInt8((byte) v);
    }

    public boolean addUInt16(int v) {
        return addInt16((short) v);
    }

    public boolean addUInt32(long v) {
        return addInt32((int) v);
    }

    /**
     * 
     * @Title:将字符串转成gbk编码并在末尾加'\0'，存入报文
     * @Description:
     * @param v
     * @return
     * @date:2016年11月23日 下午5:40:02
     * @author:孙尚天
     */
    public boolean addString(String v) {
        if ( null == v ) {
            return false;
        }
        
        curParamIndex++;
        byte[] gbk = null;
        try {
            gbk = v.getBytes("gbk");
        } catch (UnsupportedEncodingException e) {
            return false;
        }

        curParamIndex--;
        if (!addInt32(gbk.length + 1)) {
            return false;
        }
        if (size + gbk.length + 1 > space) {
            return false;
        }
        int i = 0;
        for (i = 0; i < gbk.length; i++) {
            stream[size] = gbk[i];
            size++;
        }
        stream[size] = 0;
        size++;

        return true;
    }

    /**
     * 
     * @Title:将字符串转成gbk编码，存入报文
     * @Description:
     * @param v
     * @return
     * @date:2016年11月23日 下午5:40:02
     * @author:孙尚天
     */
    public boolean addStream(String v) {
        if ( null == v ) {
            return false;
        }
        
        curParamIndex++;
        byte[] gbk = null;
        try {
            gbk = v.getBytes("gbk");
        } catch (UnsupportedEncodingException e) {
            return false;
        }

        curParamIndex--;
        if (!addInt32(gbk.length)) {
            return false;
        }
        if (size + gbk.length > space) {
            return false;
        }
        int i = 0;
        for (i = 0; i < gbk.length; i++) {
            stream[size] = gbk[i];
            size++;
        }

        return true;
    }

    public boolean readError = false;

    public byte getInt8() {
        curParamIndex++;
        readError = true;
        byte value = 0;
        if (pos + 1 > size)
            return 0;
        value = stream[pos];
        pos += 1;
        readError = false;
        return value;
    }

    public short getInt16() {
        curParamIndex++;
        readError = true;
        short value = 0;
        if (pos + 2 > size)
            return 0;
        value = (short) Stream2I(stream, pos, 2);
        pos += 2;
        readError = false;
        return value;
    }

    public int getInt32() {
        curParamIndex++;
        readError = true;
        int value = 0;
        if (pos + 4 > size)
            return 0;
        value = (int) Stream2I(stream, pos, 4);
        pos += 4;
        readError = false;
        return value;
    }

    public long getInt64() {
        curParamIndex++;
        readError = true;
        long value = 0;
        if (pos + 8 > size)
            return 0L;

        value = Stream2I(stream, pos, 8);
        pos += 8;
        readError = false;
        return value;
    }

    public short getUInt8() {
        short value = (short) (0xff & getInt8());
        if (readError)
            return 0;

        return value;
    }

    public int getUInt16() {
        int value = (0xffff & getInt16());
        if (readError)
            return 0;
        return value;
    }

    public long getUInt32() {
        readError = true;
        long value = 0;
        if (pos + 4 > size)
            return 0;
        value = Stream2I(stream, pos, 4);
        pos += 4;
        readError = false;
        return value;
    }

    public boolean getBoolean() {
        byte value = getInt8();
        if (readError)
            return false;
        return 0 == value ? false : true;
    }

    /*
     * 取出带'\0'结尾的gbk编码字符串，转成utf8格式字符串输出
     */
    public String getString() {
        int len = getInt32();
        if (readError)
            return null;

        readError = true;
        if (0 >= len)
            return null;

        String value;
        try {
            value = new String(stream, pos, len - 1, "gbk");
        } catch (UnsupportedEncodingException e) {
            return null;
        }
        pos += len;
        readError = false;
        return value;
    }

    public String getStream() {
        int len = getInt32();
        if (readError)
            return null;

        readError = true;
        if (0 >= len)
            return null;

        String value;
        try {
            value = new String(stream, pos, len, "gbk");
        } catch (UnsupportedEncodingException e) {
            return null;
        }
        pos += len;
        readError = false;
        return value;
    }

    protected void I2Stream(byte[] stream, int pos, long value, int size) {
        if (smallOrder) {
            i2memSmall(stream, pos, value, size);
        } else {
            i2memBig(stream, pos, value, size);
        }
    }

    protected long Stream2I(byte[] stream, int pos, int size) {
        if (smallOrder) {
            return mem2iSmall(stream, pos, size);
        } else {
            return mem2iBig(stream, pos, size);
        }
    }

    //将一个整数存储到字节流buf，按照小端字节序(低位在前，高位在后)
    private void i2memSmall(byte[] stream, int pos, long value, int size) {
        int move = (size - 1) * 8;
        int del = (size - 1) * 8;
        int i = 0;
        for (i = 0; i < size; i++) {
            stream[pos + i] = (byte) ((value << move) >> del);
            move -= 8;
        }

        return;
    }

    //将buf字节流专程整数，按照小端字节序(低位在前，高位在后)
    private long mem2iSmall(byte[] stream, int pos, int size) {
        long value = 0;
        int i = size - 1;
        value += 0xff & stream[pos + i];
        for (i = size - 2; 0 <= i; i--) {
            value <<= 8;
            value += 0xff & stream[pos + i];
        }
        return value;
    }

    //将一个整数存储到字节流buf，按照大端字节序(网络序，高位在前，低位在后)
    private void i2memBig(byte[] stream, int pos, long value, int size) {
        int move = (size - 1) * 8;
        int del = (size - 1) * 8;
        int i = size - 1;
        for (; i >= 0; i--) {
            stream[pos + i] = (byte) (char) ((value << move) >> del);
            move -= 8;
        }
    }

    //将buf字节流专程整数，按照大端字节序(网络序，高位在前，低位在后)
    private long mem2iBig(byte[] stream, int pos, int size) {
        long value = 0;
        int i = 0;
        value += 0xff & stream[pos + i];
        for (i = 1; i < size; i++) {
            value <<= 8;
            value += 0xff & stream[pos + i];
        }
        return value;
    }

    public byte[] Bytes() {
        I2Stream(stream, 0, size, 4);
        pos = 4;
        return stream;
    }

    public int Size() {
        pos = 0;
        size = (int) Stream2I(stream, pos, 4);
        pos += 4;
        if (size <= 4 || size > space) {
            Init();
            return -1;
        }

        return size;
    }

    //参数位置开始
    public void ParamStart() {
        curParamIndex = 0;
        return;
    }

    //失败的参数位置，解析，构造失败时候，用此方法查看是解析到第几个参数时失败的
    public int FailedParam() {
        return curParamIndex;
    }
    
    //将报文转换成二进制信息的字符串
    @Override
    public String toString(){
        I2Stream(stream, 0, size, 4);
        String ascii = "stream(" + size + " byte):";
        int i = 0;
        int value = 0;
        for ( i = 0; i < size; i++ ){
            value = 0xff & stream[i];
            if ( 0 != i ){
                ascii = ascii + ", ";
            }
            ascii = ascii + value;
        }

        return ascii;
    }
 
    //使用二进制字符串，构造报文
    public boolean createFromAscii(String ascii){
        int pos = ascii.indexOf("stream(");
        if (0 != pos) {
            return false;
        }
        pos += 7;
        int posEnd = ascii.indexOf(" byte):", pos);
        if (-1 == pos){
            return false;
        }
        String sizeText = ascii.substring(pos, posEnd);
        size = Integer.valueOf(sizeText).intValue();
        pos = posEnd + 7;

        int i = 0;
        String valueText = null;
        int value;
        for ( i = 0; i < size; i++ ){
            posEnd = ascii.indexOf(", ", pos);
            if ( -1 == posEnd ){
                if ( posEnd >= ascii.length() ) {
                    return false;
                }
                posEnd = ascii.length();
            }
            valueText = ascii.substring(pos, posEnd);
            value = Integer.valueOf(valueText).intValue();
            stream[i] = (byte)(0xff & value);
            pos = posEnd + 2;
        }
        
        return true;
    }
    
}
