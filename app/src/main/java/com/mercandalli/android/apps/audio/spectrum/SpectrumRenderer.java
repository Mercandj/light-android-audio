package com.mercandalli.android.apps.audio.spectrum;


import android.opengl.GLES20;
import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/* package */
class SpectrumRenderer implements GLSurfaceView.Renderer {

    @SuppressWarnings("unused")
    private static final String TAG = "SpectrumRenderer";

    private Line mLine;

    @Override
    public void onSurfaceCreated(GL10 gl10, EGLConfig eglConfig) {
        GLES20.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        mLine = new Line();
    }

    @Override
    public void onSurfaceChanged(GL10 gl10, int width, int height) {
        GLES20.glViewport(0, 0, width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl10) {
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);

        mLine.draw();
    }

    /* package */
    void drawData(short[] data, int desiredNumberData){
        mLine.drawData(data, desiredNumberData);
    }
}
