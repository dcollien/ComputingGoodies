mkdir -p ./bin/
gcc -Wno-deprecated-declarations -Wall -Werror -o ./bin/tryDecrypt ./src/tryDecrypt.c ./src/decryptURLCode.c -I./include `pkg-config --cflags --libs openssl`
gcc -Wno-deprecated-declarations -Wall -Werror -o ./bin/tryOpenLearning ./src/tryOpenLearning.c ../lib/openlearningAPI/src/*.c -I./include -I../lib/openlearningAPI/include -lcurl -ljansson

