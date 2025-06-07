/**
 * @file Utils.h
 * @brief Utility functions for the Paint application.
 */
#pragma once

 /**
  * @brief Clamp a value to a given range [min, max].
  * @tparam T Numeric type.
  * @param value Value to clamp.
  * @param min Minimum allowed value.
  * @param max Maximum allowed value.
  * @return Clamped value.
  */
template<typename T>
T Clamp(T value, T min, T max) {
    return (value < min) ? min : (value > max) ? max : value;
}
