<?php require_once("base/Message.php");?>

<?php

/*
    通信协议接口类
    接收报文
      $msg = new SmsMsg();
      socket_recv(sock, $msg->Data(), $msg->HeaderSize(), false);//接收报文头，保存到msg中,不清除tcp缓冲
      if ( 0 > $msg->Size() ) //已经可以返回报文总长度，包含报文头,< 0表示报文头非法
      {
        错误处理
        return;
      }
      $client->Recv($msg->Data(), $msg->Size());//接收完整报文，保存到msg中
      if ( SmsMsg::$publish == msg.MsgId() )//检查协议号
      {
        if ( !$msg->PublishResultParam($serialNo, $reason) )//取出发布请求报文的参数
        {
          回应的报文格式无法解析
          return;
        }
        做业务
      }

    发送请求报文
      $msg = new SmsMsg();
      $msg->CreatePublish(...);//将发布请求数据压入报文
      socket_send($sock, $msg->Data(), $msg->Size(), ...);//发送报文

    业务通信流程参考NewsClient.php
*/

class ExplameMsg extends Message
{
  public static $freeSMS            = 10001;//自定义短信

  public function __construct()
  {
    parent::__construct();
    $this->BindSetting(Stream::$smallOrder, 6);
  }

  public function __destruct()
  {
  }

  /*
    创建自由格式短信
    $type 短信模块编号 string
          模板需要到运营商网站注册，目前已经注册验证码模板编号"3298"
    $phone  接收号码 string 
    $params 参数自定义参数 string数组 模板中定义的参数，      
      3298模板需要2个参数
      code  验证码 string gbk编码，最少个5字符
      time  时效  string 单位分钟
  */
  public function CreateFreeSMS( $type, $phone, $params )
  {
    $this->SetId(SmsMsg::$freeSMS);
    $this->AddStream( $type, strlen($type) );
    $this->AddStream( $phone, strlen($phone) );
    $this->AddInt32( count($params) );
    $i = 0;
    for ( $i = 0; $i < count($params); $i++ )
    {
      $this->AddStream( $params[$i], strlen($params[$i]) );
    }

    return;
  }

  public function FreeSMSParam( &$res, &$reason )
  {
      $this->pos = $this->HeaderSize();
      if ( !$this->GetInt16($res) ) return "1";
      if ( !$this->GetStream($reason, $size) ) return "5";
      return 0;
  }
}

?>

