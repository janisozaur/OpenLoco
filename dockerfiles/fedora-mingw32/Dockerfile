FROM fedora:27
RUN dnf update -y && \
  dnf install -y findutils mingw32-boost mingw32-boost-static mingw32-SDL2 mingw32-SDL2-static cmake git ninja-build dnf-plugins-core && \
  dnf clean all --enablerepo=\*
RUN dnf copr enable -y janisozaur/mingw32-yaml-cpp && \
  dnf install -y mingw32-yaml-cpp && \
  dnf clean all --enablerepo=\*
