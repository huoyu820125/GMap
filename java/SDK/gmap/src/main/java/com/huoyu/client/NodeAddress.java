package com.huoyu.client;



/**
 * 服务节点地址
 * 
 * @author 孙尚天
 * @version $Id: NodeAddress.java, v 0.1 2016年11月29日 下午8:03:57 孙尚天 Exp $
 */
public class NodeAddress extends Object implements Comparable<NodeAddress> {
    private int  nodeId = 0;
    private String ip     = null;
    private int    port   = 0;

    public int compareTo(NodeAddress arg0) {
        Integer tid = nodeId;
        Integer aid = arg0.nodeId;
        return tid.compareTo(aid);//升序
    }

    public NodeAddress(int nodeId, String ip, int port) {
        this.nodeId = nodeId;
        this.ip = ip;
        this.port = port;
    }

    public NodeAddress(NodeAddress obj) {
        this.nodeId = obj.nodeId;
        this.ip = obj.ip;
        this.port = obj.port;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }
        if (null == obj || getClass() != obj.getClass()) {
            return false;
        }

        NodeAddress nodeAddress = (NodeAddress) obj;
        //结点id不能重复，ip端口也不能重复，重复则视为相同
        if (nodeId == nodeAddress.nodeId 
            ||(ip.equals(nodeAddress.ip) && port == nodeAddress.port)) {
            return true;
        }

        return false;
    }

    @Override
    public int hashCode() {
        return port + (ip != null ? ip.hashCode() : 0);
    }

    public int getNodeId() {
        return nodeId;
    }

    public void setNodeId(int nodeId) {
        this.nodeId = nodeId;
    }

    public String getIp() {
        return ip;
    }

    public void setIp(String ip) {
        this.ip = ip;
    }

    public int getPort() {
        return port;
    }

    public void setPort(int port) {
        this.port = port;
    }

}
