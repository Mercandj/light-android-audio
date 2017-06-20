package com.mercandalli.android.apps.audio;

import android.Manifest;
import android.annotation.TargetApi;
import android.content.pm.PackageManager;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.IntDef;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

import com.mercandalli.android.apps.audio.utils.AudioFeaturesManager;
import com.mercandalli.android.apps.audio.utils.FindTrackManager;
import com.mercandalli.android.apps.audio.utils.Track;
import com.mercandalli.android.sdk.audio.SoundSystem;
import com.mercandalli.android.sdk.audio.listener.SSExtractionObserver;
import com.mercandalli.android.sdk.audio.listener.SSPlayingStatusObserver;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

/**
 * Simple activity launching the sound system.
 */
public class MainActivity extends AppCompatActivity {

    @Retention(RetentionPolicy.SOURCE)
    @IntDef({ACTION_NO_ACTION, ACTION_LOAD_FILE, ACTION_LOAD_FILE_FFMPEG})
    @interface Action {
    }

    private static final int ACTION_NO_ACTION = 0;
    private static final int ACTION_LOAD_FILE = 1;
    private static final int ACTION_LOAD_FILE_FFMPEG = 2;

    private static final int MY_PERMISSIONS_REQUEST_READ_EXTERNAL_STORAGE = 1;

    /**
     * UI
     */
    private Button toggleStop;
    private Button btnExtractFile;
    private Button btnExtractFfmpeg;
    private ToggleButton togglePlayPause;

    private long extractionStartTimestamp;
    private long extractionDusration;

    /**
     * Sound system
     */
    private SoundSystem soundSystem;

    @Action
    private int action = ACTION_NO_ACTION;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        setSupportActionBar((Toolbar) findViewById(R.id.activity_main_toolbar));

        final AudioFeaturesManager audioFeaturesManager = AudioFeaturesManager.init(this);

        soundSystem = SoundSystem.getInstance();
        if (!soundSystem.isSoundSystemInit()) {
            soundSystem.initSoundSystem(
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
        btnExtractFile = (Button) findViewById(R.id.btn_extract_file);
        btnExtractFile.setOnClickListener(mOnClickListener);

        btnExtractFfmpeg = (Button) findViewById(R.id.btn_extract_file_ffmpeg);
        btnExtractFfmpeg.setOnClickListener(mOnClickListener);

        // play pause button
        togglePlayPause = (ToggleButton) findViewById(R.id.toggle_play_pause);
        togglePlayPause.setOnCheckedChangeListener(mOnCheckedChangeListener);

        // stop button
        toggleStop = (Button) findViewById(R.id.btn_stop);
        toggleStop.setOnClickListener(mOnClickListener);

        if (soundSystem.isLoaded()) {
            togglePlayPause.setEnabled(true);
            toggleStop.setEnabled(true);
            btnExtractFile.setEnabled(false);
            btnExtractFfmpeg.setEnabled(false);
        } else {
            btnExtractFile.setEnabled(true);
            btnExtractFfmpeg.setEnabled(true);
            togglePlayPause.setEnabled(false);
            toggleStop.setEnabled(false);
        }

        togglePlayPause.setChecked(soundSystem.isPlaying());
    }

    @Override
    protected void onDestroy() {
        if (!isChangingConfigurations()) {
            detachListeners();
            soundSystem.release();
        }
        super.onDestroy();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {
        if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            switch (action) {
                case ACTION_LOAD_FILE_FFMPEG:
                    soundSystem.loadFileWithFfmpeg(FindTrackManager.getTrackPath(MainActivity.this).getPath());
                    break;
                case ACTION_LOAD_FILE:
                    soundSystem.loadFile(FindTrackManager.getTrackPath(MainActivity.this).getPath());
                    break;
                case ACTION_NO_ACTION:
                    break;
            }
            action = ACTION_NO_ACTION;
        } else {
            Toast.makeText(this, "No permission, no extraction !", Toast.LENGTH_SHORT).show();
        }
    }

    private void attachToListeners() {
        soundSystem.addPlayingStatusObserver(mSSPlayingStatusObserver);
        soundSystem.addExtractionObserver(mSSExtractionObserver);
    }

    private void detachListeners() {
        soundSystem.removePlayingStatusObserver(mSSPlayingStatusObserver);
        soundSystem.removeExtractionObserver(mSSExtractionObserver);
    }

    @TargetApi(Build.VERSION_CODES.LOLLIPOP)
    private void displayAvailableAudioCodecs() {
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
        log("Available codecs : \n" + stringBuilder.toString());
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
            btnExtractFfmpeg.setEnabled(false);
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
                case R.id.btn_extract_file:
                    loadTrackOrAskPermission();
                    break;

                case R.id.btn_extract_file_ffmpeg:
                    loadTrackWithFfmpefOrAskPermission();
                    break;

                case R.id.btn_stop:
                    soundSystem.stopMusic();
                    break;
            }
        }
    };

    private void loadTrackOrAskPermission() {
        final int permissionCheck = ContextCompat.checkSelfPermission(MainActivity.this, Manifest.permission.READ_EXTERNAL_STORAGE);
        if (permissionCheck == PackageManager.PERMISSION_GRANTED) {
            soundSystem.loadFile(FindTrackManager.getTrackPath(MainActivity.this).getPath());
        } else {
            action = ACTION_LOAD_FILE;
            askForReadExternalStoragePermission();
        }
    }

    private void loadTrackWithFfmpefOrAskPermission() {
        int permissionCheck = ContextCompat.checkSelfPermission(MainActivity.this, Manifest.permission.READ_EXTERNAL_STORAGE);
        if (permissionCheck == PackageManager.PERMISSION_GRANTED) {
            Track trackPath = FindTrackManager.getTrackPath(MainActivity.this);
            String path = trackPath.getPath();
            soundSystem.loadFileWithFfmpeg(path);
        } else {
            action = ACTION_LOAD_FILE_FFMPEG;
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
                    soundSystem.playMusic(isChecked);
                    break;
            }
        }
    };
}
