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
import android.view.GestureDetector
import android.view.MotionEvent
import android.view.View
import android.view.View.OnTouchListener
import android.widget.OverScroller
import android.widget.Scroller
import java.util.*
import java.util.concurrent.LinkedBlockingDeque
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class MyGLSurfaceView(context: Context) : GLSurfaceView(context), OnTouchListener {
    private val mGestureDetector: GestureDetector
    private var mRenderer: Renderer? = null
    private val mScroller = Scroller(context)

    companion object {
        var ourCurrentSurface: MyGLSurfaceView? = null


        @JvmStatic
        private external fun handleMouseButtonDown(x: Int, y: Int)

        @JvmStatic
        private external fun handleMouseButtonUp(x: Int, y: Int)

        @JvmStatic
        private external fun handleMouseMove(x: Int, y: Int)

        @JvmStatic
        private external fun handleKineticScroll(x: Int, y: Int)

        @JvmStatic
        private external fun handleScroll(originX: Int, originY: Int, velX: Float, velY: Float)

        @JvmStatic
        private external fun handleInit(internalStoragePath: String)

        @JvmStatic
        private external fun handleRedraw()

        @JvmStatic
        private external fun handleResize(width: Int, height: Int)
    }

    init {
        ourCurrentSurface = this
        mGestureDetector = GestureDetector(context, object : GestureDetector.OnGestureListener {
            override fun onDown(motionEvent: MotionEvent): Boolean {
                mScroller.forceFinished(true)
                return true
            }

            override fun onShowPress(motionEvent: MotionEvent) {}
            override fun onSingleTapUp(motionEvent: MotionEvent): Boolean {
                return false
            }

            override fun onScroll(
                motionEvent: MotionEvent,
                motionEvent1: MotionEvent,
                velX: Float,
                velY: Float
            ): Boolean {
                handleScroll(
                    motionEvent.x.toInt(),
                    motionEvent.y.toInt(),
                    velX,
                    velY
                )
                return true
            }

            override fun onLongPress(motionEvent: MotionEvent) {}
            override fun onFling(
                start: MotionEvent,
                current: MotionEvent,
                velX: Float,
                velY: Float
            ): Boolean {
                mScroller.fling(start.x.toInt(), start.y.toInt(),
                                velX.toInt(), velY.toInt(),
                                0, 999999999,
                                0, 999999999)
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
                if (mScroller.computeScrollOffset()) {
                    handleKineticScroll(mScroller.currX, mScroller.currY)

                    handleRedraw()
                    requestRender()
                } else {
                    handleRedraw()
                }
            }
        }.also { mRenderer = it })
        renderMode = RENDERMODE_CONTINUOUSLY
        setOnTouchListener(this)
    }

    override fun onTouch(v: View, event: MotionEvent): Boolean {
        val x = event.x.toInt()
        val y = event.y.toInt()
        when (event.action) {
            MotionEvent.ACTION_DOWN -> handleMouseButtonDown(x, y)
            MotionEvent.ACTION_MOVE -> handleMouseMove(x, y)
            MotionEvent.ACTION_UP -> handleMouseButtonUp(x, y)
        }
        mGestureDetector.onTouchEvent(event)
        return true
    }

}