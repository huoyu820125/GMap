<?php

class Stream {

    var $stream;
    var $size;
    var $byteOrder;
    var $pos;
    public static $bigOrder = 1; //大端序
    public static $smallOrder = 2; //小端序
    public static $netOrder = 3; //网络序

    function __construct() {
        $this->byteOrder = Stream::$smallOrder;
        $this->Clear();
    }

    //发送时用于直接传递给send接口，避免多次copy
    function &Data() {
        $this->SaveSize();
        return $this->stream;
    }

    function Clear() {
        $this->stream = "";
        $this->size = 0;
        $this->pos = 0;
    }

    //设置字节序
    function SetByteOrder($order) {
        $this->byteOrder = $order;
    }

    function Size() {
        $this->pos = 0;
         $this->size = 4;
        if (!$this->GetInt32($size))
        {
            return -1;
        }
        $this->size = $size;

        return $this->size;
    }

    function SaveSize() {
        $size = new Stream();
        $size->SetByteOrder($this->byteOrder);
        $size->AddInt32($this->size);
        $i = 0;
        for ($i = 0; $i < 4; $i++) {
            $c = ord($size->stream[$i]);
            $this->stream[$i] = chr($c);
        }
    }

    function AddStream($stream, $size) {
        $this->AddInt32($size);
        for ($i = 0; $i < $size; $i++) {
            $c = ord($stream[$i]);
            $this->stream .= chr($c);
        }
        $this->size += $size;
    }

    function GetStream(&$stream, &$size) {
        if (!$this->GetInt32($size))
            return false;
        if ($this->pos + $size > $this->size) {
            echo "长度越界：byte流总长" . $this->size . "，希望从位置" . $pos . "开始取" . $size . "byte</br>";
            return false;
        }
        $stream = '';
        for ($i = 0; $i < $size; $i++) {
            $c = ord($this->stream[$this->pos]);
            $stream .= chr($c);
            $this->pos++;
        }

        return true;
    }

    function AddString($stream) {
        $size = strlen($stream);
        $this->AddInt32($size + 1); //末尾加0
        for ($i = 0; $i < $size; $i++) {
            $c = ord($stream[$i]);
            $this->stream .= chr($c);
        }
        $this->size += $size;
        //末尾加0
        $this->stream .= chr(0);
        $this->size += 1;
    }

    function GetString(&$stream) {
        $size = 0;
        if (!$this->GetInt32($size)) return false;
        if ($this->pos + $size > $this->size) {
            echo "长度越界：byte流总长" . $this->size . "，希望从位置" . $pos . "开始取" . $size . "byte</br>";
            return false;
        }
        $stream = '';
        $size--;
        for ($i = 0; $i < $size; $i++) {
            $c = ord($this->stream[$this->pos]);
            $stream .= chr($c);
            $this->pos++;
        }
        $endFlag;
        if (!$this->GetInt8($endFlag)) return false;
        if ( 0 != $endFlag ) return false;
        return true;
    }

    function AddBool($value) {
        if ( true == $value ) $this->AddInt8(1);
        else $this->AddInt8(0);
    }

    function GetBool(&$value) {
        $data;
        if ( !$this->GetInt8($data) ) return false;
        if ( 1 == $data ) $value = true;
        else $value = false;
        return true;
    }

    function AddInt8($value) {
        if (Stream::$smallOrder == $this->byteOrder) {
            $this->Value2DescStream($value, 1);
        } else if (Stream::$bigOrder == $this->byteOrder) {
            $this->Value2Stream($value, 1);
        }
    }

    function GetInt8(&$value) {
        if (Stream::$smallOrder == $this->byteOrder) {
            return $this->DescStream2Value($value, 1);
        } else if (Stream::$bigOrder == $this->byteOrder) {
            return $this->Stream2Value($value, 1);
        }
    }

    function AddInt16($value) {
        if (Stream::$smallOrder == $this->byteOrder) {
            $this->Value2DescStream($value, 2);
        } else if (Stream::$bigOrder == $this->byteOrder) {
            $this->Value2Stream($value, 2);
        }
    }

    function GetInt16(&$value) {
        if (Stream::$smallOrder == $this->byteOrder) {
            return $this->DescStream2Value($value, 2);
        } else if (Stream::$bigOrder == $this->byteOrder) {
            return $this->Stream2Value($value, 2);
        }
    }

    function AddInt32($value) {
        if (Stream::$smallOrder == $this->byteOrder) {
            $this->Value2DescStream($value, 4);
        } else if (Stream::$bigOrder == $this->byteOrder) {
            $this->Value2Stream($value, 4);
        }
    }

    function GetInt32(&$value) {
        if (Stream::$smallOrder == $this->byteOrder) {
            return $this->DescStream2Value($value, 4);
        } else if (Stream::$bigOrder == $this->byteOrder) {
            return $this->Stream2Value($value, 4);
        }
    }

    function AddInt64($value) {
        if (Stream::$smallOrder == $this->byteOrder) {
            $this->Value2DescStream($value, 8);
        } else if (Stream::$bigOrder == $this->byteOrder) {
            $this->Value2Stream($value, 8);
        }
    }

    function GetInt64(&$value) {
        if (Stream::$smallOrder == $this->byteOrder) {
            return $this->DescStream2Value($value, 8);
        } else if (Stream::$bigOrder == $this->byteOrder) {
            return $this->Stream2Value($value, 8);
        }
    }

    private function Value2NetStream($value, $size) {
        return;
    }

    private function NetStream2Value(&$value, $size) {
        if ($this->pos + $size > $this->size) {
            echo "长度越界：byte流总长" . $this->size . "，希望从位置" . $this->pos . "开始取" . $size . "byte</br>";
            return 0;
        }

        return 0;
    }

    private function Value2DescStream($value, $size) {
        $c = $value & 0x00000000000000FF;
        $this->stream .= chr($c);

        for ($i = 1; $i < $size; $i++) {
            $c = $value >> ($i * 8) & 0x00000000000000FF;
            $this->stream .= chr($c);
        }
        $this->size += $size;

        return;
    }

    private function DescStream2Value(&$value, $size) {
        if ($this->pos + $size > $this->size) {
            echo "长度越界：byte流总长" . $this->size . "，希望从位置" . $this->pos . "开始取" . $size . "byte</br>";
            return false;
        }

        $this->pos += $size;
        $this->pos--;
        $c = ord($this->stream[$this->pos]);
        $value = $c;
        for ($i = 1; $i < $size; $i++) {
            $this->pos--;
            $c = ord($this->stream[$this->pos]);
            $value <<= 8;
            $value += $c;
        }
        $this->pos += $size;

        return true;
    }

    private function Value2Stream($value, $size) {

        for ($i = $size - 1; $i > 0; $i--) {
            $c = $value >> ($i * 8) & 0x00000000000000FF;
            $this->stream .= chr($c);
        }

        $c = $value & 0x00000000000000FF;
        $this->stream .= chr($c);
        $this->size += $size;

        return;
    }

    private function Stream2Value(&$value, $size) {
        if ($this->pos + $size > $this->size) {
            echo "长度越界：byte流总长" . $this->size . "，希望从位置" . $this->pos . "开始取" . $size . "byte</br>";
            return false;
        }
        $value = 0;

        $c = ord($this->stream[$this->pos]);
        $value += $c;
        $this->pos++;

        for ($i = 1; $i < $size; $i++) {
            $c = ord($this->stream[$this->pos]);
            $value <<= 8;
            $value += $c;
            $this->pos++;
        }

        return true;
    }

}

?>
