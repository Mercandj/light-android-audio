package com.mercandalli.android.apps.audio.spectrum;

import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;

/* package */
class ContextFactory implements GLSurfaceView.EGLContextFactory {

    private final static int EGL_CONTEXT_CLIENT_VERSION = 0x3098;

    private final static int GL_VERSION = 2;

    @Override
    public EGLContext createContext(EGL10 egl, EGLDisplay display, EGLConfig config) {
        final int[] attrib_list = {EGL_CONTEXT_CLIENT_VERSION, GL_VERSION, EGL10.EGL_NONE};
        return egl.eglCreateContext(display, config, EGL10.EGL_NO_CONTEXT, attrib_list);
    }

    @Override
    public void destroyContext(EGL10 egl, EGLDisplay display, EGLContext context) {
        egl.eglDestroyContext(display, context);
    }
}