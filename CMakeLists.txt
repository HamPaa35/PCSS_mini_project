cmake_minimum_required(VERSION 3.12)
project(SoccerManager)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(SoccerManager
        bin/Debug/SoccerManager.exe
        export/codeForLecture1.cpp
        export/codeForLecture1.rtf
        export/Person.rtf
        export/player.cpp
        export/Player.rtf
        export/player2.rtf
        obj/Debug/Coach.o
        obj/Debug/FriendlySoccerMatch.o
        obj/Debug/Keeper.o
        obj/Debug/main.o
        obj/Debug/Person.o
        obj/Debug/Player.o
        obj/Debug/Team.o
        Coach.cpp
        Coach.h
        FriendlyMatch.h
        FriendlySoccerMatch.cpp
        FriendlySoccerMatch.h
        Keeper.cpp
        Keeper.h
        main.cpp
        Person.cpp
        Person.h
        Player.cpp
        Player.h
        SoccerManager.cbp
        SoccerManager.depend
        SoccerManager.layout
        Team.cpp
        Team.h
        test.txt)
