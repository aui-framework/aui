package com.github.aui.android

import android.app.Activity
import android.content.Intent
import android.net.Uri
import androidx.appcompat.app.AlertDialog

object Platform {
    @JvmField
    var ourContext: Activity? = null

    @JvmStatic
    fun showMessageBox(title: String, message: String) {
        ourContext?.runOnUiThread {
            AlertDialog.Builder(ourContext!!).setMessage(message).setTitle(title)
                .setPositiveButton("Ok") { dialog, which -> }.show()
        }
    }

    @JvmStatic
    private fun requestRedraw() {
        AuiView.ourCurrentSurface?.requestRender()
    }

    @JvmStatic
    private fun showKeyboard() {
        // TODO
    }

    @JvmStatic
    private fun hideKeyboard() {
        // TODO
    }
    @JvmStatic
    private fun openUrl(url: String) {
        val intent = Intent(Intent.ACTION_VIEW, Uri.parse(url))
        AuiView.ourCurrentSurface?.context?.startActivity(intent)
    }

    @JvmStatic
    fun getDpiRatio(): Float = ourContext?.resources?.displayMetrics?.density ?: 2.0f
}