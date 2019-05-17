package org.anyrtc.anyrtmp;


import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class MyBroadcastReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        switch (action) {
            //插入和拔出耳机会触发此广播
            case Intent.ACTION_HEADSET_PLUG:
                //addHint(Intent.ACTION_HEADSET_PLUG);
                int state = intent.getIntExtra("state", 0);
                if (state == 1) {
                    //耳机已插入
                    PlayerManager.getManager().changeToHeadsetMode();
                } else if (state == 0) {
                    //耳机已拔出
                    PlayerManager.getManager().changeToSpeakerMode();
                }
                break;
        }

    }
}
