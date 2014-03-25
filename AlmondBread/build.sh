mkdir -p ./bin
cc -Wall -Werror -o ./bin/almondBread ./src/MandelbrotSet.c ./src/mandelbrotServer.c ./src/puzzleQuest.c ../ImageProcessing/src/*.c ./src/runServer.c -I./include -I../ImageProcessing/include `pkg-config --cflags --libs libevent MagickWand`
