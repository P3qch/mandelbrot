#pragma once
#include <SDL3/SDL.h>
#include <memory>
#include <cmath>
#include <algorithm> // for std::clamp
#include "Exceptions.h"

template <auto fn>
struct deleter_from_fn
{
    template <typename T>
    constexpr void operator()(T* arg) const noexcept
    {
        fn(arg);
    }
};

template <typename T, auto fn>
using custom_unique_ptr = std::unique_ptr<T, deleter_from_fn<fn>>;

template <typename T, auto fn>
using sdl_gpu_ptr = std::unique_ptr<T, deleter_from_fn<fn>>;


struct SdlContext {
    explicit SdlContext(Uint32 flags) {
        if (!SDL_Init(flags)) {
            throw SdlException("Failed to init SDL");
        }
    }

    ~SdlContext() {
        SDL_Quit();
    }
};
