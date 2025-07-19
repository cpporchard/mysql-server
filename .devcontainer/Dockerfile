# Fedora 37 carries CMake 3.22.2 and GDB 13.2 in its base repos.
FROM --platform=$TARGETPLATFORM fedora:37

RUN dnf -y update && \
    # === replicate your commands verbatim ===
    dnf -y groupinstall "Development Tools" && \
    dnf -y install \
        make cmake git gcc gcc-c++ \
        openssl-devel ncurses-devel libcurl-devel libudev-devel \
        bison libtirpc-devel rpcgen && \
    dnf -y remove cmake gdb && \
    dnf -y install cmake-3.22.2 gdb-13.2 && \
    # === (optional) small conveniences ===
    dnf -y install vim curl procps lsof && \
    dnf clean all

WORKDIR /workspace
