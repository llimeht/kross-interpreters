unset(RUBY_VERSION_MAJOR CACHE)
unset(RUBY_INCLUDE_DIR CACHE)
unset(RUBY_CONFIG_INCLUDE_DIR CACHE)
unset(RUBY_LIBRARY CACHE)
find_package(Ruby 1.9)

set(krossruby_PART_SRCS
    rubyvariant.cpp
    rubyinterpreter.cpp
    rubyextension.cpp
    rubyscript.cpp
    rubymodule.cpp
    rubycallcache.cpp
    rubyobject.cpp
)

if(RUBY_FOUND)
    include_directories(${RUBY_INCLUDE_DIRS})
    add_library(krossruby MODULE ${krossruby_PART_SRCS})
    target_link_libraries(krossruby
        KF5::KrossCore
        Qt5::Gui
        Qt5::Widgets
        ${RUBY_LIBRARY}
    )

    install(TARGETS krossruby DESTINATION ${KDE_INSTALL_PLUGINDIR})
endif()
