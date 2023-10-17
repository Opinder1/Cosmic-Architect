#pragma once

#include <godotengine/core/string/ustring.h>

#include <godotengine/core/error/error_list.h>
#include <godotengine/core/error/error_macros.h>

inline String GodotErrorString(Error error)
{
    if (error < 0 || error >= ERR_MAX)
    {
        return String("(invalid error code)");
    }

    return String(error_names[error]);
}