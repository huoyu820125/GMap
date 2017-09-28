package com.huoyu.client;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.net.ConnectException;
import java.net.InetSocketAddress;
import java.net.NoRouteToHostException;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;
import java.net.UnknownServiceException;

import com.huoyu.client.msg.Message;

/**
 * 网络连接
 * 封装Message收发方法
 * @author 孙尚天
 * @version $Id: NetConnect.java, v 0.1 2016年11月23日 下午9:12:00 孙尚天 Exp $
 */
public class NetConnect {
    private Socket        tcpConnect     = null;
    private BufferedOutputStream writer = null;
    private BufferedInputStream reader = null;
    private int           sendTimeOut    = 0;
    private int           recvTimeOut    = 0;
    private int           nodeId = 0;
    private String        serviceIP             = null;
    private int           servicePort           = 0;
    private SocketAddress serviceAddress = null;

    public NetConnect() {
        tcpConnect = new Socket();
    }

    //是否连接
    public boolean isConnect() {
        return tcpConnect.isConnected();
    }

    //设置服务节点id,ip,port
    public ResultCode setService(int nodeId, String ip, int port) {
        try {
            this.nodeId = nodeId;
            serviceAddress = new InetSocketAddress(ip, port);
            this.serviceIP = ip;
            this.servicePort = port;
        } catch (Exception e) {
            serviceAddress = null;
            return ResultCode.IP_ERROR;
        }

        return ResultCode.SUCCESS;
    }

    //发送请求，并接收回应
    public ResultCode query(MsgDefaultParam msg, int millSecond)
    {
        ResultCode result = sendMsg(msg, millSecond);
        if ( ResultCode.SUCCESS != result ) return result;

        return recvMsg(msg, millSecond);
    }

    //使用上次连接的ip，连接服务，一直等待
    public ResultCode connect() {
        return connect(0);
    }

    //使用上次连接的ip，连接服务，最多等待timeOut毫秒, timeOut<=0时一直等待
    public ResultCode connect(int timeOut) {
        if (null == serviceAddress) {
            return ResultCode.UN_INIT;
        }
        if (0 > timeOut) {
            timeOut = 0;
        }

        try {
            tcpConnect.connect(serviceAddress, timeOut);
            tcpConnect.setTcpNoDelay(true);
        } catch (UnknownHostException e) {
            tcpConnect = new Socket();//必须重新new，api不支持重用
            return ResultCode.IP_ERROR;
        } catch (ConnectException e) {
            tcpConnect = new Socket();//必须重新new，api不支持重用
            return ResultCode.NO_SERVER;
        } catch (SocketTimeoutException e) {
            tcpConnect = new Socket();//必须重新new，api不支持重用
            return ResultCode.TIME_OUT;
        } catch (IOException e) {
            tcpConnect = new Socket();//必须重新new，api不支持重用
            return ResultCode.NO_SERVER;
        }

        //绑定接收流/发送流，不能每次接收/发送时再
        try {
            reader = new BufferedInputStream(tcpConnect.getInputStream());
            writer = new BufferedOutputStream(tcpConnect.getOutputStream());
        } catch (IOException e) {
            return ResultCode.SYS_ERROR;
        }

        return ResultCode.SUCCESS;
    }

    /**
     * @Title:从连接上接收数据，阻塞模式
     * @Description:
     * @param connect 连接
     * @param stream 保存接收到的数据
     * @param pos   从流中的指定位置开始存入接收到的数据
     * @param size  期望接收的长度，如果数据不够，则阻塞，直达数据达到或者连接断开
     * @return
     * @date:2016年11月23日 下午6:51:22
     * @author:孙尚天
     */
    public ResultCode recv(BufferedInputStream connect, byte[] stream, int pos, int size) {
        int readSize = 0;
        while (true) {
            try {
                readSize = connect.read(stream, pos, size);
            } catch (SocketTimeoutException e) {
                return ResultCode.TIME_OUT;
            } catch (ConnectException e) {
                return ResultCode.NO_SERVER;
            } catch (UnknownHostException e) {
                return ResultCode.NO_SERVER;
            } catch (UnknownServiceException e) {
                return ResultCode.NO_SERVER;
            } catch (NoRouteToHostException e) {
                return ResultCode.NO_SERVER;
            } catch (IOException e) {
                return ResultCode.NO_SERVER;
            }
            if (0 >= readSize) {
                return ResultCode.NO_SERVER;
            }

            if (readSize < size) {
                pos += readSize;
                size -= readSize;
                continue;
            }
            break;
        }

        return ResultCode.SUCCESS;
    }

    public ResultCode send(BufferedOutputStream connect, byte[] stream, int pos, int size) {
        try {
            connect.write(stream, pos, size);
            connect.flush();
        } catch (SocketTimeoutException e) {
            return ResultCode.TIME_OUT;
        } catch (ConnectException e) {
            return ResultCode.NO_SERVER;
        } catch (UnknownHostException e) {
            return ResultCode.NO_SERVER;
        } catch (UnknownServiceException e) {
            return ResultCode.NO_SERVER;
        } catch (NoRouteToHostException e) {
            return ResultCode.NO_SERVER;
        } catch (IOException e) {
            return ResultCode.NO_SERVER;
        }

        return ResultCode.SUCCESS;
    }

    public void close() {
        if (!tcpConnect.isConnected())
            return;
        try {
            tcpConnect.close();
        } catch (IOException e) {
        }
        tcpConnect = new Socket();//必须重新new，api不支持重用
    }

    /**
     * 
     * @Title:接收完整的报文，阻塞模式知道接收完成，或连接断开，或者超时
     * @Description:
     * @param msg   保存接收到的报文
     * @param timeOut   超时时间（毫秒） 此参数<=0时一直等待
     * @return
     * @date:2016年11月23日 下午6:55:22
     * @author:孙尚天
     */
    public ResultCode recvMsg(MsgDefaultParam msg, int timeOut) {
        if (null == msg) {
            return ResultCode.PARAM_ERROR;
        }
        if (0 > timeOut) {
            timeOut = 0;
        }

        try {
            if (timeOut != recvTimeOut) {
                recvTimeOut = timeOut;
                tcpConnect.setSoTimeout(recvTimeOut);
            }
        } catch (IOException e) {
            return ResultCode.SYS_ERROR;
        }
        ResultCode result = recv(reader, msg.Bytes(), 0, msg.HeaderSize());
        if (ResultCode.SUCCESS != result) {
            close();//接收失败必须断开连接，以免存在垃圾数据，破坏下次请求
            return result;
        }
        if (-1 == msg.Size()) {
            return ResultCode.MSG_FORMAT_ERROR;
        }

        result = recv(reader, msg.Bytes(), msg.HeaderSize(), msg.Size() - msg.HeaderSize());
        if (ResultCode.SUCCESS != result) {
            close();//接收失败必须断开连接，以免存在垃圾数据，破坏下次请求
            return result;
        }
        if ( !msg.Parse() ) 
        {
            close();//解析失败必须断开连接，以免存在垃圾数据，破坏下次请求
            return ResultCode.MSG_FORMAT_ERROR;
        }

        return ResultCode.toEnum(msg.code);
    }

    //接收数据，一直等待
    public ResultCode recvMsg(MsgDefaultParam msg) {
        return recvMsg(msg, 0);
    }

    /**
     * 
     * @Title:发送报文
     * @Description:
     * @param msg       要发送的报文
     * @param timeOut 超时时间（毫秒） 此参数<=0时一直等待
     * @return
     * @date:2016年11月23日 下午8:22:57
     * @author:孙尚天
     */
    public ResultCode sendMsg(Message msg, int timeOut) {
        //参数检查
        if (null == msg) return ResultCode.PARAM_ERROR;
        if (0 > timeOut) timeOut = 0;

        //检查连接
        ResultCode result = null;
        if ( !isConnect() )
        {
            this.close();
            result = connect(1000);
            if ( ResultCode.SUCCESS != result ) return ResultCode.NO_SERVER;
        }
        //构造报文
        if ( !msg.Build(false) ) return ResultCode.PARAM_ERROR;

        //发送报文
        try {
            if (timeOut != sendTimeOut) {
                sendTimeOut = timeOut;
                tcpConnect.setSoTimeout(sendTimeOut);
            }
        } catch (IOException e) {
            return ResultCode.SYS_ERROR;
        }
        result = send(writer, msg.Bytes(), 0, msg.Size());
        if (ResultCode.SUCCESS != result) {
            close();
            return result;
        }

        return ResultCode.SUCCESS;
    }

    //发送报文，一直等待发送完成
    public ResultCode sendMsg(Message msg) {
        return sendMsg(msg, 0);
    }
    
    public int getNodeId() {
        return nodeId;
    }

    public void setNodeId(int nodeId) {
        this.nodeId = nodeId;
    }

    public String getServiceIP() {
        return serviceIP;
    }

    public void setServiceIP(String serviceIP) {
        this.serviceIP = serviceIP;
    }

    public int getServicePort() {
        return servicePort;
    }

    public void setServicePort(int servicePort) {
        this.servicePort = servicePort;
    }

    
}
