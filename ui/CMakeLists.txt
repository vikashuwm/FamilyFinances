# CMakeLists.txt (ui directory)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(UI_SOURCES
    src/FamilyFinances.cpp
    src/LoginPage.cpp
    src/AccountManager.cpp
    src/TransactionManager.cpp
)

set(UI_HEADERS
    include/FamilyFinances.h
    include/LoginPage.h
    include/AccountManager.h
    include/TransactionManager.h
)

add_library(UI ${UI_SOURCES} ${UI_HEADERS})

target_include_directories(UI PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../bank/include
)

target_link_libraries(UI PUBLIC
    Qt6::Widgets
    Bank
)

# Explicitly run moc on header files
qt6_wrap_cpp(MOC_FILES ${UI_HEADERS})

# Add the generated moc files to the library
target_sources(UI PRIVATE ${MOC_FILES})