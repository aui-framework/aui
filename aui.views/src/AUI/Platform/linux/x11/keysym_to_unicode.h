/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#include <X11/Xlib.h>
#include <algorithm>
#include <iterator>
#include <array>
namespace aui::x11 {

/**
 * @brief Converts an X11 KeySym to a Unicode (UTF-32) code point.
 * Follows the X.Org / Unicode Consortium KeySym to Unicode mapping specification.
 */
inline char32_t keysymToUnicode(KeySym keysym) noexcept {
    // Direct Unicode representation in modern X11 (0x01000100 - 0x0110ffff)
    if (keysym >= 0x01000100 && keysym <= 0x0110ffff) {
        return static_cast<char32_t>(keysym - 0x01000000);
    }

    // ASCII range (0x0020 - 0x007e)
    if (keysym >= 0x0020 && keysym <= 0x007e) {
        return static_cast<char32_t>(keysym);
    }

    // Latin-1 supplement (0x00a0 - 0x00ff)
    if (keysym >= 0x00a0 && keysym <= 0x00ff) {
        return static_cast<char32_t>(keysym);
    }

    // Cyrillic (0x06a0 - 0x06ff)
    if (keysym >= 0x06a0 && keysym <= 0x06ff) {
        switch (keysym) {
            case 0x06a1: return 0x045e; // Cyrillic_shortu
            case 0x06a2: return 0x0452; // Cyrillic_dje
            case 0x06a3: return 0x0453; // Cyrillic_gje
            case 0x06a4: return 0x0454; // Cyrillic_ie (Ukrainian)
            case 0x06a5: return 0x0455; // Cyrillic_dze
            case 0x06a6: return 0x0456; // Cyrillic_i (Ukrainian)
            case 0x06a7: return 0x0457; // Cyrillic_yi (Ukrainian)
            case 0x06a8: return 0x0458; // Cyrillic_je
            case 0x06a9: return 0x0459; // Cyrillic_lje
            case 0x06aa: return 0x045a; // Cyrillic_nje
            case 0x06ab: return 0x045b; // Cyrillic_tshe
            case 0x06ac: return 0x045c; // Cyrillic_kje
            case 0x06ad: return 0x0491; // Cyrillic_ghe_upturn (small)
            case 0x06ae: return 0x045e; // Cyrillic_shortu
            case 0x06af: return 0x045f; // Cyrillic_dzhe
            case 0x06b0: return 0x2116; // numero
            case 0x06b1: return 0x040e; // Cyrillic_SHORTU
            case 0x06b2: return 0x0402; // Cyrillic_DJE
            case 0x06b3: return 0x0403; // Cyrillic_GJE
            case 0x06b4: return 0x0404; // Cyrillic_IE (Ukrainian)
            case 0x06b5: return 0x0405; // Cyrillic_DZE
            case 0x06b6: return 0x0406; // Cyrillic_I (Ukrainian)
            case 0x06b7: return 0x0407; // Cyrillic_YI (Ukrainian)
            case 0x06b8: return 0x0408; // Cyrillic_JE
            case 0x06b9: return 0x0409; // Cyrillic_LJE
            case 0x06ba: return 0x040a; // Cyrillic_NJE
            case 0x06bb: return 0x040b; // Cyrillic_TSHE
            case 0x06bc: return 0x040c; // Cyrillic_KJE
            case 0x06bd: return 0x0490; // Cyrillic_GHE_upturn (capital)
            case 0x06be: return 0x040e; // Cyrillic_SHORTU
            case 0x06bf: return 0x040f; // Cyrillic_DZHE
            case 0x06c0: return 0x044e; // Cyrillic_yu
            case 0x06c1: return 0x0430; // Cyrillic_a
            case 0x06c2: return 0x0431; // Cyrillic_be
            case 0x06c3: return 0x0446; // Cyrillic_tse
            case 0x06c4: return 0x0434; // Cyrillic_de
            case 0x06c5: return 0x0435; // Cyrillic_ie
            case 0x06c6: return 0x0444; // Cyrillic_ef
            case 0x06c7: return 0x0433; // Cyrillic_ghe
            case 0x06c8: return 0x0445; // Cyrillic_ha
            case 0x06c9: return 0x0438; // Cyrillic_i
            case 0x06ca: return 0x0439; // Cyrillic_shorti
            case 0x06cb: return 0x043a; // Cyrillic_ka
            case 0x06cc: return 0x043b; // Cyrillic_el
            case 0x06cd: return 0x043c; // Cyrillic_em
            case 0x06ce: return 0x043d; // Cyrillic_en
            case 0x06cf: return 0x043e; // Cyrillic_o
            case 0x06d0: return 0x043f; // Cyrillic_pe
            case 0x06d1: return 0x044f; // Cyrillic_ya
            case 0x06d2: return 0x0440; // Cyrillic_er
            case 0x06d3: return 0x0441; // Cyrillic_es
            case 0x06d4: return 0x0442; // Cyrillic_te
            case 0x06d5: return 0x0443; // Cyrillic_u
            case 0x06d6: return 0x0436; // Cyrillic_zhe
            case 0x06d7: return 0x0432; // Cyrillic_ve
            case 0x06d8: return 0x044c; // Cyrillic_softsign
            case 0x06d9: return 0x044b; // Cyrillic_yeru
            case 0x06da: return 0x0437; // Cyrillic_ze
            case 0x06db: return 0x0448; // Cyrillic_sha
            case 0x06dc: return 0x044d; // Cyrillic_e
            case 0x06dd: return 0x0449; // Cyrillic_shcha
            case 0x06de: return 0x0447; // Cyrillic_che
            case 0x06df: return 0x044a; // Cyrillic_hardsign
            case 0x06e0: return 0x042e; // Cyrillic_YU
            case 0x06e1: return 0x0410; // Cyrillic_A
            case 0x06e2: return 0x0411; // Cyrillic_BE
            case 0x06e3: return 0x0426; // Cyrillic_TSE
            case 0x06e4: return 0x0414; // Cyrillic_DE
            case 0x06e5: return 0x0415; // Cyrillic_IE
            case 0x06e6: return 0x0424; // Cyrillic_EF
            case 0x06e7: return 0x0413; // Cyrillic_GHE
            case 0x06e8: return 0x0425; // Cyrillic_HA
            case 0x06e9: return 0x0418; // Cyrillic_I
            case 0x06ea: return 0x0419; // Cyrillic_SHORTI
            case 0x06eb: return 0x041a; // Cyrillic_KA
            case 0x06ec: return 0x041b; // Cyrillic_EL
            case 0x06ed: return 0x041c; // Cyrillic_EM
            case 0x06ee: return 0x041d; // Cyrillic_EN
            case 0x06ef: return 0x041e; // Cyrillic_O
            case 0x06f0: return 0x041f; // Cyrillic_PE
            case 0x06f1: return 0x042f; // Cyrillic_YA
            case 0x06f2: return 0x0420; // Cyrillic_ER
            case 0x06f3: return 0x0421; // Cyrillic_ES
            case 0x06f4: return 0x0422; // Cyrillic_TE
            case 0x06f5: return 0x0423; // Cyrillic_U
            case 0x06f6: return 0x0416; // Cyrillic_ZHE
            case 0x06f7: return 0x0412; // Cyrillic_VE
            case 0x06f8: return 0x042c; // Cyrillic_SOFTSIGN
            case 0x06f9: return 0x042b; // Cyrillic_YERU
            case 0x06fa: return 0x0417; // Cyrillic_ZE
            case 0x06fb: return 0x0428; // Cyrillic_SHA
            case 0x06fc: return 0x042d; // Cyrillic_E
            case 0x06fd: return 0x0429; // Cyrillic_SHCHA
            case 0x06fe: return 0x0427; // Cyrillic_CHE
            case 0x06ff: return 0x042a; // Cyrillic_HARDSIGN
            default: break;
        }
    }

    // Greek (0x07a1 - 0x07f9)
    if (keysym >= 0x07a1 && keysym <= 0x07f9) {
        switch (keysym) {
            case 0x07a1: return 0x0386; // Greek_ALPHAaccent
            case 0x07a2: return 0x0388; // Greek_EPSILONaccent
            case 0x07a3: return 0x0389; // Greek_ETAaccent
            case 0x07a4: return 0x038a; // Greek_IOTAaccent
            case 0x07a5: return 0x03aa; // Greek_IOTAdieresis
            case 0x07a7: return 0x038c; // Greek_OMICRONaccent
            case 0x07a8: return 0x038e; // Greek_UPSILONaccent
            case 0x07a9: return 0x03ab; // Greek_UPSILONdieresis
            case 0x07ab: return 0x038f; // Greek_OMEGAaccent
            case 0x07ae: return 0x0385; // Greek_accentdieresis
            case 0x07af: return 0x2015; // Greek_horizbar
            case 0x07b1: return 0x03ac; // Greek_alphaaccent
            case 0x07b2: return 0x03ad; // Greek_epsilonaccent
            case 0x07b3: return 0x03ae; // Greek_etaaccent
            case 0x07b4: return 0x03af; // Greek_iotaaccent
            case 0x07b5: return 0x03ca; // Greek_iotadieresis
            case 0x07b6: return 0x0390; // Greek_iotaaccentdieresis
            case 0x07b7: return 0x03cc; // Greek_omicronaccent
            case 0x07b8: return 0x03cd; // Greek_upsilonaccent
            case 0x07b9: return 0x03cb; // Greek_upsilondieresis
            case 0x07ba: return 0x03b0; // Greek_upsilonaccentdieresis
            case 0x07bb: return 0x03ce; // Greek_omegaaccent
            case 0x07c1: return 0x0391; // Greek_ALPHA
            case 0x07c2: return 0x0392; // Greek_BETA
            case 0x07c3: return 0x0393; // Greek_GAMMA
            case 0x07c4: return 0x0394; // Greek_DELTA
            case 0x07c5: return 0x0395; // Greek_EPSILON
            case 0x07c6: return 0x0396; // Greek_ZETA
            case 0x07c7: return 0x0397; // Greek_ETA
            case 0x07c8: return 0x0398; // Greek_THETA
            case 0x07c9: return 0x0399; // Greek_IOTA
            case 0x07ca: return 0x039a; // Greek_KAPPA
            case 0x07cb: return 0x039b; // Greek_LAMBDA
            case 0x07cc: return 0x039c; // Greek_MU
            case 0x07cd: return 0x039d; // Greek_NU
            case 0x07ce: return 0x039e; // Greek_XI
            case 0x07cf: return 0x039f; // Greek_OMICRON
            case 0x07d0: return 0x03a0; // Greek_PI
            case 0x07d1: return 0x03a1; // Greek_RHO
            case 0x07d2: return 0x03a3; // Greek_SIGMA
            case 0x07d4: return 0x03a4; // Greek_TAU
            case 0x07d5: return 0x03a5; // Greek_UPSILON
            case 0x07d6: return 0x03a6; // Greek_PHI
            case 0x07d7: return 0x03a7; // Greek_CHI
            case 0x07d8: return 0x03a8; // Greek_PSI
            case 0x07d9: return 0x03a9; // Greek_OMEGA
            case 0x07e1: return 0x03b1; // Greek_alpha
            case 0x07e2: return 0x03b2; // Greek_beta
            case 0x07e3: return 0x03b3; // Greek_gamma
            case 0x07e4: return 0x03b4; // Greek_delta
            case 0x07e5: return 0x03b5; // Greek_epsilon
            case 0x07e6: return 0x03b6; // Greek_zeta
            case 0x07e7: return 0x03b7; // Greek_eta
            case 0x07e8: return 0x03b8; // Greek_theta
            case 0x07e9: return 0x03b9; // Greek_iota
            case 0x07ea: return 0x03ba; // Greek_kappa
            case 0x07eb: return 0x03bb; // Greek_lambda
            case 0x07ec: return 0x03bc; // Greek_mu
            case 0x07ed: return 0x03bd; // Greek_nu
            case 0x07ee: return 0x03be; // Greek_xi
            case 0x07ef: return 0x03bf; // Greek_omicron
            case 0x07f0: return 0x03c0; // Greek_pi
            case 0x07f1: return 0x03c1; // Greek_rho
            case 0x07f2: return 0x03c3; // Greek_sigma
            case 0x07f3: return 0x03c2; // Greek_finalsmallsigma
            case 0x07f4: return 0x03c4; // Greek_tau
            case 0x07f5: return 0x03c5; // Greek_upsilon
            case 0x07f6: return 0x03c6; // Greek_phi
            case 0x07f7: return 0x03c7; // Greek_chi
            case 0x07f8: return 0x03c8; // Greek_psi
            case 0x07f9: return 0x03c9; // Greek_omega
            default: break;
        }
    }

    // Katakana (0x04a1 - 0x04df)
    if (keysym >= 0x04a1 && keysym <= 0x04df) {
        return static_cast<char32_t>(keysym - 0x04a1 + 0x30a1);
    }

    // Hebrew (0x0ca0 - 0x0cfa)
    if (keysym >= 0x0ca0 && keysym <= 0x0cfa) {
        return static_cast<char32_t>(keysym - 0x0ca0 + 0x05d0);
    }

    // Arabic (0x0590 - 0x05fe)
    if (keysym >= 0x0590 && keysym <= 0x05fe) {
        return static_cast<char32_t>(keysym - 0x0590 + 0x0600);
    }

    // Hangul / Korean (0x0ea1 - 0x0efe)
    if (keysym >= 0x0ea1 && keysym <= 0x0efe) {
        return static_cast<char32_t>(keysym - 0x0ea1 + 0x3131);
    }

    // Latin-2, Latin-3, Latin-4 table
    struct KeySymPair {
        KeySym keysym;
        char32_t ucs;
    };
    static constexpr KeySymPair pairs[] = {
        { 0x01a1, 0x0104 }, // Aogonek
        { 0x01a2, 0x02d8 }, // breve
        { 0x01a3, 0x0141 }, // Lstroke
        { 0x01a5, 0x013d }, // Lcaron
        { 0x01a6, 0x015a }, // Sacute
        { 0x01a9, 0x0160 }, // Scaron
        { 0x01aa, 0x015e }, // Scedilla
        { 0x01ab, 0x0164 }, // Tcaron
        { 0x01ac, 0x0179 }, // Zacute
        { 0x01ae, 0x017d }, // Zcaron
        { 0x01af, 0x017b }, // Zabovedot
        { 0x01b1, 0x0105 }, // aogonek
        { 0x01b2, 0x02db }, // ogonek
        { 0x01b3, 0x0142 }, // lstroke
        { 0x01b5, 0x013e }, // lcaron
        { 0x01b6, 0x015b }, // sacute
        { 0x01b7, 0x02c7 }, // caron
        { 0x01b9, 0x0161 }, // scaron
        { 0x01ba, 0x015f }, // scedilla
        { 0x01bb, 0x0165 }, // tcaron
        { 0x01bc, 0x017a }, // zacute
        { 0x01bd, 0x02dd }, // doubleacute
        { 0x01be, 0x017e }, // zcaron
        { 0x01bf, 0x017c }, // zabovedot
        { 0x01c0, 0x0154 }, // Racute
        { 0x01c3, 0x0102 }, // Abreve
        { 0x01c5, 0x0139 }, // Lacute
        { 0x01c6, 0x0106 }, // Cacute
        { 0x01c8, 0x010c }, // Ccaron
        { 0x01ca, 0x0118 }, // Eogonek
        { 0x01cc, 0x011a }, // Ecaron
        { 0x01cf, 0x010e }, // Dcaron
        { 0x01d0, 0x0110 }, // Dstroke
        { 0x01d1, 0x0143 }, // Nacute
        { 0x01d2, 0x0147 }, // Ncaron
        { 0x01d5, 0x0150 }, // Odoubleacute
        { 0x01d8, 0x0158 }, // Rcaron
        { 0x01d9, 0x016e }, // Uring
        { 0x01db, 0x0170 }, // Udoubleacute
        { 0x01de, 0x0162 }, // Tcedilla
        { 0x01e0, 0x0155 }, // racute
        { 0x01e3, 0x0103 }, // abreve
        { 0x01e5, 0x013a }, // lacute
        { 0x01e6, 0x0107 }, // cacute
        { 0x01e8, 0x010d }, // ccaron
        { 0x01ea, 0x0119 }, // eogonek
        { 0x01ec, 0x011b }, // ecaron
        { 0x01ef, 0x010f }, // dcaron
        { 0x01f0, 0x0111 }, // dstroke
        { 0x01f1, 0x0144 }, // nacute
        { 0x01f2, 0x0148 }, // ncaron
        { 0x01f5, 0x0151 }, // odoubleacute
        { 0x01f8, 0x0159 }, // rcaron
        { 0x01f9, 0x016f }, // uring
        { 0x01fb, 0x0171 }, // udoubleacute
        { 0x01fe, 0x0163 }, // tcedilla
        { 0x01ff, 0x02d9 }, // abovedot
    };

    auto it = std::lower_bound(
        std::begin(pairs), std::end(pairs), keysym,
        [](const KeySymPair& p, KeySym k) { return p.keysym < k; });
    if (it != std::end(pairs) && it->keysym == keysym) {
        return it->ucs;
    }

    return 0;
}

} // namespace aui::x11
