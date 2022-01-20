package com.github.aui.android;

import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MyGLRenderer implements GLSurfaceView.Renderer {
    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        handleInit();
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        handleResize(width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        handleRedraw();
    }

    private static native void handleInit();
    private static native void handleRedraw();
    private static native void handleResize(int width, int height);
}
