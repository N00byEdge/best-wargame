pushd build && \
conan install .. -pr gcc-7.3.0-linux-x86_64 --build missing && \
source ./activate.sh && \
cmake .. && \
cmake --build . && \
boot wargame_service --create-bridge && \
source deacticate.sh
