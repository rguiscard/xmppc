# xmppc

xmppc - command line interface (CLI) XMPP Client.

xmppc is a XMPP command line interface client. It's written in C and is using
the xmpp library [libstrophe](http://strophe.im/libstrophe/).

## Dependencies

* [libc6-dev](https://packages.debian.org/buster/libc6-dev) (2.28-10)
* [libglib2.0-dev](https://packages.debian.org/buster/libglib2.0-dev) (2.58.3)
* [libstrophe-dev](https://packages.debian.org/buster/libstrophe-dev) (0.9.2-2)
* [libgpgme-dev](https://packages.debian.org/buster/libgpgme-dev) (1.12.0)

## Build

The project is using [GNU Automake](https://www.gnu.org/software/automake/).

```
aptitude install libstrophe-dev
./bootstrap.sh
./configure
make
```

## Usage

```
xmppc --jid user@domain.tld --pwd "password" --mode roster list
xmppc -j user@domain.tld -p "password" -m roster list
xmppc -j user@domain.tld -p "password" -m roster export
xmppc -j user@domain.tld -p "password" -m message chat friend@domain.tld "Message"
xmppc -j user@domain.tld -p "password" -m pgp chat friend@domain.tld "Message"
xmppc -j user@domain.tld -p "password" -m openpgp signcrypt friend@domain.tld "Message"
xmppc -j user@domain.tld -p "password" -m omemo list
xmppc -j user@domain.tld -p "password" -m monitor stanza
xmppc -a alice -m mam list bob@domain.tld
xmppc -m bookmark list
xmppc -m discovery info domain.tld
xmppc -m discovery item conference.domain.tld
```
Use xmppc with [pass](https://packages.debian.org/buster/pass)

```
xmppc --jid user@domain.tld --pwd $(pass XMPP/domain.tld/user) --mode roster list
```

## Config file

Config file: ` ~/.config/xmppc.conf`

```
[default]
jid=user@domain.tld
pwd=YourSecret

[account1]
jid=account1@domain.tld
pwd=YourSecret
```

## Documentation

* [Wiki](https://codeberg.org/Anoxinon_e.V./xmppc/wiki)

## Contact details

* MUC: xmpp-messenger@conference.anoxinon.me

