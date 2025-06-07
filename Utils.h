#pragma once

// Example template utility
template<typename T>
T Clamp(T value, T min, T max) {
    return (value < min) ? min : (value > max) ? max : value;
}
