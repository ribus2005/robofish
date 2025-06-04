FROM gcc:latest


RUN apt-get update
RUN apt-get install -y cmake 


ADD ./src /app/src

WORKDIR /app/build

RUN cmake ../src
RUN cmake --build .


CMD ["ctest", "-C"]

