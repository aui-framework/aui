package com.github.aui.android;

import android.content.Context;
import android.content.DialogInterface;

import androidx.appcompat.app.AlertDialog;

public class AUI {
    public static MainActivity ourContext;

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

    public static float getDpiRatio() {
        return ourContext.getResources().getDisplayMetrics().density * 1.2f;
    }
}
