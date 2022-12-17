add_library(planet-android
        asset_loader.cpp
    )
target_link_libraries(planet-android PUBLIC
        planet-sdl
    )
