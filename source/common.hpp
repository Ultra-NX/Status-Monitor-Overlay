#pragma once
#include "Utils.hpp"

ButtonMapperImpl buttonMapper; // Custom button mapper implementation
static bool returningFromSelection = false; // for removing the necessity of svcSleepThread
static tsl::elm::OverlayFrame* rootFrame = nullptr;
static bool skipMain = false;
