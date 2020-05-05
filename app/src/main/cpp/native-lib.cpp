#include <jni.h>
#include <string>
#include "gif_lib.h"
#include <base.h>
#include <android/bitmap.h>
#define argb(a,r,g,b) (((a)& 0xff) << 24)|( ((r)& 0xff) << 16 )|( ((g)& 0xff) << 8 )| ((b)& 0xff)

typedef struct GifBean{
    //播放帧
    int current_frame;
    int total_frame;
    //延迟时间数组
    int *delays;
}GifBean;

extern "C" JNIEXPORT jstring JNICALL
Java_com_sjq_giftso_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}


//绘制一张图片
void drawFrame(GifFileType *gifFileType,GifBean *gifBean,AndroidBitmapInfo info,void *pixels){

    //当前帧
    SavedImage savedImage = gifFileType->SavedImages[gifBean->current_frame];
    //整幅图片的首地址
    int *px = (int *)(pixels);
    int pointPixel;
    GifImageDesc frameInfo = savedImage.ImageDesc;
    GifByteType gifByteType;//压缩数据
    //rgb数据   压缩工具
    ColorMapObject* colorMapObject = frameInfo.ColorMap;
    //bitmap 往下偏移
    px = (int *)((char *)px + info.stride * frameInfo.Top);
    //每一行的首地址
    int *line;

    for (int y = frameInfo.Top; y < frameInfo.Top + frameInfo.Height; ++y) {
        line = px;
        for (int x = frameInfo.Left; x < frameInfo.Left + frameInfo.Width; ++x) {

            //拿到每一个坐标的位置  索引   数据
            pointPixel = (y - frameInfo.Top)*frameInfo.Width + (x -frameInfo.Left);
            //索引  rgb lzw压缩  字典  （）缓存在一个字典
            gifByteType = savedImage.RasterBits[pointPixel];
            GifColorType gifColorType = colorMapObject->Colors[gifByteType];
            line[x]= argb(255,gifColorType.Red,gifColorType.Green,gifColorType.Blue);


        }
        px = (int *)((char *) px + info.stride);
    }
}
extern "C"
JNIEXPORT jlong JNICALL
Java_com_sjq_giftso_GifHandler_loadPath(JNIEnv *env, jobject thiz, jstring path_) {

    const char *path = env->GetStringUTFChars(path_,0);
    int error;
    GifFileType *gifFileType =DGifOpenFileName(path,&error);
    DGifSlurp(gifFileType);
    //new GifBean
    GifBean *giftBean = static_cast<GifBean *>(malloc(sizeof(GifBean)));
    //清空内存地址
    memset(giftBean,0, sizeof(GifBean));
    gifFileType->UserData = giftBean;
    //初始化数组
    giftBean->delays = static_cast<int *>(malloc(sizeof(int) * gifFileType->ImageCount));
    memset(giftBean->delays,0, sizeof(int)*gifFileType->ImageCount);
    //延迟事件，读取
    //Delay Time -单位 1/100秒，如果值不为1，标识暂停规定的时间后再继续往下处理数据流
    //获取时间
    gifFileType->UserData = giftBean;
    giftBean->current_frame = 0;
    giftBean->total_frame = gifFileType->ImageCount;

    ExtensionBlock *ext;
    for (int i = 0; i <gifFileType->ImageCount ; ++i) {
        SavedImage frame = gifFileType->SavedImages[i];
        for (int j = 0; j < frame.ExtensionBlockCount; ++j) {
            if(frame.ExtensionBlocks[j].Function==GRAPHICS_EXT_FUNC_CODE){

                ext = &frame.ExtensionBlocks[j];
                break;
            }

        }

        //25 30 25 30
        if(ext){
            int frame_delay = 10*(ext->Bytes[1]|(ext->Bytes[2] << 8));
            LOGE("时间 %d ",frame_delay);
            giftBean->delays[i] = frame_delay;
        }
    }


    LOGE("gif 长度大小  %d",gifFileType->ImageCount);
    env->ReleaseStringUTFChars(path_,path);
    return reinterpret_cast<jlong>(gifFileType);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_sjq_giftso_GifHandler_getWidth(JNIEnv *env, jobject thiz, jlong ndk_gif) {

    GifFileType *gifFileType = reinterpret_cast<GifFileType *>(ndk_gif);
    return gifFileType->SWidth;


}

extern "C"
JNIEXPORT jint JNICALL
Java_com_sjq_giftso_GifHandler_getHeight(JNIEnv *env, jobject thiz, jlong ndk_gif) {
    GifFileType *gifFileType = reinterpret_cast<GifFileType *>(ndk_gif);
    return gifFileType->SHeight;

}

extern "C"
JNIEXPORT jint JNICALL
Java_com_sjq_giftso_GifHandler_updateFrame(JNIEnv *env, jobject thiz, jlong ndk_gif,
                                           jobject bitmap) {

    GifFileType *gifFileType = (GifFileType *)(ndk_gif);
    GifBean *gifBean = static_cast<GifBean *>(gifFileType->UserData);
    AndroidBitmapInfo intfo;
    //入参 出参对象


    //像素数组
    AndroidBitmap_getInfo(env,bitmap,&intfo);
    //空的bitmap
    void *pixels;
    AndroidBitmap_lockPixels(env,bitmap,&pixels);
    drawFrame(gifFileType,gifBean,intfo,pixels);
    gifBean->current_frame +=1;
    if(gifBean->current_frame >= gifBean->total_frame -1){

        gifBean->current_frame = 0;
        LOGE("重新过来  %d",gifBean->current_frame);
    }
    AndroidBitmap_unlockPixels(env,bitmap);

    return gifBean->delays[gifBean->current_frame];





}
