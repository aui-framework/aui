//
// Created by alex2 on 04.01.2021.
//

#pragma once

enum class Visibility {

    /**
     * AView виден и активен
     */
    VISIBLE,

    /**
     * AView невидим, но мышкой кликнуть можно
     */
    INVISIBLE,

    /**
     * AView невидим и мышкой попасть невозможно
     */
    GONE
};