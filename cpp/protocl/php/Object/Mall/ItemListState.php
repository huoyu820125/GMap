<?php require_once("../../../sdk/php/Protocl/Protocl.php"); ?>
<?php

class ItemListState extends Message {

    //请求参数
    public $connectId; //链接Id	Tcp服务填写(回应必填)
    public $isGuest; //游客进入(TCP接入服填�?
    public $objectId; //m_isGuest=true 游客Id m_isGuest=false用户id(TCP接入服填�?
    public $ip;     //ip地址
    public $code; //结果�?0成功 >0失败
    public $reason; //失败原因(成功时不填写)
    public $m_start;  //状�?�?�� 结束

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

        if (!$this->GetInt8($this->m_start))
            return false; //状�?不对(回应失败不填�?

        if (!$this->IsResult() || 0 != $this->code)
            return true; //�?�� �?���?��解析回应参数

        return true;
    }

}

?>