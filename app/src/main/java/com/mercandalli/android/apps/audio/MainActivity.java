package com.mercandalli.android.apps.audio;

import android.os.Bundle;
import android.support.v4.view.MotionEventCompat;
import android.support.v7.app.AppCompatActivity;
import android.view.MotionEvent;

import com.mercandalli.android.sdk.audio.SoundSystem;
import com.mercandalli.android.sdk.audio.SoundSystemNative;

public class MainActivity extends AppCompatActivity {

    private SoundSystem soundSystem = new SoundSystemNative();
    private boolean created;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // initialize native audio system
        //created = soundSystem.create();
    }

    @Override
    protected void onDestroy() {
        soundSystem.destroy();
        super.onDestroy();
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        /*if (!created) {
            return super.onTouchEvent(event);
        }*/
        created = soundSystem.create();
        int action = MotionEventCompat.getActionMasked(event);
        switch (action) {
            case (MotionEvent.ACTION_DOWN):
                soundSystem.start();
                break;
            case (MotionEvent.ACTION_UP):
                soundSystem.stop();
                break;
        }
        return super.onTouchEvent(event);
    }
}
