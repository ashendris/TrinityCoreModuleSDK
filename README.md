# TrinityCoreModuleSDK

Standalone SDK for building secure, dynamically loaded TrinityCore modules outside the TrinityCore source tree.

## What this repository provides

- SDK-owned stable C ABI v1 headers;
- opaque handles and versioned host/module tables;
- CMake package configuration;
- `trinity_add_module()` build helper;
- Ed25519 module-package signing tool;
- signed manifest and resource-hash workflow;
- installation instructions for external module repositories.

TrinityCore owns the runtime host and module loader. This repository owns the public module contract and packaging tools. The SDK does not link modules against TrinityCore implementation targets such as `game` or `worldserver`.

## Requirements

- CMake 3.24 or newer;
- Python 3;
- a C++17 compiler;
- a compiler/runtime compatible with the target `worldserver`.

The SDK is platform-independent at the source level. Build the SDK and modules for the same architecture and compatible native runtime as the server.

## Build and install

### Windows / Visual Studio

```powershell
cmake -S . -B build `
  -DCMAKE_INSTALL_PREFIX=C:/sdk/TrinityCoreModuleSDK `
  -G "Visual Studio 18 2026" -A x64
cmake --build build --config RelWithDebInfo --parallel
cmake --install build --config RelWithDebInfo
```

### Linux / Ninja

```sh
cmake -S . -B build -GNinja \
  -DCMAKE_INSTALL_PREFIX=/opt/trinitycore-module-sdk \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build --parallel
cmake --install build
```

The installation contains:

```text
include/trinity/TrinityModule.h
include/trinity/TrinityAbiV1.h
include/trinity/TrinitySDKBuild.h
lib/cmake/TrinitySDK/TrinitySDKConfig.cmake
lib/cmake/TrinitySDK/TrinitySDKModules.cmake
bin/sign_module.py
```

## Create an external module

Example module layout:

```text
my-module/
├── CMakeLists.txt
├── module.json
├── src/my_module.cpp
├── conf/my-module.conf.dist
└── sql/world/my_module_001.sql
```

`CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.24)
project(MyModule LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(TrinitySDK CONFIG REQUIRED)

trinity_add_module(NAME my-module
    SOURCES src/my_module.cpp
    MANIFEST ${CMAKE_CURRENT_SOURCE_DIR}/module.json
    CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/conf/my-module.conf.dist
    SQL ${CMAKE_CURRENT_SOURCE_DIR}/sql/world/my_module_001.sql)
```

Build it against the installed SDK:

```sh
cmake -S /path/to/my-module -B /path/to/my-module-build \
  -DCMAKE_PREFIX_PATH=/path/to/trinitycore-module-sdk
cmake --build /path/to/my-module-build --config RelWithDebInfo --parallel
```

The output is placed under:

```text
bin/<configuration>/modules/my-module/
```

Modules MUST include only SDK-owned headers and module-owned headers. They MUST NOT include TrinityCore private headers or link `game`, `worldserver`, or other core targets.

## Stable ABI

The module entry point is the C ABI export:

```cpp
#include "TrinityModule.h"

namespace
{
int32_t Load(void*, trinity_host const* host)
{
    if (!host)
        return -1;

    auto const* api = TrinityHostApiV1(host);
    if (!api || api->abi_version != TRINITY_ABI_V1 ||
        api->size < sizeof(*api) || !api->log.write)
        return -1;

    constexpr char message[] = "module loaded.";
    api->log.write(const_cast<trinity_host*>(host), 1,
        message, sizeof(message) - 1);
    return 0;
}

void Unload(void*, trinity_host const*) { }
void Reload(void*, trinity_host const*) { }
}

extern "C" int32_t TrinityModuleGetApi(
    uint32_t requested_abi, trinity_module_api* output)
{
    if (requested_abi != TRINITY_ABI_V1 || !output)
        return -1;

    *output = {
        sizeof(*output), TRINITY_ABI_V1, nullptr,
        Load, Unload, Reload};
    return 0;
}
```

The boundary uses fixed-width C types, versioned structures, caller-owned buffers, and opaque handles. Do not expose C++ virtual classes, STL containers, exceptions, RTTI-dependent objects, or raw TrinityCore pointers.

## Manifest and package signing

Start with:

```json
{
  "name": "my-module",
  "version": "1.0.0",
  "sdk": ">=1.0 <2.0",
  "depends": [],
  "load_order": 100,
  "databases": ["world"]
}
```

Sign the final package after building the library and copying every configuration and SQL resource:

```sh
python /path/to/trinitycore-module-sdk/bin/sign_module.py \
  --manifest build/bin/RelWithDebInfo/modules/my-module/module.json \
  --library build/bin/RelWithDebInfo/modules/my-module/my-module.so \
  --package-root build/bin/RelWithDebInfo/modules/my-module \
  --private-key /secure/keys/publisher-private.key \
  --key-id publisher-2026
```

Use `.dll` instead of `.so` on Windows.

The signing tool:

- hashes every regular package file except `module.json`;
- writes the `files` SHA-256 map;
- canonicalizes the manifest with sorted keys;
- writes the Ed25519 signature;
- binds the signature to the final binary and resources.

Never modify a package after signing. Never deploy the private key.

## Deploy to TrinityCore

Install the public key on the server:

```text
<worldserver>/keys/publisher-2026.pub
```

Configure the server:

```ini
Modules.TrustedKeyDirectory = "keys"
```

Copy the complete signed package without flattening it:

```text
<worldserver>/modules/my-module/
├── my-module.dll
├── module.json
├── conf/
└── sql/
```

Start `worldserver` from its server directory. Verify logs contain:

```text
Discovered <N> module manifest(s)
Loaded module my-module v<version>
worldserver ... ready
```

The loader rejects invalid signatures, changed resource hashes, unsafe paths, symlinks, malformed manifests, missing dependencies, and unsupported ABI versions before loading native code.

## TrinityCore integration

TrinityCore is configured against an installed SDK package:

```sh
cmake -S /path/to/TrinityCore -B /path/to/core-build \
  -DCMAKE_PREFIX_PATH=/path/to/trinitycore-module-sdk
cmake --build /path/to/core-build --config RelWithDebInfo --parallel
```

The core repository does not contain or build the SDK source tree. The core host and this SDK must agree on the ABI version and supported capabilities.

## Examples and full guide

The companion repository contains working examples:

- `mod-greeting` provides a signed C ABI service;
- `mod-example` consumes that service and registers a command;
- both use external SDK headers only.

Repository: [TrinityCoreModulesExamples](https://github.com/julienguillot77/TrinityCoreModulesExamples)

End-to-end guide: [DEVELOPMENT.md](https://github.com/julienguillot77/TrinityCoreModuleSDK/blob/master/DEVELOPMENT.md)

## Current support boundary

The stable SDK currently supports module lifecycle, logging, configuration, services, commands, world callbacks, and broad Spell/Aura infrastructure. Generated declarations for other script families do not imply runtime support; unsupported registrations fail closed until the TrinityCore host implements the corresponding adapter.
