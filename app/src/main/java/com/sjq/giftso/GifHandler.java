package com.sjq.giftso;

import android.graphics.Bitmap;

public class GifHandler {
    static {
        System.loadLibrary("native-lib");
    }

    public GifHandler(String path){
        //加载，信使
        this.gifAddr = loadPath(path );
    }

    //放在Java层，方便传参数
    private long gifAddr;

    public int getWidth(){
        return getWidth(gifAddr);
    }

    public int getHeight(){
        return getHeight(gifAddr);
    }

    public int updateFrame(Bitmap bitmap){
        return updateFrame(gifAddr,bitmap);
    }


    private native long loadPath(String path);

    public native int getWidth(long ndkGif);

    public native int getHeight(long ndkGif);

    public native int updateFrame(long ndkGif, Bitmap bitmap);
}
