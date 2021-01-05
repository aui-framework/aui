//
// Created by alex2 on 04.01.2021.
//

#pragma once

enum class Visibility {

    /**
     * AView is visible and active
     */
    VISIBLE,

    /**
     * AView is invisible but still interacting to the mouse
     */
    INVISIBLE,

    /**
     * AView is invisible and does not interact with the mouse
     */
    GONE
};