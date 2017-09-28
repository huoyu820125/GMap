package com.huoyu.gmap.svr.gsrc.protocl;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.huoyu.client.MsgDefaultParam;
import com.huoyu.client.NetLine;
import com.huoyu.client.NodeAddress;
import com.huoyu.client.ResultCode;
import com.huoyu.gmap.svr.gsrc.constant.ServiceType;

public class MsgGetCluster extends MsgDefaultParam {

    //请求参数
    public ServiceType                  serviceType; //要取的服务类型，对应c++ int32   
    public int                          start;      //从第start个服务开始

    // 回应参数
    public Map<NetLine, List<NodeAddress>> cluster; //服务列表：map<线路, vector<结点>>

    public MsgGetCluster() {
        super(10240, true, (short)ServiceType.GSRC.value());
    }

    @Override
    public boolean Build(boolean isResult) {
        if ( isResult ) return false;//不创建回应报文
    
        SetId((short) ProtoclNo.GET_CLUSTER.value(), isResult);
        defaultParam();
        if (!addInt32(serviceType.value())) {//要取的服务类型，对应c++ int32 
            return false;
        }
        if (!addInt32(start)) {//从开始位置获取服务信息
            return false;
        }

        return true;
    }

    @Override
    public boolean Parse() {
        if ( !super.Parse() ) return false;
        if ( !isResult() ) return false;//不解析请求报文

        serviceType = ServiceType.toEnum(getInt32());//要取的服务类型，对应c++ int32 
        if (readError) {
            return false;
        }
        start = getInt32();//从开始位置获取服务信息
        if (readError) {
            return false;
        }

        // 回应参数
        if (!isResult()) {
            return false;
        }
        if (ResultCode.SUCCESS.value() != code) {
            return true;
        }

        //服务列表
        byte lineCount = getInt8();//几条线路
        if (readError) {
            return false;
        }
        if (4 < lineCount || 0 > lineCount) {
            return false;
        }

        int i = 0;
        int nodeCount;//节点数量
        int nodeId = 0;//节点id
        String ip = null;//ip
        int port = 0;//端口
        NetLine line = null;
        cluster = new HashMap<NetLine, List<NodeAddress>>();
        for ( i = 0; i < lineCount; i++ ){
            nodeCount = getUInt8();//线路，临时用一下，下面取节点数量就修正了
            if (readError) {
                return false;
            }
            line = NetLine.toEnum(nodeCount);

            nodeCount = getUInt8();
            if (readError) {
                return false;
            }
            if ( 0 >= nodeCount ){
                return false;
            }
            
            int j = 0;
            List<NodeAddress> services = new ArrayList<NodeAddress>();
            for (j = 0; j < nodeCount; j++) {
                nodeId = getInt32();//节点id
                if (readError) {
                    return false;
                }
                ip = getString();//ip
                if (readError) {
                    return false;
                }
                port = getInt32();//端口
                if (readError) {
                    return false;
                }
                services.add(new NodeAddress(nodeId, ip, port));
            }
            cluster.put(line, services);
        }

        return true;
    }
}
