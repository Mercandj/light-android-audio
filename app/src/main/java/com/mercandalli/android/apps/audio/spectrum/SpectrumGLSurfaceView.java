package com.mercandalli.android.apps.audio.spectrum;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;


public class SpectrumGLSurfaceView extends GLSurfaceView {

    @SuppressWarnings("unused")
    private static final String TAG = "SpectrumGLSurfaceView";

    private SpectrumRenderer mRenderer;

    public SpectrumGLSurfaceView(Context context) {
        super(context);
    }

    public SpectrumGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);

        // use OpenGL ES version 2
        setEGLContextClientVersion(2);

        // can only be call before setRenderer
        setEGLContextFactory(new ContextFactory());
        //setEGLConfigChooser(8, 8, 8, 8, 16, 8);

        mRenderer = new SpectrumRenderer();
        setRenderer(mRenderer);

        // can only be call after setRenderer
        // redraw the GLView at each frame, even if there is no change.
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }

    public void drawData(short[] data, int desiredNumberData){
        mRenderer.drawData(data, desiredNumberData);
    }

    @Override
    public void onResume() {
        super.onResume();
        Log.i(TAG, "onResume");
    }

    @Override
    public void onPause() {
        super.onPause();
        Log.i(TAG, "onPause");
    }

}
