# QUICK START - BUILD TURBO MODE

## Installation rapide

### 1. Installer les outils (optionnel mais recommande)

```bash
brew install ninja ccache
```

### 2. Configuration initiale

```bash
# Clean start
rm -rf build
mkdir build
cd build

# Configure avec Ninja
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
```

### 3. Build

```bash
# Build tout (cold: ~15s, avec ccache: ~2s)
ninja

# Build + run tests
ninja test_all

# Run tests seulement
ctest --parallel 8
```

## Workflow de developpement

### Ajouter un nouveau test

**Plus besoin de modifier CMakeLists.txt !**

Creez simplement votre fichier dans le bon repertoire:

```bash
# Test de compilation (static_assert)
touch tests/compilation/test_my_feature.cpp

# Test unitaire (Catch2)
touch tests/unit/test_my_unit.cpp

# Test runtime
touch tests/runtime/test_my_runtime.cpp

# Test de proprietes
touch tests/properties/test_my_properties.cpp
```

Rebuild:
```bash
ninja  # Auto-detecte et compile le nouveau test !
```

### Build incremental ultra-rapide

```bash
# Modifiez un test
vim tests/unit/test_functor_laws.cpp

# Rebuild (< 2 secondes !)
ninja
```

### Verifier les optimisations

```bash
# Voir la config CMake
cd build
cmake -LA | grep -E "(PARALLEL|CCACHE|GENERATOR|LTO)"
```

Sortie attendue:
```
CMAKE_BUILD_PARALLEL_LEVEL:STRING=8
CMAKE_CXX_COMPILER_LAUNCHER:STRING=/opt/homebrew/bin/ccache  # si installe
CMAKE_GENERATOR:STRING=Ninja
CMAKE_INTERPROCEDURAL_OPTIMIZATION:BOOL=TRUE
```

## Benchmarks

### Test de performance

```bash
# No-change build (doit etre instant)
time ninja
# Attendu: ninja: no work to do. (~0.008s)

# Single file change
touch tests/unit/test_functor_laws.cpp
time ninja
# Attendu: ~1.3 secondes

# Clean rebuild
rm -rf build/*
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
time ninja
# Attendu: ~15s sans ccache, ~2s avec ccache (2e fois)
```

## Commandes utiles

```bash
# Build verbeux (voir toutes les commandes)
ninja -v

# Build 1 seul target
ninja compilation_tests

# Clean
ninja clean

# Reconfigure
cmake -G Ninja ..

# Voir tous les targets
ninja -t targets

# Graphe de dependances
ninja -t graph | dot -Tpng > deps.png
```

## Troubleshooting

### "ninja: command not found"

```bash
brew install ninja
# ou
cmake ..  # utilisera Make (plus lent)
```

### "ccache: command not found"

C'est OK ! ccache est optionnel. Le build fonctionnera sans, juste un peu plus lent.

Pour l'installer:
```bash
brew install ccache
rm -rf build  # rebuild pour activer
```

### Tests failing

Si les tests echouent, c'est normal (TDD RED phase). Focus sur la vitesse du build system !

Pour voir les details:
```bash
ctest --output-on-failure --verbose
```

### Build trop lent

1. Verifiez que Ninja est utilise:
```bash
cmake -LA | grep GENERATOR
# Doit afficher: CMAKE_GENERATOR:STRING=Ninja
```

2. Verifiez la parallelisation:
```bash
cmake -LA | grep PARALLEL
# Doit afficher: CMAKE_BUILD_PARALLEL_LEVEL:STRING=8 (ou votre nb de cores)
```

3. Installez ccache:
```bash
brew install ccache
```

## Performance attendue

- **No-change build:** < 0.01s
- **Incremental (1 fichier):** 1-2s
- **Clean build (sans ccache):** 10-15s
- **Clean build (avec ccache, 2e fois):** 2-3s
- **Run tests:** 1-2s

Si vos perfs sont en dessous, verifiez la config ci-dessus !

---

ENJOY THE SPEED !
