<?php require_once("Stream.php");?>

<?php
class Message extends Stream
{
  var $moudleId;
  var $id;
  var $isResult;
  var $headerIsReaded;
  
	function __construct()
	{
    parent::__construct();
    $this->moudleId = -1;
    $this->id = -1;
    $this->isResult = -1;
    $this->headerIsReaded = false;
	}

  //报文头长度
	function HeaderSize()
  {
    return 	4 + 1 + 2 + 1;
  }
  
    function Memcpy($msg)
    {
        $this->Clear();
        $this->moudleId = -1;
        $this->id = -1;
        $this->isResult = -1;
        $this->headerIsReaded = false;
        for ($i = 0; $i < $msg->Size(); $i++) {
            $c = ord( $msg->Data()[$i]);
            $this->stream .= chr($c);
//            echo"<br>byte".$c;
        }
        

    }
    
    function Init()
    {
      $this->Clear();
      $this->moudleId = -1;
      $this->id = -1;
      $this->isResult = -1;
      $this->headerIsReaded = false;
    }
  function ReadHeader()
  {
      if ( $this->headerIsReaded ) return;
      $this->headerIsReaded = true;
      if($this->Size()<0)   return FALSE;
      $result = 0;
      
      $this->pos = 4;
      $this->GetInt8($this->moudleId);
      $this->GetInt16($this->id);
      $this->GetInt8($result);
      if ( 1 == $result ) $this->isResult = true;
      else $this->isResult = false;
  }
  
  //模块号，避免同时加载多个sdk时协议号重复
  function MoudleId()
  {
      $this->ReadHeader();
      return $this->moudleId;
	}

  //返回协议号
	function Id()
  {
      $this->ReadHeader();
      return $this->id;
  }
  
  //返回是回应报文
	function IsResult()
  {
      $this->ReadHeader();
      return $this->isResult;
  }
  
  //设置模块号，字节序
	function BindSetting( $byteOrder, $moudleId )
  {
    $this->byteOrder = $byteOrder;
    $this->moudleId = $moudleId;
  }
  
  //设置报文头
	function SetId( $id, $isResult = false )
  {
    $this->Clear();
  	$this->AddInt32($this->HeaderSize());
  	$this->AddInt8($this->moudleId);
	  $this->AddInt16($id);
	  if ( $isResult ) $this->AddInt8(1);
	  else $this->AddInt8(0);
  }

}
?>
