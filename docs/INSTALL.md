

- [Overview](#overview)
- [Install emsdk \& node](#install-emsdk--node)
  - [Install emsdk](#install-emsdk)
  - [Install node](#install-node)
  - [Configure environment variables](#configure-environment-variables)


## Overview

Assuming the current working directory is `/home/work/`.


## Install emsdk & node


Install `emsdk` and `node` as follows.


### Install emsdk

On Linux or MacOS,

```sh
# Get the emsdk repo
git clone https://github.com/emscripten-core/emsdk.git

# Enter that directory
cd emsdk

# Download and install a specific version SDK tools.
./emsdk install 3.1.31

# Make the SDK "active" for the current user. (writes .emscripten file)
./emsdk activate 3.1.31
```

<br/>

Q1:

If you encounter the following problem when executing the `./emsdk install ...` command:

```log
Warning: Possibly SSL/TLS issue. Update or install Python SSL root certificates (2048-bit or greater) supplied in Python folder or https://pypi.org/project/certifi/ and try again.
```

A1:

It looks like there is something wrong with the python environment. Please add the following two lines to the head of `emsdk.py`:

```
import ssl
ssl._create_default_https_context = ssl._create_unverified_context
```


### Install node

On Linux,

```sh
wget https://nodejs.org/dist/v18.14.2/node-v18.14.2-linux-x64.tar.xz
tar -xJf node-v18.14.2-linux-x64.tar.xz
```

On MacOS,

```sh
wget https://nodejs.org/dist/v18.14.2/node-v18.14.2-darwin-x64.tar.gz
tar -zxf node-v18.14.2-darwin-x64.tar.gz
```

### Configure environment variables

Open `~/.bash_profile` and add the following environment variables:

On Linux,

```
export NODE_HOME=/home/work/node-v18.14.2-linux-x64
export EMSDK=/home/work/emsdk
export EMSDK_NODE=$NODE_HOME/bin/node
export PATH=$EMSDK:$EMSDK/upstream/emscripten:$NODE_HOME/bin:$PATH
```

On MacOS,

```
export NODE_HOME=/home/work/node-v18.14.2-darwin-x64
export EMSDK=/home/work/emsdk
export EMSDK_NODE=$NODE_HOME/bin/node
export PATH=$EMSDK:$EMSDK/upstream/emscripten:$NODE_HOME/bin:$PATH
```

<br/>

After the configuration is complete, making environment variables take effect by:

```sh
source ~/.bash_profile
```

Then, run the following commands to check the version:

```sh
emcc -v
node -v
```
