#!/usr/bin/env bash

# Add every specific linkages with libs inside this case

linking_lib() {
    linkingLibs="";

    case "$1" in
        pthreads.c)
            linkingLibs="-pthread"
            ;;
        *)
            linkingLibs=""
            ;;
        #TODO: make cases for every file
    esac

    echo ${linkingLibs}
}

#check if "build" directory not exists and create it
if [ ! -d "./build" ]; then
    mkdir ./build
fi

# destination should be build folder
if [ $# -eq 0 ]; then
	for name in *.c; do
	    echo "Compiling \033[0;33m$name\033[0m ..."
        gcc $(linking_lib ${name}) ${name} -o ./build/"${name%.*}" && echo "\033[1;92m$name\033[0m done."
	done
else
    for name in "$@"; do
        echo "Compiling \033[0;33m$name\033[0m ..."
        case "$name" in
            *.c)
                gcc $(linking_lib ${name}) $(basename "$name") -o ./build/"${name%.*}" && echo "\033[1;92m$name\033[0m done."
                ;;
            *.cc)
                g++ $(linking_lib ${name}) $(basename "$name") -o ./build/"${name%.*}" && echo "\033[1;92m$name\033[0m done."
                ;;
            *)
                echo "Ignoring $name, since it is not one of my files!"
                ;;
        esac
    done
fi