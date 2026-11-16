#pragma once
#include <stddef.h>
#define SETTINGS_STATIC_HANDLER_DEFINE(name, subtree, get, set, commit, export) \
    static int (*const registered_commit)(void) = commit
