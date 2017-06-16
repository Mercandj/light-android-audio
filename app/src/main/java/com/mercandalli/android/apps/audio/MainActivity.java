package com.mercandalli.android.apps.audio;

import android.Manifest;
import android.annotation.TargetApi;
import android.content.pm.PackageManager;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.os.Build;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

import com.mercandalli.android.apps.audio.utils.AudioFeaturesManager;
import com.mercandalli.android.apps.audio.utils.FindTrackManager;
import com.mercandalli.android.sdk.audio.SoundSystem;
import com.mercandalli.android.sdk.audio.listener.SSExtractionObserver;
import com.mercandalli.android.sdk.audio.listener.SSPlayingStatusObserver;

/**
 * Simple activity launching the sound system.
 */
public class MainActivity extends AppCompatActivity {
    private static final int MY_PERMISSIONS_REQUEST_READ_EXTERNAL_STORAGE = 1;

    /**
     * UI
     */
    private Button toggleStop;
    private Button btnExtractFile;
    private ToggleButton togglePlayPause;

    private long extractionStartTimestamp;
    private long extractionDusration;

    /**
     * Sound system
     */
    private SoundSystem mSoundSystem;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        final AudioFeaturesManager audioFeaturesManager = AudioFeaturesManager.init(this);

        mSoundSystem = SoundSystem.getInstance();
        if (!mSoundSystem.isSoundSystemInit()) {
            mSoundSystem.initSoundSystem(
                    audioFeaturesManager.getSampleRate(),
                    audioFeaturesManager.getFramesPerBuffer());
        }

        initUI();

        attachToListeners();

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            displayAvailableAudioCodecs();
        }
    }

    private void initUI() {
        // extract button
        btnExtractFile = (Button) findViewById(R.id.toggle_extract_file);
        btnExtractFile.setOnClickListener(mOnClickListener);

        // play pause button
        togglePlayPause = (ToggleButton) findViewById(R.id.toggle_play_pause);
        togglePlayPause.setOnCheckedChangeListener(mOnCheckedChangeListener);

        // stop button
        toggleStop = (Button) findViewById(R.id.btn_stop);
        toggleStop.setOnClickListener(mOnClickListener);

        if (mSoundSystem.isLoaded()) {
            togglePlayPause.setEnabled(true);
            toggleStop.setEnabled(true);
            btnExtractFile.setEnabled(false);
        } else {
            btnExtractFile.setEnabled(true);
            togglePlayPause.setEnabled(false);
            toggleStop.setEnabled(false);
        }

        togglePlayPause.setChecked(mSoundSystem.isPlaying());
    }

    @Override
    protected void onDestroy() {
        if (!isChangingConfigurations()) {
            detachListeners();
            mSoundSystem.release();
        }
        super.onDestroy();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {
        if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            mSoundSystem.loadFile(FindTrackManager.getTrackPath(MainActivity.this).getPath());
        } else {
            Toast.makeText(this, "No permission, no extraction !", Toast.LENGTH_SHORT).show();
        }
    }

    private void attachToListeners() {
        mSoundSystem.addPlayingStatusObserver(mSSPlayingStatusObserver);
        mSoundSystem.addExtractionObserver(mSSExtractionObserver);
    }

    private void detachListeners() {
        mSoundSystem.removePlayingStatusObserver(mSSPlayingStatusObserver);
        mSoundSystem.removeExtractionObserver(mSSExtractionObserver);
    }

    @TargetApi(Build.VERSION_CODES.LOLLIPOP)
    private void displayAvailableAudioCodecs() {
        final TextView tvAvailableCodecs = (TextView) findViewById(R.id.tv_available_codecs);

        final MediaCodecList mediaCodecList = new MediaCodecList(MediaCodecList.REGULAR_CODECS);
        final MediaCodecInfo[] codecInfos = mediaCodecList.getCodecInfos();
        final StringBuilder stringBuilder = new StringBuilder();
        for (final MediaCodecInfo codecInfo : codecInfos) {
            if (!codecInfo.isEncoder()) {
                boolean isAudioCodec = false;
                for (final String type : codecInfo.getSupportedTypes()) {
                    if (type.contains("audio/")) {
                        isAudioCodec = true;
                    }
                }
                if (isAudioCodec) {
                    stringBuilder.append(codecInfo.getName()).append(" : \n");
                    for (final String type : codecInfo.getSupportedTypes()) {
                        if (type.contains("audio/")) {
                            stringBuilder.append(type).append("\n");
                        }
                    }
                }
            }
        }

        tvAvailableCodecs.setText(stringBuilder.toString());
    }

    private void log(String message) {
        final TextView textView = (TextView) findViewById(R.id.tv_logs);
        textView.setText(message + "\n" + textView.getText().toString());
    }

    private SSExtractionObserver mSSExtractionObserver = new SSExtractionObserver() {
        @Override
        public void onExtractionStarted() {
            extractionStartTimestamp = System.currentTimeMillis();
            btnExtractFile.setEnabled(false);
            log("Extraction started");
        }

        @Override
        public void onExtractionCompleted() {
            extractionDusration = System.currentTimeMillis() - extractionStartTimestamp;
            togglePlayPause.setEnabled(true);
            toggleStop.setEnabled(true);
            log("Extraction ended");
            log("Extraction duration " + (extractionDusration / 1_000f) + "s");
        }
    };

    private SSPlayingStatusObserver mSSPlayingStatusObserver = new SSPlayingStatusObserver() {
        @Override
        public void onPlayingStatusDidChange(final boolean playing) {
            log(playing ? "Playing" : "Pause");
        }

        @Override
        public void onEndOfMusic() {
            togglePlayPause.setChecked(false);
            log("Track finished");
        }

        @Override
        public void onStopTrack() {
            togglePlayPause.setChecked(false);
            log("Track stopped");
        }
    };

    private View.OnClickListener mOnClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            switch (v.getId()) {
                case R.id.toggle_extract_file:
                    loadTrackOrAskPermission();
                    break;
                case R.id.btn_stop:
                    mSoundSystem.stopMusic();
                    break;
            }
        }
    };

    private void loadTrackOrAskPermission() {
        final int permissionCheck = ContextCompat.checkSelfPermission(MainActivity.this, Manifest.permission.READ_EXTERNAL_STORAGE);
        if (permissionCheck == PackageManager.PERMISSION_GRANTED) {
            mSoundSystem.loadFile(FindTrackManager.getTrackPath(MainActivity.this).getPath());
        } else {
            askForReadExternalStoragePermission();
        }
    }

    private void askForReadExternalStoragePermission() {
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this,
                    new String[]{Manifest.permission.READ_EXTERNAL_STORAGE},
                    MY_PERMISSIONS_REQUEST_READ_EXTERNAL_STORAGE);
        }
    }

    private CompoundButton.OnCheckedChangeListener mOnCheckedChangeListener = new CompoundButton.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            switch (buttonView.getId()) {
                case R.id.toggle_play_pause:
                    mSoundSystem.playMusic(isChecked);
                    break;
            }
        }
    };
}
