package com.huoyu.client;

import java.io.Serializable;

public class CallResult<T> implements Serializable {
    private static final long serialVersionUID = 6291855806635739209L;

    private T                 data;
    private int               code;
    private String            reason;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //以下是成功结果
    //成功,不需要返回数据
    public CallResult() {
        this.code = ResultCode.SUCCESS.value();
        this.reason = null;
        this.data = null;
    }

    //成功，需要返回数据
    public CallResult(T data) {
        this.code = ResultCode.SUCCESS.value();
        this.reason = null;
        this.data = data;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //以下是失败的结果
    //失败,返回继承的原因
    @SuppressWarnings("rawtypes")
    public CallResult(CallResult result) {
        this.code = result.code;
        this.reason = result.reason;
        this.data = null;
    }

    //失败,返回自定义原因
    public CallResult(int code, String reason) {
        this.code = code;
        this.reason = reason;
        this.data = null;
    }

    //失败,返回自定义原因
    public CallResult(ResultCode code, String reason) {
        this.code = code.value();
        this.reason = reason;
        this.data = null;
    }

    //失败,返回标准原因
//    public CallResult(int code) {
//        this.code = code;
//        this.reason = ResultCode.toEnum(code).descript();
//        this.data = null;
//    }

    //失败,返回标准原因
    public CallResult(ResultCode code) {
        this.code = code.value();
        this.reason = code.descript();
        this.data = null;
    }

    public T getData() {
        return data;
    }

    public int getCode() {
        return code;
    }

    public String getReason() {
        return reason;
    }
}