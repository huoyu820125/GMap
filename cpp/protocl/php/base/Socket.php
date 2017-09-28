<?php 
/*
	Socket类
*/
class TcpClient
{
	var $inited;	// 是否初始化
    var $connected; //是否连接
	var $socket;	// socket句柄
	var $lastError;	// 上次发生的错误
	
	function TcpClient()
	{
		$this->inited = true;
    	$this->connected = false;
		$this->lastError = "没有错误发生";
		$this->socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
		if ($this->socket < 0)
		{
			$this->lastError = "socket_create() failed reason: ".socket_strerror($this->socket).socket_strerror(socket_last_error());
			$this->inited = false;
			$this->socket = -1;
			return;
		}
		if ( !$this->SetBlock(true) ) return;
	}

	// 设置套接字模式阻塞/非阻塞
    private function SetBlock( $block )
	{
		if ( !$this->inited ) return $this->inited;
		$result = true;
		if ($block) $result = socket_set_block($this->socket);
		else $result = socket_set_nonblock($this->socket);
		if ( !$result ) $this->lastError = "SetMode() failed reason: ".socket_strerror(socket_last_error());
		
		return $result;
	}
	 
    // 连接一个地址
    function Connect( $ip, $port )
    {
		if ( !$this->inited ) return $this->inited;
    if ( $this->connected ) return $this->connected;

		if ( @socket_connect($this->socket, $ip, $port) ) 
    {
      $this->connected = true;
      return true;
    }
		echo "连接(".$ip.",".$port.")失败\n";	
		if ( !$result ) $this->lastError = "Connect(".$ip.",".$port.") failed reason:".socket_strerror(socket_last_error());
		return false;
    }

    /*
		发送数据
			msg			要发送的数据
			size		要发送的长度
		返回值
			true成功，false失败
	*/
    function Send( $msg, $size )
    {
		if ( !$this->inited ) return $this->inited;

		if ( @$size > socket_send( $this->socket, $msg, $size, 0 ) )
		{
			$this->lastError = "Send() failed: reason: ".socket_strerror(socket_last_error());
			return false;
		}
		
		return true;
    }

    /*
		接收数据
			msg			收到的数据
			size		想要接收的长度
			delFromBuf	将本次收到的数据从缓冲删除
		返回值
			true成功，false失败
	*/
    function Recv( &$msg, $size, $delFromBuf = true )
    {
		if ( !$this->inited ) return $this->inited;
		$flag = 2;
		if ( $delFromBuf ) $flag = 8;
		@$recvSize = socket_recv( $this->socket, $msg, $size, $flag );
		if ( $size > $recvSize )
		{
			$this->lastError = "Recv() failed: reason: ".socket_strerror(socket_last_error());
			echo "recv:(".$recvSize.")".$this->lastError."</br>";
			return false;
		}
//		echo "recv:(".$recvSize.")".$msg."</br>";
		return true;
    }

}

?>
