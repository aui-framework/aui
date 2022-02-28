#pragma once

/**
 * Common json exception
 */
class AJsonException: public AException {
public:
    using AException::AException;
};

/**
 * Thrown when a parse error occurred
 */
class AJsonParseException: public AJsonException {
public:
    AJsonParseException(const AString& message) : AJsonException(message) {}
};

/**
 * Thrown when type mismatch occurred
 */
class AJsonTypeMismatchException: public AJsonException {
public:
    AJsonTypeMismatchException(const AString& message) : AJsonException(message) {}
};