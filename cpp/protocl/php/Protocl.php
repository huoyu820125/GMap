<?php require_once("base/Message.php");?>
<?php

class Protocl
{
  public static $maxMsgSize = 10240;//报文最大长度
  public static $byteOrder = 2;//Stream::$smallOrder;//字节序
}

class Moudle {

    public static $all = 0;   //所有模块
    public static $Auth = 1; //认证模块
    public static $Room = 2; //互动模块
    public static $Mall = 3; //商城模块
    public static $Notify = 4;  //广播模块
    public static $Operation = 5;  //运维模块
    public static $TcpEntry = 6;  //Tcp入口
    public static $DBEntry = 7;  //数据中心
    public static $HistoryEntry = 8; //历史入口

}

class MsgId
{
		//认证模块
		public static $userRegister = 1001;//注册
		public static $userLogin = 1002;//登录
		//互动模块
		public static $enterRoom = 1001;//进入直播间
		public static $roomInfo = 2001;//直播间信息
		public static $lookerListState = 2002;//直播间观众列表开始/结束
		public static $lookerPart = 2003;//直播间观众列表片段
		public static $outRoom = 1002;//离开直播间
		public static $showStart = 1003;//开播
		public static $showEnd = 1004;//结束直播
		public static $tickOutRoom = 1005;//踢人
		public static $banTalk = 1006;//禁言/解禁
		public static $roomFrozen = 1007;//冻结/解冻房间
		public static $useItem = 1008;//使用道具
		public static $talk = 1009;//聊天
		//商城模块
		public static $queryItemList = 1001;//商品列表
		public static $itemListState = 2001;//商品列表开始/结束
		public static $itemInfo = 2002;//单个商品信息
		public static $buyItem = 1002;//购买商品
}
class ClientType
{
		//认证模块
		public static $web = 0;//
		public static $flash = 1;//
		//互动模块
		public static $android = 2;//
		public static $iphone = 3;//
		public static $robot = 4;//机器人
		
}


?>

