FROM ubuntu:18.04

RUN apt-get update && apt-get -y install git clang-4.0 lldb-4.0 libclang-4.0-dev cmake make automake libbz2-dev libssl-dev libgmp3-dev autotools-dev build-essential libicu-dev python2.7-dev python3-dev autoconf libtool curl zlib1g-dev doxygen graphviz libboost-all-dev sudo wget
RUN git clone https://github.com/UOSnetwork/uos && cd uos && git submodule update --init --recursive && ./eosio_build.sh && ./eosio_install.sh
RUN apt -y remove *-dev && apt -y autoremove && apt-get -y clean && rm -rf /uos && rm -rf /root/opt
ADD start.sh /usr/local/bin/

EXPOSE 8888 9876

ENTRYPOINT ["/usr/local/bin/start.sh"]
CMD []
