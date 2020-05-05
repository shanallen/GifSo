package com.sjq.giftso

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Environment
import android.os.Handler
import android.os.Message
import android.view.View
import android.widget.ImageView
import kotlinx.android.synthetic.main.activity_main.*
import java.io.File

class MainActivity : AppCompatActivity() {

    var bitmap:Bitmap?= null
    var  iv:ImageView?= null
    var handler:Handler?= null
    var gifHandler:GifHandler?= null
    var nextFrame:Int?=null
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        iv = findViewById(R.id.iv_gif)
        handler = object :Handler(){
            override fun handleMessage(msg: Message) {
                super.handleMessage(msg)

                //需要刷新下一帧
                var nextFrame = gifHandler?.updateFrame(bitmap)?.toLong()
                handler?.sendEmptyMessageDelayed(1,nextFrame!! )
                iv?.setImageBitmap(bitmap)
            }
        }

        bt_loadgif.setOnClickListener {

            ndkLoadGif()
        }

    }

    fun  ndkLoadGif(){

        var file = File(Environment.getDataDirectory(),"renzhe.gif")

        gifHandler = GifHandler(file.absolutePath)
        var width = gifHandler?.width
        var height = gifHandler?.height
        bitmap = Bitmap.createBitmap(width!!,height!!,Bitmap.Config.ARGB_8888)
        //下一帧的刷新事件
        var nextFrame = gifHandler?.updateFrame(bitmap)?.toLong()
        handler?.sendEmptyMessageDelayed(1,nextFrame!!)

    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String

    companion object {

        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("native-lib")
        }
    }
}
