#pragma once

#include <optional>

namespace fuzzing {
namespace truth {

struct Comparison {
    std::optional<bool> EQ, NEQ, GT, LT, EQGT, EQLT;
};

inline bool isValid(Comparison comparison) {
    if ( comparison.EQ && comparison.NEQ && *comparison.EQ && *comparison.NEQ ) {
        /* Cannot be both equal and not equal */
        return false;
    }

    if ( comparison.NEQ && comparison.GT && *comparison.NEQ && *comparison.GT ) {
        /* Greater than implies inequality */
        return false;
    }

    if ( comparison.NEQ && comparison.LT && *comparison.NEQ && *comparison.LT ) {
        /* Less than implies inequality */
        return false;
    }

    if ( comparison.EQ && comparison.GT && *comparison.EQ && *comparison.GT ) {
        /* Cannot be both equal and greater than */
        return false;
    }

    if ( comparison.EQ && comparison.LT && *comparison.EQ && *comparison.LT ) {
        /* Cannot be both equal and less than */
        return false;
    }

    if ( comparison.GT && comparison.LT && *comparison.GT && *comparison.LT ) {
        /* Cannot be both greater than and less than */
        return false;
    }

    if ( comparison.EQ && comparison.EQGT && *comparison.EQ && !(*comparison.EQGT) ) {
        /* If equal, then must be (equal or greater than) */
        return false;
    }

    if ( comparison.EQ && comparison.EQLT && *comparison.EQ && !(*comparison.EQLT) ) {
        /* If equal, then must be (equal or less than) */
        return false;
    }

    if ( comparison.GT && comparison.EQGT && *comparison.GT && !(*comparison.EQGT) ) {
        /* If greater than, then must be (equal or greater than) */
        return false;
    }

    if ( comparison.LT && comparison.EQLT && *comparison.LT && !(*comparison.EQLT) ) {
        /* If less than, then must be (equal or less than) */
        return false;
    }

    return true;
}

} /* namespace fuzzing */
} /* namespace truth */
