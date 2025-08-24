/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors, Magicsea
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package com.github.aui.android

import android.app.Activity
import android.content.Intent
import android.net.Uri
import android.text.InputType
import android.view.inputmethod.EditorInfo
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
    fun copyToClipboard(text: String) {
    }

    @JvmStatic
    private fun showKeyboard(iKeyboardType: Int, iKeyboardAction: Int, isPassword: Boolean) {
        // TODO
        var keyboardType = when (iKeyboardType) {
            /* EMAIL */ 1 -> InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_VARIATION_EMAIL_ADDRESS
            /* MULTILINE */ 2 -> InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_FLAG_MULTI_LINE
            /* NUMBER */ 3 -> InputType.TYPE_CLASS_NUMBER
            /* URL */ 4 -> InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_VARIATION_URI
            /* default */ else -> InputType.TYPE_CLASS_TEXT
        }
        if (isPassword) {
            keyboardType = keyboardType or InputType.TYPE_TEXT_VARIATION_PASSWORD
        }

        val keyboardAction = when (iKeyboardAction) {
            /* DONE */ 1 -> EditorInfo.IME_ACTION_DONE
            /* GO */ 2 -> EditorInfo.IME_ACTION_GO
            /* SEND */ 3 -> EditorInfo.IME_ACTION_SEND
            /* SEARCH */ 4 -> EditorInfo.IME_ACTION_SEARCH
            /* NEXT */ 5 -> EditorInfo.IME_ACTION_NEXT
            /* default */ else -> EditorInfo.IME_ACTION_UNSPECIFIED
        }
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