# ---------------------------------------------------------------------------
# Test projects. Each test class becomes its own executable and is
# registered with CTest.
# ---------------------------------------------------------------------------

function(bws_add_test testname)
    qt_add_executable(${testname} ${ARGN})
    target_link_libraries(${testname} PRIVATE bws_controller Qt6::Test)
    target_include_directories(${testname} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_definitions(${testname} PRIVATE
        TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
    add_test(NAME ${testname} COMMAND ${testname})
endfunction()

# --- Phase 2: users, roles, login ---
bws_add_test(tst_passwordhasher       tst_passwordhasher.cpp)
bws_add_test(tst_userrole             tst_userrole.cpp)
bws_add_test(tst_authenticationservice tst_authenticationservice.cpp)

# --- Phase 3: persistence ---
bws_add_test(tst_database             tst_database.cpp)
bws_add_test(tst_sqliteuserrepository tst_sqliteuserrepository.cpp)

# --- Phase 4: arXiv integration ---
bws_add_test(tst_discipline           tst_discipline.cpp)
bws_add_test(tst_arxivatomparser      tst_arxivatomparser.cpp)
bws_add_test(tst_sqlitepublicationrepository tst_sqlitepublicationrepository.cpp)

# --- Phase 5: reading list process and permissions ---
bws_add_test(tst_readingstatus        tst_readingstatus.cpp)
bws_add_test(tst_permissionservice    tst_permissionservice.cpp)
bws_add_test(tst_readinglistservice   tst_readinglistservice.cpp)

# --- Phase 6: user management and controller layer ---
bws_add_test(tst_usermanagementservice tst_usermanagementservice.cpp)
bws_add_test(tst_controller            tst_controller.cpp)

# --- Phase 8: acceptance test ---
# The only test that needs the view layer.
qt_add_executable(tst_acceptancetest tst_acceptancetest.cpp)
target_link_libraries(tst_acceptancetest PRIVATE bws_view Qt6::Test)
target_include_directories(tst_acceptancetest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME tst_acceptancetest COMMAND tst_acceptancetest)

# Column-header sorting (B-27) -- also needs the view layer.
qt_add_executable(tst_sorting tst_sorting.cpp)
target_link_libraries(tst_sorting PRIVATE bws_view Qt6::Test)
target_include_directories(tst_sorting PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME tst_sorting COMMAND tst_sorting)

# --- Phase 9: automatic daily refresh (US-09) ---
bws_add_test(tst_arxivscheduler tst_arxivscheduler.cpp)
