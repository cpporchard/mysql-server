FROM public.ecr.aws/amazonlinux/amazonlinux:2023

# ── refresh metadata ───────────────────────────────
RUN dnf -y update

# ── full compiler toolchain group ──────────────────
RUN dnf -y groupinstall "Development Tools"

# ── core build dependencies (one line each) ────────
RUN dnf -y install make
RUN dnf -y install cmake
RUN dnf -y install git
RUN dnf -y install gcc
RUN dnf -y install gcc-c++
RUN dnf -y install openssl-devel
RUN dnf -y install ncurses-devel
RUN dnf -y install libcurl-devel
RUN dnf -y install libudev-devel
RUN dnf -y install bison
RUN dnf -y install libtirpc-devel
RUN dnf -y install rpcgen

# ── convenience CLI tools (also split) ─────────────
RUN dnf -y install vim
RUN dnf -y install curl
RUN dnf -y install procps
RUN dnf -y install lsof

# ── clean RPM cache ────────────────────────────────
RUN dnf clean all

WORKDIR /workspace
