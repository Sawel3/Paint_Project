#pragma once

// Clamp: Utility function to restrict a value to a given range [min, max].
// Returns min if value < min, max if value > max, otherwise value.
template<typename T>
T Clamp(T value, T min, T max) {
    return (value < min) ? min : (value > max) ? max : value;
}
