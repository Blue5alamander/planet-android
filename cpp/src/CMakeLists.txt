add_library(planet-android
        asset_loader.cpp
        log_android.cpp
    )
target_link_libraries(planet-android PUBLIC
        android
        log
        planet-sdl
    )
