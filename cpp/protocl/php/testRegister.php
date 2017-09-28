<?php require_once("base/Socket.php");?>
<?php require_once("Object/Auth/UserRegister.php");?>

<?php

function FUserRegister($accountType, $account, $pwd, &$userId, &$reason)
{
      $client = new TcpClient();
      $client->Connect("192.168.11.109", 6001);
      $msg = new UserRegister();
      $msg->connectId = 0;//链接Id	Tcp服务填写(回应必填)
      $msg->accountType = $accountType;//账号类型(回应失败不填写) 参考c++ UserRegister类
      $msg->account = $account;//账号	账号/手机/qq号码/email(回应失败不填写)
      $msg->pwd = $pwd;//密码	md5(明文)(回应失败不填写)
      $msg->Build();
      $client->Send($msg->Data(), $msg->Size());
      $client->Recv($msg->Data(), $msg->HeaderSize(), false);
      if ( 0 > $msg->Size() ) //已经可以返回报文总长度，包含报文头,< 0表示报文头非法
      {
        echo "非法长度";
        return;
      }
      $client->Recv($msg->Data(), $msg->Size());//接收完整报文，保存到msg中
      if ( !$msg->Parse() )
      {
          echo "无法解析结果";
          return;
      }
      if(MsgId::$userRegister != $msg->Id() || !$msg->IsResult())
      {
      	return;
      }
      
#      echo "connectId".$msg->connectId.'\r\n';//链接Id	Tcp服务填写(回应必填)
#      echo "accountType".$msg->accountType.'\r\n';//账号类型(回应失败不填写) 参考c++ UserRegister类
#      echo "account".$msg->account.'\r\n';//账号	账号/手机/qq号码/email(回应失败不填写)
#      echo "pwd".$msg->pwd.'\n';//密码	md5(明文)(回应失败不填写)
#      echo "code".$msg->code.'\n';//结果码 0成功 >0失败
#      echo "reason".$msg->reason.'\n';//失败原因(成功时不填写)
#      echo "userId".$msg->userId.'\n';//用户id(失败时候不填写)
      $userId = $msg->userId;
      if ( 0 != $msg->code ) 
      {
        $reason = $msg->reason;
        return false;
      }
      
      return true;
}

$userId;
$reason;
FUserRegister(2, "sunqian", "123456", $userId, $reason);

?>

