// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772, Magicsea
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.
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
                e2: MotionEvent?,
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
                start: MotionEvent,
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
            MotionEvent.ACTION_DOWN, MotionEvent.ACTION_POINTER_DOWN -> handlePointerButtonDown(event.getX(index), event.getY(index), index)
            MotionEvent.ACTION_MOVE -> {
                for (i in 0 until event.pointerCount) {
                    handlePointerMove(event.getX(i), event.getY(i), i)
                }
            }
            MotionEvent.ACTION_UP -> handlePointerButtonUp(event.getX(index), event.getY(index), index)
        }
        mGestureDetector.onTouchEvent(event)
        return true
    }
}