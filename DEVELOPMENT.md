# TrinityCore modules: end-to-end development, signing, deployment, and testing

This guide takes a module from an empty directory to a signed package loaded by `worldserver`.

The workflow is:

1. build and install a TrinityCore Module SDK package;
2. create the module source, CMake target, manifest, configuration, and SQL resources;
3. build the module outside the TrinityCore source tree;
4. sign the complete package and bind its files to the signature;
5. install the trusted public key on the server;
6. deploy the package and configuration;
7. start `worldserver`, verify module discovery/load, and test behavior;
8. update or roll back the package safely.

The current module API is stable C ABI v1. Modules use SDK-owned C-compatible tables and opaque handles. Native modules are trusted in-process code: a loaded module has the privileges of `worldserver`. Signatures provide publisher authenticity and package integrity; they do not sandbox native code.

## 1. Requirements

### Build tools

Install:

- CMake 3.24 or newer;
- Python 3;
- a C++17 compiler;
- the compiler runtime required by the target server;
- Git, if obtaining the SDK and examples from repositories.

The module and core must use compatible architecture, compiler runtime, and build configuration. The stable C ABI removes TrinityCore C++ class-layout dependencies, but a native DLL still depends on its platform runtime.

### Runtime requirements

You need:

- a built `worldserver.exe` or `worldserver` from the TrinityCore revision you target;
- the runtime DLLs/shared libraries required by that executable and module;
- a running TrinityCore database installation;
- database credentials in the active worldserver configuration;
- permission to write the server's `modules/` and `keys/` directories.

The database port is configured by the normal TrinityCore database settings. This guide does not assume a particular port; use the port configured by your server. For example, a local MySQL instance may use port `3317`.

## 2. Build and install the SDK

Build the SDK from the TrinityCore checkout that produced the `worldserver` executable. Use a user-writable install prefix.

### Windows / Visual Studio

```powershell
cmake -S C:/src/TrinityCore -B C:/build/trinitycore `
  -DCMAKE_INSTALL_PREFIX=C:/sdk/TrinityCore `
  -DBUILD_TESTING=OFF
cmake --build C:/build/trinitycore --config RelWithDebInfo --parallel
cmake --install C:/build/trinitycore --config RelWithDebInfo
```

### Linux / Ninja

```sh
cmake -S /src/TrinityCore -B /build/trinitycore -GNinja \
  -DCMAKE_INSTALL_PREFIX=/opt/trinitycore-sdk \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DBUILD_TESTING=OFF
cmake --build /build/trinitycore --parallel
cmake --install /build/trinitycore
```

The install prefix must contain at least:

```text
include/trinity/TrinityModule.h
include/trinity/TrinityAbiV1.h
include/trinity/TrinitySDKBuild.h
lib/cmake/TrinitySDK/TrinitySDKConfig.cmake
lib/cmake/TrinitySDK/TrinitySDKModules.cmake
bin/sign_module.py
```

Do not copy private TrinityCore headers into a module repository. Modules MUST include only SDK-owned headers and their own public headers.

## 3. Create a module repository

Recommended layout:

```text
my-module/
├── CMakeLists.txt
├── module.json
├── src/
│   └── my_module.cpp
├── include/                    # optional module-owned public headers
├── conf/
│   └── my-module.conf.dist     # optional default configuration
└── sql/
    └── world/
        └── my_module_001.sql   # optional database update
```

The module name is part of the runtime path. Use lowercase ASCII letters, digits, `-`, and `_`; keep the directory name, CMake `NAME`, manifest `name`, output library name, and package directory identical.

## 4. Configure the module build

Minimal external `CMakeLists.txt`:

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
    SQL ${CMAKE_CURRENT_SOURCE_DIR}/sql/world/my_module_001.sql
    INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
```

Remove `CONFIG`, `SQL`, or `INCLUDE_DIRS` when that resource is not used. `trinity_add_module()` links only `Trinity::SDK`; it MUST NOT link `game`, `worldserver`, or private TrinityCore targets.

Configure from the module repository root:

```sh
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/path/to/TrinityCoreSDK
```

For Visual Studio:

```powershell
cmake -S . -B build `
  -DCMAKE_PREFIX_PATH=C:/sdk/TrinityCore `
  -G "Visual Studio 18 2026" -A x64
```

## 5. Write the manifest

The loader requires these fields:

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

The signing command adds these release fields:

```json
{
  "key_id": "publisher-2026",
  "files": {
    "my-module.dll": "<sha256>",
    "conf/my-module.conf.dist": "<sha256>",
    "sql/world/my_module_001.sql": "<sha256>"
  },
  "signature": "<ed25519-signature-hex>"
}
```

Additional rules:

- `name` MUST match the module directory and CMake `NAME`.
- `version` must be a valid numeric release version.
- `sdk` must declare a supported ABI range; the current loader accepts `>=1.0 <2.0`.
- `depends` contains module dependency expressions, for example `mod-greeting >=1.0`.
- `load_order` controls ordering after dependency resolution.
- `databases` lists databases whose SQL resources the module supplies, using `world`, `characters`, `auth`, or `hotfixes` as appropriate for the server integration.
- `key_id` is a safe filename identifier and resolves to `keys/<key_id>.pub`.
- `files` MUST hash every regular package file except `module.json`.
- `signature` is generated last, after the binary and every resource are final.

Do not manually type hashes. Use the signing tool.

## 6. Write the stable C ABI entry point

A minimal lifecycle-only module is:

```cpp
#include "TrinityModule.h"

namespace
{
int32_t Load(void*, trinity_host const* host)
{
    if (!host)
        return -1;

    trinity_host_api_v1 const* api = TrinityHostApiV1(host);
    if (!api || api->abi_version != TRINITY_ABI_V1 ||
        api->size < sizeof(*api) || !api->log.write)
        return -1;

    constexpr char message[] = "my-module loaded.";
    api->log.write(const_cast<trinity_host*>(host), 1, message, sizeof(message) - 1);
    return 0;
}

void Unload(void*, trinity_host const*)
{
}

void ConfigReload(void*, trinity_host const*)
{
}
}

extern "C" int32_t TrinityModuleGetApi(uint32_t requested_abi, trinity_module_api* output)
{
    if (requested_abi != TRINITY_ABI_V1 || !output)
        return -1;

    *output = {
        sizeof(*output),
        TRINITY_ABI_V1,
        nullptr,
        Load,
        Unload,
        ConfigReload};
    return 0;
}
```

Do not export C++ classes, virtual classes, STL objects, exceptions, RTTI-dependent types, or raw TrinityCore pointers. Use the generated SDK tables, fixed-width integers, caller-owned buffers, and opaque handles.

## 7. Use configuration

A distributed default configuration file is copied into the module package's `conf/` directory:

```ini
[worldserver]
Module.my-module.Enabled = 1
Module.my-module.Label = Example
```

Retrieve values through the configuration capability:

```cpp
auto const* config = static_cast<trinity_config_api_v1 const*>(
    api->query.get_capability(
        const_cast<trinity_host*>(host),
        TRINITY_CAPABILITY_CONFIG_V1));

char label[128]{};
if (config && config->size >= sizeof(*config) && config->get_string)
    config->get_string(
        const_cast<trinity_host*>(host),
        "Module.my-module.Label",
        "default",
        label,
        sizeof(label));
```

Configuration access uses caller-owned output buffers. Check the returned required size and never retain host-owned pointers beyond their documented lifetime.

## 8. Register services

Define a module-owned, versioned C service table:

```cpp
struct MyService
{
    uint32_t size;
    uint32_t version;
    void* context;
    int32_t (*operation)(void*, char const*, char*, size_t);
};
```

Register it through `api->query.register_service`. Consumers retrieve it by the agreed numeric service ID through `get_service`, then validate `size` and `version` before calling it.

The provider MUST remain loaded while consumers use the service. Declare the provider in the consumer manifest's `depends` list.

## 9. Register commands

Command registration uses a C callback and explicit permission/console policy:

```cpp
int32_t Execute(void*, trinity_command_context*, char const*, size_t)
{
    return 0;
}

trinity_command_registration command{
    sizeof(command),
    "module my-module status",
    "Show module status.",
    Execute,
    nullptr,
    371, // existing GM permission in this core build
    1};  // console allowed
```

Register with `api->query.register_command`. Command names and help text must remain valid for the registration lifetime. Use the command capability to send a response through `trinity_command_context`; do not access `ChatHandler` directly.

## 10. Register world callbacks

The currently supported runtime callback family is the world family:

```cpp
void Startup(void*, trinity_host const* host, uint8_t)
{
    constexpr char message[] = "my-module world startup.";
    TrinityHostApiV1(host)->log.write(
        const_cast<trinity_host*>(host), 1, message, sizeof(message) - 1);
}

trinity_world_callbacks callbacks{};
callbacks.size = sizeof(callbacks);
callbacks.host = host;
callbacks.OnStartup = Startup;
```

Register it with:

```cpp
api->query.register_family(
    const_cast<trinity_host*>(host),
    TRINITY_FAMILY_WORLD,
    &callbacks);
```

Unsupported generated callback families fail closed until their concrete core adapter is enabled. Do not register an unsupported family and do not assume a declaration alone means runtime dispatch exists.

## 11. Build the module

```sh
cmake --build build --config RelWithDebInfo --parallel
```

The output layout is:

```text
build/bin/RelWithDebInfo/modules/my-module/
├── my-module.dll          # Windows
├── module.json
├── conf/
└── sql/
```

On Linux the library is normally `.so`.

Before signing, check that:

- the library exists;
- `module.json` exists;
- all configuration and SQL resources were copied;
- the module directory name matches the manifest name;
- dependencies are present in the deployment directory;
- the build architecture/runtime matches `worldserver`.

## 12. Sign the package

Generate or obtain an Ed25519 private key outside the server directory. Never deploy the private key.

For a test key with Python `cryptography`:

```powershell
python -c "from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PrivateKey; from pathlib import Path; k=Ed25519PrivateKey.generate(); Path('module-test-private.key').write_bytes(k.private_bytes_raw()); Path('module-test-public.pub').write_bytes(k.public_key().public_bytes_raw())"
```

Sign a built package from the package root:

```powershell
python C:/path/to/TrinityCore/sdk/tools/sign_module.py `
  --manifest build/bin/RelWithDebInfo/modules/my-module/module.json `
  --library build/bin/RelWithDebInfo/modules/my-module/my-module.dll `
  --package-root build/bin/RelWithDebInfo/modules/my-module `
  --private-key C:/keys/module-private.key `
  --key-id publisher-2026
```

POSIX equivalent:

```sh
python /path/to/TrinityCore/sdk/tools/sign_module.py \
  --manifest build/bin/RelWithDebInfo/modules/my-module/module.json \
  --library build/bin/RelWithDebInfo/modules/my-module/my-module.so \
  --package-root build/bin/RelWithDebInfo/modules/my-module \
  --private-key /secure/keys/module-private.key \
  --key-id publisher-2026
```

The signer inventories every regular file except `module.json`, computes SHA-256 values, canonicalizes the signed manifest with sorted keys, and writes the detached Ed25519 signature into `module.json`.

After signing, do not modify, rebuild, strip, copy-over, or replace any package file. Any change invalidates the signed file hashes.

## 13. Prepare the server

Stop `worldserver` before deployment. Back up the relevant database if the module supplies SQL.

Install the trusted public key:

```text
<server>/keys/publisher-2026.pub
```

The active worldserver configuration MUST contain:

```ini
Modules.TrustedKeyDirectory = "keys"
```

The directory is relative to the worldserver working directory unless configured otherwise. A key file must be exactly 32 raw bytes or a valid 64-character hex public key. The private key MUST NOT be copied to the server.

## 14. Deploy the package

Copy the complete signed module directory, preserving the layout:

```text
<server>/modules/my-module/
├── my-module.dll
├── module.json
├── conf/
└── sql/
```

Windows PowerShell example:

```powershell
$server = 'C:/servers/ForeverRepack/server'
New-Item -ItemType Directory -Force "$server/modules/my-module" | Out-Null
Copy-Item -Recurse -Force `
  'build/bin/RelWithDebInfo/modules/my-module/*' `
  "$server/modules/my-module/"
Copy-Item -Force 'C:/keys/publisher-2026.pub' "$server/keys/"
```

Do not flatten the package. Do not copy only the DLL. The loader verifies every file listed by `files` before loading the library.

Merge the distributed module configuration into the active server configuration. Keep module defaults in the module package and operator overrides in the server configuration.

## 15. Apply SQL resources

SQL resources are copied under `sql/<database>/` and listed in the manifest's `databases` field.

The server's normal database updater validates the signed package before accepting module SQL. Keep SQL files:

- ordered with stable numeric prefixes;
- idempotent where the update system requires idempotency;
- scoped to module-owned tables/data where possible;
- compatible with the target TrinityCore schema;
- documented for rollback or backup requirements.

Do not run unsigned or manually modified SQL from a signed package. If a package changes after signing, sign it again.

The current core update callback is integrated with the World update include path. Treat non-World database resources as requiring explicit verification against the target core's database-update integration before release; do not assume every database pool is automatically registered by the same table.

## 16. Start and verify

Start from the server directory so relative paths resolve correctly:

```powershell
cd C:/servers/ForeverRepack/server
./worldserver.exe -c worldserver.conf
```

On Windows PowerShell, if `./worldserver.exe` is not accepted by the shell, run:

```powershell
& C:/servers/ForeverRepack/server/worldserver.exe -c C:/servers/ForeverRepack/server/worldserver.conf
```

The server log MUST show the following sequence:

```text
Discovered <N> module manifest(s)
Loaded module my-module v<version>
worldserver ... ready
```

Also verify module-specific behavior. For the examples repository:

```text
Hello, world! mod-greeting is ready.
mod-example started; use 'module example hello'.
```

The example command is available to an authorized GM or console:

```text
.module example hello
```

Expected result: the command obtains the greeting service and sends the greeting through the command capability.

If the server rejects a module, stop and fix the first error. Common failures:

| Log symptom | Cause | Fix |
|---|---|---|
| `invalid module manifest` | Missing/unknown/duplicate field or invalid JSON | Rebuild/sign the manifest with the current tool |
| `module directory does not match manifest name` | Directory and `name` differ | Rename the directory or manifest consistently |
| signature/key failure | Wrong key, changed manifest, or changed file | Install the matching `.pub` key and re-sign the final package |
| resource hash failure | A file changed after signing | Re-sign after the final copy/build |
| dependency failure | Missing or incompatible dependency | Deploy dependencies first and match versions |
| ABI failure | Wrong SDK ABI or malformed table | Rebuild against the installed SDK and validate table sizes |
| DLL load failure | Missing runtime dependency or wrong architecture | Install/copy the required runtime and match x64/x86 |
| module callback failure | Module `on_load` returned an error | Inspect module logs and capability checks |

## 17. Test procedure

### Build test

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/TrinityCoreSDK
cmake --build build --config RelWithDebInfo --parallel
```

### Artifact test

Verify the module directory contains the complete package and that the manifest contains `key_id`, `files`, and `signature` after signing.

### ABI export test

Windows PowerShell/Python:

```powershell
python -c "import ctypes; h=ctypes.WinDLL('build/bin/RelWithDebInfo/modules/my-module/my-module.dll'); print(bool(h.TrinityModuleGetApi))"
```

### Runtime test

Run the target worldserver with the package installed, then verify discovery, signature acceptance, load output, module behavior, and clean shutdown.

### Failure test

For release validation, make a disposable copy of the package, change one byte in the DLL or a listed resource, and confirm the loader rejects it. Restore the original signed package afterward.

## 18. Release packaging

A release archive should preserve:

```text
my-module-1.0.0/
└── modules/
    └── my-module/
        ├── my-module.dll
        ├── module.json
        ├── conf/
        └── sql/
```

Package only after the final build and signing operation:

```powershell
Compress-Archive -Path build/bin/RelWithDebInfo/modules/my-module `
  -DestinationPath my-module-1.0.0.zip -Force
```

Record with the release:

- module version;
- SDK ABI and capability requirements;
- TrinityCore revision used for runtime validation;
- compiler/runtime and architecture;
- public key ID;
- database migrations and backup requirements;
- dependency versions;
- tested worldserver configuration.

Publish the public key separately through a trusted channel. Never publish the private signing key.

## 19. Upgrade and rollback

For an upgrade:

1. stop `worldserver`;
2. back up databases affected by module SQL;
3. retain the previous signed package;
4. install the new public key if the publisher key changed;
5. deploy the complete new package atomically;
6. apply documented migrations;
7. start `worldserver` and verify logs and behavior.

For rollback:

1. stop `worldserver`;
2. restore the previous signed module directory;
3. restore the database backup if the new migration is not backward-compatible;
4. start the matching core/module combination;
5. verify the old package signature and behavior.

Never disable signature verification to recover from a package failure. Fix the key, manifest, resource hash, dependency, or deployment contents.

## 20. Current ABI support boundary

The SDK currently supports stable lifecycle, logging, configuration, services, commands, world callbacks, and broad Spell/Aura infrastructure. Generated declarations for other script families do not imply runtime support: unsupported families fail closed until a concrete core adapter exists.

A module MUST use only capabilities supported by the target worldserver. Test the exact package against the exact server build before release.
