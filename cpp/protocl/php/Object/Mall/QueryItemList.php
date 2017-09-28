<?php require_once("../../../sdk/php/Protocl/Protocl.php"); ?>
<?php

class QueryItemList extends Message {

    //请求参数
    public $connectId; //链接Id	Tcp服务填写(回应必填)
    public $isGuest; //游客进入(TCP接入服填�?
    public $objectId; //m_isGuest=true 游客Id m_isGuest=false用户id(TCP接入服填�?
    public $ip;     //ip地址
    public $code; //结果�?0成功 >0失败
    public $reason; //失败原因(成功时不填写)
    
    public $m_itemBigType; //商品大类�?ItemBigType,全部，礼物，道具，特殊礼�?
    public $m_startPos; //�?��查看位置，从那个商品id�?��32
    public $m_count; //数量，一共读取的数量16

    public function __construct() {
        parent::__construct();
        $this->BindSetting(Protocl::$byteOrder, Moudle::$Mall);
        $this->code = 0;

    }

    public function __destruct() {
        
    }

    //构�?报文，发�?isResult=true构�?回应报文
    public function Build($isResult = false) {
        $this->SetId(MsgId::$queryItemList, $isResult);
        $this->AddInt64($this->connectId); //链接Id	Tcp服务填写(回应必填)
        $this->AddInt8($this->isGuest); //游客进入(TCP接入服填�?
        $this->AddInt32($this->objectId); //m_isGuest=true 游客Id m_isGuest=false用户id(TCP接入服填�?
        $this->AddInt32(0); //ip地址
        $this->AddInt8($this->code);

        $this->AddInt8($this->m_itemBigType); //商品大类�?回应失败不填�?
        $this->AddInt32($this->m_startPos); //从那个商品开�?
        $this->AddInt16($this->m_count); // 数量
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

        if (!$this->IsResult() || 0 != $this->code) return true; //�?�� �?���?��解析回应参数
        return true;
    }

}

?>