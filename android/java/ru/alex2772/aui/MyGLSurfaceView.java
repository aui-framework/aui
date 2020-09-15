package ru.alex2772.aui;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.view.MotionEvent;
import android.view.View;

import static android.view.MotionEvent.ACTION_DOWN;
import static android.view.MotionEvent.ACTION_UP;

public class MyGLSurfaceView extends GLSurfaceView implements View.OnTouchListener {
    private static MyGLSurfaceView mLastSurface;

    private final MyGLRenderer mRenderer;

    public MyGLSurfaceView(Context context) {
        super(context);
        mLastSurface = this;
        setEGLContextClientVersion(3);
        setEGLConfigChooser(8, 8, 8, 8, 16, 8);
        setRenderer(mRenderer = new MyGLRenderer());
        setRenderMode(RENDERMODE_WHEN_DIRTY);
        setOnTouchListener(this);
    }

    private static void requestRedraw() {
        mLastSurface.requestRender();
    }


    private static native void handleMouseButtonDown(int x, int y);
    private static native void handleMouseButtonUp(int x, int y);

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        switch (event.getAction()) {
            case ACTION_DOWN:
                handleMouseButtonDown((int) event.getX(), (int) event.getY());
                break;
            case ACTION_UP:
                handleMouseButtonUp((int) event.getX(), (int) event.getY());
                break;
        }
        return true;
    }
}
