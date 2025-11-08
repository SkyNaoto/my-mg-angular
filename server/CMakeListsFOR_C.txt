cmake_minimum_required(VERSION 3.16)
project(mg_server C)

set(CMAKE_C_STANDARD 11)

set(DEFAULT_DB_PATH "./sample.db" CACHE STRING "Default DB path")

set(SOURCES
    main.c
    mongoose.c
)

add_compile_definitions(DEFAULT_DB_PATH=\"${DEFAULT_DB_PATH}\")

add_executable(server ${SOURCES})

find_package(SQLite3 REQUIRED)
target_link_libraries(server PRIVATE SQLite::SQLite3 pthread)


# sample.db を build にコピー
add_custom_command(
    TARGET server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/sample.db
            ${CMAKE_BINARY_DIR}/sample.db
)

# web ディレクトリを build にコピー
#add_custom_commandを使用して、ビルド後に特定のディレクトリをコピーするカスタムコマンドを定義しています。
#1. TARGET server POST_BUILD
#  serverというターゲット（ここではadd_executableで定義された実行ファイル）に対して、このカスタムコマンドを適用します。
#  POST_BUILDは、ターゲットのビルドが完了した後にこのコマンドを実行することを指定します。
#2. COMMAND ${CMAKE_COMMAND} -E copy_directory
#  ${CMAKE_COMMAND}は、CMake自体の実行可能ファイルを指します。
#  -E copy_directoryは、CMakeにディレクトリ全体をコピーするよう指示します。
#3. ${CMAKE_SOURCE_DIR}/web
#  プロジェクトのソースディレクトリ（CMakeLists.txtが存在するディレクトリ）内のwebディレクトリを指定しています。
#4.${CMAKE_BINARY_DIR}/web
#  ビルドディレクトリ内のwebディレクトリを指定しています。
#  webディレクトリが存在しない場合は自動的に作成され、ソースディレクトリの内容がコピーされます。
add_custom_command(
    TARGET server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/web
            ${CMAKE_BINARY_DIR}/web
)
