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

import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.graphics.Bitmap;
import android.os.Build;
import android.os.Message;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.webkit.CookieManager;
import android.webkit.CookieSyncManager;
import android.webkit.JavascriptInterface;
import android.webkit.JsResult;
import android.webkit.WebChromeClient;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.EditText;
import android.widget.Toast;

import org.anyrtc.core.AnyRTMP;
import org.anyrtc.core.RTMPGuestKit;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;

public class MainActivity extends AppCompatActivity {

    private EditText mEditRtmpUrl;
    private WebView webView;
    //设置时间间隔和上次退出时间
    private static final long TIME=2000;
    private long exitTime;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        webView = (WebView) findViewById(R.id.webview);
        webView.getSettings().setDomStorageEnabled(true);
        webView.getSettings().setAppCacheMaxSize(1024*1024*8);
        String appCachePath = getApplicationContext().getCacheDir().getAbsolutePath();
        webView.getSettings().setAppCachePath(appCachePath);
        webView.getSettings().setAllowFileAccess(true);
        webView.getSettings().setAppCacheEnabled(true);
//        webView.loadUrl("file:///android_asset/test.html");//加载asset文件夹下html
        //webView.loadUrl(getResources().getString(R.string.app_uri));//加载url
        webView.loadUrl(getResources().getString(R.string.app_urir));//加载url

        //使用webview显示html代码
//        webView.loadDataWithBaseURL(null,"<html><head><title> 欢迎您 </title></head>" +
//                "<body><h2>使用webview显示 html代码</h2></body></html>", "text/html" , "utf-8", null);

        webView.addJavascriptInterface(this,"android");//添加js监听 这样html就能调用客户端
        webView.setWebChromeClient(webChromeClient);
        webView.setWebViewClient(webViewClient);

        WebSettings webSettings=webView.getSettings();
        webSettings.setJavaScriptEnabled(true);//允许使用js

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            if (0 != (getApplicationInfo().flags & ApplicationInfo.FLAG_DEBUGGABLE))
            { webView.setWebContentsDebuggingEnabled(true); }
        }

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


        {//* Init UI
            //mEditRtmpUrl = (EditText) findViewById(R.id.edit_rtmp_url);
        }
        AnyRTMP.Inst();
    }

    //重写onKeyDown方法
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        //判断是否按的后退键，而且按了一次
        if(keyCode== KeyEvent.KEYCODE_BACK&&event.getRepeatCount()==0)
        {
            //获取当前的系统时间，和exitTime相减，判断两次间隔是否大于规定时间
            //exitTime没有初始值则默认为0
            //如果大于设定的时间，则弹出提示，同时把exitTime设置为当前时间
            if(System.currentTimeMillis()-exitTime>TIME)
            {
                // 这里处理一下浏览器的back动作。
                if(webView.canGoBack()){
                    webView.goBack();
                    return false;
                }
                Toast.makeText(this,"再按一次退出程序",Toast.LENGTH_LONG).show();
                exitTime= System.currentTimeMillis();
            }
            else
            {
                //如果再次按后退的时间小于规定时间，则退出
                finish();
            }
            //消费事件
            return true;
        }
        //不处理事件
        return false;
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
                Toast.makeText(MainActivity.this,"国内不能访问google,拦截该url",Toast.LENGTH_LONG).show();
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
            //AlertDialog.Builder localBuilder = new AlertDialog.Builder(webView.getContext());
           // localBuilder.setMessage(message).setPositiveButton("确定",null);
           // localBuilder.setCancelable(false);
            //localBuilder.create().show();
            try {
                JSONObject obj = new JSONObject(message);
                final String cmd = obj.getString("cmd");
                if(cmd.equals("pulldlghis117") || cmd.equals("pulldlghisgp") || cmd.equals("pulldlghis")){ // history
                    final String info1 = message;
                    new Thread(new Runnable() {
                        @Override
                        public void run() {
                            try {
                                JSONObject obj = new JSONObject(info1);
                                JSONObject objData = obj.getJSONObject("data");
                                String id = objData.getString("id");
                                String pulluri = objData.getString("filepath");
                                int enc = objData.getInt("encrypted");
                                char enckey = 0;
                                objData = obj.getJSONObject("ui");

                                int userid = objData.optInt("UserID");//objData.getInt("UserId");
                                String sid = objData.optString("SessionID");// objData.getString("SessionId");

                                JSONObject data = new JSONObject();
                                try {
                                    data.put("action", "caniplay");
                                    data.put("id", id);
                                } catch (Exception e) {

                                }
                                URL url = new URL(MyApplication.apiServer + "/api/1.00/private");//放网站
                                if(cmd.equals("pulldlghis117")){
                                    url = new URL(MyApplication.apiServer + "/api/1.00/public");//放网站
                                }
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
                                            if (res.getInt("status") == 0 || cmd.equals("pulldlghis117")) {
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
                }else { //living
                    JSONObject objData = obj.getJSONObject("data");
                    String roomid = objData.getString("roomid");
                    String pulluri ="rtmp://" + objData.getString("vhost") + ":" +
                            objData.getString("vport") + "/" + objData.getString("vapp") +
                            "/" + objData.getString("vstream");//= objData.getString("pulluri");
                    int enc = 0;// = objData.optInt("Encryptioned");// .getInt("Encryptioned");
                    objData = obj.getJSONObject("ui");
                    String sid = objData.getString("SessionID");
                    String tkn = objData.getString("Token");
                    int userid= objData.getInt("UserID");

                    Intent it = new Intent(getApplicationContext(), GuestActivity.class);
                    Bundle bd = new Bundle();
                    bd.putString("rtmp_url", pulluri + "?s=" + sid + "&t=" + tkn);
                    bd.putInt("Encryptioned",enc);
                    bd.putInt("UserId",userid);
                    bd.putString("roomid",roomid);
                    it.putExtras(bd);
                    startActivity(it);
                }

            }catch (Exception e1){
                Log.i("",e1.toString());

            }

            //注意:
            //必须要这一句代码:result.confirm()表示:
            //处理结果为确定状态同时唤醒WebCore线程
            //否则不能继续点击按钮
            result.confirm();
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


    public void OnBtnClicked(View view) {
        String rtmpUrl = mEditRtmpUrl.getEditableText().toString();
        if (rtmpUrl.length() == 0) {
            return;
        }

    }



}
