package org.anyrtc.anyrtmp;

import android.annotation.SuppressLint;
import android.graphics.Point;
import android.os.Environment;
import android.os.Message;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.os.Handler;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.View;
import android.webkit.WebView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;

import org.anyrtc.core.AnyRTMP;
import org.anyrtc.core.RTMPGuestHelper;
import org.anyrtc.core.RTMPGuestKit;
import org.json.JSONObject;
import org.webrtc.RendererCommon;
import org.webrtc.SurfaceViewRenderer;
import org.webrtc.VideoRenderer;

import java.io.File;

/**
 * An example full-screen activity that shows and hides the system UI (i.e.
 * status bar and navigation/system bar) with user interaction.
 */
public class FlvPlayerActivity extends AppCompatActivity implements RTMPGuestHelper,  SurfaceHolder.Callback{
    private TextView mTxtStatus = null;
    private RTMPGuestKit mGuest = null;
    private SurfaceViewRenderer mSurfaceView = null;
    private VideoRenderer mRenderer = null;
    private SurfaceHolder mHolder;

    //屏幕宽度
    private int mScreenWidth;
    private int mVideoWidth;
    private int mVideoLeft;
    //屏幕高度
    private int mScreenHeight;
    private DisplayMetrics displayMetrics;
    private RelativeLayout mSurfaceLayout;
    /**
     * Whether or not the system UI should be auto-hidden after
     * {@link #AUTO_HIDE_DELAY_MILLIS} milliseconds.
     */
    private static final boolean AUTO_HIDE = true;

    /**
     * If {@link #AUTO_HIDE} is set, the number of milliseconds to wait after
     * user interaction before hiding the system UI.
     */
    private static final int AUTO_HIDE_DELAY_MILLIS = 3000;

    /**
     * Some older devices needs a small delay between UI widget updates
     * and a change of the status and navigation bar.
     */
    private static final int UI_ANIMATION_DELAY = 300;
    private final Handler mHideHandler = new Handler();
    private View mContentView;
    private final Runnable mHidePart2Runnable = new Runnable() {
        @SuppressLint("InlinedApi")
        @Override
        public void run() {
            // Delayed removal of status and navigation bar

            // Note that some of these constants are new as of API 16 (Jelly Bean)
            // and API 19 (KitKat). It is safe to use them, as they are inlined
            // at compile-time and do nothing on earlier devices.
            mContentView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LOW_PROFILE
                    | View.SYSTEM_UI_FLAG_FULLSCREEN
                    | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                    | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                    | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                    | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION);
        }
    };
    private View mControlsView;
    private final Runnable mShowPart2Runnable = new Runnable() {
        @Override
        public void run() {
            // Delayed display of UI elements
            ActionBar actionBar = getSupportActionBar();
            if (actionBar != null) {
                actionBar.show();
            }
            mControlsView.setVisibility(View.VISIBLE);

        }
    };
    private boolean mVisible;
    private final Runnable mHideRunnable = new Runnable() {
        @Override
        public void run() {
            hide();
        }
    };
    /**
     * Touch listener to use for in-layout UI controls to delay hiding the
     * system UI. This is to prevent the jarring behavior of controls going away
     * while interacting with activity UI.
     */
    private final View.OnTouchListener mDelayHideTouchListener = new View.OnTouchListener() {
        @Override
        public boolean onTouch(View view, MotionEvent motionEvent) {
            if (AUTO_HIDE) {
                delayedHide(AUTO_HIDE_DELAY_MILLIS);
            }
            return false;
        }
    };
    /**
     * 接收解析后传过来的数据
     */
    Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            mTxtStatus.setVisibility(View.VISIBLE);//(View.GONE);
            //RelativeLayout.LayoutParams lp =
            //        (RelativeLayout.LayoutParams) mSurfaceLayout.getLayoutParams();
           // lp.width = mVideoWidth;//(int) (mScreenWidth * SHOW_SCALE);
           // lp.height = mVideoWidth*576/720;
           // lp.leftMargin = mVideoLeft;
        }
    };
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_flv_player);

        mVisible = true;
        mControlsView = findViewById(R.id.fullscreen_content_controls);
        mContentView = findViewById(R.id.layout_gesture);
        mSurfaceLayout = (RelativeLayout) findViewById(R.id.layout_gesture);

        // Set up the user interaction to manually show or hide the system UI.
        mContentView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                toggle();
            }
        });

        // Upon interacting with UI controls, delay any scheduled hide()
        // operations to prevent the jarring behavior of controls going away
        // while interacting with the UI.
        findViewById(R.id.dummy_button).setOnTouchListener(mDelayHideTouchListener);


        mTxtStatus = (TextView) findViewById(R.id.txt_rtmp_status);
        mSurfaceView = (SurfaceViewRenderer) findViewById(R.id.suface_view);
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
        try {
            String info = getIntent().getExtras().getString("minfo");
            JSONObject obj = new JSONObject(info);
            String cmd = obj.getString("cmd");
            JSONObject objData = obj.getJSONObject("data");
            String pulluri = objData.getString("FilePath");
            objData = obj.getJSONObject("ui");
            String sid = objData.getString("SessionId");
            String tkn = objData.getString("Token");
            String url = getResources().getString(R.string.app_hosthis) +pulluri;
            mGuest = new RTMPGuestKit(this, this);
            mGuest.StartRtmpPlay(url, mRenderer.GetRenderPointer(), "flv", getDataPath() );
        }catch (Exception e1){

        }
    }

    public String getDataPath() {
        String ret = "";
        //首先判断外部存储是否可用
        if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
            ret = getExternalFilesDir("").getAbsolutePath();
            File sd = new File(ret);
            Log.e("qq", "sd = " + sd);//sd = /storage/emulated/0
            if (!sd.canWrite()) {
                ret = "";
            }
        }
        if(ret == ""){
            ret = getFilesDir().getAbsolutePath();
            File sd = new File(ret);
            Log.e("qq", "sd = " + sd);//sd = /storage/emulated/0
            if (!sd.canWrite()) {
                ret = "";
            }
        }

        return ret;
    }

    @Override
    protected void onPostCreate(Bundle savedInstanceState) {
        super.onPostCreate(savedInstanceState);

        // Trigger the initial hide() shortly after the activity has been
        // created, to briefly hint to the user that UI controls
        // are available.
        delayedHide(100);
    }

    private void toggle() {
        if (mVisible) {
            hide();
        } else {
            show();
            delayedHide(5000);
        }
    }

    private void hide() {
        // Hide UI first
        ActionBar actionBar = getSupportActionBar();
        if (actionBar != null) {
            actionBar.hide();
        }
        mControlsView.setVisibility(View.GONE);
        mVisible = false;

        // Schedule a runnable to remove the status and navigation bar after a delay
        mHideHandler.removeCallbacks(mShowPart2Runnable);
        mHideHandler.postDelayed(mHidePart2Runnable, UI_ANIMATION_DELAY);
    }

    @SuppressLint("InlinedApi")
    private void show() {
        // Show the system bar
        mContentView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION);
        mVisible = true;

        // Schedule a runnable to display UI elements after a delay
        mHideHandler.removeCallbacks(mHidePart2Runnable);
        mHideHandler.postDelayed(mShowPart2Runnable, UI_ANIMATION_DELAY);
    }

    /**
     * Schedules a call to hide() in delay milliseconds, canceling any
     * previously scheduled calls.
     */
    private void delayedHide(int delayMillis) {
        mHideHandler.removeCallbacks(mHideRunnable);
        mHideHandler.postDelayed(mHideRunnable, delayMillis);
    }
    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Point pt = mSurfaceView.getVideoSize();
        Log.i("Tag",pt.toString());
    }
    @Override
    public void surfaceChanged(SurfaceHolder holder
            , int format, int width, int height) {
        Point pt = mSurfaceView.getVideoSize();
        Log.i("Tag",pt.toString());
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
        Log.i("org.anyrtc.anyrtmp",String.format("Status TotalTime:%d",totalTime));
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
                //mwvMediaList.loadUrl(getResources().getString(R.string.app_uri) + "#/pages/chatroom/chatroom");
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

}
