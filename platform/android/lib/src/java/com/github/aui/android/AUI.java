package com.github.aui.android;

import android.content.Context;
import android.content.DialogInterface;

import androidx.appcompat.app.AlertDialog;

public class AUI {
    public static AuiActivity ourContext;

    public static void showMessageBox(final String title, final String message) {
        ourContext.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                new AlertDialog.Builder(ourContext).setMessage(message).setTitle(title)
                        .setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {

                            }
                        }).show();
            }
        });
    }


    private static void requestRedraw() {
        MyGLSurfaceView.getLastSurface().requestRender();
    }
    private static void showKeyboard() {
        // TODO
    }

    private static void hideKeyboard() {
        // TODO
    }

    public static float getDpiRatio() {
        return ourContext.getResources().getDisplayMetrics().density * 1.2f;
    }
}
