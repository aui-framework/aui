package com.github.aui.android;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.View;

import static android.view.MotionEvent.*;

import java.util.NoSuchElementException;
import java.util.Queue;
import java.util.concurrent.LinkedBlockingDeque;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MyGLSurfaceView extends GLSurfaceView implements View.OnTouchListener {
    private static MyGLSurfaceView ourLastSurface;
    private GestureDetector mGestureDetector;

    private final GLSurfaceView.Renderer mRenderer;
    private static Queue<Runnable> ourOnUiThread = new LinkedBlockingDeque<Runnable>();

    public MyGLSurfaceView(Context context) {
        super(context);
        mGestureDetector = new GestureDetector(context, new GestureDetector.OnGestureListener() {
            @Override
            public boolean onDown(MotionEvent motionEvent) {
                return false;
            }

            @Override
            public void onShowPress(MotionEvent motionEvent) {

            }

            @Override
            public boolean onSingleTapUp(MotionEvent motionEvent) {
                return false;
            }

            @Override
            public boolean onScroll(MotionEvent motionEvent, MotionEvent motionEvent1, float velX, float velY) {
                onUiThread(() -> handleScroll((int)motionEvent.getX(), (int)motionEvent.getY(), velX, velY));
                return true;
            }

            @Override
            public void onLongPress(MotionEvent motionEvent) {

            }

            @Override
            public boolean onFling(MotionEvent start, MotionEvent current, float velX, float velY) {
                return false;
            }
        });
        ourLastSurface = this;
        setEGLContextClientVersion(2);
        setEGLConfigChooser(8, 8, 8, 8, 16, 8);
        String internalStoragePath = context.getFilesDir().path();
        setRenderer(mRenderer = new GLSurfaceView.Renderer() {

            @Override
            public void onSurfaceCreated(GL10 gl, EGLConfig config) {
                handleInit(internalStoragePath);
            }

            @Override
            public void onSurfaceChanged(GL10 gl, int width, int height) {
                handleResize(width, height);
            }

            @Override
            public void onDrawFrame(GL10 gl) {
                try {
                    for (Runnable i = null; (i = ourOnUiThread.remove()) != null; ) {
                        i.run();
                    }
                } catch (NoSuchElementException ignored) {}
                handleRedraw();
            }

        });
        setRenderMode(RENDERMODE_CONTINUOUSLY);
        setOnTouchListener(this);
    }

    private static void requestRedraw() {
        ourLastSurface.requestRender();
    }

    public static void onUiThread(Runnable runnable) {
        ourOnUiThread.add(runnable);
        ourLastSurface.requestRender();
    }

    private static native void handleMouseButtonDown(int x, int y);

    private static native void handleMouseButtonUp(int x, int y);
    private static native void handleMouseMove(int x, int y);
    private static native void handleScroll(int originX, int originY, float velX, float velY);

    private static native void handleInit(String internalStoragePath);
    private static native void handleRedraw();
    private static native void handleResize(int width, int height);

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        int x = (int) event.getX();
        int y = (int) event.getY();
        switch (event.getAction()) {
            case ACTION_DOWN:
                onUiThread(() -> handleMouseButtonDown(x, y));
                break;
            case ACTION_MOVE:
                onUiThread(() -> handleMouseMove(x, y));
                break;
            case ACTION_UP:
                onUiThread(() -> handleMouseButtonUp(x, y));
                break;
        }
        mGestureDetector.onTouchEvent(event);
        return true;
    }

}
