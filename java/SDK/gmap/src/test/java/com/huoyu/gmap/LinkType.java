package com.huoyu.gmap;


public enum LinkType {
    UNKNOW(0, "未定义关系"),
    Friend(1, "好友"),
    Colleague(2, "同事"),
    Kinsfolk(3, "亲戚"),
    Schoolmate(4, "同学"),
    Blacklist(5, "黑名单"),
    Whitelist(6, "白名单");

    private final int    value;
    private final String descript;

    //构造器默认也只能是private, 从而保证构造函数只能在内部使用
    LinkType(int value, String descript) {
        this.value = value;
        this.descript = descript;
    }

    public static LinkType toEnum(int value) {
        if (LinkType.Friend.equals(value)) {
            return LinkType.Friend;
        }
        else if (LinkType.Colleague.equals(value)) {
            return LinkType.Colleague;
        }
        else if (LinkType.Kinsfolk.equals(value)) {
            return LinkType.Kinsfolk;
        }
        else if (LinkType.Schoolmate.equals(value)) {
            return LinkType.Schoolmate;
        }
        else if (LinkType.Blacklist.equals(value)) {
            return LinkType.Blacklist;
        }
        else if (LinkType.Whitelist.equals(value)) {
            return LinkType.Whitelist;
        }
        
        return LinkType.UNKNOW;
    }

    public static LinkType toEnum(Integer value) {
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
