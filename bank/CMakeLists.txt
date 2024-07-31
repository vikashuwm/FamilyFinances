# In the Bank library CMakeLists.txt
add_library(Bank
    src/Account.cpp
    src/Bank.cpp
    src/Money.cpp
    src/OverdraftException.cpp
    src/Transaction.cpp
)
target_include_directories(Bank PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)