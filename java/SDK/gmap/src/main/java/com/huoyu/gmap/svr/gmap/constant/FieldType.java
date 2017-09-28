package com.huoyu.gmap.svr.gmap.constant;


public enum FieldType {
    UNKNOW(0, "未定义类型"),
    int8(1, "byte"),
    int16(2, "short"),
    int32(3, "int"),
    int64(4, "long"),
    str(5, "字符串"),
    date(6, "日期");

    private final int    value;
    private final String descript;

    //构造器默认也只能是private, 从而保证构造函数只能在内部使用
    FieldType(int value, String descript) {
        this.value = value;
        this.descript = descript;
    }

    public static FieldType toEnum(int value) {
        if (FieldType.int8.equals(value)) {
            return FieldType.int8;
        }
        else if (FieldType.int16.equals(value)) {
            return FieldType.int16;
        }
        else if (FieldType.int32.equals(value)) {
            return FieldType.int32;
        }
        else if (FieldType.int64.equals(value)) {
            return FieldType.int64;
        }
        else if (FieldType.str.equals(value)) {
            return FieldType.str;
        }
        else if (FieldType.date.equals(value)) {
            return FieldType.date;
        }
        
        return FieldType.UNKNOW;
    }

    public static FieldType toEnum(Integer value) {
        return toEnum(value.intValue());
    }

    public boolean equals(int v) {
        if (v == value) {
            return true;
        }

        return false;
    }

    public boolean equals(Integer v) {
        return equals(v.intValue());
    }

    /*
     * 枚举int类型值
     */
    public int value() {
        return value;
    }

    /*
     * 枚举的字符串描述
     */
    public String descript() {
        return descript;
    }

}
