<?php require_once("../../../sdk/php/Protocl/Protocl.php");?>
<?php

class UserRegister extends Message
{
  //请求参数
  public $connectId;//链接Id	Tcp服务填写(回应必填)
  public $isGuest;//游客进入(TCP接入服填�?
  public $objectId;//m_isGuest=true 游客Id m_isGuest=false用户id(TCP接入服填�?
  public $ip;     //ip地址
  public $code;//结果�?0成功 >0失败
  public $reason;//失败原因(成功时不填写)
  
  public $accountType;//账号类型(回应失败不填�? 参�?c++ UserRegister类�?
  public $account;//账号	账号/手机/qq号码/email(回应失败不填�?
  public $pwd;//密码	md5(明文)(回应失败不填�?
  public $city;// 城市
  //回应参数
  public $userId;//用户id(失败时�?不填�?

  public function __construct()
  {
    parent::__construct();
    $this->BindSetting(Protocl::$byteOrder, Moudle::$Auth);
    $this->code = 0;
    $this->city = "Beijing";
  }

  public function __destruct()
  {
  }

  //构�?报文，发�?isResult=true构�?回应报文
  public function Build( $isResult = false )
  {
	$this->SetId(MsgId::$userRegister, $isResult);
	$this->AddInt64($this->connectId);//链接Id	Tcp服务填写(回应必填)
	$this->AddInt8($this->isGuest);//游客进入(TCP接入服填�?
	$this->AddInt32($this->objectId);//m_isGuest=true 游客Id m_isGuest=false用户id(TCP接入服填�?
        $this->AddInt32(0);//ip地址
	$this->AddInt8($this->code);
	
	$this->AddInt8($this->accountType);//账号类型(回应失败不填�?
	$this->AddString($this->account);//账号	账号/手机/qq号码/email(回应失败不填�?
	$this->AddString($this->pwd);//密码	md5(明文)(回应不填�?
        $this->AddString($this->city);// 城市
  }

  //解析解析，接�?
  function Parse()
  {
      $this->ReadHeader();
	  if ( !$this->GetInt64($this->connectId) ) return false;//链接Id	Tcp服务填写(回应必填)
	  if ( !$this->GetInt8($this->isGuest) ) return false;//游客连接(TCP接入服填�?
	  if ( !$this->GetInt32($this->objectId) ) return false;//m_isGuest=true 游客Id m_isGuest=false用户id(TCP接入服填�?
          if ( !$this->GetInt32($this->ip) ) return false;//ip地址
	  if ( !$this->GetInt8($this->code) ) return false;//结果�?0成功 >0失败
	  if($this->code != 0){
	  	if ( !$this->GetString($this->reason) ) return false;//失败原因(成功时不填写)
	  }
	  if ( !$this->GetInt8($this->accountType) ) return false;//账号类型(回应失败不填�?
	  if ( !$this->GetString($this->account) ) return false;//账号	账号/手机/qq号码/email(回应失败不填�?
	  if ( !$this->GetString($this->pwd) ) return false;//密码	md5(明文)(回应不填�?
          if ( !$this->GetString($this->city) ) return false;// 城市
		  
	  if (!$this->IsResult() || 0 != $this->code ) return true;//�?�� �?���?��解析回应参数
	  //回应参数
 	  if ( !$this->GetInt32($this->userId) ) return false;//用户id(失败时�?不填�?
	  
	  return true;
  }

}
?>