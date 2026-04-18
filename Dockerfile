# Start from the gcc image
FROM gcc:9

# Install stuff
RUN set -ex;                                                                      \
    apt-get update;                                                               \
    apt-get install -y cmake libzmq3-dev gdb;                                     \
    mkdir -p /usr/src;                                                            \
    cd /usr/src;                                                                  \
    curl -L https://github.com/zeromq/cppzmq/archive/v4.6.0.tar.gz | tar -zxf -;  \
    cd /usr/src/cppzmq-4.6.0;                                                     \
    cmake -D CPPZMQ_BUILD_TESTS:BOOL=OFF .; make; make install

# Copy the content of the current directory into docker /usr/src
COPY . /usr/src

# Install Allegro with CMake
RUN set -ex;              \
    cd /usr/src/allegro-4.4.2;  \
    mkdir build;  \
    cd build;  \
    cmake ..; make; make install

# Copy allegro's lib into global library
#RUN cp -r /usr/src/allegro-4.4.2/build/lib /lib/

# Export Allegro's library for binding (pass as argument on runtime instead: docker run -e ...)
#RUN export LD_LIBRARY_PATH="/usr/src/allegro-4.4.2/build/lib/"

# Try to build the test file
#RUN cd /usr/src;  \
#    gcc main.c -o Test `allegro-config --libs`;


ENTRYPOINT ["bash"]
