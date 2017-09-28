<?php require_once("../../../sdk/php/Protocl/Protocl.php"); ?>
<?php

class ItemInfo extends Message {

//请求参数
public $connectId; //链接Id	Tcp服务填写(回应必填)
public $isGuest; //游客进入(TCP接入服填�?
public $objectId; //m_isGuest=true 游客Id m_isGuest=false用户id(TCP接入服填�?
public $ip;     //ip地址
public $code; //结果�?0成功 >0失败
public $reason; //失败原因(成功时不填写)

public $m_itemId; //商品id int
public $m_itemBigType; //商品大类�?char
public $m_itemType; //商品类型 char
public $m_level_limit; //会员等级
public $m_name ;     //商品名称
public $m_effect; //商品作用  std::string
public $m_info; //商品信息 std::string
public $m_time; //商品有效�?单位分钟�?无限�?int
public $m_coin; //标价 int
public $m_ico; //图片Id int
public $m_icoName; //图片名称 std::string
public $m_icoType; //图片类型 0：礼物图�?1：表情图�?2：道具图�?3：首页轮播图 4：直播间背景�?, int
public $m_icoAddress; //图片地址 std::string


public function __construct() {
parent::__construct();
$this->BindSetting(Protocl::$byteOrder, Moudle::$DBEntry);
$this->code = 0;

}

public function __destruct() {

}

//解析解析，接�?
function Parse() {
$this->ReadHeader();
$size;
if (!$this->GetInt64($this->connectId))
return false; //链接Id	Tcp服务填写(回应必填)
if (!$this->GetInt8($this->isGuest))
return false; //游客连接(TCP接入服填�?
if (!$this->GetInt32($this->objectId))
return false; //m_isGuest=true 游客Id m_isGuest=false用户id(TCP接入服填�?
if (!$this->GetInt32($this->ip))
return false; //ip地址
if (!$this->GetInt8($this->code))
return false; //结果�?0成功 >0失败
if ($this->code != 0) {
if (!$this->GetString($this->reason))
return false; //失败原因(成功时不填写)
}

if ( !$this->GetInt32($this->m_itemId) ) return false;//商品id int
if ( !$this->GetInt8($this->m_itemBigType) ) return false;//商品大类�?char
if ( !$this->GetInt8($this->m_itemType) ) return false;//商品类型 char
if ( !$this->GetInt16($this->m_level_limit) ) return false;//商品类型 char
if ( !$this->GetString($this->m_name) ) return false;//商品名称  std::string
if ( !$this->GetString($this->m_effect) ) return false;//商品作用  std::string
if ( !$this->GetString($this->m_info) ) return false;//商品信息 std::string
if ( !$this->GetInt32($this->m_time) ) return false;///商品有效�?单位分钟�?无限�?int
if ( !$this->GetInt32($this->m_coin) ) return false;//标价 int
if ( !$this->GetInt32($this->m_ico) ) return false;//图片Id int
if ( !$this->GetString($this->m_icoName) ) return false;//图片名称 std::string
if ( !$this->GetInt32($this->m_icoType) ) return false;//图片类型 0：礼物图�?1：表情图�?2：道具图�?3：首页轮播图 4：直播间背景�?, int
if ( !$this->GetString($this->m_icoAddress) ) return false;//图片地址 std::string

if (!$this->IsResult() || 0 != $this->code) return true; //�?�� �?���?��解析回应参数
return true;
}

}
?>