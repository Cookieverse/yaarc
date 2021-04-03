set(YAARC_TEST_FILES
        yaarc_test_parsing.cpp
        yaarc_test_encoding.cpp
        yaarc_test_value.cpp
        yaarc_test_string_to_int.cpp)

foreach (F IN LISTS YAARC_TEST_FILES)
    STRING(REPLACE .cpp "" FEXE ${F})
    ADD_EXECUTABLE(${FEXE} ${F})
    TARGET_LINK_LIBRARIES(${FEXE} PRIVATE fmt::fmt-header-only)
    ADD_TEST(NAME ${FEXE} COMMAND ${FEXE})
endforeach ()
if (YAARC_TEST_REDIS)
    set(YAARC_TEST_REDIS_FILES
            yaarc_test_redis_basic.cpp)
    foreach (F IN LISTS YAARC_TEST_REDIS_FILES)
        STRING(REPLACE .cpp "" FEXE ${F})
        ADD_EXECUTABLE(${FEXE} ${F})
        TARGET_LINK_LIBRARIES(${FEXE} PRIVATE fmt::fmt-header-only pthread)
        ADD_TEST(NAME ${FEXE} COMMAND ${FEXE})
    endforeach ()
endif()