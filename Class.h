#pragma once

#include "Class.g.h"

namespace winrt::Learn_Composition::implementation
{
    struct Class : ClassT<Class>
    {
        Class() = default;

        int32_t MyProperty();
        void MyProperty(int32_t value);
    };
}

namespace winrt::Learn_Composition::factory_implementation
{
    struct Class : ClassT<Class, implementation::Class>
    {
    };
}
