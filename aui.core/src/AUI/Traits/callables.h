//
// Created by Alex2772 on 11/19/2021.
//

#pragma once

namespace aui {
    /**
     * That `<code>overloaded</code>` trick
     */
    template<class... Lambdas>
    struct lambda_overloaded : Lambdas... {
        using Lambdas::operator()...;
    };

    // deduction guide
    template<class... Lambdas>
    lambda_overloaded(Lambdas...) -> lambda_overloaded<Lambdas...>;
}