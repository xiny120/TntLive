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
package org.anyrtc.core;

import android.app.Activity;

import org.webrtc.EglBase;

/**
 * Created by Eric on 2016/7/28.
 */
public class RTMPGuestKit {
    private static final String TAG = "RTMPGuestKit";
    /**
     * 构造访问jni底层库的对象
     */
    private long fNativeAppId;
    private Activity mActivity;
    private RTMPGuestHelper mGuestHelper;
    private final LooperExecutor mExecutor;
    private final EglBase mEglBase;

    public RTMPGuestKit(Activity act, final RTMPGuestHelper guestHelper) {
        RTMPUtils.assertIsTrue(act != null && guestHelper != null);
        mActivity = act;
        mGuestHelper = guestHelper;
        AnyRTMP.Inst().Init(mActivity.getApplicationContext());

        mExecutor = AnyRTMP.Inst().Executor();
        mEglBase = AnyRTMP.Inst().Egl();

        mExecutor.execute(new Runnable() {
            @Override
            public void run() {
                fNativeAppId = nativeCreate(guestHelper);
            }
        });
    }

    public void Clear() {
        mExecutor.execute(new Runnable() {
            @Override
            public void run() {
                nativeStopRtmpPlay();
                nativeDestroy();
            }
        });
    }


    //* Rtmp function for pull rtmp stream 
    public void StartRtmpPlay(final String strUrl, final long renderPointer, final  String type_, final String dir_, final int enc, final String userid, final short [][] marker, final int[] len) {
        mExecutor.execute(new Runnable() {
            @Override
            public void run() {
                nativeStartRtmpPlay(strUrl, renderPointer,type_,dir_,enc,userid,marker,len);
            }
        });
    }
    public void StopRtmpPlay() {
        mExecutor.execute(new Runnable() {
            @Override
            public void run() {
                nativeStopRtmpPlay();
            }
        });
    }
    public void SeekTo(final  long p1,final double p2) {
        mExecutor.execute(new Runnable() {
            @Override
            public void run() {
                nativeSeekTo(p1,p2);
            }
        });
    }

    /**
     *  Native function
     */
    private native long nativeCreate(Object obj);
    private native void nativeStartRtmpPlay(final String strUrl, final long renderPointer,final  String type_,final String dir_, final int enc, final String userid, final short [][] marker, final int[] len);
    private native void nativeSeekTo(final  long p1,final double p2);
    private native void nativeStopRtmpPlay();
    private native void nativeDestroy();
}
