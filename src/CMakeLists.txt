add_library(ViaductABY SHARED
  ViaductABY.cpp
  ViaductABY_wrap.cpp
  ${ViaductABY_SOURCE_DIR}/include/ViaductABY/ViaductABY.h
)

target_include_directories(ViaductABY
  PUBLIC
    $<BUILD_INTERFACE:${ViaductABY_SOURCE_DIR}/include>
    ${JNI_INCLUDE_DIRS}
)

target_link_libraries(ViaductABY PRIVATE ABY::aby)
