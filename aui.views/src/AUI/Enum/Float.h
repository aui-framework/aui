#pragma once


enum class Float {
    /**
     * Entry's default position behaviour.
     */
    NONE,

    /**
     * Entry is placed on the row where's it appeared. Entry is snapped to the left border. Other entries are
     * wrapped around this entry.
     */
    LEFT,

    /**
     * Entry is placed on the row where's it appeared. Entry is snapped to the right border. Other entries are
     * wrapped around this entry.
     */
    RIGHT
};