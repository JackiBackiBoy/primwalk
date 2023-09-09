#pragma once

// primwalk
#include "primwalk/core.hpp"

// std
#include <algorithm>
#include <string>
#include <unordered_map>
#include <stdexcept>

namespace pw {
  enum class FontWeight : int {
    Thin = 100,
    ExtraLight = 200,
    Light = 300,
    Regular = 400,
    Medium = 500,
    SemiBold = 600,
    Bold = 700,
    ExtraBold = 800,
    Black = 900,
    ExtraBlack = 950,
  };

  inline const std::unordered_map<std::string, FontWeight> fontWeightStringTable =
  {
    { "thin",        FontWeight::Thin},
    { "hairline",    FontWeight::Thin},
    { "ultra-light", FontWeight::ExtraLight},
    { "ultra light", FontWeight::ExtraLight},
    { "extra-light", FontWeight::ExtraLight},
    { "extra light", FontWeight::ExtraLight},
    { "light",       FontWeight::Light},
    { "normal",      FontWeight::Regular},
    { "regular",     FontWeight::Regular},
    { "medium",      FontWeight::Medium},
    { "semi-bold",   FontWeight::SemiBold},
    { "semi bold",   FontWeight::SemiBold},
    { "demi-bold",   FontWeight::SemiBold},
    { "demi bold",   FontWeight::SemiBold},
    { "bold",        FontWeight::Bold},
    { "extra-bold",  FontWeight::ExtraBold},
    { "extra bold",  FontWeight::ExtraBold},
    { "ultra-bold",  FontWeight::ExtraBold},
    { "ultra bold",  FontWeight::ExtraBold},
    { "heavy",       FontWeight::Black},
    { "black",       FontWeight::Black},
    { "extra-black", FontWeight::ExtraBlack},
    { "ultra-black", FontWeight::ExtraBlack},
  };

  inline FontWeight getWeightFromString(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
      [](const char v) { return std::tolower(v); });

    auto search = fontWeightStringTable.find(str);
    if (search == fontWeightStringTable.end()) { // no search result found
      throw std::runtime_error("No font weight found!");
    }

    return search->second;
  }
}
