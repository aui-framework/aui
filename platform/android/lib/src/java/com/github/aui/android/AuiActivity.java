package com.github.aui.android;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.os.Build;
import android.os.Bundle;
import android.view.WindowManager;

public class AuiActivity extends AppCompatActivity {
    private String getApplicationName() {
        Context ctx = getApplicationContext();
        ApplicationInfo applicationInfo = ctx.getApplicationInfo();
        int id = applicationInfo.labelRes;
        return id == 0 ? applicationInfo.nonLocalizedLabel.toString() : ctx.getString(id);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getWindow().setFlags(WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS,
                WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS);

        AUI.ourContext = this;

        MyGLSurfaceView surfaceView = new MyGLSurfaceView(this);
        setContentView(surfaceView);
    }
}
