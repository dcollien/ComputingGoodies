mkdir -p ./bin/
gcc -Wall -Werror -o ./bin/runPerceptionServer ./src/perceptionRequestHandler.c ./src/weakEncrypt.c ./src/runServer.c ../lib/openlearningAPI/src/*.c ../../ImageProcessing/src/*.c ../lib/perception/src/*.c -I./include -I../lib/perception/include -I../lib/openlearningAPI/include -I../../ImageProcessing/include -lcurl -ljansson `pkg-config --cflags --libs libevent MagickWand`
