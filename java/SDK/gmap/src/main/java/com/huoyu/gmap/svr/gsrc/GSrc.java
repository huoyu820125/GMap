package com.huoyu.gmap.svr.gsrc;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.huoyu.client.CallResult;
import com.huoyu.client.NetConnect;
import com.huoyu.client.NetLine;
import com.huoyu.client.NodeAddress;
import com.huoyu.client.ResultCode;
import com.huoyu.gmap.svr.gsrc.constant.ServiceType;
import com.huoyu.gmap.svr.gsrc.protocl.MsgGetCluster;
import com.huoyu.gmap.svr.gsrc.protocl.MsgNewClientId;

//访问图数据库集群配置服务的SDK对象
public class GSrc extends NetConnect{
    public GSrc()
    {
        
    }
    
    public CallResult<List<NodeAddress>> GetCluster(int millSecond)
    {
        List<NodeAddress> cluster = new ArrayList<NodeAddress>();
        Map<Integer, Boolean> idMap = new HashMap<Integer, Boolean>();
        MsgGetCluster msg = new MsgGetCluster();
        msg.serviceType = ServiceType.GMAP;
        msg.start = 1;
        ResultCode ret = null;
        while ( true )
        {
            ret = query(msg, millSecond);
            if ( ResultCode.TIME_OUT == ret )
            {
                return new CallResult<List<NodeAddress>>(ret, "集群配置服务超时" + millSecond + "毫秒");
            }
            if ( ResultCode.NO_SERVER == ret )
            {
                return new CallResult<List<NodeAddress>>(ret.value(), "集群配置服务不可用");
            }
            if ( ResultCode.SUCCESS != ret ) 
            {
                return new CallResult<List<NodeAddress>>(ret);
            }
            if ( 0 == msg.cluster.size() ) break;
            //复制服务数据
            for (Map.Entry<NetLine, List<NodeAddress>> entry: msg.cluster.entrySet()){
                if ( NetLine.LOCAL != entry.getKey() ) continue;//不是内网线路直接忽略
                List<NodeAddress> services = entry.getValue();
                for ( NodeAddress node : services ){
                    if ( idMap.containsKey(node.getNodeId())) continue;
                    idMap.put(node.getNodeId(), true);
                    cluster.add(node);
                }
                msg.start += services.size();
            }
            msg.cluster.clear();
        }
        if ( 0 == cluster.size() ){
            return new CallResult<List<NodeAddress>>(ResultCode.UN_INIT, "数据节点未配置");
        }
        Collections.sort(cluster);
        int i = 0;
        int nextNodeId = 1;
        for ( i = 0; i < cluster.size(); i++ )
        {
            if ( cluster.get(i).getNodeId() != nextNodeId ) 
            {
                return new CallResult<List<NodeAddress>>(ResultCode.UN_INIT, "数据节点(" + nextNodeId + ")未配置");
            }
            nextNodeId++;
        }
        return new CallResult<List<NodeAddress>>(cluster);
    }

    public CallResult<Long> NewClientId(int millSecond)
    {
        MsgNewClientId  msg = new MsgNewClientId();
        ResultCode ret = query(msg, millSecond);
        if ( ResultCode.TIME_OUT == ret )
        {
            return new CallResult<Long>(ret, "集群配置服务超时" + millSecond + "毫秒");
        }
        if ( ResultCode.NO_SERVER == ret )
        {
            return new CallResult<Long>(ret, "集群配置服务不可用");
        }
        if ( ResultCode.SUCCESS != ret ) 
        {
            return new CallResult<Long>(ret);
        }
        return new CallResult<Long>(msg.m_clientId);
    }

}
