# template-imguiWinAppDX11-with-rest-and-ws-client

## about

an imgui based DX11 app template with rest and websocket client.

### build

to build run the python script

```bash
python build.py
```

#### alternative build - individual build steps

get all packages from conan for both debug and release

```bash
conan install . --build=missing -s build_type=Debug
conan install . --build=missing -s build_type=Release
```

now build the cmake project

```bash
cmake --preset conan-default -DDEV_MACHINE=ON
```

## author

Stefan Kazassoglou
