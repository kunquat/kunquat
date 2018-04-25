FROM ubuntu:latest
RUN apt-get -qq update
RUN apt-get install -y xvfb build-essential check libpulse-dev libsamplerate0-dev libsndfile1-dev libwavpack-dev python3 python3-pip clang-5.0
RUN pip3 install pylint flake8 pyqt5
COPY ./ /tmp/kunquat
WORKDIR /tmp/kunquat
RUN ./get_build_support.sh
ENV TRAVIS_OS_NAME linux
CMD ["./test_runner.sh"]
