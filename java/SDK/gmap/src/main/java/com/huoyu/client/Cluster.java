package com.huoyu.client;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 服务器集群
 * 封装路由算法
 * 
 * @author 孙尚天
 * @version $Id: Cluster.java, v 0.1 2016年11月28日 下午9:28:27 孙尚天 Exp $
 */
public class Cluster {

    private Map<NodeAddress, NetConnect> clients            = null; //路由服务连接
    private int                          nextNodeIndex = 0;   //轮询算法，下次使用节点的游标

    public Cluster() {
        clients = new HashMap<NodeAddress, NetConnect>();
        nextNodeIndex = 0; //默认下次使用第一个节点
    }

    //初始化数据集群
    public CallResult<Void> init(List<NodeAddress> cluster) {
        if ( 0 == cluster.size() ) return new CallResult<Void>(ResultCode.UN_INIT, "图数据库集群连接信息未配置");
        int i = 0;
        ResultCode rc;
        for ( i = 0; i < cluster.size(); i++ )
        {
            NodeAddress node = cluster.get(i);
            rc = addNode(node.getNodeId(), node.getIp(), node.getPort());
            if ( ResultCode.SUCCESS != rc ) return new CallResult<Void>(ResultCode.NO_SERVER, "图数据库节点" + node.getNodeId() + "不可用");
        }

        return new CallResult<Void>();
    }

    //添加服务节点
    private ResultCode addNode(int nodeId, String ip, int port) {
        NodeAddress adr = new NodeAddress(nodeId, ip, port);
        if (clients.containsKey(adr)) {
            return ResultCode.SUCCESS;
        }
        NetConnect service = new NetConnect();
        ResultCode result = service.setService(nodeId, ip, port);
        if (ResultCode.SUCCESS != result)  return ResultCode.IP_ERROR;
        if ( ResultCode.SUCCESS != service.connect(1000) ) return ResultCode.NO_SERVER;
        clients.put(adr, service);

        return ResultCode.SUCCESS;
    }

    //结点数量
    public int nodeCount(){
        return clients.size();
    }
    
    //轮询服务节点拿到可用连接
    public NetConnect routeNode() {
        NetConnect serviceNode;
        int i = 0;
        //轮询lastClientIndex~size寻找可用服务
        for (Map.Entry<NodeAddress, NetConnect> entry : clients.entrySet()) {
            if (i < nextNodeIndex) {//遍历到被使用的节点
                i++;
                continue;
            }
            i++;//下次使用的节点游标

            serviceNode = entry.getValue();
            if (!serviceNode.isConnect()) {//确保节点可访问
                if (ResultCode.SUCCESS != serviceNode.connect(10)) {
                    continue;
                }
            }

            //记录下次使用的节点游标
            nextNodeIndex = i;
            if (nextNodeIndex == clients.size()) nextNodeIndex = 0;

            return serviceNode;
        }

        //nextNodeIndex~size-1之间没有找到可用服务
        //从头寻找，轮询0~nextNodeIndex-1之间的服务
        i = 0;
        for (Map.Entry<NodeAddress, NetConnect> entry : clients.entrySet()) {
            if (i == nextNodeIndex) break;//停止轮询
            i++;

            serviceNode = entry.getValue();
            if (!serviceNode.isConnect()) {//确保节点可访问
                if (ResultCode.SUCCESS != serviceNode.connect(10)) {
                    continue;
                }
            }

            //记录下次使用的节点游标
            nextNodeIndex = i;
            if (nextNodeIndex == clients.size()) nextNodeIndex = 0;

            return serviceNode;
        }

        //没有可用服务
        return null;
    }

    //按照值定位服务节点
    public NetConnect findNode(long value){
        int nodeId = findNodeId(value);
        for (Map.Entry<NodeAddress, NetConnect> entry : clients.entrySet()) {
            if ( nodeId != entry.getKey().getNodeId() ) continue;
            
            return entry.getValue();
        }
        
        return null;//init方法确保了clients中一定可以找到，不可能执行到这一行
    }
    
    //按照值定位服务节点Id
    public int findNodeId(long value){
        //init方法确保了clients.size()>0,不做检查
        int nodeId = (int)((value % clients.size()) + 1);
        
        return nodeId;
    }
    
    protected NetConnect GetNode(int nodeId){
        for (Map.Entry<NodeAddress, NetConnect> entry : clients.entrySet()) {
            if ( nodeId != entry.getKey().getNodeId() ) continue;
            
            return entry.getValue();
        }
        
        return null;
    }
    
    //断开连接
    public void CloseConnect(){
        NetConnect serviceNode = null;
        for (Map.Entry<NodeAddress, NetConnect> entry : clients.entrySet()) {
            serviceNode = entry.getValue();
            serviceNode.close();
        }
    }
}
