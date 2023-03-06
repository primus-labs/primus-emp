# pado-emp
Including emp-tool, emp-ot, emp-zk. Some optimizations. WebAssembly supports.


## Overview

Current repo initialization date: 03/03/2023.

**EMP** initial version:

| project  | origin                                                          | commit-id | commit-date          |
| -------- | --------------------------------------------------------------- | --------- | -------------------- |
| emp-tool | [emp-toolkit/emp-tool](https://github.com/emp-toolkit/emp-tool) | 636b07d   | Feb 23 12:48:40 2023 |
| emp-ot   | [emp-toolkit/emp-ot](https://github.com/emp-toolkit/emp-ot)     | d5f4fc8   | Feb 24 11:41:21 2023 |
| emp-zk   | [emp-toolkit/emp-zk](https://github.com/emp-toolkit/emp-zk)     | d16142e   | Feb 22 21:24:57 2023 |


## Dependenices

- CPU: Intel
- OS: ubuntu 22.04; MacOS 12.6.3
- emcc: 3.1.31
- node: 16.16.0
- cmake: 3.22.1


## Compile and Run EMP*

- Compile

```sh
bash ./scripts/compile.sh
```

- Run

```sh
bash ./scripts/run.sh
```

By default, the `cmakes`, `includes`, `libs`, `logs` will output to `./build`.


## Compile and Run EMP* (WebAssembly)

- Compile

```sh
bash ./scripts/compile_wasm.sh
```

- Run

```sh
bash ./scripts/run_wasm.sh
```

By default, the `cmakes`, `includes`, `libs`, `logs` will output to `./build_wasm`.

