package org.anyrtc.anyrtmp;


import android.app.Application;
import android.content.res.Resources;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;

import static android.content.ContentValues.TAG;

public class MyApplication extends Application {
    //声明一个变量
    public String nameString;
    public short [][] pp = new short[10][];
    public int [] len = new int[10];
    @Override
    public void onCreate() {
        super.onCreate();
        int i = 0;
        setname("英雄联盟");
        int [] resids = {R.raw.a0_192k, R.raw.a1_192k, R.raw.a2_192k, R.raw.a3_192k, R.raw.a4_192k, R.raw.a5_192k,
                R.raw.a6_192k, R.raw.a7_192k, R.raw.a8_192k, R.raw.a9_192k};
        for(i = 0; i < 10; i++){
            pp[i] = readRawFile(resids[i]);
            len[i] = readRawFileLen(resids[i]);
        }
    }


    public int readRawFileLen(int id){
        Resources resources=this.getResources();
        InputStream is=null;
        int ret = 0;
        try{
            is=resources.openRawResource(id);
            ret = is.available() * 4;
        }catch(IOException e){
            Log.e(TAG, "write file",e);
        }
        if(is!=null) {
            try {
                is.close();
            } catch (IOException e) {
                Log.e(TAG, "close file", e);
            }
        }
        return ret;
    }

    public short[] readRawFile(int id){
        Resources resources=this.getResources();
        InputStream is=null;
        short [] ret = null;
        try{
            is=resources.openRawResource(id);
            byte buffer[]=new byte[is.available()];
            is.read(buffer);
            short[] or = BytesTransUtil.getInstance().Bytes2Shorts(buffer);
            ret = new short[or.length*4];
            int z = 0;
            for(int i = 0; i < or.length; i++){
                ret[z] = (short)((float)or[i] * 0.03f);
                z++;
                ret[z] = (short)((float)or[i] * 0.03f);
                z++;
                ret[z] = (short)((float)or[i] * 0.03f);
                z++;
                ret[z] = (short)((float)or[i] * 0.03f);
                z++;
            }
        }catch(IOException e){
            Log.e(TAG, "write file",e);
        }
        if(is!=null) {
            try {
                is.close();
            } catch (IOException e) {
                Log.e(TAG, "close file", e);
            }
        }
        return ret;
    }

    //实现setname()方法，设置变量的值
    public void setname(String name) {
        this.nameString = name;
    }

    //实现getname()方法，获取变量的值
    public String getname() {
        return nameString;
    }
}
