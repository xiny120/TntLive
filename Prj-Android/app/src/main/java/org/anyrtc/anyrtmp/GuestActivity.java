/**
 *  Copyright (c) 2016 The AnyRTC project authors. All Rights Reserved.
 *
 *  Please visit https://www.anyrtc.io for detail.
 *
 * The GNU General Public License is a free, copyleft license for
 * software and other kinds of works.
 *
 * The licenses for most software and other practical works are designed
 * to take away your freedom to share and change the works.  By contrast,
 * the GNU General Public License is intended to guarantee your freedom to
 * share and change all versions of a program--to make sure it remains free
 * software for all its users.  We, the Free Software Foundation, use the
 * GNU General Public License for most of our software; it applies also to
 * any other work released this way by its authors.  You can apply it to
 * your programs, too.
 * See the GNU LICENSE file for more info.
 */
package org.anyrtc.anyrtmp;

import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Point;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.preference.PreferenceManager;
import android.support.v7.app.ActionBar;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.WindowManager;
import android.webkit.JavascriptInterface;
import android.webkit.JsResult;
import android.webkit.WebChromeClient;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import org.anyrtc.core.AnyRTMP;
import org.anyrtc.core.RTMPGuestHelper;
import org.anyrtc.core.RTMPGuestKit;
import org.json.JSONException;
import org.json.JSONObject;
import org.webrtc.RendererCommon;
import org.webrtc.SurfaceViewRenderer;
import org.webrtc.VideoRenderer;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;

import static android.content.ContentValues.TAG;

/**
 * Created by Eric on 2016/9/16.
 */
public class GuestActivity extends Activity implements RTMPGuestHelper,  SurfaceHolder.Callback{
    private WebView mwvMediaList;
    //private LinearLayout mToolbar = null;
    private ImageView mIvClose = null;
    private ImageView mIvFull = null;
    private TextView mTxtStatus = null;
    private RTMPGuestKit mGuest = null;
    private SurfaceViewRenderer mSurfaceView = null;
    private VideoRenderer mRenderer = null;
    private SurfaceHolder mHolder;

    public static final float SHOW_SCALE = 16 * 1.0f / 9;

    private RelativeLayout mSurfaceLayout;

    //屏幕宽度
    private boolean mFullScreen;
    private int mScreenWidth;
    private int mVideoWidth;
    private int mVideoLeft;
    //屏幕高度
    private int mScreenHeight;
    private DisplayMetrics displayMetrics;

    /**
     * 接收解析后传过来的数据
     */
    Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            mTxtStatus.setVisibility(View.GONE);
            //mToolbar.setVisibility(View.VISIBLE);
            mIvFull.setVisibility(View.VISIBLE);
            RelativeLayout.LayoutParams lp =
                    (RelativeLayout.LayoutParams) mSurfaceLayout.getLayoutParams();
            lp.width = mVideoWidth;//(int) (mScreenWidth * SHOW_SCALE);
            lp.height = mVideoWidth*576/720;
            lp.leftMargin = mVideoLeft;
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_guest);
        {//* Init UI
            mFullScreen = false;
            mTxtStatus = (TextView) findViewById(R.id.txt_rtmp_status);
            mSurfaceView = (SurfaceViewRenderer) findViewById(R.id.suface_view);
            //mToolbar = (LinearLayout)findViewById(R.id.llayout_host_tools);
            //mToolbar.setVisibility(View.GONE);
            mIvFull = (ImageView)findViewById(R.id.btn_tofullscreen);
            mIvFull.setVisibility(View.GONE);
            mwvMediaList = (WebView)findViewById(R.id.webviewmeidalist);
            displayMetrics = new DisplayMetrics();
            this.getWindow().getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
            mScreenWidth = displayMetrics.widthPixels;
            mScreenHeight = displayMetrics.heightPixels;
            mVideoWidth = mScreenWidth * 4 / 5;
            mVideoLeft = (mScreenWidth - mVideoWidth) / 2;
            mSurfaceView.init(AnyRTMP.Inst().Egl().getEglBaseContext(), new RendererCommon.RendererEvents(){
                public void onFirstFrameRendered(){
                    handler.sendMessage(handler.obtainMessage());
                };
                public void onFrameResolutionChanged(int videoWidth, int videoHeight, int rotation){

                };
            });
            mRenderer = new VideoRenderer(mSurfaceView);
        }
        String rtmpUrl = getIntent().getExtras().getString("rtmp_url");
        int enc = getIntent().getExtras().getInt("Encryptioned");
        int enckey = 0;
        int userid = getIntent().getExtras().getInt("UserId");
        mGuest = new RTMPGuestKit(this, this);
        MyApplication myApp = (MyApplication) getApplication();
        mGuest.StartRtmpPlay(rtmpUrl, mRenderer.GetRenderPointer(),"rtmp","",enc,enckey,String.valueOf(userid),myApp.pp,myApp.len);
        initView();
        initWebView();
    }

    public int readRawFileLen(int id){
        Resources resources=this.getResources();
        InputStream is=null;
        int ret = 0;
        try{
            is=resources.openRawResource(id);
            ret = is.available();
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
            ret = BytesTransUtil.getInstance().Bytes2Shorts(buffer);
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


    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Point pt = mSurfaceView.getVideoSize();
        Log.i(TAG,pt.toString());
    }
    @Override
    public void surfaceChanged(SurfaceHolder holder
            , int format, int width, int height) {
        Point pt = mSurfaceView.getVideoSize();
        Log.i(TAG,pt.toString());
    }
    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
    }
    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mGuest != null) {
            mGuest.StopRtmpPlay();
            mGuest.Clear();
            mGuest = null;
        }
    }

    //当指定了android:configChanges="orientation"后,方向改变时onConfigurationChanged被调用,并且activity不再销毁重建
    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        View btn = findViewById(R.id.btn_tofullscreen);
        super.onConfigurationChanged(newConfig);
        switch (newConfig.orientation) {
            case Configuration.ORIENTATION_PORTRAIT://竖屏
                mFullScreen = false;
                ((ImageView) btn).setImageResource(R.mipmap.tofullscreen);
                break;
            case Configuration.ORIENTATION_LANDSCAPE://横屏
                mFullScreen = true;
                ((ImageView) btn).setImageResource(R.mipmap.tosmallwindow);
            default:
                break;
        }

        fullScreenChange(!mFullScreen);
        Point ptVideo = mSurfaceView.getVideoSize();
        displayMetrics = new DisplayMetrics();
        this.getWindow().getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
        mScreenWidth = displayMetrics.widthPixels;
        mScreenHeight = displayMetrics.heightPixels;
        mVideoWidth = mScreenWidth * 4 / 5;
        mVideoLeft = (mScreenWidth - mVideoWidth) / 2;
        mSurfaceLayout = (RelativeLayout) findViewById(R.id.layout_gesture);
        RelativeLayout.LayoutParams lp =
                (RelativeLayout.LayoutParams) mSurfaceLayout.getLayoutParams();
        lp.width = mVideoWidth;//(int) (mScreenWidth * SHOW_SCALE);
        lp.height = mVideoWidth*ptVideo.y/ptVideo.x;

        if(lp.height > mScreenHeight){
            lp.height = mScreenHeight;
            lp.width = lp.height * ptVideo.x / ptVideo.y;
        }
        lp.leftMargin = mVideoLeft;
        mSurfaceLayout.setLayoutParams(lp);
    }

     public void fullScreenChange(boolean fullScreen) {
        //SharedPreferences mPreferences = PreferenceManager.getDefaultSharedPreferences(this);
        //boolean fullScreen = mPreferences.getBoolean("fullScreen", false);
        WindowManager.LayoutParams attrs = getWindow().getAttributes();
        System.out.println("fullScreen的值:" + fullScreen);
        if (fullScreen) {
            attrs.flags &= (~WindowManager.LayoutParams.FLAG_FULLSCREEN);
            getWindow().setAttributes(attrs);
            //取消全屏设置
            getWindow().clearFlags(WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS);
            this.getActionBar().show();
            //mPreferences.edit().putBoolean("fullScreen", false).commit() ;
            if (Build.VERSION.SDK_INT > 11 && Build.VERSION.SDK_INT < 19) {
                //低版本sdk
                View v = getWindow().getDecorView();
                v.setSystemUiVisibility(View.VISIBLE);
            } else if (Build.VERSION.SDK_INT >= 19) {
                View decorView = getWindow().getDecorView();
                int uiOptions = View.VISIBLE; //.SYSTEM_UI_FLAG_FULLSCREEN;
                decorView.setSystemUiVisibility(uiOptions);
            }
        } else {
            attrs.flags |= WindowManager.LayoutParams.FLAG_FULLSCREEN;
            getWindow().setAttributes(attrs);
            getWindow().addFlags(WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS);
            //mPreferences.edit().putBoolean("fullScreen", true).commit();
            if (Build.VERSION.SDK_INT > 11 && Build.VERSION.SDK_INT < 19) {
                View v = getWindow().getDecorView();
                v.setSystemUiVisibility(View.GONE);
            } else if (Build.VERSION.SDK_INT >= 19) {
                View decorView = getWindow().getDecorView();
                int uiOptions = View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                        | View.SYSTEM_UI_FLAG_FULLSCREEN;
                decorView.setSystemUiVisibility(uiOptions);
            }

        }
    }
    /**
     * the button click event listener
     *
     * @param btn
     */
    public void OnBtnClicked(View btn) {
        if (btn.getId() == R.id.btn_close) {
            if (mGuest != null) {
                mGuest.StopRtmpPlay();
                mGuest.Clear();
                mGuest = null;
            }
            finish();
        }else if(btn.getId() == R.id.btn_tofullscreen){
            if(!mFullScreen){
                setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
            }else{
                setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
            }
        }
    }

    /**
     * Implements for RTMPGuestHelper
     */
    @Override
    public void OnRtmplayerOK() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mTxtStatus.setText(R.string.str_rtmp);
            }
        });
    }

    @Override
    public void OnRtmplayerStatus(final int cacheTime, final int curBitrate,final int curTime, final int totalTime) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if(curBitrate > 0 && cacheTime > 0)
                    mTxtStatus.setText(String.format( getString(R.string.str_rtmp_pull_status), cacheTime, curBitrate,curTime,totalTime));
            }
        });
    }

    @Override
    public void OnRtmplayerCache(int time) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
            }
        });
    }

    @Override
    public void OnRtmplayerClosed(int errcode) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mTxtStatus.setText(R.string.str_rtmp);
            }
        });
    }
    @Override
    public void OnRtmplayer1stVideo() {

    }
    @Override
    public void OnRtmplayer1stAudio() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                //mTxtStatus.setText(String.format( getString(R.string.str_rtmp_pull_status), cacheTime, curBitrate,curTime,totalTime));
                mwvMediaList.loadUrl(getResources().getString(R.string.app_urir) + "#/pages/chatroom/chatroom");
            }
        });
    }
    @Override
    public void OnRtmplayerConnectionFailed(int a) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mTxtStatus.setText(R.string.str_rtmp);
            }
        });
    }

    //WebViewClient主要帮助WebView处理各种通知、请求事件
    private WebViewClient webViewClient=new WebViewClient(){
        @Override
        public void onPageFinished(WebView view, String url) {//页面加载完成

        }

        @Override
        public void onPageStarted(WebView view, String url, Bitmap favicon) {//页面开始加载

        }

        @Override
        public boolean shouldOverrideUrlLoading(WebView view, String url) {
            Log.i("ansen","拦截url:"+url);
            if(url.equals("http://www.google.com/")){
                //Toast.makeText(MainActivity.this,"国内不能访问google,拦截该url",Toast.LENGTH_LONG).show();
                return true;//表示我已经处理过了
            }
            return super.shouldOverrideUrlLoading(view, url);
        }

    };

    //WebChromeClient主要辅助WebView处理Javascript的对话框、网站图标、网站title、加载进度等
    private WebChromeClient webChromeClient=new WebChromeClient(){
        //不支持js的alert弹窗，需要自己监听然后通过dialog弹窗
        @Override
        public boolean onJsAlert(WebView webView, String url, String message, JsResult result) {
            //注意:
            //必须要这一句代码:result.confirm()表示:
            //处理结果为确定状态同时唤醒WebCore线程
            //否则不能继续点击按钮
            //Toast.makeText(getApplicationContext(),url,Toast.LENGTH_LONG);
            result.confirm();

            try {
                JSONObject obj = new JSONObject(message);
                String cmd = obj.getString("cmd");
                JSONObject objData = obj.getJSONObject("data");
                String pulluri = objData.getString("FilePath");
                objData = obj.getJSONObject("ui");
                String sid = objData.getString("SessionId");
                String tkn = objData.getString("Token");


                final String info1 = message;
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        try {
                            JSONObject obj = new JSONObject(info1);
                            JSONObject objData = obj.getJSONObject("data");
                            String id = objData.getString("Id");
                            String pulluri = objData.getString("FilePath");
                            int enc = objData.getInt("Encryptioned");
                            char enckey = 0;
                            objData = obj.getJSONObject("ui");

                            int userid = objData.optInt("UserId");//objData.getInt("UserId");
                            String sid = objData.optString("SessionId");// objData.getString("SessionId");

                            JSONObject data = new JSONObject();
                            try {
                                data.put("action", "caniplay");
                                data.put("id", id);
                            } catch (Exception e) {

                            }
                            URL url = new URL(MyApplication.apiServer + "/api/1.00/private");//放网站
                            HttpURLConnection httpURLConnection = (HttpURLConnection) url.openConnection();
                            httpURLConnection.setRequestMethod("POST");
                            httpURLConnection.setRequestProperty("Content-Type", "application/json");
                            httpURLConnection.setRequestProperty("mster-token", sid);
                            OutputStream ots = httpURLConnection.getOutputStream();
                            ots.write(data.toString().getBytes());
                            InputStream inputStream = httpURLConnection.getInputStream();
                            InputStreamReader reader = new InputStreamReader(inputStream, "UTF-8");
                            BufferedReader bufferedReader = new BufferedReader(reader);
                            final StringBuffer buffer = new StringBuffer();
                            String temp = null;
                            while ((temp = bufferedReader.readLine()) != null) {
                                buffer.append(temp);
                            }
                            bufferedReader.close();//记得关闭
                            reader.close();
                            inputStream.close();
                            Log.e("MAIN", buffer.toString());//打印结果
                            runOnUiThread(new Runnable() {
                                public void run() {
                                    try {
                                        JSONObject res = new JSONObject(buffer.toString());
                                        if (res.getInt("status") == 0 ) {
                                            //JSONObject resdata = res.getJSONObject("data");
                                            //if(resdata != null) {
                                            try {
                                                Intent it = new Intent(getApplicationContext(), FlvPlayerActivity.class);
                                                Bundle bd = new Bundle();
                                                bd.putString("minfo", info1);
                                                bd.putString("res", buffer.toString());
                                                it.putExtras(bd);
                                                startActivity(it);
                                            }catch (Exception e1){
                                                Log.i("",e1.toString());
                                            }
                                            //}else{
                                            //    Toast.makeText(getApplicationContext(), "播放数据为空！", Toast.LENGTH_LONG).show();
                                            // }
                                        } else {
                                            Toast.makeText(getApplicationContext(), res.getString("msg"), Toast.LENGTH_LONG).show();
                                        }
                                    } catch (Exception e1) {
                                        e1.printStackTrace();
                                    }
                                }
                            });
                        }catch (JSONException je){
                            je.printStackTrace();
                        } catch (MalformedURLException e) {
                            e.printStackTrace();
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                    }
                }).start();




/*

                Intent it = new Intent(getApplicationContext(), FlvPlayerActivity.class);
                Bundle bd = new Bundle();
                bd.putString("minfo", message);
                it.putExtras(bd);
                startActivity(it);
                */

            }catch (Exception e1){
                Log.i("",e1.toString());
            }
            return true;
        }

        //获取网页标题
        @Override
        public void onReceivedTitle(WebView view, String title) {
            super.onReceivedTitle(view, title);
            Log.i("ansen","网页标题:"+title);
        }

        //加载进度回调
        @Override
        public void onProgressChanged(WebView view, int newProgress) {
            Log.i("ansen","加载进度:"+newProgress);
        }
    };

    /**
     * JS调用android的方法
     * @param str
     * @return
     */
    @JavascriptInterface //仍然必不可少
    public void  getClient(String str){
        Log.i("ansen","html调用客户端:"+str);
    }

    private void initView() {
        mSurfaceLayout = (RelativeLayout) findViewById(R.id.layout_gesture);
        RelativeLayout.LayoutParams lp = (RelativeLayout.LayoutParams) mSurfaceLayout.getLayoutParams();
        lp.width = mVideoWidth;//(int) (mScreenWidth * SHOW_SCALE);
        lp.height = 120;//mVideoWidth*576/720;
        lp.leftMargin = mVideoLeft;
        mSurfaceLayout.setLayoutParams(lp);
        mHolder = mSurfaceView.getHolder();
        mHolder.addCallback(this);
        mTxtStatus.setVisibility(View.VISIBLE);
    }
    private void initWebView(){
        WebSettings webSettings;
        mwvMediaList = (WebView) findViewById(R.id.webviewmeidalist);
        webSettings=mwvMediaList.getSettings();
        webSettings.setDomStorageEnabled(true);
        webSettings.setAppCacheMaxSize(1024*1024*8);
        String appCachePath = getApplicationContext().getCacheDir().getAbsolutePath();
        webSettings.setAppCachePath(appCachePath);
        webSettings.setAllowFileAccess(true);
        webSettings.setAppCacheEnabled(true);
        mwvMediaList.loadUrl(getResources().getString(R.string.app_urir) + "#/pages/medialist/medialist");//加载url
        //mwvMediaList.loadUrl("http://www.qq.com");

//        webView.loadDataWithBaseURL(null,"<html><head><title> 欢迎您 </title></head>" +
//                "<body><h2>使用webview显示 html代码</h2></body></html>", "text/html" , "utf-8", null);

        mwvMediaList.addJavascriptInterface(this,"android");//添加js监听 这样html就能调用客户端
        mwvMediaList.setWebChromeClient(webChromeClient);
        mwvMediaList.setWebViewClient(webViewClient);
        webSettings.setJavaScriptEnabled(true);//允许使用js
        /**
         * LOAD_CACHE_ONLY: 不使用网络，只读取本地缓存数据
         * LOAD_DEFAULT: （默认）根据cache-control决定是否从网络上取数据。
         * LOAD_NO_CACHE: 不使用缓存，只从网络获取数据.
         * LOAD_CACHE_ELSE_NETWORK，只要本地有，无论是否过期，或者no-cache，都使用缓存中的数据。
         */
        webSettings.setCacheMode(WebSettings.LOAD_NO_CACHE);//不使用缓存，只从网络获取数据.
        //支持屏幕缩放
        webSettings.setSupportZoom(true);
        webSettings.setBuiltInZoomControls(true);
        //mwvMediaList.setWebContentsDebuggingEnabled(true);
    }
}
