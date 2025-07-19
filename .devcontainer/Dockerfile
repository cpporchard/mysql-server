# .devcontainer/Dockerfile
FROM registry.fedoraproject.org/fedora:37      # ← no auth needed

RUN dnf -y update && \
    dnf -y groupinstall "Development Tools" && \
    dnf -y install \
        make cmake git gcc gcc-c++ \
        openssl-devel ncurses-devel libcurl-devel libudev-devel \
        bison libtirpc-devel rpcgen && \
    dnf -y remove cmake gdb && \
    dnf -y install cmake-3.22.2 gdb-13.2 && \
    dnf clean all

WORKDIR /workspace
