# I have no idea about its versions
# it is just the version I have
cmake_minimum_required(VERSION 3.28.3)

# Not like I have an idea what is the difference
# it is the last one at the moment
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED True)

project(
	Pages
	VERSION 0.1
	DESCRIPTION "Zrucxne peremisxcxennqa storinok cxerez terminal"
	LANGUAGES C
)
add_executable(pages.o ../code/pages.c)

