set (web_SRCS
	kadu-web-engine-page.cpp
	kadu-web-engine-profile.cpp
	web-module.cpp
)

kadu_subdirectory (web "${web_SRCS}")
