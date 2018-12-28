#!/bin/bash

/usr/local/bin/keosd &

fresh_chain=0
for i in $@; do
  if [ "$i" == "--delete-all-blocks" ]; then fresh_chain=1;fi
done

if [ -d /root/.local/share/eosio/nodeos/data ] && [ $fresh_chain -eq 0  ]; then
  /usr/local/bin/nodeos "$@"
else
  if [ ! -f /root/.local/share/eosio/nodeos/config/config.ini ]; then 
    mkdir -p /root/.local/share/eosio/nodeos/config
    wget -O  https://raw.githubusercontent.com/UOSnetwork/uos.docs/master/testnetv1/config.ini
  fi
  if [ ! -f /root/.local/share/eosio/nodeos/config/genesis.json ]; then 
    wget -O /root/.local/share/eosio/nodeos/config/genesis.json https://raw.githubusercontent.com/UOSnetwork/uos.docs/master/testnetv1/genesis.json
  fi
  /usr/local/bin/nodeos --genesis-json /root/.local/share/eosio/nodeos/config/genesis.json "$@"
fi
