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

import android.content.Context
import android.opengl.GLSurfaceView
import android.util.Log
import android.view.GestureDetector
import android.view.MotionEvent
import android.view.View
import android.view.View.OnTouchListener
import android.widget.Scroller
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class AuiView(context: Context) : GLSurfaceView(context) {
    private val mGestureDetector: GestureDetector
    private var mRenderer: Renderer? = null

    companion object {
        var ourCurrentSurface: AuiView? = null


        @JvmStatic
        private external fun handlePointerButtonDown(x: Float, y: Float, pointerId: Int)

        @JvmStatic
        private external fun handlePointerButtonUp(x: Float, y: Float, pointerId: Int)

        @JvmStatic
        private external fun handlePointerMove(x: Float, y: Float, pointerId: Int)

        @JvmStatic
        private external fun handleInit(internalStoragePath: String)

        @JvmStatic
        private external fun handleRedraw()

        @JvmStatic
        private external fun handleResize(width: Int, height: Int)

        @JvmStatic
        private external fun handleLongPress(width: Int, height: Int)
    }

    init {
        ourCurrentSurface = this
        mGestureDetector = GestureDetector(context, object : GestureDetector.OnGestureListener {
            override fun onDown(motionEvent: MotionEvent): Boolean {
                return true
            }

            override fun onScroll(
                e1: MotionEvent?,
                e2: MotionEvent,
                distanceX: Float,
                distanceY: Float
            ): Boolean {
                return true
            }


            override fun onShowPress(motionEvent: MotionEvent) {}
            override fun onSingleTapUp(motionEvent: MotionEvent): Boolean {
                return false
            }

            override fun onLongPress(event: MotionEvent) {
                handleLongPress(event.x.toInt(), event.y.toInt())
            }
            override fun onFling(
                start: MotionEvent?,
                current: MotionEvent,
                velX: Float,
                velY: Float
            ): Boolean {
                return false
            }
        })

        setEGLContextClientVersion(2)
        setEGLConfigChooser(8, 8, 8, 8, 16, 8)
        val internalStoragePath = context.filesDir.path
        setRenderer(object : Renderer {
            override fun onSurfaceCreated(gl: GL10, config: EGLConfig) {
                handleInit(internalStoragePath)
            }

            override fun onSurfaceChanged(gl: GL10, width: Int, height: Int) {
                handleResize(width, height)
            }

            override fun onDrawFrame(gl: GL10) {
                handleRedraw()
            }
        }.also { mRenderer = it })
        renderMode = RENDERMODE_CONTINUOUSLY
    }

    override fun onTouchEvent(event: MotionEvent?): Boolean {
        if (event == null) {
            return true
        }
        val index = event.actionIndex
        when (event.action and MotionEvent.ACTION_MASK) {
            MotionEvent.ACTION_DOWN, MotionEvent.ACTION_POINTER_DOWN -> handlePointerButtonDown(event.getX(index), event.getY(index), event.getPointerId(index))
            MotionEvent.ACTION_MOVE -> {
                for (i in 0 until event.pointerCount) {
                    handlePointerMove(event.getX(i), event.getY(i), event.getPointerId(i))
                }
            }
            MotionEvent.ACTION_UP, MotionEvent.ACTION_POINTER_UP -> handlePointerButtonUp(event.getX(index), event.getY(index), event.getPointerId(index))
        }
        mGestureDetector.onTouchEvent(event)
        return true
    }
}